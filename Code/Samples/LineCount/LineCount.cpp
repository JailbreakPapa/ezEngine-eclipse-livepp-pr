#include <Foundation/Application/Application.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Logging/HTMLWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Strings/PathUtils.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>

// In general it is not possible to have global or static variables that (indirectly) require an allocator.
// If you create a variable that somehow needs to have an allocator, an assert will fail.
// Instead you should initialize such variables dynamically (e.g. make it into a pointer and create it at startup,
// and destroy it before shutdown, to prevent messages about memory leaks).
// However, if you absolutely need a global/static variable and cannot initialize it dynamically, wrap it inside the 'ezStatic'
// template. This will make sure that the variable uses a different allocator, such that it won't be counted as a memory leak
// at shutdown.
ezLogWriter::HTML g_HtmlLog;

struct FileStats
{
  FileStats()
  {
    m_uiFileCount = 0;
    m_uiLines = 0;
    m_uiEmptyLines = 0;
    m_uiBytes = 0;
    m_uiCharacters = 0;
    m_uiWords = 0;
  }

  void operator+=(const FileStats& rhs)
  {
    m_uiFileCount += rhs.m_uiFileCount;
    m_uiLines += rhs.m_uiLines;
    m_uiEmptyLines += rhs.m_uiEmptyLines;
    m_uiBytes += rhs.m_uiBytes;
    m_uiCharacters += rhs.m_uiCharacters;
    m_uiWords += rhs.m_uiWords;
  }

  ezUInt32 m_uiFileCount;
  ezUInt32 m_uiLines;
  ezUInt32 m_uiEmptyLines;
  ezUInt32 m_uiBytes;
  ezUInt32 m_uiCharacters;
  ezUInt32 m_uiWords;
};

ezResult ReadCompleteFile(const char* szFile, ezDynamicArray<ezUInt8>& out_fileContent)
{
  out_fileContent.Clear();

  ezFileReader File;
  if (File.Open(szFile) == EZ_FAILURE)
    return EZ_FAILURE;

  ezUInt8 uiTemp[1024];
  while (true)
  {
    const ezUInt64 uiRead = File.ReadBytes(uiTemp, 1023);

    if (uiRead == 0)
      return EZ_SUCCESS; // file is automatically closed here

    out_fileContent.PushBackRange(ezArrayPtr<ezUInt8>(uiTemp, (ezUInt32)uiRead));
  }

  return EZ_SUCCESS; // file is automatically closed here
}

// Removes all spaces and tabs from the front and end of a line
void TrimWhitespaces(ezStringBuilder& ref_sLine)
{
  bool b = true;

  while (b)
  {
    b = false;

    if (ref_sLine.EndsWith(" "))
    {
      b = true;
      ref_sLine.Shrink(0, 1);
    }
    if (ref_sLine.EndsWith("\t"))
    {
      b = true;
      ref_sLine.Shrink(0, 1);
    }
    if (ref_sLine.StartsWith(" "))
    {
      b = true;
      ref_sLine.Shrink(1, 0);
    }
    if (ref_sLine.StartsWith("\t"))
    {
      b = true;
      ref_sLine.Shrink(1, 0);
    }
  }
}

FileStats GetFileStats(const char* szFile)
{
  FileStats s;

  ezDynamicArray<ezUInt8> FileContent;
  if (ReadCompleteFile(szFile, FileContent) == EZ_FAILURE)
    return s;

  FileContent.PushBack('\0');

  if (!ezUnicodeUtils::IsValidUtf8((const char*)&FileContent[0]))
  {
    ezLog::Warning("File is not valid Utf-8: '{0}'", szFile);
    return s;
  }

  // We should not append that directly at the ezStringBuilder, as the file read operations may end
  // in between a Utf8 sequence and then ezStringBuilder will complain about invalid Utf8 strings.
  ezStringBuilder sContent = (const char*)&FileContent[0];

  // count the number of lines
  {
    ezDynamicArray<ezString> Lines;
    sContent.ReplaceAll("\r", ""); // remove carriage return

    // splits the string at occurrence of '\n' and adds each line to the 'Lines' container
    sContent.Split(true, Lines, "\n");

    ezStringBuilder sLine;

    for (ezUInt32 l = 0; l < Lines.GetCount(); ++l)
    {
      sLine = Lines[l].GetData();

      TrimWhitespaces(sLine);

      if (sLine.IsEmpty())
        ++s.m_uiEmptyLines;
      else
      {
        ++s.m_uiLines;

        ezStringView LineIt = sLine;

        bool bIsInWord = false;
        while (!LineIt.IsEmpty())
        {
          const bool bNewWord = ezStringUtils::IsIdentifierDelimiter_C_Code(LineIt.GetCharacter());

          if (bIsInWord != bNewWord)
          {
            // count every whole word as one word and everything in between as another word
            ++s.m_uiWords;
            bIsInWord = bNewWord;
          }

          ++LineIt;
        }
      }
    }
  }

  s.m_uiBytes += sContent.GetElementCount();
  s.m_uiCharacters += sContent.GetCharacterCount();

  return s;
}


class ezLineCountApp : public ezApplication
{
private:
  ezString m_sSearchDir;

public:
  using SUPER = ezApplication;

  ezLineCountApp()
    : ezApplication("LineCountApp")
  {
    m_sSearchDir = "";
  }

  virtual void AfterCoreSystemsStartup() override
  {
    auto pCmd = ezCommandLineUtils::GetGlobalInstance();

    // pass the absolute path to the directory that should be scanned as the first parameter to this application
    if (pCmd->GetParameterCount() > 1)
      m_sSearchDir = ezCommandLineUtils::GetGlobalInstance()->GetParameter(1);

    if (m_sSearchDir.IsEmpty())
    {
      ezStringBuilder sEzCode = ezFileSystem::GetSdkRootDirectory();
      sEzCode.AppendPath("Code");
      sEzCode.MakeCleanPath();

      m_sSearchDir = sEzCode;
    }

    ezLog::Info("Search-dir: {}", m_sSearchDir);

    // Then add a folder as a data directory (the previously registered Factory will take care of creating the proper handler)
    // As we only need access to files through global paths, we add the "empty data directory"
    // This data dir will manage all accesses through absolute paths, unless any other data directory can handle them
    // since we don't add any further data dirs, this is it
    ezFileSystem::AddDataDirectory("", "", ":", ezFileSystem::AllowWrites).IgnoreResult();


    // now we can set up the logging system (we could do it earlier, but the HTML writer needs access to the file system)

    ezStringBuilder sLogPath = m_sSearchDir;
    sLogPath.PathParentDirectory(); // go one folder up
    sLogPath.AppendPath("CodeStatistics.htm");

    // The console log writer will pass all log messages to the standard console window
    ezGlobalLog::AddLogWriter(ezLogWriter::Console::LogMessageHandler);
    // The Visual Studio log writer will pass all messages to the output window in VS
    ezGlobalLog::AddLogWriter(ezLogWriter::VisualStudio::LogMessageHandler);
    // The HTML log writer will write all log messages to an HTML file
    g_HtmlLog.BeginLog(sLogPath.GetData(), "Code Statistics");
    ezGlobalLog::AddLogWriter(ezLoggingEvent::Handler(&ezLogWriter::HTML::LogMessageHandler, &g_HtmlLog));
  }

  virtual void BeforeCoreSystemsShutdown() override
  {
    // close the HTML log, from now on no more log messages are written to the file
    g_HtmlLog.EndLog();
  }

  virtual ezApplication::Execution Run() override
  {
#if EZ_ENABLED(EZ_SUPPORTS_FILE_ITERATORS) || defined(EZ_DOCS)

    ezUInt32 uiDirectories = 0;
    ezUInt32 uiFiles = 0;
    ezMap<ezString, FileStats> FileTypeStatistics;

    // get a directory iterator for the search directory
    ezFileSystemIterator it;
    it.StartSearch(m_sSearchDir);

    if (it.IsValid())
    {
      ezStringBuilder b, sExt;

      // while there are additional files / folders
      for (; it.IsValid(); it.Next())
      {
        // build the absolute path to the current file
        b = it.GetCurrentPath();
        b.AppendPath(it.GetStats().m_sName.GetData());

        // log some info
        ezLog::Info("{0}: {1}", it.GetStats().m_bIsDirectory ? "Directory" : "File", b);

        if (it.GetStats().m_bIsDirectory)
        {
          ++uiDirectories;
        }
        else
        {
          // file extensions are always converted to lower-case actually
          sExt = b.GetFileExtension();

          if (sExt.IsEqual_NoCase("cpp") || sExt.IsEqual_NoCase("h") || sExt.IsEqual_NoCase("hpp") || sExt.IsEqual_NoCase("inl"))
          {
            ++uiFiles;

            // get additional stats and add them to the overall stats
            FileStats& TypeStats = FileTypeStatistics[sExt.GetData()];
            ++TypeStats.m_uiFileCount;

            TypeStats += GetFileStats(b.GetData());
          }
        }
      }

      // now output some statistics
      ezLog::Info("Directories: {0}, Files: {1}, Avg. Files per Dir: {2}", uiDirectories, uiFiles, ezArgF(uiFiles / (float)uiDirectories, 1));

      FileStats AllTypes;

      // iterate over all elements in the amp
      ezMap<ezString, FileStats>::Iterator MapIt = FileTypeStatistics.GetIterator();
      while (MapIt.IsValid())
      {
        ezLog::Info("File Type: '{0}': {1} Files, {2} Lines, {3} Empty Lines, Bytes: {4}, Non-ASCII Characters: {5}, Words: {6}", MapIt.Key(), MapIt.Value().m_uiFileCount, MapIt.Value().m_uiLines, MapIt.Value().m_uiEmptyLines, MapIt.Value().m_uiBytes,
          MapIt.Value().m_uiBytes - MapIt.Value().m_uiCharacters, MapIt.Value().m_uiWords);

        AllTypes += MapIt.Value();

        ++MapIt;
      }

      ezLog::Info("File Type: '{0}': {1} Files, {2} Lines, {3} Empty Lines, All Lines: {4}, Bytes: {5}, Non-ASCII Characters: {6}, Words: {7}", "all", AllTypes.m_uiFileCount, AllTypes.m_uiLines, AllTypes.m_uiEmptyLines, AllTypes.m_uiLines + AllTypes.m_uiEmptyLines, AllTypes.m_uiBytes,
        AllTypes.m_uiBytes - AllTypes.m_uiCharacters, AllTypes.m_uiWords);
    }
    else
    {
      ezLog::Error("Could not search the directory '{0}'", m_sSearchDir);
    }
#else
    EZ_REPORT_FAILURE("No file system iterator support, LineCount sample can't run.");
#endif
    return ezApplication::Execution::Quit;
  }
};


EZ_CONSOLEAPP_ENTRY_POINT(ezLineCountApp);
