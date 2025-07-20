#ifndef __SMARTSTRING_H__
#define __SMARTSTRING_H__

#include <stdlib.h>

class CSmartString
{
	char *m_pchString;
	int m_iLength;
	int m_iSize;
	bool m_bIsStringStatic;
	
public:
	CSmartString();

	~CSmartString();

	void operator = (CSmartString &NewString);
	void operator = ( const char* szText );

	bool operator == (CSmartString &CmpString);
	bool operator == (const char *pszCmpString);
	void operator += ( const char* sz2Add );
	operator const char* ();

	bool Reference(CSmartString &ReferenceString);
	bool Construct(int iStringPoolSize, void *pvStringPool = NULL);
	void Destruct();

	bool SetString(const char *pchNewString, int iNewStringLength = -1);
	const char* GetString() { return m_pchString; }
	int GetLength() { return m_iLength; }
	bool IsEmpty() { return ( m_iLength == 0 ); }
	void Empty() { m_iLength = 0; }
	bool AddChar( char chChar );
	bool Compare( const char *pchCmpString, int iCmpStringLength = -1 );
	bool Reallocate( int iNewStringSize );
	void SetChar( char c, int iIdx );
	void RecalcLen();

private:
	CSmartString(CSmartString&);

};

#endif
