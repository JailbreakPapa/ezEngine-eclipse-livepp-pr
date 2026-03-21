ez_cmake_init()

ez_requires_alembic()

include(FetchContent)

# Alembic build options: static lib, Ogawa backend only, no HDF5, no tests, no tools.
set(USE_HDF5 OFF CACHE BOOL "" FORCE)
set(USE_MAYA OFF CACHE BOOL "" FORCE)
set(USE_PRMAN OFF CACHE BOOL "" FORCE)
set(USE_ARNOLD OFF CACHE BOOL "" FORCE)
set(USE_PYALEMBIC OFF CACHE BOOL "" FORCE)
set(USE_BINARIES OFF CACHE BOOL "" FORCE)
set(USE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_TESTS OFF CACHE BOOL "" FORCE)
set(ALEMBIC_BUILD_LIBS ON CACHE BOOL "" FORCE)
set(ALEMBIC_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ALEMBIC_ILMBASE_LINK_STATIC ON CACHE BOOL "" FORCE)

# Alembic's lib/Alembic/CMakeLists.txt has unconditional install(EXPORT) and
# export(EXPORT) commands that fail because FetchContent'd Imath isn't in any
# export set. We manually populate and patch these out before add_subdirectory.
FetchContent_Declare(
  Alembic
  GIT_REPOSITORY https://github.com/alembic/alembic.git
  GIT_TAG        1.8.9
  GIT_SHALLOW    TRUE
)

FetchContent_GetProperties(Alembic)
if(NOT alembic_POPULATED)
  FetchContent_Populate(Alembic)

  # Patch out the install/export commands that require Imath in an export set.
  set(_alembic_lib_cmake "${alembic_SOURCE_DIR}/lib/Alembic/CMakeLists.txt")
  file(READ "${_alembic_lib_cmake}" _alembic_lib_content)
  # Remove EXPORT from the install(TARGETS) command
  string(REPLACE "EXPORT AlembicTargets" "" _alembic_lib_content "${_alembic_lib_content}")
  # Comment out export(TARGETS ...) block — match the multi-line EXPORT(TARGETS ... ) call
  string(REGEX REPLACE "EXPORT\\(TARGETS[^)]*\\)" "# [patched out by ezEngine]" _alembic_lib_content "${_alembic_lib_content}")
  # Comment out install(EXPORT ...) block
  string(REGEX REPLACE "INSTALL\\(EXPORT[^)]*\\)" "# [patched out by ezEngine]" _alembic_lib_content "${_alembic_lib_content}")
  file(WRITE "${_alembic_lib_cmake}" "${_alembic_lib_content}")

  add_subdirectory(${alembic_SOURCE_DIR} ${alembic_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Configure the fetched Alembic target.
if(TARGET Alembic)
  target_compile_definitions(Alembic PUBLIC BUILDSYSTEM_ENABLE_ALEMBIC_SUPPORT)
  set_target_properties(Alembic PROPERTIES FOLDER "ThirdParty")

  if(EZ_CMAKE_COMPILER_MSVC)
    target_compile_options(Alembic PRIVATE /wd4267 /wd4244 /wd4100 /wd4456 /wd4457 /wd4996)
  endif()
endif()

# Provide a link function for consumers.
function(ez_link_target_alembic TARGET_NAME)
  ez_requires_alembic()
  target_link_libraries(${TARGET_NAME} PRIVATE Alembic)
endfunction()
