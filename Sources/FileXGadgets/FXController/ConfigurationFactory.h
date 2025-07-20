#pragma once

#include <iterator>  
#include <vector>  
#include <iostream>
#include <sstream>
#include <string>
#include <fxfc/fxfc.h>
#include "ModeConfigRepository.h"

using namespace std;

enum INPUT_DATA_FIELDS
{
	LEDS_TO_ON_VIDEO_RED = 0,
	LEDS_TO_ON_VIDEO_GREEN,
	LEDS_TO_ON_VIDEO_BLUE,
  LEDS_TO_ON_VIDEO_SPECIAL, //by request of Oded (20180805) added single LED of Near Infra Red
  FRAMES_PER_SECOND,        //by request of Oded (20180927) added Video Mode Frame Rate
	TIME_TO_ON_VIDEO,

	LEDS_TO_ON_SNAPSHOT_RED,
	LEDS_TO_ON_SNAPSHOT_GREEN,
	LEDS_TO_ON_SNAPSHOT_BLUE,
	TIME_TO_ON_SNAPSHOT_RED,
	TIME_TO_ON_SNAPSHOT_GREEN,
	TIME_TO_ON_SNAPSHOT_BLUE,

	// ADD NEW INPUT INDEXES BEFORE
	TOTAL_FIELDS_IN_INPUT
};

//enum INPUT_DATA_FIELDS
//{
//  INTENSITY_PERC_X100_R = 0,
//  INTENSITY_PERC_X100_Y,
//  INTENSITY_PERC_X100_G,
//  INTENSITY_PERC_X100_IR,
//
//  EXPOSURE_MICROSECS_R,
//  EXPOSURE_MICROSECS_Y,
//  EXPOSURE_MICROSECS_G,
//  EXPOSURE_MICROSECS_IR,
//
//  PERIOD_MICROSECS,
//  TOTAL_FIELDS_IN_INPUT
//};

class ConfigurationFactory
{
private:


public:
	ConfigurationFactory(){}
	virtual ~ConfigurationFactory(){}

	static const ModeConfigRepository* deserialize(const string& configInput, __out string& err);
};