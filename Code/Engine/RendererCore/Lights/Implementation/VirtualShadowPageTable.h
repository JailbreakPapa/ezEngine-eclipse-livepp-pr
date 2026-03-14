#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Mat4.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// Manages the virtual-to-physical page mapping for virtual shadow maps.
///
/// Tracks which virtual pages are allocated, maps them to physical page indices in the
/// shadow atlas, and provides LRU-based eviction when the physical page pool is full.
/// Each clipmap level has its own set of virtual pages.
class ezVirtualShadowPageTable
{
public:
  struct PageInfo
  {
    ezUInt16 m_uiPhysicalPageIndex = 0xFFFF; ///< Index into the physical page pool, 0xFFFF = not allocated.
    ezUInt32 m_uiLastUsedFrame = 0;
    bool m_bDirty = true;
    bool m_bRequested = false;
    ezInt32 m_iWrapOffsetX = 0; ///< Toroidal wrap period when last rendered, X axis.
    ezInt32 m_iWrapOffsetY = 0; ///< Toroidal wrap period when last rendered, Y axis.
  };

  ezVirtualShadowPageTable();
  ~ezVirtualShadowPageTable();

  void Initialize(ezUInt32 uiPagesPerLevel, ezUInt32 uiNumLevels, ezUInt32 uiMaxPhysicalPages);
  void Reset();

  /// Mark a virtual page as requested for the current frame.
  void MarkPageRequested(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY, ezUInt32 uiCurrentFrame);

  /// Mark a virtual page as dirty (needs re-rendering).
  void MarkPageDirty(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY);

  /// Allocate physical pages for all requested pages. Returns the list of newly allocated pages.
  void AllocateRequestedPages(ezUInt32 uiCurrentFrame, ezDynamicArray<ezUInt32>& out_newlyAllocated);

  /// Clear the dirty flag for a page after it has been rendered.
  void ClearDirtyFlag(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY);

  /// Store the toroidal wrap offset for a page after rendering.
  void SetWrapOffset(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY, ezInt32 iWrapX, ezInt32 iWrapY);

  /// Get the page info for a given virtual page.
  const PageInfo& GetPageInfo(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY) const;

  /// Total number of physical pages currently allocated.
  ezUInt32 GetNumAllocatedPages() const { return m_uiNumAllocatedPages; }

  ezUInt32 GetPagesPerLevel() const { return m_uiPagesPerLevel; }
  ezUInt32 GetNumLevels() const { return m_uiNumLevels; }

private:
  ezUInt32 GetFlatIndex(ezUInt32 uiLevel, ezUInt32 uiPageX, ezUInt32 uiPageY) const;
  ezUInt16 AllocatePhysicalPage(ezUInt32 uiCurrentFrame);
  void EvictLRUPage(ezUInt32 uiCurrentFrame);

  ezUInt32 m_uiPagesPerLevel = 0;
  ezUInt32 m_uiNumLevels = 0;
  ezUInt32 m_uiMaxPhysicalPages = 0;
  ezUInt32 m_uiNumAllocatedPages = 0;

  ezDynamicArray<PageInfo> m_Pages;        ///< Flat array: level * pagesPerLevel^2 + y * pagesPerLevel + x
  ezDynamicArray<bool> m_PhysicalPageUsed; ///< Whether each physical page slot is in use.
};
