#include "stdafx.h"
#include "ConfigurationFactory.h"

const ModeConfigRepository* ConfigurationFactory::deserialize(const string & configInput, __out string & err)
{
	ModeConfigRepository* pModes=NULL;

	istringstream iss(configInput);
	vector<string> subProgs((istream_iterator<string>(iss)), istream_iterator<string>());

	if (subProgs.size() != INPUT_DATA_FIELDS::TOTAL_FIELDS_IN_INPUT)
	{
		FXString errMsg;
		errMsg.Format(" The fields quantity (%d) is wrong. Number of congigurations should be %d in format [dd dd dd dd dd dddd dd dd dd dddd dddd dddd]."
			, subProgs.size()
			, INPUT_DATA_FIELDS::TOTAL_FIELDS_IN_INPUT);
		err = errMsg;
	}
	else
	{
		try
		{
			pModes = new ModeConfigRepository();

			pModes->setConfVideo
			(
        {
          stoi(subProgs[INPUT_DATA_FIELDS::LEDS_TO_ON_VIDEO_RED])
        , stoi(subProgs[INPUT_DATA_FIELDS::LEDS_TO_ON_VIDEO_GREEN])
        , stoi(subProgs[INPUT_DATA_FIELDS::LEDS_TO_ON_VIDEO_BLUE])
        , stoi(subProgs[INPUT_DATA_FIELDS::LEDS_TO_ON_VIDEO_SPECIAL])
        }
				, stoi(subProgs[INPUT_DATA_FIELDS::TIME_TO_ON_VIDEO])
        ,
        {
          ColorIDs::CLRS_TOTAL_MASK
        }
        , stoi(subProgs[INPUT_DATA_FIELDS::FRAMES_PER_SECOND])
			);

			pModes->setConfSnapshot
			(
        {
          stoi(subProgs[INPUT_DATA_FIELDS::LEDS_TO_ON_SNAPSHOT_RED])
        , stoi(subProgs[INPUT_DATA_FIELDS::LEDS_TO_ON_SNAPSHOT_GREEN])
        , stoi(subProgs[INPUT_DATA_FIELDS::LEDS_TO_ON_SNAPSHOT_BLUE])
        },
        {
          stoi(subProgs[INPUT_DATA_FIELDS::TIME_TO_ON_SNAPSHOT_RED])
        , stoi(subProgs[INPUT_DATA_FIELDS::TIME_TO_ON_SNAPSHOT_GREEN])
        , stoi(subProgs[INPUT_DATA_FIELDS::TIME_TO_ON_SNAPSHOT_BLUE])
        }
        ,
        {
          ColorIDs::COLOR_RED
        , ColorIDs::COLOR_GREEN
        , ColorIDs::COLOR_BLUE
        }
			);
		}
		catch (exception ex)
		{
			err = "The configurations format is wrong.\rCheck that all data are numbers in format [dd dd dd dddd dd dd dd dddd dddd dddd].";
			err.append("\r").append(ex.what());
		}
	}

	subProgs.clear();

	return pModes;
}
