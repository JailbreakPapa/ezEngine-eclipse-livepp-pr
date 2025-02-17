#pragma once

#include <Foundation/Basics.h>

#if EZ_ENABLED(EZ_PLATFORM_LINUX) && defined(BUILDSYSTEM_ENABLE_TRACELOGGING_LTTNG_SUPPORT)

#  include <Foundation/FoundationInternal.h>
#  include <Foundation/Logging/Log.h>

EZ_FOUNDATION_INTERNAL_HEADER

class ezETWProvider
{
public:
  ezETWProvider();
  ~ezETWProvider();

  void LogMessage(ezLogMsgType::Enum eventType, ezUInt8 uiIndentation, ezStringView sText);

  static ezETWProvider& GetInstance();
};

#endif
