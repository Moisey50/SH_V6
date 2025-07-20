#ifndef __INC__NodeList_H__
#define __INC__NodeList_H__

#include "Spot.h"

class Node1DList ;


class Node1D
{
public:

  Spot * m_Spot;

  Node1D* next;
  Node1D* previous;
  Node1DList * m_pList ;

  inline Node1D( Spot * s ) : m_Spot( s )
  {
    next = previous = NULL;
    m_pList = NULL ;
  }
  inline bool checkLocation( const double x1 , const double x2 , const double y1 , const double y2 ) const
  {
    return m_Spot->checkLocation( x1 , x2 , y1 , y2 );
  }
  inline bool checkLocation( const cmplx& UpLeft , const cmplx& DownRight ) const
  {
    return m_Spot->checkLocation( UpLeft , DownRight );
  }
  inline double getSize() const
  {
    return m_Spot->getSize();
  }
  inline double getDistance( const Spot& spot )
  {
    return m_Spot->getDistance( spot );
  }
  inline double getDistance( const cmplx& Pos )
  {
    return m_Spot->getDistance( Pos );
  }
  inline double getDistance( const Node1D& node )
  {
    return m_Spot->getDistance( *( node.m_Spot ) );
  }
};

typedef vector<Node1D*> Nodes1D ;

class Node1DList
{
  Node1D* m_pHead;
  Node1D* m_pTail;
  unsigned int m_uiCnt;

  bool CheckForPresence( Node1D * pNode )
  {
    Node1D * p = m_pHead ;

    while ( p )
    {
      if ( ( p == pNode ) || ( p->m_Spot == pNode->m_Spot ) )
        return true ;
      p = p->next ;
    }
    return false ;
  }

public:
  inline void remove( Node1D& node )
  {
    ASSERT( this == node.m_pList ) ;
    if ( node.previous )
      node.previous->next = node.next;
    else if ( m_pHead == &node )
      m_pHead = node.next;
    else
      ASSERT( 0 ) ;

    if ( node.next )
      node.next->previous = node.previous;
    else if ( m_pTail == &node )
      m_pTail = node.previous;
    else
      ASSERT( 0 ) ;

    node.previous = NULL;
    node.next = NULL;
    node.m_pList = NULL ;
    m_uiCnt--;
  }

  inline void remove( Node1D * pNode )
  {
    return remove( *pNode ) ;
  }
  inline void Reverse()
  {
    if ( m_uiCnt > 1 )
    {
      Node1D * pFirst = m_pHead ;
      Node1D * pCurrent = m_pHead ;
      do
      {
        Node1D * pTmp = pCurrent->next ;
        pCurrent->next = pCurrent->previous ;
        pCurrent->previous = pTmp ;
        pCurrent = pTmp ;
      } while ( pCurrent ) ;
      m_pHead = m_pTail ;
      m_pTail = pFirst ;
    }
  }
  int GetRealLength()
  {
    Node1D * pNext = m_pHead ;
    int iCnt = 0 ;
    while ( pNext )
    {
      iCnt++ ;
      pNext = pNext->next ;
    }
    return iCnt ;
  }

  inline Node1D* getNearestNeighbor( const Spot& spot ) const
  {
    return getNearestNeighbor( &spot ) ;
  }

  inline Node1D* getNearestNeighbor( const Spot * pSpot ) const
  {
    if ( m_uiCnt < 1 )
      return NULL;

    Node1D* candidate = NULL;
    double d , min;
    Node1D* pNext = m_pHead;
    min = DBL_MAX;

    while ( pNext )
    {
      if ( pNext->m_Spot == pSpot )
      {
        pNext = pNext->next;
        continue;
      }

      d = pSpot->getDistance( *pNext->m_Spot );
      if ( d < min )
      {
        min = d;
        candidate = pNext;
      }
      pNext = pNext->next;
    }

    return candidate;
  }

     // the same as previous, but with selection of neighbor
     // in proper sector (from dLowAngle to dHighAngle)
  inline Node1D* getNearestNeighbor(
    const Spot * pSpot , double dLowAngle , double dHighAngle ) const
  {
    if ( m_uiCnt < 1 )
      return NULL;

    Node1D* candidate = NULL;
    double d , min;
    Node1D* pNext = m_pHead;

    min = DBL_MAX;
    double dDeltaAngle = dHighAngle - dLowAngle ;
    dHighAngle = fmod( dHighAngle + M_2PI , M_PI ) ;
    dLowAngle = dHighAngle - dDeltaAngle ;
    while ( pNext )
    {
      if ( pNext->m_Spot == pSpot )
      {
        pNext = pNext->next;
        continue;
      }

      d = pSpot->getDistance( *pNext->m_Spot );
      if ( d < min )
      {
        double dAngle = pSpot->getAngle( *pNext->m_Spot ) ;
        if ( fabs( dAngle ) >= M_PI )
          dAngle = fmod( dAngle + M_2PI , M_PI ) ;
        if ( ( dLowAngle <= dAngle && dAngle <= dHighAngle ) )
        {
          min = d;
          candidate = pNext;
        }
      }
      pNext = pNext->next;
    }
    return candidate;
  }

     // Get vector of nearest nodes, which are in the range up to
     // 2.5 of minimal distances from original spot
  inline Nodes1D* getNearestNeighbors(
    const Spot * pSpot , double dRange ) const
  {
    if ( m_uiCnt < 1 )
      return NULL;

    Node1D* candidate = NULL;
    double dMinDistance = DBL_MAX ;
    Node1D* pNext = m_pHead;
    Nodes1D * pResult = NULL ;
    double dStopRange = dRange * 2. ;

    while ( pNext )
    {
      if ( pNext->m_Spot == pSpot )
      {
        pNext = pNext->next;
        continue;
      }

      if ( pSpot->MatchInImg( pNext->m_Spot->m_imgCoord , dRange ) )
      {
        if ( !pResult )
          pResult = new Nodes1D ;

        pResult->push_back( pNext ) ;
      }
      else
      {
        if ( pResult
          && ( fabs( pNext->m_Spot->m_imgCoord.real() - pSpot->m_imgCoord.real() ) > dStopRange )
          && ( fabs( pNext->m_Spot->m_imgCoord.imag() - pSpot->m_imgCoord.imag() ) > dStopRange ) )
        {
          break ;
        }
      }

      pNext = pNext->next;
    }
    return pResult ;
  }

  // returned value should be deleted by caller
  inline Nodes1D * GetNodeWith4Neightbors( Node1D * pStartNode , int iNAttempts = 10 )
  {
    if ( m_uiCnt < 5 )
      return NULL;

    Node1D* pCurrent = pStartNode ;
    Node1D * pNearest = getNearestNeighbor( pCurrent->m_Spot ) ;
    cmplx cDirToNearest = pNearest->m_Spot->m_imgCoord - pCurrent->m_Spot->m_imgCoord ;
    double dNearestDist = abs( cDirToNearest ) ;

    Nodes1D * pNodes = NULL ;
    Node1D * pPrevSelectedNode = NULL ;
    while ( iNAttempts-- )
    {
      pNodes = getNearestNeighbors( pCurrent->m_Spot , dNearestDist * 1.2 ) ;
      if ( pNodes )
      {
        if ( pNodes->size() == 4 )
        {
          pNodes->insert( pNodes->begin() , pCurrent ) ; // central node is first
          return pNodes ;
        }
        delete pNodes ;
      }

      pCurrent = pCurrent->next ;
      if ( pCurrent )
      {
        pCurrent = pCurrent->next ;
        if ( pCurrent )
        {
          pNearest = getNearestNeighbor( pCurrent->m_Spot ) ;
          cDirToNearest = pNearest->m_Spot->m_imgCoord - pCurrent->m_Spot->m_imgCoord ;
          dNearestDist = abs( cDirToNearest ) ;
          continue ;
        }
      }
      break ;
    }

    return NULL ;
  }

  inline Node1DList()
  {
    m_pHead = m_pTail = NULL;
    m_uiCnt = 0;
  }


  Node1D * GetHead() { return m_pHead ; }
  Node1D * GetTail() { return m_pTail ; }
  inline void AddToHead( Node1D * pNode )
  {
#ifdef _DEBUG
    ASSERT( !CheckForPresence( pNode ) ) ;
#endif
    pNode->previous = NULL ;
    if ( m_pHead )
    {
      pNode->next = m_pHead ;
      m_pHead->previous = pNode ;
      m_pHead = pNode ;
    }
    else
    {
      m_pHead = m_pTail = pNode ;
      ASSERT( !pNode->next && !pNode->previous ) ;
    }
    pNode->m_pList = this ;
    m_uiCnt++ ;
  }
  inline void AddToTail( Node1D * pNode )
  {
#ifdef _DEBUG
    ASSERT( !CheckForPresence( pNode ) ) ;
#endif
    pNode->next = NULL ;
    if ( m_pTail )
    {
      m_pTail->next = pNode ;
      pNode->previous = m_pTail ;
      m_pTail = pNode ;
    }
    else
    {
      m_pHead = m_pTail = pNode ;
      ASSERT( !pNode->next && !pNode->previous ) ;
    }
    pNode->m_pList = this ;
    m_uiCnt++ ;
  }
  inline Node1D * AddToHead( Spot * pSpot )
  {
    Node1D* pNode = new Node1D( pSpot );
    AddToHead( pNode ) ;
    return pNode ;
  }
  inline Node1D * AddToTail( Spot * pSpot )
  {
    Node1D* pNode = new Node1D( pSpot );
    AddToTail( pNode ) ;
    return pNode ;
  }

  inline void insert( Node1D& node , Node1D*& h , Node1D*& t )
  {
#ifdef _DEBUG
    ASSERT( !CheckForPresence( &node ) ) ;
#endif
    if ( h )
    {
      node.previous = t;
      t->next = &node;
      t = &node;
    }
    else
      h = t = &node;
    m_uiCnt++ ;
  }
  inline Node1D* insert( Spot * pSpot )
  {
    return AddToTail( pSpot ) ;
  }

  inline Node1D* insert( Spot& spot ) { return insert( &spot ) ; }

  inline void swap( Node1D& node1 , Node1D& node2 )
  {
    Node1D* tmp;

    tmp = node1.next;
    node1.next = node2.next;
    node2.next = tmp;

    tmp = node1.previous;
    node1.previous = node2.previous;
    node2.previous = tmp;
  }
  inline void del( Node1D& node )
  {
    remove( node );
    delete &node;
  }
  inline void del( Node1D * pNode )
  {
    remove( pNode );
    delete pNode ;
  }
  inline void delAll()
  {
    Node1D* node = m_pHead;
    Node1D* next;

    while ( node )
    {
      next = node->next;
      del( node );
      node = next;
    }
    m_pHead = m_pTail = NULL ;
    m_uiCnt = 0 ;
  }
  inline unsigned int getCnt() const
  {
    return m_uiCnt;
  }
  inline Node1D* find( const Spot& spot ) const
  {
    Node1D* node = m_pHead;

    while ( node && ( node->m_Spot != &spot ) )
      node = node->next;

    return node;
  }
  inline Node1D* find( const cmplx& Point ) const
  {
    Node1D* node = m_pHead;

    while ( node && ( node->m_Spot->m_imgCoord != Point ) )
      node = node->next;

    return node;
  }
  inline Node1D* find( const double x1 , const double x2 , const double y1 , const double y2 ) const
  {
    Node1D* node = m_pHead;

    while ( node && !node->checkLocation( x1 , x2 , y1 , y2 ) )
      node = node->next;

    return node;
  }
  inline Node1D* find( cmplx& UpLeft , cmplx DownRight ) const
  {
#ifdef _DEBUG
    int iNodeCnt = 0 ;
#endif
    Node1D* node = m_pHead;

    while ( node && !node->checkLocation( UpLeft , DownRight ) )
    {
      node = node->next;
#ifdef _DEBUG
      iNodeCnt++ ;
#endif
    }

    return node;
  }
  inline Node1D* find( cmplx& NearPos , double dRadius ) const
  {
    Node1D* node = m_pHead;
    cmplx UpLeft = NearPos - cmplx( dRadius , dRadius ) ;
    cmplx DownRight = NearPos + cmplx( dRadius , dRadius ) ;

    while ( node && !node->checkLocation( UpLeft , DownRight ) )
      node = node->next;

    return node;
  }
  inline Node1D* find_nearest( cmplx& Pos ) const
  {
    Node1D* pNode = m_pHead;
    double dMinDist = 1e40 ;
    Node1D * pMinDistNode = NULL ;

    while ( pNode )
    {
      double dDist = pNode->m_Spot->getDistance( Pos ) ;
      if ( dDist < dMinDist )
      {
        dMinDist = dDist ;
        pMinDistNode = pNode ;
      }
      pNode = pNode->next;
    }

    return pMinDistNode ;
  }


  inline bool sort( Node1D& edge )
  {
    Node1D* t = NULL;
    Node1D* h = NULL;
    unsigned int i = 0;
    unsigned int j = m_uiCnt;

    Node1D* n = &edge;

    while ( n )
    {
      remove( *n );
      insert( *n , h , t );
      i++;
      n = getNearestNeighbor( *n->m_Spot );
    }

    m_pHead = h;
    m_pTail = t;
    m_uiCnt = i;

    return ( m_uiCnt == j );
  }
  Node1DList* getNonCommonSpotsList( Node1DList& other ) const
  {
    Node1DList* res = new Node1DList();

    Node1D* n1 = m_pHead;

    while ( n1 )
    {
      Node1D* n2 = other.m_pHead;
      bool b = false;

      while ( n2 && !b )
      {
        b |= ( &n1->m_Spot == &n2->m_Spot );
        n2 = n2->next;
      }

      if ( !b )
      {
        res->insert( *n1->m_Spot );
      }

      n1 = n1->next;
    }

    return res;
  }
//   Node1D* getNextLineNode(const Spot& current, const double dx, const double dy, const double dPosTol) const
//   {
//     double d = sqrt(dx*dx + dy*dy);
//     double tolerance = d * dPosTol;
// 
//     double x = current.m_imgCoord.real();
//     double y = current.m_imgCoord.imag();
// 
//     double x1 = x + dx - tolerance;
//     double x2 = x + dx + tolerance;
//     double y1 = y + dy - tolerance;
//     double y2 = y + dy + tolerance;
// 
//     Node1D* node = find(x1, x2, y1, y2);
// 
//     return node;
//   }
  Node1D* getNextLineNode( const Spot& current , const cmplx& dPos , const double ddPosTol ) const
  {
    double d = abs( dPos ) ;
    double tolerance = d * ddPosTol;

    cmplx UpLeft = current.m_imgCoord + dPos - cmplx( tolerance , tolerance ) ;
    cmplx DownRight = current.m_imgCoord + dPos + cmplx( tolerance , tolerance ) ;

    Node1D* node = find( UpLeft , DownRight );

    return node;
  }

  ~Node1DList()
  {
    delAll();
  }

  friend class Reservuar;
  friend class Axis;
  friend class Grid;
};

struct GridCoordinates
{
  int x;
  int y;
};

class Node2D
{

  friend class Node2DList;
  friend class Grid;
  friend struct Line;
  friend struct VerticalLine;
  friend struct HorizontalLine;

public:
  Node2D( Spot * s = NULL )
  {
    left = right = up = down = NULL;
    m_Spot = s ;
  }
  inline void setGridCoordinates( int x , int y )
  {
    gridCoordinates.x = x;
    gridCoordinates.y = y;
  }
  inline Node2D* addLeft( Spot& spot )
  {
    Node2D * node = new Node2D( &spot );
    left = node;
    node->right = this;

    if ( up && up->left )
    {
      node->up = up->left;
      up->left->down = node;

      if ( up->left->left && up->left->left->down )
      {
        node->left = up->left->left->down;
        up->left->left->down->right = node;
      }
    }
    if ( down && down->left )
    {
      node->down = down->left;
      down->left->up = node;

      if ( down->left->left && down->left->left->up )
      {
        node->left = down->left->left->up;
        down->left->left->up->right = node;
      }
    }

    node->setGridCoordinates( gridCoordinates.x - 1 , gridCoordinates.y );
    return node;
  }
  inline Node2D* addRight( Spot& spot )
  {
    Node2D * node = new Node2D( &spot );
    right = node;
    node->left = this;

    if ( up && up->right )
    {
      node->up = up->right;
      up->right->down = node;

      if ( up->right->right && up->right->right->down )
      {
        node->right = up->right->right->down;
        up->right->right->down->left = node;
      }
    }
    if ( down && down->right )
    {
      node->down = down->right;
      down->right->up = node;

      if ( down->right->right && down->right->right->up )
      {
        node->right = down->right->right->up;
        down->right->right->up->left = node;
      }
    }

    node->setGridCoordinates( gridCoordinates.x + 1 , gridCoordinates.y );
    return node;
  }
  inline Node2D* addUp( Spot& spot )
  {
    Node2D * node = new Node2D( &spot );
    up = node;
    node->down = this;

    if ( left && left->up )
    {
      node->left = left->up;
      left->up->right = node;

      if ( left->up->up && left->up->up->right )
      {
        node->up = left->up->up->right;
        left->up->up->right->down = node;
      }
    }
    if ( right && right->up )
    {
      node->right = right->up;
      right->up->left = node;

      if ( right->up->up && right->up->up->left )
      {
        node->up = right->up->up->left;
        right->up->up->left->down = node;
      }
    }

    node->setGridCoordinates( gridCoordinates.x , gridCoordinates.y + 1 );
    return node;
  }
  inline Node2D* addDown( Spot& spot )
  {
    Node2D * node = new Node2D( &spot );
    down = node;
    node->up = this;

    if ( left && left->down )
    {
      node->left = left->down;
      left->down->right = node;

      if ( left->down->down && left->down->down->right )
      {
        node->down = left->down->down->right;
        left->down->down->right->up = node;
      }
    }
    if ( right && right->down )
    {
      node->right = right->down;
      right->down->left = node;

      if ( right->down->down && right->down->down->left )
      {
        node->down = right->down->down->left;
        right->down->down->left->up = node;
      }
    }

    node->setGridCoordinates( gridCoordinates.x , gridCoordinates.y - 1 );
    return node;
  }

  inline Node2D* getLeft( unsigned int steps = 1 )
  {
    return ( steps && left ) ? left->getLeft( steps - 1 ) : this;
  }
  inline Node2D* getRight( unsigned int steps = 1 )
  {
    return ( steps && right ) ? right->getRight( steps - 1 ) : this;
  }
  inline Node2D* getUp( unsigned int steps = 1 )
  {
    return ( steps && up ) ? up->getUp( steps - 1 ) : this;
  }
  inline Node2D* getDown( unsigned int steps = 1 )
  {
    return ( steps && down ) ? down->getDown( steps - 1 ) : this;
  }
  inline Node2D* getHorizontal( int steps = 0 )
  {
    return ( steps ) ? ( ( steps > 0 ) ? getRight( steps ) : getLeft( abs( steps ) ) ) : this;
  }
  inline Node2D* getVertical( int steps = 0 )
  {
    return ( steps ) ? ( ( steps > 0 ) ? getDown( steps ) : getUp( abs( steps ) ) ) : this;
  }

  inline Node2D* getLeftUnbound( unsigned int steps = 1 )
  {
    return ( steps ) ? ( ( left ) ? left->getLeft( steps - 1 ) : NULL ) : this;
  }
  inline Node2D* getRightUnbounded( unsigned int steps = 1 )
  {
    return ( steps ) ? ( ( right ) ? right->getRight( steps - 1 ) : NULL ) : this;
  }
  inline Node2D* getUpUnbounded( unsigned int steps = 1 )
  {
    return ( steps ) ? ( ( up ) ? up->getUp( steps - 1 ) : NULL ) : this;
  }
  inline Node2D* getDownUnbounded( unsigned int steps = 1 )
  {
    return ( steps ) ? ( ( down ) ? up->getDown( steps - 1 ) : NULL ) : this;
  }
  inline Node2D* getHorizontalUnbounded( int steps = 0 )
  {
    return ( steps ) ? ( ( steps > 0 ) ? getRightUnbounded( steps ) : getLeftUnbound( abs( steps ) ) ) : this;
  }
  inline Node2D* getVerticalUnbounded( int steps = 0 )
  {
    return ( steps ) ? ( ( steps > 0 ) ? getDownUnbounded( steps ) : getUpUnbounded( abs( steps ) ) ) : this;
  }

  inline Node2D* getNext()
  {
    if ( right )
      return right;
    else if ( down )
      return down->getLeft( -1 );
    else
      return NULL;
  }

  inline double getDistance( const Node2D& node ) const
  {
    return m_Spot->getDistance( *( node.m_Spot ) );
  }
  inline double getDistance( const cmplx& point ) const
  {
    return m_Spot->getDistance( point );
  }

  inline ~Node2D()
  {
    delete &m_Spot;
  }
private:
  Node2D* left;
  Node2D* right;
  Node2D* up;
  Node2D* down;
  Spot * m_Spot;
  GridCoordinates gridCoordinates;
};

class Node2DList
{
private:

  Node2D& center;

  inline void deleteAll()
  {
    Node2D* node1 = getFirst();

    while ( node1 )
    {
      Node2D* node2 = node1->getNext();
      delete node1;
      node1 = node2;
    }
  }

public:

  inline Node2DList( const Node2DList& other ) : center( other.center )
  {

  }
  inline Node2DList( Spot& centralSpot ) : center( *new Node2D( &centralSpot ) )
  {

    center.setGridCoordinates( 0 , 0 );
  }
  inline Node2D* getLeftUpVertix()
  {
    return center.getLeft( -1 )->getUp( -1 );
  }
  inline Node2D* getLeftDownVertix()
  {
    return center.getLeft( -1 )->getDown( -1 );
  }
  inline Node2D* getRightUpVertix()
  {
    return center.getRight( -1 )->getUp( -1 );
  }
  inline Node2D* getRightDownVertix()
  {
    return center.getRight( -1 )->getDown( -1 );
  }
  inline Node2D* getCenter()
  {
    return &center;
  }
  inline Node2D* getFirst()
  {
    return getLeftUpVertix();
  }
  inline Node2D* getLast()
  {
    return getRightDownVertix();
  }
  inline Node2D* getRow( int i )
  {
    Node2D* node = &center;

    if ( i > 0 )
      while ( node && i-- )
        node = node->down;
    else if ( i < 0 )
      while ( node && i++ )
        node = node->up;

    if ( node )
      node = node->getLeft( -1 );
  }
  inline Node2D* getColumn( int i )
  {
    Node2D* node = &center;

    if ( i > 0 )
      while ( node && i-- )
        node = node->right;
    else if ( i < 0 )
      while ( node && i++ )
        node = node->left;

    if ( node )
      node = node->getUp( -1 );
  }
  inline Node2D* getPosition( int i , int j )
  {
    Node2D* node = &center;

    if ( i > 0 )
      while ( node && i-- )
        node = node->right;
    else if ( i < 0 )
      while ( node && i++ )
        node = node->left;

    if ( node )
    {
      if ( j > 0 )
        while ( node && j-- )
          node = node->down;
      else if ( j < 0 )
        while ( node && j++ )
          node = node->up;
    }

    return node;
  }
  inline Node2DList* operator=( const Node2DList& other )
  {
    return this;
  }
  ~Node2DList()
  {
    deleteAll();
  }
};


#endif	//	__INC__NodeList_H__

