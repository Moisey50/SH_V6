#ifndef __INC__Reservuar_H__
#define __INC__Reservuar_H__

#include "NodeList.h"



class Reservuar
{
  Node1DList m_NodeList;
  Node1D * m_pCentralNode = NULL ;
 
public:
  double m_dMinSize = DBL_MAX ;
  double m_dMaxSize = 0 ;;
  CDRect m_NodesArea ;

  Reservuar()
  {
    m_NodesArea = CDRect( DBL_MAX , -DBL_MAX , DBL_MAX , -DBL_MAX ) ;
  }
  ~Reservuar()
  {
    delAll();
  }
  Node1DList * GetNodeList() { return &m_NodeList ; }
  Node1D * GetCentralNode() { return m_pCentralNode ; }

  bool CheckConsistence()
  {
    int iCnt = 0 ;
    Node1D * p = m_NodeList.GetHead() ;
    while ( p )
    {
      iCnt++ ;
      p = p->next ;
    } ;
    bool bOK = (iCnt == m_NodeList.getCnt()) ;
    ASSERT( bOK ) ;
    return bOK ;
  }
  inline void insert(Spot& spot)
  {
    double dSize = spot.getSize() ;
    if(m_dMinSize > dSize) 
      m_dMinSize= dSize ;
    if(m_dMaxSize < dSize ) 
      m_dMaxSize= dSize ;

    if ( spot.m_imgCoord.real() < m_NodesArea.left )
      m_NodesArea.left = spot.m_imgCoord.real() ;
    if ( spot.m_imgCoord.real() > m_NodesArea.right )
      m_NodesArea.right = spot.m_imgCoord.real() ;
    if ( spot.m_imgCoord.imag() < m_NodesArea.top )
      m_NodesArea.top = spot.m_imgCoord.imag() ;
    if ( spot.m_imgCoord.imag() > m_NodesArea.bottom )
      m_NodesArea.bottom = spot.m_imgCoord.imag() ;
    m_NodeList.insert(spot);
    if ( !m_pCentralNode )
      m_pCentralNode = m_NodeList.GetHead() ;
    else if ( ( m_NodeList.getCnt() & 1 ) == 0 )
      m_pCentralNode = m_pCentralNode->next ;
  }
  inline void insert( Spot * pSpot )
  {
    insert( *pSpot ) ;
  }
  inline void insert(unsigned int idx, cmplx& Pos, double width, double height)
  {
    Spot* spot = new Spot( idx, Pos , width , height );
    insert(*spot);
  }
  inline Node1D* find(const Spot& spot) const
  {
    return m_NodeList.find(spot);
  }
  inline Node1D* find(const double x1, const double x2, const double y1, const double y2) const
  {
    return m_NodeList.find(x1, x2, y1, y2);
  }
  inline Node1D* find( cmplx& NearPos , double dRadius ) const
  {
    Node1D* node = m_NodeList.m_pHead ;
    cmplx UpLeft = NearPos - cmplx( dRadius , dRadius ) ;
    cmplx DownRight = NearPos + cmplx( dRadius , dRadius ) ;

    while (node && !node->checkLocation( UpLeft , DownRight ))
      node = node->next;

    return node;
  }
  inline Spot * FindSpot( cmplx& NearPos , double dRadius ) const
  {
    Node1D* node = m_NodeList.m_pHead ;
    cmplx UpLeft = NearPos - cmplx( dRadius , dRadius ) ;
    cmplx DownRight = NearPos + cmplx( dRadius , dRadius ) ;

    while (node && !node->checkLocation( UpLeft , DownRight ))
      node = node->next;

    return (node) ? node->m_Spot : NULL ;
  }

  inline void remove(Node1D& node)
  {
    m_NodeList.remove(node);
  }
  inline void del(Node1D& node)
  {
    m_NodeList.del(node);
  }
  inline void delAll()
  {
    Node1D * pNode = m_NodeList.GetHead() ;
    while ( pNode )
    {
      delete pNode->m_Spot ;
      pNode = pNode->next ;
    }

    m_NodeList.delAll();
  }
  inline Node1DList* getBigs() const
  {
    double dDiff = m_dMaxSize - m_dMinSize ;
    if ( dDiff / m_dMaxSize < 0.25 )
      return NULL ;
    double threshold = m_dMinSize + ((m_dMaxSize - m_dMinSize) * 3./4.) ;

    Node1DList* bigs = new Node1DList();
    Node1D* node = m_NodeList.m_pHead;

    while(node)
    {
      if(node->getSize() > threshold)
        bigs->insert(*node->m_Spot);

      node = node->next;
    }

    return bigs;
  }
  inline Spot* outNextSpot(const Spot& current, const double dx, const double dy, const double dPosTol)
  {
    Spot* spot = NULL;

    Node1D* node = m_NodeList.getNextLineNode(current, cmplx( dx , dy ) , dPosTol);	

    if(node)
    {
      spot = node->m_Spot;
      m_NodeList.del(*node);
    }
    else // Fix Missing Points
    {
      node = m_NodeList.getNextLineNode(current, 2.*cmplx( dx , dy ), 2*dPosTol); // Look For Missing Point
      if(node)
      {
        spot = new Spot(-1, (current.m_imgCoord + node->m_Spot->m_imgCoord)/2. ,
          GetAveSize( current.m_Sizes , node->m_Spot->m_Sizes ) ) ;
      }
    }

    return spot;
  }
  inline Spot* outNextSpot(const Spot& current, const cmplx& dPos , const double dPosTol)
  {
    Spot* spot = NULL;

    Node1D* node = m_NodeList.getNextLineNode(current , dPos , dPosTol);	

    if(node)
    {
      spot = node->m_Spot;
      m_NodeList.del(*node);
    }
    else // Fix Missing Points
    {
      node = m_NodeList.getNextLineNode(current, 2. * dPos , 2.*dPosTol); // Look For Missing Point
      if(node)
      {
        spot = new Spot(-1, (current.m_imgCoord + node->m_Spot->m_imgCoord)/2. ,
          GetAveSize( current.m_Sizes , node->m_Spot->m_Sizes ) ) ;
      }
    }

    return spot;
  }
  inline unsigned int getCount()
  {
    return m_NodeList.m_uiCnt;
  }
};



#endif	//	__INC__Reservuar_H__



