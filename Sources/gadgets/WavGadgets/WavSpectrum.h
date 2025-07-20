#pragma once
#include <gadgets\gadbase.h>
#include <gadgets\waveframe.h>

class WavSpectrum : public CFilterGadget
{
	FXLockObject m_Lock, m_BufferLock;
	void*       m_pBuffer;
	DWORD       m_BufferSize;
	DWORD       m_DataWritten;
	BYTE        m_nBitsPerSample;
	DWORD       m_nRange;	// FFT window size = 2^range
	double*     m_pRe;
	double*     m_pIm;
	double*     m_LUT; // look-up table

public:
	void ShutDown();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);
private:
	WavSpectrum();
	void Reset();
	void AppendData(void* pData, DWORD cData);
	void* GetLastBlock();
	void DropBlock();
	void CountSpectrum(void* pData);

	DECLARE_RUNTIME_GADGET(WavSpectrum);
};

class ReIm2AmpPh : public CFilterGadget
{
public:
	void ShutDown();

private:
	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	ReIm2AmpPh();

	DECLARE_RUNTIME_GADGET(ReIm2AmpPh);
};

class Spectrum2Wave : public CFilterGadget
{
	int     m_nRange;
	double* m_pRe;
	double* m_pIm;
	bool    m_bStartLoop;
	double* m_LUT; // look-up table
public:
	void ShutDown();

private:
	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	pWaveData CalcFromAmpPh(const CArrayFrame* AmpFrame, const CArrayFrame* PhFrame);
	pWaveData CalcFromAmp(const CArrayFrame* AmpFrame);
	pWaveData CalcFromReIm(const CArrayFrame* ReFrame, const CArrayFrame* ImFrame);
	bool InitBlock(DPOINT* Re, DPOINT* Im, int count, bool bCopyData);
	pWaveData CreateWave(double fMax);
	Spectrum2Wave();

	DECLARE_RUNTIME_GADGET(Spectrum2Wave);
};
