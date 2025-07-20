#ifndef DECODE128_INC
#define DECODE128_INC

#include <helpers\memory.h>

typedef struct tagCode
{
	char code[7];
	char code2[6];
	char cA;
	char cB;
	char cC[3];
}Code;

#define NUL '\x00'
#define SOH '\x01'
#define STX '\x02'
#define ETX '\x03'
#define EOT '\x04'
#define ENQ '\x05'
#define ACK '\x06'
#define BEL '\x07'
#define BS '\x08'
#define HT '\x09'
#define LF '\x0A'
#define VT '\x0B'
#define FF '\x0C'
#define CR '\x0D'
#define SO '\x0E'
#define SI '\x0F'
#define DLE '\x10'
#define DC1 '\x11'
#define DC2 '\x12'
#define DC3 '\x13'
#define DC4 '\x14'
#define NAK '\x15'
#define SYN '\x16'
#define ETB '\x17'
#define CAN '\x18'
#define EM '\x19'
#define SUB '\x1A'
#define ESC '\x1B'
#define FS '\x1C'
#define GS '\x1D'
#define RS '\x1E'
#define US '\x1F'
#define DEL '\x7F'

// nonstandard versions
#define FNC1 '\xF1'
#define FNC2 '\xF2'
#define FNC3 '\xF3'
#define FNC4 '\xF4'
#define ShiftA '\xF5'
#define ShiftB '\xF6'
#define CodeA '\xF7'
#define CodeB  '\xF8'
#define CodeC  '\xF9'
#define StartCodeA '\xFA'
#define StartCodeB '\xFB'
#define StartCodeC '\xFC'
#define EOD		   '\xFE'

Code Codes[]=
{
	{"212222","33446",' ',' ',"00"},	// 00
	{"222122","44336",'!','!',"01"},	// 01
	{"222221","44446",'"','"',"02"},	// 02
	{"121223","33344",'#','#',"03"},	// 03
	{"121322","33454",'$','$',"04"},	// 04
	{"131222","44344",'%','%',"05"},	// 05
	{"122213","34434",'&','&',"06"},	// 06
	{"122312","34544",'\'','\'',"07"},	// 07
	{"132212","45434",'(','(',"08"},	// 08
	{"221213","43334",')',')',"09"},	// 09
	{"221312","43444",'*','*',"10"},	// 10
	{"231212","54334",'+','+',"11"},	// 11
	{"112232","23456",',',',',"12"},	// 12
	{"122132","34346",'-','-',"13"},	// 13
	{"122231","34456",'.','.',"14"},	// 14
	{"113222","24546",'/','/',"15"},	// 15
	{"123122","35436",'0','0',"16"},	// 16
	{"123221","35546",'1','1',"17"},	// 17
	{"223211","45536",'2','2',"18"},	// 18
	{"221132","43246",'3','3',"19"},	// 19
	{"221231","43356",'4','4',"20"},	// 20
	{"213212","34536",'5','5',"21"},	// 21
	{"223112","45426",'6','6',"22"},	// 22
	{"312131","43348",'7','7',"23"},	// 23
	{"311222","42346",'8','8',"24"},	// 24
	{"321122","53236",'9','9',"25"},	// 25
	{"321221","53346",':',':',"26"},	// 26
	{"312212","43436",';',';',"27"},	// 27
	{"322112","54326",'<','<',"28"},	// 28
	{"322211","54436",'=','=',"29"},	// 29
	{"212123","33336",'>','>',"30"},	// 30
	{"212321","33556",'?','?',"31"},	// 31
	{"232121","55336",'@','@',"32"},	// 32
	{"111323","22454",'A','A',"33"},	// 33
	{"131123","44234",'B','B',"34"},	// 34
	{"131321","44454",'C','C',"35"},	// 35
	{"112313","23544",'D','D',"36"},	// 36
	{"132113","45324",'E','E',"37"},	// 37
	{"132311","45544",'F','F',"38"},	// 38
	{"211313","32444",'G','G',"39"},	// 39
	{"231113","54224",'H','H',"40"},	// 40
	{"231311","54444",'I','I',"41"},	// 41
	{"112133","23346",'J','J',"42"},	// 42
	{"112331","23566",'K','K',"43"},	// 43
	{"132131","45346",'L','L',"44"},	// 44
	{"113123","24436",'M','M',"45"},	// 45
	{"113321","24656",'N','N',"46"},	// 46
	{"133121","46436",'O','O',"47"},	// 47
	{"313121","44438",'P','P',"48"},	// 48
	{"211331","32466",'Q','Q',"49"},	// 49
	{"231131","54246",'R','R',"50"},	// 50
	{"213113","34426",'S','S',"51"},	// 51
	{"213311","34646",'T','T',"52"},	// 52
	{"213131","34448",'U','U',"53"},	// 53
	{"311123","42236",'V','V',"54"},	// 54
	{"311321","42456",'W','W',"55"},	// 55
	{"331121","64236",'X','X',"56"},	// 56
	{"312113","43326",'Y','Y',"57"},	// 57
	{"312311","43546",'Z','Z',"58"},	// 58
	{"332111","65326",'[','[',"59"},	// 59
	{"314111","45528",'\\','\\',"60"},	// 60
	{"221411","43554",']',']',"61"},	// 61
	{"431111","74226",'^','^',"62"},	// 62
	{"111224","22344",'_','_',"63"},	// 63
	{"111422","22564", NUL,'`',"64"},	// 64
	{"121124","33234", SOH,'a',"65"},	// 65
	{"121421","33564", STX,'b',"66"},	// 66
	{"141122","55234", ETX,'c',"67"},	// 67
	{"141221","55344", EOT,'d',"68"},	// 68
	{"112214","23434", ENQ,'e',"69"},	// 69
	{"112412","23654", ACK,'f',"70"},	// 70
	{"122114","34324", BEL,'g',"71"},	// 71
	{"122411","34654", BS, 'h',"72"},	// 72
	{"142112","56324", HT, 'i',"73"},	// 73
	{"142211","56434", LF, 'j',"74"},	// 74
	{"241211","65334", VT, 'k',"75"},	// 75
	{"221114","43224", FF, 'l',"76"},	// 76
	{"413111","54428", CR, 'm',"77"},	// 77
	{"241112","65224", SO, 'n',"78"},	// 78
	{"134111","47526", SI ,'o',"79"},	// 79
	{"111242","22366", DLE,'p',"80"},	// 80
	{"121142","33256", DC1,'q',"81"},	// 81
	{"121241","33366", DC2,'r',"82"},	// 82
	{"114212","25636", DC3,'s',"83"},	// 83
	{"124112","36526", DC4,'t',"84"},	// 84
	{"124211","36636", NAK,'u',"85"},	// 85
	{"411212","52336", SYN,'v',"86"},	// 86
	{"421112","63226", ETB,'w',"87"},	// 87
	{"421211","63336", CAN,'x',"88"},	// 88
	{"212141","33358", EM, 'y',"89"},	// 89
	{"214121","35538", SUB,'z',"90"},	// 90
	{"412121","53338", ESC,'{',"91"},	// 91
	{"111143","22256", FS, '|',"92"},	// 92
	{"111341","22476", GS, '}',"93"},	// 93
	{"131141","44256", RS, '~',"94"},	// 94
	{"114113","25526", US, DEL,"95"},	// 95
	{"114311","25746", FNC3, FNC3,"96"},	// 96
	{"411113","52226", FNC2, FNC2,"97"},	// 97
	{"411311","52446", ShiftB, ShiftA,"98"},	// 98
	{"113141","24458", CodeC,	CodeC,"99"},	// 99
	{"114131","25548", CodeB,	FNC4,	CodeB},	// 100
	{"311141","42258", FNC4,	CodeA,	CodeA},	// 101
	{"411131","52248", FNC1,	FNC1,	FNC1},	// 102
	{"211412","32554", StartCodeA,0,""},	// 103
	{"211214","32334", StartCodeB,0,""},	// 104
	{"211232","32356", StartCodeC,0,""},	// 105
	{"233111","56426", EOD,0,""}	// 106
};

//Stop (7 bars/spaces)
//#define StopCode "2331112"

__forceinline int seeksymbol(char res[8])
{
	for (int i=0; i<sizeof(Codes)/sizeof(Code); i++)
	{
		if (strcmp(res,Codes[i].code)==0)
			return i;
	}
	return -1;
}

__forceinline int seeksymbol2(char res[8])
{
	for (int i=0; i<sizeof(Codes)/sizeof(Code); i++)
	{
		//if (strcmp(res,Codes[i].code2)==0)
		if (memcmp(res,Codes[i].code2,4)==0)
			return i;
	}
	return -1;
}

__forceinline bool valid_e(int e)
{
	return ((e>1) && (e<8));
}

__forceinline bool getsymbol2(int* data, char res[8])
{
	memset(res,0,8);
	double len=data[0]+data[1]+data[2]+data[3]+data[4]+data[5];
	len/=11;
	{
		int e1,e2,e3,e4;
		int b1,b2,b3;
		b1=(int)(data[0]/len+0.5);
		b2=(int)(data[2]/len+0.5);
		b3=(int)(data[4]/len+0.5);
		int p0=data[0];
		int p1=p0+data[1];
		int p2=p1+data[2];
		int p3=p2+data[3];
		int p4=p3+data[4];
		int p5=p4+data[5];
		e1=(int)(p1/len+0.5);		if (!valid_e(e1)) return false;
		e2=(int)((p2-p0)/len+0.5);  if (!valid_e(e2)) return false;
		e3=(int)((p3-p1)/len+0.5);  if (!valid_e(e3)) return false;
		e4=(int)((p4-p2)/len+0.5);	if (!valid_e(e4)) return false;
		sprintf(res,"%1d%1d%1d%1d%1d",e1,e2,e3,e4,b1+b2+b3);
		//TRACE("%1d%1d%1d%1d%1d\n",e1,e2,e3,e4,b1+b2+b3);
	}
	return true;
}

__forceinline bool getsymbol(int* data, char res[8])
{
	double len=data[0]+data[1]+data[2]+data[3]+data[4]+data[5];
	len/=11;
	int i;
	memset(res,0,8);
	for (i=0; i<6; i++)
	{
		res[i]=(int)(data[i]/len+0.5);
		if ((res[i]==0) || (res[i]>4)) return false;
	}
	int parity=res[0]+res[2]+res[4];
	if ((parity%2)==0)
	{
		for (i=0; i<6; i++)
			res[i]+='0';
		return true;
	}
	return false;
}

__forceinline bool _decode(CDWordArray& ca, FXString& res)
{
	int mode=0;
	int shift=0;
	res.Empty();
	for (int i=0; i<ca.GetSize()-2; i++)
	{
		int nmb=ca[i];
		TRACE(" - 0x%x(%c) 0x%x(%c) %s\n",Codes[nmb].cA,Codes[nmb].cA, Codes[nmb].cB, Codes[nmb].cB,Codes[nmb].cC);
		if (Codes[nmb].cA==StartCodeA)
		{
			mode=CodeA;
			TRACE("Start CodeA\n");
		}
		else if (Codes[nmb].cA==StartCodeB)
		{
			mode=CodeB;
			TRACE("Start CodeB\n");
		}
		else  if (Codes[nmb].cA==StartCodeC)
		{
			mode=CodeC;
			TRACE("Start CodeC\n");
		}
		else  if ((mode==CodeA) && (Codes[nmb].cA==CodeC))
		{
			mode=CodeC;
			TRACE("Change to CodeC\n");
		}
		else  if ((mode==CodeB) && (Codes[nmb].cB==CodeC))
		{
			mode=CodeC;
			TRACE("Change to CodeC\n");
		}
		else  if ((mode==CodeA) && (Codes[nmb].cA==CodeB))
		{
			mode=CodeB;
			TRACE("Change to CodeB\n");
		}
		else  if ((mode==CodeC) && (Codes[nmb].cC[0]==CodeB))
		{
			mode=CodeB;
			TRACE("Change to CodeB\n");
		}
		else  if ((mode==CodeB) && (Codes[nmb].cA==CodeA))
		{
			mode=CodeA;
			TRACE("Change to CodeB\n");
		}
		else  if ((mode==CodeC) && (Codes[nmb].cC[0]==CodeA))
		{
			mode=CodeA;
			TRACE("Change to CodeB\n");
		}
		else  if ((mode==CodeA) && (Codes[nmb].cA==ShiftB))
		{
			shift=CodeB;
			TRACE("Change to CodeB\n");
		}
		else  if ((mode==CodeB) && (Codes[nmb].cC[0]==ShiftA))
		{
			shift=CodeB;
			TRACE("Change to CodeB\n");
		}
		else
		{
			switch (mode)
			{
			case CodeA:
				if (shift)
				{
					res+=Codes[nmb].cB;
					shift=0;
				}
				else
					res+=Codes[nmb].cA; 
				break;
			case CodeB:
				if (shift)
				{
					res+=Codes[nmb].cA;
					shift=0;
				}
				else
					res+=Codes[nmb].cB;
				break;
			case CodeC:
				res+=Codes[nmb].cC;
				break;
			}
		}
	}
	return true;
}

__forceinline bool parse128(LPBYTE d, int size, FXString& res)
{
	//wait black
	bool result=false;
	LPBYTE s=d;
	LPBYTE eod=d+size;
	int* data=(int*)malloc(size*sizeof(int));
	memset(data,0,size*sizeof(int));
	while ((*s>128) && (s<eod))
		s++;
	int p=0;
	while (s<eod)
	{
		int cnt=0;
		while((*s<=128) && (s<eod))
		{
			cnt++;
			s++;
		}
		ASSERT(p<size);
		data[p]=cnt; p++;
		cnt=0;
		while((*s>128) && (s<eod))
		{
			cnt++;
			s++;
		}
		ASSERT(p<size);
		data[p]=cnt; p++;
	}
	int i=0;
	int mode=0;
	res.Empty();
	CDWordArray carray;
	while (i<p-6)
	{
		char code[8];
		//if (getsymbol(data+i,code))
		if 	(getsymbol2(data+i,code))
		{
			//TRACE("%s",code);
			//int nmb=seeksymbol(code);
			int nmb=seeksymbol2(code);
			carray.Add(nmb);
			if (nmb==-1)
			{
				//TRACE(" - fail find symbol\n");
				res.Empty();
				carray.RemoveAll();
				mode=0;
				i+=2;
				continue;
			}
			else
			{
				//TRACE(" - 0x%x(%c) 0x%x(%c) %s\n",Codes[nmb].cA,Codes[nmb].cA, Codes[nmb].cB, Codes[nmb].cB,Codes[nmb].cC);
				if (Codes[nmb].cA==StartCodeA)
				{
					mode=CodeA;
					TRACE("Start CodeA\n");
				}
				else if (Codes[nmb].cA==StartCodeB)
				{
					mode=CodeB;
					TRACE("Start CodeB\n");
				}
				else  if (Codes[nmb].cA==StartCodeC)
				{
					mode=CodeC;
					TRACE("Start CodeC\n");
				}
				else  if ((mode!=0) && (Codes[nmb].cA==EOD))
				{
					// calc crc
					if (carray.GetSize()<3) return false;
					DWORD crc=0;
					for (int j=0; j<carray.GetSize()-2; j++)
					{
						if (j==0)
							crc+=carray[j];
						else
							crc+=j*carray[j];
					}
					crc=crc%103;
					TRACE("End data reached, crc=%d, last code=%d\n", crc,carray[carray.GetSize()-2]);
					result=(crc==carray[carray.GetSize()-2]);
					if (result)
					{
						_decode(carray, res);
					}
					else
						res="";
					break;
				}
				i+=6;
			}
		}
		else
		{
			mode=0;
			res.Empty();
			carray.RemoveAll();
			i+=2;
		}
	}
	free(data);
	return result;
}

__forceinline bool parse128(const pTVFrame tvf, FXString& res)
{
    bool retV=false;
    LPBYTE data=GetData(tvf);
    int w=tvf->lpBMIH->biWidth;
    int h=tvf->lpBMIH->biHeight;
	LPBYTE invdata=(LPBYTE)malloc(tvf->lpBMIH->biWidth);
    for (int i=0; i<h; i++)
    {
		//TRACE("New line %d\n", i);
		retV=parse128(data+i*w,w,res);
		if (!retV) // try invers order
		{
			memcpy_i(invdata,data+i*w,w);
			retV=parse128(invdata,w,res);
		}
        if (retV)  break;
    }
	free(invdata);
    return retV;
}


#endif //DECODE128_INC