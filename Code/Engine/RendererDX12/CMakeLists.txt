ez_cmake_init()


ez_requires_dx12()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

ez_link_target_dx12(${PROJECT_NAME})

ez_enable_strict_warnings(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Foundation
  RendererFoundation
  D3D12MA
)

if(EZ_3RDPARTY_GLFW_SUPPORT)

  target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

endif()
