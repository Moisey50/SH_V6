// CorrelatorOCRGadget.h : Implementation of the CorrelatorOCR class


#include "StdAfx.h"
#include "CorrelatorOCRGadget.h"
#include <imageproc\clusters\segmentation.h>
#include <imageproc\utilities.h>
#include <gadgets\vftempl.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "Pattern.h"
#include <direct.h>
//#include "fxlogsystem.h"
using namespace std;

IMPLEMENT_RUNTIME_GADGET_EX(CorrelatorOCR, CFilterGadget, "Radial", TVDB400_PLUGIN_NAME);

//Correlator is responsible for finding the correlation coefficient between a given pattern and a cut frame
CorrelatorOCR::CorrelatorOCR(void)
{
	m_pInput = new CInputConnector(transparent);
	m_pOutput = new COutputConnector(transparent);
	m_OutputMode = modeAppend;
	m_Mode = Learning;
	m_iHeight = 20;
	m_iWidth = 15;
	POSSIBLE_MODES[0] = "Learning";
	POSSIBLE_MODES[1] = "Recognition";
  m_sDirPath = "D:\\Correlator\\Output";

	Resume();
}

//Called every time the gadget is deleted
void CorrelatorOCR::ShutDown()
{
	CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	bool patternsEmpty = m_pPatterns.empty();
	if (!patternsEmpty)
	{
		for (int i = (int) m_pPatterns.size() - 1; i > 0; i--)
			delete m_pPatterns[i];

		m_pPatterns.clear();
	}
}

//Inserts the values in the input fields into their respective places in the BL
bool CorrelatorOCR::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	FXPropertyKit pk(text);
	pk.GetInt("Height", m_iHeight);
	pk.GetInt("Width", m_iWidth);
	pk.GetInt("Mode", (int&)m_Mode);

	return true;
}

//Displays the properties in the dialog window of the gadget. Called every time anything changes in the ENTIRE GRAPH
bool CorrelatorOCR::PrintProperties(FXString& text)
{
	LPCTSTR characterString;
	if (!m_pPatterns.empty())
	{
		characterString = (LPCTSTR)(m_pPatterns[0]->m_sLearnedString);
	}
	FXPropertyKit pk;
	CFilterGadget::PrintProperties(text);

	pk.WriteInt("Height", m_iHeight);
	pk.WriteInt("Width", m_iWidth);
	pk.WriteInt("Mode", (int)m_Mode);

	text += pk;
	return true;
}


//Determines which fields will appear in the dialog window and what default or possible values they can have
bool CorrelatorOCR::ScanSettings(FXString& text)
{
	text = "template("
		"Spin(Height,1,50)" //TODO: Determine logical max and min values for this
		",Spin(Width,1,30)" //TODO: Determine logical max and min values for this
		",ComboBox(Mode(Learning(0),Recognition(1)))"
		")";
	return true;
}

bool DoesDirectoryExist(LPCTSTR path)
{
	DWORD attributes = GetFileAttributes(path);
	
	if (attributes == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return false;
	}

	return true;
}

//Grabs a cut frame and either stores it as a determined pattern (Learning) or compares it to existing patterns, ouputting the most suitable one (Recognition)
CDataFrame* CorrelatorOCR::DoProcessing(const CDataFrame* pDataFrame) //Don't forget to fix return and parameters
{
	FXString message;
	CTextFrame * pTextFrame = NULL;
	if (!pDataFrame || !m_iHeight || !m_iWidth)
		return NULL;

	const CVideoFrame* pVideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);

	if (pVideoFrame) //TODO: Add try-catch block?
	{
		TRACE(" pDataFrame has content ");
		TRACE(" This is the start of Do Processing ");
		TRACE(pDataFrame->GetLabel());

		ASSERT(m_Mode == Recognition || m_Mode == Learning);

		if (m_Mode == Learning)
		{
			TRACE("Inside Learning Mode");

			Pattern* ppattern = new Pattern(m_iHeight, m_iWidth, *pVideoFrame, new FXString(pDataFrame->GetLabel()));
			
			m_pPatterns.push_back(ppattern);

			string dirPath = "D:\\Correlator\\Output\\Patterns"; //TODO: Get dirPath from user?

			if (DoesDirectoryExist(dirPath.c_str()) == false)
			{
				CreateDirectory(dirPath.c_str(), NULL);
			}
			if (DoesDirectoryExist(dirPath.c_str()))
			{
				string fileName = dirPath + "\\pattern_" + to_string(m_pPatterns.size() - 1) + ".ext";

				fstream stream(fileName, ios::out | ios::trunc | ios::binary);

				if (stream.is_open())
				{
					ppattern->Serialize(stream);
					stream.close();
				}

				// Test deserialization

				fstream testRead(fileName, ios::in | ios::binary);
				Pattern* testPattern = new Pattern();
				testPattern->Deserialize(testRead);
				SENDINFO("what's in the testPattern:");
				SENDINFO((LPCTSTR)testPattern->m_sLearnedString);
				testRead.close();
			}	
			else
			{
				throw new exception(); // handle a case where directory cannot be created because reasons
			}


			LPCTSTR symbol = *(m_pPatterns.back()->m_sLearnedString);

			pTextFrame = CTextFrame::Create(symbol); //Outputs the symbol
			TRACE("\n End of Learning pText=%p" , pTextFrame );
			SENDINFO(symbol);
			// TODO: Write/overwrite m_pPatterns to file for every new pattern created ?
		}
		else if (m_Mode == Recognition)
		{
			SENDINFO("Inside Recognition Mode");

			Pattern* ppattern = new Pattern(m_iHeight, m_iWidth, *pVideoFrame);
			double dCoef = 0.0; 

			//If this is the first time the program runs, import m_pPatterns from the relevant file

			if (!m_pPatterns.empty())
			{
				FXString Addition, character;
				for (UINT i = 0; i < m_pPatterns.size(); i++)
				{
					double tempCoef = ppattern->Correlate(*m_pPatterns[i]);
					if (fabs(tempCoef) > fabs(dCoef)) 
					{
						dCoef = tempCoef;
						character = *m_pPatterns[i]->m_sLearnedString;
					}
				}
				ppattern->m_sLearnedString = &character; 
				Addition.Format("The correlation coef to %s is %f.\n",
					(LPCTSTR)character,
					dCoef); 
				message += Addition;
				pTextFrame = CTextFrame::Create(message);
				TRACE("\n End of Recognition pText=%p", pTextFrame);
				PutFrame(m_pOutput, pTextFrame);
			}
			else
			{
				//Throw an exception about there being nothing to compare to
				SENDINFO("********** There are no patterns to compare to ********");
				message = "There are no patterns to compare to";
				pTextFrame = CTextFrame::Create((LPCTSTR)message);
				TRACE("\n End of else in Recognition pText=%p", pTextFrame);
				PutFrame(m_pOutput, pTextFrame);
			}
		}
	}
	else
	{
		SENDINFO("**********Frame is empty***********");
		//throw exception(); // IS THIS RIGHT?
	}

	return pTextFrame; //BREAKS HERE. FIX!!!
}
