#include <RendererDX12/RendererDX12PCH.h>

#include <Core/System/Window.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/FrameAllocator.h>
#include <Foundation/Platform/Win/Utils/HResultUtils.h>
#include <Foundation/Platform/Win/Utils/IncludeWindows.h>
#include <Foundation/System/SystemInformation.h>
#include <RendererDX12/CommandEncoder/CommandEncoderImplDX12.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Device/InitContext.h>
#include <RendererDX12/Device/SwapChainDX12.h>
#include <RendererDX12/MemoryAllocator/MemoryAllocatorDX12.h>
#include <RendererDX12/Pools/CommandListPoolDX12.h>
#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>
#include <RendererDX12/Pools/FencePoolDX12.h>
#include <RendererDX12/Pools/QueryPoolDX12.h>
#include <RendererDX12/Pools/StagingBufferPoolDX12.h>
#include <RendererDX12/Pools/UniformBufferPoolDX12.h>
#include <RendererDX12/Resources/BufferDX12.h>
#include <RendererDX12/Resources/ReadbackBufferDX12.h>
#include <RendererDX12/Resources/ReadbackTextureDX12.h>
#include <RendererDX12/Resources/RenderTargetViewDX12.h>
#include <RendererDX12/Resources/SharedTextureDX12.h>
#include <RendererDX12/Resources/TextureDX12.h>
#include <RendererDX12/Shader/BindGroupDX12.h>
#include <RendererDX12/Shader/BindGroupLayoutDX12.h>
#include <RendererDX12/Shader/PipelineLayoutDX12.h>
#include <RendererDX12/Shader/ShaderDX12.h>
#include <RendererDX12/Shader/VertexDeclarationDX12.h>
#include <RendererDX12/State/ComputePipelineDX12.h>
#include <RendererDX12/State/GraphicsPipelineDX12.h>
#include <RendererDX12/State/StateDX12.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/CommandEncoder/CommandEncoderPlatformInterface.h>
#include <RendererFoundation/Device/DeviceFactory.h>
#include <RendererFoundation/Profiling/Profiling.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

// ---- Factory registration ----

ezInternal::NewInstance<ezGALDevice> CreateDX12Device(ezAllocator* pAllocator, const ezGALDeviceCreationDescription& description)
{
  return EZ_NEW(pAllocator, ezGALDeviceDX12, description);
}

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RendererDX12, DeviceFactoryDX12)

ON_CORESYSTEMS_STARTUP
{
  ezGALDeviceFactory::RegisterCreatorFunc("DX12", &CreateDX12Device, "DX12", "ezShaderCompilerDX12");
}

ON_CORESYSTEMS_SHUTDOWN
{
  ezGALDeviceFactory::UnregisterCreatorFunc("DX12");
}

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

// ---- Helper: Enumerate a high-performance adapter ----

namespace
{
  IDXGIAdapter1* CreateHighPerformanceAdapter(IDXGIFactory6* pFactory6)
  {
    IDXGIAdapter1* pAdapter = nullptr;
    if (pFactory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)) == DXGI_ERROR_NOT_FOUND)
    {
      // Fallback to first available adapter.
      if (FAILED(pFactory6->EnumAdapters1(0, &pAdapter)))
        return nullptr;
    }
    return pAdapter;
  }
} // namespace

// ---- Constructor / Destructor ----

ezGALDeviceDX12::ezGALDeviceDX12(const ezGALDeviceCreationDescription& Description)
  : ezGALDevice(Description)
{
}

ezGALDeviceDX12::~ezGALDeviceDX12() = default;

// ---- Init & Shutdown ----

ezStringView ezGALDeviceDX12::GetRendererPlatform()
{
  return "DX12";
}

ezResult ezGALDeviceDX12::InitPlatform()
{
  EZ_LOG_BLOCK("ezGALDeviceDX12::InitPlatform");

  // 1. Enable debug layer if requested.
  if (m_Description.m_bDebugDevice)
  {
    ID3D12Debug* pDebugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
    {
      pDebugController->EnableDebugLayer();
      m_pDebug = pDebugController;
      ezLog::Info("D3D12 debug layer enabled.");
    }
    else
    {
      ezLog::Warning("Failed to enable D3D12 debug layer.");
    }
  }

  // 2. Create DXGI factory.
  {
    UINT factoryFlags = 0;
    if (m_Description.m_bDebugDevice)
      factoryFlags = DXGI_CREATE_FACTORY_DEBUG;

    HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_pDXGIFactory));
    if (FAILED(hr))
    {
      ezLog::Error("Failed to create IDXGIFactory6: {}", ezArgErrorCode(hr));
      return EZ_FAILURE;
    }
  }

  // 3. Enumerate adapters and select GPU.
  m_pDXGIAdapter = CreateHighPerformanceAdapter(m_pDXGIFactory);
  if (m_pDXGIAdapter == nullptr)
  {
    ezLog::Error("No suitable DXGI adapter found.");
    return EZ_FAILURE;
  }

  {
    DXGI_ADAPTER_DESC1 adapterDesc = {};
    m_pDXGIAdapter->GetDesc1(&adapterDesc);
    ezLog::Info("Selected GPU adapter: {}", ezStringUtf8(adapterDesc.Description).GetData());
  }

  // 4. Create D3D12 device.
  {
    HRESULT hr = D3D12CreateDevice(m_pDXGIAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice));
    if (FAILED(hr))
    {
      ezLog::Error("Failed to create D3D12 device with feature level 12_0: {}", ezArgErrorCode(hr));
      return EZ_FAILURE;
    }
    ezLog::Success("Created D3D12 device with feature level 12_0.");
  }

  // Set up debug info queue if debug device is active.
  if (m_pDebug && m_pDevice)
  {
    ID3D12InfoQueue* pInfoQueue = nullptr;
    if (SUCCEEDED(m_pDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue))))
    {
      if (ezSystemInformation::IsDebuggerAttached())
      {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
      }

      // Suppress noisy messages that are commonly benign.
      D3D12_MESSAGE_ID denyIds[] = {
        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
        D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
      };

      D3D12_INFO_QUEUE_FILTER filter = {};
      filter.DenyList.NumIDs = EZ_ARRAY_SIZE(denyIds);
      filter.DenyList.pIDList = denyIds;
      pInfoQueue->AddStorageFilterEntries(&filter);
      pInfoQueue->Release();
    }
  }

  // 5. Create graphics command queue.
  {
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    HRESULT hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pGraphicsQueue));
    if (FAILED(hr))
    {
      ezLog::Error("Failed to create D3D12 graphics command queue: {}", ezArgErrorCode(hr));
      return EZ_FAILURE;
    }
  }

  // 6. Initialize D3D12MA memory allocator.
  {
    HRESULT hr = ezMemoryAllocatorDX12::Initialize(m_pDevice, m_pDXGIAdapter);
    if (FAILED(hr))
    {
      ezLog::Error("Failed to initialize D3D12 memory allocator: {}", ezArgErrorCode(hr));
      return EZ_FAILURE;
    }
  }

  // 7. Create all pools.
  m_pFenceQueue = EZ_NEW(&m_Allocator, ezFenceQueueDX12, &m_Allocator);
  m_pFenceQueue->Initialize(m_pDevice);

  m_pCommandListPool = EZ_NEW(&m_Allocator, ezCommandListPoolDX12);
  m_pCommandListPool->Initialize(m_pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);

  m_pDescriptorHeapPool = EZ_NEW(&m_Allocator, ezDescriptorHeapPoolDX12);
  m_pDescriptorHeapPool->Initialize(m_pDevice);

  m_pStagingBufferPool = EZ_NEW(&m_Allocator, ezStagingBufferPoolDX12);
  m_pStagingBufferPool->Initialize(m_pDevice);

  m_pUniformBufferPool = EZ_NEW(&m_Allocator, ezUniformBufferPoolDX12);
  m_pUniformBufferPool->Initialize(m_pDevice);

  m_pQueryPool = EZ_NEW(&m_Allocator, ezQueryPoolDX12);
  m_pQueryPool->Initialize(m_pDevice);

  // Query GPU timestamp frequency for profiling.
  {
    ezUInt64 uiFrequency = 0;
    if (SUCCEEDED(m_pGraphicsQueue->GetTimestampFrequency(&uiFrequency)))
    {
      m_pQueryPool->SetGPUFrequency(uiFrequency);
    }
  }

  // 8. Create init context (for uploading initial resource data).
  m_pInitContext = EZ_NEW(&m_Allocator, ezInitContextDX12);
  m_pInitContext->Initialize(this);

  // 9. Create command encoder.
  m_pCommandEncoderImpl = EZ_DEFAULT_NEW(ezGALCommandEncoderImplDX12, *this);
  m_pCommandEncoder = EZ_DEFAULT_NEW(ezGALCommandEncoder, *this, *m_pCommandEncoderImpl);

  // 10. Create indirect command signatures.
  {
    D3D12_INDIRECT_ARGUMENT_DESC argDesc = {};
    D3D12_COMMAND_SIGNATURE_DESC sigDesc = {};
    sigDesc.NumArgumentDescs = 1;
    sigDesc.pArgumentDescs = &argDesc;

    // Draw indirect
    argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
    sigDesc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
    HRESULT hr = m_pDevice->CreateCommandSignature(&sigDesc, nullptr, IID_PPV_ARGS(&m_pDrawIndirectSignature));
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create draw indirect signature");

    // Draw indexed indirect
    argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
    sigDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
    hr = m_pDevice->CreateCommandSignature(&sigDesc, nullptr, IID_PPV_ARGS(&m_pDrawIndexedIndirectSignature));
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create draw indexed indirect signature");

    // Dispatch indirect
    argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
    sigDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
    hr = m_pDevice->CreateCommandSignature(&sigDesc, nullptr, IID_PPV_ARGS(&m_pDispatchIndirectSignature));
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create dispatch indirect signature");
  }

  // 11. Fill format lookup table.
  FillFormatLookupTable();

  // 12. Set clip space conventions for D3D12.
  ezClipSpaceDepthRange::Default = ezClipSpaceDepthRange::ZeroToOne;
  ezClipSpaceYMode::RenderToTextureDefault = ezClipSpaceYMode::Regular;

  // 13. Register swap chain factory.
  ezGALWindowSwapChain::SetFactoryMethod([this](const ezGALWindowSwapChainCreationDescription& desc) -> ezGALSwapChainHandle
    { return CreateSwapChain([&desc](ezAllocator* pAllocator) -> ezGALSwapChain*
        { return EZ_NEW(pAllocator, ezGALSwapChainDX12, desc); }); });

  return EZ_SUCCESS;
}

void ezGALDeviceDX12::ReportLiveGpuObjects()
{
#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
  const HMODULE hDxgiDebugDLL = LoadLibraryW(L"Dxgidebug.dll");

  if (hDxgiDebugDLL == nullptr)
    return;

  using FnGetDebugInterfacePtr = HRESULT(WINAPI*)(REFIID, void**);
  FnGetDebugInterfacePtr GetDebugInterfacePtr = (FnGetDebugInterfacePtr)GetProcAddress(hDxgiDebugDLL, "DXGIGetDebugInterface");

  if (GetDebugInterfacePtr == nullptr)
    return;

  IDXGIDebug* dxgiDebug = nullptr;
  GetDebugInterfacePtr(IID_PPV_ARGS(&dxgiDebug));

  if (dxgiDebug == nullptr)
    return;

  OutputDebugStringW(L" +++++ Live DX12 Objects: +++++\n");
  dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
  OutputDebugStringW(L" ----- Live DX12 Objects: -----\n");

  dxgiDebug->Release();
#endif
}

void ezGALDeviceDX12::FlushDeadObjects()
{
  DestroyDeadObjects();
}

ezResult ezGALDeviceDX12::ShutdownPlatform()
{
  ezGALWindowSwapChain::SetFactoryMethod({});

  // Wait for the GPU to finish all work before tearing down.
  if (m_pGraphicsQueue && m_pFenceQueue)
  {
    auto hFence = m_pFenceQueue->SubmitCurrentFence(m_pGraphicsQueue);
    m_pFenceQueue->GetFenceResult(hFence, ezTime::MakeFromHours(1));
  }

  m_pInitContext->DeInitialize();
  m_pInitContext = nullptr;

  m_pQueryPool->DeInitialize();
  m_pQueryPool = nullptr;

  m_pUniformBufferPool->DeInitialize();
  m_pUniformBufferPool = nullptr;

  m_pStagingBufferPool->DeInitialize();
  m_pStagingBufferPool = nullptr;

  m_pDescriptorHeapPool->DeInitialize();
  m_pDescriptorHeapPool = nullptr;

  m_pCommandListPool->DeInitialize();
  m_pCommandListPool = nullptr;

  m_pFenceQueue->DeInitialize();
  m_pFenceQueue = nullptr;

  m_pCommandEncoder = nullptr;
  m_pCommandEncoderImpl = nullptr;

  ezMemoryAllocatorDX12::DeInitialize();

  EZ_GAL_DX12_RELEASE(m_pDrawIndirectSignature);
  EZ_GAL_DX12_RELEASE(m_pDrawIndexedIndirectSignature);
  EZ_GAL_DX12_RELEASE(m_pDispatchIndirectSignature);

  EZ_GAL_DX12_RELEASE(m_pGraphicsQueue);
  EZ_GAL_DX12_RELEASE(m_pDevice);
  EZ_GAL_DX12_RELEASE(m_pDebug);
  EZ_GAL_DX12_RELEASE(m_pDXGIAdapter);
  EZ_GAL_DX12_RELEASE(m_pDXGIFactory);

  ReportLiveGpuObjects();

  return EZ_SUCCESS;
}

// ---- Command encoder ----

ezGALCommandEncoder* ezGALDeviceDX12::BeginCommandsPlatform(const char* szName)
{
  // In DX12, a command list must be open before any GPU commands (including profiling timestamps).
  // Unlike DX11's always-available immediate context, DX12 requires explicit command list management.
  if (m_pCommandEncoderImpl->GetCommandList() == nullptr)
  {
    m_pCommandEncoderImpl->BeginCommands(szName);
  }

#if EZ_ENABLED(EZ_USE_PROFILING)
  m_pPassTimingScope = ezProfilingScopeAndMarker::Start(m_pCommandEncoder.Borrow(), szName);
#else
  EZ_IGNORE_UNUSED(szName);
#endif

  return m_pCommandEncoder.Borrow();
}

void ezGALDeviceDX12::EndCommandsPlatform(ezGALCommandEncoder* pPass)
{
  EZ_ASSERT_DEV(m_pCommandEncoder.Borrow() == pPass, "Invalid pass");
  EZ_IGNORE_UNUSED(pPass);

#if EZ_ENABLED(EZ_USE_PROFILING)
  ezProfilingScopeAndMarker::Stop(m_pCommandEncoder.Borrow(), m_pPassTimingScope);
#endif
}

void ezGALDeviceDX12::FlushPlatform()
{
  m_pCommandEncoderImpl->FlushPlatform();
}

// ---- State creation ----

ezGALBlendState* ezGALDeviceDX12::CreateBlendStatePlatform(const ezGALBlendStateCreationDescription& Description)
{
  ezGALBlendStateDX12* pState = EZ_NEW(&m_Allocator, ezGALBlendStateDX12, Description);

  if (pState->InitPlatform(this).Succeeded())
  {
    return pState;
  }

  EZ_DELETE(&m_Allocator, pState);
  return nullptr;
}

void ezGALDeviceDX12::DestroyBlendStatePlatform(ezGALBlendState* pBlendState)
{
  ezGALBlendStateDX12* pState = static_cast<ezGALBlendStateDX12*>(pBlendState);
  pState->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pState);
}

ezGALDepthStencilState* ezGALDeviceDX12::CreateDepthStencilStatePlatform(const ezGALDepthStencilStateCreationDescription& Description)
{
  ezGALDepthStencilStateDX12* pState = EZ_NEW(&m_Allocator, ezGALDepthStencilStateDX12, Description);

  if (pState->InitPlatform(this).Succeeded())
  {
    return pState;
  }

  EZ_DELETE(&m_Allocator, pState);
  return nullptr;
}

void ezGALDeviceDX12::DestroyDepthStencilStatePlatform(ezGALDepthStencilState* pDepthStencilState)
{
  ezGALDepthStencilStateDX12* pState = static_cast<ezGALDepthStencilStateDX12*>(pDepthStencilState);
  pState->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pState);
}

ezGALRasterizerState* ezGALDeviceDX12::CreateRasterizerStatePlatform(const ezGALRasterizerStateCreationDescription& Description)
{
  ezGALRasterizerStateDX12* pState = EZ_NEW(&m_Allocator, ezGALRasterizerStateDX12, Description);

  if (pState->InitPlatform(this).Succeeded())
  {
    return pState;
  }

  EZ_DELETE(&m_Allocator, pState);
  return nullptr;
}

void ezGALDeviceDX12::DestroyRasterizerStatePlatform(ezGALRasterizerState* pRasterizerState)
{
  ezGALRasterizerStateDX12* pState = static_cast<ezGALRasterizerStateDX12*>(pRasterizerState);
  pState->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pState);
}

ezGALSamplerState* ezGALDeviceDX12::CreateSamplerStatePlatform(const ezGALSamplerStateCreationDescription& Description)
{
  ezGALSamplerStateDX12* pState = EZ_NEW(&m_Allocator, ezGALSamplerStateDX12, Description);

  if (pState->InitPlatform(this).Succeeded())
  {
    return pState;
  }

  EZ_DELETE(&m_Allocator, pState);
  return nullptr;
}

void ezGALDeviceDX12::DestroySamplerStatePlatform(ezGALSamplerState* pSamplerState)
{
  ezGALSamplerStateDX12* pState = static_cast<ezGALSamplerStateDX12*>(pSamplerState);
  pState->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pState);
}

ezGALBindGroupLayout* ezGALDeviceDX12::CreateBindGroupLayoutPlatform(const ezGALBindGroupLayoutCreationDescription& Description)
{
  ezGALBindGroupLayoutDX12* pLayout = EZ_NEW(&m_Allocator, ezGALBindGroupLayoutDX12, Description);

  if (pLayout->InitPlatform(this).Succeeded())
  {
    return pLayout;
  }

  EZ_DELETE(&m_Allocator, pLayout);
  return nullptr;
}

void ezGALDeviceDX12::DestroyBindGroupLayoutPlatform(ezGALBindGroupLayout* pBindGroupLayout)
{
  ezGALBindGroupLayoutDX12* pLayout = static_cast<ezGALBindGroupLayoutDX12*>(pBindGroupLayout);
  pLayout->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pLayout);
}

ezGALBindGroup* ezGALDeviceDX12::CreateBindGroupPlatform(const ezGALBindGroupCreationDescription& Description)
{
  ezGALBindGroupDX12* pBindGroup = EZ_NEW(&m_Allocator, ezGALBindGroupDX12, Description);

  if (pBindGroup->InitPlatform(this).Succeeded())
  {
    return pBindGroup;
  }

  EZ_DELETE(&m_Allocator, pBindGroup);
  return nullptr;
}

void ezGALDeviceDX12::DestroyBindGroupPlatform(ezGALBindGroup* pBindGroup)
{
  ezGALBindGroupDX12* pDX12BindGroup = static_cast<ezGALBindGroupDX12*>(pBindGroup);
  pDX12BindGroup->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12BindGroup);
}

ezGALPipelineLayout* ezGALDeviceDX12::CreatePipelineLayoutPlatform(const ezGALPipelineLayoutCreationDescription& Description)
{
  ezGALPipelineLayoutDX12* pLayout = EZ_NEW(&m_Allocator, ezGALPipelineLayoutDX12, Description);

  if (pLayout->InitPlatform(this).Succeeded())
  {
    return pLayout;
  }

  EZ_DELETE(&m_Allocator, pLayout);
  return nullptr;
}

void ezGALDeviceDX12::DestroyPipelineLayoutPlatform(ezGALPipelineLayout* pPipelineLayout)
{
  ezGALPipelineLayoutDX12* pLayout = static_cast<ezGALPipelineLayoutDX12*>(pPipelineLayout);
  pLayout->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pLayout);
}

ezGALGraphicsPipeline* ezGALDeviceDX12::CreateGraphicsPipelinePlatform(const ezGALGraphicsPipelineCreationDescription& Description)
{
  ezGALGraphicsPipelineDX12* pPipeline = EZ_NEW(&m_Allocator, ezGALGraphicsPipelineDX12, Description);

  if (pPipeline->InitPlatform(this).Succeeded())
  {
    return pPipeline;
  }

  EZ_DELETE(&m_Allocator, pPipeline);
  return nullptr;
}

void ezGALDeviceDX12::DestroyGraphicsPipelinePlatform(ezGALGraphicsPipeline* pGraphicsPipeline)
{
  ezGALGraphicsPipelineDX12* pPipeline = static_cast<ezGALGraphicsPipelineDX12*>(pGraphicsPipeline);
  pPipeline->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pPipeline);
}

ezGALComputePipeline* ezGALDeviceDX12::CreateComputePipelinePlatform(const ezGALComputePipelineCreationDescription& Description)
{
  ezGALComputePipelineDX12* pPipeline = EZ_NEW(&m_Allocator, ezGALComputePipelineDX12, Description);

  if (pPipeline->InitPlatform(this).Succeeded())
  {
    return pPipeline;
  }

  EZ_DELETE(&m_Allocator, pPipeline);
  return nullptr;
}

void ezGALDeviceDX12::DestroyComputePipelinePlatform(ezGALComputePipeline* pComputePipeline)
{
  ezGALComputePipelineDX12* pPipeline = static_cast<ezGALComputePipelineDX12*>(pComputePipeline);
  pPipeline->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pPipeline);
}

// ---- Resource creation ----

ezGALShader* ezGALDeviceDX12::CreateShaderPlatform(const ezGALShaderCreationDescription& Description)
{
  ezGALShaderDX12* pShader = EZ_NEW(&m_Allocator, ezGALShaderDX12, Description);

  if (!pShader->InitPlatform(this).Succeeded())
  {
    EZ_DELETE(&m_Allocator, pShader);
    return nullptr;
  }

  return pShader;
}

void ezGALDeviceDX12::DestroyShaderPlatform(ezGALShader* pShader)
{
  ezGALShaderDX12* pDX12Shader = static_cast<ezGALShaderDX12*>(pShader);
  pDX12Shader->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12Shader);
}

ezGALBuffer* ezGALDeviceDX12::CreateBufferPlatform(const ezGALBufferCreationDescription& Description, ezArrayPtr<const ezUInt8> pInitialData)
{
  ezGALBufferDX12* pBuffer = EZ_NEW(&m_Allocator, ezGALBufferDX12, Description);

  if (!pBuffer->InitPlatform(this, pInitialData).Succeeded())
  {
    EZ_DELETE(&m_Allocator, pBuffer);
    return nullptr;
  }

  return pBuffer;
}

void ezGALDeviceDX12::DestroyBufferPlatform(ezGALBuffer* pBuffer)
{
  ezGALBufferDX12* pDX12Buffer = static_cast<ezGALBufferDX12*>(pBuffer);
  pDX12Buffer->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12Buffer);
}

ezGALTexture* ezGALDeviceDX12::CreateTexturePlatform(const ezGALTextureCreationDescription& Description, ezArrayPtr<ezGALSystemMemoryDescription> pInitialData)
{
  ezGALTextureDX12* pTexture = EZ_NEW(&m_Allocator, ezGALTextureDX12, Description);

  if (!pTexture->InitPlatform(this, pInitialData).Succeeded())
  {
    EZ_DELETE(&m_Allocator, pTexture);
    return nullptr;
  }

  return pTexture;
}

void ezGALDeviceDX12::DestroyTexturePlatform(ezGALTexture* pTexture)
{
  ezGALTextureDX12* pDX12Texture = static_cast<ezGALTextureDX12*>(pTexture);
  pDX12Texture->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12Texture);
}

ezGALTexture* ezGALDeviceDX12::CreateSharedTexturePlatform(const ezGALTextureCreationDescription& Description, ezArrayPtr<ezGALSystemMemoryDescription> pInitialData, ezEnum<ezGALSharedTextureType> sharedType, ezGALPlatformSharedHandle handle)
{
  ezGALSharedTextureDX12* pTexture = EZ_NEW(&m_Allocator, ezGALSharedTextureDX12, Description, sharedType, handle);

  if (!pTexture->InitPlatform(this, pInitialData).Succeeded())
  {
    EZ_DELETE(&m_Allocator, pTexture);
    return nullptr;
  }

  return pTexture;
}

void ezGALDeviceDX12::DestroySharedTexturePlatform(ezGALTexture* pTexture)
{
  ezGALSharedTextureDX12* pDX12Texture = static_cast<ezGALSharedTextureDX12*>(pTexture);
  pDX12Texture->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12Texture);
}

ezGALReadbackBuffer* ezGALDeviceDX12::CreateReadbackBufferPlatform(const ezGALBufferCreationDescription& Description)
{
  ezGALReadbackBufferDX12* pReadbackBuffer = EZ_NEW(&m_Allocator, ezGALReadbackBufferDX12, Description);

  if (!pReadbackBuffer->InitPlatform(this).Succeeded())
  {
    EZ_DELETE(&m_Allocator, pReadbackBuffer);
    return nullptr;
  }

  return pReadbackBuffer;
}

void ezGALDeviceDX12::DestroyReadbackBufferPlatform(ezGALReadbackBuffer* pReadbackBuffer)
{
  ezGALReadbackBufferDX12* pDX12ReadbackBuffer = static_cast<ezGALReadbackBufferDX12*>(pReadbackBuffer);
  pDX12ReadbackBuffer->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12ReadbackBuffer);
}

ezGALReadbackTexture* ezGALDeviceDX12::CreateReadbackTexturePlatform(const ezGALTextureCreationDescription& Description)
{
  ezGALReadbackTextureDX12* pReadbackTexture = EZ_NEW(&m_Allocator, ezGALReadbackTextureDX12, Description);

  if (!pReadbackTexture->InitPlatform(this).Succeeded())
  {
    EZ_DELETE(&m_Allocator, pReadbackTexture);
    return nullptr;
  }

  return pReadbackTexture;
}

void ezGALDeviceDX12::DestroyReadbackTexturePlatform(ezGALReadbackTexture* pReadbackTexture)
{
  ezGALReadbackTextureDX12* pDX12ReadbackTexture = static_cast<ezGALReadbackTextureDX12*>(pReadbackTexture);
  pDX12ReadbackTexture->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12ReadbackTexture);
}

ezGALRenderTargetView* ezGALDeviceDX12::CreateRenderTargetViewPlatform(ezGALTexture* pTexture, const ezGALRenderTargetViewCreationDescription& Description)
{
  ezGALRenderTargetViewDX12* pRTView = EZ_NEW(&m_Allocator, ezGALRenderTargetViewDX12, pTexture, Description);

  if (!pRTView->InitPlatform(this).Succeeded())
  {
    EZ_DELETE(&m_Allocator, pRTView);
    return nullptr;
  }

  return pRTView;
}

void ezGALDeviceDX12::DestroyRenderTargetViewPlatform(ezGALRenderTargetView* pRenderTargetView)
{
  ezGALRenderTargetViewDX12* pDX12RenderTargetView = static_cast<ezGALRenderTargetViewDX12*>(pRenderTargetView);
  pDX12RenderTargetView->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12RenderTargetView);
}

ezGALVertexDeclaration* ezGALDeviceDX12::CreateVertexDeclarationPlatform(const ezGALVertexDeclarationCreationDescription& Description)
{
  ezGALVertexDeclarationDX12* pVertexDeclaration = EZ_NEW(&m_Allocator, ezGALVertexDeclarationDX12, Description);

  if (pVertexDeclaration->InitPlatform(this).Succeeded())
  {
    return pVertexDeclaration;
  }

  EZ_DELETE(&m_Allocator, pVertexDeclaration);
  return nullptr;
}

void ezGALDeviceDX12::DestroyVertexDeclarationPlatform(ezGALVertexDeclaration* pVertexDeclaration)
{
  ezGALVertexDeclarationDX12* pDX12VertexDeclaration = static_cast<ezGALVertexDeclarationDX12*>(pVertexDeclaration);
  pDX12VertexDeclaration->DeInitPlatform(this).IgnoreResult();
  EZ_DELETE(&m_Allocator, pDX12VertexDeclaration);
}

// ---- Resource updates ----

void ezGALDeviceDX12::UpdateBufferForNextFramePlatform(const ezGALBuffer* pBuffer, ezConstByteArrayPtr sourceData, ezUInt32 uiDestOffset)
{
  auto& update = m_PendingBufferUpdates.ExpandAndGetRef();
  update.m_pBuffer = pBuffer;
  update.m_uiDestOffset = uiDestOffset;
  update.m_Data.SetCountUninitialized(static_cast<ezUInt32>(sourceData.GetCount()));
  ezMemoryUtils::Copy(update.m_Data.GetData(), sourceData.GetPtr(), static_cast<ezUInt32>(sourceData.GetCount()));
}

void ezGALDeviceDX12::UpdateTextureForNextFramePlatform(const ezGALTexture* pTexture, const ezGALSystemMemoryDescription& sourceData, const ezGALTextureSubresource& destinationSubResource, const ezBoundingBoxu32& destinationBox)
{
  auto& update = m_PendingTextureUpdates.ExpandAndGetRef();
  update.m_pTexture = pTexture;
  update.m_DestSubResource = destinationSubResource;
  update.m_DestBox = destinationBox;

  // Copy the source data so it survives until next frame.
  const ezUInt32 uiDataSize = static_cast<ezUInt32>(sourceData.m_pData.GetCount());
  update.m_DataCopy.SetCountUninitialized(uiDataSize);
  ezMemoryUtils::Copy(update.m_DataCopy.GetData(), sourceData.m_pData.GetPtr(), uiDataSize);

  update.m_SourceData.m_pData = ezConstByteBlobPtr(update.m_DataCopy.GetData(), update.m_DataCopy.GetCount());
  update.m_SourceData.m_uiRowPitch = sourceData.m_uiRowPitch;
  update.m_SourceData.m_uiSlicePitch = sourceData.m_uiSlicePitch;
}

// ---- GPU -> CPU queries ----

ezEnum<ezGALAsyncResult> ezGALDeviceDX12::GetTimestampResultPlatform(ezGALTimestampHandle hTimestamp, ezTime& out_result)
{
  ezUInt64 uiTimestampTicks = 0;
  if (m_pQueryPool->GetTimestampResult(hTimestamp.m_Data, uiTimestampTicks))
  {
    const ezUInt64 uiFrequency = m_pQueryPool->GetGPUFrequency();
    out_result = ezTime::MakeFromSeconds(static_cast<double>(uiTimestampTicks) / static_cast<double>(uiFrequency));
    return ezGALAsyncResult::Ready;
  }

  return ezGALAsyncResult::Pending;
}

ezEnum<ezGALAsyncResult> ezGALDeviceDX12::GetOcclusionResultPlatform(ezGALOcclusionHandle hOcclusion, ezUInt64& out_uiResult)
{
  if (m_pQueryPool->GetOcclusionResult(hOcclusion.m_Data, out_uiResult))
  {
    return ezGALAsyncResult::Ready;
  }

  return ezGALAsyncResult::Pending;
}

ezEnum<ezGALAsyncResult> ezGALDeviceDX12::GetFenceResultPlatform(ezGALFenceHandle hFence, ezTime timeout)
{
  return m_pFenceQueue->GetFenceResult(hFence, timeout);
}

ezResult ezGALDeviceDX12::LockBufferPlatform(const ezGALReadbackBuffer* pBuffer, ezArrayPtr<const ezUInt8>& out_Memory) const
{
  const ezGALReadbackBufferDX12* pDX12Buffer = static_cast<const ezGALReadbackBufferDX12*>(pBuffer);
  ID3D12Resource* pResource = pDX12Buffer->GetDXResource();

  void* pMappedData = nullptr;
  D3D12_RANGE readRange = {0, pBuffer->GetDescription().m_uiTotalSize};
  HRESULT hr = pResource->Map(0, &readRange, &pMappedData);
  if (FAILED(hr))
  {
    return EZ_FAILURE;
  }

  out_Memory = ezArrayPtr<const ezUInt8>(reinterpret_cast<const ezUInt8*>(pMappedData), pBuffer->GetDescription().m_uiTotalSize);
  return EZ_SUCCESS;
}

void ezGALDeviceDX12::UnlockBufferPlatform(const ezGALReadbackBuffer* pBuffer) const
{
  const ezGALReadbackBufferDX12* pDX12Buffer = static_cast<const ezGALReadbackBufferDX12*>(pBuffer);
  D3D12_RANGE writtenRange = {0, 0}; // Nothing written by the CPU.
  pDX12Buffer->GetDXResource()->Unmap(0, &writtenRange);
}

ezResult ezGALDeviceDX12::LockTexturePlatform(const ezGALReadbackTexture* pTexture, const ezArrayPtr<const ezGALTextureSubresource>& subResources, ezDynamicArray<ezGALSystemMemoryDescription>& out_Memory) const
{
  out_Memory.Clear();
  const ezGALReadbackTextureDX12* pDX12Texture = static_cast<const ezGALReadbackTextureDX12*>(pTexture);
  ID3D12Resource* pResource = pDX12Texture->GetDXResource();

  const ezUInt32 uiSubResources = subResources.GetCount();
  for (ezUInt32 i = 0; i < uiSubResources; i++)
  {
    const ezGALTextureSubresource& subRes = subResources[i];
    const ezUInt32 uiSubResourceIndex = subRes.m_uiMipLevel + subRes.m_uiArraySlice * pTexture->GetDescription().m_uiMipLevelCount;

    void* pMappedData = nullptr;
    D3D12_RANGE readRange = {}; // The entire subresource.
    HRESULT hr = pResource->Map(uiSubResourceIndex, &readRange, &pMappedData);
    if (FAILED(hr))
    {
      ezLog::Error("Failed to map readback texture sub resource (mip {}, slice {}): {}", subRes.m_uiMipLevel, subRes.m_uiArraySlice, ezArgErrorCode(hr));
      // Still add an entry to keep array indices consistent.
      out_Memory.ExpandAndGetRef() = {};
      continue;
    }

    const auto& desc = pTexture->GetDescription();
    ezGALSystemMemoryDescription& memDesc = out_Memory.ExpandAndGetRef();

    switch (desc.m_Type)
    {
      case ezGALTextureType::Texture2D:
      case ezGALTextureType::Texture2DProxy:
      case ezGALTextureType::Texture2DShared:
      case ezGALTextureType::TextureCube:
      {
        const ezUInt64 uiRowPitch = pDX12Texture->GetRowPitch();
        memDesc.m_pData = ezMakeByteBlobPtr(pMappedData, uiRowPitch * desc.m_uiHeight);
        memDesc.m_uiRowPitch = static_cast<ezUInt32>(uiRowPitch);
        memDesc.m_uiSlicePitch = 0;
        break;
      }
      case ezGALTextureType::Texture3D:
      {
        const ezUInt64 uiRowPitch = pDX12Texture->GetRowPitch();
        const ezUInt64 uiSlicePitch = uiRowPitch * desc.m_uiHeight;
        memDesc.m_pData = ezMakeByteBlobPtr(pMappedData, uiSlicePitch * desc.m_uiDepth);
        memDesc.m_uiRowPitch = static_cast<ezUInt32>(uiRowPitch);
        memDesc.m_uiSlicePitch = static_cast<ezUInt32>(uiSlicePitch);
        break;
      }
      default:
        break;
    }
  }

  return EZ_SUCCESS;
}

void ezGALDeviceDX12::UnlockTexturePlatform(const ezGALReadbackTexture* pTexture, const ezArrayPtr<const ezGALTextureSubresource>& subResources) const
{
  const ezGALReadbackTextureDX12* pDX12Texture = static_cast<const ezGALReadbackTextureDX12*>(pTexture);
  ID3D12Resource* pResource = pDX12Texture->GetDXResource();

  const ezUInt32 uiSubResources = subResources.GetCount();
  for (ezUInt32 i = 0; i < uiSubResources; i++)
  {
    const ezGALTextureSubresource& subRes = subResources[i];
    const ezUInt32 uiSubResourceIndex = subRes.m_uiMipLevel + subRes.m_uiArraySlice * pTexture->GetDescription().m_uiMipLevelCount;

    D3D12_RANGE writtenRange = {0, 0};
    pResource->Unmap(uiSubResourceIndex, &writtenRange);
  }
}

// ---- Frame lifecycle ----

void ezGALDeviceDX12::BeginFramePlatform(ezArrayPtr<ezGALSwapChain*> swapchains, const ezUInt64 uiAppFrame)
{
  // Check if previous frame fences have been reached and reclaim resources.
  for (ezUInt64 uiFrame = m_uiSafeFrame + 1; uiFrame < m_uiFrameCounter; uiFrame++)
  {
    auto& perFrameData = m_PerFrameData[uiFrame % FRAMES];

    // If we accumulate more frames than we can hold in the ring buffer, force waiting.
    const bool bForce = uiFrame % FRAMES == m_uiFrameCounter % FRAMES;
    if (perFrameData.m_uiFrame != ((ezUInt64)-1))
    {
      EZ_ASSERT_DEBUG(uiFrame == perFrameData.m_uiFrame, "Frame data mismatch.");
      bool bFenceReached = m_pFenceQueue->GetFenceResult(perFrameData.m_hFence) == ezGALAsyncResult::Ready;
      if (!bFenceReached && bForce)
      {
        m_pFenceQueue->GetFenceResult(perFrameData.m_hFence, ezTime::MakeFromHours(1));
        bFenceReached = true;
      }
      if (bFenceReached)
      {
        m_pStagingBufferPool->Reclaim(perFrameData.m_uiFrame);
        perFrameData.m_hFence = {};
        m_uiSafeFrame = uiFrame;
      }
      else
        break;
    }
  }

  EZ_ASSERT_DEBUG((m_uiFrameCounter % FRAMES) == m_uiCurrentPerFrameData, "");
  m_PerFrameData[m_uiCurrentPerFrameData].m_uiFrame = m_uiFrameCounter;

  // Reset per-frame allocators and pools.
  m_pDescriptorHeapPool->BeginFrame();
  m_pUniformBufferPool->BeginFrame();
  m_pQueryPool->BeginFrame();

  // Submit any pending init context commands (initial resource uploads).
  m_pInitContext->SubmitPendingCommands();

  // Acquire the frame's main command list before the profiling scope uses it.
  m_pCommandEncoderImpl->BeginCommands("Frame");

  // Process pending resource updates that were deferred to this frame.
  {
    for (auto& update : m_PendingBufferUpdates)
    {
      m_pCommandEncoderImpl->UpdateBufferPlatform(update.m_pBuffer, update.m_uiDestOffset, update.m_Data, ezGALUpdateMode::AheadOfTime);
    }
    m_PendingBufferUpdates.Clear();

    for (auto& update : m_PendingTextureUpdates)
    {
      m_pCommandEncoderImpl->UpdateTexturePlatform(update.m_pTexture, update.m_DestSubResource, update.m_DestBox, update.m_SourceData);
    }
    m_PendingTextureUpdates.Clear();
  }

#if EZ_ENABLED(EZ_USE_PROFILING)
  ezStringBuilder sb;
  sb.SetFormat("RENDER FRAME {}", uiAppFrame);
  m_pFrameTimingScope = ezProfilingScopeAndMarker::Start(m_pCommandEncoder.Borrow(), sb);
#else
  EZ_IGNORE_UNUSED(uiAppFrame);
#endif

  for (ezGALSwapChain* pSwapChain : swapchains)
  {
    pSwapChain->AcquireNextRenderTarget(this);
  }
}

void ezGALDeviceDX12::EndFramePlatform(ezArrayPtr<ezGALSwapChain*> swapchains)
{
#if EZ_ENABLED(EZ_USE_PROFILING)
  ezProfilingScopeAndMarker::Stop(m_pCommandEncoder.Borrow(), m_pFrameTimingScope);
#endif

  // Transition swap chain back buffers to PRESENT state before closing the command list.
  for (ezGALSwapChain* pSwapChain : swapchains)
  {
    ezGALSwapChainDX12* pDXSwapChain = static_cast<ezGALSwapChainDX12*>(pSwapChain);
    const ezGALTexture* pTex = GetTexture(pDXSwapChain->m_RenderTargets.m_hRTs[0]);
    if (pTex != nullptr)
    {
      ezGALTextureDX12* pBackBuffer = const_cast<ezGALTextureDX12*>(static_cast<const ezGALTextureDX12*>(pTex));
      m_pCommandEncoderImpl->GetBarrierTracker().TextureBarrier(
        pBackBuffer->GetDXResource(), pBackBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_PRESENT);
      pBackBuffer->SetCurrentState(D3D12_RESOURCE_STATE_PRESENT);
    }
  }

  m_pCommandEncoderImpl->EndFrame();

  // Close and execute the frame's command list.
  m_pCommandEncoderImpl->EndCommands();

  ID3D12GraphicsCommandList* pClosedList = m_pCommandEncoderImpl->GetClosedCommandList();
  if (pClosedList)
  {
    ID3D12CommandList* ppCommandLists[] = {pClosedList};
    m_pGraphicsQueue->ExecuteCommandLists(1, ppCommandLists);
  }

  // Present AFTER command list execution so the GPU has transitioned the back buffers.
  for (ezGALSwapChain* pSwapChain : swapchains)
  {
    pSwapChain->PresentRenderTarget(this);
  }

  m_pStagingBufferPool->FinishFrame(m_uiFrameCounter);

  auto& currentFrameData = m_PerFrameData[m_uiCurrentPerFrameData];
  currentFrameData.m_hFence = m_pFenceQueue->SubmitCurrentFence(m_pGraphicsQueue);

  ++m_uiFrameCounter;
  m_uiCurrentPerFrameData = m_uiFrameCounter % FRAMES;
}

ezUInt64 ezGALDeviceDX12::GetCurrentFramePlatform() const
{
  return m_uiFrameCounter;
}

ezUInt64 ezGALDeviceDX12::GetSafeFramePlatform() const
{
  return m_uiSafeFrame;
}

// ---- Capabilities ----

void ezGALDeviceDX12::FillCapabilitiesPlatform()
{
  {
    DXGI_ADAPTER_DESC1 adapterDesc;
    m_pDXGIAdapter->GetDesc1(&adapterDesc);

    m_Capabilities.m_sAdapterName = ezStringUtf8(adapterDesc.Description).GetData();
    m_Capabilities.m_uiDedicatedVRAM = static_cast<ezUInt64>(adapterDesc.DedicatedVideoMemory);
    m_Capabilities.m_uiDedicatedSystemRAM = static_cast<ezUInt64>(adapterDesc.DedicatedSystemMemory);
    m_Capabilities.m_uiSharedSystemRAM = static_cast<ezUInt64>(adapterDesc.SharedSystemMemory);
    m_Capabilities.m_bHardwareAccelerated = (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0;
  }

  m_Capabilities.m_bSupportsMultithreadedResourceCreation = true;
  m_Capabilities.m_bSupportsTexelBuffer = true;
  m_Capabilities.m_bSupportsMultipleSRVTypes = true;
  m_Capabilities.m_bSupportsMultiSampledArrays = true;

  // D3D12 with feature level 12.0 guarantees all shader stages.
  m_Capabilities.m_bShaderStageSupported[ezGALShaderStage::VertexShader] = true;
  m_Capabilities.m_bShaderStageSupported[ezGALShaderStage::HullShader] = true;
  m_Capabilities.m_bShaderStageSupported[ezGALShaderStage::DomainShader] = true;
  m_Capabilities.m_bShaderStageSupported[ezGALShaderStage::GeometryShader] = true;
  m_Capabilities.m_bShaderStageSupported[ezGALShaderStage::PixelShader] = true;
  m_Capabilities.m_bShaderStageSupported[ezGALShaderStage::ComputeShader] = true;
  m_Capabilities.m_bSupportsIndirectDraw = true;
  m_Capabilities.m_bSupportsSharedTextures = true;
  m_Capabilities.m_bSupportsWireframe = true;

  // Check for conservative rasterization support.
  {
    D3D12_FEATURE_DATA_D3D12_OPTIONS featureOptions = {};
    if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureOptions, sizeof(featureOptions))))
    {
      m_Capabilities.m_bSupportsConservativeRasterization =
        (featureOptions.ConservativeRasterizationTier != D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED);
    }
  }

  // Check for VP/RT array index from any shader.
  // In D3D12, this is in D3D12_FEATURE_DATA_D3D12_OPTIONS (not OPTIONS3 like in D3D11).
  {
    D3D12_FEATURE_DATA_D3D12_OPTIONS featureOptions1 = {};
    if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureOptions1, sizeof(featureOptions1))))
    {
      m_Capabilities.m_bSupportsVSRenderTargetArrayIndex =
        featureOptions1.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation != 0;
    }
  }

  // Fill per-format support flags.
  m_Capabilities.m_FormatSupport.SetCount(ezGALResourceFormat::ENUM_COUNT);
  for (ezUInt32 i = 0; i < ezGALResourceFormat::ENUM_COUNT; i++)
  {
    ezGALResourceFormat::Enum format = (ezGALResourceFormat::Enum)i;
    const ezGALFormatLookupEntryDX12& entry = m_FormatLookupTable.GetFormatInfo(format);
    const bool bIsDepth = ezGALResourceFormat::IsDepthFormat(format);

    if (bIsDepth)
    {
      D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {entry.m_eDepthOnlyType};
      if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
      {
        if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)
          m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::Texture);
      }

      D3D12_FEATURE_DATA_FORMAT_SUPPORT dsFormatSupport = {entry.m_eDepthStencilType};
      if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &dsFormatSupport, sizeof(dsFormatSupport))))
      {
        if (dsFormatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
          m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::RenderTarget);
      }
    }
    else
    {
      D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {entry.m_eResourceViewType};
      if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
      {
        UINT uiSampleFlag = ezGALResourceFormat::IsIntegerFormat(format) ? D3D12_FORMAT_SUPPORT1_SHADER_LOAD : D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE;
        if (formatSupport.Support1 & uiSampleFlag)
          m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::Texture);

        if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW)
          m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::TextureRW);
      }

      D3D12_FEATURE_DATA_FORMAT_SUPPORT vertexFormatSupport = {entry.m_eVertexAttributeType};
      if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &vertexFormatSupport, sizeof(vertexFormatSupport))))
      {
        if (vertexFormatSupport.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
          m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::VertexAttribute);
      }

      D3D12_FEATURE_DATA_FORMAT_SUPPORT rtFormatSupport = {entry.m_eRenderTarget};
      if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &rtFormatSupport, sizeof(rtFormatSupport))))
      {
        if (rtFormatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
          m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::RenderTarget);
      }

      // Check MSAA support.
      DXGI_FORMAT rtFormat = entry.m_eRenderTarget;
      {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaCheck = {};
        msaaCheck.Format = rtFormat;

        msaaCheck.SampleCount = 2;
        if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaCheck, sizeof(msaaCheck))))
        {
          if (msaaCheck.NumQualityLevels > 0)
            m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::MSAA2x);
        }

        msaaCheck.SampleCount = 4;
        if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaCheck, sizeof(msaaCheck))))
        {
          if (msaaCheck.NumQualityLevels > 0)
            m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::MSAA4x);
        }

        msaaCheck.SampleCount = 8;
        if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaCheck, sizeof(msaaCheck))))
        {
          if (msaaCheck.NumQualityLevels > 0)
            m_Capabilities.m_FormatSupport[i].Add(ezGALResourceFormatSupport::MSAA8x);
        }
      }
    }
  }
}

void ezGALDeviceDX12::WaitIdlePlatform()
{
  if (m_pGraphicsQueue && m_pFenceQueue)
  {
    auto hFence = m_pFenceQueue->SubmitCurrentFence(m_pGraphicsQueue);
    m_pFenceQueue->GetFenceResult(hFence, ezTime::MakeFromHours(1));
  }
  DestroyDeadObjects();
}

const ezGALSharedTexture* ezGALDeviceDX12::GetSharedTexture(ezGALTextureHandle hTexture) const
{
  auto pTexture = GetTexture(hTexture);
  if (pTexture == nullptr)
  {
    return nullptr;
  }

  // Resolve proxy texture if any.
  return static_cast<const ezGALSharedTextureDX12*>(pTexture->GetParentResource());
}

// ---- Format lookup table ----

void ezGALDeviceDX12::FillFormatLookupTable()
{
  // The format table is identical to DX11 since both APIs use DXGI formats.

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAFloat, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32B32A32_TYPELESS).RT(DXGI_FORMAT_R32G32B32A32_FLOAT).VA(DXGI_FORMAT_R32G32B32A32_FLOAT).RV(DXGI_FORMAT_R32G32B32A32_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAUInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32B32A32_TYPELESS).RT(DXGI_FORMAT_R32G32B32A32_UINT).VA(DXGI_FORMAT_R32G32B32A32_UINT).RV(DXGI_FORMAT_R32G32B32A32_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32B32A32_TYPELESS).RT(DXGI_FORMAT_R32G32B32A32_SINT).VA(DXGI_FORMAT_R32G32B32A32_SINT).RV(DXGI_FORMAT_R32G32B32A32_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBFloat, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32B32_TYPELESS).RT(DXGI_FORMAT_R32G32B32_FLOAT).VA(DXGI_FORMAT_R32G32B32_FLOAT).RV(DXGI_FORMAT_R32G32B32_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBUInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32B32_TYPELESS).RT(DXGI_FORMAT_R32G32B32_UINT).VA(DXGI_FORMAT_R32G32B32_UINT).RV(DXGI_FORMAT_R32G32B32_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32B32_TYPELESS).RT(DXGI_FORMAT_R32G32B32_SINT).VA(DXGI_FORMAT_R32G32B32_SINT).RV(DXGI_FORMAT_R32G32B32_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::B5G6R5UNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_B5G6R5_UNORM).RT(DXGI_FORMAT_B5G6R5_UNORM).VA(DXGI_FORMAT_B5G6R5_UNORM).RV(DXGI_FORMAT_B5G6R5_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BGRAUByteNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_B8G8R8A8_TYPELESS).RT(DXGI_FORMAT_B8G8R8A8_UNORM).VA(DXGI_FORMAT_B8G8R8A8_UNORM).RV(DXGI_FORMAT_B8G8R8A8_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BGRAUByteNormalizedsRGB, ezGALFormatLookupEntryDX12(DXGI_FORMAT_B8G8R8A8_TYPELESS).RT(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB).RV(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAHalf, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16B16A16_TYPELESS).RT(DXGI_FORMAT_R16G16B16A16_FLOAT).VA(DXGI_FORMAT_R16G16B16A16_FLOAT).RV(DXGI_FORMAT_R16G16B16A16_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAUShort, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16B16A16_TYPELESS).RT(DXGI_FORMAT_R16G16B16A16_UINT).VA(DXGI_FORMAT_R16G16B16A16_UINT).RV(DXGI_FORMAT_R16G16B16A16_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAUShortNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16B16A16_TYPELESS).RT(DXGI_FORMAT_R16G16B16A16_UNORM).VA(DXGI_FORMAT_R16G16B16A16_UNORM).RV(DXGI_FORMAT_R16G16B16A16_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAShort, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16B16A16_TYPELESS).RT(DXGI_FORMAT_R16G16B16A16_SINT).VA(DXGI_FORMAT_R16G16B16A16_SINT).RV(DXGI_FORMAT_R16G16B16A16_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAShortNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16B16A16_TYPELESS).RT(DXGI_FORMAT_R16G16B16A16_SNORM).VA(DXGI_FORMAT_R16G16B16A16_SNORM).RV(DXGI_FORMAT_R16G16B16A16_SNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGFloat, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32_TYPELESS).RT(DXGI_FORMAT_R32G32_FLOAT).VA(DXGI_FORMAT_R32G32_FLOAT).RV(DXGI_FORMAT_R32G32_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGUInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32_TYPELESS).RT(DXGI_FORMAT_R32G32_UINT).VA(DXGI_FORMAT_R32G32_UINT).RV(DXGI_FORMAT_R32G32_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32G32_TYPELESS).RT(DXGI_FORMAT_R32G32_SINT).VA(DXGI_FORMAT_R32G32_SINT).RV(DXGI_FORMAT_R32G32_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGB10A2UInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R10G10B10A2_TYPELESS).RT(DXGI_FORMAT_R10G10B10A2_UINT).VA(DXGI_FORMAT_R10G10B10A2_UINT).RV(DXGI_FORMAT_R10G10B10A2_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGB10A2UIntNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R10G10B10A2_TYPELESS).RT(DXGI_FORMAT_R10G10B10A2_UNORM).VA(DXGI_FORMAT_R10G10B10A2_UNORM).RV(DXGI_FORMAT_R10G10B10A2_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RG11B10Float, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R11G11B10_FLOAT).RT(DXGI_FORMAT_R11G11B10_FLOAT).VA(DXGI_FORMAT_R11G11B10_FLOAT).RV(DXGI_FORMAT_R11G11B10_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAUByteNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8B8A8_TYPELESS).RT(DXGI_FORMAT_R8G8B8A8_UNORM).VA(DXGI_FORMAT_R8G8B8A8_UNORM).RV(DXGI_FORMAT_R8G8B8A8_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAUByteNormalizedsRGB, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8B8A8_TYPELESS).RT(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB).RV(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAUByte, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8B8A8_TYPELESS).RT(DXGI_FORMAT_R8G8B8A8_UINT).VA(DXGI_FORMAT_R8G8B8A8_UINT).RV(DXGI_FORMAT_R8G8B8A8_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAByteNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8B8A8_TYPELESS).RT(DXGI_FORMAT_R8G8B8A8_SNORM).VA(DXGI_FORMAT_R8G8B8A8_SNORM).RV(DXGI_FORMAT_R8G8B8A8_SNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGBAByte, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8B8A8_TYPELESS).RT(DXGI_FORMAT_R8G8B8A8_SINT).VA(DXGI_FORMAT_R8G8B8A8_SINT).RV(DXGI_FORMAT_R8G8B8A8_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGHalf, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16_TYPELESS).RT(DXGI_FORMAT_R16G16_FLOAT).VA(DXGI_FORMAT_R16G16_FLOAT).RV(DXGI_FORMAT_R16G16_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGUShort, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16_TYPELESS).RT(DXGI_FORMAT_R16G16_UINT).VA(DXGI_FORMAT_R16G16_UINT).RV(DXGI_FORMAT_R16G16_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGUShortNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16_TYPELESS).RT(DXGI_FORMAT_R16G16_UNORM).VA(DXGI_FORMAT_R16G16_UNORM).RV(DXGI_FORMAT_R16G16_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGShort, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16_TYPELESS).RT(DXGI_FORMAT_R16G16_SINT).VA(DXGI_FORMAT_R16G16_SINT).RV(DXGI_FORMAT_R16G16_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGShortNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16G16_TYPELESS).RT(DXGI_FORMAT_R16G16_SNORM).VA(DXGI_FORMAT_R16G16_SNORM).RV(DXGI_FORMAT_R16G16_SNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGUByte, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8_TYPELESS).RT(DXGI_FORMAT_R8G8_UINT).VA(DXGI_FORMAT_R8G8_UINT).RV(DXGI_FORMAT_R8G8_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGUByteNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8_TYPELESS).RT(DXGI_FORMAT_R8G8_UNORM).VA(DXGI_FORMAT_R8G8_UNORM).RV(DXGI_FORMAT_R8G8_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGByte, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8_TYPELESS).RT(DXGI_FORMAT_R8G8_SINT).VA(DXGI_FORMAT_R8G8_SINT).RV(DXGI_FORMAT_R8G8_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RGByteNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8G8_TYPELESS).RT(DXGI_FORMAT_R8G8_SNORM).VA(DXGI_FORMAT_R8G8_SNORM).RV(DXGI_FORMAT_R8G8_SNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::DFloat, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32_TYPELESS).RV(DXGI_FORMAT_R32_FLOAT).D(DXGI_FORMAT_R32_FLOAT).DS(DXGI_FORMAT_D32_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RFloat, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32_TYPELESS).RT(DXGI_FORMAT_R32_FLOAT).VA(DXGI_FORMAT_R32_FLOAT).RV(DXGI_FORMAT_R32_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RUInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32_TYPELESS).RT(DXGI_FORMAT_R32_UINT).VA(DXGI_FORMAT_R32_UINT).RV(DXGI_FORMAT_R32_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RInt, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R32_TYPELESS).RT(DXGI_FORMAT_R32_SINT).VA(DXGI_FORMAT_R32_SINT).RV(DXGI_FORMAT_R32_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RHalf, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16_TYPELESS).RT(DXGI_FORMAT_R16_FLOAT).VA(DXGI_FORMAT_R16_FLOAT).RV(DXGI_FORMAT_R16_FLOAT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RUShort, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16_TYPELESS).RT(DXGI_FORMAT_R16_UINT).VA(DXGI_FORMAT_R16_UINT).RV(DXGI_FORMAT_R16_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RUShortNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16_TYPELESS).RT(DXGI_FORMAT_R16_UNORM).VA(DXGI_FORMAT_R16_UNORM).RV(DXGI_FORMAT_R16_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RShort, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16_TYPELESS).RT(DXGI_FORMAT_R16_SINT).VA(DXGI_FORMAT_R16_SINT).RV(DXGI_FORMAT_R16_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RShortNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16_TYPELESS).RT(DXGI_FORMAT_R16_SNORM).VA(DXGI_FORMAT_R16_SNORM).RV(DXGI_FORMAT_R16_SNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RUByte, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8_TYPELESS).RT(DXGI_FORMAT_R8_UINT).VA(DXGI_FORMAT_R8_UINT).RV(DXGI_FORMAT_R8_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RUByteNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8_TYPELESS).RT(DXGI_FORMAT_R8_UNORM).VA(DXGI_FORMAT_R8_UNORM).RV(DXGI_FORMAT_R8_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RByte, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8_TYPELESS).RT(DXGI_FORMAT_R8_SINT).VA(DXGI_FORMAT_R8_SINT).RV(DXGI_FORMAT_R8_SINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::RByteNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R8_TYPELESS).RT(DXGI_FORMAT_R8_SNORM).VA(DXGI_FORMAT_R8_SNORM).RV(DXGI_FORMAT_R8_SNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::AUByteNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_A8_UNORM).RT(DXGI_FORMAT_A8_UNORM).VA(DXGI_FORMAT_A8_UNORM).RV(DXGI_FORMAT_A8_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::D16, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R16_TYPELESS).RV(DXGI_FORMAT_R16_UNORM).DS(DXGI_FORMAT_D16_UNORM).D(DXGI_FORMAT_R16_UNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::D24S8, ezGALFormatLookupEntryDX12(DXGI_FORMAT_R24G8_TYPELESS).DS(DXGI_FORMAT_D24_UNORM_S8_UINT).D(DXGI_FORMAT_R24_UNORM_X8_TYPELESS).S(DXGI_FORMAT_X24_TYPELESS_G8_UINT));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC1, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC1_TYPELESS).RV(DXGI_FORMAT_BC1_UNORM));
  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC1sRGB, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC1_TYPELESS).RV(DXGI_FORMAT_BC1_UNORM_SRGB));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC2, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC2_TYPELESS).RV(DXGI_FORMAT_BC2_UNORM));
  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC2sRGB, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC2_TYPELESS).RV(DXGI_FORMAT_BC2_UNORM_SRGB));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC3, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC3_TYPELESS).RV(DXGI_FORMAT_BC3_UNORM));
  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC3sRGB, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC3_TYPELESS).RV(DXGI_FORMAT_BC3_UNORM_SRGB));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC4UNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC4_TYPELESS).RV(DXGI_FORMAT_BC4_UNORM));
  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC4Normalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC4_TYPELESS).RV(DXGI_FORMAT_BC4_SNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC5UNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC5_TYPELESS).RV(DXGI_FORMAT_BC5_UNORM));
  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC5Normalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC5_TYPELESS).RV(DXGI_FORMAT_BC5_SNORM));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC6UFloat, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC6H_TYPELESS).RV(DXGI_FORMAT_BC6H_UF16));
  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC6Float, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC6H_TYPELESS).RV(DXGI_FORMAT_BC6H_SF16));

  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC7UNormalized, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC7_TYPELESS).RV(DXGI_FORMAT_BC7_UNORM));
  m_FormatLookupTable.SetFormatInfo(ezGALResourceFormat::BC7UNormalizedsRGB, ezGALFormatLookupEntryDX12(DXGI_FORMAT_BC7_TYPELESS).RV(DXGI_FORMAT_BC7_UNORM_SRGB));
}

EZ_STATICLINK_FILE(RendererDX12, RendererDX12_Device_Implementation_DeviceDX12);
