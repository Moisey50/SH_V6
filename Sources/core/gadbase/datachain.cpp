// DataChain.cpp: implementation of the CDataChain class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\datachain.h>
#include <gadgets\videoframe.h>
#include <gadgets\containerframe.h>
#include <gadgets\textframe.h>
#include <gadgets\metafileframe.h>
#include <gadgets\gadbase.h>

#define THIS_MODULENAME "CDataChain"

__forceinline char* _getlabel(FLWDataFrame* dt)
{
  return (((char*)dt)+dt->uLabelOffset);
}

__forceinline char* _getattributes(FLWDataFrame* dt)
{
  return (((char*)dt)+dt->uAttributesOffset);
}

__forceinline __int64* _getunsignedData(FLWDataFrame* dt)
{
  return (__int64*)(((char*)dt)+dt->uFrameDataOffset);
}

__forceinline void* _getPntr(FLWDataFrame* dt, __int64 offset)
{
  return (((char*)dt)+offset);
}

__forceinline char* _getData(FLWDataFrame* dt)
{
  return (((char*)dt)+dt->uFrameDataOffset);
}

inline BOOL AnalyzeTiming( double& dValueNow , double& dMin , double& dMax , 
  double dTimeNow , double& dTimeOfMax )
{
  SetMinMax( dValueNow , dMin , dMax );
  if ( dValueNow == dMax )
  {
    dTimeOfMax = dTimeNow ;
    return TRUE ;
  }
  else if ( dTimeNow - dTimeOfMax > 3000. )
  {
    dTimeOfMax = dTimeNow ;
    dMax = dValueNow ;
  }
  return FALSE ;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataChain::CDataChain(CGadget* host):
  m_HostGadget(host)
{
}

CDataChain::~CDataChain()
{
}

FLWDataFrame* CDataChain::Serialize(const CDataFrame *df) const
{
  FLWDataFrame* dt=NULL;
  CFramesIterator* Iterator = df->CreateFramesIterator(transparent);
  if (Iterator)
  {
    CDataFrame* pFrame = NULL;
    FXArray<FLWDataFrame*,FLWDataFrame*> frames;
    while (pFrame = Iterator->Next(NULL))
    {
      TRACE("Found frame %s\n",Tvdb400_TypeToStr(pFrame->GetDataType()));
      FLWDataFrame* sf=Serialize(pFrame);
      if (sf)
        frames.Add(sf);
    }
    delete Iterator;
    FXSIZE dReq=sizeof(FLWDataFrame);
    dReq+=(strlen(df->GetLabel())+1);
    dReq+=(df->Attributes()->GetLength()+1);
    dReq+=(frames.GetUpperBound()+1)*sizeof(unsigned);
    dt=(FLWDataFrame*)malloc(dReq);
    dt->uLabel=HUNK;
    dt->uSize= ( UINT ) dReq;
    dt->dWriteTime=m_HostGadget->GetGraphTime() * 1.e-3 ; 
    dt->uFrames=(UINT)frames.GetUpperBound()+1;
    dt->dwID=df->GetId();
    dt->dTime=df->GetTime();
    dt->uDataType=df->GetDataType();
    dt->bRegistered = df->IsRegistered() ;
    dt->uLabelOffset=sizeof(FLWDataFrame);
    dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
    dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1+ ( UINT ) df->Attributes()->GetLength()+1;
    strcpy(_getlabel(dt),df->GetLabel());
    strcpy(_getattributes(dt),*df->Attributes());
    // FLW file format for container
    // 1. Filled FLWDataFrame structure (#Pragma pack(1) !!!!!!)
    //    IMPORTANT - uFrames field should consist of number of included into container frames
    //                When not container frame serialized, this field holds 0
    //    Field uSize holds full length of serialized container frame
    // 2. Label text with trailing zero (if no label, than only one byte zero)
    // 3. Attributes text with trailing zero (if no attributes, than only one byte zero)
    // 4. Array of __i64 lengths for included into container frames
    // 5. Serialized frames 

    for (FXSIZE i=0; i<=frames.GetUpperBound(); i++)
    {
      _getunsignedData(dt)[i]=dReq;
      int sz= (int) frames[i]->uSize;
      dReq += sz;
      dt=(FLWDataFrame*)realloc(dt,dReq);
      memcpy(_getPntr(dt,_getunsignedData(dt)[i]),frames[i],(FXSIZE)sz);
      dt->uSize= ( UINT ) dReq;
      free(frames[i]);
    }
  }
  else 
  {
    datatype dtype = df->GetDataType();
    switch (dtype)
    {
    case transparent:
    case nulltype:
      {
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+= ( UINT ) (strlen(df->GetLabel())+1);
        dReq+= ( UINT ) (df->Attributes()->GetLength()+1);
        dt=(FLWDataFrame*)malloc((size_t)dReq);
        dt->uLabel=HUNK;
        dt->uSize=(UINT)dReq;
        dt->uFrames=0;
        dt->dwID=df->GetId();
        dt->dTime=df->GetTime();
        dt->uDataType=df->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset=0; // not present
        strcpy(_getlabel(dt),df->GetLabel());
        strcpy(_getattributes(dt),*df->Attributes());
        break;
      }
    case vframe:
      {
        const CVideoFrame* vf=df->GetVideoFrame(DEFAULT_LABEL);
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+= ( UINT ) (strlen(df->GetLabel())+1);
        dReq+= ( UINT ) (df->Attributes()->GetLength()+1);
        dReq+=getsize4BMIH(vf);

        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=df->GetId();
        dt->dTime=df->GetTime();
        dt->uDataType=df->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1+ ( UINT ) df->Attributes()->GetLength()+1;
        strcpy(_getlabel(dt),df->GetLabel());
        strcpy(_getattributes(dt),*df->Attributes());
        copy2BMIH((LPBITMAPINFOHEADER)_getData(dt),vf);
        break;
      }
    case text:
      {
        const CTextFrame* tf=df->GetTextFrame(DEFAULT_LABEL);
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+= ( UINT ) (strlen(tf->GetLabel())+1);
        dReq+= ( UINT ) (tf->Attributes()->GetLength()+1);
        dReq+= ( UINT ) tf->GetString().GetLength()+1;

        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=tf->GetId();
        dt->dTime=tf->GetTime();
        dt->uDataType=tf->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1+ ( UINT ) df->Attributes()->GetLength()+1;
        strcpy(_getlabel(dt),df->GetLabel());
        strcpy(_getattributes(dt),*df->Attributes());
        strcpy(_getData(dt),tf->GetString()); 
        break;
      }
    case wave:
      {
        const CWaveFrame* wf=df->GetWaveFrame(DEFAULT_LABEL);
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+=(strlen(df->GetLabel())+1);
        dReq+=(df->Attributes()->GetLength()+1);
        bool empty=(wf->waveformat==0);
        if (!empty)
          dReq+=sizeof(WAVEFORMATEX)+sizeof(WAVEHDR)+wf->data->dwBytesRecorded;

        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=wf->GetId();
        dt->dTime=wf->GetTime();
        dt->uDataType=wf->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1+ ( UINT ) df->Attributes()->GetLength()+1;
        strcpy(_getlabel(dt),df->GetLabel());
        strcpy(_getattributes(dt),*df->Attributes());
        if (!empty)
        {
          LPWAVEFORMATEX lpwf=(LPWAVEFORMATEX)_getData(dt); memcpy(lpwf,wf->waveformat,sizeof(WAVEFORMATEX));
          LPWAVEHDR      lpwh=(LPWAVEHDR)(_getData(dt)+sizeof(WAVEFORMATEX)); memcpy(lpwh,wf->data,sizeof(WAVEHDR));
          lpwh->lpData=(LPSTR)(((LPBYTE)lpwh)+sizeof(WAVEHDR));
          memcpy(lpwh->lpData,wf->data->lpData,wf->data->dwBytesRecorded);
        }
        break;
      }
    case quantity:
      {
        const CQuantityFrame* qf=df->GetQuantityFrame(DEFAULT_LABEL);
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+= ( UINT ) (strlen(qf->GetLabel())+1);
        dReq+= ( UINT ) (qf->Attributes()->GetLength()+1);
        dReq+= ( UINT )sizeof(GENERICQUANTITY);

        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=qf->GetId();
        dt->dTime=qf->GetTime();
        dt->uDataType=qf->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1+ ( UINT ) df->Attributes()->GetLength()+1;
        strcpy(_getlabel(dt),qf->GetLabel());
        strcpy(_getattributes(dt),*qf->Attributes());
        LPGENERICQUANTITY lpQF=(LPGENERICQUANTITY)qf;
        memcpy(_getData(dt),lpQF,sizeof(GENERICQUANTITY));
        break;
      }
    case logical:
      {
        const CBooleanFrame* lf = df->GetBooleanFrame(DEFAULT_LABEL);
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+= ( UINT ) (strlen(lf->GetLabel())+1);
        dReq+= ( UINT ) (lf->Attributes()->GetLength()+1);
        dReq+=sizeof(bool);

        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=lf->GetId();
        dt->dTime=lf->GetTime();
        dt->uDataType=lf->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1+ ( UINT ) df->Attributes()->GetLength()+1;
        strcpy(_getlabel(dt),lf->GetLabel());
        strcpy(_getattributes(dt),*lf->Attributes());
        bool value=lf->operator bool();
        memcpy(_getData(dt),&value,sizeof(bool));
        break;
      }
    case rectangle:
      {
        const CRectFrame* tf = df->GetRectFrame(DEFAULT_LABEL);
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+= ( UINT ) (strlen(tf->GetLabel())+1);
        dReq+= ( UINT ) (tf->Attributes()->GetLength()+1);
        dReq += sizeof(CRect);

        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=tf->GetId();
        dt->dTime=tf->GetTime();
        dt->uDataType=tf->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1+ ( UINT ) df->Attributes()->GetLength()+1;
        strcpy(_getlabel(dt),df->GetLabel());
        strcpy(_getattributes(dt),*df->Attributes());
        memcpy(_getData(dt) , (CRect*)tf , sizeof(CRect) ) ; 
        SENDERR_0("Can't serialize rectangle dataframe");
        break;
      }
    case figure:
      {
        const CFigureFrame* ff=df->GetFigureFrame(DEFAULT_LABEL);
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+= ( UINT ) (strlen(ff->GetLabel())+1);
        dReq+= ( UINT ) (ff->Attributes()->GetLength()+1);
        dReq+= ( UINT ) ff->GetNumberVertex()*sizeof(DPOINT);

        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=ff->GetId();
        dt->dTime=ff->GetTime();
        dt->uDataType=ff->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(ff->GetLabel())+1;
        dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(ff->GetLabel())+1+ ( UINT ) ff->Attributes()->GetLength()+1;
        strcpy(_getlabel(dt),ff->GetLabel());
        strcpy(_getattributes(dt),*ff->Attributes());
        for (FXSIZE i=0; i<ff->GetNumberVertex(); i++)
        {
          ((DPOINT*)_getData(dt))[i].x=(*ff)[i].x;
          ((DPOINT*)_getData(dt))[i].y=(*ff)[i].y;
        }
        break;
      }
    case metafile:
      {
        LPVOID data;
        UINT   size;
        const CMetafileFrame* mf=df->GetMetafileFrame(DEFAULT_LABEL);
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+=(strlen(mf->GetLabel())+1);
        dReq+=(mf->Attributes()->GetLength()+1);

        mf->GetMFData(&data,&size);
        dReq+=size;

        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=mf->GetId();
        dt->dTime=mf->GetTime();
        dt->uDataType=mf->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1+ ( UINT ) df->Attributes()->GetLength()+1;
        strcpy(_getlabel(dt),mf->GetLabel());
        strcpy(_getattributes(dt),*mf->Attributes());
        memcpy(_getData(dt),data,size);
        break;
      }
    case userdata:
      {
        CUserDataFrame * uf = (CUserDataFrame *)(df->GetUserDataFrame( NULL , NULL )) ; // any type, any label
        __int64 dReq=sizeof(FLWDataFrame);
        dReq+= ( UINT ) (strlen(df->GetLabel())+1);
        dReq+= ( UINT ) (df->Attributes()->GetLength()+1);
        dReq += ( UINT ) (strlen(uf->GetUserType()) + 1) ;
        __int64 uiBeforeUserData = dReq ;
        dReq += uf->GetThisFrameDataLen() + uf->GetThisFrameDataLenLen() ;
        dt=(FLWDataFrame*)malloc( (size_t) dReq);
        dt->uLabel=HUNK;
        dt->uSize= ( UINT ) dReq;
        dt->uFrames=0;
        dt->dwID=df->GetId();
        dt->dTime=df->GetTime();
        dt->uDataType=df->GetDataType();
        dt->bRegistered = df->IsRegistered() ;
        dt->uLabelOffset=sizeof(FLWDataFrame);
        dt->uAttributesOffset=sizeof(FLWDataFrame)+ ( UINT ) strlen(df->GetLabel())+1;
        dt->uFrameDataOffset = ( UINT ) uiBeforeUserData; // User data beginning
        strcpy(_getlabel(dt),df->GetLabel());
        strcpy(_getattributes(dt),*df->Attributes());
        FXSIZE uiWRittenUserLen = 0 ;
        LPBYTE pBegin = (LPBYTE)_getData( dt ) ;
        uf->SerializeUserData( &pBegin , &uiWRittenUserLen ) ;
      }
      break ;
    }
  }
  if ( dt )
    dt->uStreamNmb = 0 ;

  return dt;
}

FLWDataFrame* CDataChain::Serialize( const CDataFrame *df , 
  LPBYTE pBuffer , __int64& CurrentWriteIndex , __int64 iBufLen ) 
{
  
  FXSIZE LabelLen , AttribLen ;
  UINT uiSerLen = GetNecessaryBufferSize( df , LabelLen , &AttribLen ) ;
  if ( CurrentWriteIndex + uiSerLen > iBufLen )
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Frame" ,
      0 , "Buffer Length %ld is not enough %ld (Add=%u)" , 
      iBufLen , CurrentWriteIndex + uiSerLen ,uiSerLen ) ;
    return NULL ;
  }

  __int64 iInitialIndex = CurrentWriteIndex ;

  __int64 i64Req = SerializeCommonData( pBuffer + CurrentWriteIndex ,
    iBufLen - CurrentWriteIndex , df ) ;
  __int64 iLastWriteIndex = CurrentWriteIndex + i64Req ;

  FLWDataFrame* dt = ( FLWDataFrame* ) ( pBuffer + CurrentWriteIndex ) ;
  CurrentWriteIndex = iLastWriteIndex ;

  CFramesIterator* Iterator = df->CreateFramesIterator( transparent );
  if ( Iterator )
  {
    // FLW file format for container
    // 1. Filled FLWDataFrame structure (#Pragma pack(1) !!!!!!)
    //    IMPORTANT - uFrames field should consist of number of included into container frames
    //                When not container frame serialized, this field holds 0
    //    Field uSize holds full length of serialized container frame
    // 2. Label text with trailing zero (if no label, than only one byte zero)
    // 3. Attributes text with trailing zero (if no attributes, than only one byte zero)
    // 4. Array of __i64 indexes for included into container frames 
    //    index points to frame beginning relatively to end of array of indexes)
    // 5. Serialized frames 

    CContainerFrame* pCdf = ( CContainerFrame* ) df ;
    FXInt64Array * pLengths = pCdf->GetLengthsOfFrames() ;
//     while ( pFrame = Iterator->NextChild( NULL ) )
//       Frames.push_back( pFrame ) ;

    size_t iAddition = pLengths->size() * sizeof( __int64 ) ; // reserve space for serialized frames sizes

    __int64 i64LengthArrayIndex = CurrentWriteIndex ;
    __int64 * pFrameLength = (__int64*) &pBuffer[ i64LengthArrayIndex ] ;
    CurrentWriteIndex += iAddition ;
    __int64 i64SubframesBeginIndex = CurrentWriteIndex ;

    dt->uSize += (UINT)iAddition ; // place for lengths of frame
    dt->uFrames = (UINT) pLengths->size() ;

    CDataFrame * pFrame = NULL ;
    int iFrameCnt = 0 ;
    while ( pFrame = Iterator->NextChild( NULL ) )
    {
      __int64 iIndexBefore = CurrentWriteIndex ;
      __int64 iFrameLen = pLengths->GetAt( iFrameCnt ) ;
      TRACE( "Found frame %s\n" , Tvdb400_TypeToStr( pFrame->GetDataType() ) );
      FLWDataFrame* sf = Serialize( pFrame , pBuffer , CurrentWriteIndex , iBufLen );
      if ( !sf )
      {
        FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Container" ,
          0 , "Lbuf=%ld is not enough %ld (FrLen=%ld, Ls=%li)" , 
          iBufLen , CurrentWriteIndex , CurrentWriteIndex - iIndexBefore , iFrameLen ) ;
        delete Iterator ;
        return NULL ;
      }
      __int64 iSize = CurrentWriteIndex - iIndexBefore ;
      _getunsignedData( dt )[ iFrameCnt++ ] = iIndexBefore - iInitialIndex ;
      dt->uSize = (UINT)( CurrentWriteIndex - iInitialIndex) ;
    }
    delete Iterator ;
  }
  else
  {
    datatype dtype = df->GetDataType();
    switch ( dtype )
    {
      case transparent:
      case nulltype:
      {
        dt->uFrameDataOffset = 0; // not present
        break;
      }
      case vframe:
      {
        const CVideoFrame* vf = df->GetVideoFrame( DEFAULT_LABEL );
        __int64 iAddition = getsize4BMIH( vf );
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Video Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }
        dt->uSize += (UINT)iAddition ;
        copy2BMIH( ( LPBITMAPINFOHEADER ) _getData( dt ) , vf );
        break;
      }
      case text:
      {
        const CTextFrame* tf = df->GetTextFrame( DEFAULT_LABEL );
        UINT uiTextLen = ( UINT ) tf->GetString().GetLength() + 1;
        __int64 iAddition = uiTextLen ;
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Text Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        strcpy( _getData( dt ) , tf->GetString() );
        break;
      }
      case wave:
      {
        const CWaveFrame* wf = df->GetWaveFrame( DEFAULT_LABEL );

        bool empty = ( wf->waveformat == 0 );
//         if ( !empty )
//         {
        __int64 iAddition = sizeof( WAVEFORMATEX ) + sizeof( WAVEHDR ) + wf->data->dwBytesRecorded;

        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Wave Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        LPWAVEFORMATEX lpwf = ( LPWAVEFORMATEX ) _getData( dt );
        memcpy( lpwf , wf->waveformat , sizeof( WAVEFORMATEX ) );
        LPWAVEHDR      lpwh = ( LPWAVEHDR ) ( _getData( dt ) + sizeof( WAVEFORMATEX ) ); 
        memcpy( lpwh , wf->data , sizeof( WAVEHDR ) );
        if ( !empty )
        {
          lpwh->lpData = ( LPSTR ) ( ( ( LPBYTE ) lpwh ) + sizeof( WAVEHDR ) );
          memcpy( lpwh->lpData , wf->data->lpData , wf->data->dwBytesRecorded );
        }
//        }
        break;
      }
      case quantity:
      {
        const CQuantityFrame* qf = df->GetQuantityFrame( DEFAULT_LABEL );
        __int64 iAddition = ( UINT )sizeof( GENERICQUANTITY );
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Quantity Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        LPGENERICQUANTITY lpQF = ( LPGENERICQUANTITY ) qf;
        memcpy( _getData( dt ) , lpQF , sizeof( GENERICQUANTITY ) );
        break;
      }
      case logical:
      {
        const CBooleanFrame* lf = df->GetBooleanFrame( DEFAULT_LABEL );
        __int64 iAddition = sizeof( bool );
        CurrentWriteIndex += i64Req ;
        if ( CurrentWriteIndex + i64Req > iBufLen )
          return NULL ;

        dt->uSize += ( UINT ) iAddition ;
        bool value = lf->operator bool();
        memcpy( _getData( dt ) , &value , sizeof( bool ) );
        break;
      }
      case rectangle:
      {
        const CRectFrame* tf = df->GetRectFrame( DEFAULT_LABEL );
        __int64 iAddition = sizeof( CRect );
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Rect Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        CRect r = *( LPRECT ) tf ;
        memcpy( _getData( dt ) , &r , sizeof( CRect ) ) ;
        break;
      }
      case figure:
      {
        const CFigureFrame* ff = df->GetFigureFrame( DEFAULT_LABEL );
        UINT uiNPoints = ( UINT)ff->GetNumberVertex() ;
        UINT uiDataLenBytes = uiNPoints * sizeof( DPOINT );
        __int64 iAddition = sizeof( int ) + uiDataLenBytes ;

        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Figure Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        *( (int*)_getData( dt ) ) = (int)ff->GetSize() ;
        memcpy( _getData( dt ) + sizeof(int) , ff->GetData() , uiDataLenBytes ) ;
        break;
      }
      case metafile:
      {
        const CMetafileFrame* mf = df->GetMetafileFrame( DEFAULT_LABEL );

        LPVOID data;
        UINT   size;
        mf->GetMFData( &data , &size );
        __int64 iAddition = size;
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize WMF Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        memcpy( _getData( dt ) , data , size );
        break;
      }
      case userdata:
      {
        CUserDataFrame * uf = ( CUserDataFrame * ) ( df->GetUserDataFrame( NULL , NULL ) ) ; // any type, any label
        i64Req += ( UINT ) ( strlen( uf->GetUserType() ) + 1 ) ;
        __int64 uiBeforeUserData = CurrentWriteIndex ;
        __int64 iAddition = uf->GetThisFrameDataLen() + uf->GetThisFrameDataLenLen() ;

        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize User Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;

        FXSIZE uiWRittenUserLen = 0 ;
        LPBYTE pBegin = ( LPBYTE ) _getData( dt ) ;
        uf->SerializeUserData( &pBegin , &uiWRittenUserLen ) ;
      }
      break ;
    }
  }
  if ( dt )
    dt->uStreamNmb = 0 ;

  return dt;
}

BOOL CDataChain::SerializeToBuf( const CDataFrame *df ,
  LPBYTE pBuffer , __int64& CurrentWriteIndex , __int64 iBufLen ) 
{
  FLWDataFrame* dt = NULL;
  __int64 iInitialIndex = CurrentWriteIndex ;

  UINT iCommonDataLen = SerializeCommonData( pBuffer + CurrentWriteIndex ,
    iBufLen - CurrentWriteIndex , df ) ;
  __int64 i64Req = iCommonDataLen ;
  __int64 iLastWriteIndex = CurrentWriteIndex + i64Req ;

  dt = ( FLWDataFrame* ) ( pBuffer + CurrentWriteIndex ) ;
  CurrentWriteIndex = iLastWriteIndex ;
  CFramesIterator* Iterator = df->CreateFramesIterator( transparent );
  if ( Iterator )
  {
    // FLW file format for container
    // 1. Filled FLWDataFrame structure (#Pragma pack(1) !!!!!!)
    //    IMPORTANT - uFrames field should consist of number of included into container frames
    //                When not container frame serialized, this field holds 0
    //    Field uSize holds full length of serialized container frame
    // 2. Label text with trailing zero (if no label, than only one byte zero)
    // 3. Attributes text with trailing zero (if no attributes, than only one byte zero)
    // 4. Array of __i64 lengths for included into container frames
    // 5. Serialized frames 

    CDataFrame* pFrame = NULL;
    vector<const CDataFrame*> Frames ;
    while ( pFrame = Iterator->Next( NULL ) )
      Frames.push_back( pFrame ) ;

    delete Iterator ;

    __int64 iAddition = Frames.size() * sizeof( unsigned ) ; // reserve space for serialized frames sizes

    CurrentWriteIndex += iAddition ;
    if ( CurrentWriteIndex > iBufLen )
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Container" ,
        0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
      return NULL ;
    }

    dt->uFrames = ( UINT ) Frames.size() ;


    for ( auto It = Frames.begin() ; It < Frames.end() ; It++ )
    {
      __int64 iIndexBefore = CurrentWriteIndex ;
      TRACE( "Found frame %s Id=%u Lab=%s\n" , 
        (*It)->IsContainer() ? _T("Container") : (LPCTSTR)Tvdb400_TypeToStr( ( *It )->GetDataType() ) , 
        (*It)->GetId() , (*It)->GetLabel() );
      FLWDataFrame* sf = Serialize( *It , pBuffer , CurrentWriteIndex , iBufLen );
      if ( !sf )
      {
        FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Container" ,
          0 , "Lbuf=%ld is not enough %ld (FrLen=%ld)" ,
          iBufLen , CurrentWriteIndex , CurrentWriteIndex - iIndexBefore ) ;
        return NULL ;
      }
      __int64 iSize = CurrentWriteIndex - iIndexBefore ;
      _getunsignedData( dt )[ It - Frames.begin() ] = iIndexBefore;
      dt->uSize = ( UINT ) ( CurrentWriteIndex - iInitialIndex ) ;
    }
  }
  else
  {
    datatype dtype = df->GetDataType();
    switch ( dtype )
    {
      case transparent:
      case nulltype:
      {
        dt->uFrameDataOffset = 0; // not present
        break;
      }
      case vframe:
      {
        const CVideoFrame* vf = df->GetVideoFrame( DEFAULT_LABEL );
        __int64 iAddition = getsize4BMIH( vf );
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Video Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }
        dt->uSize += ( UINT ) iAddition ;
        copy2BMIH( ( LPBITMAPINFOHEADER ) _getData( dt ) , vf );
        break;
      }
      case text:
      {
        const CTextFrame* tf = df->GetTextFrame( DEFAULT_LABEL );
        UINT uiTextLen = ( UINT ) tf->GetString().GetLength() + 1;
        __int64 iAddition = uiTextLen ;
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Text Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        strcpy( _getData( dt ) , tf->GetString() );
        break;
      }
      case wave:
      {
        const CWaveFrame* wf = df->GetWaveFrame( DEFAULT_LABEL );

        bool empty = ( wf->waveformat == 0 );
//         if ( !empty )
//         {
        __int64 iAddition = sizeof( WAVEFORMATEX ) + sizeof( WAVEHDR ) + wf->data->dwBytesRecorded;

        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Wave Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        LPWAVEFORMATEX lpwf = ( LPWAVEFORMATEX ) _getData( dt );
        memcpy( lpwf , wf->waveformat , sizeof( WAVEFORMATEX ) );
        LPWAVEHDR      lpwh = ( LPWAVEHDR ) ( _getData( dt ) + sizeof( WAVEFORMATEX ) );
        memcpy( lpwh , wf->data , sizeof( WAVEHDR ) );
        if ( !empty )
        {
          lpwh->lpData = ( LPSTR ) ( ( ( LPBYTE ) lpwh ) + sizeof( WAVEHDR ) );
          memcpy( lpwh->lpData , wf->data->lpData , wf->data->dwBytesRecorded );
        }
//        }
        break;
      }
      case quantity:
      {
        const CQuantityFrame* qf = df->GetQuantityFrame( DEFAULT_LABEL );
        __int64 iAddition = ( UINT )sizeof( GENERICQUANTITY );
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Quantity Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        LPGENERICQUANTITY lpQF = ( LPGENERICQUANTITY ) qf;
        memcpy( _getData( dt ) , lpQF , sizeof( GENERICQUANTITY ) );
        break;
      }
      case logical:
      {
        const CBooleanFrame* lf = df->GetBooleanFrame( DEFAULT_LABEL );
        __int64 iAddition = sizeof( bool );
        CurrentWriteIndex += i64Req ;
        if ( CurrentWriteIndex + i64Req > iBufLen )
          return NULL ;

        dt->uSize += ( UINT ) iAddition ;
        bool value = lf->operator bool();
        memcpy( _getData( dt ) , &value , sizeof( bool ) );
        break;
      }
      case rectangle:
      {
        const CRectFrame* tf = df->GetRectFrame( DEFAULT_LABEL );
        __int64 iAddition = sizeof( CRect );
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Rect Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        memcpy( _getData( dt ) , ( CRect* ) tf , sizeof( CRect ) ) ;
        break;
      }
      case figure:
      {
        const CFigureFrame* ff = df->GetFigureFrame( DEFAULT_LABEL );
        UINT uiNPoints = ( UINT ) ff->GetNumberVertex() ;
        UINT uiDataLenBytes = uiNPoints * sizeof( DPOINT );
        __int64 iAddition = sizeof( int ) + uiDataLenBytes ;

        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize Figure Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        *( ( int* ) _getData( dt ) ) = ( int ) ff->GetSize() ;
        memcpy( _getData( dt ) + sizeof( int ) , ff->GetData() , uiDataLenBytes ) ;
        break;
      }
      case metafile:
      {
        const CMetafileFrame* mf = df->GetMetafileFrame( DEFAULT_LABEL );

        LPVOID data;
        UINT   size;
        mf->GetMFData( &data , &size );
        __int64 iAddition = size;
        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize WMF Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;
        memcpy( _getData( dt ) , data , size );
        break;
      }
      case userdata:
      {
        CUserDataFrame * uf = ( CUserDataFrame * ) ( df->GetUserDataFrame( NULL , NULL ) ) ; // any type, any label
        i64Req += ( UINT ) ( strlen( uf->GetUserType() ) + 1 ) ;
        __int64 uiBeforeUserData = CurrentWriteIndex ;
        __int64 iAddition = uf->GetThisFrameDataLen() + uf->GetThisFrameDataLenLen() ;

        CurrentWriteIndex += iAddition ;
        if ( CurrentWriteIndex > iBufLen )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , "FLW Serialize User Frame" ,
            0 , "Lbuf=%ld is not enough %ld (Add=%ld)" , iBufLen , CurrentWriteIndex , iAddition ) ;
          return NULL ;
        }

        dt->uSize += ( UINT ) iAddition ;

        FXSIZE uiWRittenUserLen = 0 ;
        LPBYTE pBegin = ( LPBYTE ) _getData( dt ) ;
        uf->SerializeUserData( &pBegin , &uiWRittenUserLen ) ;
      }
      break ;
    }
  }
  if ( dt )
    dt->uStreamNmb = 0 ;

  return FALSE ;
}


BOOL CDataChain::SerializeSeparateVideoFrameToFile( const CDataFrame * df , FILE * pFile )
{
  const CVideoFrame * pV = df->GetVideoFrame() ;
  if ( !pV )
    return FALSE ;

  FLWDataFrame CommonData ;
  UINT uiLabelLen = ( UINT ) ( strlen( df->GetLabel() ) + 1 );
  UINT uiAttribLen = ( UINT ) ( df->Attributes()->GetLength() + 1 );
  UINT uiFLWFrameSize = sizeof( FLWDataFrame );
  uiFLWFrameSize += uiLabelLen + uiAttribLen ;

  FXSIZE WriteBMIHSize = ( pV->lpData == 0 ) ? 0 : pV->lpBMIH->biSize ;
  FXSIZE WriteFullImageSize = ( pV->lpData == 0 ) ? getsize4BMIH( pV ) : GetImageSize( pV ) ;
  uiFLWFrameSize += (UINT)(WriteBMIHSize + WriteFullImageSize) ;

  CommonData.uLabel = HUNK;
  CommonData.uSize = uiFLWFrameSize ;
  CommonData.uStreamNmb = 0 ;
  CommonData.dWriteTime = m_HostGadget->GetGraphTime() * 1.e-3 ;
  CommonData.uFrames = 0; // for non container frame
  CommonData.dwID = df->GetId();
  CommonData.dTime = df->GetTime();
  CommonData.uDataType = df->GetDataType();
  CommonData.bRegistered = df->IsRegistered() ;
  CommonData.uLabelOffset = sizeof( FLWDataFrame );
  CommonData.uAttributesOffset = sizeof( FLWDataFrame ) + uiLabelLen;
  CommonData.uFrameDataOffset = sizeof( FLWDataFrame ) + uiLabelLen + uiAttribLen ;
  double dStart = GetHRTickCount() ;
  size_t sz = fwrite( &CommonData , 1 , sizeof( CommonData ) , pFile ) ;
  m_dWriteFLWHeaderTime_ms = GetHRTickCount() - dStart ;
  AnalyzeTiming( m_dWriteFLWHeaderTime_ms ,
    m_dWriteFLWHeaderTimeMin_ms , m_dWriteFLWHeaderTimeMax_ms ,
    dStart , m_dWriteFLWHeaderTimeMaxTime_ms ) ;
  UINT uiWritten = (UINT)sz ;

  dStart = GetHRTickCount() ;
  if ( uiLabelLen > 1 )
    sz = fwrite( df->GetLabel() , 1 , uiLabelLen , pFile ) ;
  else
    sz = putc( 0 , pFile ) ;
  m_dWriteLabelTime_ms = GetHRTickCount() - dStart ;
  AnalyzeTiming( m_dWriteLabelTime_ms ,
    m_dWriteLabelTimeMin_ms , m_dWriteLabelTimeMax_ms ,
    dStart , m_dWriteLabelTimeMaxTime_ms ) ;
  uiWritten += ( UINT ) sz ;

  dStart = GetHRTickCount() ;
  if ( uiAttribLen > 1 )
    size_t sz = fwrite( (LPCTSTR)(*(df->Attributes())) , 1 , uiAttribLen , pFile ) ;
  else
    sz = putc( 0 , pFile ) ; 
  m_dWriteAttribTime_ms = GetHRTickCount() - dStart ;
  AnalyzeTiming( m_dWriteAttribTime_ms ,
    m_dWriteAttribTimeMin_ms , m_dWriteAttribTimeMax_ms ,
    dStart , m_dWriteAttribTimeMaxTime_ms ) ;
  uiWritten += ( UINT ) sz ;

  dStart = GetHRTickCount() ;
  if ( WriteBMIHSize )
  {
    sz = fwrite( &WriteBMIHSize , 1 , sizeof( WriteBMIHSize ) , pFile ) ;
    m_dWriteBMIHTimeMaxTime_ms = GetHRTickCount() - dStart ;
    AnalyzeTiming( m_dWriteBMIHTimeMaxTime_ms ,
      m_dWriteBMIHTimeMin_ms , m_dWriteBMIHTimeMax_ms ,
      dStart , m_dWriteBMIHTimeMaxTime_ms ) ;
    uiWritten += ( UINT ) sz ;

    dStart = GetHRTickCount() ;
    sz = fwrite( GetData( pV ) , 1 , WriteFullImageSize , pFile ) ;
    m_dWriteImageTime_ms = GetHRTickCount() - dStart ;
    AnalyzeTiming( m_dWriteImageTime_ms ,
      m_dWriteImageTimeMin_ms , m_dWriteImageTimeMax_ms ,
      dStart , m_dWriteImageTimeMaxTime_ms ) ;
  }
  else
  {
    m_dWriteBMIHTimeMaxTime_ms = 0. ;
    AnalyzeTiming( m_dWriteBMIHTimeMaxTime_ms ,
      m_dWriteBMIHTimeMin_ms , m_dWriteBMIHTimeMax_ms ,
      dStart , m_dWriteBMIHTimeMaxTime_ms ) ;
    sz = fwrite( GetData( pV ) , 1 , WriteFullImageSize , pFile ) ;
    m_dWriteImageTime_ms = GetHRTickCount() - dStart ;
    AnalyzeTiming( m_dWriteImageTime_ms ,
      m_dWriteImageTimeMin_ms , m_dWriteImageTimeMax_ms ,
      dStart , m_dWriteImageTimeMaxTime_ms ) ;
  }
  uiWritten += ( UINT ) sz ;

  return TRUE ;
}

UINT CDataChain::GetNecessaryBufferSize( const CDataFrame * df , FXSIZE& LabelLen , FXSIZE * pAttribLen )
{
  __int64 i64SerLen , i64FLWSerLen;
  if ( !df->IsContainer() )
  {
    // simple length for serialization through channel
    i64SerLen = df->GetSerializeLength( LabelLen , pAttribLen ) ;
    i64FLWSerLen = i64SerLen + ( sizeof( FLWDataFrame ) - sizeof( CDataFrame::DestSerDataFrame ) + 1 );
#ifdef SERIALIZE_DEBUG
    TRACE( "CDataChain::GetNecessaryBufferSize - %s Lab=%s Lser=%li Lflw=%li\n " , 
      Tvdb400_TypeToStr( df->GetDataType() ) , 
      df->GetLabel() , i64SerLen , i64FLWSerLen ) ;
#endif
  }
  else
  {
    UINT uiNFramesInContainer = 0 ; // this frame
    // Calculate length for serialization through channel
    i64SerLen = (( ( CContainerFrame* ) df )->GetSerializeLength( LabelLen , uiNFramesInContainer , pAttribLen ) ) ;
    i64FLWSerLen = i64SerLen +
          // addition for file serialization (difference between FLWDataFrame and SetSerDataFrame)
         // (uiNFramesInContainer + 1) : add length for sub frames and for container self
       (uiNFramesInContainer + 1) * ( sizeof( FLWDataFrame ) - sizeof( CDataFrame::DestSerDataFrame ) + 1 ) 
      + uiNFramesInContainer * sizeof( __int64 ) ; // Addition for sub frames offsets (one per sub frame)
#ifdef SERIALIZE_DEBUG
    TRACE( "CDataChain::GetNecessaryBufferSize - Container Lab=%s Lser=%li Lflw=%li Nasf=%u\n " , df->GetLabel() ,
      i64SerLen , i64FLWSerLen ,
      ( ( CContainerFrame* ) df )->GetNAllSubFrames() ) ;
#endif
  }

   return (UINT)i64FLWSerLen ;
}

int CDataChain::SerializeCommonData( LPBYTE pBuf , FSIZE iBufLen , const CDataFrame * df )
{
  FLWDataFrame& CommonData = ( FLWDataFrame& ) pBuf[ 0 ] ;
  UINT uiLabelLen = ( UINT ) ( strlen( df->GetLabel() ) + 1 );
  UINT uiAttribLen = ( UINT ) ( df->Attributes()->GetLength() + 1 );
  UINT uiFLWFrameSize = sizeof( FLWDataFrame );
  UINT uiAddedSize = uiFLWFrameSize + uiLabelLen + uiAttribLen ;

  if ( iBufLen >= uiAddedSize )
  {
    CommonData.uLabel = HUNK;
    CommonData.uSize = uiAddedSize ;
    CommonData.uStreamNmb = 0 ;
    CommonData.dWriteTime = m_HostGadget->GetGraphTime() * 1.e-3 ;
    CommonData.uFrames = 0; // for non container frame
    CommonData.dwID = df->GetId();
    CommonData.dTime = df->GetTime();
    CommonData.uDataType = df->GetDataType();
    CommonData.bRegistered = df->IsRegistered() ;
    CommonData.uLabelOffset = sizeof( FLWDataFrame );
    CommonData.uAttributesOffset = sizeof( FLWDataFrame ) + uiLabelLen;
    CommonData.uFrameDataOffset = sizeof( FLWDataFrame ) + uiLabelLen + uiAttribLen ;

    strcpy_s( _getlabel( &CommonData ) , (UINT)iBufLen - sizeof( FLWDataFrame ) , df->GetLabel() );
    strcpy_s( _getattributes( &CommonData ) , (UINT)iBufLen - sizeof( FLWDataFrame ) - uiLabelLen , *df->Attributes() );

    return uiAddedSize ;
  }
  else
    return -(int) uiAddedSize ;
}

int CDataChain::SerializeSeparateVideoFrameToBuf( LPBYTE pBuf , UINT iBufLen , const CDataFrame * df )
{
  const CVideoFrame * pV = df->GetVideoFrame() ;
  if ( !pV )
    return FALSE ;

  FLWDataFrame& CommonData = ( FLWDataFrame& ) pBuf[0] ;
  UINT uiLabelLen = ( UINT ) ( strlen( df->GetLabel() ) + 1 );
  UINT uiAttribLen = ( UINT ) ( df->Attributes()->GetLength() + 1 );
  UINT uiFLWFrameSize = sizeof( FLWDataFrame );
  uiFLWFrameSize += uiLabelLen + uiAttribLen ;

  FXSIZE WriteBMIHSize = ( pV->lpData == 0 ) ? 0 : pV->lpBMIH->biSize ;
  FXSIZE WriteFullImageSize = ( pV->lpData == 0 ) ? getsize4BMIH( pV ) : GetImageSize( pV ) ;
  uiFLWFrameSize += ( UINT ) ( WriteBMIHSize + WriteFullImageSize ) ;
  if ( iBufLen >= uiFLWFrameSize )
  {
    CommonData.uLabel = HUNK;
    CommonData.uSize = uiFLWFrameSize ;
    CommonData.uStreamNmb = 0 ;
    CommonData.dWriteTime = m_HostGadget->GetGraphTime() * 1.e-3 ;
    CommonData.uFrames = 0; // for non container frame
    CommonData.dwID = df->GetId();
    CommonData.dTime = df->GetTime();
    CommonData.uDataType = df->GetDataType();
    CommonData.bRegistered = df->IsRegistered() ;
    CommonData.uLabelOffset = sizeof( FLWDataFrame );
    CommonData.uAttributesOffset = sizeof( FLWDataFrame ) + uiLabelLen;
    CommonData.uFrameDataOffset = sizeof( FLWDataFrame ) + uiLabelLen + uiAttribLen ;

    strcpy_s( _getlabel( &CommonData ) , iBufLen - sizeof(FLWDataFrame) , df->GetLabel() );
    strcpy_s( _getattributes( &CommonData ) , iBufLen - sizeof( FLWDataFrame ) - uiLabelLen , *df->Attributes() );

    if ( WriteBMIHSize )
    {
      memcpy( _getData( &CommonData ) , pV->lpBMIH , WriteBMIHSize ) ;
      memcpy( _getData( &CommonData ) , pV->lpData , WriteFullImageSize ) ;
    }
    else
      memcpy( _getData( &CommonData ) , pV->lpBMIH , WriteFullImageSize ) ;
    return uiFLWFrameSize ;
  }
  return NULL ;
}


CDataFrame* CDataChain::Restore(FLWDataFrame *dt)
{
  CDataFrame* pNewFrame=NULL;
  if (dt->uFrames) // container frame
  {
    CContainerFrame* cf = CContainerFrame::Create();
    for (unsigned i=0; i < dt->uFrames; i++)
    {
      CDataFrame* df=Restore((FLWDataFrame*)_getPntr(dt,_getunsignedData(dt)[i]));
      if (df)
        cf->PushFrame(df);
    }
    pNewFrame=cf;
  }
  else
  {
    switch (dt->uDataType)
    {
    case transparent:
    case nulltype:
      {
        CDataFrame* vf=CDataFrame::Create();
        pNewFrame=vf;
        break;
      }
    case vframe:
      {
        CVideoFrame* vf=CVideoFrame::Create();
        vf->lpData=NULL;
        LPBITMAPINFOHEADER lpBMIH=(LPBITMAPINFOHEADER)(((char*)dt)+dt->uFrameDataOffset);
        vf->lpBMIH=(LPBITMAPINFOHEADER)malloc(getBMIHsize(lpBMIH));
        if ( vf->lpBMIH )
        {
          memcpy(vf->lpBMIH,lpBMIH,getBMIHsize(lpBMIH));
          pNewFrame=vf;
        }
        break;
      }
    case text:
      {
        CTextFrame* tf=CTextFrame::Create();
        tf->SetString(_getData(dt));
        pNewFrame=tf;
        break;
      }
    case wave:
      {
        CWaveFrame* wf=CWaveFrame::Create();

        bool empty=(dt->uSize==dt->uFrameDataOffset);
        if (!empty)
        {
          wf->waveformat=(PWAVEFORMATEX)malloc(sizeof(WAVEFORMATEX));
          memcpy(wf->waveformat, _getData(dt), sizeof(WAVEFORMATEX));

          wf->data=(LPWAVEHDR)malloc(sizeof(WAVEHDR));
          memcpy(wf->data,_getData(dt)+sizeof(WAVEFORMATEX), sizeof(WAVEHDR));

          wf->data->dwUser=NULL;
          wf->data->lpData=(LPSTR)malloc(wf->data->dwBufferLength);
          memcpy(wf->data->lpData,_getData(dt)+sizeof(WAVEFORMATEX)+sizeof(WAVEHDR),wf->data->dwBytesRecorded);
          wf->data->lpNext=NULL;
        }
        pNewFrame=wf;
        break;
      }
    case quantity:
      {
        CQuantityFrame* qf=CQuantityFrame::Create(0);
        LPGENERICQUANTITY lpQF=(LPGENERICQUANTITY)qf;
        memcpy(lpQF,_getData(dt),sizeof(GENERICQUANTITY));
        pNewFrame=qf;
        break;
      }
    case logical:
      {
        CBooleanFrame* lf = CBooleanFrame::Create();
        bool value;
        memcpy(&value,_getData(dt),sizeof(bool));
        lf->SetValue(value);
        pNewFrame=lf;
        break;
      }
    case rectangle:
      {
//         SENDERR_0("Can't deserialize rectangle dataframe");
        CRect rect ;
        memcpy(&rect,_getData(dt),sizeof(CRect));
        CRectFrame* rf = CRectFrame::Create( rect );
        pNewFrame=rf;
        break;
      }
    case figure:
      {
        CFigureFrame* ff=CFigureFrame::Create();
        FXSIZE size=(FXSIZE)(dt->uSize-dt->uFrameDataOffset)/sizeof(DPOINT);
        ff->SetSize( size ) ;
        memcpy( ff->GetData() , _getData( dt ) + sizeof( UINT ) , size * sizeof(DPOINT) ) ;
        pNewFrame=ff;
        break;
      }
    case metafile:
      {
        CMetafileFrame* mf=CMetafileFrame::Create();
        FXSIZE size=(FXSIZE)(dt->uSize-dt->uFrameDataOffset);
        LPVOID data=malloc( (size_t) size);
        memcpy(data,_getData(dt),size);
        mf->SetMFData(data,(int)size);
        pNewFrame=mf;
        break;
      }
    case userdata:
      {
        long long llId = 0 ;


        LPTSTR pllId = (LPTSTR) &llId ;
        LPTSTR pName = (LPTSTR) _getData(dt) ;
        FXSIZE cnt = 0 ;
        while ( *pName && cnt++ < 8 )
          *(pllId++) = *(pName++) ;
        FXSIZE uiDataSize = (FXSIZE) (dt->uSize - dt->uFrameDataOffset) ;
        while ( *(pName++) )  ; //pass zero
        FXSIZE uiUserDataSize = *(FXSIZE*)pName ;
        FXSIZE uiRealDataSize = (FXSIZE)dt->uSize - ((LPBYTE)pName - (LPBYTE)dt) - sizeof(UINT) ; // last data len size
        if ( uiRealDataSize >= uiUserDataSize )
        {
          CUserDataFrame * uf = CUserDataFrame::CreateEmpty( llId ) ;

          if ( uf )
          {
            if ( uf->RestoreUserData( (LPBYTE)_getData(dt) , 0 ) )
            {
              pNewFrame = uf ;
              break ;
            }
            TRACE("  Can't restore UserData\n") ;
          }
          else
            TRACE( "Can't create empty UserDataFrame %s" , _getData(dt) ) ;
        }
        else
          TRACE("Not ebough length in UserData '%s' %u (necessary %u) " , _getData(dt) , uiRealDataSize , uiUserDataSize ) ;
        break ;
      }
    default:
      TRACE("Unsupported type %d\n",dt->uDataType);
      break;
    }
  }
  if ( pNewFrame )
  {
    pNewFrame->ChangeId(dt->dwID);
    pNewFrame->SetTime(dt->dTime);
    if ( dt->bRegistered )
      pNewFrame->SetRegistered() ;
    pNewFrame->SetLabel(_getlabel(dt));
    (*pNewFrame->Attributes())=_getattributes(dt);
  }
  return pNewFrame;
}

