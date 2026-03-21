InputSet
{
	string %Set{"Common"}
	Slot
	{
		string %Name{"CloseApplication"}
		uint16 %Flags{1}
	}
	Slot
	{
		string %Name{"ReloadResources"}
		uint16 %Flags{1}
	}
	Slot
	{
		string %Name{"LeftClick"}
		uint16 %Flags{1}
	}
	Slot
	{
		string %Name{"RightClick"}
		uint16 %Flags{1}
	}
	Slot
	{
		string %Name{"MousePosX"}
		uint16 %Flags{0}
	}
	Slot
	{
		string %Name{"MousePosY"}
		uint16 %Flags{0}
	}
}
InputAction
{
	string %Set{"Common"}
	string %Action{"CloseApplication"}
	string %Slot1{"keyboard_escape"}
	float %Scale1{1}
}
InputAction
{
	string %Set{"Common"}
	string %Action{"ReloadResources"}
	string %Slot1{"keyboard_f4"}
	float %Scale1{1}
}
InputAction
{
	string %Set{"Common"}
	string %Action{"LeftClick"}
	string %Slot1{"mouse_button_0"}
	float %Scale1{1}
}
InputAction
{
	string %Set{"Common"}
	string %Action{"RightClick"}
	string %Slot1{"mouse_button_1"}
	float %Scale1{1}
}
InputAction
{
	string %Set{"Common"}
	string %Action{"MousePosX"}
	string %Slot1{"mouse_position_x"}
	float %Scale1{1}
}
InputAction
{
	string %Set{"Common"}
	string %Action{"MousePosY"}
	string %Slot1{"mouse_position_y"}
	float %Scale1{1}
}
