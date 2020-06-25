/*-------------------------------------------------------------------------*
 * STRTABLE.C:
 * Functions for maintaining string tables - they're kinda like memory
 * files.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *
 * Code begins here
 *
 *-------------------------------------------------------------------------*/

STRTABLE *strtbl_init( uchar *name, short maxrow, short maxcol )
{
    STRTABLE *s = (STRTABLE*) memget( sizeof( STRTABLE ) );
    short    i;
    uchar    *p;

    s->maxitem = maxrow;
    s->maxcol = maxcol;
    s->nitem = 0;
    s->kwdofs = 0;
    s->kwdlen = 0;
    strcpy( s->name, name );
    s->stp = (STR*) memget( s->maxitem * sizeof( STR ) );

    _memstr = "String Table Buffer";
    s->strbuf = p = (uchar*) memget( (maxcol + 1) * s->maxitem );

    for( i = 0; i < s->maxitem; i++, p += (maxcol + 1) )
    {
        s->stp[i].str = p;
        s->stp[i].ident = 0;
    }

    return s;
}

void strtbl_clear( STRTABLE *st )
{
    short   idx;
    STR     *stp = st->stp;

    for( idx = 0; idx < st->maxitem; idx++ )
    {
        memset( stp->str, 0, st->maxcol );
        stp->ident = 0;
    }

    st->nitem = 0;
}

void strtbl_del( STRTABLE *st, short item )
{
    short   idx = item;
    STR     *stp = st->stp + idx;
    uchar   *p = stp->str;

    for( ; idx < st->nitem; idx++, stp++ )
        memcpy( stp, stp+1, sizeof *stp );

    stp->str = p;

    --st->nitem;
}

short strtbl_len( STRTABLE *st )
/*-------------------------------------------------------------------------*
 * Returns the length in items of a string table.. takes into account
 * keyworded sections and all that.
 *-------------------------------------------------------------------------*/
{
    if( st->kwdlen )    return st->kwdlen;
    else                return st->nitem;
}

void strtbl_free( STRTABLE *st )
{
    short   idx;

    memfree( st->strbuf );
    memfree( st->stp );
    memfree( st );
}

void strtbl_kwdset( STRTABLE *st, uchar *kwd )
{
    short n;

    if( kwd )
    {
        st->kwdofs = 0;
        st->kwdlen = 0;

        n = strtbl_find( st, kwd );

        if( n != -1 )
        {
            st->kwdofs = ++n;

            /* find the next keyword.. set the length of the section */
            for( ; n < st->nitem; n++ )
            {
                if( *(st->stp[n].str) == '*' )
                    break;
            }

            st->kwdlen = n - st->kwdofs;
        }
    }
    else
    {
        st->kwdofs = 0;
        st->kwdlen = 0;
    }
}

uchar *strtbl_get( STRTABLE *st, uchar *s, short pos )
/*-------------------------------------------------------------------------*
 * get the text from 'pos' in the string table.. takes notice of keywords.
 *-------------------------------------------------------------------------*/
{
    uchar   *ptr;

    if( pos >= st->kwdlen && st->kwdlen || pos < 0 )
        return NULL;

    /* get the text */
    ptr = st->stp[pos+st->kwdofs].str;

    if( s == NULL )
        return ptr;

    else return strcpy( s, ptr );
}

void strtbl_add( STRTABLE *st, uchar *s, short ident )
{
    short   i = st->nitem;

    ++st->nitem;

    if( st->nitem > st->maxitem )
        error( "string table '%s' is full", st->name );

    if( strlen( s ) > st->maxcol )
        error( "item too long for string table '%s':\n\"%s\"", st->name, s );

    strcpy( st->stp[i].str, s );
    st->stp[i].ident = ident;
}

void strtbl_rpl( STRTABLE *st, short pos, uchar *s, short ident )
{
    if( strlen( s ) > st->maxcol )
        error( "item too long for string table '%s':\n\"%s\"", st->name, s );

    strcpy( st->stp[pos].str, s );
    st->stp[pos].ident = ident;
}

char *strtbl_idtostr( STRTABLE *st, short id )
{
    short   idx;
    STR     *stp = st->stp;

    for( idx = 0; idx < st->nitem; idx++, stp++ )
    {
        if( stp->ident == id )
            return stp->str;
    }

    return NULL;
}

short strtbl_findid( STRTABLE *st, short id )
{
    short   idx;
    STR     *stp = st->stp;

    for( idx = 0; idx < st->nitem; idx++, stp++ )
    {
        if( stp->ident == id )
            return idx;
    }

    return -1;
}

short strtbl_strtoid( STRTABLE *st, uchar *s )
{
    short   idx;
    STR     *stp = st->stp;

    for( idx = 0; idx < st->nitem; idx++, stp++ )
    {
        if( !strcmpi( s, stp->str ) )
            return stp->ident;
    }

    return -1;
}

short strtbl_find( STRTABLE *st, uchar *s )
{
    short   idx;
    STR     *stp = st->stp + st->kwdofs;
    short   len = strtbl_len( st );

    for( idx = st->kwdofs; idx < len; idx++, stp++ )
    {
        if( !strcmp( s, stp->str ) )
            return idx;
    }

    stp = st->stp + st->kwdofs;

    for( idx = st->kwdofs; idx < len; idx++, stp++ )
    {
        if( striinc( s, stp->str ) )
            return idx;
    }

    return -1;
}

static int _stsort_fp( const void *a, const void *b )
{
    STR *_a = (STR*) a, *_b = (STR*) b;

    return strcmpi( _a->str, _b->str );
}

void strtbl_sort( STRTABLE *st )
/*-------------------------------------------------------------------------*
 * Sorts the string table.  pays attention to keywords.
 *-------------------------------------------------------------------------*/
{
    short   len = strtbl_len( st );

    qsort( st->stp + st->kwdofs, len, sizeof( STR ), _stsort_fp );
}

short strtbl_findnokey( STRTABLE *st, uchar *s )
/*-------------------------------------------------------------------------*
 * Same as strtbl_find, but doesn't pay attention to keyworded sections.
 * Ignores keyworded LINES, though..
 *-------------------------------------------------------------------------*/
{
    short   idx;
    STR     *stp = st->stp;

    for( idx = 0; idx < st->nitem; idx++, stp++ )
    {
        if( stp->str[0] == '*' )
            continue;

        if( !strcmp( s, stp->str ) )
            return idx;
    }

    stp = st->stp;

    for( idx = 0; idx < st->nitem; idx++, stp++ )
    {
        if( stp->str[0] == '*' )
            continue;

        if( striinc( s, stp->str ) )
            return idx;
    }

    return -1;
}
