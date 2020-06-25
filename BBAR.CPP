/*-------------------------------------------------------------------------*
 * BBAR.C:                                                                 *
 * FUnctions for creating/processing button bars.  Button bars are simply  *
 * a "window" with a bunch of defined buttons in them.  Each button has    *
 * its own unique coordinate space and defined function.  When the mouse   *
 * is clicked inside the button bar (check with isinbbar()), bb_proc() is  *
 * called with the coordinates of the mouse and the appropriate button     *
 * function called.                                                        *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

void _bb_postch( short ch )   { keystuff( ch ); }
void _bb_paste( short notused )
/* Makes sure the next left click is paste.. */
{
    if( current_op == OP_NOTHING )
        current_op = OP_PASTENEXTCLICK;
}

void make_bbar( void )
/*-------------------------------------------------------------------------*
 * Creates the appropriate button bar depending on the current edit mode,  *
 * and returns the height or width of the bar.                             *
 *-------------------------------------------------------------------------*/
{
    if( bbar )      /* Delete the old one? */
    {
        bb_free( bbar );
        bbar = NULL;
    }

    bbar = bb_init( 0, mnu_main->y2+1, sxlen, MENU_HORIZ, 40, sbStat );

    /* Default items */
    bb_additem( bbar, _BB_MISC_LOAD, _bb_postch, KEY_ALTE,
                "Load a map from disk [Alt-E]" );
    bb_additem( bbar, _BB_MISC_SAVE, _bb_postch, KEY_ALTS,
                "Save current map to disk [Alt-S]" );
    bb_additem( bbar, _BB_MISC_RUN, _bb_postch, KEY_F9,
                "Run map [F9]" );
    bb_additem( bbar, _BB_MISC_CHECK, _bb_postch, KEY_ALTC,
                "Consistency checker [Alt+C]" );
    bb_additem( bbar, NULL, NULL, 0, NULL );

    bb_additem( bbar, _BB_MISC_COPY, _bb_postch, KEY_CTRLINS,
                "Copy marked object(s) to clipboard [Alt-Grey Plus]" );
    bb_additem( bbar, _BB_MISC_CUT, _bb_postch, KEY_CTRLDEL,
                "Cut marked object(s) to clipboard [Alt-Grey Minus]" );
    bb_additem( bbar, _BB_MISC_PASTE, _bb_paste, 0,
                "Paste clipboard object(s) to screen [Alt-Grey Star]" );
    bb_additem( bbar, _BB_MISC_KILL, _bb_postch, KEY_DEL,
                "Delete marked object(s) [Del]" );
    bb_additem( bbar, NULL, NULL, 0, NULL );

    bb_additem( bbar, _BB_MISC_GRID, _bb_postch, 'G',
                "Toggle grid on/off [G]" );
    bb_additem( bbar, _BB_MISC_SNAP, _bb_postch, 'P',
                "Toggle snap to grid on/off [P]" );
    bb_additem( bbar, NULL, NULL, 0, NULL );

    bb_additem( bbar, _BB_MISC_MAPDOWN, _bb_postch, KEY_DOWN,
                "Move map down [Down Arrow]" );
    bb_additem( bbar, _BB_MISC_MAPUP, _bb_postch, KEY_UP,
                "Move map up [Up Arrow]" );
    bb_additem( bbar, _BB_MISC_MAPRIGHT, _bb_postch, KEY_RIGHT,
                "Move map right [Right Arrow]" );
    bb_additem( bbar, _BB_MISC_MAPLEFT, _bb_postch, KEY_LEFT,
                "Move map left [Left Arrow]" );

    bb_additem( bbar, NULL, NULL, 0, NULL );
    bb_additem( bbar, _BB_MISC_ZOOMIN, _bb_postch, '+',
                "Zoom map in [+]" );
    bb_additem( bbar, _BB_MISC_ZOOMOUT, _bb_postch, '-',
                "Zoom map out [-]" );

    bb_additem( bbar, NULL, NULL, 0, NULL );
    bb_additem( bbar, _BB_MISC_ROTATE, _bb_postch, KEY_CTRLO,
                "Rotate marked objects [Ctrl+O]" );
    bb_additem( bbar, _BB_MISC_SCALE, _bb_postch, KEY_CTRLA,
                "Scale marked objects [Ctrl+A]" );

    switch( editmode )
    {
    case MODE_VERTEX:       /* No button bar here */
        break;
    case MODE_LINE:     /* Line-specific buttons */
        bb_additem( bbar, NULL, NULL, 0, NULL );
        bb_additem( bbar, _BB_LINE_EDITSD1, _bb_postch, KEY_ALT1,
                    "Edit sidedef 1 [Alt-1]" );
        bb_additem( bbar, _BB_LINE_EDITSD2, _bb_postch, KEY_ALT2,
                    "Edit sidedef 2 [Alt-2]" );
        bb_additem( bbar, _BB_LINE_FLIPSIDE, _bb_postch, KEY_CTRLR,
                    "Flip sidedefs only [Ctrl-R]" );
        bb_additem( bbar, _BB_LINE_FLIPLINE, _bb_postch, KEY_CTRLF,
                    "Flip linedef and sidedefs [Ctrl-F]" );
        bb_additem( bbar, _BB_LINE_MKSTAIR, _bb_postch, CTRL+'Z',
                    "Make stairs from two selected linedefs [Ctrl-Z]" );
        break;
    case MODE_SECTOR:   /* Sector-specific buttons */
        bb_additem( bbar, NULL, NULL, 0, NULL );
        bb_additem( bbar, _BB_SECT_MKDOOR, _bb_postch, KEY_CTRLD,
                    "Make a door from the marked sector" );
        break;
    }

    bb_disp( bbar );
}

/* BBAR.C                                                                  */
BBAR *bb_init( short x, short y, short span, short type, short maxbutton,
               short sbh );
void bb_additem( BBAR *bbp, uchar *bitmap, BUTFUNC func, short wparm,
                 uchar *desc );
short bb_isinside( BBAR *bbp, short x, short y );
void bb_disp( BBAR *bbp );
void bb_proc( BBAR *bbp, short x, short y );
void bb_free( BBAR *bbp );
void bb_showtext( BBAR *bbp, short x, short y );

/*-------------------------------------------------------------------------*
 * Button-bar definitions                                                  *
 *-------------------------------------------------------------------------*/

typedef void (* BUTFUNC)(short);

typedef struct
{
    short     x1, y1, x2, y2;     /* button coordinates */
    uchar   *bitmap;            /* a bitmap to stick in there */
    short     wparm;              /* a parameter! */
    BUTFUNC func;               /* What to run when pressed... */
    uchar   *desc;              /* description! */

} BUTTON;

typedef struct
{
    short     x1, y1, x2, y2;     /* Coordinates */
    short     nbutton;            /* number of buttons in the bar */
    short     maxbutton;          /* max number of buttons */
    short     type;               /* button bar type */
    uchar   *savebuf;           /* underlying image */
    unsigned  sbsize;             /* underlying image size */
    BUTTON  *blist;             /* button list */
    short     sbh;

} BBAR;


/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#define BUTTONSIZE              16

BBAR *bb_init( short x, short y, short span, short type, short maxbutton,
               short sbh )
/*-------------------------------------------------------------------------*
 * Initializes a new button bar.                                           *
 * Creates the bar's coordinates, and initializes "other stuff."           *
 *-------------------------------------------------------------------------*/
{
    BBAR    *bbp;

    _memstr = "button bar";
    bbp = memget( sizeof( BBAR ) );

    /* Initialize misc. structures */
    bbp->maxbutton = maxbutton;
    bbp->nbutton = 0;
    bbp->type = type;
    bbp->savebuf = NULL;
    _memstr = "button";
    bbp->blist = memget( sizeof( BUTTON ) * maxbutton );

    /* Initialize coordinates */
    bbp->x1 = x;
    bbp->y1 = y;
    bbp->sbh = sbh;

    /* Type = horizontal? */
    if( type == MENU_HORIZ )
    {
        bbp->y2 = bbp->y1 + BUTTONSIZE;
        bbp->x2 = bbp->x1 + span;
    }
    /* type = vertical? */
    else
    {
        bbp->x2 = bbp->x1 + BUTTONSIZE-1;
        bbp->y2 = bbp->y1 + span;
    }

    /* outta here */
    return bbp;
}

void bb_additem( BBAR *bbp, uchar *bitmap, BUTFUNC func, short wparm,
                 uchar *desc )
/*-------------------------------------------------------------------------*
 * Adds an item to the button bar.                                         *
 *-------------------------------------------------------------------------*/
{
    BUTTON  *butp = &(bbp->blist[bbp->nbutton]);
    short     ppos;

    butp->bitmap = bitmap;
    butp->func = func;
    butp->wparm = wparm;
    butp->desc = desc;

    /* Calculate coordinates */
    if( bbp->type == MENU_HORIZ )
    {
        ppos = bbp->nbutton ? bbp->blist[bbp->nbutton-1].x2 : -1;
        butp->x1 = ppos+1;
        butp->y1 = 0;
        butp->x2 = butp->x1 + (bitmap ? BUTTONSIZE : BUTTONSIZE/2);
        butp->y2 = BUTTONSIZE;
    }
    else
    {
        ppos = bbp->nbutton ? bbp->blist[bbp->nbutton-1].y2 : -1;
        butp->x1 = 0;
        butp->y1 = ppos+1;
        butp->x2 = BUTTONSIZE;
        butp->y2 = butp->y1 + (bitmap ? BUTTONSIZE : BUTTONSIZE/2);
    }

    ++bbp->nbutton;
}

short bb_isinside( BBAR *bbp, short x, short y )
/*-------------------------------------------------------------------------*
 * Returns -1 if not inside the button bar, or the number of the button    *
 * it's on.                                                                *
 *-------------------------------------------------------------------------*/
{
    short   i;
    BUTTON  *butp;

    /* Not in the bar at all? */
    if( !(inrange( x, bbp->x1, bbp->x2 ) &&
          inrange( y, bbp->y1, bbp->y2 )) )
        return -1;

    /* Stick coordinates inside the bar */
    x -= bbp->x1;
    y -= bbp->y1;

    /* Search button list */
    for( i = 0, butp = bbp->blist; i < bbp->nbutton; i++, butp++ )
    {
        /* In this button? */
        if( inrange( x, butp->x1, butp->x2 ) &&
            inrange( y, butp->y1, butp->y2 ) )
            return i;   /* Yep! */
    }

    /* Not in any buttons, so it may as well be outside the button bar. */
    return -1;
}

void bb_disp( BBAR *bbp )
/*-------------------------------------------------------------------------*
 * Displays the button bar                                                 *
 *-------------------------------------------------------------------------*/
{
    short   i;
    struct  viewporttype vp;

    getviewsettings( &vp );

    cursor_put( CSR_ERASE );

    /* Draw the bar first */
    SetViewPort( 0, 0, sxlen, sylen );
    gr_rect( bbp->x1, bbp->y1, bbp->x2, bbp->y2, 8 );
    SetViewPort( bbp->x1, bbp->y1, bbp->x2, bbp->y2 );

    if( bbp->type == MENU_HORIZ )
    {
        /* Do a horizontal bar! */
        for( i = 0; i < bbp->nbutton; i++ )
        {
            if( bbp->blist[i].bitmap )
            /* Draw this button */
                putimage( bbp->blist[i].x1, 0, bbp->blist[i].bitmap,
                          COPY_PUT );
        }
    }
    else
    {
        for( i = 0; i < bbp->nbutton; i++ )
        {
            if( bbp->blist[i].bitmap )
            /* Draw this button */
                putimage( 0, bbp->blist[i].y1, bbp->blist[i].bitmap,
                          COPY_PUT );
        }
    }

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom, 1 );

    cursor_put( CSR_PUT );
}

void bb_proc( BBAR *bbp, short x, short y )
/*-------------------------------------------------------------------------*
 * Processes the button bar.                                               *
 * Checks to see what button the coordinates are in, and runs the          *
 * function associated with that button, if any.                           *
 *-------------------------------------------------------------------------*/
{
    short   bnumber;
    BUTFUNC func;

    /* Get the button and return if none */
    if( (bnumber = bb_isinside( bbp, x, y )) == -1 )
        return;

    /* Run the button's function .. this is easy! */
    func = bbp->blist[bnumber].func;

    if( func )
        (*func)(bbp->blist[bnumber].wparm);
}

void bb_free( BBAR *bbp )
/*-------------------------------------------------------------------------*
 * Frees a button bar                                                      *
 *-------------------------------------------------------------------------*/
{
    memfree( bbp->blist );
    memfree( bbp );
}

void bb_showtext( BBAR *bbp, short x, short y )
{
    short   bnumber;
    uchar   *desc;

    /* Get the button and return if none */
    if( (bnumber = bb_isinside( bbp, x, y )) == -1 )
        return;

    desc = bbp->blist[bnumber].desc;
    if( !desc )
        desc = "";

    sbs_string( bbp->sbh, desc );
    sb_update( 0 );
}
