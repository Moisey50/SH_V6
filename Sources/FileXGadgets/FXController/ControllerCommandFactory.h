#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <cstdarg>
#include <list>
#include <typeinfo>


#include "helpers\UserPropertiesStruct.h"

//#include "ConfigurationFactory.h"

using namespace std;
#define CMD_DELIMITER_DEFAULT     ("=")
#define CMD_EOL                   ("\r\n")



#define CMD_NAME_ABORT            ("A")
#define CMD_NAME_SET_DATA         ("D")
#define CMD_NAME_START_VIDEO      ("V")
#define CMD_NAME_START_SNAPSHOT   ("S")

class CommandBase
{
private:
	string m_key;

protected:
	inline virtual string onGetCmd() const
	{
		return m_key;
	}

public:
	static const CommandBase Empty;
  
  inline virtual bool isEmpty() const
  {
    return m_key.empty();
  }

  inline static bool isCommand(const CommandBase& cmd, const string& expectedKeyName)
  {
    return cmd.getKey().compare(expectedKeyName) == 0;
  }

	inline string getKey() const
	{
		return m_key;
	}

	inline CommandBase(const string& key)
		: m_key(key)
	{
			
	}

	inline string getCmdText() const
	{
    return onGetCmd().append(CMD_EOL);
	}
};

template<typename T>
class CommandWithValue : public CommandBase
{
private:
	T m_value;
	string m_delimiter;

	inline virtual string getValueAsTxt() const
	{		
		return getValueAsTxt<T>(m_value);
	}

protected:
	static const int HEX_PADDING_WIDTN_16BIT = 4;

	template<typename V>
	inline static string getValueAsTxt(const V& value, bool isHex = false, int paddingWidth = HEX_PADDING_WIDTN_16BIT)
	{
		const char HEX_PADDING_CHAR = '0';
		ostringstream oss;
		if (isHex)
		{
			oss.fill(HEX_PADDING_CHAR);
			oss.width(paddingWidth);
			oss << hex;
		}
		oss << value;
		return oss.str();
	}

	inline string getDelimiter() const
	{
		return m_delimiter;
	}

	inline string onGetCmd()   const override
	{
		return getKey().append(m_delimiter).append(getValueAsTxt());
	}

public:
	inline CommandWithValue(const string& key, const T& value, const string& delimiter = CMD_DELIMITER_DEFAULT)
		: CommandBase(key)
		, m_value(value)
		, m_delimiter(delimiter)
	{

	}

  inline string getKeyValueAsTxt(const string& delimiter = CMD_DELIMITER_DEFAULT) const
  {
    string res = getKey();
    return res
      .append(delimiter)
      .append(getValueAsTxt());
  }

  inline virtual bool isEmpty() const override 
  {
    return CommandBase::isEmpty() || getValueAsTxt().empty();
  }
};

typedef CommandWithValue<int> CommandWithValueInt;
typedef CommandWithValue<string> CommandWithValueTxt;
//class CommandWithValueInt : public CommandWithValue<int>
//{
//public:
//	using CommandWithValue<int>::CommandWithValue;
//};
//class CommandWithValueTxt : public CommandWithValue<string>
//{
//public:
//	using CommandWithValue<string>::CommandWithValue;
//};


class CommandAbort : public CommandBase
{
public:
	inline CommandAbort()
		: CommandBase(CMD_NAME_ABORT)
	{			
	}
};

class CommandStartVideo : public CommandBase
{
public:
  inline CommandStartVideo()
    : CommandBase(CMD_NAME_START_VIDEO)
  {
  }
};

class CommandStartSnapshot : public CommandBase
{
public:
  inline CommandStartSnapshot()
    : CommandBase(CMD_NAME_START_SNAPSHOT)
  {
  }
};

class CommandSetData :
  public CommandWithValueTxt
{
public:
  const static string CMD_DFLT_KEY_VAL_DLMTR_CLEAR;// = ("");

  inline CommandSetData(const string& data)
    : CommandWithValueTxt(CMD_NAME_SET_DATA, data, CMD_DFLT_KEY_VAL_DLMTR_CLEAR)
  {}
};


//class ControllerOperationBase
//{
//private:
//	const ModeConfigBase& m_mode;
//	int m_cameraShutter_us;
//	vector<const CmdsSequenceBase*> m_pCmdsSubSequences;
//
//protected:
//  inline void getKeyValueAsTxt(const CmdsSequenceBase * pCmdsSqnc, const string& cmdKey, __out string &txt) const
//  {
//    const CommandBase* pCmdByName = NULL;
//    int indx = pCmdsSqnc->getCmdByKey(cmdKey, 0, &pCmdByName);
//    if (indx >= 0 && pCmdByName != NULL)
//      txt = ((CommandWithValueInt*)pCmdByName)->getKeyValueAsTxt();
//  }
//public:
//	static const ControllerOperationBase Empty;
//
//  inline bool isEmpty() const
//	{
//		return m_pCmdsSubSequences.empty();
//	}
//
//	virtual void clear()
//	{
//		for (vector<const CmdsSequenceBase*>::iterator i = m_pCmdsSubSequences.begin(); i != m_pCmdsSubSequences.end(); i++)
//		{
//			delete (*i);
//			(*i) = NULL;
//		}
//
//		m_pCmdsSubSequences.clear();
//	}
//
//  inline int getCameraShutter() const
//	{
//		return m_cameraShutter_us;
//	}
//
//  inline const string getModeName() const
//	{
//		return m_mode.getModeName();
//	}
//
//  inline const string getColorConfigDescr() const
//	{
//		return m_mode.getModeDescription();
//	}
//	
//	ControllerOperationBase(const ModeConfigBase& mode, int cameraShutter_us, const list<const CmdsSequenceBase*> subSequences)
//		: m_mode(mode)
//		, m_cameraShutter_us(cameraShutter_us)
//		, m_pCmdsSubSequences(subSequences.begin(), subSequences.end())
//	{
//	}
//
//	virtual ~ControllerOperationBase()
//	{
//		clear();
//	}
//
//  inline const string getCmdsAsTxt() const
//  {
//    string commandsTxt;
//    for (vector<const CmdsSequenceBase*>::const_iterator ci = m_pCmdsSubSequences.begin(); ci != m_pCmdsSubSequences.end(); ci++)
//    {
//      if (*ci)
//        commandsTxt.append((*ci)->getCmdsTxt());
//    }
//    return commandsTxt;
//  }
//
//  inline const string getDKandELandTicksAsTxt() const
//  {
//    string res;
//    string encLenAndColorsWithTicksAsTxt;
//    for (vector<const CmdsSequenceBase*>::const_iterator ci = m_pCmdsSubSequences.begin(); ci != m_pCmdsSubSequences.end() /*&& colorsWithTicksAsTxt.empty()*/; ci++)
//    {
//      string txt;
//      const CmdsSequenceBase* pCmdsNonTicks = NULL;
//      if (typeid(**ci) == typeid(CmdsSequenceConfigNonTicksBase) || typeid(**ci) == typeid(CmdsSequenceConfigNonTicksWithLTO))
//        pCmdsNonTicks = *ci;
//
//      if (pCmdsNonTicks)
//        getKeyValueAsTxt(pCmdsNonTicks, CMD_NAME_ENCODER_LENGTH, txt);
//      else
//      {
//        const CmdsSequenceConfigTicks* pCmdsTicks = typeid(**ci) != typeid(CmdsSequenceConfigTicks) ? NULL : (const CmdsSequenceConfigTicks*)*ci;
//        if (pCmdsTicks)
//          txt = pCmdsTicks->getColorsWithTicks();
//      }
//
//      if (!txt.empty())
//      {
//        if (!encLenAndColorsWithTicksAsTxt.empty())
//          encLenAndColorsWithTicksAsTxt.append(";");
//        encLenAndColorsWithTicksAsTxt.append(txt);
//      }
//    }
//    
//    string dk;
//    vector<const CmdsSequenceBase*>::const_reverse_iterator criDk = m_pCmdsSubSequences.crbegin();
//
//    if (*criDk)
//      getKeyValueAsTxt(*criDk, CMD_NAME_DK, dk);
//
//
//    if (!dk.empty() && !encLenAndColorsWithTicksAsTxt.empty())
//      res = dk.append(";").append(encLenAndColorsWithTicksAsTxt);
//
//    return res;
//  }
//
//};

//class ControllerOperationVideo
//	: public ControllerOperationBase
//{
//private:
//	
//
//	int m_calculatedFPS;
//
//protected:
//	
//
//public:
//	
//  inline int getCalculatedFPS() const
//	{
//		return m_calculatedFPS;
//	}
//
//	ControllerOperationVideo(const ModeConfigBase& mode, int cameraShutter, int calcFPS, const list<const CmdsSequenceBase*> subSequences)
//		: ControllerOperationBase(mode, cameraShutter, subSequences)
//		, m_calculatedFPS(calcFPS)
//	{
//		
//	}
//};

//class ControllerOperationFactory
//{
//private:
//	static const int MICROSECONDS_PER_SECOND  = 1000 * 1000;
//
//	static int calcCameraShutter(int maxLightOnDuration_us, int shutterLightDefence /*= SHUTTER_LIGHT_DEFENSE_us*/)
//	{
//		int res = maxLightOnDuration_us + shutterLightDefence;
//		return res;
//	}
//
//	static int getRequiredFrameTime(int maxLightOnDuration_us, int maxFPS /*= MAX_FPS*/, int interStrobeDelay /*= INTERSTROBE_DELAY_us*/)
//	{
//		int allColorsMaxTimeAtMaxFPS = getAllColorsTimeOnMaxFPS(maxFPS);
//
//		int allColorsActualTime = (maxLightOnDuration_us + interStrobeDelay);
//
//		if (allColorsActualTime < allColorsMaxTimeAtMaxFPS)
//			allColorsActualTime = allColorsMaxTimeAtMaxFPS;
//
//		return allColorsActualTime;
//	}
//
//	static int calculatedFPS(int requiredFrameTime)
//	{		
//		int res = (int)((double)MICROSECONDS_PER_SECOND / requiredFrameTime);
//		return res;
//	}
//	
//	// Returns total duration (in micro seconds) for all colors frames on maximal FPS
//	static int getAllColorsTimeOnMaxFPS(int maxFPS /*= MAX_FPS*/, int colorFrames = COLORS_MASKS_BY_ID.size())
//	{
//		int res = -1;
//		double dCommonTimePerMaxFps = ((double)MICROSECONDS_PER_SECOND / (double)maxFPS);
//		res = (int)dCommonTimePerMaxFps;
//		return res;
//	}
//
//  static int getDK_Period(int requiredFrameTime, int encoderLingth /*= CommandEL::CMD_VAL_DFLT_ENCDR_LNGHT*/, int colorFrames /*= COLORS_MASKS_BY_ID.size()*/, int requiredFrameRate)
//  {
//    int res = 0;
//    // the 'encoderLingth * MICROSECONDS_PER_SECOND' is to big number and it is overflowing then
//    //double allFramesTime = requiredFrameTime * colorFrames;
//    //res = (int)(MICROSECONDS_PER_SECOND * (encoderLingth / allFramesTime));
//
//    return requiredFrameRate * encoderLingth / colorFrames;
//  }
//
//	static int getDK_Period(int requiredFrameTime, int encoderLingth /*= CommandEL::CMD_VAL_DFLT_ENCDR_LNGHT*/, int colorFrames /*= COLORS_MASKS_BY_ID.size()*/)
//	{
//		int res = 0;
//		// the 'encoderLingth * MICROSECONDS_PER_SECOND' is to big number and it is overflowing then
//		double allFramesTime = requiredFrameTime * colorFrames;
//		res = (int)(MICROSECONDS_PER_SECOND * (encoderLingth / allFramesTime));
//
//		return res;
//	}
//
//public:
//	
//  //static const ControllerOperationBase* create(const ModeConfigBase& mode, ControllerOperSettings& settings)
//  //{
//  //  return create(mode
//  //    , settings.getIsSixPinsConnectors().getCurrent()
//  //    , settings.getEncoderLength().getCurrent()
//  //    , settings.getEncoderShift().getCurrent()
//  //    ,
//  //    {
//  //      settings.getEncoderTickRed().getCurrent()
//  //    , settings.getEncoderTickGreen().getCurrent()
//  //    , settings.getEncoderTickBlue().getCurrent()
//  //    }
//  //    , settings.getToEnforce3TicksInVideo().getCurrent()
//  //    , settings.getMaxFPS().getCurrent()
//  //    , settings.getInterStrobeDefence().getCurrent()
//  //    , settings.getShutterLightDefence().getCurrent()
//  //    , settings.getLTO().getCurrent()
//  //  );
//  //}
//	//static const ControllerOperationBase* create(const ModeConfigBase& mode, bool isSixPinsConnector, int encoderLength /*= CommandEL::CMD_VAL_DFLT_ENCDR_LNGHT*/, int encoderShift, const list<int>& predfEncTicks, bool toEnforce3TicksPerRevInVideo, int maxFPS /*= MAX_FPS*/, int interStrobeDefence /*= INTERSTROBE_DEFENSE_us*/, int shutterLightDefence /*= SHUTTER_LIGHT_DEFENSE_us*/, int lto = CommandLTO::CMD_VAL_DFLT_LTO)
//	//{
//	//	const ControllerOperationBase* pRes = NULL;
//  //
//	//	int interStrobeDelay = shutterLightDefence + interStrobeDefence;
//  //
//	//	int requiredFrameTime = getRequiredFrameTime(mode.getMaxLightOnDuration(),  maxFPS /*= MAX_FPS*/,  interStrobeDelay /*= INTERSTROBE_DELAY_us*/);
//  //
//	//	const CmdsSequenceBase* pCommandsPreConfig = new CmdsSequencePreconfig();
//	//	const CmdsSequenceBase* pCommandsConfigTicks = new CmdsSequenceConfigTicks(mode, isSixPinsConnector, encoderLength, encoderShift, predfEncTicks, toEnforce3TicksPerRevInVideo);
//  //  const CmdsSequenceBase* pCommandsConfigPostTicks = new CmdsSequenceBase({ new CommandPT(mode.isFullColor()) });
//	//	const int cameraShutter = calcCameraShutter(mode.getMaxLightOnDuration(), shutterLightDefence);
//  //
//  //  int colorsPerRev = CmdsSequenceConfigTicks::TICKS_PER_REV_RGB;
//  //
//  //  switch (mode.getModeID())
//  //  {
//  //  case ModeIDs::MODE_VIDEO:
//  //  {
//  //    if (!toEnforce3TicksPerRevInVideo)
//  //      colorsPerRev = (int)COLORS_MASKS_BY_ID.size();
//  //
//  //    ModeConfigVideo* pVModeVideo = (ModeConfigVideo*)&mode;
//  //
//  //    pRes = new ControllerOperationVideo
//  //    (
//  //      mode
//  //      , cameraShutter
//  //      , calculatedFPS(requiredFrameTime)
//  //      , {
//  //        pCommandsPreConfig
//  //        , new CmdsSequenceConfigNonTicksWithLTO(lto, encoderLength)
//  //        , pCommandsConfigTicks
//  //        , pCommandsConfigPostTicks
//  //        , new CmdsSequenceBase({ new CommandDK(getDK_Period(requiredFrameTime, encoderLength, colorsPerRev, pVModeVideo->getFrameRate())) })
//  //      }
//  //    );
//  //  }
//  //  break;
//  //  case ModeIDs::MODE_SNAPSHOT:
//  //    pRes = new ControllerOperationBase
//  //    (
//  //      mode
//  //      , cameraShutter
//  //      , {
//  //        pCommandsPreConfig
//  //        , new CmdsSequenceConfigNonTicksBase(encoderLength)
//  //        , pCommandsConfigTicks
//  //        , pCommandsConfigPostTicks
//  //        , new CmdsSequenceBase({ new CommandDK(getDK_Period(requiredFrameTime, encoderLength, colorsPerRev)) })
//  //      }
//  //    );
//  //    break;
//  //  }
//  //
//	//	return pRes;
//	//}
//
//	static string getCmdAbort()
//	{
//		return CmdsSequencePreconfig().getCmdsTxt();
//	}
//};