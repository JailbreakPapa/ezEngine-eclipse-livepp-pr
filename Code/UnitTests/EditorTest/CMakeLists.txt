ez_cmake_init()

ez_requires_editor()
ez_requires(EZ_CMAKE_PLATFORM_WINDOWS)

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(APPLICATION ${PROJECT_NAME})

ez_link_target_qt(TARGET ${PROJECT_NAME} COMPONENTS Core Gui Widgets Network)

target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../EditorPlugins/Scene")

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  TestFramework
  EditorFramework
  EditorEngineProcessFramework
  RendererDX11
  EditorPluginScene
)

add_dependencies(${PROJECT_NAME}
  EditorProcessor
  EditorEngineProcess
  EditorPluginScene
  EditorPluginAssets
  EditorPluginParticle
  EditorPluginProcGen
  ParticlePlugin
  TexConv
  Player
  InspectorPlugin
  ShaderCompiler
)

ez_ci_add_test(${PROJECT_NAME} NEEDS_HW_ACCESS)
