#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/VirtualShadowPageTable.h>

ezVirtualShadowPageTable::ezVirtualShadowPageTable() = default;
ezVirtualShadowPageTable::~ezVirtualShadowPageTable() = default;

void ezVirtualShadowPageTable::Initialize(ezUInt32 uiPagesPerLevel, ezUInt32 uiNumLevels, ezUInt32 uiMaxPhysicalPages)
{
  m_uiPagesPerLevel = uiPagesPerLevel;
  m_uiNumLevels = uiNumLevels;
  m_uiMaxPhysicalPages = uiMaxPhysicalPages;
  m_uiNumAllocatedPages = 0;

  const ezUInt32 totalVirtualPages = uiNumLevels * uiPagesPerLevel * uiPagesPerLevel;
  m_Pages.SetCount(totalVirtualPages);

  m_PhysicalPageUsed.SetCount(uiMaxPhysicalPages);
  for (ezUInt32 i = 0; i < uiMaxPhysicalPages; ++i)
  {
    m_PhysicalPageUsed[i] = false;
  }
}

void ezVirtualShadowPageTable::Reset()
{
  for (auto& page : m_Pages)
  {
    page = PageInfo();
  }
  for (auto& used : m_PhysicalPageUsed)
  {
    used = false;
  }
  m_uiNumAllocatedPages = 0;
}

void ezVirtualShadowPageTable::MarkPageRequested(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY, ezUInt32 uiCurrentFrame)
{
  const ezUInt32 idx = GetFlatIndex(uiLevel, uiPageX, uiPageY);
  m_Pages[idx].m_bRequested = true;
  m_Pages[idx].m_uiLastUsedFrame = uiCurrentFrame;
}

void ezVirtualShadowPageTable::MarkPageDirty(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY)
{
  const ezUInt32 idx = GetFlatIndex(uiLevel, uiPageX, uiPageY);
  m_Pages[idx].m_bDirty = true;
}

void ezVirtualShadowPageTable::AllocateRequestedPages(ezUInt32 uiCurrentFrame, ezDynamicArray<ezUInt32>& out_newlyAllocated)
{
  out_newlyAllocated.Clear();

  for (ezUInt32 i = 0; i < m_Pages.GetCount(); ++i)
  {
    PageInfo& page = m_Pages[i];

    if (!page.m_bRequested)
    {
      continue;
    }

    page.m_bRequested = false;

    // Already allocated and valid
    if (page.m_uiPhysicalPageIndex != 0xFFFF)
    {
      page.m_uiLastUsedFrame = uiCurrentFrame;
      if (page.m_bDirty)
      {
        out_newlyAllocated.PushBack(i);
      }
      continue;
    }

    // Need to allocate a new physical page
    ezUInt16 physPage = AllocatePhysicalPage(uiCurrentFrame);
    if (physPage == 0xFFFF)
    {
      // Pool is full, try to evict
      EvictLRUPage(uiCurrentFrame);
      physPage = AllocatePhysicalPage(uiCurrentFrame);
    }

    if (physPage != 0xFFFF)
    {
      page.m_uiPhysicalPageIndex = physPage;
      page.m_uiLastUsedFrame = uiCurrentFrame;
      page.m_bDirty = true;
      out_newlyAllocated.PushBack(i);
    }
  }
}

void ezVirtualShadowPageTable::ClearDirtyFlag(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY)
{
  const ezUInt32 idx = GetFlatIndex(uiLevel, uiPageX, uiPageY);
  m_Pages[idx].m_bDirty = false;
}

void ezVirtualShadowPageTable::SetWrapOffset(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY, ezInt32 iWrapX, ezInt32 iWrapY)
{
  const ezUInt32 idx = GetFlatIndex(uiLevel, uiPageX, uiPageY);
  m_Pages[idx].m_iWrapOffsetX = iWrapX;
  m_Pages[idx].m_iWrapOffsetY = iWrapY;
}

const ezVirtualShadowPageTable::PageInfo& ezVirtualShadowPageTable::GetPageInfo(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY) const
{
  return m_Pages[GetFlatIndex(uiLevel, uiPageX, uiPageY)];
}

ezUInt32 ezVirtualShadowPageTable::GetFlatIndex(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY) const
{
  return uiLevel * m_uiPagesPerLevel * m_uiPagesPerLevel + uiPageY * m_uiPagesPerLevel + uiPageX;
}

ezUInt16 ezVirtualShadowPageTable::AllocatePhysicalPage(ezUInt32 uiCurrentFrame)
{
  for (ezUInt32 i = 0; i < m_uiMaxPhysicalPages; ++i)
  {
    if (!m_PhysicalPageUsed[i])
    {
      m_PhysicalPageUsed[i] = true;
      m_uiNumAllocatedPages++;
      return static_cast<ezUInt16>(i);
    }
  }
  return 0xFFFF;
}

void ezVirtualShadowPageTable::EvictLRUPage(ezUInt32 uiCurrentFrame)
{
  // Find the least recently used allocated page
  ezUInt32 bestIndex = ezMath::MaxValue<ezUInt32>();
  ezUInt32 oldestFrame = uiCurrentFrame;

  for (ezUInt32 i = 0; i < m_Pages.GetCount(); ++i)
  {
    const PageInfo& page = m_Pages[i];
    if (page.m_uiPhysicalPageIndex != 0xFFFF && page.m_uiLastUsedFrame < oldestFrame && !page.m_bRequested)
    {
      oldestFrame = page.m_uiLastUsedFrame;
      bestIndex = i;
    }
  }

  if (bestIndex != ezMath::MaxValue<ezUInt32>())
  {
    PageInfo& evicted = m_Pages[bestIndex];
    m_PhysicalPageUsed[evicted.m_uiPhysicalPageIndex] = false;
    m_uiNumAllocatedPages--;
    evicted.m_uiPhysicalPageIndex = 0xFFFF;
    evicted.m_bDirty = true;
  }
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_VirtualShadowPageTable);
