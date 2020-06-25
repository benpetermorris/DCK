/*-------------------------------------------------------------------------*
 * SBAR.C:                                                                 *
 * Functions for maintaining the status bar.                               *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

// #define TOGTEST

#define NUM_MSGS    3
#define STRINGSIZE  384

#define SBXPAD      4

#define SB_MAXSTR   128

#define C_SBAR      C_GRAY3
#define C_SBHIGH    C_GRAY1
#define C_SBLOW     C_GRAY4
#define C_TEXT      WHITE

static NODEPTR      sbar_list;

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

void sbs_data( stBarp sbp, char *data )
{
    if( sbp->data == NULL )
        sbp->data = (uchar*) memget( 1024 );

    strcpy( sbp->data, data );
}

void sbs_string( stBarp sbp, char *fmt, ... )
/*-------------------------------------------------------------------------*
 * Sets the data string for a status bar
 *-------------------------------------------------------------------------*/
{
    char    *buf = new char[1024];
    va_list vl;

    va_start( vl, fmt );
    vsprintf( buf, fmt, vl );
    va_end( vl );

    if( !(sbp->data) || strcmp( sbp->data, buf ) )
    {
        /* normal handling */
        if( !sbp->update )
            sbp->update = 1;
        sbs_data( sbp, buf );
    }

    delete buf;
}

void sb_clearitems( stBarp sbp )
{
    NODEPTR ndp = NodeFirst( sbp->itemlist );
    sbItemp itemp;

    sb_setupdate( sbp, 1 );

    while( ndp != sbp->itemlist )
    {
        itemp = (sbItemp) ndp->data;

        if( si_getflag( itemp, SIF_NOCLEAR ) )
            goto _next;

        if( itemp->type == SBI_TOGGLE )
            si_onoff( itemp, 0 );
        if( itemp->type == SBI_STRING )
        {
            si_clr( itemp, C_TEXT );
            si_str( itemp, "" );
        }

_next:
        ndp = NodeNext( ndp );
    }
}

void si_str( sbItemp item, char *fmt, ... )
/*-------------------------------------------------------------------------*
 * Sets a status bar item's string.
 *-------------------------------------------------------------------------*/
{
    va_list vl;
    char    buf[SB_MAXSTR*2];

    va_start( vl, fmt );
    vsprintf( buf, fmt, vl );
    va_end( vl );

    /* only copy&update if it's not the same as the current string */
    if( strcmp( buf, item->is_stp ) )
    {
        item->update = 1;
        /* check length of string */
        if( strlen( buf ) > SB_MAXSTR )
            strcpy( item->is_stp, "*** STRING TOO LONG" );
        else
            strcpy( item->is_stp, buf );
    }
}

void si_onoff( sbItemp item, short onoff )
/*-------------------------------------------------------------------------*
 * Sets an item on or off
 *-------------------------------------------------------------------------*/
{
    if( item->it_onoff != onoff )
    {
        item->update = 1;
        item->it_onoff = onoff;
    }
}

void si_clr( sbItemp item, short c )
/*-------------------------------------------------------------------------*
 * Sets an item's color, and its update flag if the color is different
 * from what it is now.
 *-------------------------------------------------------------------------*/
{
    if( item->color != c )
    {
        item->color = c;
        item->update = 1;
    }
}

void sbd_string( stBarp sbp )
/*-------------------------------------------------------------------------*
 * Displays the string pointed to by sbp->data in the status bar.  This
 * is a user function.
 *-------------------------------------------------------------------------*/
{
    char *ptr, tmp[128];
    short  i = 0;

    /* if data pointer is null, there's no string there */
    if( !(sbp->data) )
        return;

    gr_setcolor( guito256[C_TEXT][3] );

    if( sbp->fheight > 1 )
    {
        ptr = sbp->data;

        while( ptr )
        {
            strccpy( tmp, ptr, "\n" );
            outtextxy( 2, i * GetFontHeight()/10, tmp );
            ptr = strchr( ptr, '\n' );
            if( !ptr )
                break;
            ++ptr; ++i;
        }
    }
    else
        outtextxy( 2, 0, sbp->data );
}

void _drawtoggle( short x, short y, short onoff )
/*-------------------------------------------------------------------------*
 * Draws a toggle.
 *-------------------------------------------------------------------------*/
{
    short radius = GetFontHeight()/20-3;

    x += radius;
    y += radius;

    setwritemode( COPY_PUT );

    /* background */
    setcolor( C_BLACK );
    gr_fillcircle( x, y, radius );
    /* outline */
    setcolor( C_GRAY4 );
    gr_drawcircle( x, y, radius );
    /* toggle */
    if( onoff )
    {
        setcolor( C_LIGHTGREEN );
        gr_fillcircle( x, y, radius-1 );
    }
}

void sbd_standard( stBarp sbp )
/*-------------------------------------------------------------------------*
 * Draws all the items in a status bar.
 *-------------------------------------------------------------------------*/
{
    short   force = sbp->update == 2;
    ushort  color, bgcolor;
    sbItemp itemp;
    NODEPTR ndp;
    short   radius, x, y;

    ndp = NodeFirst( sbp->itemlist );

    while( ndp != sbp->itemlist )
    {
        itemp = (sbItemp) ndp->data;  /* get item pointer */

        if( itemp->color == -1 )    /* woo - kludge! */
            itemp->color = C_TEXT;

        if( force || itemp->update )
        {
            switch( itemp->type )
            {
            case SBI_STRING:
            case SBI_TEXT:
                color = itemp->color;
                if( color >= 256 )
                    bgcolor = guito256[color / 256][3];
                else
                    bgcolor = C_GRAY3;
                color = guito256[color % 256][3];
                gr_rect( itemp->x, itemp->y, itemp->x+itemp->xwidth,
                         itemp->y + GetFontHeight()/10 - 1, bgcolor );
                gr_setcolor( color );
                outtextxy( itemp->x, itemp->y, itemp->is_stp );
                break;
            case SBI_RECT:
                x = itemp->x; y = itemp->y;
                setfillstyle( SOLID_FILL, guito256[itemp->color][3] );
                bar( x, y, x + GetFontHeight()/10, y + GetFontHeight()/10 );
                break;
            case SBI_CIRCLE:
                radius = GetFontHeight()/20-2;
                x = itemp->x + radius; y = itemp->y + radius;
                setcolor( guito256[itemp->color][3] );
                gr_fillcircle( x, y, radius );
                break;
            case SBI_TOGGLE:
                _drawtoggle( itemp->x, itemp->y, itemp->it_onoff );
                break;
            }

            itemp->update = 0;
        }

        ndp = NodeNext( ndp );
    }
}

sbItemp sbi_add( stBarp sbp, short type, short width, char *stp )
/*-------------------------------------------------------------------------*
 * Some of the parameters are only used with specific types
 *-------------------------------------------------------------------------*/
{
    sbItemp itemp, previtemp;

    if( sbp->itemlist == NULL )
    {
        /* first item added. */
        sbp->itemlist = InitList();
        /* add the first item, which is just a marker that starts at 0, 0 */
        itemp = (sbItemp) NodeInsert( sbp->itemlist, sizeof *itemp )->data;
        memset( itemp, 0, sizeof *itemp );
        itemp->type = SBI_FIRST;
        itemp->x = itemp->y = itemp->xwidth = 0;
    }

    /* find the previous item */
    previtemp = (sbItemp) NodeLast( sbp->itemlist )->data;

    /* now add a new item */
    itemp = (sbItemp) NodeInsert( NodeLast( sbp->itemlist ), sizeof *itemp )->data;
    memset( itemp, 0, sizeof *itemp );

    /* calculate its x and y coordinates */
    if( type != SBI_NEXTLINE )
    {
        itemp->x = previtemp->x + previtemp->xwidth + SBXPAD;
        itemp->y = previtemp->y;

        if( type == SBI_TOGGLE || type == SBI_CIRCLE || type == SBI_RECT )
            itemp->xwidth = GetFontHeight()/10 + 2;
        else
        {
            if( width == -1 )
                itemp->xwidth = width = textwidth( stp ) + 2;
            else
                itemp->xwidth = _XUnitsToPixels( width ) + 2;

            if( type == SBI_STRING )
            {
                itemp->is_stp = (char*) memget( SB_MAXSTR+1 );

                /* not necessarily going to be anything in stp right now */
                if( stp )
                    si_str( itemp, stp );
            }
            else if( type == SBI_TEXT )
            {
                /* no need to make a copy - static text.  just point to it */
                itemp->is_stp = stp;
            }
        }
    }
    else
    {
        itemp->x = 0;
        itemp->y = previtemp->y + GetFontHeight()/10;
        itemp->xwidth = 0;
    }

    itemp->color = C_TEXT;

    /* set the type */
    itemp->type = type;

    /* return it */
    return itemp;
}

static sbItemp lc_text;

sbItemp sbc_gettext( void )
/*-------------------------------------------------------------------------*
 * Returns the text item from the last combo created with sbi_addcombo().
 *-------------------------------------------------------------------------*/
{
    return lc_text;
}

void sb_free( stBarp sbp )
/*-------------------------------------------------------------------------*
 * Guess what this does.
 *-------------------------------------------------------------------------*/
{
    NODEPTR ndp;
    sbItemp itemp;

    ndp = NodeFirst( sbp->itemlist );

    if( sbp->data )
        memfree( sbp->data );

    while( ndp != sbp->itemlist )
    {
        itemp = (sbItemp) ndp->data;

        if( itemp->type == SBI_STRING )
            memfree( itemp->is_stp );

        ndp = NodeNext( ndp );
    }

    NukeLinkedList( sbp->itemlist );

    memfree( sbp );
}

sbItemp sbi_addcombo( stBarp sbp, char *stp )
/*-------------------------------------------------------------------------*
 * A function to facilitate adding a toggle and a string to describe the
 * toggle.
 *-------------------------------------------------------------------------*/
{
    sbItemp tog;

    tog = sbi_add( sbp, SBI_TOGGLE, 0, NULL );
    lc_text = sbi_add( sbp, SBI_TEXT, -1, stp );

    return tog;
}

stBarp sb_init( short height, uchar rerect, VFP upd_func )
/*-------------------------------------------------------------------------*
 * Initializes a status bar structure and returns it without adding it
 * to the list of active status bars.  That's handled by sb_attach().
 *-------------------------------------------------------------------------*/
{
    stBarp  sbp;
    short   _height = (height * GetFontHeight()/10) + 4;

    sbp = (stBarp) memget( sizeof( stBar ) );
    memset( sbp, 0, sizeof( stBar ) );

    /* Update the new sbar structure */
    if( upd_func )
        sbp->upd_func = upd_func;
    else
        sbp->upd_func = (VFP) sbd_standard;
    sbp->height = _height;
    sbp->update = 1;
    sbp->data = NULL;
    sbp->rerect = rerect;
    sbp->fheight = height;

    /* Return the status bar */
    return sbp;
}

void sb_attach( stBarp sbp, short where )
/*-------------------------------------------------------------------------*
 * Attaches an initialized status bar to the list of status bars.
 *-------------------------------------------------------------------------*/
{
    NODEPTR ndp;

    if( sbar_list == NULL )
        sbar_list = InitList();

    if( where == SB_BOTTOM )
        ndp = NodeLast( sbar_list );
    else if( where == SB_TOP )
        ndp = sbar_list;
    else    /* insert at this position, so after the one before it. */
        ndp = NodePrev( NodeNumtoPtr( where, sbar_list ) );

    ndp = NodeInsert( ndp, 0 );

    ndp->data = sbp;
    sbp->update = 2;
    sb_recalcpos();
}

void sb_detach( stBarp sbp )
/*-------------------------------------------------------------------------*
 * Removes sbp from the list
 *-------------------------------------------------------------------------*/
{
    NODEPTR ndp = NodeFindData( sbar_list, sbp );

    /* remove it from the list, but don't delete its data */
    ndp->data = NULL;
    NodeDelete( ndp );
    /* recalculate */
    sb_recalcpos();
}

void sb_recalcpos( void )
/*-------------------------------------------------------------------------*
 * Recalculates the positions of all the status bars and sets all their    *
 * update flags to 1.                                                      *
 *-------------------------------------------------------------------------*/
{
    NODEPTR ndp;
    stBarp  sbp;
    short   ypos = sylen;

    ndp = NodeLast( sbar_list );

    while( ndp != sbar_list )
    {
        sbp = (stBarp) ndp->data;

        /* Update the position */
        ypos -= sbp->height;
        sbp->x1 = 0; sbp->x2 = sxlen;
        sbp->y1 = ypos; sbp->y2 = ypos + sbp->height;
        sbp->update = 1;

        ndp = NodePrev( ndp );
    }

    sb_y = ypos;
}

void sb_setupdate( stBarp sbp, short mode )
{
    if( !sbp )
        return;

    if( mode > sbp->update )
        sbp->update = mode;
}

short sb_getupdate( stBarp sbp )
{
    if( !sbp )
        return 0;

    return  sbp->update;
}

void sb_update( short fFullUpdate )
/*-------------------------------------------------------------------------*
 * Updates the status bars.                                                *
 * If fFullUpdate is 1, redraws ALL the status bars.  If not, only redraws *
 * the ones whose update flag is set.                                      *
 *-------------------------------------------------------------------------*/
{
    short   ypos = sb_y;
    stBar   *sbp;
    NODEPTR ndp;
    VFP     fp;
    struct  viewporttype vp;

    if( !sbar_list )
        return;

    getviewsettings( &vp );

    ndp = NodeFirst( sbar_list );

    /* loop through all the status bars and draw them */
    while( ndp != sbar_list )
    {
        /* Update each status bar */
        sbp = (stBarp) ndp->data;

        if( sbp->update || fFullUpdate )
        {
            SetViewPort( 0, 0, sxlen, sylen, 1 );

            if( fFullUpdate || sbp->rerect || sbp->update == 2 )    /* Draw this sbar's rect */
                gr_shadrect( 0, ypos, sxlen, ypos+sbp->height-1, C_SBAR );

            /* Make the viewport proper */
            SetViewPort( 2, ypos+2, sxlen, ypos+2+(sbp->height-4), 1 );

            if( fFullUpdate )
                sbp->update = 2;

            /* Call the update routine */
            fp = sbp->upd_func;
            (*fp)(sbp);

            sbp->update = 0;
        }

        ypos += sbp->height;
        ndp = NodeNext( ndp );
    }

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom, 1 );
}
