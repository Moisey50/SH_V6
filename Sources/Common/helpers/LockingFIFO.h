#pragma once

#include <fxfc/fxfc.h>

template<typename T>
class LockingFIFO
{
private:
  FXArray<T>     vect;
  FXLockObject   guard;

public:
  void clear() const
  {
    FXAutolock al( guard );
    vect.clear();
  }

  void push( T const& _data )
  {
    FXAutolock al( guard );
    vect.Add( _data );
  }

  size_t size()
  {
    FXAutolock al( guard );
    return vect.size() ;
  }

  bool getfront( T& _value )
  {
    FXAutolock al( guard );
    if ( vect.IsEmpty() )
      return false;

    _value = vect.GetAt( 0 );
    vect.RemoveAt( 0 ) ;
    return true;
  }
};