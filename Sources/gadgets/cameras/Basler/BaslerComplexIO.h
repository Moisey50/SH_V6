#pragma once

class BaslerIOLine
{
public:
  string m_Name ;
  string m_Mode ;
  string m_Source ;
  BOOL   m_bInverter ;

  BaslerIOLine( LPCTSTR pLineName , LPCTSTR pLineMode , LPCTSTR pLineSource , BOOL bLineInverter )
  {
    m_Name = pLineName ;
    m_Mode = pLineMode ;
    m_Source = pLineSource ;
    m_bInverter = bLineInverter ;
  }
  BaslerIOLine( LPCTSTR pAsString )
  {
    FromString( pAsString ) ;
  }

  BaslerIOLine& operator =( BaslerIOLine& Orig )
  {
    m_Name = Orig.m_Name     ;
    m_Mode = Orig.m_Mode     ;
    m_Source = Orig.m_Source    ;
    m_bInverter = Orig.m_bInverter ;
    return *this ;
  }
  void FromString( LPCTSTR pString )
  {
    FXPropertyKit pk( pString ) ;
    FXString sVal ;
    if ( pk.GetString( "LineSelector_" , sVal , false ) )
      m_Name = sVal ;
    if ( pk.GetString( "LineMode_" , sVal , false ) )
      m_Mode = sVal ;
    if ( pk.GetString( "LineSource_" , sVal , false ) )
      m_Source = sVal ;
    pk.GetInt( "LineInverter_" , m_bInverter ) ;
  }
  string ToString()
  {
    FXPropertyKit pk ;
    pk.WriteString( "LineSelector_" , m_Name.c_str() , false ) ;
    pk.WriteString( "LineMode_" , m_Mode.c_str() , false ) ;
    pk.WriteString( "LineSource_" , m_Source.c_str() , false ) ;
    pk.WriteInt( "LineInverter_" , m_bInverter ) ;
    return string( (LPCTSTR) pk ) ;

  }
};

class BaslerTimer
{
public:
  string m_Name ;
  string m_Source ;
  string m_Activation ;
  double m_dDuration ;
  double m_dDelay ;

  BaslerTimer( LPCTSTR pTimerName , LPCTSTR pTimerSource ,
    double dDuration = 0. , double dDelay = 0. , LPCTSTR pActivation = "Unknonwn" )
  {
    m_Name = pTimerName ;
    m_Source = pTimerSource ;
    m_Activation = pActivation ;
    m_dDuration = dDuration ;
    m_dDelay = dDelay ;
  }
  BaslerTimer( LPCTSTR pAsString )
  {
    FromString( pAsString ) ;
  }
  BaslerTimer& operator =( BaslerTimer& Orig )
  {
    m_Name = Orig.m_Name ;
    m_Activation = Orig.m_Activation ;
    m_Source = Orig.m_Source ;
    m_dDuration = Orig.m_dDuration  ;
    m_dDelay = Orig.m_dDelay    ;
    return *this ;
  }
  void FromString( LPCTSTR pString )
  {
    FXPropertyKit pk( pString ) ;
    FXString sVal ;
    if ( pk.GetString( "TimerSelector_" , sVal , false ) )
      m_Name = sVal ;
    if ( pk.GetString( "TimerTriggerSource_" , sVal , false ) )
      m_Source = sVal ;
    if ( pk.GetString( "TimerTriggerActivation_" , sVal , false ) )
      m_Activation = sVal ;
    pk.GetDouble( "TimerDuration_" , m_dDuration ) ;
    pk.GetDouble( "TimerDelay_" , m_dDelay ) ;
  }
  string ToString()
  {
    FXPropertyKit pk ;
    pk.WriteString( "TimerSelector_" , m_Name.c_str() , false ) ;
    pk.WriteString( "TimerTriggerSource_" , m_Source.c_str() , false ) ;
    pk.WriteString( "TimerTriggerActivation_" , m_Activation.c_str() , false ) ;
    pk.WriteDouble( "TimerDuration_" , m_dDuration ) ;
    pk.WriteDouble( "TimerDelay_" , m_dDelay ) ;
    return string( (LPCTSTR) pk ) ;
  }
};

class BaslerUserOut
{
public:
  string m_Name ;
  BOOL   m_bValue ;

  BaslerUserOut( LPCTSTR pName , BOOL bValue = false )
  {
    m_Name = pName ;
    m_bValue = bValue ;
  }
  BaslerUserOut( LPCTSTR pAsString )
  {
    FromString( pAsString ) ;
  }

  BaslerUserOut& operator=( BaslerUserOut& Orig )
  {
    m_Name = Orig.m_Name     ;
    m_bValue = Orig.m_bValue ;
    return *this ;
  }
  void FromString( LPCTSTR pString )
  {
    FXPropertyKit pk( pString ) ;
    FXString sVal ;
    if ( pk.GetString( "UserOutputSelector_" , sVal , false ) )
      m_Name = sVal ;
    pk.GetInt( "UserOutputValue_" , m_bValue ) ;
  }
  string ToString()
  {
    FXPropertyKit pk ;
    pk.WriteString( "UserOutputSelector_" , m_Name.c_str() , false ) ;
    pk.WriteInt( "UserOutputValue_" , m_bValue ) ;
    return string( (LPCTSTR) pk ) ;

  }
};

typedef vector<BaslerIOLine> BaslerIOLines ;
typedef vector<BaslerTimer>  BaslerTimers ;
typedef vector<BaslerUserOut>  BaslerUserOuts ;

class BaslerComplexIOs
{
public:
  BaslerIOLines m_Lines ;
  BaslerTimers  m_Timers ;
  BaslerUserOuts m_Outs ;

  void clear()
  {
    m_Lines.clear() ;
    m_Timers.clear() ;
    m_Outs.clear() ;
  }
  void Copy( BaslerComplexIOs& Other )
  {
    clear() ;
    m_Lines = Other.m_Lines ;
    m_Timers = Other.m_Timers ;
    m_Outs = Other.m_Outs ;
  }
  BaslerComplexIOs& operator=( BaslerComplexIOs& Other )
  {
    Copy( Other ) ;
    return *this ;
  }
  bool IsEmpty()
  {
    return (m_Lines.empty() && m_Outs.empty() && m_Timers.empty()) ;
  }
  BaslerIOLine * FindLine( LPCTSTR pName )
  {
    for ( size_t i = 0 ; i < m_Lines.size() ; i++ )
    {
      if ( m_Lines[ i ].m_Name == pName )
        return &m_Lines[ i ] ;
    }
    return NULL ;
  }
  BaslerTimer * FindTimer( LPCTSTR pName )
  {
    for ( size_t i = 0 ; i < m_Timers.size() ; i++ )
    {
      if ( m_Timers[ i ].m_Name == pName )
        return &m_Timers[ i ] ;
    }
    return NULL ;
  }
  BaslerUserOut * FindOut( LPCTSTR pName )
  {
    for ( size_t i = 0 ; i < m_Outs.size() ; i++ )
    {
      if ( m_Outs[ i ].m_Name == pName )
        return &m_Outs[ i ] ;
    }
    return NULL ;
  }
};