#pragma once
#include "ZCommandHeader.h"
#include "ZCommandsHeadersRepository.h"
#include "ZDeviceSettings.h"


class ZCommandBase;

typedef ZCommandBase* (__stdcall *FnDeserializeByFactory)(const ZCommandHeader* pHeader, int rawData, const ZDeviceSettings& settings);

class ZCommandBase
{
private:
	const ZCommandHeader* m_pHeader;
	int                   m_Data;
	eCommandDataType      m_simplifiedDataType;

protected:	
public:

private:

	ZCommandBase& SetHeader(const string& cmdIdTxt, const ZCommandsHeadersRepository& headers)
	{
		int cmdId;
		if(cmdIdTxt.length() == 2 && ZUtils::HexByte2DecByte(cmdIdTxt, cmdId))
			return SetHeader(cmdId, headers);

		return *this;
	}

	ZCommandBase& SetHeader(short cmdId, const ZCommandsHeadersRepository& headers)
	{
		const ZCommandHeader& header = headers[cmdId];
		if(!ZCommandHeader::IsEmptyCommand(header))
			m_pHeader = &header;
		return *this;
	}

	ZCommandBase& SetData(const string& cmdDataTxt)
	{
		int hexBytesLen = (int) cmdDataTxt.length();
		if(hexBytesLen == 8)
		{
			string hexBytes = cmdDataTxt;			
			int decByte = 0;
			int bytePos = 3;
			int decData = 0;

			while(hexBytesLen && ZUtils::HexByte2DecByte(hexBytes.substr(hexBytesLen - 2, 2), decByte))
			{
				decData |= decByte << 8 * bytePos;
				bytePos--;
				hexBytes.erase(hexBytesLen - 2, 2);
				hexBytesLen = (int) hexBytes.length();
			}

      if ( !hexBytesLen && bytePos == -1 )
      {
        // DeviceMessage message = new DeviceMessage( aDataPacket );
        // if ( ( MessageType.Binary == message.MessageType ) && AreMessageIdsEnabled )
        // {

        // Take most significant byte as Id. //Yuris_20171210 THEREFORE NOT IN USE
        int messageId = ( BYTE )( decData >> 24 );

        // 255 or 0 means no Id. //Yuris_20171210 THEREFORE NOT IN USE
        if ( messageId == 255 )
        {
          messageId = 0; //Yuris_20171210 THEREFORE NOT IN USE
        }

        //Extract data as 3 last bytes;
        decData = decData & 0xFFFFFF;

        // negative data has to be extended back into the most 
        // significant bit.
        if ( ( decData & 0x800000 ) != 0 )
        {
          decData |= -0x1000000;
        }
        // }

        m_Data = decData;
      }
		}

		return *this;
	}
protected:

	ZCommandBase& SetSimplifiedDataType(eCommandDataType dataType)
	{
		m_simplifiedDataType = dataType;
		return *this;
	}

public:
	eCommandDataType GetSimplifiedDataType() const
	{
		return m_simplifiedDataType;
	}

	ZCommandBase& SetHeader(const ZCommandHeader* pHeader)
	{
		m_pHeader = pHeader;

		return *this;
	}
	const ZCommandHeader* GetHeader() const
	{
		return m_pHeader;
	}

	int GetData() const
	{
		return m_Data;
	}
	ZCommandBase& SetData(int rawData)
	{
		m_Data = rawData;

		return *this;
	}
private:
	ZCommandBase(const ZCommandBase&);
	ZCommandBase& operator=(const ZCommandBase&);

protected:
public:
	ZCommandBase(short cmdId = COMMAND_ID_NONE, int data = 0, 
    const ZCommandsHeadersRepository* pHeaders = NULL)
		: m_pHeader(NULL)
		, m_Data(data)
		, m_simplifiedDataType(CDT_NONE)
	{
		if(cmdId != COMMAND_ID_NONE && pHeaders!=NULL)
			SetHeader(cmdId, *pHeaders);

	}
	virtual ~ZCommandBase(void)
	{

	}

private:

protected:
	string SerializeData() const
	{
		ostringstream oss;

		int lastByte = !GetHeader() || GetHeader()->GetCommandDataType2Send() == eCommandDataType::CDT_NONE ? 0 : m_Data;
		
		int bytesToSerialize = 4;

		for (int byteIdx = 0; byteIdx < bytesToSerialize; byteIdx++)
		{			
			string hexByte;
			if(!ZUtils::DecByte2HexByte(lastByte & 0xff, hexByte))
				hexByte = "00";
		
			oss << hexByte;
			lastByte >>= 8;
		}

		return oss.str();
	}


public:

	static const ZCommandBase* Deserialize(const string& cmdTxt, const ZCommandsHeadersRepository& headers)
	{
		ZCommandBase* pRes = NULL;
		const int digitsPerByte = 2;
		const int bytes4CmdOnly = 1;
		const int bytes4DataOnly = 4;
		const int bytes4CmdWithData = 5;
		const int digits4CmdWithData = bytes4CmdWithData * digitsPerByte;
		if(cmdTxt.length() == digits4CmdWithData)
		{
			pRes = new ZCommandBase();
			pRes->SetHeader(cmdTxt.substr(0, bytes4CmdOnly*digitsPerByte), headers);
			pRes->SetData(cmdTxt.substr(bytes4CmdOnly*digitsPerByte, bytes4DataOnly*digitsPerByte));
		}
		return pRes;
	}

	static const ZCommandBase* Deserialize(const string& cmdName, const string& dataTxt, const ZCommandsHeadersRepository& headers)
	{
		ZCommandBase* pRes = NULL;

		if(!cmdName.empty())
		{
			string instructionName = ZCommandHeader::NormalizeInstructionName(cmdName);
			const ZCommandHeader& header = headers[instructionName];

			if(!ZCommandHeader::IsEmptyCommand(header))
			{
				pRes = new ZCommandBase();
				pRes->m_pHeader = &header;
				//TODO pRes->SetData(cmdTxt.substr(2, 8));
				pRes->m_Data = atoi(dataTxt.c_str());
			}
		}
		return pRes;
	}

	virtual string ToString() const
	{
		ostringstream oss;

		if(!GetHeader())
			oss << "NO COMMAND";
		else
		{
			oss << GetHeader()->ToString() << KEY_VAL_DELIMITER << GetData();
		}
		return oss.str();
	}

	virtual string Serialize() const
	{
		ostringstream oss;
		
		if(GetHeader())
		{
			oss << GetHeader()->Serialize() << SerializeData();
		}
		return oss.str();
	}

};

