#pragma once
#include <string>
#include <list>
#include <map>

using namespace std;

enum ColorIDs
{
	COLOR_UNKNOWN = 0,
	COLOR_RED = 1,
	COLOR_GREEN = COLOR_RED << 1,
	COLOR_BLUE = COLOR_GREEN << 1,
  COLOR_SPECIAL = COLOR_BLUE << 1,

	// ADD A NEW COLOR BEFORE
	CLRS_TOTAL_MASK = COLOR_RED | COLOR_GREEN | COLOR_BLUE | COLOR_SPECIAL
};

static const map<ColorIDs,string> COLORS_NAMES =
{
  { COLOR_UNKNOWN, "UNKNOWN"},
  { COLOR_RED, "Red"},
  { COLOR_GREEN, "Green"},
  { COLOR_BLUE, "Blue"},
  { COLOR_SPECIAL, "Special(Near Infra Red)"}
};
const map<ColorIDs, ColorIDs> getCollection();
static const map<ColorIDs, ColorIDs> COLORS_MASKS_BY_ID = getCollection();



//
struct ColorConfig //common for all colors
{
private: //make these values BYTE or short int?
	static const int MAXIMUM_LIGHT_TIME_US = 50000;
	static const int MINIMUM_LIGHT_TIME_US = 16;
	

  const int m_ledsMask;

	ColorIDs m_colorId;
	
	int m_ledsConfiguration;
	int m_light_Time_us;
  

	inline void setLEDsToOnConfig(int ledsToOnMask)
	{
		m_ledsConfiguration = ledsToOnMask & m_ledsMask;
	};
	inline void setLightTime_us(int lightTime_us)
	{
		m_light_Time_us = lightTime_us;

		if (lightTime_us > MAXIMUM_LIGHT_TIME_US)
			m_light_Time_us = MAXIMUM_LIGHT_TIME_US;
		else if (lightTime_us < MINIMUM_LIGHT_TIME_US)
			m_light_Time_us = MINIMUM_LIGHT_TIME_US;
	};

	static int createDefaultLEDsMask(int ledsForColor = ColorConfig::NUMBER_OF_LEDS_PER_COLOR);
public:
  static const int NUMBER_OF_LEDS_PER_COLOR = 4;

	static const ColorConfig Empty;

	inline static bool isValidNotZeroLEDsOnMask(int maskLEDsOn, int ledsInColor)
	{
    int allLedsMask = createDefaultLEDsMask(ledsInColor);
		return (allLedsMask & maskLEDsOn) > 0;
	}

	inline bool isEmpty() const
	{
		return m_colorId == ColorIDs::COLOR_UNKNOWN
			&& m_ledsConfiguration == 0
			&& m_light_Time_us == 0;
	}

	inline bool isColor(ColorIDs colorId) const
	{
		return m_colorId == colorId;
	}
	inline const ColorIDs& getColorID() { return m_colorId; }
	inline const ColorIDs& getColorID() const { return m_colorId; }
	inline string getName() const { return COLORS_NAMES.at(m_colorId); }
	inline int getLedsConfiguration() const { return m_ledsConfiguration; };
	inline int getLightTime_us() const { return m_light_Time_us; };

	inline int getLEDsMask(bool isSixPinsConnector) const
	{
		int res = 0;
		int colorRelatedMask = m_ledsConfiguration;
		int colorId = m_colorId;
		while (ColorIDs::COLOR_UNKNOWN + 1 < colorId)
		{
			colorRelatedMask <<= 1 * NUMBER_OF_LEDS_PER_COLOR;
			colorId >>= 1;
		}
		res = colorRelatedMask;

		if (isSixPinsConnector)
		{
			int sift = 2;
			switch (m_colorId)
			{
			case COLOR_GREEN:
			{
				int bitsToFrezeMask = 0x30;
				int bitsToMoveMask = 0xc0;

				int bitsToFreze = res & bitsToFrezeMask;
				int bitsToMove = res & bitsToMoveMask;

				res = bitsToMove << sift;
				res |= bitsToFreze;

				break;
			}
			case COLOR_BLUE:
      case COLOR_SPECIAL:
				res <<= sift;
				break;
			default:
				break;
			}
		}

		return res;
	}

	inline ColorConfig(ColorIDs colorId, int ledsToOn, int lightTime_us, int ledsInColor )//= ColorConfig::NUMBER_OF_LEDS_PER_COLOR)
		: m_colorId(colorId)
		, m_ledsConfiguration(0)
		, m_light_Time_us(0)
    , m_ledsMask(createDefaultLEDsMask(ledsInColor))
	{
		if (colorId != ColorIDs::COLOR_UNKNOWN && colorId != ColorIDs::CLRS_TOTAL_MASK)
		{
			setLEDsToOnConfig(ledsToOn);
			setLightTime_us(lightTime_us);
		}
	}
};


enum ModeIDs
{
	MODE_UNKNOWN = 0,
	MODE_VIDEO = 1,
	MODE_SNAPSHOT = MODE_VIDEO << 1,

	// ADD A NEW MODE BEFORE
	MODES_TOTAL_MASK = MODE_VIDEO | MODE_SNAPSHOT
};

static const map<ModeIDs,string> MODES_NAMES =
{
	{ MODE_UNKNOWN, "UNKNOWN"},
	{ MODE_VIDEO, "Video"},
	{ MODE_SNAPSHOT,"Snapshot"}
};

struct ModeConfigBase
{
private:
	ModeIDs                           m_modeId;
	string                            m_modeDescr;
	int                               m_timeToOnMax;

	list<const ColorConfig*>          m_pColors;
	map<ColorIDs, const ColorConfig*> m_mapByID;

	const list<const ColorConfig*> createColors(ModeIDs modeId, const list<int>& ledsToOnPerColor, const list<int>& timeToOnPerColor, const list<ColorIDs>& colorIDs) const;

	inline const map<ColorIDs, const ColorConfig*>& sortById(const list<const ColorConfig*>& colors, __out map<ColorIDs, const ColorConfig*>& mapByID)
	{
		mapByID.clear();
		list<const ColorConfig*>::const_iterator ci = colors.begin();
		for (; ci != colors.end(); ci++)
		{
			mapByID[(*ci)->getColorID()] = (*ci);
		}
		return mapByID;
	}

	inline int getMaxTimeToOn(const list<const ColorConfig*>& colors) const
	{
		int maxTime = 0;
		list<const ColorConfig*>::const_iterator ci = colors.begin();
		for (; ci != colors.end(); ci++)
		{
			if ((*ci)->getLightTime_us() > maxTime)
				maxTime = (*ci)->getLightTime_us();
		}
		return maxTime;
	}

	ModeConfigBase(const ModeConfigBase&);
	ModeConfigBase& operator=(const ModeConfigBase&);
protected:

	inline const ColorConfig& getColor(ColorIDs colorId) const
	{
		return *(m_mapByID.at(colorId));
	}

	inline string getAbbriviation() const
	{
		string res;
		for (list<const ColorConfig*>::const_iterator ci = m_pColors.begin(); ci != m_pColors.end(); ci++)
		{
			res.append((*ci)->getName(), 0, 1);
		}
		return res;
	}
public:
	static const ModeConfigBase Empty;

	inline bool isEmpty() const { return m_pColors.empty(); }

	inline const ModeIDs& getModeID() const
	{
		return m_modeId;
	}
	inline int getMaxLightOnDuration() const
	{
		return m_timeToOnMax;
	}
	inline const string& getModeName() const { return MODES_NAMES.at(m_modeId); }
	inline const string& getModeDescription() const { return m_modeDescr; }
	inline const ModeConfigBase& setModeDescription(const string& mmodeDescr)
	{
		m_modeDescr = mmodeDescr;
		return *this;
	}
	inline const list<const ColorConfig*>& getColors() const { return m_pColors; }
  inline bool isFullColor(/*int colorsPerRev*/) const
  {
    const int fullRGBcolorMask = ((int)ColorIDs::CLRS_TOTAL_MASK) & ~((int)ColorIDs::COLOR_SPECIAL);
    int res = fullRGBcolorMask;

    for each (const ColorConfig* pCC in m_pColors)
    {
      if (res == 0)
        break;
      res &= ~((int)pCC->getColorID());
    }

    return res == 0; /* m_pColors.size() == COLORS_MASKS_BY_ID.size();*/
  }
	
	inline bool isMode(ModeIDs modeId) const
	{
		return m_modeId == modeId;
	}

	inline virtual void clear()
	{
		m_mapByID.clear();
		for (list<const ColorConfig*>::iterator i = m_pColors.begin(); i != m_pColors.end(); i++)
		{
			delete (*i);
			(*i) = NULL;
		}
		m_pColors.clear();
		m_timeToOnMax = 0;
		m_modeDescr.clear();
		m_modeId = ModeIDs::MODE_UNKNOWN;
	}
	
	inline ModeConfigBase(ModeIDs modeId, const list<int>& ledsToOnPerColor, const list<int>& timeToOnPerColor, const list<ColorIDs>& colorsIDs)
		: ModeConfigBase(modeId, createColors(modeId, ledsToOnPerColor, timeToOnPerColor, colorsIDs))
	{
	}

	inline ModeConfigBase(ModeIDs modeId, const list<const ColorConfig*>& colors)
		: m_modeId(modeId)
		, m_modeDescr()
		, m_pColors(colors)
		, m_mapByID()
		, m_timeToOnMax()		
	{
		m_mapByID = sortById(m_pColors, __out m_mapByID);
		m_timeToOnMax = getMaxTimeToOn(m_pColors);
		string abbr = getAbbriviation();
		string temp("View");
			
		m_modeDescr = abbr.empty() ? "" : temp.append(abbr);
	}

	inline virtual ~ModeConfigBase()
	{
		clear();
	}
};

struct ModeConfigVideo : public ModeConfigBase
{
private:
  int                               m_frameRate;
public:
  inline int getFrameRate() const
  {
    return m_frameRate;
  }

  inline ModeConfigVideo(const list<int>& ledsToOnPerColor, int timeToOn, const list<ColorIDs>& colorsIDs, int frameRate)
    : ModeConfigBase::ModeConfigBase(ModeIDs::MODE_VIDEO, ledsToOnPerColor, { timeToOn, timeToOn, timeToOn, timeToOn }, colorsIDs)
    , m_frameRate(frameRate)
  {

  }

  inline virtual void clear()
  {
    ModeConfigBase::clear();
    m_frameRate = 0;
  }
};




class ModeConfigRepository
{
private:
	ModeConfigBase* m_pConfVideo;
	ModeConfigBase* m_pConfSnapshot;

	ModeConfigRepository(const ModeConfigRepository&);
	ModeConfigRepository& operator=(const ModeConfigRepository&);
public:
	static const ModeConfigRepository Empty;

	inline bool isEmpty() const
	{
		return !m_pConfVideo || !m_pConfSnapshot || m_pConfVideo->isEmpty() || m_pConfSnapshot->isEmpty();
	}

	inline void clear()
	{
		if(m_pConfVideo)
			m_pConfVideo->clear();
		if(m_pConfSnapshot)
			m_pConfSnapshot->clear();
	}

	inline const ModeConfigBase* getConfVideo() const
	{
		return m_pConfVideo;
	}
	inline ModeConfigRepository& setConfVideo(const list<int>& ledsToOnPerColor, int ledsToOnTime_us, const list<ColorIDs>& colorsIDs, int reqFrameRate)
	{
		m_pConfVideo = new ModeConfigVideo(ledsToOnPerColor,  ledsToOnTime_us, colorsIDs, reqFrameRate);
		return *this;
	}

	inline const ModeConfigBase* getConfSnapshot() const
	{
		return m_pConfSnapshot;
	}
	inline ModeConfigRepository& setConfSnapshot(const list<int>& ledsToOnPerColor, const list<int>& timeToOnPerColor, const list<ColorIDs>& colorsIDs)
	{
		m_pConfSnapshot = new ModeConfigBase(ModeIDs::MODE_SNAPSHOT, ledsToOnPerColor, timeToOnPerColor, colorsIDs);
		return *this;
	}

	inline ModeConfigRepository()
		: m_pConfVideo()
		, m_pConfSnapshot()
	{
			
	}

	inline ~ModeConfigRepository()
	{
		clear();

		delete m_pConfVideo;
		m_pConfVideo = NULL;
		delete m_pConfSnapshot;
		m_pConfSnapshot = NULL;
	}
};