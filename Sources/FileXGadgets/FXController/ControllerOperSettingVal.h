#pragma once

#include <string>
#include <vector>
#include "helpers\UserPropertiesStruct.h"

using namespace std;

template<typename T>
class ControllerOperSettingValue
{
private:
  string           m_name;
  T                m_current;
  T                m_default;
  T                m_lowerBound;
  T                m_upperBound;
  PropertyUpdateCB m_propChangedHandler;
  PropertyUpdateCB m_valueChangedHandler;
  void*            m_pOwner;

  inline static void propertyParamChanged(LPCTSTR pName, void* pValue, bool& bInvalidate)
  {
    ControllerOperSettingValue * pThisValue = (ControllerOperSettingValue*)pValue;
    if (pThisValue)
    {
      pThisValue->setCurrent(pThisValue->m_currentRef);

      if (pThisValue->m_currentRef != pThisValue->getCurrent())
        pThisValue->m_currentRef = pThisValue->getCurrent();
    }
  }

  inline void onValueChanged()
  {
    bool bIsHandled = false;
    if (m_valueChangedHandler && m_pOwner)
      m_valueChangedHandler(getName().c_str(), m_pOwner, bIsHandled);
  }

  ControllerOperSettingValue(const ControllerOperSettingValue&);
  ControllerOperSettingValue& operator=(const ControllerOperSettingValue&);
public:
  T m_currentRef; //the field to attach to the gadget property;

  inline const string& getName() const
  {
    return m_name;
  }
  inline const T& getCurrent() const
  {
    return m_current;
  }
  inline const T& getDefault() const
  {
    return m_default;
  }
  inline const T& getLowerBound() const
  {
    return m_lowerBound;
  }
  inline const T& getUpperBound() const
  {
    return m_upperBound;
  }
  inline const PropertyUpdateCB& getPropertyChangedHandler() const
  {
    return m_propChangedHandler;
  }

  inline const ControllerOperSettingValue& setCurrent(const T& current)
  {
    bool toNotify = m_current != current;

    m_current = current;
    if (m_current < m_lowerBound)
      m_current = m_lowerBound;
    if (m_current > m_upperBound)
      m_current = m_upperBound;

    if (toNotify)
      onValueChanged();
    return *this;
  }
  inline const ControllerOperSettingValue& setDefault(const T& defaultVal)
  {
    m_defaultVal = defaultVal;
    if (m_defaultVal < m_lowerBound)
      m_defaultVal = m_lowerBound;
    if (m_defaultVal > m_upperBound)
      m_defaultVal = m_upperBound;
    return *this;
  }
  inline const ControllerOperSettingValue& setLowerBound(const T& lowerBound)
  {
    m_lowerBound = lowerBound;
    return *this;
  }
  inline const ControllerOperSettingValue& setUpperBound(const T& upperBound)
  {
    m_upperBound = upperBound;
    return *this;
  }

  inline const ControllerOperSettingValue& resetCurrentToDefault()
  {
    m_currentRef = getDefault();
    return setCurrent(m_currentRef);
  }

  inline ControllerOperSettingValue& setValueChangedListener(const PropertyUpdateCB& valueChangedHandler, void* pOwner)
  {
    m_valueChangedHandler = valueChangedHandler;
    m_pOwner = pOwner;
    return *this;
  }

  inline ControllerOperSettingValue(const string& propName, const T& defaultVal, const T& lowerBound, const T& upperBound)
    : m_name(propName)
    , m_current(defaultVal)
    , m_currentRef(defaultVal)
    , m_default(defaultVal)
    , m_lowerBound(lowerBound)
    , m_upperBound(upperBound)
    , m_propChangedHandler(propertyParamChanged)
    , m_valueChangedHandler(NULL)
    , m_pOwner(NULL)
  { }

  inline virtual ~ControllerOperSettingValue()
  {
    m_propChangedHandler = NULL;
    m_valueChangedHandler = NULL;
    m_pOwner = NULL;
  }
};

typedef
ControllerOperSettingValue<bool> ControllerOperSettingValueBool;

typedef
ControllerOperSettingValue<int> ControllerOperSettingValueInt;

class ControllerOperSettings
{
private:

  void*                                  m_pOwner;
  ControllerOperSettingValueInt          m_valueIndxToReset;
  //ControllerOperSettingValueBool         m_bIsSixPinsConnectors;
  //ControllerOperSettingValueInt          m_encoderLength;
  //ControllerOperSettingValueInt          m_encoderShift;
  ControllerOperSettingValueInt          m_encoderTick_Red;
  ControllerOperSettingValueInt          m_encoderTick_Green;
  ControllerOperSettingValueInt          m_encoderTick_Blue;
  ControllerOperSettingValueBool         m_bToEnforce3TicksInVideo;
  //ControllerOperSettingValueInt          m_lto;
  //ControllerOperSettingValueInt          m_maxFPS;
  //ControllerOperSettingValueInt          m_interStrobeDefence;
  //ControllerOperSettingValueInt          m_shutterLightDefence;
  PropertyUpdateCB                       m_valueChangedHandler;

  vector<ControllerOperSettingValueInt*> m_arrayValsInt =
  {
    //&m_encoderLength,
    //&m_encoderShift,
    &m_encoderTick_Red,
    &m_encoderTick_Green,
    &m_encoderTick_Blue //,
    //&m_lto,
    //&m_maxFPS,
    //&m_interStrobeDefence,
    //&m_shutterLightDefence
  };

  inline void onValueChanged(const string& valName)
  {
    bool bIsHandled = false;
    if (m_valueChangedHandler && m_pOwner)
      m_valueChangedHandler(valName.c_str(), m_pOwner, bIsHandled);
  }

  inline void resetProperties()
  {
    const int propIndxToReset = m_valueIndxToReset.getCurrent();
    const int INDX_NONE = 0;
    const int INDX_ALL_VALUES = INDX_NONE + 1;
    const int INDX_IsSixPinsConnectors = INDX_ALL_VALUES + 1;
    const int INDX_ToEnforce3TicksInVideo = INDX_IsSixPinsConnectors + 1;

    size_t intsPropsColLowerIndx = 0;
    size_t intsPropsColUpperInx = 0;
    switch (propIndxToReset)
    {
    case INDX_NONE:
    case INDX_IsSixPinsConnectors:
    case INDX_ToEnforce3TicksInVideo:
      intsPropsColLowerIndx = 0;
      intsPropsColUpperInx = 0;
      break;
    case INDX_ALL_VALUES:
      intsPropsColUpperInx = m_arrayValsInt.size();
      break;
    default:
      intsPropsColLowerIndx = propIndxToReset - (INDX_IsSixPinsConnectors + 1);
      intsPropsColUpperInx = intsPropsColLowerIndx + 1;
      break;
    }

    for (; intsPropsColLowerIndx < intsPropsColUpperInx; intsPropsColLowerIndx++)
    {
      m_arrayValsInt.at(intsPropsColLowerIndx)->resetCurrentToDefault();
    }

    //if (propIndxToReset > INDX_NONE && propIndxToReset <= INDX_IsSixPinsConnectors)
    //  m_bIsSixPinsConnectors.resetCurrentToDefault();

    if (propIndxToReset == INDX_ALL_VALUES || propIndxToReset == INDX_ToEnforce3TicksInVideo)
      m_bToEnforce3TicksInVideo.resetCurrentToDefault();


    //Resets the property that used to determine which property/ies to reset;
    m_valueIndxToReset.resetCurrentToDefault();
  }

  inline static void resetPropertyParamChanged(LPCTSTR pName, void* pValue, bool& bInvalidate)
  {
    ControllerOperSettings * pThisValue = (ControllerOperSettings*)pValue;
    if (pThisValue)
    {
      pThisValue->resetProperties();
    }
  }



  ControllerOperSettings(const ControllerOperSettings&);
  ControllerOperSettings& operator=(const ControllerOperSettings&);
public:
  static const int SHUTTER_LIGHT_DEFENSE_us = 300; //us
  static const int INTERSTROBE_DEFENSE_us = 100; //us
  //static const int INTERSTROBE_DELAY_us = SHUTTER_LIGHT_DEFENSE_us + INTERSTROBE_DEFENSE_us; //full defense delay
  static const int MAX_FPS = 40; //can be 60?


  inline ControllerOperSettingValueInt& getPropertyIndxToReset()
  {
    return m_valueIndxToReset;
  }

  //inline ControllerOperSettingValueBool& getIsSixPinsConnectors()
  //{
  //	return m_bIsSixPinsConnectors;
  //}

  //inline ControllerOperSettingValueInt& getEncoderLength()
  //{
  //	return m_encoderLength;
  //}
  //inline ControllerOperSettingValueInt& getEncoderShift()
  //{
  //	return m_encoderShift;
  //}
  inline ControllerOperSettingValueInt& getEncoderTickRed()
  {
    return m_encoderTick_Red;
  }
  inline ControllerOperSettingValueInt& getEncoderTickGreen()
  {
    return m_encoderTick_Green;
  }
  inline ControllerOperSettingValueInt& getEncoderTickBlue()
  {
    return m_encoderTick_Blue;
  }
  inline ControllerOperSettingValueBool& getToEnforce3TicksInVideo()
  {
    return m_bToEnforce3TicksInVideo;
  }

  //inline ControllerOperSettingValueInt& getLTO()
  //{
  //	return m_lto;
  //}
  //inline ControllerOperSettingValueInt& getMaxFPS()
  //{
  //	return m_maxFPS;
  //}
  //inline ControllerOperSettingValueInt& getInterStrobeDefence()
  //{
  //	return m_interStrobeDefence;
  //}
  //inline ControllerOperSettingValueInt& getShutterLightDefence()
  //{
  //	return m_shutterLightDefence;
  //}

  inline const vector<ControllerOperSettingValueInt*>& getIntValuesCollection() const
  {
    return m_arrayValsInt;
  }

  inline string getAllValuesNames(const string& delimiter = ";") const
  {
    string res = "None;All";
    if (!res.empty())
      res.append(delimiter);
    res.append(m_bToEnforce3TicksInVideo.getName());

    for (vector<ControllerOperSettingValueInt*>::const_iterator ci = m_arrayValsInt.begin(); ci != m_arrayValsInt.end(); ci++)
      res.append(delimiter).append((*ci)->getName());
    return res;
  }

  inline string getAllValuesNamesForReset() const
  {
    return getAllValuesNames();
  }

  inline ControllerOperSettings& setSettingsChangedListener(const PropertyUpdateCB& handler, void* pOwner)
  {
    //m_bIsSixPinsConnectors.setValueChangedListener(handler, pOwner);

    for (vector<ControllerOperSettingValueInt*>::iterator i = m_arrayValsInt.begin(); i != m_arrayValsInt.end(); i++)
      (*i)->setValueChangedListener(handler, pOwner);

    m_valueChangedHandler = handler;
    m_pOwner = pOwner;
    return *this;
  }

  inline ControllerOperSettings()
    : //m_bIsSixPinsConnectors("IsSixPinsConnector", true, false, true)
    //, m_encoderLength("EncoderLength", CommandEL::CMD_VAL_DFLT_ENCDR_LNGHT, 0, CommandEL::CMD_VAL_DFLT_ENCDR_LNGHT)
    //, m_encoderShift("EncoderShift", 0/*CmdsSequenceConfigTicks::ENCODER_SHIFT*/, 0, CmdsSequenceConfigTicks::ENCODER_SHIFT)
    /*,*/ m_encoderTick_Red("EncoderTick_RED", /*CmdsSequenceConfigTicks::ENCODER_TICK_RED*/0, 0,/* CmdsSequenceConfigTicks::ENCODER_SHIFT*/0)
    , m_encoderTick_Green("EncoderTick_GREEN", /*CmdsSequenceConfigTicks::ENCODER_TICK_GREEN*/-1000, -1000/*CmdsSequenceConfigTicks::ENCODER_TICK_RED*/, -1000/*CmdsSequenceConfigTicks::ENCODER_TICK_GREEN*/)
    , m_encoderTick_Blue("EncoderTick_BLUE", /*CmdsSequenceConfigTicks::ENCODER_TICK_BLUE*/1000, 1000/*CmdsSequenceConfigTicks::ENCODER_TICK_GREEN*/, 1000/*CmdsSequenceConfigTicks::ENCODER_TICK_BLUE*/)
    , m_bToEnforce3TicksInVideo("Enforce3TicksInVideo", true, false, true)
    //, m_lto("LTO", CommandLTO::CMD_VAL_DFLT_LTO, 0, CommandLTO::CMD_VAL_DFLT_LTO)
    //, m_maxFPS("MaximalFPS", ControllerOperSettings::MAX_FPS, 1, ControllerOperSettings::MAX_FPS)
    //, m_interStrobeDefence("InterStrobeDefence", ControllerOperSettings::INTERSTROBE_DEFENSE_us, 0, ControllerOperSettings::INTERSTROBE_DEFENSE_us)
    //, m_shutterLightDefence("ShutterLightDefence", ControllerOperSettings::SHUTTER_LIGHT_DEFENSE_us, 0, ControllerOperSettings::SHUTTER_LIGHT_DEFENSE_us)
    , m_valueChangedHandler(NULL)
    , m_pOwner(NULL)
    , m_valueIndxToReset("PropertyToReset", 0, 0, 1024)
  {
    m_valueIndxToReset.setValueChangedListener(ControllerOperSettings::resetPropertyParamChanged, this);
  }

  inline virtual ~ControllerOperSettings()
  {
    m_valueChangedHandler = (NULL);
    m_pOwner = (NULL);
  }
};

