#ifndef HTRIGGERBIN_INC
#define HTRIGGERBIN_INC

#define TRIGGER_THRESHOLD8 20

__forceinline unsigned char triggerval8(unsigned a)
{
	a+=TRIGGER_THRESHOLD8;
	return (a>255)?255:a;
}

__forceinline pTVFrame htriggerbin8(const pTVFrame  org)
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
  else 	if (res->lpBMIH->biCompression==BI_YUV12)
		memset(uv,128,h*w/2);

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
				if (*e<=triggerval8(*(e+1)))
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
				if (triggerval8(*e)>=*(e+1))
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

#define TRIGGER_THRESHOLD16 5140

__forceinline unsigned triggerval16(unsigned a)
{
	a+=TRIGGER_THRESHOLD16;
	return (a>65535)?65535:a;
}

__forceinline pTVFrame htriggerbin16(const pTVFrame  org)
{
	pTVFrame res= new TVFrame;
	res->lpBMIH=(LPBITMAPINFOHEADER)malloc(getsize4BMIH(org));
	res->lpData=NULL;
	memcpy(res->lpBMIH,org->lpBMIH,org->lpBMIH->biSize);
	LPWORD dst=(LPWORD)GetData(res);
	LPWORD src=(LPWORD)GetData(org);
	int h=res->lpBMIH->biHeight;
	int w=res->lpBMIH->biWidth;
	for (int y=0; y<h; y++)
	{
		LPWORD s=src,e=src;
		bool increase=(*e<=*(e+1));
		int x;
		#ifdef _DEBUG
		LPWORD eols=s+w;
		LPWORD eold=dst+w;
		#endif
		for (x=0; x<w-1; x++)
		{
			if (increase)
			{
				if (*e<=triggerval16(*(e+1)))
				{
					e++;
					if (x==w-2)
					{
						while (s<e)
						{
							*dst=65535;
							s++; dst++;
						}
						src=e;
					}
				}
				else
				{
					e++;
					unsigned mid=(*s+*e)/2;
					while (s<e)
					{
						*dst=(*s<mid)?0:65535;
						s++; dst++;
					}
					src=e;
					increase=false;
				}
			}
			else
			{
				if (triggerval16(*e)>=*(e+1))
				{
					e++;
					if (x==w-2)
					{
						while (s<e)
						{
							*dst=65535;
							s++; dst++;
						}
						src=e;
					}
				}
				else
				{
					e++;
					unsigned mid=(*s+*e)/2;
					while (s<e)
					{
						*dst=(*s<mid)?0:65535;
						s++; dst++;
					}
					src=e;
					increase=true;
				}
			}
		}
		*dst=65535;
		src++; dst++;
		#ifdef _DEBUG
		ASSERT(eols==src);
		ASSERT(eold==dst);
		#endif
	}
	return res;
}


__forceinline pTVFrame htriggerbin(const pTVFrame  org)
{
    switch (org->lpBMIH->biCompression)
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
        return htriggerbin8(org);
    case BI_Y16:
        return htriggerbin16(org);
    }
    return NULL;
}

#endif //HTRIGGERBIN_INC
