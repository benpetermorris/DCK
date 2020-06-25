/*-------------------------------------------------------------------------*
 * WINDOW.C:                                                               *
 * Very simple functions for defining, displaying, removing and freeing    *
 * graphics windows.  A window is defined as four coordinates on the       *
 * screen, with a possible screen "box", etc, etc.                         *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

GWINDOW defwn =
{
    0, 0, 0, 0,
    8, 8,
    6, 6,
    WF_POPUP,
    NULL,
    0,
    0,
    0,
    0,
    0,
    0, 0,
    NULL
};

GWINDOW *_act_wnp;

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

GWINDOW *wn_def( short x, short y, short xlen, short ylen, uchar color,
                 uchar bcolor, short style, uchar *title )
/*-------------------------------------------------------------------------*
 * Defines a new window structure.  Does not display it or save its        *
 * underlying image .. just defines it!  OKAY?!                            *
 *-------------------------------------------------------------------------*/
{
    GWINDOW *wnp = (GWINDOW*) memget( sizeof( GWINDOW ) );

    /* Copy defaults in */
    memcpy( wnp, &defwn, sizeof( GWINDOW ) );

    wnp->xlen = xlen;
    wnp->ylen = ylen;

    /* title .. expand the top margin so we can have a pretty bar
     * there and not interfere with other things. */
    if( title )
        wnp->tmarg += _fontheight+2;

    /* Check for centering */
    if( x == Centered )
        x = (sxlen / 2) - (wnp->lmarg + wnp->rmarg + wnp->xlen * _fontwidth) / 2;
    if( y == Centered )
        y = (sylen - (wnp->tmarg + wnp->bmarg + wnp->ylen * _fontheight)) / 3;

    /* X and Y are in absolute screen coordinates.  xlen and ylen are in
     * text character widths and heights, respectively. */
    wnp->x1 = x;
    wnp->y1 = y;

    wnp->x2 = wnp->x1 + (xlen * _fontwidth) + wnp->lmarg + wnp->rmarg;
    wnp->y2 = wnp->y1 + (ylen * _fontheight) + wnp->tmarg + wnp->bmarg;

    /* Calculate save buffer size */
    wnp->sblen = imagesize( wnp->x1, wnp->y1, wnp->x2, wnp->y2 );
    wnp->savebuf = NULL;

    /* Other misc. things */
    wnp->color = color;
    wnp->bcolor = bcolor;
    wnp->title = title;
    wnp->txtcolor = C_WHITE;
    wnp->tcolor = C_BLUE * 256 + C_WHITE;
    wnp->flags |= style;
    wnp->xcenter = ((wnp->x2-wnp->x1)-wnp->lmarg-wnp->rmarg) / 2;
    wnp->ycenter = ((wnp->y2-wnp->y1)-wnp->bmarg-wnp->tmarg) / 2;

    return wnp;
}

void wn_disp( GWINDOW *wnp )
/*-------------------------------------------------------------------------*
 * Displays a window if it's not already displayed.                        *
 *-------------------------------------------------------------------------*/
{
    /* Already there..? */
    if( wnp->flags & WF_ONSCREEN )
        return;

    cursor_put( CSR_ERASE );

    /* Set viewport to entire screen */
    SetViewPort( 0, 0, sxlen, sylen, 1 );

    /* Save underlying image */
    if( wnp->flags & WF_POPUP )
        wnp->savebuf = (uchar*) malloc( wnp->sblen );

    if( !(wnp->savebuf) )   /* Can't save it, d00d! */
        wnp->flags &= ~WF_POPUP;

    if( wnp->flags & WF_POPUP )
        getimage( wnp->x1, wnp->y1, wnp->x2, wnp->y2, wnp->savebuf );

    /* Display a rectangle */
    if( wnp->flags & WF_3D )
    {
        gr_shadrect( wnp->x1, wnp->y1, wnp->x2, wnp->y2, wnp->color );
        gr_shadrect( wnp->x1+1, wnp->y1+1, wnp->x2-1, wnp->y2-1, -1 );
    }
    else
    {
        gr_rect( wnp->x1, wnp->y1, wnp->x2, wnp->y2, wnp->color );
        gr_box( wnp->x1, wnp->y1, wnp->x2, wnp->y2, wnp->bcolor );
    }

    wnp->flags |= WF_ONSCREEN;

    /* Display its title, if any.  If it HAS a title, the top y margin is
     * expanded to disclude the title from the "main window" coordinates.
     */
    if( wnp->title )
    {
        /* This is the only time we use non-window coordinates to write
         * stuff to it */
        gr_rshadrect( wnp->x1+2, wnp->y1+2, wnp->x2-2, wnp->y1 + wnp->tmarg - 3,
                      wnp->tcolor / 256 );
        gr_setcolor( wnp->tcolor % 256 );
        outtextxy( wnp->x1 + wnp->xcenter - textwidth( wnp->title ) / 2,
                      wnp->y1 + (wnp->tmarg / 2) - (_fontheight / 2),
                      wnp->title );
    }

    /* Make it active.. */
    wn_active( wnp );

    cursor_put( CSR_PUT );
}

void wn_active( GWINDOW *wnp )
/*-------------------------------------------------------------------------*
 * Makes unactive the currentw indow and active the passed window          *
 *-------------------------------------------------------------------------*/
{
    if( !wn_isup( wnp ) )
        return;

    /* Like this... */
    SetViewPort( wnp->x1+wnp->lmarg, wnp->y1+wnp->tmarg,
                 wnp->x2-wnp->rmarg, wnp->y2-wnp->bmarg, 1 );

    _act_wnp = wnp;
}

void wn_clear( GWINDOW *wnp )
/*-------------------------------------------------------------------------*
 * Clears a window to its face color                                       *
 *-------------------------------------------------------------------------*/
{
    if( !wn_isup( wnp ) )
        return;

    wn_active( wnp );
    gr_rect( 0, 0, wnp->x2-wnp->x1, wnp->y2-wnp->y1, wnp->color );
}

void wn_undisp( GWINDOW *wnp )
/*-------------------------------------------------------------------------*
 * Undisplays a window:--                                                  *
 * 1) restores its underlying image                                        *
 * 2) says it's not displayed by unsetting the WF_ONSCREEN flag            *
 * 3) frees savebuf memory                                                 *
 *-------------------------------------------------------------------------*/
{
    GWINDOW *__act_wnp = _act_wnp;

    if( !wn_isup( wnp ) )
        return;

    cursor_put( CSR_ERASE );

    if( wnp->flags & WF_POPUP )
    {
        /* Fullscreen viewport for this one */
        SetViewPort( 0, 0, sxlen, sylen, 1 );
        /* Restore.. */
        putimage( wnp->x1, wnp->y1, wnp->savebuf, COPY_PUT );
        memfree( wnp->savebuf );
    }

    wnp->flags &= ~WF_ONSCREEN;

    _act_wnp = NULL;

    if( __act_wnp && __act_wnp != wnp )
        wn_active( __act_wnp );

    cursor_put( CSR_PUT );
}

void wn_free( GWINDOW *wnp )
/*-------------------------------------------------------------------------*
 * Well, duh.                                                              *
 *-------------------------------------------------------------------------*/
{
    if( (wnp->flags & WF_ONSCREEN) && (wnp->flags & WF_POPUP) )
        memfree( wnp->savebuf );    /* bye bye to save buffer */
    /* That's all, folks! */
    memfree( wnp );
}

void _textrect( short x, short y, char *buf, short color )
{
    gr_rect( x, y, x + textwidth( buf ), y + textheight( buf )-1, color );
}

void wn_xytext( GWINDOW *wnp, short x, short y, char *fmt, ... )
/*-------------------------------------------------------------------------*
 * Displays text in the window at text coordinates x, y.                   *
 *-------------------------------------------------------------------------*/
{
    char    buf[256];
    va_list vl;

    if( !wn_isup( wnp ) )
        return;

    wn_active( wnp );

    va_start( vl, fmt );
    vsprintf( buf, fmt, vl );
    va_end( vl );

    /* Center? */
    if( x == Centered )
        x = wnp->xlen / 2 - zstrlen( buf ) / 2;

    if( !(wnp->flags & WF_NOTEXTBG) )
        _textrect( x * _fontwidth, y * _fontheight, buf, wnp->color );

    gr_setcolor( wnp->txtcolor );

    outtextxy( x * _fontwidth, y * _fontheight, buf );
}

void wn_xyputs( GWINDOW *wnp, short x, short y, char *s )
/*-------------------------------------------------------------------------*
 * Displays text in the window at text coordinates x, y.                   *
 *-------------------------------------------------------------------------*/
{
    if( !wn_isup( wnp ) )
        return;

    wn_active( wnp );

    /* Center? */
    if( x == Centered )
        x = wnp->xlen / 2 - zstrlen( s ) / 2;

    if( !(wnp->flags & WF_NOTEXTBG) )
        _textrect( x * _fontwidth, y * _fontheight, s, wnp->color );

    gr_setcolor( wnp->txtcolor );

    outtextxy( x * _fontwidth, y * _fontheight, s );
}

void wn_xyctext( GWINDOW *wnp, short x, short y, uchar color, char *fmt, ... )
/*-------------------------------------------------------------------------*
 * Displays COLORED text ..!  woo!                                         *
 *-------------------------------------------------------------------------*/
{
    char    buf[256];
    va_list vl;

    if( !wn_isup( wnp ) )
        return;

    wn_active( wnp );

    va_start( vl, fmt );
    vsprintf( buf, fmt, vl );
    va_end( vl );

    gr_setcolor( color );

    /* Center? */
    if( x == Centered )
        x = wnp->xlen / 2 - zstrlen( buf ) / 2;

    outtextxy( x * _fontwidth, y * _fontheight, buf );
}

void wn_box( GWINDOW *wnp, short x1, short y1, short xw, short yw,
             short style, uchar color )
/*-------------------------------------------------------------------------*
 * Draws a box in the window using the text color.                         *
 *-------------------------------------------------------------------------*/
{
    if( !wn_isup( wnp ) )
        return;

    wn_active( wnp );

    switch( style )
    {
    case WNB_3D:
        if( color == C_BLACK )
            gr_rshadrect( x1, y1, x1+xw, y1+yw, color );
        else
            gr_shadrect( x1, y1, x1+xw, y1+yw, color );
        break;
    case WNB_FILL:
        gr_rect( x1, y1, x1+xw, y1+yw, color );
        break;
    case WNB_EMPTY:
        gr_box( x1, y1, x1+xw, y1+yw, color );
        break;
    }
}

void wn_line( GWINDOW *wnp, short x1, short y1, short x2, short y2,
              uchar color )
/*-------------------------------------------------------------------------*
 * Draws a line in the window .. duhh                                      *
 *-------------------------------------------------------------------------*/
{
    if( !wn_isup( wnp ) )
        return;

    wn_active( wnp );

    gr_line( x1, y1, x2, y2, color );
}

void wn_stat( GWINDOW *wnp, short n, short nmax )
/*-------------------------------------------------------------------------*
 * Displays a status bar in the window.                                    *
 *-------------------------------------------------------------------------*/
{
    short   maxwidth = 38 * _fontwidth - 2;
    short   nwidth;
    static  unsigned short onwidth;
    static  GWINDOW *ownp;

    if( ownp != wnp )
        onwidth = 0xFFFE;

    nwidth = calc_percent( n, nmax, maxwidth );

    cursor_put( CSR_ERASE );

    if( nwidth < onwidth )
        /* draw box */
        wn_box( wnp, 1 * _fontwidth, 2 * _fontheight, 38 * _fontwidth, _fontheight*15/10,
                WNB_3D, C_BLACK );

    /* do new status */
    wn_box( wnp, 1 * _fontwidth+1, 2 * _fontheight+1, nwidth, _fontheight*15/10-2,
            WNB_FILL, C_LIGHTBLUE );

    cursor_put( CSR_PUT );

    ownp = wnp;
    onwidth = nwidth;
}

void wn_statstr( GWINDOW *wnp, char *text )
{
    /* Display what we're doing to the user */
    wn_box( wnp, 1*_fontwidth, 1*_fontheight, 38*_fontwidth, _fontheight,
            WNB_FILL, wnp->color );
    wn_xytext( wnp, 1, 1, text );
}
