ez_cmake_init()

ez_requires_yoga()
ez_requires_skribidi()

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

ez_enable_strict_warnings(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Foundation
  Core
  PRIVATE
  yogacore
  Skribidi
)

