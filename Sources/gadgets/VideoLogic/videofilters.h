#ifndef VIDEOFILTERS_INC 
#define VIDEOFILTERS_INC 

#include "VideoLogic.h"
#include <gadgets\gadbase.h>
#include <imageproc\videologic.h>

#ifdef THIS_MODULENAME 
    #undef THIS_MODULENAME 
#endif

#define THIS_MODULENAME "VideoLogic.Video_XOR"

class Video_XOR: public CTwoPinCollector
{
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    DECLARE_RUNTIME_GADGET(Video_XOR);
};

class Video_XORB: public CTwoPinCollector
{
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    DECLARE_RUNTIME_GADGET(Video_XORB);
};

class Video_AND: public CTwoPinCollector
{
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    DECLARE_RUNTIME_GADGET(Video_AND);
};

class Video_Mask: public CTwoPinCollector
{
public:
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
  DECLARE_RUNTIME_GADGET(Video_Mask);
};

class Video_ANDB: public CTwoPinCollector
{
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    DECLARE_RUNTIME_GADGET(Video_ANDB);
};

class Video_OR: public CTwoPinCollector
{
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    DECLARE_RUNTIME_GADGET(Video_OR);
};

class Video_ORB: public CTwoPinCollector
{
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    DECLARE_RUNTIME_GADGET(Video_ORB);
};

class Video_SUB: public CTwoPinCollector
{
    int m_iOffset ;
public:
    Video_SUB() { m_iOffset = 0 ; } ;
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    virtual bool PrintProperties( FXString& text );
    virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
    virtual bool ScanSettings( FXString& text );
    DECLARE_RUNTIME_GADGET( Video_SUB );
};

class Video_EQ: public CTwoPinCollector
{
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    DECLARE_RUNTIME_GADGET(Video_EQ);
};


#endif