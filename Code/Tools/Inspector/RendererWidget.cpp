#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Math/Color.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <Inspector/RendererWidget.moc.h>
#include <QFileDialog>
#include <QMessageBox>

ezQtRendererWidget* ezQtRendererWidget::s_pWidget = nullptr;

ezQtRendererWidget::ezQtRendererWidget(ads::CDockManager* pDockManager, QWidget* pParent)
  : ads::CDockWidget(pDockManager, "Renderer Widget", pParent)
{
  s_pWidget = this;

  setupUi(this);
  setWidget(RendererWidgetFrame);

  setIcon(QIcon(":/Icons/Icons/Type.svg")); // Use existing icon for now

  // Set initial splitter sizes
  QList<int> sizes;
  sizes << 250 << 650;
  splitter->setSizes(sizes);

  ResetStats();
}

void ezQtRendererWidget::ResetStats()
{
  m_Views.Clear();
  TreePasses->clear();
  m_CurrentTexture = TextureData();
  m_DisplayImage = QImage();

  LabelImage->setPixmap(QPixmap());
  LabelImage->setText("Select a render target and click Capture");

  LabelFormatValue->setText("-");
  LabelSizeValue->setText("-");
  LabelSamplesValue->setText("-");

  ButtonCapture->setEnabled(false);
  ButtonSave->setEnabled(false);
}

void ezQtRendererWidget::UpdateStats()
{
  if (!isVisible())
    return;

  if (!ezTelemetry::IsConnectedToServer())
  {
    ButtonRefresh->setEnabled(false);
    ButtonCapture->setEnabled(false);
    return;
  }

  ButtonRefresh->setEnabled(true);
}

void ezQtRendererWidget::ProcessTelemetry(void* pUnused)
{
  if (s_pWidget == nullptr)
    return;

  ezTelemetryMessage Msg;

  while (ezTelemetry::RetrieveMessage('RNDR', Msg) == EZ_SUCCESS)
  {
    if (Msg.GetMessageID() == 'PIPE')
    {
      // Pipeline info response
      s_pWidget->m_Views.Clear();

      ezUInt32 viewCount;
      Msg.GetReader() >> viewCount;

      for (ezUInt32 v = 0; v < viewCount; ++v)
      {
        ViewInfo& view = s_pWidget->m_Views.ExpandAndGetRef();
        Msg.GetReader() >> view.m_sViewName;

        ezUInt32 passCount;
        Msg.GetReader() >> passCount;

        for (ezUInt32 p = 0; p < passCount; ++p)
        {
          PassInfo& pass = view.m_Passes.ExpandAndGetRef();
          Msg.GetReader() >> pass.m_sPassName;

          ezUInt32 outputCount;
          Msg.GetReader() >> outputCount;

          for (ezUInt32 o = 0; o < outputCount; ++o)
          {
            OutputInfo& output = pass.m_Outputs.ExpandAndGetRef();
            Msg.GetReader() >> output.m_sPinName;
            Msg.GetReader() >> output.m_bHasTexture;
            Msg.GetReader() >> output.m_uiWidth;
            Msg.GetReader() >> output.m_uiHeight;
            Msg.GetReader() >> output.m_sFormat;
          }
        }
      }

      s_pWidget->PopulateTreeView();
    }
    else if (Msg.GetMessageID() == 'TDAT')
    {
      // Texture data response
      TextureData& tex = s_pWidget->m_CurrentTexture;

      Msg.GetReader() >> tex.m_sViewName;
      Msg.GetReader() >> tex.m_sPassName;
      Msg.GetReader() >> tex.m_sPinName;
      Msg.GetReader() >> tex.m_uiWidth;
      Msg.GetReader() >> tex.m_uiHeight;
      Msg.GetReader() >> tex.m_sFormat;

      ezUInt32 dataSize;
      Msg.GetReader() >> dataSize;

      tex.m_RawPixels.SetCountUninitialized(dataSize);
      if (dataSize > 0)
      {
        Msg.GetReader().ReadBytes(tex.m_RawPixels.GetData(), dataSize);
      }

      s_pWidget->ProcessTextureForDisplay();
      s_pWidget->UpdateTextureDisplay();
    }
    else if (Msg.GetMessageID() == 'TERR')
    {
      // Error response
      ezString sError;
      Msg.GetReader() >> sError;

      s_pWidget->LabelImage->setPixmap(QPixmap());
      s_pWidget->LabelImage->setText(QString("Error: %1").arg(sError.GetData()));
    }
  }
}

void ezQtRendererWidget::on_ButtonRefresh_clicked()
{
  LabelImage->setText("Requesting pipeline info...");

  ezTelemetryMessage Msg;
  Msg.SetMessageID('RNDR', 'LIST');
  ezTelemetry::SendToServer(Msg);
}

void ezQtRendererWidget::on_ButtonCapture_clicked()
{
  OutputInfo* pOutput = GetSelectedOutput();
  if (pOutput == nullptr)
    return;

  if (!pOutput->m_bHasTexture)
  {
    LabelImage->setText("Selected output has no texture");
    return;
  }

  // Find the view and pass names
  QTreeWidgetItem* pItem = TreePasses->currentItem();
  if (pItem == nullptr)
    return;

  // Get the actual pin name (not the display text with size)
  QString pinName = QString::fromUtf8(pOutput->m_sPinName.GetData());

  QTreeWidgetItem* pPassItem = pItem->parent();
  if (pPassItem == nullptr)
    return;

  QString passName = pPassItem->text(0);

  QTreeWidgetItem* pViewItem = pPassItem->parent();
  if (pViewItem == nullptr)
    return;

  QString viewName = pViewItem->text(0);

  LabelImage->setPixmap(QPixmap());
  LabelImage->setText("Capturing texture...");

  ezTelemetryMessage Msg;
  Msg.SetMessageID('RNDR', ' REQ');
  Msg.GetWriter() << ezString(viewName.toUtf8().data());
  Msg.GetWriter() << ezString(passName.toUtf8().data());
  Msg.GetWriter() << ezString(pinName.toUtf8().data());
  ezTelemetry::SendToServer(Msg);
}

void ezQtRendererWidget::on_ButtonSave_clicked()
{
  if (m_DisplayImage.isNull())
    return;

  QString fileName = QFileDialog::getSaveFileName(this, "Save Texture", QString(), "PNG Images (*.png);;All Files (*)");

  if (fileName.isEmpty())
    return;

  if (!m_DisplayImage.save(fileName))
  {
    QMessageBox::warning(this, "Save Failed", "Failed to save the texture to file.");
  }
}

void ezQtRendererWidget::on_TreePasses_itemSelectionChanged()
{
  OutputInfo* pOutput = GetSelectedOutput();

  if (pOutput == nullptr)
  {
    ButtonCapture->setEnabled(false);
    LabelFormatValue->setText("-");
    LabelSizeValue->setText("-");
    LabelSamplesValue->setText("-");
    return;
  }

  ButtonCapture->setEnabled(pOutput->m_bHasTexture);

  LabelFormatValue->setText(pOutput->m_sFormat.GetData());

  ezStringBuilder s;
  s.SetFormat("{0} x {1}", pOutput->m_uiWidth, pOutput->m_uiHeight);
  LabelSizeValue->setText(s.GetData());

  LabelSamplesValue->setText(pOutput->m_bHasTexture ? "1" : "N/A");
}

void ezQtRendererWidget::on_ComboChannel_currentIndexChanged(int index)
{
  m_iChannelMode = index;
  ProcessTextureForDisplay();
  UpdateTextureDisplay();
}

void ezQtRendererWidget::on_SliderExposure_valueChanged(int value)
{
  m_fExposure = value / 10.0f;

  ezStringBuilder s;
  s.SetFormat("{0} EV", ezArgF(m_fExposure, 1));
  LabelExposureValue->setText(s.GetData());

  ProcessTextureForDisplay();
  UpdateTextureDisplay();
}

void ezQtRendererWidget::on_ComboDepthMode_currentIndexChanged(int index)
{
  m_iDepthMode = index;
  ProcessTextureForDisplay();
  UpdateTextureDisplay();
}

void ezQtRendererWidget::on_CheckNormalize_stateChanged(int state)
{
  m_bNormalize = (state == Qt::Checked);
  ProcessTextureForDisplay();
  UpdateTextureDisplay();
}

void ezQtRendererWidget::PopulateTreeView()
{
  TreePasses->clear();

  for (ViewInfo& view : m_Views)
  {
    QTreeWidgetItem* pViewItem = new QTreeWidgetItem(TreePasses);
    pViewItem->setText(0, view.m_sViewName.GetData());
    pViewItem->setExpanded(true);
    view.m_pTreeItem = pViewItem;

    for (PassInfo& pass : view.m_Passes)
    {
      // Only show passes that have outputs with textures
      bool hasOutputs = false;
      for (const OutputInfo& output : pass.m_Outputs)
      {
        if (output.m_bHasTexture)
        {
          hasOutputs = true;
          break;
        }
      }

      if (!hasOutputs && pass.m_Outputs.IsEmpty())
        continue;

      QTreeWidgetItem* pPassItem = new QTreeWidgetItem(pViewItem);
      pPassItem->setText(0, pass.m_sPassName.GetData());
      pass.m_pTreeItem = pPassItem;

      for (OutputInfo& output : pass.m_Outputs)
      {
        QTreeWidgetItem* pOutputItem = new QTreeWidgetItem(pPassItem);

        ezStringBuilder s;
        if (output.m_bHasTexture)
        {
          s.SetFormat("{0} ({1}x{2})", output.m_sPinName, output.m_uiWidth, output.m_uiHeight);
        }
        else
        {
          s.SetFormat("{0} (no texture)", output.m_sPinName);
          pOutputItem->setForeground(0, QBrush(Qt::gray));
        }
        pOutputItem->setText(0, s.GetData());
        pOutputItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(&output)));
      }
    }
  }
}

void ezQtRendererWidget::ProcessTextureForDisplay()
{
  if (m_CurrentTexture.m_RawPixels.IsEmpty())
    return;

  const ezUInt32 width = m_CurrentTexture.m_uiWidth;
  const ezUInt32 height = m_CurrentTexture.m_uiHeight;

  m_CurrentTexture.m_DisplayPixels.SetCountUninitialized(width * height * 4);

  // Apply exposure multiplier
  float exposureMultiplier = ezMath::Pow(2.0f, m_fExposure);

  // Find min/max for normalization if enabled
  float minVal = 1e10f, maxVal = -1e10f;

  if (m_bNormalize)
  {
    // Assume RGBA float for now - this would need format-specific handling
    const ezUInt32 pixelCount = width * height;
    const ezUInt8* pData = m_CurrentTexture.m_RawPixels.GetData();

    // Simple scan for min/max in first channel
    for (ezUInt32 i = 0; i < pixelCount && i * 4 + 3 < m_CurrentTexture.m_RawPixels.GetCount(); ++i)
    {
      float val = pData[i * 4] / 255.0f; // Assuming 8-bit data
      minVal = ezMath::Min(minVal, val);
      maxVal = ezMath::Max(maxVal, val);
    }

    if (maxVal <= minVal)
    {
      minVal = 0.0f;
      maxVal = 1.0f;
    }
  }
  else
  {
    minVal = 0.0f;
    maxVal = 1.0f;
  }

  const ezUInt32 pixelCount = width * height;
  const ezUInt8* pSrc = m_CurrentTexture.m_RawPixels.GetData();
  ezUInt8* pDst = m_CurrentTexture.m_DisplayPixels.GetData();

  for (ezUInt32 i = 0; i < pixelCount; ++i)
  {
    // Read source values (assuming RGBA8 for now)
    float r = 0, g = 0, b = 0, a = 1;

    if (i * 4 + 3 < m_CurrentTexture.m_RawPixels.GetCount())
    {
      r = pSrc[i * 4 + 0] / 255.0f;
      g = pSrc[i * 4 + 1] / 255.0f;
      b = pSrc[i * 4 + 2] / 255.0f;
      a = pSrc[i * 4 + 3] / 255.0f;
    }

    // Apply normalization
    if (m_bNormalize)
    {
      float range = maxVal - minVal;
      r = (r - minVal) / range;
      g = (g - minVal) / range;
      b = (b - minVal) / range;
    }

    // Apply exposure
    r *= exposureMultiplier;
    g *= exposureMultiplier;
    b *= exposureMultiplier;

    // Apply channel mode
    float outR = r, outG = g, outB = b, outA = 255;

    switch (m_iChannelMode)
    {
      case 0: // RGB
        break;
      case 1: // R
        outG = r;
        outB = r;
        break;
      case 2: // G
        outR = g;
        outB = g;
        break;
      case 3: // B
        outR = b;
        outG = b;
        break;
      case 4: // A
        outR = a;
        outG = a;
        outB = a;
        break;
      case 5: // Luminance
      {
        float lum = 0.299f * r + 0.587f * g + 0.114f * b;
        outR = lum;
        outG = lum;
        outB = lum;
        break;
      }
    }

    // Clamp and convert to 8-bit
    pDst[i * 4 + 0] = static_cast<ezUInt8>(ezMath::Clamp(outB * 255.0f, 0.0f, 255.0f)); // B (BGRA for QImage)
    pDst[i * 4 + 1] = static_cast<ezUInt8>(ezMath::Clamp(outG * 255.0f, 0.0f, 255.0f)); // G
    pDst[i * 4 + 2] = static_cast<ezUInt8>(ezMath::Clamp(outR * 255.0f, 0.0f, 255.0f)); // R
    pDst[i * 4 + 3] = 255;                                                              // A
  }
}

void ezQtRendererWidget::UpdateTextureDisplay()
{
  if (m_CurrentTexture.m_DisplayPixels.IsEmpty())
  {
    LabelImage->setPixmap(QPixmap());
    LabelImage->setText("No texture data");
    ButtonSave->setEnabled(false);
    return;
  }

  const ezUInt32 width = m_CurrentTexture.m_uiWidth;
  const ezUInt32 height = m_CurrentTexture.m_uiHeight;

  m_DisplayImage = QImage(m_CurrentTexture.m_DisplayPixels.GetData(), width, height, width * 4, QImage::Format_ARGB32);

  // Make a copy since QImage doesn't own the data
  m_DisplayImage = m_DisplayImage.copy();

  LabelImage->setPixmap(QPixmap::fromImage(m_DisplayImage));

  // Update properties
  LabelFormatValue->setText(m_CurrentTexture.m_sFormat.GetData());

  ezStringBuilder s;
  s.SetFormat("{0} x {1}", width, height);
  LabelSizeValue->setText(s.GetData());

  ButtonSave->setEnabled(true);
}

ezQtRendererWidget::OutputInfo* ezQtRendererWidget::GetSelectedOutput()
{
  QTreeWidgetItem* pItem = TreePasses->currentItem();
  if (pItem == nullptr)
    return nullptr;

  QVariant data = pItem->data(0, Qt::UserRole);
  if (!data.isValid())
    return nullptr;

  quintptr ptr = data.value<quintptr>();
  if (ptr == 0)
    return nullptr;

  return reinterpret_cast<OutputInfo*>(ptr);
}
