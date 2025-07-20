#pragma once
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

#define KEY_VAL_DELIMITER                   ("=")
#define LIST_DELIMITER                      (",")

class ZUtils
{
public:
	ZUtils(void)
	{

	}
	~ZUtils(void)
	{

	}

	static bool HexByte2DecByte(const string& hexByte, __out int& decByte)
	{
		bool isDone = false;
		if(hexByte.length() == 2)
		{
			char* pErr = NULL;
			int dec = std::strtol(hexByte.c_str(), &pErr, 16);
			if(!strlen(pErr))
			{
				decByte = dec;
				isDone = true;
			}
		}
		return isDone;
	}

	static bool DecByte2HexByte(int decByte, __out string& hexByte)
	{
		bool isDone = false;
		ostringstream oss;
		oss << std::setfill ('0')
			<< std::setw(numeric_limits<int>::digits/12)
			<< std::hex
			<< decByte;
		if(oss.str().length() > 0)
		{
			hexByte = oss.str();
			isDone = true;
		}
		return isDone;
	}

	static string Tokenize(const string& src, 
    const string& delimiter, __in __out int& start)
	{
		int delimiterIndx = (int)src.find_first_of(delimiter, start);
		string res;
		if(delimiterIndx == std::basic_string<char>::npos)
		{
			res = src.substr(start, src.length() - start);;
			start = (int)std::basic_string<char>::npos;
		}
		else if(delimiterIndx > 0 && delimiterIndx != std::basic_string<char>::npos)
		{
			res = src.substr(start, delimiterIndx - start);
			start = delimiterIndx;
		}
		return res;
	}

	static bool GetKeyValue(const string& keyValPair, const string& delimiter, __out string& key, __out string& value)
	{
		int delimiterIndx = (int)keyValPair.find_first_of(delimiter, 0);
		bool res = false;
		if(delimiterIndx > 0 && delimiterIndx != std::basic_string<char>::npos)
		{
			key = keyValPair.substr(0, delimiterIndx);
			delimiterIndx++;
			value = keyValPair.substr(delimiterIndx, keyValPair.length() - delimiterIndx);
			res = true;
		}

		return res;
	}

	static const string ToLower(const string& src)
	{
		char* pLowerCase  = strdup(src.c_str());
		strlwr(pLowerCase);
		string lowerCase = pLowerCase;
		free(pLowerCase);
		return lowerCase;
	}
};

