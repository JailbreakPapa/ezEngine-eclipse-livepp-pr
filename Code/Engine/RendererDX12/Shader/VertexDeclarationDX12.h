#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/Shader/VertexDeclaration.h>
#include <Foundation/Types/TypeTraits.h>

#include <d3d12.h>

EZ_DEFINE_AS_POD_TYPE(D3D12_INPUT_ELEMENT_DESC);

/// D3D12 vertex declaration implementation.
///
/// D3D12 does not have a standalone input layout object like D3D11. Instead, the
/// input element descriptions are embedded into the graphics pipeline state object.
/// This class builds and stores the D3D12_INPUT_ELEMENT_DESC array and per-slot
/// strides for use during PSO creation.
class EZ_RENDERERDX12_DLL ezGALVertexDeclarationDX12 : public ezGALVertexDeclaration
{
public:
  EZ_ALWAYS_INLINE ezArrayPtr<const D3D12_INPUT_ELEMENT_DESC> GetInputElements() const { return m_InputElements; }
  EZ_ALWAYS_INLINE ezArrayPtr<const ezUInt32> GetVertexBufferStrides() const { return m_VertexBufferStrides; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  ezGALVertexDeclarationDX12(const ezGALVertexDeclarationCreationDescription& Description);
  virtual ~ezGALVertexDeclarationDX12();

  ezHybridArray<D3D12_INPUT_ELEMENT_DESC, 8> m_InputElements;
  ezHybridArray<ezUInt32, EZ_GAL_MAX_VERTEX_BUFFER_COUNT> m_VertexBufferStrides;
};
