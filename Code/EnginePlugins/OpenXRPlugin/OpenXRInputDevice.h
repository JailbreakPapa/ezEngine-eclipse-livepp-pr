#pragma once

#include <OpenXRPlugin/Basics.h>
#include <OpenXRPlugin/OpenXRIncludes.h>

#include <GameEngine/XR/XRInputDevice.h>
#include <GameEngine/XR/XRInterface.h>

class ezOpenXR;

EZ_DEFINE_AS_POD_TYPE(XrActionSuggestedBinding);
EZ_DEFINE_AS_POD_TYPE(XrActiveActionSet);

class EZ_OPENXRPLUGIN_DLL ezOpenXRInputDevice : public ezXRInputDevice
{
  EZ_ADD_DYNAMIC_REFLECTION(ezOpenXRInputDevice, ezXRInputDevice);

public:
  void GetDeviceList(ezHybridArray<ezXRDeviceID, 64>& out_devices) const override;
  ezXRDeviceID GetDeviceIDByType(ezXRDeviceType::Enum type) const override;
  const ezXRDeviceState& GetDeviceState(ezXRDeviceID deviceID) const override;
  ezString GetDeviceName(ezXRDeviceID deviceID) const override;
  ezBitflags<ezXRDeviceFeatures> GetDeviceFeatures(ezXRDeviceID deviceID) const override;

private:
  friend class ezOpenXR;
  struct Bind
  {
    XrAction action;
    const char* szPath;
  };

  struct Action
  {
    ezXRDeviceFeatures::Enum m_Feature;
    XrAction m_Action;
    ezString m_sKey[2];
  };

  struct Vec2Action
  {
    Vec2Action(ezXRDeviceFeatures::Enum feature, XrAction pAction, ezStringView sLeft, ezStringView sRight);
    ezXRDeviceFeatures::Enum m_Feature;
    XrAction m_Action;
    ezString m_sKey_negx[2];
    ezString m_sKey_posx[2];
    ezString m_sKey_negy[2];
    ezString m_sKey_posy[2];
  };

  ezOpenXRInputDevice(ezOpenXR* pOpenXR);
  XrResult CreateActions(XrSession session, XrSpace m_sceneSpace);
  void DestroyActions();

  XrPath CreatePath(const char* szPath);
  XrResult CreateAction(ezXRDeviceFeatures::Enum feature, const char* actionName, XrActionType actionType, XrAction& out_action);
  XrResult SuggestInteractionProfileBindings(const char* szInteractionProfile, const char* szNiceName, ezArrayPtr<Bind> bindings);
  XrResult AttachSessionActionSets(XrSession session);
  XrResult UpdateCurrentInteractionProfile();

  void InitializeDevice() override;
  void RegisterInputSlots() override;
  void UpdateInputSlotValues() override {}

  XrResult UpdateActions();
  void UpdateControllerState();

private:
  ezOpenXR* m_pOpenXR = nullptr;
  XrInstance m_pInstance = XR_NULL_HANDLE;
  XrSession m_pSession = XR_NULL_HANDLE;

  ezXRDeviceState m_DeviceState[3]; // Hard-coded for now
  ezString m_sActiveProfile[3];
  ezBitflags<ezXRDeviceFeatures> m_SupportedFeatures[3];
  const ezInt8 m_iLeftControllerDeviceID = 1;
  const ezInt8 m_iRightControllerDeviceID = 2;

  XrActionSet m_pActionSet = XR_NULL_HANDLE;
  ezHashTable<XrPath, ezString> m_InteractionProfileToNiceName;

  ezStaticArray<const char*, 2> m_SubActionPrefix;
  ezStaticArray<XrPath, 2> m_SubActionPath;

  ezHybridArray<Action, 4> m_BooleanActions;
  ezHybridArray<Action, 4> m_FloatActions;
  ezHybridArray<Vec2Action, 4> m_Vec2Actions;
  ezHybridArray<Action, 4> m_PoseActions;

  XrSpace m_gripSpace[2];
  XrSpace m_aimSpace[2];
};
