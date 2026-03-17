ez_cmake_init()

ez_requires_dx12()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  ShaderCompilerDXC
)

# Need d3dcompiler for D3D12 shader reflection headers
if (EZ_CMAKE_PLATFORM_WINDOWS)
    ez_link_target_dx11(${PROJECT_NAME})
endif()

# Need dxcompiler for DxcCreateInstance
ez_link_target_dxc(${PROJECT_NAME})
