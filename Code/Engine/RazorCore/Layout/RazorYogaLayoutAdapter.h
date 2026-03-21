#pragma once

#include <RazorCore/Layout/RazorLayoutAdapter.h>

#include <Foundation/Types/UniquePtr.h>

/// Yoga-backed implementation of the Razor layout adapter.
///
/// Translates ezRazorComputedStyle flex/sizing properties into Yoga node configuration
/// and reads back the computed layout rectangles. Yoga types are fully contained within
/// this class and its .cpp — nothing in Razor's public headers depends on Yoga.
class EZ_RAZORCORE_DLL ezRazorYogaLayoutAdapter final : public ezRazorLayoutAdapter
{
public:
  ezRazorYogaLayoutAdapter();
  ~ezRazorYogaLayoutAdapter();

  void SyncFromDocument(const ezRazorDocument& document) override;
  void ApplyStyles(const ezRazorDocument& document) override;
  void ComputeLayout(const ezVec2& vViewportSize, ezDynamicArray<ezRectFloat>& out_layoutRects) override;
  void Invalidate(ezRazorElementId subtreeRoot) override;

private:
  struct Impl;
  ezUniquePtr<Impl> m_pImpl;
};
