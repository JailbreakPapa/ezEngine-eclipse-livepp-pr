#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/RenderPipelineNode.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/ReadbackHelper.h>
#include <RendererFoundation/Resources/Texture.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>

namespace
{
  struct PendingPipelineInfo
  {
    struct OutputInfo
    {
      ezString m_sPinName;
      ezUInt32 m_uiWidth = 0;
      ezUInt32 m_uiHeight = 0;
      ezString m_sFormat;
      bool m_bHasTexture = false;
    };

    struct PassInfo
    {
      ezString m_sPassName;
      ezDynamicArray<OutputInfo> m_Outputs;
    };

    struct ViewInfo
    {
      ezString m_sViewName;
      ezDynamicArray<PassInfo> m_Passes;
    };

    ezDynamicArray<ViewInfo> m_Views;
    bool m_bDataReady = false;
  };

  struct CaptureRequest
  {
    ezString m_sViewName;
    ezString m_sPassName;
    ezString m_sPinName;
    bool m_bPending = false;
  };

  struct CapturedTexture
  {
    ezString m_sViewName;
    ezString m_sPassName;
    ezString m_sPinName;
    ezUInt32 m_uiWidth = 0;
    ezUInt32 m_uiHeight = 0;
    ezString m_sFormat;
    ezDynamicArray<ezUInt8> m_PixelData; // RGBA8
    bool m_bDataReady = false;
    ezString m_sError;
  };

  static ezMutex s_Mutex;
  static PendingPipelineInfo s_PipelineInfo;
  static CaptureRequest s_CaptureRequest;
  static CapturedTexture s_CapturedTexture;
  static bool s_bCollectPipelineInfo = false;
  static ezTime s_CollectPipelineInfoRequestTime;
  static ezGALReadbackTextureHelper s_Readback;
  static ezGALTextureHandle s_hTextureToCapture;
  static bool s_bReadbackInProgress = false;
  static ezEventSubscriptionID s_RenderEventSubscription = 0;
  static bool s_bRenderEventHandlerAdded = false;

  ezString GetFormatName(ezGALResourceFormat::Enum format)
  {
    switch (format)
    {
      case ezGALResourceFormat::RGBAFloat:
        return "RGBAFloat";
      case ezGALResourceFormat::RGBAUInt:
        return "RGBAUInt";
      case ezGALResourceFormat::RGBAInt:
        return "RGBAInt";
      case ezGALResourceFormat::RGBFloat:
        return "RGBFloat";
      case ezGALResourceFormat::RGBUInt:
        return "RGBUInt";
      case ezGALResourceFormat::RGBInt:
        return "RGBInt";
      case ezGALResourceFormat::BGRAUByteNormalized:
        return "BGRAUByteNormalized";
      case ezGALResourceFormat::BGRAUByteNormalizedsRGB:
        return "BGRAUByteNormalizedsRGB";
      case ezGALResourceFormat::RGBAHalf:
        return "RGBAHalf";
      case ezGALResourceFormat::RGBAUShort:
        return "RGBAUShort";
      case ezGALResourceFormat::RGBAUShortNormalized:
        return "RGBAUShortNormalized";
      case ezGALResourceFormat::RGBAShort:
        return "RGBAShort";
      case ezGALResourceFormat::RGBAShortNormalized:
        return "RGBAShortNormalized";
      case ezGALResourceFormat::RGFloat:
        return "RGFloat";
      case ezGALResourceFormat::RGUInt:
        return "RGUInt";
      case ezGALResourceFormat::RGInt:
        return "RGInt";
      case ezGALResourceFormat::RGB10A2UInt:
        return "RGB10A2UInt";
      case ezGALResourceFormat::RGB10A2UIntNormalized:
        return "RGB10A2UIntNormalized";
      case ezGALResourceFormat::RG11B10Float:
        return "RG11B10Float";
      case ezGALResourceFormat::RGBAUByteNormalized:
        return "RGBAUByteNormalized";
      case ezGALResourceFormat::RGBAUByteNormalizedsRGB:
        return "RGBAUByteNormalizedsRGB";
      case ezGALResourceFormat::RGBAUByte:
        return "RGBAUByte";
      case ezGALResourceFormat::RGBAByteNormalized:
        return "RGBAByteNormalized";
      case ezGALResourceFormat::RGBAByte:
        return "RGBAByte";
      case ezGALResourceFormat::RGHalf:
        return "RGHalf";
      case ezGALResourceFormat::RGUShort:
        return "RGUShort";
      case ezGALResourceFormat::RGUShortNormalized:
        return "RGUShortNormalized";
      case ezGALResourceFormat::RGShort:
        return "RGShort";
      case ezGALResourceFormat::RGShortNormalized:
        return "RGShortNormalized";
      case ezGALResourceFormat::RGUByte:
        return "RGUByte";
      case ezGALResourceFormat::RGUByteNormalized:
        return "RGUByteNormalized";
      case ezGALResourceFormat::RGByte:
        return "RGByte";
      case ezGALResourceFormat::RGByteNormalized:
        return "RGByteNormalized";
      case ezGALResourceFormat::DFloat:
        return "DFloat";
      case ezGALResourceFormat::RFloat:
        return "RFloat";
      case ezGALResourceFormat::RUInt:
        return "RUInt";
      case ezGALResourceFormat::RInt:
        return "RInt";
      case ezGALResourceFormat::RHalf:
        return "RHalf";
      case ezGALResourceFormat::RUShort:
        return "RUShort";
      case ezGALResourceFormat::RUShortNormalized:
        return "RUShortNormalized";
      case ezGALResourceFormat::RShort:
        return "RShort";
      case ezGALResourceFormat::RShortNormalized:
        return "RShortNormalized";
      case ezGALResourceFormat::RUByte:
        return "RUByte";
      case ezGALResourceFormat::RUByteNormalized:
        return "RUByteNormalized";
      case ezGALResourceFormat::RByte:
        return "RByte";
      case ezGALResourceFormat::RByteNormalized:
        return "RByteNormalized";
      case ezGALResourceFormat::AUByteNormalized:
        return "AUByteNormalized";
      case ezGALResourceFormat::D16:
        return "D16";
      case ezGALResourceFormat::D24S8:
        return "D24S8";
      case ezGALResourceFormat::BC1:
        return "BC1";
      case ezGALResourceFormat::BC1sRGB:
        return "BC1sRGB";
      case ezGALResourceFormat::BC2:
        return "BC2";
      case ezGALResourceFormat::BC2sRGB:
        return "BC2sRGB";
      case ezGALResourceFormat::BC3:
        return "BC3";
      case ezGALResourceFormat::BC3sRGB:
        return "BC3sRGB";
      case ezGALResourceFormat::BC4UNormalized:
        return "BC4UNormalized";
      case ezGALResourceFormat::BC4Normalized:
        return "BC4Normalized";
      case ezGALResourceFormat::BC5UNormalized:
        return "BC5UNormalized";
      case ezGALResourceFormat::BC5Normalized:
        return "BC5Normalized";
      case ezGALResourceFormat::BC6UFloat:
        return "BC6UFloat";
      case ezGALResourceFormat::BC6Float:
        return "BC6Float";
      case ezGALResourceFormat::BC7UNormalized:
        return "BC7UNormalized";
      case ezGALResourceFormat::BC7UNormalizedsRGB:
        return "BC7UNormalizedsRGB";
      default:
        return "Unknown";
    }
  }

  void CollectPipelineInfo(const ezRenderWorldRenderEvent& e)
  {
    if (e.m_pPipeline == nullptr)
      return;

    EZ_LOCK(s_Mutex);

    // Find the view for this pipeline
    ezString viewName = e.m_pPipeline->GetViewName().GetString();
    if (viewName.IsEmpty())
      viewName = "Unknown View";

    // Check if this view already exists
    PendingPipelineInfo::ViewInfo* pViewInfo = nullptr;
    for (auto& view : s_PipelineInfo.m_Views)
    {
      if (view.m_sViewName == viewName)
      {
        pViewInfo = &view;
        break;
      }
    }

    if (pViewInfo == nullptr)
    {
      pViewInfo = &s_PipelineInfo.m_Views.ExpandAndGetRef();
      pViewInfo->m_sViewName = viewName;
    }

    // Clear and rebuild pass info
    pViewInfo->m_Passes.Clear();

    // Get all passes from the pipeline
    ezDynamicArray<const ezRenderPipelinePass*> passes;
    e.m_pPipeline->GetPasses(passes);

    for (const ezRenderPipelinePass* pPass : passes)
    {
      auto& passInfo = pViewInfo->m_Passes.ExpandAndGetRef();
      passInfo.m_sPassName = pPass->GetName();

      // Iterate output pins
      for (const ezRenderPipelineNodePin* pPin : pPass->GetOutputPins())
      {
        ezHashedString pinName = pPass->GetPinName(pPin);
        const ezRenderPipelinePassConnection* pConn = e.m_pPipeline->GetOutputConnection(pPass, pinName);

        auto& outputInfo = passInfo.m_Outputs.ExpandAndGetRef();
        outputInfo.m_sPinName = pinName.GetString();

        if (pConn && !pConn->m_TextureHandle.IsInvalidated())
        {
          outputInfo.m_bHasTexture = true;
          outputInfo.m_uiWidth = pConn->m_Desc.m_uiWidth;
          outputInfo.m_uiHeight = pConn->m_Desc.m_uiHeight;
          outputInfo.m_sFormat = GetFormatName(pConn->m_Desc.m_Format);
        }
      }
    }

    s_PipelineInfo.m_bDataReady = true;
  }

  void HandleTextureCapture(const ezRenderWorldRenderEvent& e)
  {
    if (e.m_pPipeline == nullptr)
      return;

    EZ_LOCK(s_Mutex);

    if (!s_CaptureRequest.m_bPending)
      return;

    // Check if this is the pipeline for the requested view
    ezString viewName = e.m_pPipeline->GetViewName().GetString();
    if (viewName.IsEmpty())
      viewName = "Unknown View";

    if (viewName != s_CaptureRequest.m_sViewName)
      return;

    // Find the requested pass and pin
    ezDynamicArray<const ezRenderPipelinePass*> passes;
    e.m_pPipeline->GetPasses(passes);

    for (const ezRenderPipelinePass* pPass : passes)
    {
      if (pPass->GetName() != s_CaptureRequest.m_sPassName)
        continue;

      for (const ezRenderPipelineNodePin* pPin : pPass->GetOutputPins())
      {
        ezHashedString pinName = pPass->GetPinName(pPin);
        if (pinName.GetString() != s_CaptureRequest.m_sPinName)
          continue;

        const ezRenderPipelinePassConnection* pConn = e.m_pPipeline->GetOutputConnection(pPass, pinName);
        if (!pConn || pConn->m_TextureHandle.IsInvalidated())
        {
          s_CapturedTexture.m_sError = "Texture handle is invalid";
          s_CapturedTexture.m_bDataReady = true;
          s_CaptureRequest.m_bPending = false;
          return;
        }

        // Store texture info for readback
        s_CapturedTexture.m_sViewName = s_CaptureRequest.m_sViewName;
        s_CapturedTexture.m_sPassName = s_CaptureRequest.m_sPassName;
        s_CapturedTexture.m_sPinName = s_CaptureRequest.m_sPinName;
        s_CapturedTexture.m_uiWidth = pConn->m_Desc.m_uiWidth;
        s_CapturedTexture.m_uiHeight = pConn->m_Desc.m_uiHeight;
        s_CapturedTexture.m_sFormat = GetFormatName(pConn->m_Desc.m_Format);

        // Start the readback
        s_hTextureToCapture = pConn->m_TextureHandle;
        s_bReadbackInProgress = true;
        s_CaptureRequest.m_bPending = false;
        return;
      }
    }

    // Pass/pin not found
    s_CapturedTexture.m_sError = "Pass or output pin not found";
    s_CapturedTexture.m_bDataReady = true;
    s_CaptureRequest.m_bPending = false;
  }

  void PerformReadback()
  {
    if (!s_bReadbackInProgress)
      return;

    ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
    if (pDevice == nullptr)
    {
      EZ_LOCK(s_Mutex);
      s_CapturedTexture.m_sError = "No GAL device available";
      s_CapturedTexture.m_bDataReady = true;
      s_bReadbackInProgress = false;
      return;
    }

    // Start the readback
    {
      auto pCommandEncoder = pDevice->BeginCommands("InspectorTextureCapture");
      if (pCommandEncoder == nullptr)
      {
        EZ_LOCK(s_Mutex);
        s_CapturedTexture.m_sError = "Failed to begin commands";
        s_CapturedTexture.m_bDataReady = true;
        s_bReadbackInProgress = false;
        return;
      }

      EZ_SCOPE_EXIT(pDevice->EndCommands(pCommandEncoder));
      s_Readback.ReadbackTexture(*pCommandEncoder, s_hTextureToCapture);
    }

    // Wait for readback to complete
    ezEnum<ezGALAsyncResult> result = s_Readback.GetReadbackResult(ezTime::MakeFromSeconds(5.0));
    if (result != ezGALAsyncResult::Ready)
    {
      EZ_LOCK(s_Mutex);
      s_CapturedTexture.m_sError = "Readback timed out";
      s_CapturedTexture.m_bDataReady = true;
      s_bReadbackInProgress = false;
      return;
    }

    // Lock and copy the texture data
    ezGALTextureSubresource subResource;
    ezArrayPtr<ezGALTextureSubresource> subResources(&subResource, 1);
    ezHybridArray<ezGALSystemMemoryDescription, 1> memory;

    ezReadbackTextureLock lock = s_Readback.LockTexture(subResources, memory);
    if (!lock)
    {
      EZ_LOCK(s_Mutex);
      s_CapturedTexture.m_sError = "Failed to lock readback texture";
      s_CapturedTexture.m_bDataReady = true;
      s_bReadbackInProgress = false;
      return;
    }

    // Convert to RGBA8 via ezImage
    const ezGALTexture* pTexture = pDevice->GetTexture(s_hTextureToCapture);
    if (pTexture == nullptr)
    {
      EZ_LOCK(s_Mutex);
      s_CapturedTexture.m_sError = "Texture no longer valid";
      s_CapturedTexture.m_bDataReady = true;
      s_bReadbackInProgress = false;
      return;
    }

    ezImage image;
    ezTextureUtils::CopySubResourceToImage(pTexture->GetDescription(), subResource, memory[0], image, true);

    // Convert to RGBA8 if needed
    if (image.GetImageFormat() != ezImageFormat::R8G8B8A8_UNORM)
    {
      ezImage converted;
      if (ezImageConversion::Convert(image, converted, ezImageFormat::R8G8B8A8_UNORM).Succeeded())
      {
        image = std::move(converted);
      }
    }

    // Copy pixel data
    EZ_LOCK(s_Mutex);
    const ezUInt32 pixelCount = s_CapturedTexture.m_uiWidth * s_CapturedTexture.m_uiHeight * 4;
    s_CapturedTexture.m_PixelData.SetCount(pixelCount);

    if (image.GetImageFormat() == ezImageFormat::R8G8B8A8_UNORM)
    {
      ezMemoryUtils::Copy(s_CapturedTexture.m_PixelData.GetData(), image.GetPixelPointer<ezUInt8>(), pixelCount);
    }
    else
    {
      // Fallback: fill with magenta to indicate unsupported format
      for (ezUInt32 i = 0; i < pixelCount; i += 4)
      {
        s_CapturedTexture.m_PixelData[i + 0] = 255; // R
        s_CapturedTexture.m_PixelData[i + 1] = 0;   // G
        s_CapturedTexture.m_PixelData[i + 2] = 255; // B
        s_CapturedTexture.m_PixelData[i + 3] = 255; // A
      }
    }

    s_CapturedTexture.m_sError.Clear();
    s_CapturedTexture.m_bDataReady = true;
    s_bReadbackInProgress = false;
  }

  void OnRenderEvent(const ezRenderWorldRenderEvent& e)
  {
    if (e.m_Type == ezRenderWorldRenderEvent::Type::AfterPipelineExecution)
    {
      if (s_bCollectPipelineInfo)
      {
        CollectPipelineInfo(e);
      }

      HandleTextureCapture(e);
    }
    else if (e.m_Type == ezRenderWorldRenderEvent::Type::EndRender)
    {
      // Perform readback after all pipelines have finished
      PerformReadback();
    }
  }

  void SendPipelineInfo()
  {
    EZ_LOCK(s_Mutex);

    if (!ezTelemetry::IsConnectedToClient())
      return;

    ezTelemetryMessage Response;
    Response.SetMessageID('RNDR', 'PIPE');

    // Write view count
    Response.GetWriter() << static_cast<ezUInt32>(s_PipelineInfo.m_Views.GetCount());

    for (const auto& view : s_PipelineInfo.m_Views)
    {
      Response.GetWriter() << view.m_sViewName;
      Response.GetWriter() << static_cast<ezUInt32>(view.m_Passes.GetCount());

      for (const auto& pass : view.m_Passes)
      {
        Response.GetWriter() << pass.m_sPassName;
        Response.GetWriter() << static_cast<ezUInt32>(pass.m_Outputs.GetCount());

        for (const auto& output : pass.m_Outputs)
        {
          Response.GetWriter() << output.m_sPinName;
          Response.GetWriter() << output.m_bHasTexture;
          Response.GetWriter() << output.m_uiWidth;
          Response.GetWriter() << output.m_uiHeight;
          Response.GetWriter() << output.m_sFormat;
        }
      }
    }

    ezTelemetry::Broadcast(ezTelemetry::Reliable, Response);

    // Clear the flag - we only collect once per request
    s_bCollectPipelineInfo = false;
  }

  void SendError(const char* szError)
  {
    if (!ezTelemetry::IsConnectedToClient())
      return;

    ezTelemetryMessage Response;
    Response.SetMessageID('RNDR', 'TERR');
    Response.GetWriter() << ezString(szError);
    ezTelemetry::Broadcast(ezTelemetry::Reliable, Response);
  }

  void SendTextureData()
  {
    EZ_LOCK(s_Mutex);

    if (!ezTelemetry::IsConnectedToClient())
    {
      s_CapturedTexture.m_bDataReady = false;
      s_CapturedTexture.m_PixelData.Clear();
      return;
    }

    if (!s_CapturedTexture.m_sError.IsEmpty())
    {
      // Send error
      ezTelemetryMessage Response;
      Response.SetMessageID('RNDR', 'TERR');
      Response.GetWriter() << s_CapturedTexture.m_sError;
      ezTelemetry::Broadcast(ezTelemetry::Reliable, Response);
    }
    else
    {
      // Send texture data
      ezTelemetryMessage Response;
      Response.SetMessageID('RNDR', 'TDAT');
      Response.GetWriter() << s_CapturedTexture.m_sViewName;
      Response.GetWriter() << s_CapturedTexture.m_sPassName;
      Response.GetWriter() << s_CapturedTexture.m_sPinName;
      Response.GetWriter() << s_CapturedTexture.m_uiWidth;
      Response.GetWriter() << s_CapturedTexture.m_uiHeight;
      Response.GetWriter() << s_CapturedTexture.m_sFormat;

      // Write pixel data
      const ezUInt32 dataSize = s_CapturedTexture.m_PixelData.GetCount();
      Response.GetWriter() << dataSize;
      if (dataSize > 0)
      {
        Response.GetWriter().WriteBytes(s_CapturedTexture.m_PixelData.GetData(), dataSize).AssertSuccess();
      }

      ezTelemetry::Broadcast(ezTelemetry::Reliable, Response);
    }

    // Clear captured data
    s_CapturedTexture.m_bDataReady = false;
    s_CapturedTexture.m_PixelData.Clear();
  }

} // namespace

static void RendererTelemetryHandler(void* pUserData)
{
  ezTelemetryMessage Msg;

  while (ezTelemetry::RetrieveMessage('RNDR', Msg) == EZ_SUCCESS)
  {
    if (Msg.GetMessageID() == 'LIST')
    {
      // Request to list pipeline info
      EZ_LOCK(s_Mutex);
      s_PipelineInfo.m_Views.Clear();
      s_PipelineInfo.m_bDataReady = false;
      s_bCollectPipelineInfo = true;
      s_CollectPipelineInfoRequestTime = ezTime::Now();
    }
    else if (Msg.GetMessageID() == ' REQ')
    {
      // Request to capture a specific texture
      EZ_LOCK(s_Mutex);

      Msg.GetReader() >> s_CaptureRequest.m_sViewName;
      Msg.GetReader() >> s_CaptureRequest.m_sPassName;
      Msg.GetReader() >> s_CaptureRequest.m_sPinName;

      s_CaptureRequest.m_bPending = true;
      s_CapturedTexture.m_bDataReady = false;
      s_CapturedTexture.m_sError.Clear();
    }
  }

  // Check if we have pipeline info ready to send
  {
    EZ_LOCK(s_Mutex);
    if (s_PipelineInfo.m_bDataReady)
    {
      SendPipelineInfo();
      s_PipelineInfo.m_bDataReady = false;
    }
    else if (s_bCollectPipelineInfo)
    {
      // Check for timeout - if no render events fired within 2 seconds, send empty response
      const ezTime timeout = ezTime::MakeFromSeconds(2.0);
      if (ezTime::Now() - s_CollectPipelineInfoRequestTime > timeout)
      {
        // Send empty pipeline info as a "no data" response
        SendPipelineInfo();
        s_bCollectPipelineInfo = false;
      }
    }
  }

  // Check if we have texture data ready to send
  {
    EZ_LOCK(s_Mutex);
    if (s_CapturedTexture.m_bDataReady)
    {
      SendTextureData();
    }
  }
}

void AddRendererEventHandler()
{
  ezTelemetry::AcceptMessagesForSystem('RNDR', true, RendererTelemetryHandler, nullptr);

  // The render event handler may fail if RendererCore isn't fully initialized
  // but the event system should handle this gracefully
  s_RenderEventSubscription = ezRenderWorld::GetRenderEvent().AddEventHandler(OnRenderEvent);
  s_bRenderEventHandlerAdded = true;
}

void RemoveRendererEventHandler()
{
  if (s_bRenderEventHandlerAdded)
  {
    ezRenderWorld::GetRenderEvent().RemoveEventHandler(s_RenderEventSubscription);
    s_bRenderEventHandlerAdded = false;
  }

  ezTelemetry::AcceptMessagesForSystem('RNDR', false);

  // Clean up
  EZ_LOCK(s_Mutex);
  s_Readback.Reset();
  s_PipelineInfo.m_Views.Clear();
  s_CapturedTexture.m_PixelData.Clear();
  s_bCollectPipelineInfo = false;
  s_bReadbackInProgress = false;
}
