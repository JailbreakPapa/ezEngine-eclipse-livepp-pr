#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/HairStrandAsset/HairStrandAssetObjects.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezHairStrandAssetProperties, 1, ezRTTIDefaultAllocator<ezHairStrandAssetProperties>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("File", m_sSourceFile)->AddAttributes(new ezFileBrowserAttribute("Select Hair File", "*.abc")),
    EZ_MEMBER_PROPERTY("GlobalWidthScale", m_fGlobalWidthScale)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.01f, 100.0f)),
    EZ_MEMBER_PROPERTY("DefaultWidth", m_fDefaultWidth)->AddAttributes(new ezDefaultValueAttribute(0.0005f), new ezClampValueAttribute(0.00001f, 0.1f)),
    EZ_MEMBER_PROPERTY("TipWidthFraction", m_fTipWidthFraction)->AddAttributes(new ezDefaultValueAttribute(0.1f), new ezClampValueAttribute(0.0f, 1.0f)),
    EZ_MEMBER_PROPERTY("MaxStrandsPerGroup", m_uiMaxStrandsPerGroup),
    EZ_MEMBER_PROPERTY("GenerateUVs", m_bGenerateUVs)->AddAttributes(new ezDefaultValueAttribute(true)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezHairStrandAssetProperties::ezHairStrandAssetProperties() = default;
