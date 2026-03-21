#include <RazorPlugin/RazorPluginPCH.h>

#include <RazorPlugin/RazorSystem.h>

#include <Foundation/Threading/Mutex.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/Style/RazorStyleSheet.h>

EZ_IMPLEMENT_SINGLETON(ezRazorSystem);

struct ezRazorSystem::Data
{
  ezMutex m_Mutex;
  ezDynamicArray<ezUniquePtr<ezRazorDocument>> m_Documents;
  ezDynamicArray<ezUniquePtr<ezRazorStyleSheet>> m_StyleSheets;
};

ezRazorSystem::ezRazorSystem()
  : m_SingletonRegistrar(this)
{
  m_pData = EZ_DEFAULT_NEW(Data);
}

ezRazorSystem::~ezRazorSystem()
{
  m_pData->m_Documents.Clear();
  m_pData->m_StyleSheets.Clear();
}

ezRazorDocument* ezRazorSystem::CreateDocument()
{
  EZ_LOCK(m_pData->m_Mutex);

  ezUniquePtr<ezRazorDocument> pDoc = EZ_DEFAULT_NEW(ezRazorDocument);
  ezRazorDocument* pRaw = pDoc.Borrow();
  m_pData->m_Documents.PushBack(std::move(pDoc));
  return pRaw;
}

void ezRazorSystem::DestroyDocument(ezRazorDocument* pDocument)
{
  EZ_LOCK(m_pData->m_Mutex);

  for (ezUInt32 i = 0; i < m_pData->m_Documents.GetCount(); ++i)
  {
    if (m_pData->m_Documents[i].Borrow() == pDocument)
    {
      m_pData->m_Documents.RemoveAtAndSwap(i);
      return;
    }
  }
}

ezRazorStyleSheet* ezRazorSystem::CreateStyleSheet()
{
  EZ_LOCK(m_pData->m_Mutex);

  ezUniquePtr<ezRazorStyleSheet> pSheet = EZ_DEFAULT_NEW(ezRazorStyleSheet);
  ezRazorStyleSheet* pRaw = pSheet.Borrow();
  m_pData->m_StyleSheets.PushBack(std::move(pSheet));
  return pRaw;
}

void ezRazorSystem::DestroyStyleSheet(ezRazorStyleSheet* pStyleSheet)
{
  EZ_LOCK(m_pData->m_Mutex);

  for (ezUInt32 i = 0; i < m_pData->m_StyleSheets.GetCount(); ++i)
  {
    if (m_pData->m_StyleSheets[i].Borrow() == pStyleSheet)
    {
      m_pData->m_StyleSheets.RemoveAtAndSwap(i);
      return;
    }
  }
}

ezMutex& ezRazorSystem::GetMutex()
{
  return m_pData->m_Mutex;
}

EZ_STATICLINK_FILE(RazorPlugin, RazorPlugin_RazorSystem);
