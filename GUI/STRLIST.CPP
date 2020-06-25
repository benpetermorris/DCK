#include "std.h"
#include "strlist.hpp"
#include "misc.h"

void error( char *str, ... );

#pragma option -O2
static const char* stristr( const char* pszSub, const char* pszString, int iLen )
/*-------------------------------------------------------------------------*
 * Searches for pszSub in pszString.
 *-------------------------------------------------------------------------*/
{
    const char* p = pszString;
    char chSub0 = pszSub[0];
    int iSubLen = iLen;

    if( iSubLen == -1 )
        iSubLen = strlen( pszSub );

    while( *p )
    {
        const char *p2 = p, *pSub = pszSub;
        short i = iSubLen;

        while( *p2 == *pSub && i-- )
            ++p2, ++pSub;

        if( !i )
            return p;

        ++p;
    }

    return NULL;
}
#pragma option -O-

StrList::StrList( int iMaxItems, int iMaxWidth, const char* pszName ) :
    nItem(0),
    nMaxItem(iMaxItems),
    nMaxCol(iMaxWidth),
    iKwdOfs(-1),
    nItemKwd(-1),
    fCopy(0)
{
    // copy name
    if( pszName )
        strcpy( szName, pszName );
    else
        szName[0] = 0;

    // allocate chunk
    pChunk = new char[nMaxItem * (nMaxCol + 1)];
    // allocate strings
    pStrHead = new Str[nMaxItem];

    for( int i = 0; i < nMaxItem; i++ )
    {
        pStrHead[i].pszText = pChunk + (i * (nMaxCol + 1));
        pStrHead[i].iIdent = -1;
    }
}

StrList::StrList( StrList* pStrList )
// makes a copy of 'pStrList' without any of its keywords.
// this string list must not be modified.
{
    nMaxItem = pStrList->nItem;
    nItem = pStrList->nItem;
    nMaxCol = pStrList->nMaxCol;
    iKwdOfs = nItemKwd = -1;
    fCopy = 1;

    // copy name
    strcpy( szName, pStrList->szName );

    // allocate strings
    pStrHead = new Str[nMaxItem];

    int i2 = 0;

    for( int i = 0; i < nMaxItem; i++ )
    {
        if( pStrList->pStrHead[i].pszText[0] != '*' )
            memcpy( &pStrHead[i2++], &(pStrList->pStrHead[i]), sizeof( Str ) );
        else
            --nItem;
    }
}

StrList::~StrList()
{
    delete pStrHead;
    if( !fCopy )
        delete pChunk;
}

void StrList::AddTail( const char* pszText, int iIdent )
/*-------------------------------------------------------------------------*
 * Add a string to the end of the list.
 *-------------------------------------------------------------------------*/
{
    if( strlen( pszText ) >= nMaxCol )
    {
        error( "StrList (%s): String too long:\n%s", szName, pszText );
    }

    strcpy( pStrHead[nItem].pszText, pszText );
    pStrHead[nItem].iIdent = iIdent;
    ++nItem;
}

void StrList::SetAt( int iIndex, const char* pszText, int iIdent )
/*-------------------------------------------------------------------------*
 * Replace a string in the list
 *-------------------------------------------------------------------------*/
{
    if( strlen( pszText ) >= nMaxCol )
    {
        error( "StrList (%s): String too long:\n%s", szName, pszText );
    }

    strcpy( pStrHead[iIndex].pszText, pszText );
    pStrHead[iIndex].iIdent = iIdent;
}

void StrList::Delete( int iIndex )
/*-------------------------------------------------------------------------*
 * Deletes an item from the list
 *-------------------------------------------------------------------------*/
{
    Str strItem;

    // copy the string structure ..
    memcpy( &strItem, &pStrHead[iIndex], sizeof( Str ) );

    // copy down the rest of them
    memcpy( &pStrHead[iIndex], &pStrHead[iIndex+1],
            sizeof( Str ) * (nMaxItem - iIndex) );

    // copy the saved one into the end of the list
    memcpy( &pStrHead[nMaxItem-1], &strItem, sizeof( Str ) );

    --nItem;    // decrease overall length
}

void StrList::DeleteAll()
/*-------------------------------------------------------------------------*
 * Kill all the items!  All of them!  YEAH!
 *-------------------------------------------------------------------------*/
{
    nItem = 0;
    iKwdOfs = -1;
    nItemKwd = -1;
    // that was tough.
}

char* StrList::IdToStr( int iIdent, char* pszText )
/*-------------------------------------------------------------------------*
 * Finds iIdent in one of the strings, copies the string into pszText,
 * and returns pszText.
 *-------------------------------------------------------------------------*/
{
    int i, iLast;

    for( i = _FirstItem(), iLast = _LastItem(); i <= iLast; i++ )
    {
        if( pStrHead[i].iIdent == iIdent )
        {
            if( pszText == NULL )
                return pStrHead[i].pszText;
            strcpy( pszText, pStrHead[i].pszText );
            return pszText;
        }
    }

    if( pszText != NULL )
        pszText[0] = 0;
    return NULL;    // not found, eh?
}

int StrList::StrToId( const char* pszText )
/*-------------------------------------------------------------------------*
 * Finds an exact match for pszText (excluding case) and returns the id
 * associated with the string.
 *-------------------------------------------------------------------------*/
{
    int i, iLast;

    for( i = _FirstItem(), iLast = _LastItem(); i <= iLast; i++ )
    {
        if( !strcmpi( pszText, pStrHead[i].pszText ) )
            return pStrHead[i].iIdent;
    }

    return -1;    // not found, eh?
}

int StrList::IdToIndex( int iIdent )
/*-------------------------------------------------------------------------*
 * Finds iIdent in one of the strings, and returns the index of it.
 *-------------------------------------------------------------------------*/
{
    int i, iLast;

    for( i = _FirstItem(), iLast = _LastItem(); i <= iLast; i++ )
    {
        if( pStrHead[i].iIdent == iIdent )
            return i - _FirstItem();
    }

    return -1;    // not found, eh?
}

char* StrList::GetAt( char* pszText, int iIndex, int *ipID )
/*-------------------------------------------------------------------------*
 * Copies the iIndex'th string into pszText, and returns pszText.
 *-------------------------------------------------------------------------*/
{
    iIndex += _FirstItem();

    if( pszText )
        strcpy( pszText, pStrHead[iIndex].pszText );
    if( ipID )
        ipID[0] = pStrHead[iIndex].iIdent;

    return pszText;
}

const char* StrList::operator[]( int iIndex )
/*-------------------------------------------------------------------------*
 * Returns the string pointer as a const char *.
 *-------------------------------------------------------------------------*/
{
    return (const char*) pStrHead[_FirstItem() + iIndex].pszText;
}

int StrList::StrToIndex( const char* pszText )
/*-------------------------------------------------------------------------*
 * Finds an exact match for pszText (excluding case) and returns its index.
 *-------------------------------------------------------------------------*/
{
    int i, iLast;

    for( i = _FirstItem(), iLast = _LastItem(); i <= iLast; i++ )
    {
        if( !strcmpi( pszText, pStrHead[i].pszText ) )
            return i - _FirstItem();
    }

    return -1;    // not found, eh?
}

int StrList::Find( const char* pszText, int iSearchLen )
/*-------------------------------------------------------------------------*
 * Returns the index of the first string with some or all of pszText in it.
 *-------------------------------------------------------------------------*/
{
    int i, iLast;

    if( iSearchLen == -1 )
    {
        for( i = _FirstItem(), iLast = _LastItem(); i <= iLast; i++ )
        {
            if( !strcmpi( pszText, pStrHead[i].pszText ) )
                return i - _FirstItem();
        }

        int iTextLen = strlen( pszText );

        for( i = _FirstItem(), iLast = _LastItem(); i <= iLast; i++ )
        {
            if( stristr( pszText, pStrHead[i].pszText, iTextLen ) )
                return i - _FirstItem();
        }

    }
    else
    {
        for( i = _FirstItem(), iLast = _LastItem(); i <= iLast; i++ )
        {
            if( !memicmp( pszText, pStrHead[i].pszText, iSearchLen ) )
                return i - _FirstItem();
        }
    }

    return -1;
}

int StrList::FindNoKey( const char* pszText, bool fPartial )
/*-------------------------------------------------------------------------*
 * Returns the index of the first string with some or all of pszText in it.
 *-------------------------------------------------------------------------*/
{
    for( int i = 0; i < nItem; i++ )
    {
        if( !strcmpi( pszText, pStrHead[i].pszText ) )
            return i;
    }

    if( !fPartial )
        return -1;  // don't search for partials

    int iTextLen = strlen( pszText );

    for( i = 0; i < nItem; i++ )
    {
        if( stristr( pszText, pStrHead[i].pszText, iTextLen ) )
            return i;
    }

    return -1;
}

void StrList::SetKey( const char* pszKey )
/*-------------------------------------------------------------------------*
 * Sets the keyword section to pszKey, eh?
 *-------------------------------------------------------------------------*/
{
    if( pszKey == NULL )
    {
        nItemKwd = iKwdOfs = -1;
        return;
    }

    for( int i = 0; i < nItem; i++ )
    {
        if( pStrHead[i].pszText[0] == '*' &&
            !strcmpi( pszKey, pStrHead[i].pszText ) )
                break;
    }

    if( i == nItem )
        return; // no key found, eh

    ++i;

    // find the next key
    for( int iNext = i; iNext < nItem; iNext++ )
    {
        if( pStrHead[iNext].pszText[0] == '*' )
            break;
    }

    nItemKwd = iNext - i;
    iKwdOfs = i;
}

static int _StrListSort( const void *a, const void *b )
{
    Str *_a = (Str*) a, *_b = (Str*) b;

    return strcmpi( _a->pszText, _b->pszText );
}

void StrList::Sort()
/*-------------------------------------------------------------------------*
 * Sorts the item list in an ascending order.
 *-------------------------------------------------------------------------*/
{
    int iLen = GetCount();
    qsort( pStrHead + _FirstItem(), iLen, sizeof( Str ), _StrListSort );
}
