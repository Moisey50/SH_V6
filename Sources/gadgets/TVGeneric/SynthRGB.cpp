#include "stdafx.h"
#include "SynthRGB.h"
#include <gadgets\VideoFrame.h>

__forceinline pTVFrame _synth_rgb(pTVFrame RFrame, pTVFrame GFrame, pTVFrame BFrame)
{
	if (memcmp(RFrame->lpBMIH, GFrame->lpBMIH, RFrame->lpBMIH->biSize) ||
		memcmp(RFrame->lpBMIH, BFrame->lpBMIH, RFrame->lpBMIH->biSize))
	{
		return makecopyTVFrame(RFrame);
	}
	LPBYTE R = GetData(RFrame);
	LPBYTE G = GetData(GFrame);
	LPBYTE B = GetData(BFrame);
	int width = RFrame->lpBMIH->biWidth;
	int height = RFrame->lpBMIH->biHeight;
	int width4 = ((width + 3) / 4) * 4;
	LPBITMAPINFOHEADER pBMIH = (LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + 3 * width4 * height);
	pBMIH->biBitCount = 24;
	pBMIH->biClrImportant = 0;
	pBMIH->biClrUsed = 0;
	pBMIH->biCompression = BI_RGB;
	pBMIH->biHeight = height;
	pBMIH->biPlanes = 1;
	pBMIH->biSize = sizeof(BITMAPINFOHEADER);
	pBMIH->biSizeImage = 0;
	pBMIH->biWidth = width;
	pBMIH->biXPelsPerMeter = 0;
	pBMIH->biYPelsPerMeter = 0;
	LPBYTE dst = (LPBYTE)pBMIH + pBMIH->biSize;
	if (width == width4)
	{
		LPBYTE end = dst + 3 * width4 * height;
		while (dst < end)
		{
			*dst++ = *B++;
			*dst++ = *G++;
			*dst++ = *R++;
		}
	}
	else
	{
		memset(dst, 0, 3 * width4 * height);
		int y;
		LPBYTE end;
		for (y = 0; y < height; y++)
		{
			end = R + width;
			while (R < end)
			{
				*dst++ = *B++;
				*dst++ = *G++;
				*dst++ = *R++;
			}
			dst += 3 * (width4 - width);
		}
	}
	LPBYTE src = (LPBYTE)pBMIH + pBMIH->biSize;
	dst = src + 3 * width4 * (height - 1);
	int cb = 3 * width4;
	LPBYTE buf = (LPBYTE)malloc(cb);
	while (src < dst)
	{
		memcpy(buf, src, cb);
		memcpy(src, dst, cb);
		memcpy(dst, buf, cb);
		src += cb;
		dst -= cb;
	}
	free(buf);
	pTVFrame Result = newTVFrame(rgb24yuv9(pBMIH));
	free(pBMIH);
	if (!Result)
		return makecopyTVFrame(RFrame);
	return Result;
}

IMPLEMENT_RUNTIME_GADGET_EX(SynthRGB, CFilterGadget, LINEAGE_VIDEO".conversion", TVDB400_PLUGIN_NAME);

SynthRGB::SynthRGB():
m_Stack(3)
{
	m_pInput = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(vframe);
	Resume();
}

void SynthRGB::ShutDown()
{
	CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	FXAutolock ALock(m_Lock);
	ClearStack();
}

void SynthRGB::ClearStack()
{
	while (m_Stack.ItemsInQueue())
	{
		CVideoFrame* df;
		if (m_Stack.GetQueueObject(df))
		{
			df->Release(df);
		}
	}
}

CDataFrame* SynthRGB::DoProcessing(const CDataFrame* pDataFrame)
{
	FXAutolock al(m_Lock);
	CVideoFrame* vFrame = (CVideoFrame*)pDataFrame->GetVideoFrame();
	if (m_Stack.PutQueueObject(vFrame))
		vFrame->AddRef();
	if (m_Stack.ItemsInQueue() == 3)
	{
		CVideoFrame *RFrame, *GFrame, *BFrame;
		m_Stack.GetQueueObject(RFrame);
		m_Stack.GetQueueObject(GFrame);
		m_Stack.GetQueueObject(BFrame);
		CVideoFrame* Result = CVideoFrame::Create(_synth_rgb(RFrame, GFrame, BFrame));
		Result->ChangeId(RFrame->GetId());
		Result->SetTime(RFrame->GetTime());
		Result->SetLabel(RFrame->GetLabel());
		RFrame->Release(RFrame);
		GFrame->Release(GFrame);
		BFrame->Release(BFrame);
		return Result;
	}
	return NULL;
}
