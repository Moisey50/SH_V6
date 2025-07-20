// GraphBuilder.cpp: implementation of the CGraphBuilder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GraphBuilder.h"
#include <security\basesecurity.h>
#include <scriptdefinitions.h>
#include "InputPinWrrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "GraphBuilder.cpp"

const char outputCode[] = ">>";
const char inputCode[] = "<<";
const char duplexCode[] = "<>";

typedef int (__stdcall *FN_GRAPHTIME)();

//////////////////////////////////////////////////////////
// Helpers
//

void GadgetClassNameToSampleName(CString& name)
{
  if (name.IsEmpty())
    name = _T("Gadget");
}

BOOL DecodeConnectorUID(const FXString& uid, FXString& GadgetUid, 
  int& number, BOOL& output)
{
  FXSIZE posO = uid.Find(outputCode);
  FXSIZE posI = uid.Find(inputCode);
  if (posO < 0 && posI < 0)
    return FALSE;
  else if (posO >= 0 && posI >= 0)
    return FALSE;
  output = (posO > posI);
  FXSIZE pos = (output) ? posO : posI;
  GadgetUid = uid.Left(pos);
  FXString tail = uid.Mid(pos + ((output) ? strlen(outputCode) : strlen(inputCode)));
  if (tail.IsEmpty() || tail.SpanIncluding("0123456789").Compare(tail))
    return FALSE;
  number = atoi(tail);
  return TRUE;
}

BOOL DecodeDuplexConnectorUID(const FXString& uid, FXString& GadgetUid, int& number)
{
  FXSIZE pos = uid.Find(duplexCode);
  if (pos < 0)
    return FALSE;
  GadgetUid = uid.Left(pos);
  FXString tail = uid.Mid(pos + strlen(duplexCode));
  if (tail.IsEmpty() || tail.SpanIncluding("0123456789").Compare(tail))
    return FALSE;
  number = atoi(tail);
  return TRUE;
}

FXString EncodeConnectorUID(FXString& GadgetUid, int number, BOOL output)
{
  FXString uid;
  uid.Format("%s%s%d", GadgetUid, (output) ? outputCode : inputCode, number);
  return uid;
}

FXString EncodeDuplexConnectorUID(FXString& GadgetUid, int number)
{
  FXString uid;
  uid.Format("%s%s%d", GadgetUid, duplexCode, number);
  return uid;
}

//////////////////////////////////////////////////////////
// CAutoName
//

#define AUTONAMEBASE	_T("Gadget")
class CAutoName
{
  FXString m_base;
  int m_counter;
public:
  CAutoName(LPCTSTR base = AUTONAMEBASE) : m_base(base), m_counter(0) {};
  void GetNextName(FXString& name) { name.Format(_T("%s%d"), m_base, ++m_counter); };
};


//////////////////////////////////////////////////////////
// CNamedGadgets
//

CNamedGadgets::CNamedGadgets() 
{
}

CNamedGadgets::~CNamedGadgets() 
{ 
  RemoveAll(); 
}

CGadget* CNamedGadgets::Lookup(LPCTSTR uid)
{
  CGadget* Gadget;
  if (CMapStringToPtr::Lookup(uid, (LPVOID&)Gadget))
    return Gadget;
  return NULL;
}

CGadget* CNamedGadgets::GetNextGadget(POSITION& pos, FXString& uid)
{
  CGadget* Gadget = NULL;
  CString _uid;
  GetNextAssoc(pos, _uid, (LPVOID&)Gadget);
  uid=(LPCTSTR)_uid;
  return Gadget;
}

CGadget* CNamedGadgets::GetNextGadget(POSITION& pos)
{
  CGadget* Gadget = NULL;
  CString uid;
  GetNextAssoc(pos, uid, (LPVOID&)Gadget);
  return Gadget;
}

CGadget* CNamedGadgets::GetNextGadgetOrdered(FXString& uid, LPCTSTR uidLast)
{
  POSITION pos = GetStartPosition();
  FXString uidTmp;
  if (!uidLast) // init search
  {
    if (!pos)
      return NULL;
    GetNextGadget(pos, uid);
    while (pos)
    {
      GetNextGadget(pos, uidTmp);
      if (_tcscmp(uidTmp, uid) < 0)
        uid = uidTmp;
    }
    return Lookup(uid);
  }
  BOOL bFound = FALSE;
  while (pos)
  {
    GetNextGadget(pos, uidTmp);
    if (_tcscmp(uidLast, uidTmp) < 0)
    {
      if (!bFound || (_tcscmp(uidTmp, uid) < 0))
      {
        uid = uidTmp;
        bFound = TRUE;
      }
    }
  }
  if (!bFound)
    return NULL;
  return Lookup(uid);
}

void CNamedGadgets::ListUIDs(CStringArray& UIDs)
{
  POSITION pos = GetStartPosition();
  while (pos)
  {
    CString uid;
    LPVOID ptr;
    GetNextAssoc(pos, uid, ptr);
    UIDs.Add(uid);
  }
}

bool CNamedGadgets::GetGadgetUID(CGadget* gadget, FXString& name)
{
  POSITION pos = GetStartPosition();
  while (pos)
  {
    CString uid;
    CGadget* ptr;
    GetNextAssoc(pos, uid, (LPVOID&)ptr);
    if (ptr==gadget)
    {
      name=uid;
      return true;
    }
  }
  return false;
}

void CNamedGadgets::RemoveKey(FXString& uid, BOOL bEraseMemory)
{
  CGadget* Gadget = Lookup(uid);
  if (Gadget && bEraseMemory) 
  {
    Gadget->ShutDown();
    delete Gadget;
  }
  CMapStringToPtr::RemoveKey(uid);
}

void CNamedGadgets::RemoveAll()
{
  POSITION pos = GetStartPosition();
  while (pos)
  {
    CString uid;
    CGadget* Gadget;
    GetNextAssoc(pos, uid, (LPVOID&)Gadget);
    try
    {
      if (Gadget) 
      {
        Gadget->ShutDown();
        delete Gadget;
      }
    }
    catch (CMemoryException* e)
    {
      char ErrMsg[4000];
      e->GetErrorMessage( ErrMsg , sizeof(ErrMsg) );
      ErrMsg[3000] = 0;
    }
  }
  CMapStringToPtr::RemoveAll();
}

//////////////////////////////////////////////////////////
// CGraphBuilder
//

CGadget* CGraphBuilder::GetGadget(LPCTSTR uid) 
{ 
  FXString suid(uid);
  FXString cGadget;
  FXSIZE pos;
  if ((pos=suid.Find('.'))>=0)
  {
    cGadget=suid.Left(pos);
    IGraphbuilder* iGB=GetSubBuilder(cGadget);
    suid=suid.Mid(pos+1);
    if (!iGB) return NULL;
    return iGB->GetGadget(suid);
  }
  return m_Gadgets.Lookup(uid); 
}

BOOL   CGraphBuilder::IsDirty() 
{ 
  if (m_bIsDirty) return TRUE;
  POSITION pos = m_Gadgets.GetStartPosition();
  FXString uid;
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos, uid);
    if (Gadget->IsKindOf(RUNTIME_GADGET(Complex)) && ((Complex*)Gadget)->IsDirty())
    {
      m_bIsDirty |= TRUE;
    }
    else
    {
      BOOL bModified;
      if (Gadget->Status().GetBool(STATUS_MODIFIED,bModified) && bModified)
        m_bIsDirty |= TRUE;
    }
  }
  return m_bIsDirty; 
};

BOOL CGraphBuilder::EnumModifiedGadgets(CStringArray& Gadgets)
{
  BOOL bFoundModified = FALSE;
  POSITION pos = m_Gadgets.GetStartPosition();
  FXString uid;
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos, uid);
    if (Gadget->IsKindOf(RUNTIME_GADGET(Complex)) && ((Complex*)Gadget)->IsDirty())
    {
      bFoundModified = TRUE;
      Gadgets.Add(uid);
    }
    else
    {
      bool bModified=false;
      if (Gadget->Status().GetBool(STATUS_MODIFIED,bModified) && bModified)
      {
        bFoundModified = TRUE;
        Gadgets.Add(uid);
      }
    }
  }
  return bFoundModified;
}

CConnector* CGraphBuilder::GetConnector(const FXString& uid)
{
  FXString GadgetUid;
  int number;
  BOOL output;
  BOOL duplex = FALSE;
  if (!DecodeConnectorUID(uid, GadgetUid, number, output))
  {
    if (!DecodeDuplexConnectorUID(uid, GadgetUid, number))
      return NULL;
    duplex = TRUE;
  }
  CGadget* Gadget = GetGadget(GadgetUid);
  if (!Gadget)
    return NULL;
  if (duplex/* && (Gadget->GetDuplexCount() > number)*/)
    return Gadget->GetDuplexConnector(number);
  else if (!duplex && output/* && (Gadget->GetOutputsCount() > number)*/)
    return Gadget->GetOutputConnector(number);
  else if (!duplex && !output/* && (Gadget->GetInputsCount() > number)*/)
    return Gadget->GetInputConnector(number);
  return NULL;
}

void CGraphBuilder::EnumGadgets(CStringArray& srcGadgets, CStringArray& dstGadgets)
{
  srcGadgets.RemoveAll(); // Gadgets having no active inputs. Normally - captures
  dstGadgets.RemoveAll();	// Other gadgets
  POSITION pos = m_Gadgets.GetStartPosition();
  FXString uid;
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos, uid);
    BOOL bGadgetHasConnectedInputs = FALSE;
    int nInputs = Gadget->GetInputsCount();
    FXPtrArray pins;
    while (nInputs--)
    {
      CConnector* Connector = Gadget->GetInputConnector(nInputs);
      if (Connector && Connector->GetComplementary(pins))
      {
        bGadgetHasConnectedInputs = TRUE;
        break;
      }
    }
    if (bGadgetHasConnectedInputs)
      dstGadgets.Add(uid);
    else
      srcGadgets.Add(uid);
  }
}

void CGraphBuilder::EnumGadgetLinks(FXString& uidGadget, CStringArray& dstGadgets)
{
  int i;
  dstGadgets.RemoveAll();
  CGadget* Gadget = GetGadget(uidGadget);
  if (!Gadget)
    return;
  FXPtrArray pins;
  int cOutputs = Gadget->GetOutputsCount();
  for (i = 0; i < cOutputs; i++)
  {
    CConnector* Output = Gadget->GetOutputConnector(i);
    if (Output)
      Output->GetComplementary(pins);
  }
  FXString uid, uidHost;
  while (pins.GetSize())
  {
    if (FindConnector((CConnector*)pins.GetAt(0), uid, &uidHost))
    {
      for (i = 0; i < dstGadgets.GetSize(); i++)
      {
        if (dstGadgets.GetAt(i) == (LPCTSTR)uidHost)
          break;
      }
      if (i == dstGadgets.GetSize())
        dstGadgets.Add(uidHost);
    }
    pins.RemoveAt(0);
  }
}

BOOL CGraphBuilder::IsComplexGadget(FXString& uid)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget) return FALSE;
  return Gadget->IsComplex();
}

BOOL CGraphBuilder::IsLibraryComplexGadget(FXString& uid)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget) return FALSE;
  if (!Gadget->IsComplex()) return FALSE;
  CString LoadPath=((Complex*)Gadget)->GetLoadPath();
  return (LoadPath.GetLength()!=0);
}

void CGraphBuilder::SetLocalComplexGadget(FXString& uid)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget) return;
  if (!Gadget->IsComplex()) return;
  ((Complex*)Gadget)->ClearLoadPath();
}


BOOL CGraphBuilder::GetScriptPath(FXString& uid, FXString& path)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget)
    return NULL;
  if (Gadget->IsKindOf(RUNTIME_GADGET(Complex)))
  {
    LPCTSTR p = ((Complex*)Gadget)->GetLoadPath();
    if (!p)
      return FALSE;
    path = CString(p);
    return TRUE;
  }
  return FALSE;
}

IGraphbuilder* CGraphBuilder::GetSubBuilder(FXString& uid)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget)
    return NULL;
  if (Gadget->IsKindOf(RUNTIME_GADGET(Complex)))
    return ((Complex*)Gadget)->Builder();
  return NULL;
}

void CGraphBuilder::EnumPluginLoaders(CPtrArray& PluginLoaders)
{
  POSITION pos = m_Gadgets.GetStartPosition();
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos);
    if (Gadget->IsKindOf(RUNTIME_GADGET(Complex)))
    {
      ((IGraphbuilder*)((Complex*)Gadget)->Builder())->EnumPluginLoaders(PluginLoaders);
      PluginLoaders.Add((void*)((Complex*)Gadget)->GetPluginLoader());
    }
  }
}

UINT CGraphBuilder::GetGadgetClassAndLineage(FXString& uidGadget, FXString& Class, FXString& Lineage)
{
  CGadget* Gadget = GetGadget(uidGadget);
  if (!Gadget)
    return TVDB400_GT_ANY;	// Failure ( 0 )
  Class = Gadget->GetRuntimeGadget()->m_lpszClassName;
  Lineage = Gadget->GetRuntimeGadget()->m_lpszLineage;
  if (Gadget->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
    return TVDB400_GT_CTRL;
  else if (Gadget->IsKindOf(RUNTIME_GADGET(CCaptureGadget)))
    return TVDB400_GT_CAPTURE;
  else if ( Gadget->IsKindOf( RUNTIME_GADGET( CPortGadget ) ) )
    return TVDB400_GT_PORT;
  else if ( Gadget->IsKindOf( RUNTIME_GADGET( CFilterGadget ) ) )
    return TVDB400_GT_FILTER;
  return TVDB400_GT_OTHER;
}

void CGraphBuilder::EncodeGadgetLinks(FXString& uidGadget, CStringArray& Links, CStringArray* ToGadgets)
{
  Links.RemoveAll();
  CGadget* Gadget = GetGadget(uidGadget);
  CStringArray uidsTo;
  if (ToGadgets)
    uidsTo.Append(*ToGadgets);
  else
  {
    CStringArray srcGadgets;
    CStringArray dstGadgets;
    EnumGadgets(srcGadgets, dstGadgets);
    uidsTo.Append(dstGadgets);
  }
  int cOutputs = Gadget->GetOutputsCount(), i = 0;
  while (i < cOutputs)
  {
    COutputConnector* OutputConnector = Gadget->GetOutputConnector(i);
    FXPtrArray pins;
    pins.RemoveAll();
    if (OutputConnector && OutputConnector->GetComplementary(pins))
    {
      FXString uidOutput = EncodeConnectorUID(uidGadget, i, TRUE);
      while (pins.GetSize())
      {
        FXString uidInput, uidHost;
        if (FindConnector((CConnector*)pins.GetAt(0), uidInput, &uidHost))
        {
          for (int j = 0; j < uidsTo.GetSize(); j++)
          {
            if (uidsTo.GetAt(j) == (LPCTSTR)uidHost)
            {
              CString link = (LPCTSTR) (uidOutput + _T( ',' )) ;
              link += uidInput;
              Links.Add(link);
              break;
            }
          }
        }
        pins.RemoveAt(0);
      }
    }
    i++;
  }
  int cDuplex = Gadget->GetDuplexCount();
  i = 0;
  while (i < cDuplex)
  {
    CDuplexConnector* DuplexConnector = Gadget->GetDuplexConnector(i);
    FXPtrArray pins;
    pins.RemoveAll();
    if (DuplexConnector && DuplexConnector->GetComplementary(pins))
    {
      FXString uidOutput = EncodeDuplexConnectorUID(uidGadget, i);
      while (pins.GetSize())
      {
        CConnector* Connector = (CConnector*)pins.GetAt(0);
        if (Connector->GetInputPin()) // (input OR duplex)
        {
          FXString uidInput, uidHost;
          if (FindConnector(Connector, uidInput, &uidHost))
          {
            if (!Connector->GetOutputPin() || (uidOutput.Compare(uidInput) < 0))
            {
              for (int j = 0; j < uidsTo.GetSize(); j++)
              {
                if (uidsTo.GetAt(j) == (LPCTSTR)uidHost)
                {
                  CString link = (LPCTSTR)((uidOutput + _T(',')) + uidInput);
                  Links.Add(link);
                  break;
                }
              }
            }
          }
        }
        pins.RemoveAt(0);
      }
    }
    i++;
  }
}

#undef THIS_MODULENAME 
#define THIS_MODULENAME "GraphBuilder.TranslateBlock"

void CGraphBuilder::TranslateBlock(CStringArray& UIDs, CStringArray& script)
{
  int i;
  script.RemoveAll();
  CStringArray uids;
  for (i = 0; i < UIDs.GetSize(); i++)
  {
    CString uid = UIDs.GetAt(i);
    CGadget* Gadget = GetGadget(uid);
    if (!Gadget)
    {
      SENDWARN_1("No gadget exists under registered ID \"%s\"", uid);
      continue;
    }
    if (_tcscmp(Gadget->GetRuntimeGadget()->m_lpszLineage, LINEAGE_DEBUG) == 0)
    {   // do not save debug renderers
      continue;
    }
    uids.Add(uid);
  }
  for (i = 0; i < uids.GetSize(); i++)
  {
    CString uid = uids.GetAt(i);
    CGadget* Gadget = GetGadget(uid);
    FXString line, params;
    FXString clName = Gadget->IsKindOf(RUNTIME_GADGET(CVirtualGadget)) ? ((CVirtualGadget*)Gadget)->GetClassName() : Gadget->GetRuntimeGadget()->m_lpszClassName;
    Gadget->PrintProperties(params);
    // Moisey and Yuri, 10-Dec-2018 - Gadget::PrintProperties will be called
    //   inside previous call. If simple gadget is used, the same data obtained in both calls
    //   If aggregate is used, some property of aggregate was removed.
    if ( !Gadget->IsComplex() )
      Gadget->CGadget::PrintProperties( params );
    if (params.IsEmpty())
      line.Format("%s %s", (LPCTSTR)clName, (LPCTSTR)::FxRegularize(uid));
    else
      line.Format("%s %s(%s)", (LPCTSTR)clName, (LPCTSTR)::FxRegularize(uid), (LPCTSTR)params);
    script.Add(line);
  }
  if (script.GetSize())
    script.Add(""); // optional separator between declarations block and connections block
  for (i = 0; i < uids.GetSize(); i++)
  {
    FXString uid = (LPCTSTR)uids.GetAt(i);
    CGadget* Gadget = GetGadget(uid);
    if (!Gadget)
      continue;
    CStringArray links;
    EncodeGadgetLinks(uid, links, &uids);
    while (links.GetSize())
    {
      FXString line;
      line.Format("Connect(%s)", ::FxRegularize(links.GetAt(0)));
      script.Add(line);
      links.RemoveAt(0);
    }
  }
}

BOOL CGraphBuilder::FindConnector(CConnector* Connector, FXString& uid, FXString* uidHost)
{
  POSITION pos = m_Gadgets.GetStartPosition();
  FXString uidGadget;
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos, uidGadget);
    int nInputs = Gadget->GetInputsCount();
    while (nInputs--)
    {
      CConnector* Input = Gadget->GetInputConnector(nInputs);
      if (Input && (Input->GetInputPin() == Connector))
      {
        uid = EncodeConnectorUID(uidGadget, nInputs, FALSE);
        if (uidHost)
          *uidHost = uidGadget;
        return TRUE;
      }
    }
    int nOutputs = Gadget->GetOutputsCount();
    while (nOutputs--)
    {
      CConnector* Output = Gadget->GetOutputConnector(nOutputs);
      if (Output && (Output->GetOutputPin() == Connector))
      {
        uid = EncodeConnectorUID(uidGadget, nOutputs, TRUE);
        if (uidHost)
          *uidHost = uidGadget;
        return TRUE;
      }
    }
    int nDuplex = Gadget->GetDuplexCount();
    while (nDuplex--)
    {
      CConnector* Duplex = Gadget->GetDuplexConnector(nDuplex);
      if (Duplex && ((Duplex->GetOutputPin() == Connector) || (Duplex->GetInputPin() == Connector)))
      {
        uid = EncodeDuplexConnectorUID(uidGadget, nDuplex);
        if (uidHost)
          *uidHost = uidGadget;
        return TRUE;
      }
    }
  }
  return FALSE;
}

void CGraphBuilder::ValidateUID(FXString& uid)
{
  LPCTSTR invalidChars = ".()";
  UINT i = 0;
  for (i = 0; i < strlen(invalidChars); i++)
    uid.Replace(invalidChars[i], '_');
}

double CGraphBuilder::GetGraphTime()
{
  return m_GraphTimer.GetTime();
}

BOOL CALLBACK getGadgetName(void* gadget, void* host, FXString& Name)
{
  return ((CGraphBuilder*)host)->GetGadgetName((CGadget*)gadget, Name);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHPTimer CGraphBuilder::m_GraphTimer = CHPTimer();

CGraphBuilder::CGraphBuilder(CExecutionStatus* Status):
  m_UID(""), 
  m_bIsDirty(FALSE)
{
  m_pStatus = CExecutionStatus::Create(Status);
}

CGraphBuilder::~CGraphBuilder()
{
  ASSERT(_heapchk()==_HEAPOK);
  if (m_pStatus)
    m_pStatus->Release();
  m_pStatus = NULL;
  ASSERT(_heapchk()==_HEAPOK);
}

void CGraphBuilder::Detach()
{
  CStringArray srcGadgets, dstGadgets;
  EnumGadgets(srcGadgets, dstGadgets);
  while (srcGadgets.GetSize())
  {
    CString uid = srcGadgets.GetAt(0);
    srcGadgets.RemoveAt(0);
    CGadget* Gadget = GetGadget(uid);
    ASSERT(Gadget);
    if (Gadget->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
      ((CCtrlGadget*)Gadget)->Detach();
  }
  while (dstGadgets.GetSize())
  {
    CString uid = dstGadgets.GetAt(0);
    dstGadgets.RemoveAt(0);
    CGadget* Gadget = GetGadget(uid);
    ASSERT(Gadget);
    if (Gadget->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
      ((CCtrlGadget*)Gadget)->Detach();
  }
}

FXString CGraphBuilder::GetID()
{
  if (m_UID.GetLength())
    return m_UID;
  return "";
}

#undef THIS_MODULENAME 
#define THIS_MODULENAME "GraphBuilder.SetID"

void    CGraphBuilder::SetID(LPCTSTR newID)
{
  if (newID)
    m_UID=newID;
  else
    m_UID.Empty();
  SENDINFO_2("Builder 0x%p gets id %s", this, m_UID);
}
#undef THIS_MODULENAME 


BOOL CGraphBuilder::GetElementInfo(FXString& uid, FXString& infostring)
{
  CGadget* Gadget = GetGadget(uid);
  if (Gadget)
  {
    if (Gadget->IsKindOf(RUNTIME_GADGET(CVirtualGadget)))
      infostring.Format("%s (?%s)", uid, ((CVirtualGadget*)Gadget)->GetClassName());
    else
      infostring.Format("%s (%s)", uid, Gadget->GetRuntimeGadget()->m_lpszClassName);
    return TRUE;
  }
  CConnector* cn = GetConnector(uid);
  if (cn)
  {
    FXString info;
    if (!cn->GetPinInfo(info))
      return FALSE;
    infostring = uid + (LPCTSTR)info;
    return TRUE;
  }
  if (uid.Find(',') > 0) // wire?
  {
    infostring = uid;
    infostring.Replace(",", " -> ");
    return TRUE;
  }
  return FALSE;
}

BOOL CGraphBuilder::RenameGadget(FXString& uidOld, FXString& uidNew, CStringArray* uidsToChange, CStringArray* uidsNewNames)
{
  FXAutolock lock(m_PropLock);

  ValidateUID(uidNew);

  CGadget* Gadget = GetGadget(uidNew);
  if (Gadget)
    return FALSE;
  Gadget = GetGadget(uidOld);
  if (!Gadget)
    return FALSE;

  uidsToChange->RemoveAll();
  uidsNewNames->RemoveAll();
  uidsToChange->Add(uidOld);
  uidsNewNames->Add(uidNew);

  CStringArray inputs, outputs, duplex, links;
  ListGadgetConnectors(uidOld, inputs, outputs, duplex);
  uidsToChange->Append(inputs);
  inputs.RemoveAll();
  uidsToChange->Append(outputs);
  outputs.RemoveAll();
  uidsToChange->Append(duplex);
  duplex.RemoveAll();
  EncodeGadgetLinks(uidOld, links);
  uidsToChange->Append(links);
  links.RemoveAll();

  m_Gadgets.RemoveKey(uidOld, FALSE);
  m_Gadgets.SetAt(uidNew, Gadget);

  ListGadgetConnectors(uidNew, inputs, outputs, duplex);
  uidsNewNames->Append(inputs);
  uidsNewNames->Append(outputs);
  uidsNewNames->Append(duplex);
  EncodeGadgetLinks(uidNew, links);
  uidsNewNames->Append(links);

  for (int i = 0; i < Gadget->GetInputsCount(); i++)
  {
    FXString uidConnector = EncodeConnectorUID(uidNew, i, FALSE);
    FXString uidConnectorOld = EncodeConnectorUID(uidOld, i, FALSE);
    CStringArray uidConnectors;
    if (IsConnected(uidConnector, uidConnectors))
    {
      while (uidConnectors.GetSize())
      {
        FXString uidWireNew = FXString(uidConnectors.GetAt(0)) + _T(",") + uidConnector;
        FXString uidWireOld = FXString(uidConnectors.GetAt(0)) + _T(",") + uidConnectorOld;
        uidConnectors.RemoveAt(0);
        uidsToChange->Add(uidWireOld);
        uidsNewNames->Add(uidWireNew);
      }
    }
  }

  VERIFY(m_ViewSection.RenameGlyph(uidOld,uidNew));
  return TRUE;
}

BOOL CGraphBuilder::GetPinLabel(FXString& uidPin, FXString& label)
{
  CConnector* Connector = GetConnector(uidPin);
  if (!Connector)
    return FALSE;
  LPCTSTR l = Connector->GetLabel();
  if (!l)
    return FALSE;
  label = CString(l);
  return TRUE;
}

BOOL CGraphBuilder::SetPinLabel(FXString& uidPin, LPCTSTR label)
{
  CConnector* Connector = GetConnector(uidPin);
  if (!Connector)
    return FALSE;
  Connector->SetLabel(label);
  return TRUE;
}

BOOL CGraphBuilder::GetOutputIDs(const FXString& uidPin, FXString& uidOut, CStringArray* uidInComplementary)
{
  CConnector* cn = GetConnector(uidPin);
  if (!cn)
    return FALSE;
  CConnector* pOutPin = cn->GetOutputPin();
  if (pOutPin)
    uidOut = uidPin;
  else
    uidOut.Empty();
  if (uidInComplementary)
  {
    uidInComplementary->RemoveAll();
    CConnector* pInPin = cn->GetInputPin();
    if (pInPin)
    {
      FXPtrArray pins;
      pInPin->GetComplementary(pins);
      while (pins.GetSize())
      {
        CConnector* pin = (CConnector*)pins.GetAt(0);
        pins.RemoveAt(0);
        FXString uid;
        if (FindConnector(pin, uid))
          uidInComplementary->Add(uid);
      }
    }
  }
  return (!uidOut.IsEmpty() || (uidInComplementary && uidInComplementary->GetSize() > 0));
}

BOOL CGraphBuilder::RenderDebugOutput(FXString& uidPin, CWnd* pRenderWnd)
{
  return TRUE;
}

BOOL CGraphBuilder::RenderDebugInput(FXString& uidPin, CWnd* pRenderWnd)
{
  return TRUE;
}

void CGraphBuilder::Start()
{
  m_GraphTimer.Start();
  m_pStatus->Start();
  //POSITION pos = m_Gadgets.GetStartPosition();
  //FXString uid;
  //while ( pos )
  //{
  //  CGadget* pGadget = m_Gadgets.GetNextGadget( pos , uid );
  //  IGraphbuilder * pSubBuilder = GetSubBuilder( uid ) ;
  //  if ( pSubBuilder )
  //  {
  //    pSubBuilder->Start() ;
  //  }
  //  if ( pGadget->IsKindOf( RUNTIME_GADGET( CCaptureGadget ) ) )
  //    ((CCaptureGadget*)pGadget)->OnStart() ;
  //}

  TRACE("CGraphBuilder::Start()\n");
}

void CGraphBuilder::Stop()
{
  m_pStatus->Stop();
  m_GraphTimer.Stop();
  //POSITION pos = m_Gadgets.GetStartPosition();
  //FXString uid;
  //while ( pos )
  //{
  //  CGadget* pGadget = m_Gadgets.GetNextGadget( pos , uid );
  //  IGraphbuilder * pSubBuilder = GetSubBuilder( uid ) ;
  //  if ( pSubBuilder )
  //    pSubBuilder->Stop() ;
  //  if ( pGadget->IsKindOf( RUNTIME_GADGET( CCaptureGadget ) ) )
  //    ((CCaptureGadget*) pGadget)->OnStop() ;
  //}
  TRACE("CGraphBuilder::Stop()\n");
}

void CGraphBuilder::Pause()
{
  m_pStatus->Pause();
  m_GraphTimer.Pause();
  //POSITION pos = m_Gadgets.GetStartPosition();
  //FXString uid;
  //while ( pos )
  //{
  //  CGadget* Gadget = m_Gadgets.GetNextGadget( pos , uid );
  //  IGraphbuilder * pSubBuilder = GetSubBuilder( uid ) ;
  //  if ( pSubBuilder )
  //    pSubBuilder->Pause() ;
  //}
  TRACE( "CGraphBuilder::Pause()\n" );
}

void CGraphBuilder::StepFwd()
{
  if (m_pStatus->GetStatus() != CExecutionStatus::PAUSE)
    m_pStatus->Pause();
  m_pStatus->StepFwd();
  TRACE("CGraphBuilder::StepFwd()\n");
}

void CGraphBuilder::SetExecutionStatus(CExecutionStatus* Status)
{
  if (m_pStatus)
  {
    //Stop();
    if (m_pStatus) 
      m_pStatus->Release();
    m_pStatus = NULL;
  }
  m_pStatus = CExecutionStatus::Create(Status);
  POSITION pos = m_Gadgets.GetStartPosition();
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos);
    if (Gadget)
      Gadget->InitExecutionStatus(m_pStatus);
  }
}

BOOL CGraphBuilder::IsRuning()
{
  return (m_pStatus->GetStatus() == CExecutionStatus::RUN);
}

BOOL CGraphBuilder::IsPaused()
{
  return (m_pStatus->GetStatus() == CExecutionStatus::PAUSE);
}

void CGraphBuilder::ShutDown()
{
  if (IsRuning()) 
    Stop();
  if ( m_pGraphSetupObject )
    m_pGraphSetupObject->Delete();
  m_pGraphSetupObject = NULL;

  m_Gadgets.RemoveAll();
}

BOOL CGraphBuilder::GetScript(FXString& script, CStringArray* BlockUIDs)
{
  CStringArray uids, dstUids;
  if (!BlockUIDs) // save entire graph
  {
    EnumGadgets(uids, dstUids);
    uids.Append(dstUids);
  }
  else
    uids.Append(*BlockUIDs);
  CStringArray lines;
  TranslateBlock(uids, lines);

  script.Empty();
  while (lines.GetSize())
  {
    script += lines.GetAt(0);
    lines.RemoveAt(0);
    script += _T("\n");
  }
  // save view section
  
  FXString ext=m_ViewSection.GetViewSection();
  if (ext.GetLength())
  {
    script+=EOL+ext+EOL;
  }
  return TRUE;
}

BOOL CGraphBuilder::Save(LPCTSTR fileName, CStringArray* BlockUIDs)
{
  FXString script;
  GetScript(script,BlockUIDs);
  CStdioFile file;
  if (!file.Open(fileName, CFile::modeCreate | CFile::modeWrite))
    return FALSE;
  file.WriteString((LPCTSTR)script);
  file.Close();

  // Reset flags modified
  CStringArray uids, dstUids;
  if (!BlockUIDs) // save entire graph
  {
    EnumGadgets(uids, dstUids);
    uids.Append(dstUids);
  }
  else
    uids.Append(*BlockUIDs);
  SetDirty(FALSE);
  while (uids.GetSize())
  {
    CGadget* Gadget = GetGadget(uids.GetAt(0));
    if (Gadget)
      Gadget->Status().WriteBool(STATUS_MODIFIED,false);
    uids.RemoveAt(0);
  }
  return TRUE;
}

#undef THIS_MODULENAME 
#define THIS_MODULENAME "GraphBuilder.Load"

int CGraphBuilder::Load(LPCTSTR fileName, LPCTSTR script, bool bClearOld)
{
  if ( !is_evalkey_exists() )
  {
    SENDERR( "\nGraph load FAILED\n reason: %s" , "SOFTWARE PACKAGE IS NOT REGISTERED." );
    return MSG_NOT_REGISTERED;
  }
  int iDaysRemain = -5000 ;

  bool bLocalMachine = true ;
  char cIsRestricted = isevaluation( bLocalMachine ) ;
  if ( cIsRestricted == EOL_Evaluation )
  {
    iDaysRemain = daysremain() ;
    if ( iDaysRemain <= 0 )
    {
      SENDERR( "\nFailed to load file %s\n reason: %s" , fileName , "Evaluation Period Expired" );
      return MSG_SYSTEM_LEVEL;
    }
    SENDINFO( "Evaluation will be expired in %d days" , iDaysRemain );
  }
  else if ( cIsRestricted == EOL_License )
  {
    iDaysRemain = daysremain() ;
    if ( iDaysRemain <= 0 )
    {
      SENDERR( "\nFailed to load file %s\n reason: %s" , fileName , "License Expired" );
      return MSG_SYSTEM_LEVEL;
    }
    SENDINFO( "License will be expired in %d days" , iDaysRemain );
  }

  int result = MSG_INFO_LEVEL;

  SetCurrentDirectory(FxGetAppPath());

  if ((fileName) && (_tcslen(fileName)!=0))
  {
    SetID(FxGetFileTitle(fileName));
  }

  ASSERT(fileName || script);
  if (bClearOld)
    m_Gadgets.RemoveAll();
  FXParser fxParser;
  if (fileName && (strlen(fileName)!=0) && !fxParser.Load(fileName))
  {
    SENDERR("Failed to load file %s, reason: %s", fileName,fxParser.GetErrorMessage());
    return MSG_ERROR_LEVEL;
  }
  else if ((!fileName || (strlen(fileName)==0))  && script)
  {
    fxParser = script;
  }

  FXSIZE iptr=0, prevptr=0;
  FXString word;
  while (fxParser.GetWord(iptr, word))
  {
    if (word.CompareNoCase("CONNECT")==0) // Connect cmd
    {
      if (!fxParser.GetParamString(iptr,word))
      {
        SENDWARN("Script parser: Failed to parse Connect parameters");
        if (result < MSG_WARNING_LEVEL)
          result = MSG_WARNING_LEVEL;
        prevptr=iptr;
        continue;
      }
      FXSIZE curpos=0;
      FXString uidOutputConnector=word.Tokenize(",",curpos),uidInputConnector=word.Tokenize(",",curpos);
      if ((uidOutputConnector.GetLength()==0) || (uidInputConnector.GetLength()==0))
      {
        SENDWARN("Failed to connect %s to %s", uidOutputConnector, uidInputConnector);
        if (result < MSG_WARNING_LEVEL)
          result = MSG_WARNING_LEVEL;
        prevptr=iptr;
        continue;
      }
      if (!Connect(uidOutputConnector, uidInputConnector))
      {
        SENDWARN_2("Failed to connect %s to %s", uidOutputConnector, uidInputConnector);
        if (result < MSG_WARNING_LEVEL)
          result = MSG_WARNING_LEVEL;
        prevptr=iptr;
        continue;
      }
    }
    else if (word.CompareNoCase(VIEW_SECTION_BEGIN)==0)
    {
      FXAutolock lock(m_PropLock);
      FXString viewsection=fxParser.Mid(prevptr);
      FXSIZE posbegin, posend;
      posbegin=viewsection.Find(VIEW_SECTION_BEGIN);
      posend=viewsection.Find(VIEW_SECTION_END);
      if ((posbegin!=-1) && (posend!=-1))
      {
        viewsection=viewsection.Mid(posbegin,posend-posbegin+_tcslen(VIEW_SECTION_END));
       // int iLen = viewsection.GetLength() ;
       // TRACE("===\n%s\n===\nLength=%d\n",(LPCTSTR)viewsection,viewsection.GetLength());
        m_ViewSection.RemoveAll();
        if (!m_ViewSection.Parse(viewsection))
        {
          SENDWARN_0("View section in script file is corrupted");
        }
        iptr=prevptr+posend+_tcslen(VIEW_SECTION_END);
      }
      else
      {
        SENDWARN_0("View section in script file is corrupted");
      }
      fxParser.TrimSeparators(iptr);
      //TRACE("===\n%s\n===\n",&(fxParser.GetBuffer()[iptr]));
    }
    else // try to treat as declaration
    {
      CString clName(word);
      if (!fxParser.GetWord(iptr, word)) // uid
      {
        SENDWARN_2("Failed to parse declaration of %s near %s", clName, word);
        if (result < MSG_WARNING_LEVEL)
          result = MSG_WARNING_LEVEL;
        prevptr=iptr;
        continue;
      }
      FXString params;
      word=::FxUnregularize(word);
      BOOL hasParams = fxParser.GetParamString(iptr,params);
      if (!hasParams)
        params.Empty();			
      CString msg("");
      if (word.IsEmpty())
        msg.Format("No UID in declaration of %s", clName);
      else if (!m_GadgetFactory.CreateGadget((LPCTSTR)clName))
      {
        msg.Format("Failed to create %s %s", clName, word);
        if (m_GadgetFactory.CreateGadget(RUNTIME_GADGET(CVirtualGadget)->m_lpszClassName) ||
          m_GadgetFactory.RegisterGadgetClass(RUNTIME_GADGET(CVirtualGadget)) &&
          m_GadgetFactory.CreateGadget(RUNTIME_GADGET(CVirtualGadget)->m_lpszClassName))
        {
          if ((RegisterCurGadget(word, params)) && ((CVirtualGadget*)GetGadget(word)))
          {
            ( ( CVirtualGadget* ) GetGadget( word ) )->SetClassName( clName );
          }
        }
      }
      else if (!RegisterCurGadget(word, params))
        msg.Format("Failed to add %s %s to graph", clName, word);
      else
      {
        CGadget* Gadget = GetGadget(word);
        ASSERT(Gadget);
        bool bInvalidate = FALSE;
        Gadget->CGadget::ScanProperties(params, bInvalidate);
        //Gadget->SetModifiedUIDsPtr( &m_ModifiedUIDs ) ;
      }
      if (!msg.IsEmpty())
      {
        SENDWARN_0(msg);
        if (result < MSG_WARNING_LEVEL)
          result = MSG_WARNING_LEVEL;
      }
    }
    prevptr=iptr;
  }
  if (!fxParser.IsEOL(iptr))
  {
    SENDWARN_1("Failed to parse after %s", word);
    if (result < MSG_WARNING_LEVEL)
      result = MSG_WARNING_LEVEL;
  }
  if (result == MSG_INFO_LEVEL)
  {
    SENDINFO_1("Graph \"%s\" successfully load", fileName);
  }
  SetDirty(FALSE);
  return result;
}

#undef THIS_MODULENAME 

void CGraphBuilder::PrintMsg(int msgLevel, LPCTSTR src, int msgId, LPCTSTR msgText)
{
  if (m_pMsgQueue)
    m_pMsgQueue->AddMsg(msgLevel, src, msgId, msgText);
  else
  {
    IGraphMsgQueue* MsgQueue = FxGetGraphMsgQueue();
    if (MsgQueue)
      MsgQueue->AddMsg(msgLevel, src, msgId, msgText);
    TRACE("LOG>> Level=%d, ID=%d, text=\"%s\"\n", msgLevel, msgId, msgText);
  }
}

void CGraphBuilder::UnregisterGadgetClass(CRuntimeGadget* RuntimeGadget)
{
  m_GadgetFactory.UnregisterGadgetClass(RuntimeGadget);
  POSITION pos = m_Gadgets.GetStartPosition();
  while (pos)
  {
    FXString uid;
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos, uid);
    if (Gadget->GetRuntimeGadget() == RuntimeGadget)
      m_Gadgets.RemoveKey(uid);
  }
}

UINT CGraphBuilder::RegisterCurGadget(FXString& uid, LPCTSTR params)
{
  ValidateUID(uid);

  bool Invalidate=false;
  CGadget* Gadget = m_GadgetFactory.GetCurGadget();
  if (!Gadget)
    return TVDB400_GT_ANY;

  if (params && !Gadget->ScanProperties(params,Invalidate))
  {
    Gadget->ShutDown();
    delete Gadget;
    return TVDB400_GT_ANY;
  }
  else if (!params)
    Gadget->ScanProperties("",Invalidate);
  if (!uid.IsEmpty())
  {
    if (GetGadget(uid))
    {
      Gadget->ShutDown();
      delete Gadget;
      return TVDB400_GT_ANY;
    }
  }
  else
  {
    CString name = Gadget->GetRuntimeGadget()->m_lpszClassName;
    GadgetClassNameToSampleName(name);
    CAutoName AutoName(name);
    do
    {
      AutoName.GetNextName(uid);
    } while (GetGadget(uid));
  }

  Gadget->SetGraphTimer(GetGraphTime);
  m_Gadgets.SetAt(uid, Gadget);
  Gadget->SetNamingFunction( ( TVDB400_FNGETGADGETNAME ) getGadgetName , this );
  ((CGadget*)Gadget)->SetThreadName(uid);
  ((CGadget*)Gadget)->InitExecutionStatus(m_pStatus);
  SetDirty();
  if (Gadget->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
    return TVDB400_GT_CTRL;
  else if (Gadget->IsKindOf(RUNTIME_GADGET(CCaptureGadget)))
    return TVDB400_GT_CAPTURE;
  else if ( Gadget->IsKindOf( RUNTIME_GADGET( CPortGadget ) ) )
  {
    ( ( CGadget* ) Gadget )->SetModifiedUIDsPtr( &m_ModifiedUIDs ) ;
    return TVDB400_GT_PORT;
  }
  else if (Gadget->IsKindOf(RUNTIME_GADGET(CFilterGadget)))
    return TVDB400_GT_FILTER;
  else if (Gadget->IsKindOf(RUNTIME_GADGET(Complex)))
  {
    ((Complex*)Gadget)->SetGraphMsgQueue(m_pMsgQueue);
    ((Complex*)Gadget)->SetBuilderName(uid);
    return TVDB400_GT_COMPLEX;
  }
  else if (Gadget->IsKindOf(RUNTIME_GADGET(CGadget)))
    return TVDB400_GT_GADGET;
  return TVDB400_GT_OTHER;
}

UINT CGraphBuilder::RegisterGadget(FXString& uid, void* Gadget)
{
  ValidateUID(uid);

  ((CGadget*)Gadget)->SetGraphTimer(GetGraphTime);
  ((CGadget*)Gadget)->SetNamingFunction((TVDB400_FNGETGADGETNAME)getGadgetName, this);
  CString name = ((CGadget*)Gadget)->GetRuntimeGadget()->m_lpszClassName;
  GadgetClassNameToSampleName(name);
  CAutoName AutoName(name);
  while (uid.IsEmpty() || GetGadget(uid))
    AutoName.GetNextName(uid);

  m_Gadgets.SetAt(uid, (CGadget*)Gadget);
  ((CGadget*)Gadget)->SetThreadName(uid);
  ((CGadget*)Gadget)->InitExecutionStatus(m_pStatus);
  SetDirty();
  if (((CGadget*)Gadget)->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
    return TVDB400_GT_CTRL;
  else if (((CGadget*)Gadget)->IsKindOf(RUNTIME_GADGET(CCaptureGadget)))
    return TVDB400_GT_CAPTURE;
  else if ( ( ( CGadget* ) Gadget )->IsKindOf( RUNTIME_GADGET( CPortGadget ) ) )
    return TVDB400_GT_PORT;
  else if (((CGadget*)Gadget)->IsKindOf(RUNTIME_GADGET(CFilterGadget)))
    return TVDB400_GT_FILTER;
  else if (((CGadget*)Gadget)->IsKindOf(RUNTIME_GADGET(Complex)))
  {
    ((Complex*)Gadget)->SetGraphMsgQueue(m_pMsgQueue);
    return TVDB400_GT_COMPLEX;
  }
  return TVDB400_GT_OTHER;
}

void CGraphBuilder::GrantGadgets(IGraphbuilder* pBuilder, CStringArray* uids, CMapStringToString* renames)
{
  CStringArray GadgetUIDs;
  if (uids)
    GadgetUIDs.Append(*uids);
  else
  {
    CStringArray srcGadgets, dstGadgets;
    EnumGadgets(srcGadgets, dstGadgets);
    GadgetUIDs.Append(srcGadgets);
    GadgetUIDs.Append(dstGadgets);
  }

  FXAutolock lock(m_PropLock);
  FXPropertyKit pk,viewsection;
  m_ViewSection.GetViewPropertyString(viewsection);
  while (GadgetUIDs.GetSize())
  {
    FXString uidPK;
    FXString uid = (LPCTSTR) GadgetUIDs.GetAt(0);
    GadgetUIDs.RemoveAt(0);
    CGadget* Gadget = GetGadget(uid);
    m_Gadgets.RemoveKey(uid, FALSE);

    if (!Gadget)
      continue;	// other glyph (wire)
    m_ViewSection.RemoveGlyph(uid);
    viewsection.GetString(uid, uidPK);
    CString old = (LPCTSTR) uid;
    VERIFY(pBuilder->RegisterGadget(uid, Gadget));
    if (renames)
      renames->SetAt(uid, old);
    if (!uidPK.IsEmpty())
      pk.WriteString(uid, uidPK);
  }
  pBuilder->SetProperties(pk);
}

BOOL CGraphBuilder::GetGadgetName(CGadget* gadget, FXString& Name)
{
  FXString uid;
  if (!m_Gadgets.GetGadgetUID(gadget, uid))
    return FALSE;
  if (GetID().GetLength())
    Name.Format("%s.%s", GetID(), uid);
  else
    Name=uid;
  return TRUE;
}

BOOL CGraphBuilder::GetProperties(FXString& props)
{
  FXAutolock lock(m_PropLock);
  return m_ViewSection.GetViewPropertyString(props);
}


void CGraphBuilder::SetProperties(LPCTSTR props)
{
  FXAutolock lock(m_PropLock);
  FXPropertyKit pk( props );
  FXStringArray uids, Props;
  pk.EnumKeys(uids, Props);
  while (uids.GetSize())
  {
    FXPropertyKit uidText;
    if (pk.GetString(uids[0], uidText))
    {
      int x, y;
      if (
        (uidText.GetInt("x", x)) &&
        (uidText.GetInt("y", y))
        )
      {
        m_ViewSection.SetGlyph(uids[0],x,y);
      }
    }
    uids.RemoveAt(0);
    Props.RemoveAt(0);
  }
}


void CGraphBuilder::UnregisterGadget(FXString& uid)
{
  CGadget* Gadget = GetGadget(uid);
  if (Gadget)
  {
    m_Gadgets.RemoveKey(uid);
    SetDirty();
  }
}

UINT CGraphBuilder::ListGadgetConnectors(FXString& uidGadget, 
  CStringArray& inputs, CStringArray& outputs, CStringArray& duplex)
{
  CGadget* Gadget = GetGadget(uidGadget);
  if (!Gadget)
    return TVDB400_GT_ANY;
  inputs.RemoveAll();
  outputs.RemoveAll();
  duplex.RemoveAll();
  {
    int i;
    for (i = 0; i < Gadget->GetInputsCount(); i++)
      inputs.Add(EncodeConnectorUID(uidGadget, i, FALSE));
    for (i = 0; i < Gadget->GetOutputsCount(); i++)
      outputs.Add(EncodeConnectorUID(uidGadget, i, TRUE));
    for (i = 0; i < Gadget->GetDuplexCount(); i++)
      duplex.Add(EncodeDuplexConnectorUID(uidGadget, i));
  }
  return KindOf(uidGadget);
}

#define THIS_MODULENAME "CGraphBuilder::AggregateBlock"
BOOL CGraphBuilder::AggregateBlock(FXString& uid, CStringArray& Gadgets, LPCTSTR loadPath)
{
  // 0. Prepare to edit
  BOOL bWasRuning = IsRuning();
  Stop();
  // 1. Create subgraph builder
  IGraphbuilder* pBuilder = Tvdb400_CreateBuilder(m_pStatus); // builder for subgraph
  ASSERT(pBuilder);
  //	pBuilder->SetPrintLogFunc(m_LogPrint);
  // 2. Grant gadgets
  //    pBuilder->m_pStatus->Copy(m_pStatus);
  SENDINFO_2("It's about to aggregate block of %d gadgets in complex gadget '%s'",Gadgets.GetSize(),uid);
  GrantGadgets((IGraphbuilder*)pBuilder, &Gadgets);
  // 3. Create complex gadget
  Complex* Gadget = (Complex*)Complex::CreateGadget();
  Gadget->SetBuilder(pBuilder);
  RegisterGadget(uid, Gadget);
  Gadget->SetLoadPath(((loadPath) ? loadPath : ""));
  Gadget->SetBuilderName(uid);
  // 4. Resume
  if (bWasRuning)
  {
    pBuilder->Start();
    Start();
  }
  return TRUE;
}
#ifdef THIS_MODULENAME
#undef THIS_MODULENAME
#endif

BOOL CGraphBuilder::ExtractBlock(FXString& uid, CMapStringToString* renames)
{
  // 0. Prepare to edit
  CGadget* Gadget = (CGadget*)GetGadget(uid);
  if (!Gadget || !Gadget->IsKindOf(RUNTIME_GADGET(Complex)))
    return FALSE;
  BOOL bWasRuning = IsRuning();
  Stop();
  // 1. Get Builder interface
  IGraphbuilder* Builder = ((Complex*)Gadget)->Builder();
  ASSERT(Builder);
  // 2. Grant gadgets
  Builder->Stop();
  ((IGraphbuilder*)Builder)->GrantGadgets(this, NULL, renames);
  // 3. Destroy complex gadget without disconnecting wires! Gadget ShutDown() should not be called
  m_Gadgets.RemoveKey(uid, FALSE);
  m_ViewSection.RemoveGlyph(uid);
  ((Complex*)Gadget)->Collapse();
  delete Gadget;
  // 4. Resume
  SetDirty();
  if (bWasRuning)
    Start();
  return TRUE;
}

int CGraphBuilder::GetGraphInputsCount()
{
  int cInputs = 0;
  POSITION pos = m_Gadgets.GetStartPosition();
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos);
    ASSERT(Gadget);
    for (int i = 0; i < Gadget->GetInputsCount(); i++)
    {
      FXPtrArray pins;
      pins.RemoveAll();
      if (!Gadget->GetInputConnector(i)->GetComplementary(pins))
        cInputs++; // free connector
      FXString uid;
      while (pins.GetSize())
      {
        if (pins.GetAt(0) && !FindConnector((CConnector*)pins.GetAt(0), uid))
        {
          cInputs++; // linked connector
          break;
        }
        pins.RemoveAt(0);
      }
    }
  }
  return cInputs;
}

CInputConnector* CGraphBuilder::GetGraphInput(int n)
{
  FXString uid, uidLast;
  CGadget* Gadget = m_Gadgets.GetNextGadgetOrdered(uid, NULL);
  FXPtrArray pins;
  while (Gadget)
  {
    for (int i = 0; i < Gadget->GetInputsCount(); i++)
    {
      pins.RemoveAll();
      CInputConnector* InputConnector = Gadget->GetInputConnector(i);
      if (!InputConnector->GetComplementary(pins) && !n--)
        return InputConnector; // free connector
      FXString uid;
      while (pins.GetSize())
      {
        if (pins.GetAt(0) && !FindConnector((CConnector*)pins.GetAt(0), uid) && !n--)
          return InputConnector; // linked connector
        pins.RemoveAt(0);
      }
    }
    uidLast = uid;
    Gadget = m_Gadgets.GetNextGadgetOrdered(uid, uidLast);
  }
  return NULL;
}

int CGraphBuilder::GetGraphOutputsCount()
{
  int cOutputs = 0;
  POSITION pos = m_Gadgets.GetStartPosition();
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos);
    ASSERT(Gadget);
    for (int i = 0; i < Gadget->GetOutputsCount(); i++)
    {
      FXPtrArray pins;
      pins.RemoveAll();
      if (!Gadget->GetOutputConnector(i)->GetComplementary(pins))
        cOutputs++; // free connector
      FXString uid;
      while (pins.GetSize())
      {
        if (pins.GetAt(0) && !FindConnector((CConnector*)pins.GetAt(0), uid))
        {
          cOutputs++; // linked connector
          break;
        }
        pins.RemoveAt(0);
      }
    }
  }
  return cOutputs;
}

COutputConnector* CGraphBuilder::GetGraphOutput(int n)
{
  FXString uid, uidLast;
  CGadget* Gadget = m_Gadgets.GetNextGadgetOrdered(uid, NULL);
  FXPtrArray pins;
  while (Gadget)
  {
    for (int i = 0; i < Gadget->GetOutputsCount(); i++)
    {
      pins.RemoveAll();
      COutputConnector* OutputConnector = Gadget->GetOutputConnector(i);
      if (!OutputConnector->GetComplementary(pins) && !n--)
        return OutputConnector; // free connector
      FXString uid;
      while (pins.GetSize())
      {
        if (pins.GetAt(0) && !FindConnector((CConnector*)pins.GetAt(0), uid))
        {
          if (!n--)
            return OutputConnector;	// linked connector
          else
            break; // don't count the same output connector twice
        }
        pins.RemoveAt(0);
      }
    }
    uidLast = uid;
    Gadget = m_Gadgets.GetNextGadgetOrdered(uid, uidLast);
  }
  return NULL;
}

int CGraphBuilder::GetGraphDuplexPinsCount()
{
  int cDuplex = 0;
  POSITION pos = m_Gadgets.GetStartPosition();
  while (pos)
  {
    CGadget* Gadget = m_Gadgets.GetNextGadget(pos);
    ASSERT(Gadget);
    for (int i = 0; i < Gadget->GetDuplexCount(); i++)
    {
      FXPtrArray pins;
      pins.RemoveAll();
      if (!Gadget->GetDuplexConnector(i)->GetComplementary(pins))
        cDuplex++; // free connector
      FXString uid;
      while (pins.GetSize())
      {
        if (pins.GetAt(0) && !FindConnector((CConnector*)pins.GetAt(0), uid))
        {
          cDuplex++; // linked connector
          break;
        }
        pins.RemoveAt(0);
      }
    }
  }
  return cDuplex;
}

CDuplexConnector* CGraphBuilder::GetGraphDuplexPin(int n)
{
  FXString uid, uidLast;
  CGadget* Gadget = m_Gadgets.GetNextGadgetOrdered(uid, NULL);
  FXPtrArray pins;
  while (Gadget)
  {
    for (int i = 0; i < Gadget->GetDuplexCount(); i++)
    {
      pins.RemoveAll();
      CDuplexConnector* DuplexConnector = Gadget->GetDuplexConnector(i);
      if (!DuplexConnector->GetComplementary(pins) && !n--)
        return DuplexConnector; // free connector
      FXString uid;
      while (pins.GetSize())
      {
        if (pins.GetAt(0) && !FindConnector((CConnector*)pins.GetAt(0), uid))
        {
          if (!n--)
            return DuplexConnector;	// linked connector
          else
            break; // don't count the same output connector twice
        }
        pins.RemoveAt(0);
      }
    }
    uidLast = uid;
    Gadget = m_Gadgets.GetNextGadgetOrdered(uid, uidLast);
  }
  return NULL;
}

BOOL CGraphBuilder::ConnectRendererAndMonitor(LPCTSTR uid, CWnd* pParentWnd, LPCTSTR Monitor, CRenderGadget* &RenderGadget)
{
  RenderGadget=NULL;
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget || !Gadget->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
    return FALSE;
  RenderGadget=(CRenderGadget*)Gadget;

  if (!pParentWnd)
  {
    ((CRenderGadget*)Gadget)->Detach();
  }
  else
  {
    // Attach view and render gadget 090609
    ((CRenderGadget*)Gadget)->Create();
    ((CRenderGadget*)Gadget)->Attach(pParentWnd);
    ((CRenderGadget*)Gadget)->SetMonitor(Monitor);
  }
  return (((CRenderGadget*)Gadget)->GetRenderWnd()!=NULL);
}

BOOL CGraphBuilder::SetRendererCallback(FXString& uid, RenderCallBack rcb, void* cbData)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget || !Gadget->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
    return FALSE;
  return ((CRenderGadget*)Gadget)->SetCallBack(rcb, cbData);
}

BOOL CGraphBuilder::GetRenderMonitor(FXString& uid, FXString& monitor)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget || !Gadget->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
    return NULL;
  LPCTSTR mon = ((CRenderGadget*)Gadget)->GetMonitor();
  if (!mon)
    return FALSE;
  monitor = CString(mon);
  return TRUE;
}

BOOL CGraphBuilder::SetOutputCallback(LPCTSTR idPin, OutputCallback ocb, void* pClient)
{
  FXString id(idPin);
  CConnector* Connector = GetConnector(id);
  if (!Connector || !Connector->GetOutputPin())
    return FALSE;
  ((COutputConnector*)Connector->GetOutputPin())->SetCallback(idPin, ocb, pClient);
  return TRUE;
}

BOOL CGraphBuilder::SendDataFrame(CDataFrame* pDataFrame, LPCTSTR idPin)
{
  FXAutolock al(m_InputWrappersLock);
  CInputPinWrrapper* ipw=NULL;
  if (!m_InputWrappers.Lookup(idPin,(void*&)ipw))
  {
    FXString gadgetuid=idPin;
    CConnector* Connector = GetConnector(gadgetuid);
    if (!Connector) 
      return FALSE;
    gadgetuid.Replace(">", "&gt;");
    gadgetuid.Replace("<", "&lt;");
    gadgetuid += _T("_IPWrapper");

    ipw = (CInputPinWrrapper*)GetGadget(gadgetuid);
    if (!ipw )
    {
      if (!CreateGadget(RUNTIME_GADGET(CInputPinWrrapper)->m_lpszClassName))
      {
        if (!RegisterGadgetClass(RUNTIME_GADGET(CInputPinWrrapper)) ||
          !CreateGadget(RUNTIME_GADGET(CInputPinWrrapper)->m_lpszClassName))
          return FALSE;
      }
      if (RegisterCurGadget(gadgetuid) != IGraphbuilder::TVDB400_GT_GADGET)
        return FALSE;
      ipw  = (CInputPinWrrapper*)GetGadget(gadgetuid);
      if (!ipw)
        return FALSE;
    }
    CStringArray inputs, outputs, duplex;
    if (!ListGadgetConnectors(gadgetuid, inputs, outputs, duplex) || !outputs.GetSize())
      return FALSE;
    FXString output=(LPCTSTR)outputs.GetAt(0);
    if (!Connect(idPin, output))
      return FALSE;
    m_InputWrappers.SetAt(idPin,ipw);
  }
  return ipw->Send(pDataFrame);
}

#define THIS_MODULENAME "GraphBuilder.Connect"
BOOL CGraphBuilder::Connect(const FXString& uid1, const FXString& uid2)
{
  //TRACE("CGraphBuilder::Connect(%s, %s)\n",uid1, uid2);
  if (uid1==uid2) return FALSE;
  CConnector* Connector1 = GetConnector(uid1);
  if (!Connector1)
    return FALSE;
  CConnector* Connector2 = GetConnector(uid2);
  if (!Connector2)
    return FALSE;
  CStringArray uidTo;
  if (IsConnected(uid1, uidTo))
  {
    while (uidTo.GetSize())
    {
      if (uid2 == (LPCTSTR)uidTo.GetAt(0))
      {
        SENDWARN_2("Pins '%s' and '%s' are already connected", uid1,uid2);
        return FALSE;
      }
      uidTo.RemoveAt(0);
    }
  }
  if (IsConnected(uid2, uidTo))
  {
    while (uidTo.GetSize())
    {
      if (uid1 == (LPCTSTR)uidTo.GetAt(0))
      {
        SENDWARN_2("Pins '%s' and '%s' are already connected", uid1,uid2);
        return FALSE;
      }
      uidTo.RemoveAt(0);
    }
  }
  if (!Connector1->Connect(Connector2))
  {
    SENDWARN_2("Unexpected fault: can't connect two pins '%s' and '%s'", uid1,uid2);
    return FALSE;
  }
  SetDirty();
  return TRUE;
}
#undef THIS_MODULENAME

#define THIS_MODULENAME "GraphBuilder.Disconnect"
BOOL CGraphBuilder::Disconnect(LPCTSTR uidConnector)
{
  CConnector* Connector = GetConnector(FXString(uidConnector));
  if (!Connector)
    SENDWARN_1("Error at disconnection: pin at '%s' gadget is not connected", uidConnector);
  else if (!Connector->Disconnect())
    SENDWARN_1("Error at disconnection: pin at '%s' gadget is not connected", uidConnector);
  return TRUE;
}
#undef THIS_MODULENAME

BOOL CGraphBuilder::Disconnect(LPCTSTR pin1, LPCTSTR pin2)
{
  FXString uid1(pin1), uid2(pin2);
  CConnector* Connector1 = GetConnector(uid1);
  CConnector* Connector2 = GetConnector(uid2);
  if (!Connector1 || !Connector2)
    return FALSE;
  if (!Connector1->Disconnect(Connector2))
    return FALSE;
  SetDirty();
  return TRUE;
}

BOOL CGraphBuilder::IsConnected(LPCTSTR uidConnector, CStringArray& uidTo)
{
  CConnector* Connector = GetConnector(uidConnector);
  if (!Connector)
    return FALSE;
  FXPtrArray pins;
  if (!Connector->GetComplementary(pins))
    return FALSE;
  while (pins.GetSize())
  {
    FXString uid;
    if (FindConnector((CConnector*)pins.GetAt(0), uid))
      uidTo.Add(uid);
    pins.RemoveAt(0);
  }
  return (uidTo.GetSize() > 0);
}

BOOL CGraphBuilder::IsValid(LPCTSTR UID)
{
  CGadget* Gadget = GetGadget(UID);
  if (!Gadget)
    return FALSE ;
  if (!Gadget->IsKindOf(RUNTIME_GADGET(CFilterGadget))) 
    return TRUE;
  return !((CFilterGadget*)Gadget)->IsInvalid();
}


UINT CGraphBuilder::KindOf(FXString& uid)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget)
    return TVDB400_GT_ANY;
  if (Gadget->IsKindOf(RUNTIME_GADGET(CCtrlGadget)))
    return TVDB400_GT_CTRL;
  else if (Gadget->IsKindOf(RUNTIME_GADGET(CCaptureGadget)))
    return TVDB400_GT_CAPTURE;
  else if ( Gadget->IsKindOf( RUNTIME_GADGET( CPortGadget ) ) )
    return TVDB400_GT_PORT;
  else if ( Gadget->IsKindOf( RUNTIME_GADGET( CFilterGadget ) ) )
    return TVDB400_GT_FILTER;
  else if ( Gadget->IsKindOf( RUNTIME_GADGET( CRenderGadget ) ) )
    return TVDB400_GT_RENDER;
  else if ( Gadget->IsKindOf( RUNTIME_GADGET( Complex ) ) )
    return TVDB400_GT_COMPLEX;
  return TVDB400_GT_OTHER;
}

BOOL CGraphBuilder::IsGadgetSetupOn(FXString& uid)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget)
    return FALSE;
  //	return Gadget->IsSetupOn();
  return FALSE; // TODO: ???
}

#define THIS_MODULENAME "GraphBuilder"

BOOL CGraphBuilder::GetGadgetMode(FXString& uid, int& mode)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget || !Gadget->IsKindOf(RUNTIME_GADGET(CFilterGadget)))
    return FALSE;
  Gadget->GetMode(mode);
  return TRUE;
}

BOOL CGraphBuilder::SetGadgetMode(FXString& uid, int mode)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget || !Gadget->IsKindOf(RUNTIME_GADGET(CFilterGadget)))
    return FALSE;
  Gadget->SetMode(mode);
  return TRUE;
}

BOOL CGraphBuilder::GetOutputMode(FXString& uid, CFilterGadget::OutputMode& mode)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget || !Gadget->IsKindOf(RUNTIME_GADGET(CFilterGadget)))
    return FALSE;
  mode= ((CFilterGadget*)Gadget)->GetOutputMode();
  return TRUE;
}

BOOL CGraphBuilder::SetOutputMode(FXString& uid, CFilterGadget::OutputMode mode)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget || !Gadget->IsKindOf(RUNTIME_GADGET(CFilterGadget)))
    return FALSE;
  ((CFilterGadget*)Gadget)->SetOutputMode(mode);
  return TRUE;
}

BOOL CGraphBuilder::GetGadgetThreadsNumber(FXString& uid, int& n)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget)
    return FALSE;
  n = Gadget->GetCoresNumber();
  return TRUE;
}

BOOL CGraphBuilder::SetGadgetThreadsNumber(FXString& uid, int n)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget || !n)
    return FALSE;
  return Gadget->SetCoresNumber(n);
}

BOOL CGraphBuilder::SetGadgetStatus(FXString& uid, LPCTSTR statusname, bool status)
{
  CGadget* gadget=GetGadget(uid);
  if (gadget)
  {
    gadget->Status().WriteBool(statusname,status);
    return true;
  }
  SENDWARN_1("Can't find gadget '%s' in SetGadgetStatus",uid);
  return false;
}

BOOL CGraphBuilder::GetGadgetStatus(FXString& uid, LPCTSTR statusname, bool& status)
{
    CGadget* gadget=GetGadget(uid);
    if (gadget)
	{
		gadget->Status().GetBool(statusname,status);
        return true;
	}
    SENDWARN_1("Can't find gadget '%s' in GetGadgetStatus",uid);
    return false;
}

BOOL CGraphBuilder::GetGadgetIsMultyCoreAllowed(FXString& uid)
{
  CGadget* Gadget = GetGadget(uid);
  if (!Gadget)
    return FALSE;
  return Gadget->IsMultyCoreAllowed();
}

bool CGraphBuilder::PrintProperties(LPCTSTR gadgetUID, FXString& text)
{
  CGadget* gadget=GetGadget(gadgetUID);
  if ((gadget) && (gadget->PrintProperties(text)))
  {
    gadget->Status().WriteBool(STATUS_MODIFIED,true);
    return true;
  }
  SENDWARN_1("Can't find gadget '%s' in PrintProperties",gadgetUID);
  return false;
}

bool CGraphBuilder::ScanProperties(LPCTSTR gadgetUID, LPCTSTR text, bool& Invalidate)
{
  CGadget* gadget=GetGadget(gadgetUID);
  if ((gadget) && (gadget->ScanProperties(text,Invalidate)))
    return true;
  SENDWARN_1("Can't find gadget '%s' in ScanProperties",gadgetUID);
  return false;
}

bool CGraphBuilder::ScanSettings(LPCTSTR gadgetUID, FXString& text)
{
  CGadget* gadget=GetGadget(gadgetUID);
  if ( gadget )
  {
    return gadget->ScanSettings( text );
  }
  SENDWARN_1( "Can't find gadget '%s' in ScanSettings" , gadgetUID );
  return false;
}


// Function returns graph gadget list in alphabet order
int CGraphBuilder::EnumAndArrangeGadgets( FXStringArray& GadgetList )
{
  CStringArray SrcGadgets ;
  CStringArray DstGadgets ;
  EnumGadgets( SrcGadgets , DstGadgets ) ;

  for ( int i = 0 ; i < SrcGadgets.GetCount() ; i++ )
    GadgetList.Add( (LPCTSTR) SrcGadgets[ i ] ) ;
  for ( int i = 0 ; i < DstGadgets.GetCount() ; i++ )
    GadgetList.Add( (LPCTSTR) DstGadgets[ i ] ) ;
   qsort( (void*) &GadgetList[ 0 ] , GadgetList.GetCount() ,
    sizeof( FXString* ) , FXStringCompareAscending ) ;

  return 0;
}

BOOL CGraphBuilder::SetFloatWnd( LPCTSTR name , double x ,
  double y , double w , double h , LPCTSTR pSelected )
{
  return m_ViewSection.SetFloatWnd( name , x , y , w , h , pSelected );
}
BOOL CGraphBuilder::GetFloatWnd( LPCTSTR name , double& x ,
  double& y , double& w , double& h , FXString * pSelected )
{
  return m_ViewSection.GetFloatWnd( name , x , y , w , h , pSelected );
}
