#pragma once

// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RAZORPLUGIN_LIB
#    define EZ_RAZORPLUGIN_DLL EZ_DECL_EXPORT
#  else
#    define EZ_RAZORPLUGIN_DLL EZ_DECL_IMPORT
#  endif
#else
#  define EZ_RAZORPLUGIN_DLL
#endif
