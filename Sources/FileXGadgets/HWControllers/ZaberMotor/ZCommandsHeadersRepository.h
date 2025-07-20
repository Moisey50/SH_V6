#pragma once
#include <set>
#include <map>
#include "ZCommandHeader.h"


using namespace std;

class ZCommandsHeadersRepository
{
public:
	typedef set<ZCommandHeader> RepositorySet;
	
private:
  typedef map<short, ZCommandHeader*> RepositoryMap;
	typedef map<string, ZCommandHeader*> RepositoryMapByName;
	RepositorySet        m_Repository;
	RepositoryMap        m_RepositoryByCmdId;
	RepositoryMapByName  m_RepositoryByCmdName;
	const ZCommandHeader m_emptyCmd;

public:
  RepositorySet& GetRepository()
  {
    return m_Repository;
  }

	ZCommandsHeadersRepository(void)
		: m_Repository()
		, m_RepositoryByCmdId()
		, m_RepositoryByCmdName()
		, m_emptyCmd()
	{
		InitRepository();
	}

	~ZCommandsHeadersRepository(void)
	{
		//for (RepositorySet::const_iterator ci = m_Repository.begin(); ci != m_Repository.end(); ci++)
		//{
		//	delete *ci;
		//}

		m_Repository.clear();
		m_RepositoryByCmdId.clear();
		m_RepositoryByCmdName.clear();
	}

	void Add(
		const string& instructionName = string()
		, const string& summary = string()
		, short id = COMMAND_ID_NONE
		, eCommandType cmdType = CT_NONE
		, eCommandDataType cmdDataType_Send = CDT_NONE
		, eCommandDataType cmdDataType_Reply = CDT_NONE
		, bool isSafe2Retry = false
		, bool isReturnsCurrentPos = false
		, bool isPreEmptive = false
		, int processingDuration = PROCESSING_DURATION_UNDEFINED
		, ePersistenceType persistence = PT_NONE
		)
	{
		ZCommandHeader * pZCmd = ZCommandHeader::CreateCommand
			( instructionName
			, summary
			, id
			, cmdType
			, cmdDataType_Send
			, cmdDataType_Reply
			, isSafe2Retry
			, isReturnsCurrentPos
			, isPreEmptive
			, processingDuration
			, persistence);

		pair<RepositorySet::iterator,bool> res = m_Repository.insert(*pZCmd);
		RepositorySet::iterator data = res.first;
		m_RepositoryByCmdId[(*(data)).GetID()] = (ZCommandHeader*)&(*(data));
		m_RepositoryByCmdName[ZUtils::ToLower((*(data)).GetInstructionName())] = (ZCommandHeader*)&(*(data));

		delete(pZCmd);
	}

	const ZCommandHeader& GetCommandById(short cmdId) const
	{
		RepositoryMap::const_iterator ci = m_RepositoryByCmdId.find(cmdId);
		return ci == m_RepositoryByCmdId.end() ? m_emptyCmd : *(ci->second);
	}

	const ZCommandHeader& GetCommandByName(const string& cmdName) const
	{
		RepositoryMapByName::const_iterator ci = m_RepositoryByCmdName.find(ZUtils::ToLower(cmdName));
		return ci == m_RepositoryByCmdName.end() ? m_emptyCmd : *(ci->second);
	}

	const ZCommandHeader& operator[](short cmdId) const
	{
		return GetCommandById(cmdId);
	}

	const ZCommandHeader& operator[](short cmdId)
	{
		return GetCommandById(cmdId);
	}

	const ZCommandHeader& operator[](const string& cmdName) const
	{
		return GetCommandByName(cmdName);
	}

	const ZCommandHeader& operator[](const string& cmdName)
	{
		return GetCommandByName(cmdName);
	}																																							             
																																								             
	void InitRepository()																																		             
	{																																							             
		Add("Reset",                                  "Sets the device to its power-up condition",                                                                            0, CT_COMMAND,           CDT_NONE,                    CDT_NONE,                    true);
		Add("Home",                                   "Moves to the home position and resets device's internal position",                                                     1, CT_COMMAND,           CDT_NONE,                    CDT_POSITION_FINAL,          true,  true);
		Add("Renumber",                               "Assigns new numbers to all devices in the order in which they are connected",                                          2, CT_COMMAND,           CDT_DEVICE_ID_NEW,           CDT_DEVICE_ID,               true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Read Register",                          "Reads from a register. Do not use this command unless directed to by Zaber technical support.",                        5, CT_COMMAND,           CDT_REGISTER_ADDRESS,        CDT_DATA,                    true); 
		Add("Set Active Register",                    "Sets the active register. Do not use this command unless directed to by Zaber technical support.",                     6, CT_SETTING,           CDT_REGISTER_ADDRESS,        CDT_REGISTER_ADDRESS,        true); 
		Add("Write Register",                         "Writes to a register. Do not use this command unless directed to by Zaber technical support.",                         7, CT_COMMAND,           CDT_DATA,                    CDT_DATA,                    true); 
		Add("Move Tracking",                          "Indicates to the user that the device has been set to a position tracking mode and given a move instruction.",         8, CT_REPLY,             CDT_NONE,                    CDT_POSITION_TRAKING,        true,  true);
		Add("Limit Active",                           "Indicates to the user that the device has reached one of the limits of travel.",                                       9, CT_REPLY,             CDT_NONE,                    CDT_POSITION_FINAL,          true,  true);
		Add("Manual Move Tracking",                   "A reply that is sent when the manual control knob is turned in velocity mode.",                                       10, CT_REPLY,             CDT_NONE,                    CDT_POSITION_TRAKING,        true,  true);
		Add("Manual Move",                            "A knob manual move in Displacement mode has completed.",                                                              11, CT_REPLY,             CDT_NONE,                    CDT_POSITION_FINAL,          true,  true);
		Add("Slip Tracking",                          "A reply that is sent when the device is slipping.",                                                                   12, CT_REPLY,             CDT_NONE,                    CDT_POSITION_TRAKING,        true,  true);
		Add("Unexpected Position",                    "A reply that is sent when the device stops at a position different from the requested location.",                     13, CT_REPLY,             CDT_NONE,                    CDT_POSITION_FINAL,          true,  true);
		Add("Store Current Position",                 "Saves the current absolute position",                                                                                 16, CT_COMMAND,           CDT_ADDRESS,                 CDT_ADDRESS,                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Return Stored Position",                 "Returns the position stored in one of the 16 position registers for the device",                                      17, CT_COMMAND,           CDT_ADDRESS,                 CDT_POSITION_STORED,         true); 
		Add("Move To Stored Position",                "Moves the device to the stored position specified by the Command Data.",                                              18, CT_COMMAND,           CDT_ADDRESS,                 CDT_POSITION_FINAL,          true,  true);
		Add("Move Absolute",                          "Moves the device to the position specified in the Command Data in micro steps.",                                      20, CT_COMMAND,           CDT_POSITION_ABSOLUTE,       CDT_POSITION_FINAL,          true,  true);
		Add("Move Relative",                          "Moves the device by positive or negative number of micro steps specified in the Command Data",                        21, CT_COMMAND,           CDT_POSITION_RELATIVE,       CDT_POSITION_FINAL,          false, true, true);
		Add("Move At Constant Speed",                 "Moves the device at a constant speed based on the value specified in the Command Data.",                              22, CT_COMMAND,           CDT_SPEED,                   CDT_SPEED,                   true);
		Add("Stop",                                   "Stops the device from moving by preempting any move instruction.",                                                    23, CT_COMMAND,           CDT_NONE,                    CDT_POSITION_FINAL,	         true,  true);
		Add("Set Active Axis",                        "Sets the active axis.",                                                                                               25, CT_SETTING,           CDT_AXIS,                    CDT_AXIS,                    true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Axis Device Number",                 "Sets the device number to be controlled using the active axis.",                                                      26, CT_SETTING,           CDT_DEVICE_ID, /*Number,*/   CDT_DEVICE_ID, /*NUmber*/    true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Axis Inversion",                     "Inverts the active joystick axis.",                                                                                   27, CT_SETTING,           CDT_INVERT_STATUS,           CDT_INVERT_STATUS,           false, false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Axis Velocity Profile",              "Sets the relationship between the angle of the active joystick axis and the velocity of the device.",                 28, CT_SETTING,           CDT_PROFILE_NUMBER,          CDT_PROFILE_NUMBER,          true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Axis Velocity Scale",                "Sets the velocity scale of the active axis.",                                                                         29, CT_SETTING,           CDT_VELOCITY_MAX,            CDT_VELOCITY_MAX,            true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Load Event Instruction",                 "Loads the next instruction as the event-triggered instruction specified in the Command Data.",                        30, CT_COMMAND,           CDT_KEY_EVENT,               CDT_KEY_EVENT,               false, false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Return Event Instruction",               "Returns the event-triggered instruction associated with the key-event.",                                              31, CT_COMMAND,           CDT_KEY_EVENT,               CDT_NONE,                    true);
		Add("Set Calibration Mode",                   "Calibrates the joystick by adjusting the limits and the dead bands.",                                                 33, CT_SETTING,           CDT_CALIBRATION_MODE,        CDT_CALIBRATION_MODE,        true);
		Add("Restore Settings",                       "Restores the device settings to the factory defaults.",                                                               36, CT_COMMAND,           CDT_PERIPHERAL_ID,           CDT_PERIPHERAL_ID,	         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Microstep Resolution",               "Changes the number of micro steps per step.",                                                                         37, CT_SETTING,           CDT_MICROSTEPS,              CDT_MICROSTEPS,	             true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Running Current",                    "Sets the desired current to be used when the device is moving.",                                                      38, CT_SETTING,           CDT_VALUE,                   CDT_VALUE,	                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Hold Current",                       "Sets the desired current to be used when the device is holding its position.",                                        39, CT_SETTING,           CDT_VALUE,                   CDT_VALUE,	                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Device Mode",                        "Sets the Device Mode for the given device.",                                                                          40, CT_SETTING,           CDT_MODE,                    CDT_MODE,	                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Home Speed",                         "Sets the speed at which the device moves when using the \"Home\" command.",                                           41, CT_SETTING,           CDT_SPEED,                   CDT_SPEED,	                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Target Speed",                       "Sets the speed at which the device moves when using the \"Move Absolute\" or \"Move Relative\" commands.",            42, CT_SETTING,           CDT_SPEED,                   CDT_SPEED,	                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Acceleration",                       "Sets the acceleration used by the movement commands.",                                                                43, CT_SETTING,           CDT_ACCELERATION,            CDT_ACCELERATION,	         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Maximum Position",                   "Sets the maximum position the device is allowed to travel to.",                                                       44, CT_SETTING,           CDT_RANGE,                   CDT_RANGE,	                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Current Position",                   "Sets the device internal position counter.",                                                                          45, CT_SETTING,           CDT_POSITION_NEW,            CDT_POSITION_NEW,	         true,  true,  false, PROCESSING_DURATION_UNDEFINED, PT_VOLATILE);
		Add("Set Home Offset",                        "Sets the new \"Home\" position which can then be used when the Home command is issued.",                              47, CT_SETTING,           CDT_OFFSET,                  CDT_OFFSET,	                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Alias Number",                       "Sets an alternate device number for a device.",                                                                       48, CT_SETTING,           CDT_ALIAS_NUMBER,            CDT_ALIAS_NUMBER,	         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Return Device Id",                       "Returns the id number for the type of device connected.",                                                             50, CT_SETTING_READ_ONLY, CDT_NONE,                    CDT_DEVICE_ID,               true);
		Add("Return Firmware Version",                "Returns the firmware version installed on the device.",                                                               51, CT_SETTING_READ_ONLY, CDT_NONE,                    CDT_VERSION,                 true);
		Add("Return Power Supply Voltage",            "Returns the voltage level of the device's power supply.",                                                             52, CT_SETTING_READ_ONLY, CDT_NONE,                    CDT_VOLTAGE,                 true);
		Add("Return Setting",                         "Returns the current value of the setting specified in the Command Data.",                                             53, CT_COMMAND,           CDT_SETTING_NUMBER,          CDT_SETTING_VALUE,	         true);
		Add("Return Status",                          "Returns the current status of the device.",                                                                           54, CT_SETTING_READ_ONLY, CDT_NONE,                    CDT_STATUS,                  true);
		Add("Echo Data",                              "Echoes back the same Command Data that was sent.",                                                                    55, CT_COMMAND,           CDT_DATA,                    CDT_DATA,                    true);
		Add("Return Firmware Build",                  "Returns the firmware build number installed on the device.",                                                          56, CT_SETTING_READ_ONLY, CDT_NONE,                    CDT_BUILD_NUMBER,            true);
		Add("Return Current Position",                "Returns the current absolute position of the device in micro steps.",                                                 60, CT_SETTING_READ_ONLY, CDT_NONE,                    CDT_POSITION,                true,  true);
		Add("Return Serial Number",                   "Returns the serial number of the device.",                                                                            63, CT_SETTING_READ_ONLY, CDT_NONE,                    CDT_SERIAL_NUMBER,           true);
		Add("Set Park State",                         "Park or unpark the device.",                                                                                          65, CT_SETTING,           CDT_PARK_STATE,              CDT_POSITION,                true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Peripheral Id",                      "Set default values for a specific peripheral device.",                                                                66, CT_SETTING,           CDT_PERIPHERAL_ID,           CDT_PERIPHERAL_ID,           true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Auto-Reply Disabled Mode",           "Disables all command auto-replies from the device.",                                                                 101, CT_SETTING,           CDT_AUTO_REPLY_MODE,         CDT_AUTO_REPLY_MODE,         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Message Id Mode",                    "Enable message ids in device communication.",                                                                        102, CT_SETTING,           CDT_MESSAGE_ID_MODE,         CDT_MESSAGE_ID_MODE,         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Home Status",                        "Sets whether a device has been homed.",                                                                              103, CT_SETTING,           CDT_HOME_STATUS,             CDT_HOME_STATUS,             true);
		Add("Set Home Sensor Type",                   "Configures the home sensor type of the device.",                                                                     104, CT_SETTING,           CDT_HOME_SENSOR_TYPE,        CDT_HOME_SENSOR_TYPE,        true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Auto-Home Disabled Mode",            "Prevents home sensor from triggering a homing procedure during movement commands.",                                  105, CT_SETTING,           CDT_AUTO_HOME_DISABLED_MODE, CDT_AUTO_HOME_DISABLED_MODE, true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Minimum Position",                   "Sets the minimum position the device is allowed to reach.",                                                          106, CT_SETTING,           CDT_POSITION_MIN,            CDT_POSITION_MIN,            true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Knob Disabled Mode",                 "Enables or disables manual movement control.",                                                                       107, CT_SETTING,           CDT_KNOB_DISABLED_MODE,      CDT_KNOB_DISABLED_MODE,      true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Knob Direction",                     "Changes direction of movement when knob is turned.",                                                                 108, CT_SETTING,           CDT_DIRECTION,               CDT_DIRECTION,               true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Knob Movement Mode",                 "Sets the movement mode used when the manual knob is turned.",                                                        109, CT_SETTING,           CDT_MOVEMENT_MODE,           CDT_MOVEMENT_MODE,           true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Knob Jog Size",                      "Sets the distance to travel for knob manual movement in displacement mode.",                                         110, CT_SETTING,           CDT_JOG_SIZE,                CDT_JOG_SIZE,                true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Knob Velocity Scale",                "Sets the maximum speed for knob manual movement in velocity mode.",                                                  111, CT_SETTING,           CDT_VELOCITY_SCALE,          CDT_VELOCITY_SCALE,          true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Knob Velocity Profile",              "Sets the relationship between knob turn and the velocity of the device for knob manual movement in velocity mode.",  112, CT_SETTING,           CDT_VELOCITY_PROFILE,        CDT_VELOCITY_PROFILE,        true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Acceleration Only",                  "Sets only the acceleration used by the movement commands, leaving deceleration value unchanged.",                    113, CT_SETTING,           CDT_ACCELERATION,            CDT_ACCELERATION,            true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Deceleration Only",                  "Sets only the deceleration used by the movement commands, leaving acceleration value unchanged.",                    114, CT_SETTING,           CDT_DECELERATION,            CDT_DECELERATION,            true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Move Tracking Mode",                 "Enable Move Tracking messages.",                                                                                     115, CT_SETTING,           CDT_TRACKING_MODE,           CDT_TRACKING_MODE,           true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Manual Move Tracking Disabled Mode", "Disable Manual Move Tracking messages.",                                                                             116, CT_SETTING,           CDT_TRACKING_MODE,           CDT_TRACKING_MODE,           true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Move Tracking Period",               "Sets the update interval between Move Tracking and Manual Move Tracking messages.",                                  117, CT_SETTING,           CDT_TRACKING_PERIOD,         CDT_TRACKING_PERIOD,         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Closed-Loop Mode",                   "Sets the closed-loop mode for encoder embedded devices.",                                                            118, CT_SETTING,           CDT_CLOSED_LOOP_MODE,        CDT_CLOSED_LOOP_MODE,        true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Slip Tracking Period",               "Sets the update interval between Slip Tracking messages.",                                                           119, CT_SETTING,           CDT_TRACKING_PERIOD,         CDT_TRACKING_PERIOD,         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Stall Timeout",                      "Sets the timeout for stall and forced displacement conditions in closed-loop modes.",                                120, CT_SETTING,           CDT_TIMEOUT,                 CDT_TIMEOUT,                 true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Device Direction",                   "Changes direction of device movement.",                                                                              121, CT_SETTING,           CDT_DIRECTION,               CDT_DIRECTION,               true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Baud Rate",                          "Changes the baud rate of the RS232 communications.",                                                                 122, CT_SETTING,           CDT_PS232_BAUD_RATE,         CDT_PS232_BAUD_RATE,         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Set Protocol",                           "Changes the communications protocol in use.",                                                                        123, CT_SETTING,           CDT_PS232_PROTOCOL,          CDT_PS232_PROTOCOL,          true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Convert To ASCII",                       "Changes the RS232 baud rate and sets the communications protocol to ASCII.",                                         124, CT_COMMAND,           CDT_PS232_BAUD_RATE,         CDT_PS232_BAUD_RATE,         true,  false, false, PROCESSING_DURATION_UNDEFINED, PT_NON_VOLATILE);
		Add("Error",                                  "Indicates to the user that an error has occurred. The error code is the command number (but not always)",            255, CT_REPLY,             CDT_NONE,                    CDT_ERROR_CODE,              true);		   
	}

	const string ToString() const
	{
		ostringstream oss;
		RepositorySet::const_iterator ci = m_Repository.begin();

		for(; ci != m_Repository.end(); ++ci)
		{
			oss << (*ci).GetInstructionName() << " - " << (*ci).GetSummary() << ";" << endl;
		}
		return oss.str();
	}
};

