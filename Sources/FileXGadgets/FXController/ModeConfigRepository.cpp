#include "stdafx.h"
#include "ModeConfigRepository.h"

const ColorConfig ColorConfig::Empty = ColorConfig(ColorIDs::COLOR_UNKNOWN, 0, 0, ColorConfig::NUMBER_OF_LEDS_PER_COLOR);

const ModeConfigBase ModeConfigBase::Empty = ModeConfigBase(ModeIDs::MODE_UNKNOWN, list<const ColorConfig*>());

const map<ColorIDs, ColorIDs> getCollection()
{
	map<ColorIDs, ColorIDs> target = map<ColorIDs, ColorIDs>();
	int c = 0;
	// collects all colors masks to the map
	for (int m = ColorIDs::COLOR_UNKNOWN + 1; c < ColorIDs::CLRS_TOTAL_MASK; m <<= 1, c |= m)
	{
		target[(ColorIDs)m] = (ColorIDs)m;
	};
	return target;
}

const ModeConfigRepository ModeConfigRepository::Empty = ModeConfigRepository();

int ColorConfig::createDefaultLEDsMask(int ledsForColor)
{
	int res = 0;
	for (int i = 0; i < ledsForColor; i++)
	{
		res <<= 1;
		res |= 1;
	}
	return res;
}

const list<const ColorConfig*> ModeConfigBase::createColors(ModeIDs modeId, const list<int>& ledsToOnPerColor, const list<int>& timeToOnPerColor, const list<ColorIDs>& colorsIDs) const
{
	list<const ColorConfig*> colors = list<const ColorConfig*>();
	int clrPresenceMaskActual = 0;
	const map<ColorIDs,ColorIDs>& clrPresenceMasks= COLORS_MASKS_BY_ID;
  int clrPresenceMaskAll = 0; /*= ColorIDs::CLRS_TOTAL_MASK*/
  for each (int var in colorsIDs)
  {
    clrPresenceMaskAll |= var;
  }

  const static int NUMBER_OF_LEDS_IN_SPECIAL_COLOR = 1;

	// creates the mask of input colors (not independent leds)
  int l = ColorIDs::COLOR_RED;
	for (list<int>::const_iterator ci = ledsToOnPerColor.begin(); ci != ledsToOnPerColor.end(); ci++, l<<=1)
	{
		if (*ci > 0 && ColorConfig::isValidNotZeroLEDsOnMask(*ci, ((ColorIDs)l)==ColorIDs::COLOR_SPECIAL ? NUMBER_OF_LEDS_IN_SPECIAL_COLOR : ColorConfig::NUMBER_OF_LEDS_PER_COLOR))
		{
			int clrMask = clrPresenceMasks.at((ColorIDs)l);
			clrPresenceMaskActual |= clrMask;
		}
	};

	bool isAll = (clrPresenceMaskAll == clrPresenceMaskActual);

	if (isAll || ModeIDs::MODE_VIDEO == modeId)
	{
		map<ColorIDs, ColorIDs>::const_iterator ciColorMask = clrPresenceMasks.begin();
		list<int>::const_iterator ciLeds = ledsToOnPerColor.begin();
		list<int>::const_iterator ciTime = timeToOnPerColor.begin();
		for (; ciColorMask != clrPresenceMasks.end() && ciLeds != ledsToOnPerColor.end() && ciTime != timeToOnPerColor.end(); ciColorMask++, ciLeds++, ciTime++)
		{
			if (isAll  || ((clrPresenceMaskActual & (int)(ciColorMask->second)) == (int)(ciColorMask->second)))
			{
				ColorConfig* pcc = new ColorConfig(ciColorMask->second, *ciLeds, *ciTime, ciColorMask->first == ColorIDs::COLOR_SPECIAL ? NUMBER_OF_LEDS_IN_SPECIAL_COLOR : ColorConfig::NUMBER_OF_LEDS_PER_COLOR);
				colors.push_back(pcc);
			}
		}
	}
	
	return colors;
}