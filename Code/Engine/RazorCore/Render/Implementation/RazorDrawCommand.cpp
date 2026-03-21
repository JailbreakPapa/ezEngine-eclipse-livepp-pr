#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Render/RazorDrawCommand.h>

#include <Foundation/Algorithm/Sorting.h>

void ezRazorRenderBatch::Clear()
{
  m_Commands.Clear();
  m_GlyphBuffer.Clear();
}

void ezRazorRenderBatch::AddCommand(ezRazorDrawCommand&& cmd)
{
  m_Commands.PushBack(std::move(cmd));
}

void ezRazorRenderBatch::SortByOrder()
{
  m_Commands.Sort([](const ezRazorDrawCommand& a, const ezRazorDrawCommand& b) -> bool
    { return a.m_iSortOrder < b.m_iSortOrder; });
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Render_RazorDrawCommand);
