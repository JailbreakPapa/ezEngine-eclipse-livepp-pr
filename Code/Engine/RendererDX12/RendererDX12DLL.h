#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Platform/Win/Utils/IncludeWindows.h>
#include <RendererFoundation/RendererFoundationDLL.h>

// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RENDERERDX12_LIB
#    define EZ_RENDERERDX12_DLL EZ_DECL_EXPORT
#  else
#    define EZ_RENDERERDX12_DLL EZ_DECL_IMPORT
#  endif
#else
#  define EZ_RENDERERDX12_DLL
#endif

#define EZ_GAL_DX12_RELEASE(d3dobj) \
  do                                \
  {                                 \
    if ((d3dobj) != nullptr)        \
    {                               \
      (d3dobj)->Release();          \
      (d3dobj) = nullptr;           \
    }                               \
  } while (0)
