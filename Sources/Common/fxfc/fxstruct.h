#if !defined(FXSTRUCT_INCLUDED_)
#define FXSTRUCT_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct tagDPOINT
{
	double x;
	double y;
}DPOINT;

typedef struct tagDSIZE
{
	double cx;
	double cy;
}DSIZE;

typedef struct tagDRECT // it use origin lower-left corner as opposed 
                        // to screen coordinates, so top>bottom
{
  double left;
  double top;
  double right;
  double bottom;
}DRECT;

#endif //#define FXSTRUCT_INCLUDED_