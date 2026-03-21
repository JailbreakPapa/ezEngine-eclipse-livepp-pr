#pragma once

#include <Foundation/Basics.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class ezHairStrandAssetDocument;

class ezQtHairStrandAssetDocumentWindow : public ezQtDocumentWindow
{
  Q_OBJECT

public:
  ezQtHairStrandAssetDocumentWindow(ezHairStrandAssetDocument* pDocument);
  ~ezQtHairStrandAssetDocumentWindow();
};
