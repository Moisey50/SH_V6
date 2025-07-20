#ifndef DECODE_UPC_INC
#define DECODE_UPC_INC

#include <FXFC\fxfc.h>
#include <imageproc\statistics.h>

#if (defined(_DEBUG) && defined(PARSE_INDEBUG))
    #pragma message("!!! Detail tracing build! No automatical recogintion will be performed!")
    #define TRACEUPCDECODE TRACE
#else 
    #pragma message("+++ no tracing for parsing defined")
    #define TRACEUPCDECODE
#endif

class Element
{
public:
    bool val;
    int width;
    Element(){};
    Element(bool v, int w)
    {
        val=v; width=w;
    }
};

typedef enum 
{
    L=0,G=1,R=2
} digittype;

__forceinline FXString Get(FXString& s, int cnt)
{
    FXString retV;
    if (cnt>s.GetLength()) return retV;
    retV=s.Left(cnt);
    s=s.Mid(cnt);
    return retV;
}

__forceinline void    Put(FXString&s, FXString& retV)
{
    s=retV+s;
}

__forceinline char Decode(FXString tmpV, digittype& dt)
{
    char retV='?';

    if (tmpV=="0001101")
    {
            dt=L;  retV='0';
    }
    else if (tmpV=="0011001")
    {
            dt=L;  retV='1';
    }
    else if (tmpV=="0010011")
    {
            dt=L;  retV='2';
    }
    else if (tmpV=="0111101")
    {
            dt=L;  retV='3';
    }
    else if (tmpV=="0100011")
    {
            dt=L;  retV='4';
    }
    else if (tmpV=="0110001")
    {
            dt=L;  retV='5';
    }
    else if (tmpV=="0101111")
    {
            dt=L;  retV='6';
    }
    else if (tmpV=="0111011")
    {
            dt=L;  retV='7';
    }
    else if (tmpV=="0110111")
    {
            dt=L;  retV='8';
    }
    else if (tmpV=="0001011")
    {
            dt=L;  retV='9';
    }
    else if (tmpV=="1110010")
    {
            dt=R;  retV='0';
    }
    else if (tmpV=="1100110")
    {
            dt=R;  retV='1';
    }
    else if (tmpV=="1101100")
    {
            dt=R;  retV='2';
    }
    else if (tmpV=="1000010")
    {
            dt=R;  retV='3';
    }
    else if (tmpV=="1011100")
    {
            dt=R;  retV='4';
    }
    else if (tmpV=="1001110")
    {
            dt=R;  retV='5';
    }
    else if (tmpV=="1010000")
    {
            dt=R;  retV='6';
    }
    else if (tmpV=="1000100")
    {
            dt=R;  retV='7';
    }
    else if (tmpV=="1001000")
    {
            dt=R;  retV='8';
    }
    else if (tmpV=="1110100")
    {
            dt=R;  retV='9';
    }
    else if (tmpV=="0100111")
    {
            dt=G;  retV='0';
    }
    else if (tmpV=="0110011")
    {
            dt=G;  retV='1';
    }
    else if (tmpV=="0011011")
    {
            dt=G;  retV='2';
    }
    else if (tmpV=="0100001")
    {
            dt=G;  retV='3';
    }
    else if (tmpV=="0011101")
    {
            dt=G;  retV='4';
    }
    else if (tmpV=="0111001")
    {
            dt=G;  retV='5';
    }
    else if (tmpV=="0000101")
    {
            dt=G;  retV='6';
    }
    else if (tmpV=="0010001")
    {
            dt=G;  retV='7';
    }
    else if (tmpV=="0001001")
    {
            dt=G;  retV='8';
    }
    else if (tmpV=="0010111")
    {
            dt=G;  retV='9';
    }
    TRACEUPCDECODE("%s (%c)\n",tmpV,retV);
    return retV;
}


__forceinline bool parse(LPBYTE d, int size, FXString& res)
{
    CArray<Element,Element> m_Code;
    int i,cnt=0;
    bool lastL;
    res.Empty();

    #define _LEVELC 128
    i=0;
    while ((i<size) && (d[i]>_LEVELC))
    {
        lastL=(d[i]<_LEVELC)?true:false;
        i++;
    }
    if (i==0) lastL=(d[i]<_LEVELC)?true:false; i++;
    for(; i<size; i++)
    {
        if (lastL==((d[i]<_LEVELC)?true:false))
        {
            cnt++;
        }
        else
        {
            if (cnt) m_Code.Add(Element(lastL,cnt));
            cnt=1;
            lastL=(d[i]<_LEVELC)?true:false;
        }
    }
    m_Code.Add(Element(lastL,cnt));
    if (m_Code.GetSize()<8) return false;
    #ifdef _DEBUG
    for (i=0; i<m_Code.GetSize(); i++)
    {
        TRACEUPCDECODE("%s(%d)",(m_Code[i].val)?"1":"0",m_Code[i].width);
    }
    TRACEUPCDECODE("\n");
    #endif
    i=0;
    while (i<m_Code.GetSize())  // skip trailing ziros
    {  
        if (m_Code[i].val) break; 
        i++; 
    }
    if (i==m_Code.GetSize()) return false;
    double sw;
    FXString v;
    int strtcnt=i;
next_attempt:
    i=strtcnt;
    while ((i<m_Code.GetSize()) && (m_Code[i].width<2)) i++;
    if (i==m_Code.GetSize()) return false;
    strtcnt=i;
    sw=m_Code[i].width;
    double crntsum=0;
    double crntlen=0;
    TRACEUPCDECODE("+++ sw= %f\n",sw);
    for (; i<m_Code.GetSize(); i++)
    {
        double cnt=(m_Code[i].width/sw);
        if ((cnt<1.7) && (i<strtcnt+4))
        {
            sw+=m_Code[i].width;
            sw/=2;
            cnt=(m_Code[i].width/sw);
            TRACEUPCDECODE("+++ sw= %f\n",sw);
        } 
        crntlen+=m_Code[i].width;
        cnt=((int)cnt)+((fmod(cnt,1.0)>0.51)?1:0);
        crntsum+=cnt;
        TRACEUPCDECODE("+++ sw= %f - %f\n",sw,crntlen/crntsum);
        sw=crntlen/crntsum;
        if ((cnt==0) && (v.GetLength()<10))
        {
            strtcnt=i;
            v.Empty();        
            goto next_attempt;
        }
        //TRACEUPCDECODE("+++ %f (%s) sw=%f\n",cnt,(m_Code[i].val)?"1":"0",sw);
        for (int j=0; j<cnt; j++)
        {
            v+=(m_Code[i].val)?"1":"0";
            if (v.GetLength()==4)
            {
                if (v!="1010")
                {
                    // No first sync mark
                    v.Empty();        
                    strtcnt+=2;
                    goto next_attempt;
                }
            }
        }
    }
    
    TRACEUPCDECODE("Input string: '%s'\n",v);
    res.Empty();
    FXString tmpV;
    tmpV=Get(v,3);
    if (tmpV!="101") 
    { 
        TRACEUPCDECODE("!!! Can't find first sync marker\n");
        Put(v,tmpV); 
        TRACEUPCDECODE("!!!! remaing %s\n",v);
        return false; 
    }
    int cdLen=-1;
    FXString cp;
    while (v.GetLength())
    {
        
        digittype part;
        if (res.GetLength()==cdLen)
        {
            // Verify end mark 
            tmpV=Get(v,3);
            if (tmpV=="101") 
            {
                TRACEUPCDECODE("+++ Found eod mark, res='%s'\n",res);
                break;
            }
            else 
            {
                TRACEUPCDECODE("!!!! Can't find EOD mark, got %s instead remaing %s\n", tmpV, v);
                return false;
            }
        }
        tmpV=Get(v,7);
        if (tmpV.GetLength()==0) 
        {
            TRACEUPCDECODE("!!!! remaing %s\n",v);
            return false;
        }
        char r=Decode(tmpV,part);
        if (r!='?')
        {
            res+=r;
            cp+=(part==L)?"L":(part==G)?"G":"R";
        }
        else
        {
            Put(v,tmpV);
            if (Get(v,5)=="01010")
            {
                cdLen = (int)res.GetLength()*2;
                TRACEUPCDECODE("+++ Found second sinc marker at %d\n", res.GetLength());
                continue;
            }
            else
            {
                TRACEUPCDECODE("!!! remaing %s\n",v);
                return false;
            }
        }
    }
    FXString Rp,Lp;
    if (cdLen==12)
    {
        Rp=cp.Mid(6);
        Lp=cp.Left(6);
        if (Lp=="LLLLLL")
            res="0"+res;
        else if (Lp=="LLGLGG")
            res="1"+res;
        else if (Lp=="LLGGLG")
            res="2"+res;
        else if (Lp=="LLGGGL")
            res="3"+res;
        else if (Lp=="LGLLGG")
            res="4"+res;
        else if (Lp=="LGGLLG")
            res="5"+res;
        else if (Lp=="LGGGLL")
            res="6"+res;
        else if (Lp=="LGLGLG")
            res="7"+res;
        else if (Lp=="LGLGGL")
            res="8"+res;
        else if (Lp=="LGGLGL")
            res="9"+res;
    }
    TRACEUPCDECODE("+++ %s\n",res);
    TRACEUPCDECODE("+++ remaing %s\n",v);
	return ((res.GetLength()==13) || (res.GetLength()==8));
}

__forceinline bool verify(FXString& cd)
{
	char* pntr;
	int len;
	int sume=0, sumo=0;
	bool ean13;
    if (cd.GetLength()==13)
    {
        pntr=(char*)&(((LPCTSTR)cd)[11]);
		len=12;
		ean13=true;
    }
    else
    {
        pntr=(char*)&(((LPCTSTR)cd)[6]);
		len=7;
		ean13=false;
    }
	while (len)
	{
		if (len%2)
		{
			sumo+=*pntr-'0';
		}
		else
		{
			sume+=*pntr-'0';
		}
		pntr--;
		len--;
	}
	int sum;
	if (ean13)
		sum=3*sume+sumo;
	else
		sum=3*sumo+sume;
	int csum=((sum/10)+((sum%10)?1:0))*10-sum;
	bool res=(csum==(cd[cd.GetLength()-1]-'0'));
	if (!res)
	{
		TRACE("CRC error CRC=%d, CRC digit=%c\n",csum,cd[cd.GetLength()-1]);
	}
	return res;
}


__forceinline bool parse(CSData& d, FXString& res)
{
    int size = (int)d.GetSize();
    bool retV=false;
    LPBYTE data=(LPBYTE)malloc(size);
    for (int i=0; i<size; i++)
    {
        data[i]=(unsigned char)d[i];
    }
    retV=parse(data, size, res);
    free(data);
    return retV;
}

__forceinline void  memcpy_i( LPBYTE dest, LPBYTE src, size_t count)
{
	void* eod=dest+count;
	src=src+count-1;
	while(dest<eod)
	{
		*dest=*src;
		dest++;
		src--;
	}
}


__forceinline bool parse(const pTVFrame tvf, FXString& res)
{
    bool retV=false;
    LPBYTE data=GetData(tvf);
    int w=tvf->lpBMIH->biWidth;
    int h=tvf->lpBMIH->biHeight;
	LPBYTE invdata=(LPBYTE)malloc(tvf->lpBMIH->biWidth);
    for (int i=0; i<h; i++)
    {
		retV=parse(data+i*w,w,res);
		if (!retV) // try invers order
		{
			memcpy_i(invdata,data+i*w,w);
			retV=parse(invdata,w,res);
		}
        if ( (retV) && (verify(res)) ) break;
    }
	free(invdata);
    return retV;
}

#endif