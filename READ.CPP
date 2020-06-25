/*-------------------------------------------------------------------------*
 * READ.C:                                                                 *
 * Zis ees a file reader.                                                  *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

// #pragma inline

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code                                                                    *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#define TBXSTART 10
#define TBYSTART 10

void scrollarea( SRECT *r, short yscroll )
{
    unsigned bufsiz = imagesize( r->left, r->top, r->right, r->bottom );
    char     *buf = new char[bufsiz];

    if( yscroll > 0 )
    {
        /* scroll up */
        getimage( r->left, r->top+yscroll, r->right, r->bottom, buf );
        putimage( r->left, r->top, buf, COPY_PUT );
    }
    else
    {
        /* scroll down */
        getimage( r->left, r->top, r->right, r->bottom+yscroll, buf );
        putimage( r->left, r->top-yscroll, buf, COPY_PUT );
    }

    delete buf;
}

short hctoi( short ch )
{
    char *hcp = "0123456789ABCDEF";
    return (short) (strchr( hcp, ch ) - hcp);
}

void wn_xyputs( GWINDOW *wnp, short x, short y, char *s );

short readtext( char *title, char **text, short numlines, short ydisp )
{
    short   ch, redoall = 1, curline = 0, i, xlen = 0, tmp, x, y, n, scroll;
    short   fg, bg;
    short   lastline = 0;
    SRECT    viewarea;
    GWINDOW *wnp;
    char    buf[128], *p;
    struct  viewporttype vp;

    getviewsettings( &vp );

    /* Get the X length */
    for( i = 0; i < numlines; i++ )
    {
        tmp = zstrlen( text[i] );

        if( tmp > xlen )
            xlen = tmp;
    }

    wnp = wn_def( Centered, Centered, xlen+2, ydisp+2, C_GRAY3, C_GRAY1,
                  WF_POPUP | WF_3D | WF_NOTEXTBG, title );
    wn_disp( wnp );

    /* draw a box */
    viewarea.left = _fontwidth-2;
    viewarea.right = 3 + xlen * _fontwidth;
    viewarea.top = _fontheight-2;
    viewarea.bottom = 4 + ydisp * _fontheight;

    cursor_put( CSR_ERASE );
    wn_box( wnp, viewarea.left-4, viewarea.top, viewarea.right+8, viewarea.bottom,
            WNB_3D | WF_NOTEXTBG, C_BLACK );
    cursor_put( CSR_PUT );

    SRECT viewrect = viewarea;

    /* originally the bottom right coords were width and height */
    viewarea.right += viewarea.left;
    viewarea.bottom += viewarea.top;

    viewarea.left --;
    viewarea.top --;

/*-------------------------------------------------------------------------*
 * Update loop                                                             *
 *-------------------------------------------------------------------------*/

    wnp->txtcolor = C_WHITE;
    wnp->color = C_BLACK;

    *(buf+xlen) = 0;
    *(buf+xlen+1) = 0;

    while( 1 )
    {
        cursor_put( CSR_ERASE );

        if( curline < 0 )
            curline = 0;

/* Redraw all text?                                                        */
        if( redoall )
        {
            wn_box( wnp, viewrect.left-4, viewrect.top, viewrect.right+8,
                    viewrect.bottom, WNB_3D, C_BLACK );

            for( i = curline; i < curline + ydisp && i < numlines; i++ )
            {
                p = text[i];

                if( *p == '|' )
                {
                    fg = *++p;
                    fg = guito256[hctoi( fg )][3];
                    bg = *++p;
                    bg = guito256[hctoi( bg )][3];
                    p++;
                    wnp->color = bg ? bg : C_BLACK;
                    wnp->txtcolor = fg;
                }
                else
                {
                    wnp->color = C_BLACK;
                    wnp->txtcolor = C_WHITE;
                }

                //memset( buf, 32, xlen );
                //memcpy( buf, p, zstrlen( p ) );
                wn_xyputs( wnp, 1, 1+(i-curline), p );
            }
        }
#if 0
/* this scrolls the area.. for some reason it's really slow */

        else if( 0 )
        {
            if( lastline != curline )
            {
                scroll = (curline-lastline) * _fontheight;
                scrollarea( &viewarea, scroll );

                if( scroll < 0 )
                    n = curline;
                else
                    n = curline + ydisp - 1;

                memset( buf, 32, xlen );
                memcpy( buf, text[n], zstrlen( text[n] ) );

                wn_xytext( wnp, 1, 1+(n-curline), buf );

                lastline = curline;
            }
        }
#endif
        cursor_put( CSR_PUT );

/* Get input                                                               */
        while( 1 )
        {
            if( keycheck() )
            {
                ch = keyget();
                break;
            }

            if( mouse_event( &x, &y ) == MOUSE_RUP )
            {
                ch = KEY_ESC;
                break;
            }
        }

        if( ch == KEY_ENTER || ch == KEY_ESC )
            break;

        redoall = 1;

        switch( ch )
        {
        case KEY_DOWN:
            curline ++;
            if( curline + ydisp > numlines )
            {
                curline--;
                redoall = 0;
            }
            break;
        case KEY_UP:
            curline --;
            if( curline < 0 )
            {
                curline = 0;
                redoall = 0;
            }
            break;
        case KEY_HOME:
            if( curline != 0 )
                curline = 0;
            else
                redoall = 0;
            break;
        case KEY_END:
            if( curline + ydisp == numlines )
                redoall = 0;
            else
                curline = numlines-ydisp;
            break;
        case KEY_PGUP:
            if( curline == 0 )
                redoall = 0;
            else
            {
                curline -= ydisp;
                if( curline < 0 )
                    curline = 0;
            }
            break;
        case KEY_PGDN:
            if( curline + ydisp == numlines )
                redoall = 0;
            else
            {
                curline += ydisp;
                if( curline + ydisp > numlines )
                    curline = numlines-ydisp;
            }
            break;
        default:
            redoall = 0;
            break;
        }
    }

    wn_undisp( wnp );
    wn_free( wnp );

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom, 1 );

    return 0;
}

short countlines( char **text )
/*-------------------------------------------------------------------------*
 * Counts the number of lines in 'text' and returns.                       *
 *-------------------------------------------------------------------------*/
{
    short i = 0;

    while( text[i] && i < 4096 )    i++;

    return i == 4096 ? -1 : i;
}
