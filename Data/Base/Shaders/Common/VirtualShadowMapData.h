#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

// Page table entry format (stored in R32_UINT texture)
// Bits [0-15]:  Physical page index (65536 pages max)
// Bits [16-23]: Reserved/flags
// Bit  [30]:    Dirty flag (page needs re-rendering)
// Bit  [31]:    Valid bit (1 = page is allocated and has valid shadow data)
#define VSM_PAGE_VALID_BIT      0x80000000u
#define VSM_PAGE_DIRTY_BIT      0x40000000u
#define VSM_PAGE_INDEX_MASK     0x0000FFFFu
#define VSM_PAGE_INVALID        0x00000000u

#define VSM_PAGE_SIZE           128
#define VSM_PHYSICAL_ATLAS_SIZE 8192
#define VSM_PAGES_PER_ATLAS_DIM (VSM_PHYSICAL_ATLAS_SIZE / VSM_PAGE_SIZE)  // 64
#define VSM_MAX_PHYSICAL_PAGES  (VSM_PAGES_PER_ATLAS_DIM * VSM_PAGES_PER_ATLAS_DIM)  // 4096
#define VSM_MAX_CLIPMAP_LEVELS  8
#define VSM_VIRTUAL_RESOLUTION  2048
#define VSM_PAGES_PER_LEVEL     (VSM_VIRTUAL_RESOLUTION / VSM_PAGE_SIZE)  // 16

#define VSM_PAGE_MARKING_THREAD_X 8
#define VSM_PAGE_MARKING_THREAD_Y 8

CONSTANT_BUFFER(ezVSMConstants, 5)
{
  UINT1(PageSize);
  UINT1(PhysicalAtlasSize);
  UINT1(NumClipmapLevels);
  UINT1(VSMPadding2);

  FLOAT4(ClipmapCenter);

  FLOAT4(ClipmapLevelWorldSize[VSM_MAX_CLIPMAP_LEVELS]);
  MAT4(ClipmapWorldToUV[VSM_MAX_CLIPMAP_LEVELS]);

  FLOAT1(VSMDepthBias);
  FLOAT1(VSMNormalBias);
  UINT1(VSMPadding0);
  UINT1(VSMPadding1);
};

#if EZ_ENABLED(PLATFORM_SHADER)

// Decode a page table entry into a physical atlas UV and validity
bool VSMDecodePage(uint pageEntry, out uint2 physicalPageCoord)
{
  if ((pageEntry & VSM_PAGE_VALID_BIT) == 0)
    return false;

  uint pageIndex = pageEntry & VSM_PAGE_INDEX_MASK;
  physicalPageCoord.x = (pageIndex % VSM_PAGES_PER_ATLAS_DIM) * VSM_PAGE_SIZE;
  physicalPageCoord.y = (pageIndex / VSM_PAGES_PER_ATLAS_DIM) * VSM_PAGE_SIZE;
  return true;
}

// Compute the physical atlas UV from a virtual UV within a clipmap level
float2 VSMVirtualToPhysicalUV(float2 virtualUV, uint2 physicalPageCoord)
{
  // Offset within the page (fractional part of the page coordinate)
  float2 pageUV = frac(virtualUV * float(VSM_PAGES_PER_LEVEL));

  // Physical atlas UV
  float2 atlasUV = (float2(physicalPageCoord) + pageUV * float(VSM_PAGE_SIZE)) / float(VSM_PHYSICAL_ATLAS_SIZE);
  return atlasUV;
}

#endif // PLATFORM_SHADER
