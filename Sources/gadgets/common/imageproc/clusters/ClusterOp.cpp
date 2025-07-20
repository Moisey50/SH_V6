#include "stdafx.h"
#include <imageproc\clusters\ClusterOp.h>

// Pens
static CPen GreenPen(PS_SOLID, 1, RGB(0,255,128));
static CPen GreenFatPen(PS_SOLID, 3, RGB(0,255,128));
static CPen RedBoldPen  (PS_SOLID, 2, RGB(255,0,0));
static CPen OrangePen  (PS_SOLID, 1, RGB(255,128,0));
static CPen BluePen (PS_SOLID, 1, RGB(0,0,255));
static CPen BlueFatPen (PS_SOLID, 3, RGB(0,0,255));
static CPen YellowPen (PS_SOLID, 1, RGB(128,128,0));
static CPen RedPen  (PS_SOLID, 1, RGB(255,0,0));


bool Cluster_DrawCurveParam(HDC hdc,RECT& rc, CDIBViewBase* view, LPVOID lParam)
{
    LPCURVEPARAM cp=(LPCURVEPARAM)lParam;

    // draw red cross at mass center

    CPoint cntr=cp->cm;
    view->Pic2Scr(cntr);
    HGDIOBJ old = SelectObject(hdc, RedPen);
    MoveToEx(hdc,cntr.x-5,cntr.y,NULL);
    LineTo(hdc,cntr.x+5,cntr.y);
    MoveToEx(hdc,cntr.x,cntr.y-5,NULL);
    LineTo(hdc,cntr.x,cntr.y+5);

    // draw GreenPoints crosses at ends of the cluster
    SelectObject(hdc, GreenFatPen);
    if (cp->ce1>=0)
    {
        cntr=cp->pnts[cp->ce1];
        view->Pic2Scr(cntr);
        MoveToEx(hdc,cntr.x,cntr.y,NULL);
        LineTo(hdc,cntr.x+5,cntr.y);
        LineTo(hdc,cntr.x+5,cntr.y+5);
        LineTo(hdc,cntr.x,cntr.y+5);
        LineTo(hdc,cntr.x,cntr.y);
    }
    SelectObject(hdc, BlueFatPen);
    if (cp->ce3>=0)
    {
        cntr=cp->pnts[cp->ce3];
        view->Pic2Scr(cntr);
        MoveToEx(hdc,cntr.x,cntr.y,NULL);
        LineTo(hdc,cntr.x+5,cntr.y);
        LineTo(hdc,cntr.x+5,cntr.y+5);
        LineTo(hdc,cntr.x,cntr.y+5);
        LineTo(hdc,cntr.x,cntr.y);
    }

    SelectObject(hdc, RedBoldPen);
    if (cp->ce2>=0)
    {
        cntr=cp->pnts[cp->ce2];
        view->Pic2Scr(cntr);
        MoveToEx(hdc,cntr.x,cntr.y,NULL);
        LineTo(hdc,cntr.x+5,cntr.y);
        LineTo(hdc,cntr.x+5,cntr.y+5);
        LineTo(hdc,cntr.x,cntr.y+5);
        LineTo(hdc,cntr.x,cntr.y);
    }
    
    SelectObject(hdc, YellowPen);
    if (cp->ce2>=0)
    {
        cntr=cp->cc;
        view->Pic2Scr(cntr);
        MoveToEx(hdc,cntr.x-5,cntr.y,NULL);
        LineTo(hdc,cntr.x+5,cntr.y);
        MoveToEx(hdc,cntr.x,cntr.y-5,NULL);
        LineTo(hdc,cntr.x,cntr.y+5);
    }
    
    SelectObject(hdc, OrangePen);

    cntr=cp->cc;
    view->Pic2Scr(cntr);
    int rad=view->Pic2Scr(cp->radius);
    AngleArc(hdc,cntr.x,cntr.y,  rad, 0, 360);

    SelectObject(hdc, old);

    return true;
}
