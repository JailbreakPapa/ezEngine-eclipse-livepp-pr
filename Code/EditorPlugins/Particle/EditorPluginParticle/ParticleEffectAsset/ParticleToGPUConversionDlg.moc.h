#pragma once

#include <EditorPluginParticle/EditorPluginParticleDLL.h>
#include <EditorPluginParticle/ui_ParticleToGPUConversionDlg.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/String.h>
#include <QDialog>

class ezAbstractObjectNode;

class EZ_EDITORPLUGINPARTICLE_DLL ezQtParticleToGPUConversionDlg : public QDialog, public Ui_ParticleToGPUConversionDlg
{
  Q_OBJECT

public:
  ezQtParticleToGPUConversionDlg(QWidget* pParent);

private Q_SLOTS:
  void on_BrowseButton_clicked();
  void on_ScanButton_clicked();
  void on_ConvertAllButton_clicked();
  void on_CloseButton_clicked();

private:
  struct FileEntry
  {
    ezString m_sAbsPath;
    ezUInt32 m_uiQuads = 0;
    ezUInt32 m_uiPoints = 0;
    ezUInt32 m_uiTrails = 0;
    bool m_bOldFormat = false;
  };

  struct ConversionResult
  {
    ezUInt32 m_uiConverted = 0;
    ezUInt32 m_uiSkipped = 0;
    bool m_bError = false;
    QString m_sError;
  };

  void ScanDirectory(const QString& sDir);
  void ScanFile(const ezString& sAbsPath, FileEntry& out_entry);
  ConversionResult ConvertFile(const FileEntry& entry);

  void ConvertQuadNode(ezAbstractObjectNode* pNode, ConversionResult& r);
  void ConvertPointNode(ezAbstractObjectNode* pNode, ConversionResult& r);
  void ConvertTrailNode(ezAbstractObjectNode* pNode, ConversionResult& r);

  void AppendLog(const QString& sMsg);

  ezDynamicArray<FileEntry> m_Files;
};
