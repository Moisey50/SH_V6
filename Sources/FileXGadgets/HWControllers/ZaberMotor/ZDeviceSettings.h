#pragma once
#include <string>

using namespace std;

template<typename TValue> class ZDeviceSetting
{
	int    m_cmdId;
	TValue m_value;
public:
	int GetCommandId() const
	{
		return m_cmdId;
	}

	TValue GetValue() const
	{
		return m_value;
	}
public:
	ZDeviceSetting(int cmdId=0, TValue value=0)
		: m_cmdId(cmdId)
		, m_value(value)
	{

	}
};


#define ZBR_DEFAULT_SETTING_PROD_ID					  (4152)
#define ZBR_DEFAULT_SETTING_PROD_NAME				  ("T-LSR150B")
#define ZBR_DEFAULT_SETTING_PROD_SN					  (-1)
//#define ZBR_DEFAULT_SETTING_MAX_POS					  (305385)
#define ZBR_DEFAULT_SETTING_ACCELERATION			  (111)
#define ZBR_DEFAULT_SETTING_SPEED_TRGT				  (2922)
#define ZBR_DEFAULT_SETTING_SPEED_HOME				  (2922)
#define ZBR_DEFAULT_SETTING_RESOLUTION				  (64)
#define ZBR_DEFAULT_SETTING_MODE					  (2048)
#define ZBR_DEFAULT_SETTING_OFFSET_HOME				  (0)
#define ZBR_DEFAULT_SETTING_CURRENT_HOLD			  (20)
#define ZBR_DEFAULT_SETTING_CURRENT_LIM				  (10)
#define ZBR_DEFAULT_SETTING_CURRENT_RUN				  (10)
													  
#define ZBR_DEFAULT_PRODUCT_TRAVEL_RANGE_UM			  (150000) //150 mm
#define ZBR_DEFAULT_PRODUCT_STEPS_PER_REVOLUTION	  (200)
#define ZBR_DEFAULT_PRODUCT_MOTION_PER_REVOLUTION_UM  (6350)   //6.35 mm
													  
#define ZBR_SPEED_COEF								  (9.357)
#define ZBR_ACCELERATION_COEF						  (11250)
#define ZBR_SPEED_COEF_UPPER_LIMIT					  (512)








class ZDeviceSettings
{
	typedef ZDeviceSetting<int>    SettingNumeric;
	typedef ZDeviceSetting<string> SettingTextual;


	SettingNumeric m_productId;
	string         m_productName;
	SettingNumeric m_productSN;
	SettingNumeric m_positionWorkingMax;
	SettingNumeric m_acceleration;
	SettingNumeric m_speedTarget;
	SettingNumeric m_speedHome;
	SettingNumeric m_resolution;
	SettingNumeric m_mode;
	SettingNumeric m_offsetHome;
	SettingNumeric m_relativeMoveMax;
	SettingNumeric m_posOnPowerup;
	SettingNumeric m_current4Hold;
	SettingNumeric m_current4Limit;
	SettingNumeric m_current4Run;

	int m_productTravelRange_um;
	int m_productStepsPerRevolution;
	double m_productLinearMotionPerRevolution_um;

protected:
public:
	static const ZDeviceSettings NULL_DEVICE_SETTINGS;


private:
protected:
public:
	int GetProductId() const
	{
		return m_productId.GetValue();
	}
	const string& GetProductName() const
	{
		return m_productName;
	}
	int GetProductSN() const
	{
		return m_productSN.GetValue();
	}
	int GetPositionMaximal() const
	{
		return 0;
		//return m_positionMax.GetValue();
	}
	int GetAcceleration() const
	{
		return m_acceleration.GetValue();
	}
	int GetSpeedTarget() const
	{
		return m_speedTarget.GetValue();
	}
	int GetSpeedHome() const
	{
		return m_speedHome.GetValue();
	}
	int GetResolution() const
	{
		return m_resolution.GetValue();
	}
	ZDeviceSettings& SetResolution(int resolution)
	{
		m_resolution = SettingNumeric(m_resolution.GetCommandId(), resolution);

		return *this;
	}

	int GetMode() const
	{
		return m_mode.GetValue();
	}
	int GetOffsetHome() const
	{
		return m_offsetHome.GetValue();
	}
	int GetRelativeMoveMax() const
	{
		return m_relativeMoveMax.GetValue();
	}
	int GetPosOnPowerup() const
	{
		return m_posOnPowerup.GetValue();
	}
	int GetCurrent4Hold() const
	{
		return m_current4Hold.GetValue();
	}
	int GetCurrent4Limit() const
	{
		return m_current4Limit.GetValue();
	}
	int GetCurrent4Run() const
	{
		return m_current4Run.GetValue();
	}
	int GetProductTravelRange_UM() const
	{
		return m_productTravelRange_um;
	}
	int GetProductStepsPerRevolution() const
	{
		return m_productStepsPerRevolution;
	}
	double GetProductLinearMotionPerRevolution_um() const
	{
		return m_productLinearMotionPerRevolution_um;
	}
	ZDeviceSettings& SetProductStepsPerRevolution(int productStepsPerRevolution)
	{
		if(productStepsPerRevolution>0)
			m_productStepsPerRevolution = productStepsPerRevolution;

		return *this;
	}
	ZDeviceSettings& SetProductLinearMotionPerRevolution_um(double productLinearMotionPerRevolution_um)
	{
		if (productLinearMotionPerRevolution_um > 0)
			m_productLinearMotionPerRevolution_um = productLinearMotionPerRevolution_um;

		return *this;
	}

public:	   

	ZDeviceSettings(int productId = ZBR_DEFAULT_SETTING_PROD_ID
		, const string& productName = ZBR_DEFAULT_SETTING_PROD_NAME
		, int productSN = ZBR_DEFAULT_SETTING_PROD_SN
		, int acceleration = ZBR_DEFAULT_SETTING_ACCELERATION
		, int speedTarget = ZBR_DEFAULT_SETTING_SPEED_TRGT
		, int speedHome = ZBR_DEFAULT_SETTING_SPEED_HOME
		, int resolution = ZBR_DEFAULT_SETTING_RESOLUTION
		, int mode = ZBR_DEFAULT_SETTING_MODE
		, int offsetHome = ZBR_DEFAULT_SETTING_OFFSET_HOME
		, int current4Hold = ZBR_DEFAULT_SETTING_CURRENT_HOLD
		, int current4Limit = ZBR_DEFAULT_SETTING_CURRENT_LIM
		, int current4Run = ZBR_DEFAULT_SETTING_CURRENT_RUN
		, int productTravelRange_um = ZBR_DEFAULT_PRODUCT_TRAVEL_RANGE_UM
		, int productStepsPerRevolution = ZBR_DEFAULT_PRODUCT_STEPS_PER_REVOLUTION
		, int productLinearMotionPerREvolution = ZBR_DEFAULT_PRODUCT_MOTION_PER_REVOLUTION_UM
		)
		: m_productId(50,						productId								)
		, m_productName(						productName								)
		, m_productSN(63,						productSN								)	    
		, m_acceleration(43,					acceleration							)
		, m_speedTarget(42,						speedTarget								)
		, m_speedHome(41,						speedHome								)
		, m_resolution(37,						resolution								)
		, m_mode(40,							mode									)
		, m_offsetHome(47,						offsetHome								)
//		, m_relativeMoveMax(46,					1 * m_positionMax.GetValue()			)
//		, m_posOnPowerup(-1,					1 * m_positionMax.GetValue()			)
		, m_current4Hold(39,					current4Hold							)
		, m_current4Limit(-1,					current4Limit							)
		, m_current4Run(38,						current4Run								)
		, m_productTravelRange_um(				productTravelRange_um					)
		, m_productStepsPerRevolution(			productStepsPerRevolution				)
		, m_productLinearMotionPerRevolution_um(	productLinearMotionPerREvolution		)
		//, m_positionWorkingMax(44,				ZDeviceSettings::GetPositionMaximal()	) // ZBR_DEFAULT_SETTING_MAX_POS)
	{

	}

	virtual ~ZDeviceSettings(void)
	{

	}

	int GetSpeedDataUpperLimit() const
	{
		return (ZBR_SPEED_COEF_UPPER_LIMIT * GetResolution() - 1);
	}

	bool GetSpeed_microstepsPerSec(int speedData, __out double& speedMicrostepsPerSec) const
	{
		bool bRes = false;

		if(speedData > 0 && speedData < GetSpeedDataUpperLimit())
		{
			speedMicrostepsPerSec = speedData * ZBR_SPEED_COEF;
			bRes = true;
		}

		return bRes;
	}

	bool GetSpeedDataFromMicrostepsPerSec(double speedMicrostepsPerSec, __out int& speedData) const
	{
		bool bRes = false;

		int data = (int)(speedMicrostepsPerSec / ZBR_SPEED_COEF);
		if(data > 0 && data < GetSpeedDataUpperLimit())
		{
			speedData = data;
			bRes = true;
		}

		return bRes;
	}

	double GetResolution_umPerMicrostep() const
	{
		return (double)GetProductLinearMotionPerRevolution_um() / GetProductStepsPerRevolution() / GetResolution();
		//return ZBR_DEFAULT_PRODUCT_TRAVEL_RANGE_UM / (double)GetPositionMaximal();
	}

	int GetPositionMicrosteps(double positionMicrons) const
	{
		return (int)(positionMicrons / GetResolution_umPerMicrostep());
	}

	double GetPositionMicrons(int positionMicrosteps) const
	{
		return positionMicrosteps * GetResolution_umPerMicrostep();
	}
};

