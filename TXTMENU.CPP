/*-------------------------------------------------------------------------*
 * TXTMENU.C:                                                              *
 * Functions for implementing text menus.                                  *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include <conio.h>

#define BBCLR_BDR       WHITE_BLUE+8
#define BBCLR_TITLE     WHITE_CYAN+8
#define BBCLR_SELITEM   BLACK_WHITE
#define BBCLR_ITEM      WHITE_BLUE
#define BBCLR_SELCH     BROWN_WHITE+8

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#if 0
static uchar far *__scr = (uchar far *) 0xB8000000L;
#endif

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static void __cputs( short x, short y, char *text, short width, uchar color )
{
    char buf[80], fmt[10], *p;

    if( width != -1 )
    {
        sprintf( fmt, " %c-%02ds ", '%', width );
        sprintf( buf, fmt, text );
        p = buf;
    }
    else p = text;

    gotoxy( x, y );
    textattr( color );
    cputs( p );
#if 0
    uchar   far *sptr = __scr + (x * 2) + (y * 160);

    while( *text )
    {
        if( *text == '~' )
        {
            ++text;
            *sptr++ = *text++;
            *sptr++ = color == BBCLR_SELITEM ? BBCLR_SELITEM : BBCLR_SELCH;
        }
        else
        {
            *sptr++ = *text++;
            *sptr++ = color;
        }
    }
#endif
}

static void __rect( short x, short y, short xw, short yw, uchar *title )
{
    uchar   *bdrbuf = "Õ¸Ô¾Í³µÆ";
    uchar   buf[80], *bptr;
    short   i, tx;

    /* Draw top left */
    tx = 0;
    buf[0] = bdrbuf[0];
    buf[1] = bdrbuf[6];
    buf[2] = 0;
    __cputs( tx+x, y, buf, -1, BBCLR_BDR );
    tx += 2;

    /* Draw title */
    __cputs( tx+x, y, title, -1, BBCLR_TITLE );
    tx += strlen( title );

    /* Draw top right */
    buf[0] = bdrbuf[7];
    bptr = buf+1;
    i = tx+1;
    while( i++ < xw-1 )
        *bptr++ = bdrbuf[4];
    *bptr++ = bdrbuf[1];
    *bptr = 0;
    __cputs( tx+x, y, buf, -1, BBCLR_BDR );

    /* Draw mid parts */
    buf[0] = bdrbuf[5];
    for( tx = 1, bptr = buf+1; tx < xw-1; tx++ )
        *bptr++ = 32;
    *bptr++ = bdrbuf[5];
    *bptr = 0;

    for( i = 1; i < yw-1; i++ )
        __cputs( x, y+i, buf, -1, BBCLR_BDR );

    /* Draw bottom */
    buf[0] = bdrbuf[2];
    for( tx = 1, bptr = buf+1; tx < xw-1; tx++ )
        *bptr++ = bdrbuf[4];
    *bptr++ = bdrbuf[3];
    *bptr = 0;

    __cputs( x, y+yw-1, buf, -1, BBCLR_BDR );
}

short bar_menu( char **sel, char *title, short start, short x, short y )
{
    short   numitem, curitem = start, lastitem;
    short   ch, tmp, width = 0, i, rvl;
    short   startx = wherex(), starty = wherey();
    uchar   *savebuf = (uchar*) malloc( 4000 );

    gettext( 1, 1, 80, 25, savebuf );

    /* Find the number of elements */
    for( i = 0; ; i++ )
    {
        if( !sel[i] )
            break;
        if( (tmp = strlen( sel[i] )) > width )
            width = tmp;
    }

    _setcursortype( _NOCURSOR );

    numitem = i;

    /* title wider than any items ?? */
    if( (tmp = strlen( title )) > width )
        width = tmp + 4;

    if( x == Centered )
        x = 40 - (width+4)/2;
    if( y == Centered )
        y = 13 - (numitem+2)/2;

    __rect( x, y, width+4, numitem+2, title );

    for( i = 0; i < numitem; i++ )
        __cputs( x+1, y+1+i, sel[i], width, i == curitem ? BBCLR_SELITEM : BBCLR_ITEM );

    lastitem = curitem;

    /* Main loop */
    while( 1 )
    {
        /* draw the current item and last item ok */
        if( lastitem != curitem )
        {
            __cputs( x+1, y+1+lastitem, sel[lastitem], width, BBCLR_ITEM );
            __cputs( x+1, y+1+curitem, sel[curitem], width, BBCLR_SELITEM );
        }

        ch = keyget();

        lastitem = curitem;

        switch( ch )
        {
        case KEY_UP:
            if( curitem )
                --curitem;
            else
                curitem = numitem-1;
            break;
        case KEY_DOWN:
            if( curitem < numitem-1 )
                curitem++;
            else
                curitem = 0;
            break;
        case KEY_END:
            curitem = numitem-1;
            break;
        case KEY_HOME:
            curitem = 0;
            break;
        case KEY_ENTER:
            rvl = curitem;
            goto done;
        case KEY_ESC:
            rvl = -1;
            goto done;
        }
    }

done:
    textattr( 7 );
    puttext( 1, 1, 80, 25, savebuf );
    memfree( savebuf );
    _setcursortype( _NORMALCURSOR );
    gotoxy( startx, starty );
    return rvl;
}
