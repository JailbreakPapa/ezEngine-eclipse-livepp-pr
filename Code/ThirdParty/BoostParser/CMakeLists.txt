ez_cmake_init()

ez_requires_boost_parser()

include(FetchContent)

# First try to find Boost from system/vcpkg
find_package(Boost 1.82 QUIET)

if(NOT Boost_FOUND)
  message(STATUS "Boost not found via find_package. Fetching required Boost components via FetchContent...")

  # Fetch minimal Boost components needed by Boost.Parser
  # Using the cmake-fetchcontent-compatible boost-cmake-header-only superproject approach
  set(BOOST_INCLUDE_LIBRARIES parser assert config core hana type_index container_hash describe mp11)
  set(BOOST_ENABLE_CMAKE ON)

  FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz
    URL_HASH SHA256=7da75f171837577a52bbf217e17f8ea576c7c246e4594d617bfde7fafd408be5
    EXCLUDE_FROM_ALL
  )

  FetchContent_MakeAvailable(Boost)
else()
  message(STATUS "Found Boost ${Boost_VERSION} via find_package")
endif()

# Configure Boost.Parser target
if(TARGET Boost::parser)
  target_compile_definitions(Boost::parser INTERFACE BUILDSYSTEM_ENABLE_BOOST_PARSER_SUPPORT)
  # Boost creates many targets; set folder for organization
  get_property(_boost_targets GLOBAL PROPERTY BUILDSYSTEM_TARGETS)
  foreach(_target ${_boost_targets})
    if(_target MATCHES "^Boost::")
      set_target_properties(${_target} PROPERTIES FOLDER "ThirdParty/Boost")
    endif()
  endforeach()
endif()
