/*-------------------------------------------------------------------------*
 * MAPDRAW.C:                                                              *
 * Updates the map at the current view coordinates with the current zoom   *
 * factor, etc, etc.                                                       *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#pragma opt -O2

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

/* Colors */
#define C_TYPNOTAG  C_LIGHTGREEN
#define C_TYP       C_LIGHTMAGENTA
#define C_ILINE_OM  C_GRAY1
#define C_PLINE_OM  C_GRAY3
#define C_VERTEX_OM C_LIGHTBLUE
#define C_VERTEX    C_GRAY1
#define C_MBLOCK    C_GREEN
#define C_SBLOCK    C_RED

inline void drawonevertex( VERTEX *vptr )
{
    short sx1, sy1;

    gr_setcolor( (editmode == MODE_LINE ? C_VERTEX : C_VERTEX_OM) );

    if( vptr->x != -32000 )
    {
        sx1 = _mapx2scr( vptr->x-xstart );
        sy1 = _mapy2scr( vptr->y-ystart );
        bar( sx1-1, sy1-1, sx1+1, sy1+1 );
    }
}

inline void setlinecolor( LINEDEF *ldp )
/*-------------------------------------------------------------------------*
 * Sets the current drawing color based on a line's data
 *-------------------------------------------------------------------------*/
{
    if( editmode == MODE_SECTOR || editmode == MODE_LINE )
    {
        if( ldp_isimpass( ldp ) )
            setcolor( C_ILINE );
        else if( ldp->attribs & LA_MBLOCKER )
            setcolor( C_MBLOCK );
        else if( ldp->attribs & LA_SBLOCK )
            setcolor( C_SBLOCK );
        else
            setcolor( C_PLINE );

        if( editmode == MODE_SECTOR && fHexen )
        {
            if( (ldp->rsdef != -1 && sectorlist[sidelist[ldp->rsdef].sector].sectag > 0) ||
                (ldp->lsdef != -1 && sectorlist[sidelist[ldp->lsdef].sector].sectag > 0) )
                setcolor( C_LIGHTCYAN );
        }
        else
        {
            if( lSecTag( ldp ) && lineneedstag( ldp->special ) )
                setcolor( C_TYP );
            else if( ldp->special )
                setcolor( C_TYPNOTAG );
        }
    }
    else
    {
        if( ldp_isimpass( ldp ) )
            setcolor( C_ILINE_OM );
        else
            setcolor( C_PLINE_OM );
    }

}

void drawoneline( short lineno )
{
    /* guess. */
    short   sx1, sy1, sx2, sy2;
    LINEDEF *ldp = &linelist[lineno];
    VERTEX  *v1, *v2;

    v1 = &vlist[ldp->vfrom];
    v2 = &vlist[ldp->vend];

    gr_setwritemode( SWM_COPY );
    gr_setlinestyle( SLS_SOLID, 1 );

    setlinecolor( ldp );

    /* Scale the coordinates into screen coordinates */
    sx1 = _mapx2scr( v1->x-xstart );
    sx2 = _mapx2scr( v2->x-xstart );
    sy1 = _mapy2scr( v1->y-ystart );
    sy2 = _mapy2scr( v2->y-ystart );
    line( sx1, sy1, sx2, sy2 );

    drawonevertex( v1 );
    drawonevertex( v2 );
}

void map_update( bool fDraftMode )
/*-------------------------------------------------------------------------*
 * Updates the map at the current view coordinates with the current zoom   *
 * factor.                                                                 *
 *-------------------------------------------------------------------------*/
{
    short   i, m, sec;
    LINEDEF *ldptr;
    VERTEX  *vptr;
    VERTEX  *v1, *v2;
    short   x1, y1, x2, y2;
    short   sx1, sy1, sx2, sy2, color;
    extern  short current_op, xend, yend;
    static  int iLastXstart, iLastYstart;
    bool    fHighlight = (!fDraftMode && !tog_drag && current_op != OP_TAGSET );

    /* Set the viewport */
    SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen );

    /* Update the grid if we want to */
    if( tog_grid && !fDraftMode )
        grid_update();

    gr_setwritemode( SWM_COPY );
    gr_setlinestyle( SLS_SOLID, 1 );

    if( !nvertex && !fDraftMode && editmode == MODE_THING )
        goto __dothings;

    if( fDraftMode )
        setcolor( C_GRAY3 );

/*-------------------------------------------------------------------------*
 * Draw lines                                                              *
 *-------------------------------------------------------------------------*/
    /* First, draw things if we're not in THING mode so they don't
     * obscure the lines and vertices */
    if( !fDraftMode && editmode != MODE_THING )
    {
        for( i = 0; i < nthing; i++ )
        {
            /* Draw it */
            sc_drawthing( i );
        }
    }

    for( i = 0, ldptr = linelist; i < nline; i++, ldptr++ )
    {
        v1 = &vlist[ldptr->vfrom];
        v2 = &vlist[ldptr->vend];

        x1 = v1->x; x2 = v2->x;
        y1 = v1->y; y2 = v2->y;

        if( (x2 > xend && x1 > xend) ||
            (x2 < xstart && x1 < xstart) ||
            (y2 > yend && y1 > yend) ||
            (y2 < ystart && y1 < ystart) )
            continue;

        if( !fDraftMode )
            setlinecolor( ldptr );

        /* Scale the coordinates into screen coordinates */
        sx1 = _mapx2scr( x1-xstart );
        sx2 = _mapx2scr( x2-xstart );
        sy1 = _mapy2scr( y1-ystart );
        sy2 = _mapy2scr( y2-ystart );
        line( sx1, sy1, sx2, sy2 );

        if( fHighlight )
        {
            /* highlight tagged object(s) */
            if( editmode == MODE_LINE )
            {
                /* Highlight the marked objects */
                if( obj_ismarked( i ) )
                    obj_highlight( MODE_LINE, i, COLOR_MARKED );
            }
            else if( editmode == MODE_SECTOR )
            {
                /* Highlight the marked objects */
                m = linelist[i].rsdef;
                sec = sidelist[m].sector;
                if( m >= 0 && sec >= 0 && obj_ismarked( sec ) )
                    obj_highlight( MODE_LINE, i, COLOR_MARKED );
                m = linelist[i].lsdef;
                sec = sidelist[m].sector;
                if( m >= 0 && sec >= 0 && obj_ismarked( sec ) )
                    obj_highlight( MODE_LINE, i, COLOR_MARKED );
            }
        }
    }

/*-------------------------------------------------------------------------*
 * Draw vertices                                                           *
 *-------------------------------------------------------------------------*/
    if( editmode != MODE_THING && !fDraftMode )
    {
        gr_setcolor( color = (editmode == MODE_LINE ? C_VERTEX : C_VERTEX_OM) );

        for( i = 0, vptr = vlist; i < nvertex; i++, vptr++ )
        {
            x1 = vptr->x;
            y1 = vptr->y;

            if( !inrange( x1, xstart, xend ) ||
                !inrange( y1, ystart, yend ) )
                continue;

            if( vptr->x != -32000 )
            {
                sx1 = _mapx2scr( x1-xstart );
                sy1 = _mapy2scr( y1-ystart );
                bar( sx1-1, sy1-1, sx1+1, sy1+1 );
            }
            if( editmode == MODE_LINE && marklist[i] )
            {
                obj_highlight( MODE_VERTEX, i, COLOR_MARKED );
                gr_setcolor( color );
            }
        }
    }

/*-------------------------------------------------------------------------*
 * Draw things                                                             *
 *-------------------------------------------------------------------------*/
__dothings:
    if( editmode == MODE_THING && !fDraftMode )
    {
        for( i = 0; i < nthing; i++ )
        {
            /* Draw it */
            sc_drawthing( i );

            /* Highlight it */
            if( obj_ismarked( i ) )
                obj_highlight( MODE_THING, i, COLOR_MARKED );
        }

        if( torigin_x != -1 )
        {
            sx1 = _mapx2scr( torigin_x - xstart );
            sy1 = _mapy2scr( torigin_y - ystart );

            /* place thing origin */
            gr_setcolor( C_WHITE );
            gr_drawcircle( sx1, sy1, 3 );
            gr_setcolor( C_LIGHTGREEN );
            gr_drawcircle( sx1, sy1, 8 );
            gr_setcolor( C_GREEN );
            gr_drawcircle( sx1, sy1, 13 );
        }
    }

__abortdraw:
}

void grid_update( void )
/*-------------------------------------------------------------------------*
 * Updates the grid using the current grid size 'gridstep' and displays it.                                                            *
 *-------------------------------------------------------------------------*/
{
    short   x, y, x1, x2, color, y1, y2;
    short   nxstart, nxend, nystart, nyend;
    short   dopoints = 1; // _mapx2scr( gridstep ) >= 12;
    short   nxgrid = 0, nygrid = 0, *xgp, *ygp;
    static  short xgrids[1024], ygrids[1024];

    // set the grid colour
    gr_setcolor( tog_grid == 1 ? C_GRID : guito256[WHITE2+1][3]  );

    if( tog_grid == 2 )
        dopoints = 0;

    /* don't display it if it's SOOOOOO SMAAAALL */
    if( _mapx2scr( gridstep ) <= 4 )
        return;

    nxstart = xstart - (xstart % gridstep) - gridstep * 2;
    nystart = ystart - (ystart % gridstep) - gridstep * 2;
    nxend = nxstart + xscale + gridstep * 4;
    nyend = ystart + yscale + gridstep * 4;

    gr_setlinestyle( SLS_SOLID, 1 );

    y1 = _mapy2scr( nystart - ystart );
    y2 = _mapy2scr( nyend - ystart );

    /* Draw it */
    for( x = nxstart; x < nxend; x += gridstep )
        if( x >= xstart )
        {
            x1 = _mapx2scr( x - xstart );
            if( dopoints )
                xgrids[nxgrid++] = x1;
            if( tog_grid == 2 )
                line( x1, y1, x1, y2 );
        }

    x1 = _mapx2scr( nxstart - xstart );
    x2 = _mapx2scr( nxend - xstart );

    /* Draw it */
    for( y = nystart; y < nyend; y += gridstep )
        if( y >= ystart )
        {
            y1 = _mapy2scr( y - ystart );
            if( dopoints )
                ygrids[nygrid++] = y1;
            if( tog_grid == 2 )
                line( x1, y1, x2, y1 );
        }

    if( dopoints && _mapx2scr( gridstep ) > 7 )
    {
        color = C_GRAY2;

        for( x = 0, xgp = xgrids; x < nxgrid; x++, xgp++ )
            for( y = 0, ygp = ygrids; y < nygrid; y++, ygp++ )
                putpixel( *xgp, *ygp, color );
    }
}

void maptoglob( short *dx, short *dy, short mx, short my )
/*-------------------------------------------------------------------------*
 * Converts map screen coordinates to global screen coordinates            *
 *-------------------------------------------------------------------------*/
{
    *dx = mx + mxpstart;
    *dy = my + mypstart;
}

void globtomap( short *dx, short *dy, short gx, short gy )
/*-------------------------------------------------------------------------*
 * Converts global coordinates to map screenc oordinates                   *
 *-------------------------------------------------------------------------*/
{
    *dx = gx - mxpstart;
    *dy = gy - mypstart;
}
