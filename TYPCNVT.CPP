/*-------------------------------------------------------------------------*
 * TYPCNVT.CPP:
 * Non-inline functions similar to those found in typcnvt.hpp.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

int ThgStrToType( char *pszStr )
{
    short n = stlThingTypes->StrToId( pszStr );

    if( n != -1 )
        return pgdCurGame->thinginfo[n].type;

    return -1;
}

const char *ThgTypeToStr( int iType )
{
    static char errstr[128];

    int i = (short) pgdCurGame->ttype2idx[iType];

    if( i != 0xffff )
        return (*stlThingTypes)[i];

    sprintf( errstr, "Unknown type %d (%04X) [Possible error]", iType, iType );
    return errstr;
}

const char *AngleTypeToStr( int iAngle )
{
    if( !fHexen )
    {
        char    *rvl;
        return ((rvl = stlAngles->IdToStr( iAngle ))==NULL ? "<error>":rvl);
    }
    return stlAngles->IdToStr( iAngle );
}

int AngleStrToType( char *pszAngle )
{
    return stlAngles->StrToId( pszAngle );
}

StrList* GetPopupList( const char* pszPopupName )
{
    int i;

    for( i = 0; i < pgdCurGame->nPopups; i++ )
    {
        if( !strcmpi( pszPopupName, pgdCurGame->stlPopups[i]->GetName() ) )
            return pgdCurGame->stlPopups[i];
    }

    return NULL;
}
