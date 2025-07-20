#ifndef __SKETCH_VIEW_MODIFIED__
#define __SKETCH_VIEW_MODIFIED__

//CSketchViewMod class - modified version of SketchView, for using with CEvadeConnectorGlyph
class CSketchViewMod : public CSketchView
{
protected:
  ViewType m_ViewType;
  FXArray<CCompositeGlyph*, CCompositeGlyph*> m_CoherentComponents;
  void GetCoherency();
public:
  CSketchViewMod() : CSketchView(), m_ViewType() {};
  virtual ~CSketchViewMod();
  //old overloaded functions
  void DeleteSelected();
  virtual void Paint();
  virtual void SetViewType(ViewType vt); 
  virtual ViewType GetViewType() {return m_ViewType;}
  //new functions
  CGlyph *IsPointInGlyphRect(CPoint pt, CRect *res_measured, int offset_X, int offset_Y, bool start = false);
  CGlyph *IsPointInGlyphRect(int x, int y, CRect *res_measured, int offset_X, int offset_Y, bool start = false);
  CGlyph *IsPointInGlyphRect(CPoint pt, CRect *res_measured, int offset_X, int offset_Y, CGlyph *father);
  CGlyph *IsPointInGlyphRect(int x, int y, CRect *res_measured, int offset_X, int offset_Y, CGlyph *father);
  CGlyph *GraphIntersect(CPoint pt);
  CGlyph *GraphIntersect(int x, int y);
  int GetGadgetCount() {return m_Graph.GetChildrenCount();}
  void ResetCornerCounters();
};

#endif