#include "dck.h"
#pragma hdrstop

short purgelist( uchar *base, size_t size, short nelem, short *uselist );

#pragma inline +memcpy

static  LINEDEF *line_list;
static  VERTEX *vertex_list, *lvertex_list;
static  uchar lastfn[128], lasttag[10];

void FreePreviewData()
{
    if( lvertex_list )
    {
        memfree( lvertex_list );
        lvertex_list = NULL;
    }
    if( vertex_list )
    {
        memfree( vertex_list );
        vertex_list = NULL;
    }
    if( line_list )
    {
        memfree( line_list );
        line_list = NULL;
    }

    lastfn[0] = lasttag[0] = 0;
}

short preview( WAD *wptr, uchar *_ltag, short xpos, short ypos,
               short width, short height, short scale, short xofs, short yofs )
{
    static  short line_num, vertex_num;
    static  short minx, maxx, miny, maxy;

    LINEDEF *ldptr;
    VERTEX  *vptr;
    long    *vlp, *lvp;
    ulong   size;
    short   *vused;
    short   i, m;
    char    oldltag[10];
    struct  viewporttype vp;
    short   sx1, sy1, sx2, sy2, myscale, lc;
    short   lowc = C_GRAY3, highc = C_GRAY1;
    bool    fHexen = FALSE;

    if( wptr == NULL )
    {
        strcpy( lastfn, "" );
        strcpy( lasttag, "" );
        return 0;     /* to make sure it does it */
    }

    getviewsettings( &vp );

    strcpy( oldltag, szCurMapTag );
    strcpy( szCurMapTag, _ltag );

    if( strcmp( lastfn, wptr->fn ) || strcmp( _ltag, lasttag ) )
    {
        FreePreviewData();
        strcpy( lastfn, wptr->fn );
        strcpy( lasttag, _ltag );
    }
    else
        goto _doprev;

    if( wad_findCurMap( wptr, "BEHAVIOR" ) )
    {
        fHexen = TRUE;
    }

    wad_findCurMap( wptr, "LINEDEFS" );

    if( fHexen )
    {
        line_list = (LINEDEF*) dirload( wptr, &size, NULL );
        line_num = size / sizeof( LINEDEF );
    }
    else
    {
        DoomLine* pDoomLines = (DoomLine*) dirload( wptr, &size, NULL );
        line_num = size / sizeof( DoomLine );

        line_list = (LINEDEF*) memget( line_num * sizeof( LINEDEF ) );

        for( i = 0; i < line_num; i++ )
        {
            memcpy( &line_list[i], &pDoomLines[i], 4 );
            line_list[i].lsdef = pDoomLines[i].lsdef;
        }

        memfree( pDoomLines );
    }

    wad_findCurMap( wptr, "VERTEXES" );
    lvertex_list = (VERTEX*) dirload( wptr, &size, NULL );
    vertex_num = size / sizeof( VERTEX );
    vertex_list = (VERTEX*) memget( vertex_num * sizeof( VERTEX ) );

    vused = (short*) memget( vertex_num * sizeof( *vused ) );
    memset( vused, 0, vertex_num * sizeof( *vused ) );

    minx = miny = 32700;
    maxx = maxy = -32700;

    for( ldptr = line_list, i = 0; i < line_num; ldptr++, i++ )
    {
        vused[ldptr->vfrom] = 1;
        vused[ldptr->vend] = 1;
    }

    for( lvp = (long*) vptr = (long*) lvertex_list, vlp = (long*) vertex_list,
         i = 0, m = 0; i < vertex_num; i++, vptr++, lvp++ )
    {
        if( vused[i] )
        {
            if( vptr->x < minx )
                minx = vptr->x;
            if( vptr->x > maxx )
                maxx = vptr->x;
            if( vptr->y < miny )
                miny = vptr->y;
            if( vptr->y > maxy )
                maxy = vptr->y;

            *vlp++ = *lvp;

            vused[i] = m++;
        }
    }

    /* update the vertex numbers in the linedefs */
    for( i = 0, ldptr = line_list; i < line_num; i++, ldptr++ )
    {
        ldptr->vfrom = vused[ldptr->vfrom];
        ldptr->vend = vused[ldptr->vend];
    }

    memfree( vused );

_doprev:
    asm push myplen mxplen xstart ystart xscale

    /* temporarily */
    myplen = height;
    mxplen = width;

    myscale = (unsigned short) (((long)(maxx - minx + 50) * 500L) / (long)scale);

    setscale( myscale );

    xstart = ((maxx + minx) / 2) - xscale / 2 + xofs;
    ystart = ((maxy + miny) / 2) - yscale / 2 + yofs;

    setcolor( lc = highc );

    setlinestyle( SOLID_LINE, 0, 1 );

    SetViewPort( vp.left + xpos, vp.top + ypos,
                 vp.left + xpos + width, vp.top + ypos + height, 1 );

    /* draws every other line ! */
    for( i = 0, ldptr = line_list; i < line_num; i ++, ldptr ++ )
    {
        /* Scale the coordinates into screen coordinates */
        sx1 = _mapx2scr( vertex_list[ldptr->vfrom].x - xstart );
        sx2 = _mapx2scr( vertex_list[ldptr->vend].x - xstart );
        sy1 = _mapy2scr( vertex_list[ldptr->vfrom].y - ystart );
        sy2 = _mapy2scr( vertex_list[ldptr->vend].y - ystart );

        if( ldp_isimpass( ldptr ) )
        {
            if( lc != highc )
                setcolor( lc = highc );
        }
        else if( lc != lowc )
            setcolor( lc = lowc );

        line( sx1, sy1, sx2, sy2 );
    }

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom, 1 );
    asm pop xscale ystart xstart mxplen myplen
    setscale( xscale );
    strcpy( szCurMapTag, oldltag );

    return 1;
}
