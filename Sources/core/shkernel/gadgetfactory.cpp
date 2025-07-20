// GadgetFactory.cpp: implementation of the CGadgetFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GadgetFactory.h"
#include "GraphBuilder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// CGadgetFactory

CGadgetFactory::CGadgetFactory():
m_pCurGadget(NULL)
{
}

CGadgetFactory::~CGadgetFactory()
{
	DropCurGadget();
}

BOOL CGadgetFactory::RegisterGadgetClass(CRuntimeGadget* RuntimeGadget)
{
	m_Gadgets.SetAt(RuntimeGadget->m_lpszClassName, RuntimeGadget);
	return TRUE;
}

void CGadgetFactory::UnregisterGadgetClass(CRuntimeGadget* RuntimeGadget)
{
	CRuntimeGadget* ThisRuntimeGadget;
	POSITION pos = m_Gadgets.GetStartPosition();
	while (pos)
	{
		CString className;
		m_Gadgets.GetNextAssoc(pos, className, (void*&)ThisRuntimeGadget);
		if (ThisRuntimeGadget == RuntimeGadget)
		{
			m_Gadgets.RemoveKey(className);
		}
	}
}

BOOL CGadgetFactory::CreateGadget(LPCTSTR GadgetClassName)
{
	DropCurGadget();
	CRuntimeGadget* RuntimeGadget = NULL;
	if (m_Gadgets.Lookup(GadgetClassName, (void*&)RuntimeGadget) && RuntimeGadget)
	{
		m_pCurGadget = RuntimeGadget->m_pfnCreateGadget();
		//m_pCurGadget->SetThreadName(GadgetClassName);
		return TRUE;
	}
	return FALSE;
}

CGadget* CGadgetFactory::GetCurGadget()
{
	CGadget* pGadget = m_pCurGadget;
	m_pCurGadget = NULL;
	return pGadget;
}

void CGadgetFactory::EnumGadgetClassesAndLineages(CUIntArray& Types, CStringArray& Classes, CStringArray& Lineages)
{
	Types.RemoveAll();
	Classes.RemoveAll();
	Lineages.RemoveAll();
	CString className;
	CRuntimeGadget* RuntimeGadget;
	POSITION pos = m_Gadgets.GetStartPosition();
	while (pos)
	{
		m_Gadgets.GetNextAssoc(pos, className, (void*&)RuntimeGadget);
		if (RuntimeGadget->IsDerivedFrom(RUNTIME_GADGET(CCtrlGadget)))
			Types.Add(IGraphbuilder::TVDB400_GT_CTRL);
		else if (RuntimeGadget->IsDerivedFrom(RUNTIME_GADGET(CCaptureGadget)))
			Types.Add(IGraphbuilder::TVDB400_GT_CAPTURE);
		else if (RuntimeGadget->IsDerivedFrom(RUNTIME_GADGET(CFilterGadget)))
			Types.Add(IGraphbuilder::TVDB400_GT_FILTER);
		else
			Types.Add(IGraphbuilder::TVDB400_GT_OTHER);
		Classes.Add(className);
		Lineages.Add(RuntimeGadget->m_lpszLineage);
	}
}

void CGadgetFactory::DropCurGadget()
{
	if (m_pCurGadget)
    {
        m_pCurGadget->ShutDown();
		delete m_pCurGadget;
    }
	m_pCurGadget = NULL;
}

void CGadgetFactory::EnumGadgetClassesAndPlugins(CStringArray &Classes, CStringArray &Plugins)
{
	Classes.RemoveAll();
	Plugins.RemoveAll();
	CString className;
	CRuntimeGadget* RuntimeGadget;
	POSITION pos = m_Gadgets.GetStartPosition();
	while (pos)
	{
		m_Gadgets.GetNextAssoc(pos, className, (void*&)RuntimeGadget);
		Classes.Add(CString(RuntimeGadget->m_lpszLineage)+"."+className);
		Plugins.Add(RuntimeGadget->m_lpszPlugin);
	}

}
