### Boost.Parser (downloaded via FetchContent)
### Note: Boost.Parser requires Boost headers. If not available via vcpkg,
### this will attempt to fetch required Boost libraries.
set (EZ_3RDPARTY_BOOST_PARSER_SUPPORT OFF CACHE BOOL "Whether to add support for Boost.Parser. Downloaded via FetchContent. Requires Boost headers (via vcpkg or FetchContent).")
mark_as_advanced(FORCE EZ_3RDPARTY_BOOST_PARSER_SUPPORT)

macro(ez_requires_boost_parser)
  ez_requires_one_of(EZ_CMAKE_PLATFORM_WINDOWS EZ_CMAKE_PLATFORM_LINUX)
  ez_requires(EZ_3RDPARTY_BOOST_PARSER_SUPPORT)
endmacro()

function(ez_link_target_boost_parser TARGET_NAME)
  ez_requires_boost_parser()
  target_link_libraries(${TARGET_NAME} PRIVATE Boost::parser)
endfunction()
