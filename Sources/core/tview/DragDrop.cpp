#include "stdafx.h"
#include "DragDrop.h"
#include "SketchView.h"

CDataObject::CDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmed, int count){
    // reference count must ALWAYS start at 1
    m_lRefCount    = 1;
    m_nNumFormats  = count;

    m_pFormatEtc   = new FORMATETC[count];
    m_pStgMedium   = new STGMEDIUM[count];

    for(int i = 0; i < count; i++)
    {
        m_pFormatEtc[i] = fmtetc[i];
        m_pStgMedium[i] = stgmed[i];
    }
}


CDataObject::~CDataObject()
{
	// cleanup
	if(m_pFormatEtc) delete[] m_pFormatEtc;
	if(m_pStgMedium) delete[] m_pStgMedium;

	OutputDebugString("oof\n");
}


HRESULT CDataObject::CreateDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count, IDataObject **ppDataObject)
{
    if(ppDataObject == 0)
        return E_INVALIDARG;

    *ppDataObject = new CDataObject(fmtetc, stgmeds, count);

    return (*ppDataObject) ? S_OK : E_OUTOFMEMORY;
}

HRESULT __stdcall CDataObject::QueryGetData(FORMATETC *pFormatEtc)
{
    return (LookupFormatEtc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
}

int CDataObject::LookupFormatEtc(FORMATETC *pFormatEtc)
{
    // check each of our formats in turn to see if one matches
    for(int i = 0; i < m_nNumFormats; i++)
    {
        if((m_pFormatEtc[i].tymed    &  pFormatEtc->tymed)   &&
            m_pFormatEtc[i].cfFormat == pFormatEtc->cfFormat &&
            m_pFormatEtc[i].dwAspect == pFormatEtc->dwAspect)
        {
            // return index of stored format
            return i;
        }
    }

    // error, format not found
    return -1;
}

HRESULT __stdcall CDataObject::GetData (FORMATETC *pFormatEtc, STGMEDIUM *pStgMedium)
{
    int idx;
	
    // try to match the specified FORMATETC with one of our supported formats
    if((idx = LookupFormatEtc(pFormatEtc)) == -1)
        return DV_E_FORMATETC;

    // found a match - transfer data into supplied storage medium
    pStgMedium->tymed = m_pFormatEtc[idx].tymed;
    pStgMedium->pUnkForRelease = 0;
	
    // copy the data into the caller's storage medium
    switch(m_pFormatEtc[idx].tymed)
    {
    case TYMED_HGLOBAL:

        pStgMedium->hGlobal = DupGlobalMem(m_pStgMedium[idx].hGlobal);
        break;

    default:
        return DV_E_FORMATETC;
    }
	
    return S_OK;
}

HRESULT __stdcall CDataObject::EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
    // only the get direction is supported for OLE
    if(dwDirection == DATADIR_GET)
    {
        // for Win2k+ you can use the SHCreateStdEnumFmtEtc API call, however
        // to support all Windows platforms we need to implement IEnumFormatEtc ourselves.
        return CEnumFormatEtc::CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);
    }
    else
    {
        // the direction specified is not supported for drag+drop
        return E_NOTIMPL;
    }
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CDataObject::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

//
//	IUnknown::Release
//
ULONG __stdcall CDataObject::Release(void)
{
    // decrement object reference count
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CDataObject::QueryInterface(REFIID iid, void **ppvObject)
{
    // check to see what interface has been requested
    if(iid == IID_IDataObject || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

HRESULT __stdcall CDataObject::GetDataHere (FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
	// GetDataHere is only required for IStream and IStorage mediums
	// It is an error to call GetDataHere for things like HGLOBAL and other clipboard formats
	//
	//	OleFlushClipboard 
	//
	return DATA_E_FORMATETC;
}

HRESULT __stdcall CDataObject::GetCanonicalFormatEtc (FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
	// Apparently we have to set this field to NULL even though we don't do anything else
	pFormatEtcOut->ptd = NULL;
	return E_NOTIMPL;
}

//
//	IDataObject::SetData
//
HRESULT __stdcall CDataObject::SetData (FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease)
{
	return E_NOTIMPL;
}

//
//	IDataObject::DAdvise
//
HRESULT __stdcall CDataObject::DAdvise (FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

//
//	IDataObject::DUnadvise
//
HRESULT __stdcall CDataObject::DUnadvise (DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

//
//	IDataObject::EnumDAdvise
//
HRESULT __stdcall CDataObject::EnumDAdvise (IEnumSTATDATA **ppEnumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}


CEnumFormatEtc::CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats)
{
    m_lRefCount   = 1;

    m_nIndex      = 0;
    m_nNumFormats = nNumFormats;
    m_pFormatEtc  = new FORMATETC[nNumFormats];

    // make a new copy of each FORMATETC structure
    for(int i = 0; i < nNumFormats; i++)
    {
        DeepCopyFormatEtc(&m_pFormatEtc[i], &pFormatEtc[i]);
    }
}


CEnumFormatEtc::~CEnumFormatEtc()
{
    // first free any DVTARGETDEVICE structures
    for(ULONG i = 0; i < m_nNumFormats; i++)
    {
        if(m_pFormatEtc[i].ptd)
            CoTaskMemFree(m_pFormatEtc[i].ptd);
    }
	
    // now free the main array
    delete[] m_pFormatEtc;
}



HRESULT CEnumFormatEtc::Reset(void)
{
    m_nIndex = 0;
    return S_OK;
}

HRESULT CEnumFormatEtc::Skip(ULONG celt)
{
    m_nIndex += celt;
    return (m_nIndex <= m_nNumFormats) ? S_OK : S_FALSE;
}

HRESULT CEnumFormatEtc::Clone(IEnumFORMATETC **ppEnumFormatEtc)
{
    HRESULT hResult;

    // make a duplicate enumerator
    hResult = CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);

    if(hResult == S_OK)
    {
        // manually set the index state
        ((CEnumFormatEtc *)*ppEnumFormatEtc)->m_nIndex = m_nIndex;
    }

    return hResult;
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CEnumFormatEtc::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

//
//	IUnknown::Release
//
ULONG __stdcall CEnumFormatEtc::Release(void)
{
    // decrement object reference count
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CEnumFormatEtc::QueryInterface(REFIID iid, void **ppvObject)
{
    // check to see what interface has been requested
    if(iid == IID_IEnumFORMATETC || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}



HRESULT CEnumFormatEtc::Next(ULONG celt, FORMATETC *pFormatEtc, ULONG *pceltFetched)
{
    ULONG copied = 0;

    // copy the FORMATETC structures into the caller's buffer
    while(m_nIndex < m_nNumFormats && copied < celt) 
    {
        DeepCopyFormatEtc(&pFormatEtc[copied], &m_pFormatEtc[m_nIndex]);
        copied++;
        m_nIndex++;
    }

    // store result
    if(pceltFetched != 0) 
        *pceltFetched = copied;

    // did we copy all that was requested?
    return (copied == celt) ? S_OK : S_FALSE;
}

HRESULT CEnumFormatEtc::CreateEnumFormatEtc(UINT cfmt, FORMATETC *afmt, IEnumFORMATETC **ppEnumFormatEtc)
{
    if(cfmt == 0 || afmt == 0 || ppEnumFormatEtc == 0)
        return E_INVALIDARG;

    *ppEnumFormatEtc = new CEnumFormatEtc(afmt, cfmt);

    return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
}

CDropSource::CDropSource() 
{
	m_lRefCount = 1;
}

//
//	Destructor
//
CDropSource::~CDropSource()
{
}

ULONG __stdcall CDropSource::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

//
//	IUnknown::Release
//
ULONG __stdcall CDropSource::Release(void)
{
    // decrement object reference count
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CDropSource::QueryInterface(REFIID iid, void **ppvObject)
{
    // check to see what interface has been requested
    if(iid == IID_IDropSource || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

HRESULT __stdcall CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    // if the Escape key has been pressed since the last call, cancel the drop
    if(fEscapePressed == TRUE)
        return DRAGDROP_S_CANCEL;	

    // if the LeftMouse button has been released, then do the drop!
    if((grfKeyState & MK_LBUTTON) == 0)
        return DRAGDROP_S_DROP;

    // continue with the drag-drop
    return S_OK;
}

HRESULT __stdcall CDropSource::GiveFeedback(DWORD dwEffect)
{    
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

CDropTarget::CDropTarget(CSketchView* pView)
{
	m_lRefCount  = 1;
	m_pView       = pView;
	m_fAllowDrop = false;
}

//
//	Destructor for the CDropTarget class
//
CDropTarget::~CDropTarget()
{
	
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CDropTarget::QueryInterface (REFIID iid, void ** ppvObject)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CDropTarget::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}	

//
//	IUnknown::Release
//
ULONG __stdcall CDropTarget::Release(void)
{
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

HRESULT __stdcall CDropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, 
                                              POINTL pt, DWORD *pdwEffect)
{
    // does the dataobject contain data we want?
    m_fAllowDrop = QueryDataObject(pDataObject);
	
    if(m_fAllowDrop)
    {
        // get the dropeffect based on keyboard state
        *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);

        SetFocus(m_hWnd);

        PositionCursor(m_hWnd, pt);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    return S_OK;
}

bool CDropTarget::QueryDataObject(IDataObject *pDataObject)
{
    FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    
    // does the data object support CF_TEXT using a HGLOBAL?
    return pDataObject->QueryGetData(&fmtetc) == S_OK ? true : false;
}

DWORD CDropTarget::DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed)
{
	DWORD dwEffect = 0;

	// 1. check "pt" -> do we allow a drop at the specified coordinates?
	
	// 2. work out that the drop-effect should be based on grfKeyState
	if(grfKeyState & MK_CONTROL)
	{
		dwEffect = dwAllowed & DROPEFFECT_COPY;
	}
	else if(grfKeyState & MK_SHIFT)
	{
		dwEffect = dwAllowed & DROPEFFECT_MOVE;
	}
	
	// 3. no key-modifiers were specified (or drop effect not allowed), so
	//    base the effect on those allowed by the dropsource
	if(dwEffect == 0)
	{
		if(dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
		if(dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
	}
	
	return dwEffect;
}

HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
    if(m_fAllowDrop)
    {
        *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
        PositionCursor(m_hWnd, pt);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    return S_OK;
}

HRESULT __stdcall CDropTarget::DragLeave(void)
{
    return S_OK;
}

HRESULT __stdcall CDropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    PositionCursor(m_hWnd, pt);

    if(m_fAllowDrop)
    {
		if(m_pView)
			DropData(m_pView, pDataObject, CPoint(pt.x, pt.y));

        *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }
    
    return S_OK;
}

HRESULT CDropSource::CreateDropSource(IDropSource **ppDropSource)
{
	if(ppDropSource == 0)
		return E_INVALIDARG;

	*ppDropSource = new CDropSource();

	return (*ppDropSource) ? S_OK : E_OUTOFMEMORY;

}

HANDLE StringToHandle(char *szText, int nTextLen)
{
    void  *ptr;

    // if text length is -1 then treat as a nul-terminated string
    if(nTextLen == -1)
        nTextLen = lstrlen(szText) + 1;
    
    // allocate and lock a global memory buffer. Make it fixed
    // data so we don't have to use GlobalLock
    ptr = (void *)GlobalAlloc(GMEM_FIXED, nTextLen);

    // copy the string into the buffer
    memcpy(ptr, szText, nTextLen);

    return ptr;
}

HGLOBAL DupGlobalMem(HGLOBAL hMem)
{
    size_t   len    = GlobalSize(hMem);
    PVOID   source = GlobalLock(hMem);
	
    PVOID   dest   = GlobalAlloc(GMEM_FIXED, len);

    memcpy(dest, source, len);

    GlobalUnlock(hMem);
    return dest;
}

void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source)
{
    // copy the source FORMATETC into dest
    *dest = *source;
	
    if(source->ptd)
    {
        // allocate memory for the DVTARGETDEVICE if necessary
        dest->ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));

        // copy the contents of the source DVTARGETDEVICE into dest->ptd
        *(dest->ptd) = *(source->ptd);
    }
}

//
//	Position the edit control's caret under the mouse
//
void PositionCursor(HWND hwndEdit, POINTL pt)
{
	FXSIZE curpos; 
		
	// get the character position of mouse
	ScreenToClient(hwndEdit, (POINT *)&pt);
	curpos = SendMessage(hwndEdit, EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));

	// set cursor position
	SendMessage(hwndEdit, EM_SETSEL, LOWORD(curpos), LOWORD(curpos));
}

void DropData(CSketchView* pView, IDataObject *pDataObject, CPoint point)
{
	// construct a FORMATETC object
	FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;

	// See if the dataobject contains any TEXT stored as a HGLOBAL
	if(pDataObject->QueryGetData(&fmtetc) == S_OK)
	{
		// Yippie! the data is there, so go get it!
		if(pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
		{
			// we asked for the data as a HGLOBAL, so access it appropriately
			PVOID data = GlobalLock(stgmed.hGlobal);
			CString sData = _T((char *) data);
			if(sData.Find("Complex(") != 0)
					pView->InsertGadget(sData, point);
			else
			{
				CString uid;
				CString params;
				uid = sData.Left(sData.Find(")"));
				uid = uid.Mid((int)strlen("Complex("));
				params = sData.Mid((int)sData.Find("):") + (int)strlen("):"));
				pView->InsertGadget("Complex", point, params, uid);
			}
			GlobalUnlock(stgmed.hGlobal);

			// release the data using the COM API
			ReleaseStgMedium(&stgmed);
		}
	}
}
 




