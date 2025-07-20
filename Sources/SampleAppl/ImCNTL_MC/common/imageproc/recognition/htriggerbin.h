#ifndef HTRIGGERBIN_INC
#define HTRIGGERBIN_INC

#define TRIGGER_THRESHOLD 20

__forceinline unsigned char triggerval(unsigned a)
{
	a+=TRIGGER_THRESHOLD;
	return (a>255)?255:a;
}

__forceinline pTVFrame htriggerbin(pTVFrame  org)
{
	pTVFrame res= new TVFrame;
	res->lpBMIH=(LPBITMAPINFOHEADER)malloc(getsize4BMIH(org));
	res->lpData=NULL;
	memcpy(res->lpBMIH,org->lpBMIH,org->lpBMIH->biSize);
	LPBYTE dst=GetData(res);
	LPBYTE src=GetData(org);
	int h=res->lpBMIH->biHeight;
	int w=res->lpBMIH->biWidth;
	LPBYTE uv=dst+h*w;
	if (res->lpBMIH->biCompression==BI_YUV9)
		memset(uv,128,h*w/8);
	for (int y=0; y<h; y++)
	{
		LPBYTE s=src,e=src;
		bool increase=(*e<=*(e+1));
		int x;
		#ifdef _DEBUG
		LPBYTE eols=s+w;
		LPBYTE eold=dst+w;
		#endif
		for (x=0; x<w-1; x++)
		{
			if (increase)
			{
				if (*e<=triggerval(*(e+1)))
				{
					e++;
					if (x==w-2)
					{
						while (s<e)
						{
							*dst=255;
							s++; dst++;
						}
						src=e;
					}
				}
				else
				{
					e++;
					unsigned char mid=(*s+*e)/2;
					while (s<e)
					{
						*dst=(*s<mid)?0:255;
						s++; dst++;
					}
					src=e;
					increase=false;
				}
			}
			else
			{
				if (triggerval(*e)>=*(e+1))
				{
					e++;
					if (x==w-2)
					{
						while (s<e)
						{
							*dst=255;
							s++; dst++;
						}
						src=e;
					}
				}
				else
				{
					e++;
					unsigned char mid=(*s+*e)/2;
					while (s<e)
					{
						*dst=(*s<mid)?0:255;
						s++; dst++;
					}
					src=e;
					increase=true;
				}
			}
		}
		*dst=255;
		src++; dst++;
		#ifdef _DEBUG
		ASSERT(eols==src);
		ASSERT(eold==dst);
		#endif
	}
	ASSERT(dst<=uv);
	TRACE("+++ %d\n", uv-dst);
	return res;
}


#endif //HTRIGGERBIN_INC
