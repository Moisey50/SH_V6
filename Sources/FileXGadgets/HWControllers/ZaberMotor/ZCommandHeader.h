#pragma once

#include <string>
#include <sstream>
#include "ZUtils.h"

using namespace std;

typedef enum _commandType
{
	CT_NONE = -1,
	CT_COMMAND,
	CT_SETTING,
	CT_SETTING_READ_ONLY,
	CT_REPLY,

}eCommandType;

typedef enum _commandDataType
{
	CDT_NONE = -1,
	CDT_DATA,
	CDT_VALUE,
	CDT_STATUS,
	CDT_MODE,
	CDT_VERSION,
	CDT_BUILD_NUMBER,
	CDT_SERIAL_NUMBER,
	CDT_TIMEOUT,
	CDT_DIRECTION,
	CDT_POSITION,
	CDT_POSITION_NEW,
	CDT_POSITION_ABSOLUTE,
	CDT_POSITION_RELATIVE,
	CDT_POSITION_FINAL,
	CDT_POSITION_STORED,
	CDT_POSITION_TRAKING,
	CDT_POSITION_MIN,
	CDT_AXIS,
	CDT_PROFILE_NUMBER,
	CDT_REGISTER_ADDRESS,
	CDT_DEVICE_ID,
	CDT_DEVICE_ID_NEW,
	CDT_PERIPHERAL_ID,
	CDT_ADDRESS,
	CDT_SPEED,
	CDT_VELOCITY_SCALE,
	CDT_VELOCITY_PROFILE,
	CDT_VELOCITY_MAX,
	CDT_MICROSTEPS,
	CDT_ACCELERATION,
	CDT_DECELERATION,
	CDT_PARK_STATE,
	CDT_RANGE,
	CDT_OFFSET,
	CDT_ALIAS_NUMBER,
	CDT_LOCK_STATE,
	CDT_VOLTAGE,
	CDT_CALIBRATION_MODE,
	CDT_SETTING_NUMBER,
	CDT_SETTING_VALUE,
	CDT_KEY_EVENT,
	CDT_MOVEMENT_MODE,
	CDT_AUTO_REPLY_MODE,
	CDT_MESSAGE_ID_MODE,
	CDT_AUTO_HOME_DISABLED_MODE,
	CDT_KNOB_DISABLED_MODE,
	CDT_TRACKING_MODE,
	CDT_TRACKING_PERIOD,
	CDT_CLOSED_LOOP_MODE,
	CDT_HOME_SENSOR_TYPE,
	CDT_INVERT_STATUS,
	CDT_HOME_STATUS,
	CDT_JOG_SIZE,
	CDT_PS232_BAUD_RATE,
	CDT_PS232_PROTOCOL, 
	CDT_ERROR_CODE,

}eCommandDataType;

typedef enum _persistenceType
{
	PT_NONE = -1,
	PT_NON_VOLATILE,
	PT_VOLATILE,
}ePersistenceType;

#define COMMAND_ID_NONE               (-1)
#define PROCESSING_DURATION_UNDEFINED (-1)

class ZCommandHeader
{
private:
	string           m_InstructionName;
	short            m_ID;
	eCommandType     m_Type;
	eCommandDataType m_DataType_Send;
	eCommandDataType m_DataType_Reply;

	bool             m_IsSafe2Retry;
	bool             m_IsReturnsCurrentPos;
	bool             m_IsPreEmptive;

	int              m_ProcessingDuration; //in milliseconds

	ePersistenceType m_Persistence;

	string           m_Summary;
public:
	const string& GetInstructionName() const
	{
		return m_InstructionName;
	}

	short GetID() const
	{
		return m_ID;
	}
	
	eCommandType GetCommandType() const
	{
		return m_Type;
	}

	eCommandDataType GetCommandDataType2Send() const
	{
		return m_DataType_Send;
	}

	eCommandDataType GetCommandDataType2Reply() const
	{
		return m_DataType_Reply;
	}

	bool IsSafe2Retry() const
	{
		return m_IsSafe2Retry;
	}
	bool IsReturnsCurrentPos() const
	{
		return m_IsReturnsCurrentPos;
	}
	bool IsPreEmptive() const
	{
		return m_IsPreEmptive;
	}

	int GetProcessDuration() const
	{
		return m_ProcessingDuration;
	}

	ePersistenceType GetPersistence() const
	{
		return m_Persistence;
	}

	const string& GetSummary() const
	{
		return m_Summary;
	}


	ZCommandHeader(void)
		: m_InstructionName()
		, m_ID(COMMAND_ID_NONE)
		, m_Type(CT_NONE)
		, m_DataType_Send(CDT_NONE)
		, m_DataType_Reply(CDT_NONE)
		, m_IsSafe2Retry(false)
		, m_IsReturnsCurrentPos(false)
		, m_IsPreEmptive(false)
		, m_ProcessingDuration(PROCESSING_DURATION_UNDEFINED)
		, m_Persistence(PT_NONE)
		, m_Summary()
	{

	}
	~ZCommandHeader(void)
	{
	}

	static ZCommandHeader* CreateCommand(
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
		, ePersistenceType persistence = PT_NONE)
	{
		ZCommandHeader* pRes = new ZCommandHeader();

		pRes->m_InstructionName = NormalizeInstructionName(instructionName);
		pRes->m_ID = id;
		pRes->m_Type = cmdType;
		pRes->m_DataType_Send = cmdDataType_Send;
		pRes->m_DataType_Reply = cmdDataType_Reply;
		pRes->m_IsSafe2Retry = isSafe2Retry;
		pRes->m_IsReturnsCurrentPos = isReturnsCurrentPos;
		pRes->m_IsPreEmptive = isPreEmptive;
		pRes->m_ProcessingDuration = processingDuration;
		pRes->m_Persistence = persistence;
		pRes->m_Summary = summary;

		return pRes;
	}

	static bool IsEmptyCommand(const ZCommandHeader& cmd)
	{
		return cmd.GetID() == COMMAND_ID_NONE;
	}

	//static const ZCommandHeader* DeserializeCommand(const string& delimitedTxt)
	//{

	//}

	static string NormalizeInstructionName( const string& cmdName )
	{
		int indx = 0;
		string instructName = ZUtils::Tokenize(cmdName, "(", indx);
		string unwanted("- .,");
		string wanted("_");
		int lastIndx = 0;
		lastIndx = (int) instructName.find_first_of(unwanted, lastIndx);
		
		while(lastIndx != basic_string<char>::npos)
		{
			instructName.replace(lastIndx, wanted.length(), wanted);
			lastIndx = (int) instructName.find_first_of(unwanted, lastIndx);
		}
		return instructName;
	}

	friend bool operator<(const ZCommandHeader& pLhs, const ZCommandHeader& pRhs)
	{
		return pLhs.m_ID<pRhs.m_ID;
	}

	string ToString() const
	{
		ostringstream ss;
		ss << GetInstructionName() << "(" << GetID() << ")";
		return ss.str();
	}
	string Serialize() const
	{
		ostringstream oss;

		string hexCmdID;
		if(!ZUtils::DecByte2HexByte(GetID(), hexCmdID))
			hexCmdID = "00";

		oss << hexCmdID;

		return oss.str();
	}
};




