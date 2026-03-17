#include <EditorPluginParticle/EditorPluginParticlePCH.h>

#include <EditorPluginParticle/ParticleEffectAsset/ParticleToGPUConversionDlg.moc.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Strings/StringBuilder.h>

#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidgetItem>

ezQtParticleToGPUConversionDlg::ezQtParticleToGPUConversionDlg(QWidget* pParent)
  : QDialog(pParent)
{
  setupUi(this);
}

void ezQtParticleToGPUConversionDlg::on_BrowseButton_clicked()
{
  QString sDir = QFileDialog::getExistingDirectory(this, "Select Folder", FolderPath->text());
  if (!sDir.isEmpty())
    FolderPath->setText(sDir);
}

void ezQtParticleToGPUConversionDlg::on_ScanButton_clicked()
{
  m_Files.Clear();
  FileList->clear();
  ConvertAllButton->setEnabled(false);
  LogOutput->clear();

  const QString sDir = FolderPath->text().trimmed();
  if (sDir.isEmpty())
  {
    AppendLog("No folder selected.");
    return;
  }

  ScanDirectory(sDir);

  if (m_Files.IsEmpty())
  {
    AppendLog("No .ezParticleEffectAsset files found.");
    return;
  }

  bool bAnyConvertible = false;
  for (const FileEntry& entry : m_Files)
  {
    const QString sName = QFileInfo(QString::fromUtf8(entry.m_sAbsPath.GetData())).fileName();
    QString sLabel;

    if (entry.m_bOldFormat)
    {
      sLabel = sName + "  [Old format - open and save in editor first]";
    }
    else if (entry.m_uiQuads == 0 && entry.m_uiPoints == 0 && entry.m_uiTrails == 0)
    {
      sLabel = sName + "  (nothing to convert)";
    }
    else
    {
      QStringList parts;
      if (entry.m_uiQuads > 0)
        parts << QString("%1 Quad").arg(entry.m_uiQuads);
      if (entry.m_uiPoints > 0)
        parts << QString("%1 Point").arg(entry.m_uiPoints);
      if (entry.m_uiTrails > 0)
        parts << QString("%1 Trail").arg(entry.m_uiTrails);
      sLabel = sName + "  [" + parts.join(", ") + "]";
      bAnyConvertible = true;
    }

    auto* pItem = new QListWidgetItem(sLabel, FileList);
    if (entry.m_bOldFormat)
      pItem->setForeground(QColor(180, 120, 0));
    else if (entry.m_uiQuads == 0 && entry.m_uiPoints == 0 && entry.m_uiTrails == 0)
      pItem->setForeground(Qt::gray);
  }

  AppendLog(QString("Found %1 file(s). %2 have convertible CPU types.")
              .arg(m_Files.GetCount())
              .arg(bAnyConvertible ? "Some" : "None"));

  ConvertAllButton->setEnabled(bAnyConvertible);
}

void ezQtParticleToGPUConversionDlg::ScanDirectory(const QString& sDir)
{
  QDirIterator it(sDir, QStringList() << "*.ezParticleEffectAsset", QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext())
  {
    const QString sFilePath = it.next();
    FileEntry entry;
    entry.m_sAbsPath = sFilePath.toUtf8().constData();
    ScanFile(entry.m_sAbsPath, entry);
    m_Files.PushBack(entry);
  }
}

void ezQtParticleToGPUConversionDlg::ScanFile(const ezString& sAbsPath, FileEntry& out_entry)
{
  ezOSFile osFile;
  if (osFile.Open(sAbsPath, ezFileOpenMode::Read).Failed())
    return;

  ezDynamicArray<ezUInt8> buffer;
  const ezUInt64 uiFileSize = osFile.GetFileSize();
  if (uiFileSize == 0)
  {
    osFile.Close();
    return;
  }

  buffer.SetCountUninitialized((ezUInt32)uiFileSize);
  osFile.Read(buffer.GetData(), buffer.GetCount());
  osFile.Close();

  ezRawMemoryStreamReader stream(buffer.GetData(), buffer.GetCount());
  ezUniquePtr<ezAbstractObjectGraph> pHeader, pGraph, pTypes;
  if (ezAbstractGraphDdlSerializer::ReadDocument(stream, pHeader, pGraph, pTypes).Failed())
    return;

  bool bFoundEffectNode = false;
  for (auto it = pGraph->GetAllNodes().GetIterator(); it.IsValid(); ++it)
  {
    const ezAbstractObjectNode* pNode = it.Value();
    const ezStringView sType = pNode->GetType();

    if (sType == "ezParticleEffectNode")
      bFoundEffectNode = true;
    else if (sType == "ezParticleTypeQuadFactory")
      out_entry.m_uiQuads++;
    else if (sType == "ezParticleTypePointFactory")
      out_entry.m_uiPoints++;
    else if (sType == "ezParticleTypeTrailFactory")
      out_entry.m_uiTrails++;
  }

  if (!bFoundEffectNode)
  {
    // Old format: pre-graph asset, cannot batch convert
    out_entry.m_bOldFormat = true;
    out_entry.m_uiQuads = 0;
    out_entry.m_uiPoints = 0;
    out_entry.m_uiTrails = 0;
  }
}

void ezQtParticleToGPUConversionDlg::on_ConvertAllButton_clicked()
{
  ConvertAllButton->setEnabled(false);
  LogOutput->clear();

  const bool bDoQuad = ConvertQuad->isChecked();
  const bool bDoPoint = ConvertPoint->isChecked();
  const bool bDoTrail = ConvertTrail->isChecked();

  ezUInt32 uiFilesConverted = 0;
  ezUInt32 uiTotalTypes = 0;

  for (const FileEntry& entry : m_Files)
  {
    if (entry.m_bOldFormat)
      continue;

    const bool bHasWork = (bDoQuad && entry.m_uiQuads > 0) ||
                          (bDoPoint && entry.m_uiPoints > 0) ||
                          (bDoTrail && entry.m_uiTrails > 0);
    if (!bHasWork)
      continue;

    const ConversionResult result = ConvertFile(entry);
    const QString sName = QFileInfo(QString::fromUtf8(entry.m_sAbsPath.GetData())).fileName();

    if (result.m_bError)
    {
      AppendLog("ERROR [" + sName + "]: " + result.m_sError);
    }
    else if (result.m_uiConverted > 0)
    {
      uiFilesConverted++;
      uiTotalTypes += result.m_uiConverted;
      AppendLog(QString("Converted %1: %2 type(s)%3.")
                  .arg(sName)
                  .arg(result.m_uiConverted)
                  .arg(result.m_uiSkipped > 0 ? QString(", %1 skipped (Mesh)").arg(result.m_uiSkipped) : ""));
    }
  }

  AppendLog(QString("Done. %1 file(s) converted, %2 type node(s) total.").arg(uiFilesConverted).arg(uiTotalTypes));
}

ezQtParticleToGPUConversionDlg::ConversionResult ezQtParticleToGPUConversionDlg::ConvertFile(const FileEntry& entry)
{
  ConversionResult result;

  // --- Read ---
  ezOSFile osFile;
  if (osFile.Open(entry.m_sAbsPath, ezFileOpenMode::Read).Failed())
  {
    result.m_bError = true;
    result.m_sError = "Failed to open file for reading.";
    return result;
  }

  ezDynamicArray<ezUInt8> buffer;
  const ezUInt64 uiFileSize = osFile.GetFileSize();
  buffer.SetCountUninitialized((ezUInt32)uiFileSize);
  osFile.Read(buffer.GetData(), buffer.GetCount());
  osFile.Close();

  ezRawMemoryStreamReader streamIn(buffer.GetData(), buffer.GetCount());
  ezUniquePtr<ezAbstractObjectGraph> pHeaderGraph, pObjectGraph, pTypesGraph;
  if (ezAbstractGraphDdlSerializer::ReadDocument(streamIn, pHeaderGraph, pObjectGraph, pTypesGraph).Failed())
  {
    result.m_bError = true;
    result.m_sError = "Failed to parse DDL graph.";
    return result;
  }

  // --- Convert ---
  const bool bDoQuad = ConvertQuad->isChecked();
  const bool bDoPoint = ConvertPoint->isChecked();
  const bool bDoTrail = ConvertTrail->isChecked();

  for (auto it = pObjectGraph->GetAllNodes().GetIterator(); it.IsValid(); ++it)
  {
    ezAbstractObjectNode* pNode = it.Value();
    const ezStringView sType = pNode->GetType();

    if (bDoQuad && sType == "ezParticleTypeQuadFactory")
      ConvertQuadNode(pNode, result);
    else if (bDoPoint && sType == "ezParticleTypePointFactory")
      ConvertPointNode(pNode, result);
    else if (bDoTrail && sType == "ezParticleTypeTrailFactory")
      ConvertTrailNode(pNode, result);
    else if (sType == "ezParticleTypeMeshFactory")
      result.m_uiSkipped++;
  }

  if (result.m_uiConverted == 0)
    return result;

  // --- Write ---
  ezContiguousMemoryStreamStorage outputStorage;
  {
    ezMemoryStreamWriter writer(&outputStorage);
    ezAbstractGraphDdlSerializer::WriteDocument(writer, pHeaderGraph.Borrow(), pObjectGraph.Borrow(), pTypesGraph.Borrow());
  }

  ezOSFile outFile;
  if (outFile.Open(entry.m_sAbsPath, ezFileOpenMode::Write).Failed())
  {
    result.m_bError = true;
    result.m_sError = "Failed to open file for writing.";
    return result;
  }
  outFile.Write(outputStorage.GetData(), outputStorage.GetStorageSize64()).IgnoreResult();
  outFile.Close();

  return result;
}

void ezQtParticleToGPUConversionDlg::ConvertQuadNode(ezAbstractObjectNode* pNode, ConversionResult& r)
{
  bool bVelocityAligned = false;
  float fStretch = 1.0f;

  if (const auto* pOrient = pNode->FindProperty("Orientation"))
  {
    const ezStringView sOrientation = pOrient->m_Value.Get<ezString>();
    bVelocityAligned = sOrientation.FindSubString("FixedAxis_ParticleDir") != nullptr;
  }

  if (const auto* pStretch = pNode->FindProperty("ParticleStretch"))
    fStretch = pStretch->m_Value.ConvertTo<float>();

  ezVariant texture, renderMode, nodePos;
  if (const auto* p = pNode->FindProperty("Texture")) texture = p->m_Value;
  if (const auto* p = pNode->FindProperty("RenderMode")) renderMode = p->m_Value;
  if (const auto* p = pNode->FindProperty("Node::Pos")) nodePos = p->m_Value;

  pNode->ClearProperties();
  pNode->SetType("ezParticleTypeGPUFactory");
  pNode->SetTypeVersion(1);

  pNode->AddProperty("MaxParticles", ezVariant((ezUInt32)65536));
  if (texture.IsValid())
    pNode->AddProperty("Texture", texture);
  if (renderMode.IsValid())
    pNode->AddProperty("RenderMode", renderMode);

  if (bVelocityAligned)
  {
    pNode->AddProperty("GPURenderType", ezVariant(ezString("ezGPUParticleRenderType::VelocityAligned")));
    pNode->AddProperty("VelocityStretch", ezVariant(fStretch));
  }
  else
  {
    pNode->AddProperty("GPURenderType", ezVariant(ezString("ezGPUParticleRenderType::Billboard")));
  }

  if (nodePos.IsValid())
    pNode->AddProperty("Node::Pos", nodePos);

  r.m_uiConverted++;
}

void ezQtParticleToGPUConversionDlg::ConvertPointNode(ezAbstractObjectNode* pNode, ConversionResult& r)
{
  ezVariant nodePos;
  if (const auto* p = pNode->FindProperty("Node::Pos")) nodePos = p->m_Value;

  pNode->ClearProperties();
  pNode->SetType("ezParticleTypeGPUFactory");
  pNode->SetTypeVersion(1);

  pNode->AddProperty("MaxParticles", ezVariant((ezUInt32)65536));
  pNode->AddProperty("GPURenderType", ezVariant(ezString("ezGPUParticleRenderType::Point")));

  if (nodePos.IsValid())
    pNode->AddProperty("Node::Pos", nodePos);

  r.m_uiConverted++;
}

void ezQtParticleToGPUConversionDlg::ConvertTrailNode(ezAbstractObjectNode* pNode, ConversionResult& r)
{
  ezVariant texture, renderMode, nodePos;
  if (const auto* p = pNode->FindProperty("Texture")) texture = p->m_Value;
  if (const auto* p = pNode->FindProperty("RenderMode")) renderMode = p->m_Value;
  if (const auto* p = pNode->FindProperty("Node::Pos")) nodePos = p->m_Value;

  ezUInt8 uiMaxTrailPoints = 16;
  if (const auto* p = pNode->FindProperty("Segments"))
  {
    const ezUInt16 uiSegments = p->m_Value.ConvertTo<ezUInt16>();
    uiMaxTrailPoints = (ezUInt8)ezMath::Clamp<ezUInt16>(uiSegments, 4, 64);
  }

  pNode->ClearProperties();
  pNode->SetType("ezParticleTypeGPUFactory");
  pNode->SetTypeVersion(1);

  pNode->AddProperty("MaxParticles", ezVariant((ezUInt32)65536));
  if (texture.IsValid())
    pNode->AddProperty("Texture", texture);
  if (renderMode.IsValid())
    pNode->AddProperty("RenderMode", renderMode);
  pNode->AddProperty("GPURenderType", ezVariant(ezString("ezGPUParticleRenderType::Trail")));
  pNode->AddProperty("MaxTrailPoints", ezVariant(uiMaxTrailPoints));

  if (nodePos.IsValid())
    pNode->AddProperty("Node::Pos", nodePos);

  r.m_uiConverted++;
}

void ezQtParticleToGPUConversionDlg::on_CloseButton_clicked()
{
  accept();
}

void ezQtParticleToGPUConversionDlg::AppendLog(const QString& sMsg)
{
  LogOutput->appendPlainText(sMsg);
}
