#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

namespace
{
  static ezMutex s_ProfilingMutex;
  static ezProfilingSystem::ProfilingData s_CapturedData;
  static bool s_bCaptureRequested = false;
  static bool s_bDataReady = false;
} // namespace

static void SendProfilingData()
{
  EZ_LOCK(s_ProfilingMutex);

  if (!s_bDataReady)
    return;

  if (!ezTelemetry::IsConnectedToClient())
  {
    s_bDataReady = false;
    return;
  }

  ezTelemetryMessage Msg;
  Msg.SetMessageID('PROF', 'DATA');

  // Write frame count
  Msg.GetWriter() << s_CapturedData.m_uiFrameCount;

  // Write thread data - limit to prevent huge messages
  const ezUInt32 maxThreads = 16;
  const ezUInt32 maxScopesPerThread = 5000;

  ezUInt32 uiThreadCount = ezMath::Min(static_cast<ezUInt32>(s_CapturedData.m_AllEventBuffers.GetCount()), maxThreads);
  Msg.GetWriter() << uiThreadCount;

  for (ezUInt32 t = 0; t < uiThreadCount; ++t)
  {
    const auto& buffer = s_CapturedData.m_AllEventBuffers[t];

    // Find thread name
    ezString threadName = "Unknown Thread";
    for (const auto& info : s_CapturedData.m_ThreadInfos)
    {
      if (info.m_uiThreadId == buffer.m_uiThreadId)
      {
        threadName = info.m_sName;
        break;
      }
    }

    Msg.GetWriter() << buffer.m_uiThreadId;
    Msg.GetWriter() << threadName;

    ezUInt32 uiScopeCount = ezMath::Min(static_cast<ezUInt32>(buffer.m_Data.GetCount()), maxScopesPerThread);
    Msg.GetWriter() << uiScopeCount;

    for (ezUInt32 s = 0; s < uiScopeCount; ++s)
    {
      const auto& scope = buffer.m_Data[s];

      Msg.GetWriter() << scope.m_BeginTime;
      Msg.GetWriter() << scope.m_EndTime;
      Msg.GetWriter() << ezString(scope.m_szName);
      Msg.GetWriter() << ezString(scope.m_szFunctionName ? scope.m_szFunctionName : "");
    }
  }

  // Write GPU scopes
  const ezUInt32 maxGPUScopes = 1000;
  ezUInt32 totalGPUScopes = 0;
  for (const auto& gpuScopes : s_CapturedData.m_GPUScopes)
  {
    totalGPUScopes += static_cast<ezUInt32>(gpuScopes.GetCount());
  }
  totalGPUScopes = ezMath::Min(totalGPUScopes, maxGPUScopes);
  Msg.GetWriter() << totalGPUScopes;

  ezUInt32 writtenGPUScopes = 0;
  for (const auto& gpuScopes : s_CapturedData.m_GPUScopes)
  {
    for (const auto& scope : gpuScopes)
    {
      if (writtenGPUScopes >= maxGPUScopes)
        break;

      Msg.GetWriter() << scope.m_BeginTime;
      Msg.GetWriter() << scope.m_EndTime;
      Msg.GetWriter() << ezString(scope.m_szName);

      ++writtenGPUScopes;
    }
    if (writtenGPUScopes >= maxGPUScopes)
      break;
  }

  // Write frame start times - limit to reasonable amount
  const ezUInt32 maxFrames = 256;
  ezUInt32 uiFrameStartCount = ezMath::Min(static_cast<ezUInt32>(s_CapturedData.m_FrameStartTimes.GetCount()), maxFrames);
  Msg.GetWriter() << uiFrameStartCount;

  for (ezUInt32 f = 0; f < uiFrameStartCount; ++f)
  {
    Msg.GetWriter() << s_CapturedData.m_FrameStartTimes[f];
  }

  ezTelemetry::Broadcast(ezTelemetry::Reliable, Msg);

  // Clear after sending
  s_CapturedData.Clear();
  s_bDataReady = false;
}

static void ProfilingTelemetryHandler(void* pUserData)
{
  ezTelemetryMessage Msg;

  while (ezTelemetry::RetrieveMessage('PROF', Msg) == EZ_SUCCESS)
  {
    if (Msg.GetMessageID() == ' REQ')
    {
      EZ_LOCK(s_ProfilingMutex);
      s_bCaptureRequested = true;
    }
  }

  // Check if capture was requested - do it here (outside message loop) to avoid blocking
  {
    bool bDoCapture = false;
    {
      EZ_LOCK(s_ProfilingMutex);
      if (s_bCaptureRequested && !s_bDataReady)
      {
        s_bCaptureRequested = false;
        bDoCapture = true;
      }
    }

    if (bDoCapture)
    {
      // Capture without holding the mutex to avoid potential deadlocks
      ezProfilingSystem::ProfilingData tempData;
      ezProfilingSystem::Capture(tempData);

      // Now store the captured data
      {
        EZ_LOCK(s_ProfilingMutex);
        s_CapturedData = std::move(tempData);
        s_bDataReady = true;
      }
    }
  }

  // Send data if ready
  SendProfilingData();
}

void AddProfilingEventHandler()
{
  ezTelemetry::AcceptMessagesForSystem('PROF', true, ProfilingTelemetryHandler, nullptr);
}

void RemoveProfilingEventHandler()
{
  ezTelemetry::AcceptMessagesForSystem('PROF', false);

  EZ_LOCK(s_ProfilingMutex);
  s_CapturedData.Clear();
  s_bCaptureRequested = false;
  s_bDataReady = false;
}
