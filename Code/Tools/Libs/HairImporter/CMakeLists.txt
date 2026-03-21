ez_cmake_init()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Foundation
  RendererCore
)

if(TARGET Alembic)
  target_link_libraries(${PROJECT_NAME} PRIVATE Alembic)
  target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDSYSTEM_ENABLE_ALEMBIC_SUPPORT)
elseif(TARGET Alembic::Alembic)
  target_link_libraries(${PROJECT_NAME} PRIVATE Alembic::Alembic)
  target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDSYSTEM_ENABLE_ALEMBIC_SUPPORT)
endif()
