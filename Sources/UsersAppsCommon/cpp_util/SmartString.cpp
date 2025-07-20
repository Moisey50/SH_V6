#include "SmartString.h"
#include <stdio.h>
#include <string.h>

CSmartString::CSmartString()
{
	m_pchString = NULL;
	m_iSize = 0;
	m_iLength = 0;
	m_bIsStringStatic = false;
}

CSmartString::~CSmartString()
{
	Destruct();
}

void CSmartString::operator = (CSmartString &NewString)
{
	SetString(NewString.m_pchString, NewString.m_iLength);
}

void CSmartString::operator = ( const char* szText )
{
	SetString( szText );
}

bool CSmartString::operator == (CSmartString &CmpString)
{
	return Compare(CmpString.m_pchString, CmpString.m_iLength);
}

bool CSmartString::operator == (const char *pszCmpString)
{
	return Compare(pszCmpString);
}

CSmartString::operator const char* ()
{
	return GetString();
}

bool CSmartString::Reference(CSmartString &ReferenceString)
{
	if ( !Construct( ReferenceString.m_iLength, ReferenceString.m_pchString ) )
		return false;

	m_iLength = ReferenceString.m_iLength;

	return true;
}

bool CSmartString::Construct(int iStringPoolSize, void *pvStringPool /* = NULL */)
{
	Destruct();

	if ( iStringPoolSize - 1 > 0 && pvStringPool != NULL )
	{
		m_pchString = (char *)pvStringPool;
		m_iSize = iStringPoolSize;
		m_bIsStringStatic = true;
		return true;
	}

	if ( Reallocate( iStringPoolSize ) )
		return true;

	return false;
}

void CSmartString::Destruct()
{
	if ( !m_bIsStringStatic )
		delete m_pchString;

	m_pchString = NULL;
	m_iSize = 0;
	m_iLength = 0;
	m_bIsStringStatic = false;
}

bool CSmartString::SetString(const char *pchNewString, int iNewStringLength /* = -1 */)
{
	if ( iNewStringLength == -1 )
		iNewStringLength = strlen(pchNewString);
	if ( iNewStringLength >= m_iSize )
	{
		if ( !Reallocate(iNewStringLength + 1) )
			return false;
	}
	strcpy( m_pchString, pchNewString );
	m_iLength = iNewStringLength;

	return true;
}

//Warning !!! - always alloc enough buffer to include extra chars
//This is required to help prevent mem fragmentation
bool CSmartString::AddChar(char chChar)
{
	//1 for new char and 1 for '\0'
	if ( m_iLength + 2 > m_iSize )
		return false;

	m_pchString[ m_iLength++ ] = chChar;
	m_pchString[ m_iLength ] = 0;

	return true;
}

bool CSmartString::Compare(const char *pchCmpString, int iCmpStringLength /* = -1 */)
{
	if ( iCmpStringLength == -1 )
		iCmpStringLength = strlen(pchCmpString);

	if ( m_iLength != iCmpStringLength )
		return false;

	return ( memcmp( m_pchString, pchCmpString, m_iLength ) == 0 );
}

bool CSmartString::Reallocate( int iNewStringSize )
{
	if ( m_bIsStringStatic )
		return false;

	Destruct();

	if ( iNewStringSize > 0 )
	{
		m_pchString = new char[ iNewStringSize ];
		if ( m_pchString == NULL )
			return false; //out of mem

		m_iSize = iNewStringSize;
	}
	return true;
}

void CSmartString::operator += ( const char* sz2Add )
{
	int iLen = strlen( sz2Add );
	int iThisLen = GetLength();
	//if not enough space, alloc some more
	if ( m_iSize - iThisLen - 1 < iLen )	
	{
		if ( m_bIsStringStatic )
			return; //cant alloc, no space

		char* szTemp = new char[ m_iSize ];
		strcpy( szTemp, m_pchString );
		if ( !Construct( iThisLen + 1 + iLen ) )
		{
			delete szTemp;
			return;
		}
		sprintf( m_pchString, "%s%s", szTemp, sz2Add );
		delete szTemp;
	}
	else
	{
		strcpy( &m_pchString[m_iLength], sz2Add );
	}

	m_iLength = iLen + iThisLen;
}

void CSmartString::SetChar( char c, int iIdx )
{
	if ( iIdx < GetLength() )
		m_pchString[ iIdx ] = c;
}

void CSmartString::RecalcLen()
{
	m_iLength = strlen( m_pchString );
}
