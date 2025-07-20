#include "stdafx.h"
#include "Pattern.h"
#include "Math\Intf_sup.h"
#include <helpers\FramesHelper.h>

Pattern::Pattern()
{
	m_iPatternHeight = 20;
	m_iPatternWidth = 15;
}

Pattern::~Pattern()
{
	//TODO: Check if this is correct
	m_pNormPattern = NULL;
	delete m_pNormPattern;
}

Pattern::Pattern(int height, int width, const CVideoFrame& frame, FXString * s)
{
	if (s!=NULL && !(s->IsEmpty()))
	{
		m_sLearnedString = s;
	}
	else
	{
		m_sLearnedString = NULL;
	}

	if (height && width)
	{
		m_iPatternHeight = height;
		m_iPatternWidth = width;
	}

	ASSERT(frame.lpBMIH);
	BYTE* pImage = GetData(&frame); 
	int size = GetImageSize(&frame);

	bool b16 = Is16BitsImage(&frame);

	int iFrameWidth = GetWidth(&frame);
	int iFrameHeight = GetHeight(&frame);
	int iPatternSize = m_iPatternHeight * m_iPatternWidth;

	m_pNormPattern = new double[iPatternSize];
	double dKx = (double)iFrameWidth / (double)m_iPatternWidth;
	double dKy = (double)iFrameHeight / (double)m_iPatternHeight;
	double dSum = 0., dSum2 = 0.;
	if (Is8BitsImage(&frame))
	{
		for (int iYp = 0; iYp < m_iPatternHeight; iYp++)
		{
			double * pRow_p = m_pNormPattern + iYp * m_iPatternWidth;
			LPBYTE pRow_o = pImage + ROUND(iYp * dKy * iFrameWidth);

			for (int iXp = 0; iXp < m_iPatternWidth; iXp++)
			{
				// 			*(pRow_p++) = 2.0 * (((double)(*(pRow_o + ROUND(iXp * dKx)) - min) / dDelta) - 0.5);
				double dValue = *(pRow_p++) = *(pRow_o + ROUND(iXp * dKx));
				dSum += dValue;
			}
		}
	}
	else if (Is16BitsImage(&frame))
	{
		for (int iYp = 0; iYp < m_iPatternHeight; iYp++)
		{
			double * pRow_p = m_pNormPattern + iYp * m_iPatternWidth;
			LPWORD pRow_o = ((LPWORD)pImage) + ROUND(iYp * dKy * iFrameWidth);

			for (int iXp = 0; iXp < m_iPatternWidth; iXp++)
			{
				// 			*(pRow_p++) = 2.0 * (((double)(*(pRow_o + ROUND(iXp * dKx)) - min) / dDelta) - 0.5);
				double dValue = *(pRow_p++) = *(pRow_o + ROUND(iXp * dKx));
				dSum += dValue;
			}
		}
	}
	else
	{
		m_dSqSum = 0;
		return;
	}

	m_dAverage = dSum / iPatternSize;
	double *p = m_pNormPattern;

	for (int i = 0; i < iPatternSize; i++)
	{
		double dValue2 = (*(p++) -= m_dAverage);
		dSum2 += dValue2 * dValue2;
	}
	m_dSqSum = dSum2;
}

double Pattern::Correlate(Pattern& pattern)
{
	double result = 0.0;
	int thisLength = m_iPatternHeight * m_iPatternWidth;
	if (m_dSqSum < 1.e-5 || pattern.m_dSqSum < 1.e-5)
		return 0.;

	int patternLength = pattern.m_iPatternHeight * pattern.m_iPatternWidth;

	if (thisLength == patternLength)
	{
		double sum = 0;
		double * pThis = m_pNormPattern;
		double * pPatt = pattern.m_pNormPattern;
		for (int i = 0; i < thisLength; i++)
			sum += *(pThis++) * (*(pPatt++));

		sum /= sqrt(m_dSqSum * pattern.m_dSqSum);
		result = sum; // 1. - sum ;
	}
	else
	{
		return 0.;
	}

	return result;
}

void Pattern::Serialize(fstream& stream)
{
	stream.write( ( char* )&m_iPatternHeight, sizeof( int ) );
	stream.write( ( char* )&m_iPatternWidth, sizeof( int ) );
	stream.write( ( char* )&m_dAverage, sizeof( double ) );
	stream.write( ( char* )&m_dSqSum, sizeof( double ) );

	int size = m_iPatternHeight * m_iPatternWidth;
	stream.write( ( char* )&size, sizeof(int));
	stream.write( ( char* )m_pNormPattern, size*( sizeof( double )) );

	int stringLength = (int) m_sLearnedString->GetLength();
	stream.write((char*)&stringLength, sizeof(int));
	stream.write((char*)m_sLearnedString->GetString(), stringLength*(sizeof(char)));
}

void Pattern::Deserialize(fstream& stream)
{
	stream.read( ( char* )&m_iPatternHeight, sizeof( int ) );
	stream.read( ( char* )&m_iPatternWidth, sizeof(int));
	stream.read( ( char* )&m_dAverage, sizeof(double));
	stream.read( ( char* )&m_dSqSum, sizeof(double));
	
	int size = 0;
	stream.read((char*)&size, sizeof(int));

	m_pNormPattern = new double[size];
	stream.read((char*)m_pNormPattern, size*(sizeof(double)));

	int stringLength = 0;
	stream.read((char*)&stringLength, sizeof(int));

	char* lstring = new char[stringLength];
	stream.read(lstring, stringLength*(sizeof(char)));
	m_sLearnedString = new FXString(lstring); //TODO: Fix jibberish at the end of string
}

