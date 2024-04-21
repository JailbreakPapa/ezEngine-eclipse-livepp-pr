# Install script for directory: D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/FidelityFX-SDK")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/opticalflow/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/frameinterpolation/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/fsr3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/fsr3upscaler/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/fsr2/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/fsr1/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/spd/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/cacao/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/lpm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/blur/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/vrs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/cas/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/dof/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/lens/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/parallelsort/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/denoiser/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/sssr/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/components/classifier/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/backends/dx12/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/src/backends/vk/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/Paradox-Engine/Code/ThirdParty/AMDFidelityFXSDK/sdk/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
