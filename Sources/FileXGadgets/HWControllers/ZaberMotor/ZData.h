#pragma once
#include <map>
#include <string>

using namespace std;


static enum MICROSTEP_RESOLUT
{
	mcrStpRes_001 = 1,
	mcrStpRes_002 = mcrStpRes_001 << 1,
	mcrStpRes_004 = mcrStpRes_002 << 1,
	mcrStpRes_008 = mcrStpRes_004 << 1,
	mcrStpRes_016 = mcrStpRes_008 << 1,
	mcrStpRes_032 = mcrStpRes_016 << 1,
	mcrStpRes_064 = mcrStpRes_032 << 1,
	mcrStpRes_128 = mcrStpRes_064 << 1
};

static enum COMMANDS
{
	cmndReset                =   0,
	cmndHome                 =   1,
	cmndRenumber             =   2,
	cmndStoreCurrPos         =  16,
	cmndReturnStoredPos      =  17,
	cmndMove2StorePos        =  18,
	cmndMoveAbsolute         =  20,
	cmndMoveRelative         =  21,
	cmndMoveAtConstSpeed     =  22,
	cmndStop                 =  23,
	cmndReadWriteMemory      =  35,
	cmndRestoreSettings      =  36,

	cmndSetMicrostepRes      =  37,
	cmndSetRunningCurrent    =  38,
	cmndSetHoldCurrent       =  39,
	cmndSetDeviceMode        =  40,
	cmndSetHomeSpeed         =  41,
	cmndSetTargetSpeed       =  42,
	cmndSetAcceleration      =  43,
	cmndSetMaxPos            =  44,
	cmndSetCurrentPos        =  45,
	cmndSetMaxRelMove        =  46,
	cmndSetHomeOffset        =  47,
	cmndSetAliasNumber       =  48,
	cmndSetLockState         =  49,

	cmndReturnDeviceId       =  50,
	cmndReturnFWVer          =  51,
	cmndReturnPowrSupplyVolt =  52,
	cmndReturnSettings       =  53,
	cmndReturnStatus         =  54,  //TODO add enum of statuses page 33
	cmndEchoData             =  55,
	cmndReturnCurrentPos     =  60,
  cmndSetHomeStatus        = 103,

	//Read-Only commands
	cmndMoveTrac             =   8,
	cmndLimitActive          =   9,
	cmndManualMoveTrac       =  10,
	cmndError                = 255
};

 typedef map<COMMANDS, string>  COMMANDS_NAMES_MAP;
 
 static COMMANDS_NAMES_MAP COMMANDS_NAMES;

	//Read-Only commands
static enum STATES //reaction to the (cmndReturnStatus = 54)
{	
	stte_Idle                  =  0,
	stte_ExecHomeInstrct       =  1,
	stte_ManualKnobeTurned     = 10,
	stte_MoveAbsInstrct        = 20,
	stte_MoveRelInstrct        = 21,
	stte_MoveConstSpeedInstrct = 22,
	stte_StopInstrct           = 23
};

//typedef std::map<STATES , const string>  STATES_DESCR_MAP;
typedef map<STATES , const string>  STATES_DESCR_MAP;

static STATES_DESCR_MAP STATES_DESCR;

static enum ERROR_CODES
{
	errCannotHome             =   cmndHome,
	errInvalidDevNum          =   cmndRenumber,
	errVoltageLow             =   14,
	errVoltageHigh            =   15,
	errInvalidStotedPos       =   cmndMove2StorePos,
	errInvalidAbsPos          =   cmndMoveAbsolute,
	errInvalidRelativePos     =   cmndMoveRelative,
	errInvalidVelocity        =   cmndMoveAtConstSpeed,
	errInvalidPeripherialId   =   36,
	errInvalidResolution      =   cmndSetMicrostepRes,
	errInvalidRunCurrent      =   cmndSetRunningCurrent,
	errInvalidHoldCurrent     =   cmndSetHoldCurrent,
	errInvalidMode            =   cmndSetDeviceMode,
	errInvalidHomeSpeed       =   cmndSetHomeSpeed,
	errInvalidSpeed           =   cmndSetTargetSpeed,
	errInvalidAcceleration    =   cmndSetAcceleration,
	errInvalidMaximumRange    =   cmndSetMaxPos,
	errInvalidCurrentPos      =   cmndSetCurrentPos,
	errInvalidMaxRelMove      =   cmndSetMaxRelMove,
	errInvalidOffset          =   cmndSetHomeOffset,
	errInvalidAlias           =   cmndSetAliasNumber,
	errInvalidLockState       =   cmndSetLockState,
	errUnknownDevID           =   cmndReturnDeviceId,
	errInvalidSetting         =   cmndReturnSettings,
	errInvalidCommand         =   64,
	errBusy                   =  255,
	errInvalidSavePos         = 1600,
	errSavePosNotHomed        = 1601,
	errInvalidReturnPos       = 1700,
	errInvalidMovePos         = 1800,
	errMovePosNotHomed        = 1801,
	errRelativePosLimited     = 2146,
	errSettingsLocked         = 3600,
	errInvalidDisableAutoHome = 4008,
	errInvalidBit10           = 4010,
	errInvalidHomeSwitch      = 4012,
	errInvalidBit13           = 4013
};

typedef struct _errorDescriptor{
	int    code;
	string name;
	string description;

	_errorDescriptor(int c=0, const string& n="", const string& descr=""):code(c),name(n),description(descr){}
}ZErrorDescriptor;

typedef std::map<ERROR_CODES, ZErrorDescriptor>  ERROR_DESCR_MAP;

static ERROR_DESCR_MAP ERROR_DESCR;


static enum SETTINGS
{
  settng_DisableAutoReplly				  = 1,
  settng_EnableAntiBacklash				  = settng_DisableAutoReplly				<< 1,
  settng_EnableAntiSticktion			  = settng_EnableAntiBacklash				<< 1,
  settng_DisablePotentiometer			  = settng_EnableAntiSticktion			    << 1,
  settng_EnableMoveTracking				  = settng_DisablePotentiometer			    << 1,
  settng_DisableManualMoveTraking	      = settng_EnableMoveTracking				<< 1,
  settng_EnableMessageIds				  = settng_DisableManualMoveTraking	        << 1,
  settng_HomeStatus						  = settng_EnableMessageIds				    << 1,
  settng_DisableAutoHome				  = settng_HomeStatus						<< 1,
  settng_ReversePotentiometer			  = settng_DisableAutoHome				    << 1,
  settng_Reserved_10					  = settng_ReversePotentiometer			    << 1,
  settng_EnableCircularPhaseMicrostepping = settng_Reserved_10					    << 1,
  settng_SetHomeSwitchLogic               = settng_EnableCircularPhaseMicrostepping << 1,
  settng_Reserved_13					  = settng_SetHomeSwitchLogic  			    << 1,
  settng_DsablePowerLED                   = settng_Reserved_13                      << 1,
  settng_DsableSerialLED                  = settng_DsablePowerLED                   << 1
};


static bool init_commands_names_map(COMMANDS_NAMES_MAP &m)
{
	m[cmndReset				  ] = "Reset";
	m[cmndHome				  ] = "Home";
	m[cmndRenumber			  ] = "Renumber";
	m[cmndStoreCurrPos        ] = "Store_Current_Position";
	m[cmndReturnStoredPos     ] = "Return_Stored_Position";
	m[cmndMove2StorePos       ] = "Move_To_Stored_Position";
	m[cmndMoveAbsolute        ] = "Move_Absolute";
	m[cmndMoveRelative        ] = "Move_Relative";
	m[cmndMoveAtConstSpeed    ] = "Move_At_Constant_Speed";
	m[cmndStop                ] = "Stop";
	m[cmndReadWriteMemory     ] = "Read_Write_Memory";
	m[cmndRestoreSettings     ] = "Restore_Settings";

	m[cmndSetMicrostepRes     ] = "Set_Microstep_Resolution";
	m[cmndSetRunningCurrent   ] = "Set_Running_Current";
	m[cmndSetHoldCurrent      ] = "Set_Hold_Current";
	m[cmndSetDeviceMode       ] = "Set_Device_Mode";
	m[cmndSetHomeSpeed        ] = "Set_Home_Speed";
	m[cmndSetTargetSpeed      ] = "Set_Target_Speed";
	m[cmndSetAcceleration     ] = "Set_Acceleration";
	m[cmndSetMaxPos           ] = "Set_Maximum_Position";
	m[cmndSetCurrentPos       ] = "Set_Current_Position";
	m[cmndSetMaxRelMove       ] = "Set_Maximum_Relative_Move";
	m[cmndSetHomeOffset       ] = "Set_Home_Offset";
	m[cmndSetAliasNumber      ] = "Set_Alias_Number";
	m[cmndSetLockState        ] = "Set_Lock_State";

	m[cmndReturnDeviceId      ] = "Return_DeviceID";
	m[cmndReturnFWVer         ] = "Return_Firmware_Ver.";
	m[cmndReturnPowrSupplyVolt] = "Return_Power_Supply_Votage";
	m[cmndReturnSettings      ] = "Return_Settings";
	m[cmndReturnStatus        ] = "Return_Status";
	m[cmndEchoData            ] = "Echo_Data";
	m[cmndReturnCurrentPos    ] = "Return_Current_Position";
  m[ cmndSetHomeStatus ]      = "Set_Home_Status" ;

	m[cmndMoveTrac            ] = "Move_Tracking";
	m[cmndLimitActive   	  ] = "Limit_Active";
	m[cmndManualMoveTrac	  ] = "Manual_Move_Tracking";
	m[cmndError         	  ] = "Error";
	return true;
}

static bool init_state_descr_map(STATES_DESCR_MAP &m)
{
	m[stte_Idle                 ] = "idle, not currently executing any instructions";
	m[stte_ExecHomeInstrct      ] = "executing a home instruction";
	m[stte_ManualKnobeTurned    ] = "executing a manual move (i.e. the manual control knob is turned)";
	m[stte_MoveAbsInstrct       ] = "executing a move absolute instruction";
	m[stte_MoveRelInstrct       ] = "executing a move relative instruction";
	m[stte_MoveConstSpeedInstrct] = "executing a move at constant speed instruction";
	m[stte_StopInstrct          ] = "executing a stop instruction (i.e. decelerating)";

	return true;
}

inline static bool init_error_descr_map(ERROR_DESCR_MAP &m)
{
	m[errCannotHome            ] = ZErrorDescriptor(errCannotHome            , "Cannot Home"                  , "Home - Device has traveled a long distance without triggering the home sensor. Device may be stalling or slipping.");
	m[errInvalidDevNum         ] = ZErrorDescriptor(errInvalidDevNum         , "Device Number Invalid"        , "Renumbering data out of range. Data (Device number) must be between 1 and 254 inclusive.");
	m[errVoltageLow            ] = ZErrorDescriptor(errVoltageLow            , "Voltage Low"                  , "Power supply voltage too low.");
	m[errVoltageHigh           ] = ZErrorDescriptor(errVoltageHigh           , "Voltage High"                 , "Power supply voltage too high.");
	m[errInvalidStotedPos      ] = ZErrorDescriptor(errInvalidStotedPos      , "Stored Position Invalid"      , "The position stored in the requested register is no longer valid. This is probably because the maximum range was reduced.");
	m[errInvalidAbsPos         ] = ZErrorDescriptor(errInvalidAbsPos         , "Absolute Position Invalid"    , "Move Absolute - Target position out of range.");
	m[errInvalidRelativePos    ] = ZErrorDescriptor(errInvalidRelativePos    , "Relative Position Invalid"    , "Move Relative - Target position out of range.");
	m[errInvalidVelocity       ] = ZErrorDescriptor(errInvalidVelocity       , "Velocity Invalid"             , "Constant velocity move. Velocity out of range.");
	m[errInvalidPeripherialId  ] = ZErrorDescriptor(errInvalidPeripherialId  , "Peripheral Id Invalid"        , "Restore Settings - peripheral id is invalid. Please use one of the peripheral ids listed in the user manual, or 0 for default.");
	m[errInvalidResolution     ] = ZErrorDescriptor(errInvalidResolution     , "Resolution Invalid"           , "Invalid micro step resolution. Resolution may only be 1, 2, 4, 8, 16, 32, 64, 128.");
	m[errInvalidRunCurrent     ] = ZErrorDescriptor(errInvalidRunCurrent     , "Run Current Invalid"          , "Run current out of range. See command 38 for allowable values.");
	m[errInvalidHoldCurrent    ] = ZErrorDescriptor(errInvalidHoldCurrent    , "Hold Current Invalid"         , "Hold current out of range. See command 39 for allowable values.");
	m[errInvalidMode           ] = ZErrorDescriptor(errInvalidMode           , "Mode Invalid"                 , "Set Device Mode - one or more of the mode bits is invalid.");
	m[errInvalidHomeSpeed      ] = ZErrorDescriptor(errInvalidHomeSpeed      , "Home Speed Invalid"           , "Home speed out of range. The range of home speed is determined by the	resolution.");
	m[errInvalidSpeed          ] = ZErrorDescriptor(errInvalidSpeed          , "Speed Invalid"                , "Target speed out of range. The range of target speed is determined by the resolution.");
	m[errInvalidAcceleration   ] = ZErrorDescriptor(errInvalidAcceleration   , "Acceleration Invalid"         , "Target acceleration out of range. The range of target acceleration is determined by the resolution.");
	m[errInvalidMaximumRange   ] = ZErrorDescriptor(errInvalidMaximumRange   , "Maximum Range Invalid"        , "The maximum range may only be set between 1 and the resolution limit of the stepper controller, which is 16,777,215.");
	m[errInvalidCurrentPos     ] = ZErrorDescriptor(errInvalidCurrentPos     , "Current Position Invalid"     , "Current position out of range. Current position must be between 0 and the	maximum range.");
	m[errInvalidMaxRelMove     ] = ZErrorDescriptor(errInvalidMaxRelMove     , "Maximum Relative Move Invalid", "Max relative move out of range. Must be between 0 and 16,777,215.");
	m[errInvalidOffset         ] = ZErrorDescriptor(errInvalidOffset         , "Offset Invalid"               , "Home offset out of range. Home offset must be between 0 and maximum range.");
	m[errInvalidAlias          ] = ZErrorDescriptor(errInvalidAlias          , "Alias Invalid"                , "Alias out of range. Alias must be between 0 and 254 inclusive.");
	m[errInvalidLockState      ] = ZErrorDescriptor(errInvalidLockState      , "Lock State Invalid"           , "Lock state must be 1 (locked) or 0 (unlocked).");
	m[errUnknownDevID          ] = ZErrorDescriptor(errUnknownDevID          , "Device Id Unknown"            , "The device id is not included in the firmware's list.");
	m[errInvalidSetting        ] = ZErrorDescriptor(errInvalidSetting        , "Setting Invalid"              , "Return Setting - data entered is not a valid setting command number. Valid setting command numbers are the command numbers of any \"Set ...\" instructions.");
	m[errInvalidCommand        ] = ZErrorDescriptor(errInvalidCommand        , "Command Invalid"              , "Command number not valid in this firmware version.");
	m[errBusy                  ] = ZErrorDescriptor(errBusy                  , "Busy"                         , "Another command is executing and cannot be pre-empted. Either stop the previous command or wait until it finishes before trying again.");
	m[errInvalidSavePos        ] = ZErrorDescriptor(errInvalidSavePos        , "Save Position Invalid"        , "Save Current Position register out of range (must be 0-15).");
	m[errSavePosNotHomed       ] = ZErrorDescriptor(errSavePosNotHomed       , "Save Position Not Homed"      , "Save Current Position is not allowed unless the device has been homed.");
	m[errInvalidReturnPos      ] = ZErrorDescriptor(errInvalidReturnPos      , "Return Position Invalid"      , "Return Stored Position register out of range (must be 0-15).");
	m[errInvalidMovePos        ] = ZErrorDescriptor(errInvalidMovePos        , "Move Position Invalid"        , "Move to Stored Position register out of range (must be 0-15).");
	m[errMovePosNotHomed       ] = ZErrorDescriptor(errMovePosNotHomed       , "Move Position Not Homed"      , "Move to Stored Position is not allowed unless the device has been homed.");
	m[errRelativePosLimited    ] = ZErrorDescriptor(errRelativePosLimited    , "Relative Position Limited"    , "Move Relative (command 20) exceeded maximum relative move range. Either move a shorter distance, or change the maximum relative move (command 46).");
	m[errSettingsLocked        ] = ZErrorDescriptor(errSettingsLocked        , "Settings Locked"              , "Must clear Lock State (command 49) first. See the Set Lock State command for details.");
	m[errInvalidDisableAutoHome] = ZErrorDescriptor(errInvalidDisableAutoHome, "Disable Auto Home Invalid"    , "Set Device Mode - this is a linear actuator; Disable Auto Home is used for	rotary actuators only.");
	m[errInvalidBit10          ] = ZErrorDescriptor(errInvalidBit10          , "Bit 10"                       , "Invalid Set Device Mode - bit 10 is reserved and must be 0.");
	m[errInvalidHomeSwitch     ] = ZErrorDescriptor(errInvalidHomeSwitch     , "Home Switch Invalid"          , "Set Device Mode - this device has integrated home sensor with preset polarity; mode bit 12 cannot be changed by the user.");
	m[errInvalidBit13          ] = ZErrorDescriptor(errInvalidBit13          , "Bit 13"                       , "Invalid Set Device Mode - bit 13 is reserved and must be 0.");
	return m.size() > 0;
}