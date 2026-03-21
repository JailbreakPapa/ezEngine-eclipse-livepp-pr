#pragma once

#include <RazorPlugin/RazorPluginDLL.h>

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Types/UniquePtr.h>

class ezRazorDocument;
class ezRazorStyleSheet;

/// The runtime singleton managing all Razor UI contexts.
///
/// Owns the shared text/rendering state and provides the entry point for creating
/// and updating live UI documents. Instantiated during high-level system startup
/// when the RazorPlugin is loaded.
class EZ_RAZORPLUGIN_DLL ezRazorSystem
{
  EZ_DECLARE_SINGLETON(ezRazorSystem);

public:
  ezRazorSystem();
  ~ezRazorSystem();

  /// Creates a new live document. The caller is responsible for feeding it a compiled template.
  ezRazorDocument* CreateDocument();

  /// Destroys a live document and releases its resources.
  void DestroyDocument(ezRazorDocument* pDocument);

  /// Creates a new stylesheet.
  ezRazorStyleSheet* CreateStyleSheet();

  /// Destroys a stylesheet and releases its resources.
  void DestroyStyleSheet(ezRazorStyleSheet* pStyleSheet);

  ezMutex& GetMutex();

private:
  struct Data;
  ezUniquePtr<Data> m_pData;
};
