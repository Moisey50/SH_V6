// GadgetFactory.h: interface for the CGadgetFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GADGETFACTORY_H__1852CD33_9235_4186_BB1F_21E3B753C0B7__INCLUDED_)
#define AFX_GADGETFACTORY_H__1852CD33_9235_4186_BB1F_21E3B753C0B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
// CGadgetFactory
//

class CGadgetFactory
{
	CGadget*		m_pCurGadget;	// currently created gadget -> will be added on ok OR dropped on cancel
	CMapStringToPtr	m_Gadgets;		// registered gadgets' classes
public:
	CGadgetFactory();
	~CGadgetFactory();

	BOOL RegisterGadgetClass(CRuntimeGadget* RuntimeGadget);
	void UnregisterGadgetClass(CRuntimeGadget* RuntimeGadget);
	BOOL CreateGadget(LPCTSTR GadgetClassName);
	CGadget* GetCurGadget();
	void EnumGadgetClassesAndLineages(CUIntArray& Types, CStringArray& Classes, CStringArray& Lineages);
	void EnumGadgetClassesAndPlugins(CStringArray& Classes, CStringArray& Plugins);

private:
	void DropCurGadget();
};

#endif // !defined(AFX_GADGETFACTORY_H__1852CD33_9235_4186_BB1F_21E3B753C0B7__INCLUDED_)
