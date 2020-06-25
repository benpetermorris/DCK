/*-------------------------------------------------------------------------*
 * Inline conversion functions to convert Special/Thing/Sector types
 * to ints, and vice-versa.
 *
 * Non-inline functions are found in typcnvt.cpp.
 *-------------------------------------------------------------------------*/

inline const char* SpcTypeToStr( int iType )
{
    return stlActTypes->IdToStr( iType );
}

inline int SpcStrToType( const char *pszType )
{
    return stlActTypes->StrToId( pszType );
}

inline int SecStrToType( const char *pszType )
{
    return stlSectorTypes->StrToId( pszType );
}

inline const char *SecTypeToStr( int iType )
{
    return stlSectorTypes->IdToStr( iType );
}

int ThgStrToType( char *pszStr );
const char *ThgTypeToStr( int iType );
const char *AngleTypeToStr( int iAngle );
StrList* GetPopupList( const char *pszPopupName );
int AngleStrToType( char *pszAngle );
