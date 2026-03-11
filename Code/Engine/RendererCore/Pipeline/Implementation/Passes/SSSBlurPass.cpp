#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/SSSBlurPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSSSBlurPass, 1, ezRTTIDefaultAllocator<ezSSSBlurPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ColorInput", m_PinColorInput),
    EZ_MEMBER_PROPERTY("DepthInput", m_PinDepthInput),
    EZ_MEMBER_PROPERTY("GBuffer1Input", m_PinGBuffer1Input),
    EZ_MEMBER_PROPERTY("ColorOutput", m_PinColorOutput),
    EZ_ACCESSOR_PROPERTY("SSSStrength", GetSSSStrength, SetSSSStrength)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 5.0f)),
    EZ_ACCESSOR_PROPERTY("KernelSize", GetKernelSize, SetKernelSize)->AddAttributes(new ezDefaultValueAttribute(25), new ezClampValueAttribute(1, 25)),
    EZ_ACCESSOR_PROPERTY("DepthThreshold", GetDepthThreshold, SetDepthThreshold)->AddAttributes(new ezDefaultValueAttribute(0.05f), new ezClampValueAttribute(0.001f, 1.0f)),
    EZ_ACCESSOR_PROPERTY("ProfileIndex", GetProfileIndex, SetProfileIndex)->AddAttributes(new ezDefaultValueAttribute(0), new ezClampValueAttribute(0, 3)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Effects")
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezSSSBlurPass::ezSSSBlurPass(const char* szName)
  : ezRenderPipelinePass(szName, true)
{
  m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/SSSBlur.ezShader");
  m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezSSSConstants>();
}

ezSSSBlurPass::~ezSSSBlurPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

bool ezSSSBlurPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  if (!inputs[m_PinColorInput.m_uiInputIndex])
  {
    ezLog::Error("No color input connected to '{0}'!", GetName());
    return false;
  }

  if (!inputs[m_PinDepthInput.m_uiInputIndex])
  {
    ezLog::Error("No depth input connected to '{0}'!", GetName());
    return false;
  }

  // Output matches input format
  outputs[m_PinColorOutput.m_uiOutputIndex] = *inputs[m_PinColorInput.m_uiInputIndex];
  return true;
}

void ezSSSBlurPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pColorInput = inputs[m_PinColorInput.m_uiInputIndex];
  auto pDepthInput = inputs[m_PinDepthInput.m_uiInputIndex];
  auto pColorOutput = outputs[m_PinColorOutput.m_uiOutputIndex];

  if (pColorInput == nullptr || pDepthInput == nullptr || pColorOutput == nullptr)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  const ezGALTexture* pColorTex = pDevice->GetTexture(pColorInput->m_TextureHandle);
  float invWidth = 1.0f / (float)pColorTex->GetDescription().m_uiWidth;
  float invHeight = 1.0f / (float)pColorTex->GetDescription().m_uiHeight;

  // Acquire temporary texture for horizontal pass output
  ezGALTextureCreationDescription tempDesc = pColorOutput->m_Desc;
  tempDesc.m_TextureFlags.Add(ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::RenderTarget);
  ezGALTextureHandle hTempTexture = ezGPUResourcePool::GetDefaultInstance()->GetRenderTarget(tempDesc);

  // Bind shader and common resources
  renderViewContext.m_pRenderContext->BindShader(m_hShader);
  renderViewContext.m_pRenderContext->BindNullMeshBuffer(ezGALPrimitiveTopology::Triangles, 1);

  bool bUseGBuffer = (inputs[m_PinGBuffer1Input.m_uiInputIndex] != nullptr);
  if (bUseGBuffer)
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("SSS_USE_GBUFFER", "TRUE");
  else
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("SSS_USE_GBUFFER", "FALSE");

  // Constant buffer is in BG_FRAME (set 0)
  ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
  bindGroup.BindBuffer("ezSSSConstants", m_hConstantBuffer);

  // DepthBuffer, GBuffer1Texture, and BlurSource are in BG_RENDER_PASS (set 1)
  ezBindGroupBuilder& bindGroupRenderPass = renderViewContext.m_pRenderContext->GetBindGroup(EZ_GAL_BIND_GROUP_RENDER_PASS);
  bindGroupRenderPass.BindTexture("DepthBuffer", pDepthInput->m_TextureHandle);

  // Bind GBuffer1 for material flag testing (deferred path only)
  if (bUseGBuffer)
  {
    bindGroupRenderPass.BindTexture("GBuffer1Texture", inputs[m_PinGBuffer1Input.m_uiInputIndex]->m_TextureHandle);
  }

  // Horizontal pass
  {
    EZ_PROFILE_SCOPE("SSS Blur Horizontal");

    ezSSSConstants* cb = ezRenderContext::GetConstantBufferData<ezSSSConstants>(m_hConstantBuffer);
    cb->InverseScreenSize.x = invWidth;
    cb->InverseScreenSize.y = invHeight;
    cb->KernelSize = m_uiKernelSize;
    cb->SSSStrength = m_fSSSStrength;
    cb->BlurDirection.x = 1.0f;
    cb->BlurDirection.y = 0.0f;
    cb->DepthThreshold = m_fDepthThreshold;
    cb->ProfileIndex = m_uiProfileIndex;

    ezGALRenderingSetup renderingSetup;
    renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(hTempTexture));

    auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(renderViewContext, renderingSetup, "SSS Blur H", renderViewContext.m_pCamera->IsStereoscopic());

    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLUR_DIRECTION", "BLUR_DIRECTION_HORIZONTAL");
    bindGroupRenderPass.BindTexture("BlurSource", pColorInput->m_TextureHandle);
    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
  }

  // Vertical pass
  {
    EZ_PROFILE_SCOPE("SSS Blur Vertical");

    ezSSSConstants* cb = ezRenderContext::GetConstantBufferData<ezSSSConstants>(m_hConstantBuffer);
    cb->BlurDirection.x = 0.0f;
    cb->BlurDirection.y = 1.0f;

    ezGALRenderingSetup renderingSetup;
    renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(pColorOutput->m_TextureHandle));

    auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(renderViewContext, renderingSetup, "SSS Blur V", renderViewContext.m_pCamera->IsStereoscopic());

    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLUR_DIRECTION", "BLUR_DIRECTION_VERTICAL");
    bindGroupRenderPass.BindTexture("BlurSource", hTempTexture);
    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
  }

  // Return temporary texture
  ezGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hTempTexture);
}

ezResult ezSSSBlurPass::Serialize(ezStreamWriter& inout_stream) const
{
  EZ_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_fSSSStrength;
  inout_stream << m_uiKernelSize;
  inout_stream << m_fDepthThreshold;
  inout_stream << m_uiProfileIndex;
  return EZ_SUCCESS;
}

ezResult ezSSSBlurPass::Deserialize(ezStreamReader& inout_stream)
{
  EZ_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const ezUInt32 uiVersion = ezTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  EZ_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_fSSSStrength;
  inout_stream >> m_uiKernelSize;
  inout_stream >> m_fDepthThreshold;
  inout_stream >> m_uiProfileIndex;
  return EZ_SUCCESS;
}

void ezSSSBlurPass::SetSSSStrength(float fStrength) { m_fSSSStrength = ezMath::Clamp(fStrength, 0.0f, 5.0f); }
float ezSSSBlurPass::GetSSSStrength() const { return m_fSSSStrength; }

void ezSSSBlurPass::SetKernelSize(ezUInt32 uiSize) { m_uiKernelSize = ezMath::Clamp(uiSize, 1u, 25u); }
ezUInt32 ezSSSBlurPass::GetKernelSize() const { return m_uiKernelSize; }

void ezSSSBlurPass::SetDepthThreshold(float fThreshold) { m_fDepthThreshold = ezMath::Clamp(fThreshold, 0.001f, 1.0f); }
float ezSSSBlurPass::GetDepthThreshold() const { return m_fDepthThreshold; }

void ezSSSBlurPass::SetProfileIndex(ezUInt32 uiIndex) { m_uiProfileIndex = ezMath::Min(uiIndex, 3u); }
ezUInt32 ezSSSBlurPass::GetProfileIndex() const { return m_uiProfileIndex; }

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_SSSBlurPass);
