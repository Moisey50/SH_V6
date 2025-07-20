#ifndef _DECODE30_INC
#define _DECODE30_INC

#include <helpers\memory.h>

typedef struct tag13Codes
{
	char a;
	char code[10];
}s13codes;

s13codes codes[]=
{
	{'0',"bwbWBwBwb"},
	{'1',"BwbWbwbwB"},
	{'2',"bwBWbwbwB"},
	{'3',"BwBWbwbwb"},
	{'4',"bwbWBwbwB"},
	{'5',"BwbWBwbwb"},
	{'6',"bwBWBwbwb"},
	{'7',"bwbWbwBwB"},
	{'8',"BwbWbwBwb"},
	{'9',"bwBWbwBwb"},
	{'A',"BwbwbWbwB"},
	{'B',"bwBwbWbwB"},
	{'C',"BwBwbWbwb"},
	{'D',"bwbwBWbwB"},
	{'E',"BwbwBWbwb"},
	{'F',"bwBwBWbwb"},
	{'G',"bwbwbWBwB"},
	{'H',"BwbwbWBwb"},
	{'I',"bwBwbWBwb"},
	{'J',"bwbwBWBwb"},
	{'K',"BwbwbwbWB"},
	{'L',"bwBwbwbWB"},
	{'M',"BwBwbwbWb"},
	{'N',"bwbwBwbWB"},
	{'O',"BwbwBwbWb"},
	{'P',"bwBwBwbWb"},
	{'Q',"bwbwbwBWB"},
	{'R',"BwbwbwBWb"},
	{'S',"bwBwbwBWb"},
	{'T',"bwbwBwBWb"},
	{'U',"BWbwbwbwB"},
	{'V',"bWBwbwbwB"},
	{'W',"BWBwbwbwb"},
	{'X',"bWbwBwbwB"},
	{'Y',"BWbwBwbwb"},
	{'Z',"bWBwBwbwb"},
	{'-',"bWbwbwBwB"},
	{'.',"BWbwbwBwb"},
	{' ',"bWBwbwBwb"},
	{'$',"bWbWbWbwb"},
	{'/',"bWbWbwbWb"},
	{'+',"bWbwbWbWb"},
	{'%',"bwbWbWbWb"},
	{'*',"bWbwBwBwb"}
};

__forceinline int codessize()
{
	return (sizeof(codes)/sizeof(s13codes));
}

__forceinline char codetosymbol(FXString& code)
{
	for (int i=0; i<codessize(); i++)
	{
		if (code==codes[i].code)
			return codes[i].a;
	}
	return 0;
}

#define START_SYMBOL "bWbwBwBwb"

typedef struct tagLine
{
	char color;
	int size;
}sLine;

__forceinline double rnd(double d)
{
	return (int(d+0.5));
}

__forceinline bool iswide(int size, double narrow)
{
	return ((size>=rnd(1.8*narrow)) && (size<=rnd(3.4*narrow)));
}

__forceinline bool isnarrow(int size, double narrow)
{
	return ((size>=rnd(0.4*narrow)) && (size<=rnd(1.5*narrow)));
}

__forceinline bool parse39(LPBYTE d, int size, FXString& res)
{
	LPBYTE eod=d+size;
	LPBYTE src=d;
	int linelen=0;
	sLine* line = (sLine*)malloc(sizeof(sLine)*size);
	int cnt=0;
	while (src<eod)
	{
		while ((src<eod) && (*src!=0)) // wait for black
		{
			src++;
		}
		while ((src<eod) && (*src==0)) // process black
		{
			src++;
			cnt++;
		}
		line[linelen].color='B';
		line[linelen].size=cnt;
		cnt=0;
		linelen++;
		while ((src<eod) && (*src!=0)) // process white
		{
			src++;
			cnt++;
		}
		line[linelen].color='W';
		line[linelen].size=cnt;
		cnt=0;
		linelen++;
	}
#ifdef _DEBUG
	for (int i=0; i<linelen; i++)
	{
		//TRACE("%c(%d)",line[i].color,line[i].size);
	}
	//TRACE("\n");
#endif
	cnt=0;
	double narrow=0;
	int    narrowcnt=0;
	FXString code;
	FXString resstring;
	bool startsybolfound=false;
	int bcnt=0;
	while (cnt<linelen-9)
	{
		// seek '*'
		while (cnt<linelen-9)
		{
			// let's first is narrow
			if (narrow==0)
			{
				narrow=line[cnt].size;
				narrowcnt=1;
				code+='b';
			}
			else
			{
				if (iswide(line[cnt].size,narrow/narrowcnt))
				{
					code+='B';
				}
				else if (isnarrow(line[cnt].size,narrow/narrowcnt))
				{
					code+='b';
					narrow+=line[cnt].size;
					narrowcnt++;
				}
				else
				{
					code.Empty();
					cnt+=2; // move to next black
					continue;
				}
			}
			if (iswide(line[cnt+1].size,narrow/narrowcnt))
			{
				code+='W';
			}
			else if (isnarrow(line[cnt+1].size,narrow/narrowcnt))
			{
				code+='w';
				narrow+=line[cnt+1].size;
				narrowcnt++;
			}
			else
			{
				code.Empty();
				cnt+=2; // move to next black
				continue;
			}
			if (iswide(line[cnt+2].size,narrow/narrowcnt))
			{
				code+='B';
			}
			else if (isnarrow(line[cnt+2].size,narrow/narrowcnt))
			{
				code+='b';
				narrow+=line[cnt+2].size;
				narrowcnt++;
			}
			else
			{
				code.Empty();
				cnt+=2; // move to next black
				continue;
			}
			if (iswide(line[cnt+3].size,narrow/narrowcnt))
			{
				code+='W';
			}
			else if (isnarrow(line[cnt+3].size,narrow/narrowcnt))
			{
				code+='w';
				narrow+=line[cnt+3].size;
				narrowcnt++;

			}
			else
			{
				code.Empty();
				cnt+=2; // move to next black
				continue;
			}
			if (iswide(line[cnt+4].size,narrow/narrowcnt))
			{
				code+='B';
			}
			else if (isnarrow(line[cnt+4].size,narrow/narrowcnt))
			{
				code+='b';
				narrow+=line[cnt+4].size;
				narrowcnt++;

			}
			else
			{
				code.Empty();
				cnt+=2; // move to next black
				continue;
			}
			if (iswide(line[cnt+5].size,narrow/narrowcnt))
			{
				code+='W';
			}
			else if (isnarrow(line[cnt+5].size,narrow/narrowcnt))
			{
				code+='w';
				narrow+=line[cnt+5].size;
				narrowcnt++;

			}
			else
			{
				code.Empty();
				cnt+=2; // move to next black
				continue;
			}
			if (iswide(line[cnt+6].size,narrow/narrowcnt))
			{
				code+='B';
			}
			else if (isnarrow(line[cnt+6].size,narrow/narrowcnt))
			{
				code+='b';
				narrow+=line[cnt+6].size;
				narrowcnt++;
			}
			else
			{
				code.Empty();
				cnt+=2; // move to next black
				continue;
			}
			if (iswide(line[cnt+7].size,narrow/narrowcnt))
			{
				code+='W';
			}
			else if (isnarrow(line[cnt+7].size,narrow/narrowcnt))
			{
				code+='w';
				narrow+=line[cnt+7].size;
				narrowcnt++;
			}
			else
			{
				code.Empty();
				cnt+=2; // move to next black
				continue;
			}
			if (iswide(line[cnt+8].size,narrow/narrowcnt))
			{
				code+='B';
			}
			else if (isnarrow(line[cnt+8].size,narrow/narrowcnt))
			{
				code+='b';
				narrow+=line[cnt+8].size;
				narrowcnt++;
			}
			else
			{
				code.Empty();
				cnt+=2; // move to next black
				continue;
			}
			if ((code==START_SYMBOL) && (startsybolfound))
			{
				resstring+='*';
				break;
			}
			if ((code==START_SYMBOL) || (startsybolfound))
			{
				cnt+=9;
				if (isnarrow(line[cnt].size,narrow/narrowcnt))
				{
					narrow+=line[cnt].size;
					narrowcnt++;
					startsybolfound=true;
					TRACE("%s\t",code);
					char newchar=codetosymbol(code);
					if (newchar!=0)
					{
						resstring+=newchar;
						TRACE("%c\n",newchar);
						code.Empty();
						cnt++;
					}
					else
					{
						TRACE("error\n");
						code.Empty();
						bcnt+=2; //reread 1-st not checked
						resstring.Empty();
						startsybolfound=false;
						cnt=bcnt;
						continue;
					}
				}
				else
				{
					code.Empty();
					narrow=0;
					narrowcnt=0;
					cnt+=2; // move to next black
					continue;
				}
			}
			else
			{
				narrow=0;
				narrowcnt=0;
				startsybolfound=false;
				code.Empty();
				cnt+=2;
			}
		}
	}
	free(line);
	res=resstring;
	return ((resstring.GetLength()>2) && (resstring[0]=='*') && (resstring[resstring.GetLength()-1]=='*')) ;
}

__forceinline bool parse39(const pTVFrame tvf, FXString& res)
{
    bool retV=false;
    LPBYTE data=GetData(tvf);
    int w=tvf->lpBMIH->biWidth;
    int h=tvf->lpBMIH->biHeight;
	LPBYTE invdata=(LPBYTE)malloc(tvf->lpBMIH->biWidth);
    for (int i=0; i<h; i++)
    {
		TRACE("New line %d\n", i);
		retV=parse39(data+i*w,w,res);
		if (!retV) // try invers order
		{
			memcpy_i(invdata,data+i*w,w);
			retV=parse39(invdata,w,res);
		}
        if (retV)  break;
    }
	free(invdata);
    return retV;
}


#endif