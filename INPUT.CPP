/*-------------------------------------------------------------------------*
 * INPUT.C:                                                                *
 * Manages input while editing the current map.                            *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\menu.hpp"
#include "gui\dialog.hpp"

#include "cmddefs.h"

#define KEY_ALTL 9728

#pragma warn -pia

#include <time.h>
#include <math.h>   /* Sigh */

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#include "toolimg.h"

// crosshair color
#define C_CROSSHAIR     C_LIGHTGREEN

// corresponds to OP_XXX defines:
uchar   *op_strings[] =
{
    "               ",
    "Pasting",
    "Dragging",
    "Drawing lines",
    "Scaling",
    "Rotating",
    "Exiting",
    "Box marking",
    "               ",
    "Combining lines",
    "Format painting",
    "Setting tags",
    "Drawing polygon",
    "Drawing rectangle"
};

// status definitions for xor operations:
enum
{
    GS_OFF = -1,
    GS_FIRST = 0,
    GS_ON = 1,
    GS_UPDATE = 2,
    GS_LAST = 3
};

// just a simple macro:
#define isalpha(c)  (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define ilEraseCursor()     if( !__putcur ) { cursor_put( CSR_ERASE ), __putcur = 1; }

// strings that are used frequently:
static  uchar *st_omclear = "Object marks cleared";
static  uchar *st_use_sr = "Use < / >, [Enter] or [Esc]; ";

// crosshair status:
static  short ch_stat = GS_OFF;

// polygon/rectangle status variables:
static  short pr_stat = GS_OFF;
static  short pr_npoints = 5;                     /* guess */
static  short poly_startangle = 0;
static  short pr_x1, pr_y1, pr_x2, pr_y2;   /* screen coords */

// inpdev.cpp (gui lib) externs:
extern  short mou_ldown, mou_rdown, mou_mdown;
extern  short mou_ldrag, mou_rdrag, mou_mdrag;
extern  short _vcj_line;

// misc variables:
short   xend, yend;     /* x and y opposite coordinates of xstart and ystart */
short   current_op;
short   scrcenterx, scrcentery;

// kill this:
stBarp  sbLineLen;

// misc:
Object  go_obj;                 // if IsValid(), the input loop centers on this
short   csr_object = -1,        // Used for selecting object under cursor
        o_csr_object = -1,
        cso_tmp = -1;
Object  objCursor;
bool    fFullScreen;
int     iCreateObject = -1;     // create polgyons/rectangles?

enum
{
    coPolygon,
    coRectangle
};

// externs:
extern  short sel_lenience;     // selection lenience
extern  Menu* mnu_main;         // main menu pointer
extern  short its_loaded;       // shows the load dialog if it's FALSE
extern  short _vc_lean;         // vertex connection leniency
extern  short _doortype;

// module-specific:
static  img_hdr *_toolimg;      // toolbox bitmap
static  uchar  _csr_moved;      // Has the cursor moved since last update?

#define _tics (biostime(0,0L))  // the time

// prototypes from line.cpp:
int getdoortype();
void CreatePolyRegion( int points[][2], short npoints );
void PolyScr2Map( int points[][2], short npoints, short snap, bool fSimple = FALSE );
void PolyMap2Scr( int points[][2], short npoints, short snap );
void CreateLiftFromSector( int iSector );
void CreateDoorFromSector( int iSector );

// other prototypes:
void run_rc_menu( void );
void run_help_menu( void );
void wad_mgmt( void );
void LinedefEdit( int iObject, int iJumpSide );
Object& obj_findtag( Object&, int iDirection, bool fFirst );
int obj_flip( Object& objCursor, int iDirection );
short obj_thingnormalize( short object );
void SetEditMode( short mode );

#include "statbars.h"

// sheesh:
#define KEY_CLEFT   29440
#define KEY_CRIGHT  29696


/*-------------------------------------------------------------------------*
 *                                                                         *
 * File code begins here                                                   *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#define MODE_LINEONLY 0x80

int GetLenience( int iObjectType )
{
    int lenience = sel_lenience;

    if( iObjectType == MODE_LINE )
    {
        lenience += _scrx2map( iScreenWidth / 180 );
    }
    else if( iObjectType != MODE_SECTOR )
    {
        lenience += _scrx2map( iScreenWidth / 128 );
    }

    return lenience;
}

int FindObjectAtMap( short x, short y, Object& obj, int iObjMode = editmode )
{
    short x1 = x;
    short y1 = y;
    short x2, y2, selvar;

    obj.SetInvalid();
    obj.iNumber = -1;

    int lenience = GetLenience( iObjMode );

    obj.iType = iObjMode;

    /* Check the edit mode and expand the box if necessary to make
     * selection margins. */
    if( iObjMode == MODE_LINE || iObjMode == MODE_LINEONLY )
    {
        int iVtxDistance = 32000, iLineDistance = 32000;
        int iVertex = -1, iLine = -1;

        if( iObjMode != MODE_LINEONLY )
        {
            // check for vertex first
            selvar = lenience;
            x2 = x1+selvar;
            y2 = y1-selvar;

            iVertex = obj_get( MODE_VERTEX, x1 - selvar, y1 + selvar, x2, y2,
                                &iVtxDistance );
        }

        // check for line
        selvar = lenience;
        x2 = x1+selvar;
        y2 = y1-selvar;

        iLine = obj_get( MODE_LINE, x1 - selvar, y1 + selvar, x2, y2,
                             &iLineDistance );

        if( iVertex != -1 && iVtxDistance < iLineDistance )
        {
            obj.iType = MODE_VERTEX;
            obj.iNumber = iVertex;
            return iVertex;
        }

        if( iLine != -1 )
        {
            obj.iType = MODE_LINE;
            obj.iNumber = iLine;
            return iLine;
        }

        obj.SetInvalid();
        return -1;
    }
    else
    {
        // other kinds of objects don't use lenience for selection
        x2 = x1;
        y2 = y1;
    }

/* Get the current object                                                  */
    int iRvl = obj_get( iObjMode, x1, y1, x2, y2 );

    if( iRvl == -1 )
    {
        obj.SetInvalid();
        return -1;
    }

    obj.iNumber = iRvl;
    return obj.iNumber;
}

int FindObjectAt( short x, short y, Object& obj, int iObjMode = editmode )
/*-------------------------------------------------------------------------*
 * Finds the object at x, y.  (X, Y are screeen coordinates.)  Returns     *
 * the number of the object found or -1.                                   *
 *-------------------------------------------------------------------------*/
{
    obj.SetInvalid();
    obj.iNumber = -1;

    if( x < 0 || y < 0 )
        return -1;
    if( x > mxplen || y > myplen )
        return -1;

    return FindObjectAtMap( xstart + _scrx2map( x ),
                            ystart + _scry2map( y ), obj, editmode );
}

static void GetScaleRotateValues( double *scale, double *rotate )
/* Gets scale and rotate values */
{
    int     iScale = (short)((*scale) * 100.0), iRotate = (short)(*rotate);

    Dialog *pDialog = new Dialog;
    pDialog->Create( Point( Centered, Centered ),
        Size( 120, 48 ), "Enter scale and rotate values" );
    Field *pField = (Field*) new fldString( pDialog,
        Point( 4, 4 ), 20, ftInteger, "Scale factor (%):" );
    pField->SetTransferBuf( PTR( &iScale ) );
    pField = (Field*) new fldString( pDialog,
        Point( 78, 4 ), 20, ftInteger, "Rotate angle:" );
    pField->SetTransferBuf( PTR( &iRotate ) );
    pDialog->AddDefaultButtons();

    if( pDialog->Process() == daExitSave )
    {
        *scale = ((double) iScale) / 100.0;
        *rotate = (double) iRotate;
    }

    delete pDialog;
}

static void DisplayCoords()
/*-------------------------------------------------------------------------*
 * DIsplays cooridnates on the button bar.                                 *
 *-------------------------------------------------------------------------*/
{
    char    buf[64];
    short   x, y, clr;
    extern  short manip_addx, manip_addy;
    extern  short manip_startx, manip_starty;
    Rect    rcMenu = mnu_main->GetRect();
    int     iTimeWidth;
    static  int iLastWidth = -1;

    SetViewPort( 0, 0, sxlen, sylen );

    if( !tog_drag )
    {
        _scr2map( &x, &y, cursorx, cursory );
        sprintf( buf, "%5d, %-5d ", x, y );
        clr = C_WHITE;
    }
    else
    {
        sprintf( buf, "%5d, %-5d ", manip_addx, manip_addy );
        if( manip_addx == manip_startx && manip_addy == manip_starty )
            clr = C_LIGHTRED;
        else
            clr = C_WHITE;
    }

    iTimeWidth = textwidth( buf );

    /* always same width .. 12 chars */
    if( iLastWidth != -1 )
        _textrect( rcMenu.right-iLastWidth-4, rcMenu.top+4, buf, C_GRAY3 );
    iLastWidth = iTimeWidth;

    setwritemode( COPY_PUT );
    gr_setcolor( C_BLACK );
    outtextxy( rcMenu.right-iTimeWidth-3, rcMenu.top+5, buf );
    gr_setcolor( clr );
    outtextxy( rcMenu.right-iTimeWidth-4, rcMenu.top+4, buf );
}

static void MoveMapTo( short where )
/*-------------------------------------------------------------------------*
 * Moves the map.                                                          *
 *-------------------------------------------------------------------------*/
{
    short xmove, ymove;

    ymove = tog_fine ? (yscale / MSTEP / 2) : (yscale / MSTEP);
    xmove = tog_fine ? (xscale / MSTEP / 2) : (xscale / MSTEP);

    if( where == KEY_LEFT )
        xstart -= xmove;
    else if( where == KEY_RIGHT )
        xstart += xmove;
    else if( where == KEY_DOWN )
        ystart -= ymove;
    else if( where == KEY_UP )
        ystart += ymove;

    xend = xstart + xscale;
    yend = ystart + yscale;
}

static void tlb( void )
/*-------------------------------------------------------------------------*
 * Finds you repugnant.                                                    *
 * THANKS ROB!                                                             *
 *-------------------------------------------------------------------------*/
{
    #define NPHRASE 12

    char *phrases[NPHRASE] =
    {
        "The Lone Banana finds you repugnant.",
        "Yeah ...my head..",
        "What a cavalier idea!",
        "what.. um... 'ERE!",
        "Alfred E. Neuman for president!",
        "\"If I gave one to you, I'd have to give one to everybody.\"",
        "Veddy veddy creamy.",
        "I'm a funkay honkay!",
        "What time is it? Four thar-TAY! It's not late.. naw, NAW!\n"
            "It's just-a er-lay, er-LAY, ER-LAY!",
        "\"mm, cake..\"\n",
        "I wish I could come back there and watch!",
            "this man may have            \n"
            "a shitload to prove\n"
            "he's got to settle a score\n"
            "against the groove"
    };
    int n;

    n = rand() % NPHRASE;

    message_box( MB_OK, "Someone once said:", phrases[n] );
}

static Object& GetJumpObject()
/*-------------------------------------------------------------------------*
 * Gets an object to go to                                                 *
 *-------------------------------------------------------------------------*/
{
    int iObject = 0;
    static uchar fGoVertex = FALSE;
    static Object obj;

    Dialog *pDialog = new Dialog;
    pDialog->Create( Point( Centered, Centered ),
        Size( 160, 40 ), "Go to object:" );
    Field *pField = (Field*) new fldString( pDialog,
        Point( 4, 4 ), 20, ftInteger, "Enter the object number to go to:",
        csPromptBeside );
    pField->SetTransferBuf( PTR( &iObject ) );
    Field *pGoVertex;
    if( editmode != MODE_SECTOR && editmode != MODE_THING )
    {
        pGoVertex = new fldCheckBox( pDialog, Point( 4, 16 ),
            "Jump to vertex" );
        pGoVertex->SetTransferBuf( PTR(&fGoVertex) );
    }
    pDialog->AddDefaultButtons();

    int iDlgRvl = pDialog->Process();

    delete pDialog;

    if( iDlgRvl == daExitSave )
    {
        obj.iType = editmode;

        if( editmode != MODE_SECTOR && editmode != MODE_THING )
            if( fGoVertex )
                obj.iType = MODE_VERTEX;

        obj.iNumber = iObject;
    }
    else
        obj.SetInvalid();

    return obj;
}

static void MakeMapScreenCoords( void )
/*-------------------------------------------------------------------------*
 * Creates the map coordinates, sets the mouse limits, sets the screen     *
 * center positions .. etc.                                                *
 *-------------------------------------------------------------------------*/
{
    if( fFullScreen )
    {
        mypstart = 0;
        mxpstart = 0;
        mxplen = iScreenWidth;
        myplen = iScreenHeight;
    }
    else
    {
        mypstart = mnu_main->GetRect().bottom + 2;
        mxpstart = 0;
        mxplen = sxlen;
        myplen = iScreenHeight-85-40 - mypstart + 2;
    }

    scrcenterx = mxplen/2;
    scrcentery = myplen/2;

    if( xscale )
        setscale( xscale );
}

static void GoToObject( Object& obj )
{
    short tmpx, tmpy;

    /* Go to this object!
     * 1. set the center of the screen to the center of the object
     * 2. put the mouse cursor there. */

    if( (obj.iType == MODE_LINE && obj.iNumber < nline) ||
        (obj.iType == MODE_SECTOR && obj.iNumber < nsector) ||
        (obj.iType == MODE_THING && obj.iNumber < nthing) ||
        (obj.iType == MODE_VERTEX && obj.iNumber < nvertex) )
    {
        if( editmode != obj.iType )
            SetEditMode( obj.iType == MODE_VERTEX ? MODE_LINE : obj.iType );

        if( obj.iType != MODE_SECTOR )
            obj_center( obj.iType, obj.iNumber, &tmpx, &tmpy );
        else
        {
            int iLine = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, obj.iNumber, LF_END );
            if( iLine != -1 )
                obj_center( MODE_LINE, iLine, &tmpx, &tmpy );
            else
                return;
        }
        setscale( config._zoom_low );     /* Zoom in man */
        xstart = tmpx - (xscale / 2);
        ystart = tmpy - (yscale / 2);
        post_fullupdate();
        mouse_setcoords( mxpstart + _mapx2scr( tmpx - xstart ),
                         mypstart + _mapy2scr( tmpy - ystart ) );
    }
}

static void MoveMapOffset()
// based on the height of the status bars, keeps the map in the same position
{
    static  short oldmyplen = -1;

    if( oldmyplen == -1 || myplen == oldmyplen )
    {
        /* Don't do it first time or if not changed */
        oldmyplen = myplen;
        return;
    }

    ystart += _absscry2map( oldmyplen-myplen );
    oldmyplen = myplen;
}

void SetEditMode( short mode )
/*-------------------------------------------------------------------------*
 * Sets the edit mode.                                                     *
 *-------------------------------------------------------------------------*/
{
    if( mode == MODE_RECTANGLE )
    {
        mode = MODE_LINE;
        iCreateObject = coRectangle;
        SetCursorType( ctCross );
        obj_clearmarks( -1, 0 );
        post_fullupdate();
    }
    else if( mode == MODE_POLYGON )
    {
        mode = MODE_LINE;
        iCreateObject = coPolygon;
        SetCursorType( ctCross );
        obj_clearmarks( -1, 0 );
        post_fullupdate();
    }
    else
    {
        iCreateObject = -1;
        SetCursorType( ctArrow );
    }

    if( mode == editmode )
        return;

    csr_object = -1;
    editmode = mode;
    mk_menus();
    MakeMapScreenCoords();

    /* Full update */
    fFullUpdate = 2;
}

static void CenterMap( void )
{
    short   i;
    signed  short minx, maxx, miny, maxy;
    VERTEX  *v;

    minx = 32000;
    miny = 32000;
    maxx = -32000;
    maxy = -32000;

    for( i = 0, v = vlist; i < nvertex; i++, v++ )
    {
        if( v->x < minx )    minx = v->x;
        if( v->x > maxx )    maxx = v->x;
        if( v->y < miny )    miny = v->y;
        if( v->y > maxy )    maxy = v->y;
    }

    xstart = ((maxx + minx) / 2) - xscale / 2;
    ystart = ((maxy + miny) / 2) - yscale / 2;

    post_fullupdate();
}

static int ShowFlagsDialog()
{
    // display a list of shift-key flags that are toggleable for the current
    // edit mode.
    if( editmode == MODE_SECTOR || editmode == MODE_VERTEX )
        return -2;     // nunna that

    Dialog* pDialog = new Dialog;

    if( editmode == MODE_THING )
        pDialog->Create( szDSFile, "ThingFlagDlg" );
    else if( editmode == MODE_LINE )
        pDialog->Create( szDSFile, "LineFlagDlg" );

    pDialog->Draw();

    int iKey = -2;

    while( isshift() )
    {
        if( keycheck() )
        {
            iKey = keyget();
            break;
        }
    }

    pDialog->Erase();
    delete pDialog;

    return iKey;
}

static short ZoomRelative( short scale )
/*-------------------------------------------------------------------------*
 * Changes the current map zoom by "scale", focussed on the cursor
 *-------------------------------------------------------------------------*/
{
    short lxscale = xscale;

    /* This is so the zoom centers on the cursor */
    xstart += _scrx2map( cursorx );
    ystart += _scry2map( cursory );

    xscale += scale;

    if( xscale < 125 )
        xscale = 125;
    if( xscale > 20000 )
        xscale = 20000;

    setscale( xscale );

    xstart -= _scrx2map( cursorx );
    ystart -= _scry2map( cursory );

    SetStatusMsg( "Zoom X span (map coordinates): %d", xscale );

    if( lxscale != xscale )
    {
        post_fullupdate();
        return 1;
    }

    return 0;
}

static short ZoomAbs( short scale )
{
    return( ZoomRelative( scale - xscale ) );
}

int CmdBehavior( char* pRaw, char* pszOutputFile );
void DrawScreen();

void DecompileScript( char* pBuffer, char* pszFilename,
    char* pszMapTag )
{
    if( !pBehavior )
    {
        message_box( MB_OK, st_dcksays,
            "There is no script data to decompile." );
        return;
    }

    char szScriptName[128];

    strccpy( szScriptName, pszFilename, "." );
    char *ptr = strrchr( szScriptName, '\\' ) + 1;
    ptr[6] = 0;
    strcat( szScriptName, pszMapTag+3 );
    strcat( szScriptName, ".acs" );

    if( !file_get( FG_SAVE | FG_NOMAPS, szScriptName, "ACS" ) )
        return;

    CmdBehavior( pBuffer, szScriptName );

    if( message_box( MB_YESNO, st_dcksays, "The behavior lump was decompiled.\n"
        "Do you want to edit it now?" ) == MB_NO )
        return;

    gr_vidmode( MODE_TEXT );
    DosCommand( "%s %s", config.szEditor, szScriptName );
    gr_vidmode( MODE_GFX );
    mouse_init();
    DrawScreen();
    mouse_show();
}

/*-------------------------------------------------------------------------*
 *
 * Mouse toolbar functions:
 *
 *-------------------------------------------------------------------------*/

struct
{
    short   key;
    uchar   *desc;

} cp_keys[] =
{
//    {'N', "Switch to the polygon tool"},        /* polygons */
    {'R', "Switch to the rectangle tool"},        /* rectangles */
    {'L', "Switch to the line and vertex tool"},        /* line */
//    {'V', "Switch to the vertex tool"},        /* vertex */
//    {CTRL+'T', "Tag the marked objects to another object"},   /* tags */
//    {CTRL+'P', "Paint the marked objects with the format of another object"},   /* painter */
    {'S', "Change to Sector Mode"},         /* sector */
    {'T', "Change to Item Mode"}       /* things */
};

short runcursorpad( void )
/*-------------------------------------------------------------------------*
 * Runs a cursor pad.
 *
 * If returns 0, do the mark box.
 * If 1, the user selected something.
 *-------------------------------------------------------------------------*/
{
    static  unsigned    imgsize;
    static  uchar       *underimg;
    short   x1, x2, y1, y2, x, y, xc, yc, mrstart = mou_rdown, moved_far = 0;
    short   angle, one_second = getduration( 70 );
    uchar   *st_edit = "Edit the object(s)";
    long    timenow = _tics;
    long    post_ch = 0;
    struct  viewporttype vp;

    getviewsettings( &vp );
    SetViewPort( 0, 0, sxlen, sylen, 1 );

    xc = scursorx;
    yc = scursory;

    x1 = xc - _toolimg->width / 2;
    y1 = yc - _toolimg->height / 2;
    x2 = x1 + _toolimg->width;
    y2 = y1 + _toolimg->height;

    if( !underimg )
    {
        imgsize = imagesize( x1, y1, x2, y2 );
        underimg = new uchar[imgsize];
    }

    cursor_put( CSR_ERASE );
    getimage( x1, y1, x2, y2, underimg );
    putimage( x1, y1, _toolimg, COPY_PUT );
    cursor_put( CSR_PUT );

    while( 1 )
    {
        if( keycheck() )
            keyget();

        if( mouse_event( &x, &y ) == MOUSE_MOVE )
        {
            /* too far out maan */
            if( abs(xc-x) > (x2-x1) || abs(yc-y) > (y2-y1) )
                SetStatusMsg( st_blank );
            else if( abs( xc - x ) <= 6 && abs( yc - y ) <= 6 )
                SetStatusMsg( st_edit );
            else
            {
                angle = _angle( xc, yc, x, y );
                angle /= 90;
                SetStatusMsg( cp_keys[angle].desc );
            }
            UpdateMessageBar();
        }

        if( mou_rdown != mrstart )
        {
            mou_rdown = 0;

            /* too far out maan */
            if( abs(xc-x) > (x2-x1) || abs(yc-y) > (y2-y1) )
                break;

            /* check out where: <= 6 from the center is in the edit button */
            if( abs( xc - x ) <= 6 && abs( yc - y ) <= 6 )
                post_ch = KEY_ENTER;
            else
            {
                angle = _angle( xc, yc, x, y );
                angle /= 90;
                post_ch = cp_keys[angle].key;
                break;
            }
        }

        if( post_ch )
            break;

        /* this is so it only happens if the user doesn't initially move */
        if( !(inrange( x, xc-1, xc+1 ) && inrange( y, yc-1, yc+1 )) )
            moved_far = 1;

        if( !moved_far && (_tics - timenow) > one_second )
        {
            post_ch = -1;
            break;
        }
    }

    SetStatusMsg( st_blank );
    UpdateMessageBar();

    if( post_ch > 0 )
        keystuff( post_ch );

    cursor_put( CSR_ERASE );
    SetViewPort( 0, 0, sxlen, sylen, 1 );
    putimage( x1, y1, underimg, COPY_PUT );
    cursor_put( CSR_PUT );

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom, 1 );

    return (post_ch != -1);
}

// **
// graphics status functions
// **


static short GS_checkerase( short stat )
/*-------------------------------------------------------------------------*
 * This takes a variable whose value is one of the GS_xxx constants, and
 * returns 1 if the old operation should be erased.
 *-------------------------------------------------------------------------*/
{
    if( stat == GS_OFF )
        return 0;

    if( fFullUpdate )   /* don't erase on full updates */
        return 0;

    /* last always erases .. update always draws */
    if( stat == GS_LAST || stat == GS_UPDATE )
        return 1;

    if( stat == GS_ON && _csr_moved )
        return 1;

    return 0;
}

static short GS_checkdraw( short stat )
/*-------------------------------------------------------------------------*
 * This takes a variable whose value is one of the GS_xxx constants, and
 * returns 1 if the operation should be redrawn.
 *-------------------------------------------------------------------------*/
{
    if( stat == GS_OFF )
        return 0;

    if( fFullUpdate )
    {
        /* full update but last?  don't redraw */
        if( stat == GS_LAST )
            return 0;
        return 1;
    }

    if( stat == GS_ON && _csr_moved )
        return 1;

    /* first always draws .. update always draws */
    if( stat == GS_FIRST || stat == GS_UPDATE )
        return 1;

    return 0;
}

static short GS_xlate( short stat )
/*-------------------------------------------------------------------------*
 * Translates the status from one of the one-time values to one of the
 * on or off values.
 *-------------------------------------------------------------------------*/
{
    if( stat == GS_UPDATE )
        stat = GS_ON;
    if( stat == GS_LAST )
        stat = GS_OFF;
    if( stat == GS_FIRST )
        stat = GS_ON;

    return stat;
}

void DrawScreen()
{
    mouse_hide();

    // draw menus and status bars since this is the first draw:
    mnu_main->Draw( FALSE );
    DrawStatusBars();

    SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen, 1 );
    map_update( FALSE );

    UpdateObjectBar( objCursor, fFullUpdate );
    UpdateMainBar();
    UpdateMessageBar();

    SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen, 1 );

    mouse_show();
}

enum
{
    UNMARK,
    MARK
};

inline void obj_highlight( Object& obj, int iColor )
{
    obj_highlight( obj.iType, obj.iNumber, iColor );
}

short le_input( void )
/*-------------------------------------------------------------------------*
 * Maintains an input loop and dispatches functions based on what the user *
 * entered.  Also handles mouse functions et al.                           *
 *-------------------------------------------------------------------------*/
{
    int     iTagDestMode;
    short   first_update = 1;       /* First update? */
    unsigned ch;                /* Keyboard input */
    short   mb_stat = GS_OFF;
    uchar   donothing = 0;          /* if 1, bypass input */
    short   mb_x1 = -1,             /* Mark box coordinates */
            mb_y1 = -1,
            mb_x2 = -1,
            mb_y2 = -1;
    short   lastshift = 0;
    short   mouseop;                /* Mouse operation (from mouse_event) */
    uchar   tog_showclip = 0;       /* Show clipboard rectangle? */
    short   scx, scy;               /* Cursor positions */
    short   tmpx, tmpy,               /* Some temporary storage */
            tmpint, rvl = 0, i;
    short   ocursorx = -1,          /* Old cursor positions */
            ocursory = -1;
    static  GWINDOW *gwnp;
    uchar   __putcur = 0;
    uchar   cbuf[4];                /* For TLB */
    uchar   upd_once = 0;
    short   mouse_obj;              /* for getting the object under the mouse */
    uchar   mouse_zoom = 0;
    double  scale_factor,           /* For scaling and rotation, surprise! */
            rotate_angle;
    static  uchar first_init = 1;
    short   opr_npoints;
    ulong   _lastscroll = 0, _lastdel = 0L, _firstscroll = 0L;
    ulong   _scrollwait;
    extern  uchar _vertex_align;    /* vertex alignment! */
    extern  short _lcomb_first;
    short   mbox_mode;              /* Mark box mode (MBOX_xxx) */
    uchar   unmarkobj = 0;
    long    lShiftDownTime = -1;
    short   obj_marked;
    static  ulong   tmplong;
    extern  short mou_deltax, mou_deltay;
    extern  short m_showlength;
    extern  short len_num;
    extern  short len_colors[32];
    extern  short len_lengths[32];
    extern  char fDemoPlaying, fDemoRecording;
    extern  int   pmPoints[pmMaxPoints+1][2];
    bool    fScrolling = FALSE, fLastUpdateDraft = FALSE;
    short   secdel_tics = getduration( 1500 );
    uchar   *vstring[] = { "Off", "On (Normal)", "On (Vertex)" };
    uchar   *tlb_st = "\r\n\r\n For enlightenment, hold thy shift key and type TLB \r\n\r\n";
    int     iObjectType, iOldObjType;
    Object  objCursorLast, objHighlighted;
    bool    fUpdateHighlight;

    extern char *picbuf;
    void putimage_bg( short x, short y, uchar *buf );

    _vertex_align = 0;
    _scrollwait = getduration( 10 );

    /* We're not doing anything, stupid. */
    current_op = OP_NOTHING;

    if( first_init )
    {
        CreateStatusBars();
        mk_menus();
        mnu_main->SetRect();
        SetEditMode( editmode );
    }

    MakeMapScreenCoords();
    setscale( xscale );

    /* Calculate the startx and starty map coordinates to center it upon
     * load.  Only if we're not restoring, though. */
    if( !(restore & R_MAP) || !first_init )
    {
        torigin_x = -1; /* no origin yet */
        setscale( config._zoom_high );
        CenterMap();
    }

    _toolimg = (img_hdr*) ipi2img( TOOLIMG, sizeof TOOLIMG, (img_hdr*) _toolimg );

    /* Do a full update first */
    post_fullupdate();

    if( first_init )
        mouse_setcoords( scrcenterx, scrcentery );

    int cpage = 1;

    while( 1 )
    {
        /* Exit??!?!?!?!!!!???????? */
        if( current_op == OP_EXIT || get_out )
        {
            if( get_out == 2 )
            {
                get_out = 0;
                rvl = 1;
            }
            goto done;
        }

        globtomap( &cursorx, &cursory, scursorx, scursory );

        if( fScrolling == FALSE && fLastUpdateDraft == TRUE )
            post_fullupdate();

        _csr_moved = (cursorx != ocursorx) || (cursory != ocursory);

        /* Do nothing?  Just jump to the input loop. */
        if( !_csr_moved && donothing && !fFullUpdate )
            goto getthekey;

        MoveMapOffset();

        SetViewPort( 0, 0, sxlen, sylen, 1 );

        /* Initialize scaling variables */
        if( fFullUpdate || first_init )
            setscale( xscale );


/*-------------------------------------------------------------------------*
 *                                                                         *
 * Screen update cycle                                                     *
 *                                                                         *
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Here's where the actual screen update occurs.  Each element of the      *
 * screen is updated if required: 1) its update flag is set or  2) a full  *
 * update is being performed.                                              *
 *-------------------------------------------------------------------------*/
        if( fFullUpdate )
        {
            ilEraseCursor();

            setactivepage( cpage );

            if( first_update || fFullUpdate == 2 )
            {
                // draw menus and status bars since this is the first draw:
                if( !fFullScreen )
                    mnu_main->Draw( FALSE );
                DrawStatusBars();
            }

            SetViewPort( mxpstart, mypstart-1, mxpstart+mxplen, mypstart+myplen+1, 1 );
            clearviewport();
        }

        SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen, 1 );

        if( !fFullUpdate && tog_showclip )
            cb_draw( -1, -1 );

        if( current_op == OP_PASTENEXTCLICK && !tog_showclip )
            tog_showclip = 1;

/*-------------------------------------------------------------------------*
 * Cursor-selection determination begins here.                             *
 * This is the beginning of the code for the cursor selection routine.     *
 * The code determines which object (if any) is under the cursor and       *
 * highlights it appropriately.  First, it removes any existing "cursor    *
 * selected"-highlighted objects.                                          *
 *-------------------------------------------------------------------------*/
        if( fFullUpdate || !donothing )
            FindObjectAt( cursorx, cursory, objCursor );

        if( current_op != OP_NOTHING )
        {
            fUpdateHighlight = FALSE;

            if( current_op == OP_COMBLINE || current_op == OP_FPAINTER ||
                current_op == OP_TAGSET )
                fUpdateHighlight = TRUE;
        }
        else
            fUpdateHighlight = TRUE;

        // remove the old highlight
        if( fUpdateHighlight && !fFullUpdate && !fScrolling )
        {
            if( objCursor != objCursorLast && objCursorLast.IsValid() )
            {
                ilEraseCursor();
                /* unHighlight the object */
                if( !obj_ismarked( objCursorLast ) )
                    obj_highlight( objCursorLast, COLOR_CSRSEL );
                else
                    obj_highlight( objCursorLast, COLOR_MARKED ^ COLOR_CSRSEL );
                objCursorLast.SetInvalid();
                objHighlighted.SetInvalid();
            }
        }
        else
        {
            objHighlighted.SetInvalid();
            objCursorLast.SetInvalid();
        }

/* Update map                                                              */
        if( fFullUpdate && !go_obj.IsValid() )
        {
            map_update( fScrolling );
            if( fScrolling )
                fLastUpdateDraft = TRUE;
            else
                fLastUpdateDraft = FALSE;
        }

/* Highlight selected object if there is one.                              */
        if( editmode <= MODE_OBJLAST && fUpdateHighlight && !fScrolling )
        {
            if( objCursor != objCursorLast && objCursor.IsValid() )
            {
                ilEraseCursor();
                objCursorLast = objCursor;
                if( !obj_ismarked( objCursor ) )
                    obj_highlight( objCursor, COLOR_CSRSEL );
                else
                    obj_highlight( objCursor, COLOR_MARKED ^ COLOR_CSRSEL );
                objHighlighted = objCursor;
            }
        }
        else
            objCursor.SetInvalid();

        // update object bar with cursor object
        if( inrange( scursory, mypstart, mypstart+myplen ) )
        {
            if( editmode == MODE_LINE && iCreateObject != -1 )
                SetCursorType( ctCross );

            if( its_loaded )
                UpdateObjectBar( objCursor, fFullUpdate );
        }
        else
            SetCursorType( ctArrow );

/* Update status bars                                                      */
        if( fFullUpdate < 2 )
        {
            UpdateMainBar();
            UpdateMessageBar();
        }

        SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen, 1 );

/*-------------------------------------------------------------------------*
 * Drag mode on?  Move selected/marked objects.                            *
 * When drag mode is on, there are a couple twists to the update cycle.    *
 * If a full update was performed, move the objects, draw the new lines.   *
 * If it was a partial update, draw over the old lines, move the objects,  *
 * and then draw new lines.                                                *
 *-------------------------------------------------------------------------*/
        if( current_op == OP_SCALE || current_op == OP_ROTATE )
        {
            if( fFullUpdate )
                obj_drawmaniplist( -1, -1, rotate_angle, scale_factor );
        }
        else if( current_op == OP_CREATELINE || current_op == OP_DRAG )
        {
            if( fFullUpdate || obj_manipupd( cursorx, cursory ) )
            {
                /* Erase the old ones if there's nothing there */
                if( !fFullUpdate )
                    obj_drawmaniplist( -1, -1, -1, -1 );
                /* Redraw the objects */
                obj_drawmaniplist( cursorx, cursory, -1, -1 );
            }
        }

/* Update the mark box                                                     */
        if( mb_stat > GS_OFF )
        {
            gr_setlinestyle( DOTTED_LINE, 1 );
            /* Set XOR mode */
            setwritemode( XOR_PUT );

            /* Erase old? */
            if( GS_checkerase( mb_stat ) )
            {
                ilEraseCursor();
                gr_box( _mapx2scr(mb_x1-xstart), _mapy2scr(mb_y1-ystart),
                        _mapx2scr(mb_x2-xstart), _mapy2scr(mb_y2-ystart),
                        C_WHITE );
            }

            /* Create new? */
            if( GS_checkdraw( mb_stat ) )
            {
                mb_x2 = xstart + _scrx2map( cursorx );
                mb_y2 = ystart + _scry2map( cursory );

                ilEraseCursor();
                gr_box( _mapx2scr(mb_x1-xstart), _mapy2scr(mb_y1-ystart),
                        _mapx2scr(mb_x2-xstart), _mapy2scr(mb_y2-ystart),
                        C_WHITE );
            }

            setwritemode( COPY_PUT );
            gr_setlinestyle( SOLID_LINE, 1 );

            mb_stat = GS_xlate( mb_stat );
        }

/* Place clipboard rectangle?                                              */
        if( tog_showclip )
            cb_draw( cursorx, cursory );

/* Draw crosshair                                                          */
        if( ch_stat > GS_OFF )
        {
            SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen, 1 );
            gr_setlinestyle( SOLID_LINE, 1 );

            if( GS_checkerase( ch_stat ) )
            {
                ilEraseCursor();
                setwritemode( XOR_PUT );
                gr_setcolor( C_CROSSHAIR );
                line( 0, ocursory, mxplen, ocursory );
                line( ocursorx, 0, ocursorx, myplen );
            }

            if( GS_checkdraw( ch_stat ) )
            {
                ilEraseCursor();
                setwritemode( XOR_PUT );
                gr_setcolor( C_CROSSHAIR );
                line( 0, cursory, mxplen, cursory );
                line( cursorx, 0, cursorx, myplen );
            }

            ch_stat = GS_xlate( ch_stat );

            setwritemode( COPY_PUT );
        }

/* Draw polygon?                                                           */
        if( pr_stat > GS_OFF )
        {
            SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen, 1 );
            setlinestyle( SOLID_LINE, 0, 1 );

            if( GS_checkerase( pr_stat ) )
            {
                ilEraseCursor();

                setwritemode( XOR_PUT );
                gr_setcolor( C_WHITE );
                /* poly or rectangle */
                drawpoly( iCreateObject == coPolygon ? opr_npoints+1 : 5,
                          (int*) pmPoints );
            }

            opr_npoints = pr_npoints;

            if( mou_rdown != 1 )
            {
                if( iCreateObject == coPolygon && isctrl() )
                {
                    tmpint = line_len( pr_x1, pr_y1,
                             xstart+_scrx2map(cursorx), ystart+_scry2map(cursory) );

                    pr_x2 = pr_x1 + tmpint;
                    pr_y2 = pr_y1 - tmpint;
                }
                else
                {
                    pr_x2 = xstart+_scrx2map(cursorx);
                    pr_y2 = ystart+_scry2map(cursory);
                }
            }
            else
            {
                /* sizes.. */
                tmpx = (pr_x2-pr_x1);
                tmpy = (pr_y2-pr_y1);

                pr_x1 += (xstart+_scrx2map(cursorx)) - pr_x2;
                pr_y1 += (ystart+_scry2map(cursory)) - pr_y2;
                pr_x2 = pr_x1 + tmpx;
                pr_y2 = pr_y1 + tmpy;
            }

            if( GS_checkdraw( pr_stat ) )
            {
                ilEraseCursor();

                setwritemode( XOR_PUT );
                gr_setcolor( C_WHITE );

                if( iCreateObject == coPolygon )  /* POLYGON */
                {
                    polyMake( pr_x1-(pr_x2-pr_x1),
                              pr_y1-(pr_y2-pr_y1),
                              pr_x1-(pr_x2-pr_x1) + (pr_x2 - pr_x1) * 2,
                              pr_y1-(pr_y2-pr_y1) + (pr_y2 - pr_y1) * 2, pr_npoints, poly_startangle );
                    PolyMap2Scr( pmPoints, pr_npoints+1, tog_snap );
                    drawpoly( pr_npoints+1, (int*) pmPoints );
                }
                else /* RECTANGLE */
                {
                    pmPoints[0][0] = pr_x1; pmPoints[0][1] = pr_y1;
                    pmPoints[1][0] = pr_x2; pmPoints[1][1] = pr_y1;
                    pmPoints[2][0] = pr_x2; pmPoints[2][1] = pr_y2;
                    pmPoints[3][0] = pr_x1; pmPoints[3][1] = pr_y2;
                    pmPoints[4][0] = pr_x1; pmPoints[4][1] = pr_y1;
                    PolyMap2Scr( pmPoints, 5, tog_snap );
                    drawpoly( 5, (int*) pmPoints );
                }
            }

            pr_stat = GS_xlate( pr_stat );

            setwritemode( COPY_PUT );
            setlinestyle( SOLID_LINE, 0, 1 );
        }

/* Place the cursor                                                        */
        if( (_csr_moved || fFullUpdate) && !fFullScreen )
            DisplayCoords();

        if( __putcur )
        {
            cursor_put( CSR_PUT );
        }

/* Reset update variables                                                  */
        ocursorx = cursorx;
        ocursory = cursory;

        if( fFullUpdate )
        {
            setvisualpage( cpage );
            cpage = !cpage;
        }

        first_update = 0;
        __putcur = 0;
        fFullUpdate = 0;

        SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen, 1 );

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Input location                                                          *
 *                                                                         *
 *-------------------------------------------------------------------------*/
getthekey:
        if( !upd_once && first_init )
        {
#ifdef REGISTERED
            if( fGetConfig || config.tog_welcome )
#endif
                about();

            if( fGetConfig )
                getconfig();

            void ShowTip();

#ifdef REGISTERED
            if( config.tog_showtips )
#endif
                ShowTip();

            if( 0 ) // !its_loaded )    /* no map loaded? load one. */
            {
                if( !map_get() )
                    goto done;
                _toolimg = (img_hdr*) ipi2img( TOOLIMG, sizeof TOOLIMG, (img_hdr*) _toolimg );

                if( !fNewMap )
                {
                    // only update screen if it's a new file, since we're only
                    // here if no file was loaded
                    upd_once = 1;
                    keystuff( 7 );  /* center it */
                    continue;
                }
            }
        }

        upd_once = 1;

        ch = -1;

        if( keycheck() )
        {
            ch = keyget();
            ch = _mtoupper( ch );

            lShiftDownTime = -1;

            if( isshift() )
            {
                /* Shift that buffer */
                cbuf[0] = cbuf[1];
                cbuf[1] = cbuf[2];
                cbuf[2] = ch;
                if( !strncmpi( cbuf, "TLB", 3 ) )
                    tlb();
            }
            donothing = 0;
        }
        else if( go_obj.IsValid() )
        {
            GoToObject( go_obj );
            ch = -2;            /* never do this character */
            go_obj.SetInvalid();        /* Don't do it again */
            globtomap( &cursorx, &cursory, scursorx, scursory );
        }

        // delayed flags toggle dialog box
        if( isshift() && lShiftDownTime == -1 && current_op == OP_NOTHING )
            lShiftDownTime = biostime( 0, 0L );
        else if( isshift() && lShiftDownTime != -1 &&
            biostime( 0, 0L ) - lShiftDownTime > 18 )
        {
            ch = ShowFlagsDialog();
            lShiftDownTime = -1;
        }
        else if( !isshift() )
            lShiftDownTime = -1;

        // check for flag toggle key:
        if( isshift() && inrange( ch, 0, 255 ) && objCursor.iType == editmode )
        {
            char *pszResult;

            pszResult = obj_toggleflags( objCursor, ch );
            if( pszResult )
            {
                SetStatusMsg( pszResult );
                donothing = 0;
                ch = -2;    // don't do dat
            }
            // if a null return, no valid key was pressed for the toggle
            // flags function - so just go to the normal processing.
        }

        csr_object = objCursor.iNumber;

        int iCurObject = csr_object;

        if( iObjectType != editmode )
            iCurObject = -1;

        switch( ch )
        {
/*-------------------------------------------------------------------------*
 * 'F': Full screen
 *-------------------------------------------------------------------------*/
        case 'F':
            fFullScreen = !fFullScreen;
            fFullUpdate = 2;
            MakeMapScreenCoords();

            break;

/*-------------------------------------------------------------------------*
 * f7: test something
 *-------------------------------------------------------------------------*/
        case CMD_FLIPHORIZ:
            if( current_op != OP_NOTHING || !(editmode & MODE_ALL) )
                break;

            obj_flip( objCursor, 0 );
            break;

        case CMD_FLIPVERT:
            if( current_op != OP_NOTHING || !(editmode & MODE_ALL) )
                break;

            obj_flip( objCursor, 1 );
            break;

/*-------------------------------------------------------------------------*
 * Find tags
 *-------------------------------------------------------------------------*/
        case ';':
        case '/':
        case '\\':
            if( current_op != OP_NOTHING || !(editmode & (MODE_LINE | MODE_SECTOR |
                MODE_THING)) )
                break;

#ifndef REGISTERED
            Unregistered();
#else
            if( ch == ';' )
                go_obj = obj_findtag( objCursor, 1, TRUE );
            else if( ch == '/' )
                go_obj = obj_findtag( objCursor, 1, FALSE );
            else if( ch == '\\' )
                go_obj = obj_findtag( objCursor, -1, FALSE );
#endif

            break;

/*-------------------------------------------------------------------------*
 * Undo
 *-------------------------------------------------------------------------*/
        case KEY_ALTBACK:
            if( current_op != OP_NOTHING )
                break;

            undo_undo();
            break;

/*-------------------------------------------------------------------------*
 * Alt+W: Wad management
 *-------------------------------------------------------------------------*/
        case 0x1100:
            wad_mgmt();
            break;

/*-------------------------------------------------------------------------*
 * ALT-G+: Copy marked objects to the clipboard                            *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLINS:
            if( current_op != OP_NOTHING )
                break;
            if( tog_showclip )
                cb_draw( -1, -1 );   /* erase it cuz we're putting it back */
            if( (tmpint = cb_clip( editmode, iCurObject )) )
            {
                SetStatusMsg( "%d object(s) copied to clipboard",
                            tmpint );
                /* Clear marks.. */
                obj_clearmarks( iCurObject, 1 );
                if( tog_showclip )
                    cb_draw( cursorx, cursory );
            }
            break;

/*-------------------------------------------------------------------------*
 * HOME/END: Mode / copy/move                                              *
 *-------------------------------------------------------------------------*/
        case KEY_HOME:
        case KEY_END:
            if( current_op == OP_NOTHING )
            {
                /* Doing nothing, so change modes. */
                switch( editmode )
                {
                case MODE_THING:
                    keystuff( ch == KEY_END ? 'L' : 'S' );
                    break;
                //case MODE_VERTEX:
                //    keystuff( ch == KEY_END ? 'L' : 'T' );
                //    break;
                case MODE_LINE:
                    keystuff( ch == KEY_END ? 'S' : 'T' );
                    break;
                case MODE_SECTOR:
                    keystuff( ch == KEY_END ? 'T' : 'L' );
                    break;
                }
            }
            else if( current_op == OP_DRAG && editmode == MODE_THING )
            {
                extern short manip_copy;
                manip_copy = !manip_copy;
                SetStatusMsg( "Object %s mode ON: Release left"
                                 " button to drop object(s)",
                                  manip_copy ? "drag" : "copy" );
            }
            break;

/*-------------------------------------------------------------------------*
 * ALT-G-: Cut marked objects to the clipboard                             *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLDEL:
            if( current_op != OP_NOTHING )
                break;
            if( (tmpint = cb_clip( editmode, iCurObject )) )
            {
                SetStatusMsg( "%d object(s) cut to clipboard",
                            tmpint );
                /* Delete marked objects */
                obj_delete( editmode, objCursor, 1 );
                post_fullupdate();
            }
            break;

/*-------------------------------------------------------------------------*
 * ALT-G*: Paste clipboard to screen                                       *
 *-------------------------------------------------------------------------*/
        case KEY_INS:
            if( current_op != OP_NOTHING )
                break;
            if( (tmpint = cb_paste( editmode, xstart + _scrx2map( cursorx ),
                                    ystart + _scry2map( cursory ))) )
            {
                SetStatusMsg( "%d object(s) pasted from clipboard", tmpint );
                if( tog_showclip )      /* No need to "un-display" it cuz it's
                                         * a full update */
                    tog_showclip = 0;
            }
            else
                SetStatusMsg( "Clipboard is empty" );
            break;

/*-------------------------------------------------------------------------*
 * O: place thing origin in thing mode                                     *
 *-------------------------------------------------------------------------*/
        case 'O':
            if( editmode != MODE_THING )
                break;

            if( torigin_x == -1 )
            {
                torigin_x = xstart + _scrx2map( cursorx );
                torigin_y = ystart + _scry2map( cursory );
            }
            else
                torigin_x = -1;
            post_fullupdate();

            break;

/*-------------------------------------------------------------------------*
 * Ctrl+W: Align
 *-------------------------------------------------------------------------*/
        case CTRL+'W':
            if( current_op == OP_NOTHING )
            {
                SetStatusMsg( "Aligning textures..." );
                UpdateMessageBar();
                if( editmode == MODE_SECTOR )
                    sector_align( csr_object );
                else
                {
#if 0
                    if( obj_nmarked() < 2 )
                    {
                        message_box( MB_OK, st_dcksays,
                          _ABBREV_ " will align the textures on any series of lines\n"
                          "that you mark.  Mark the objects you want to align and\n"
                          "press Ctrl+W again." );
                    }
                    else
#endif
                    line_align( iCurObject );
                }
                SetStatusMsg( st_blank );
                UpdateMessageBar();
            }
            break;

/*-------------------------------------------------------------------------*
 * Ctrl+Z: StairZzzZZZzZ                                                   *
 *-------------------------------------------------------------------------*/
        case CTRL+'Z':
            if( current_op == OP_NOTHING )
                stair_do();
            break;

/*-------------------------------------------------------------------------*
 * 'D': Thing display flags
 *-------------------------------------------------------------------------*/
        case 'D':
            if( editmode == MODE_THING )
            {
                if( td_flags.use )
                {
                    td_flags.use = 0;
                    post_fullupdate();
                }
                else
                {
                    if( td_getflags() )
                    {
                        td_flags.use = 1;
                        post_fullupdate();
                    }
                }
            }
            break;

/*-------------------------------------------------------------------------*
 * ctrl-left and ctrl-right: rotate things
 *-------------------------------------------------------------------------*/
        case KEY_CLEFT:
        case KEY_CRIGHT:

            if( editmode == MODE_THING && current_op == OP_NOTHING &&
                csr_object != -1 )
            {
                THING *tdp = &thinglist[csr_object];

                if( ch == KEY_CLEFT )
                    tdp->angle += 45;
                else
                    tdp->angle -= 45;

                if( tdp->angle < 0 )
                    tdp->angle += 360;

                tdp->angle %= 360;

                post_fullupdate();
            }

            break;

/*-------------------------------------------------------------------------*
 * CTRL-V: Snap marked objects                                             *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLV:
            if( current_op == OP_NOTHING )
            {
                tmpint = obj_snapmarked( objCursor );
                SetStatusMsg( "%d object(s) snapped to grid", tmpint );
                if( tmpint )
                    post_fullupdate();
            }
            break;

/*-------------------------------------------------------------------------*
 * CTRL+G: Center map                                                      *
 *-------------------------------------------------------------------------*/
        case 0x0007:
            CenterMap();
            break;

/*-------------------------------------------------------------------------*
 * CTRL-R: Flip sidedefs only                                              *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLR:
            if( current_op != OP_NOTHING )
                break;
            if( editmode == MODE_LINE )
            {
                tmpint = line_flip( iCurObject, 0 );
                SetStatusMsg( "%d line(s) flipped (sidedefs only)", tmpint );
                if( tmpint )
                    post_fullupdate();
            }
            break;

/*-------------------------------------------------------------------------*
 * ALT-G: Go to object                                                     *
 *-------------------------------------------------------------------------*/
        case KEY_ALTG:
            go_obj = GetJumpObject();
            break;

/*-------------------------------------------------------------------------*
 * ALT-C: COnsistency checker                                              *
 *-------------------------------------------------------------------------*/
        case KEY_ALTC:
            cs_run( 0 );
            break;

/*-------------------------------------------------------------------------*
 * CTRL-I: Fix textures                                                    *
 * CTRL-U: Fix unused textures                                             *
 *-------------------------------------------------------------------------*/
        case CTRL+'I':
            if( current_op == OP_DRAG || current_op == OP_CREATELINE )
            {
                if( m_showlength == GS_ON )
                    m_showlength = GS_LAST;
                else
                    m_showlength = GS_FIRST;

                // erase
                obj_drawmaniplist( -1, -1, -1, -1 );
                // redraw
                obj_drawmaniplist( cursorx, cursory, -1, -1 );

                break;
            }

        case CTRL+'U':      /* Remove unused textures */
            tmpint = (ch == CTRL+'U') ? 1 : 0;
            if( current_op != OP_NOTHING )
                break;
            if( editmode == MODE_LINE || editmode == MODE_SECTOR )
            {
                short mode = tmpint;
                unmarkobj = 0;

                if( iCurObject != -1 && !obj_ismarked( iCurObject ) )
                    obj_mark( iCurObject ), unmarkobj = 1;

                tmpint = (editmode == MODE_LINE) ? nline : nsector;

                for( i = 0; i < tmpint; i++ )
                {
                    if( obj_ismarked( i ) )
                    {
                        if( editmode == MODE_LINE )
                        {
                            _WFfixtexnames( i, 0, mode );
                            if( linelist[i].attribs & LA_TSIDE )
                                _WFfixtexnames( i, 1, mode );
                        }
                        else if( editmode == MODE_SECTOR )
                            sect_fixtex( i, mode );
                    }
                }

                if( unmarkobj )
                    obj_unmark( iCurObject );

                post_fullupdate();
            }
            break;

/*-------------------------------------------------------------------------*
 * Ctrl+N: Normalize lines                                                 *
 *-------------------------------------------------------------------------*/
        case CTRL+'N':
            if( current_op != OP_NOTHING )
                break;

            if( editmode == MODE_LINE )
            {
                tmpint = obj_linenormalize( iCurObject );
                SetStatusMsg( "%d line(s) normalized", tmpint );

                if( tmpint )
                    post_fullupdate();
            }
            else if( editmode == MODE_SECTOR )
            {
                tmpint = obj_sectnormalize( iCurObject );
                SetStatusMsg( "%d sector(s) normalized", tmpint );

                if( tmpint )
                    post_fullupdate();
            }
            else if( editmode == MODE_THING )
            {
                tmpint = obj_thingnormalize( iCurObject );
                SetStatusMsg( "%d Item(s) normalized", tmpint );

                if( tmpint )
                    post_fullupdate();
            }

            break;

/*-------------------------------------------------------------------------*
 * CTRL-F: Flip linedefs and sidedefs                                      *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLF:
            if( current_op != OP_NOTHING )
                break;
            if( editmode == MODE_LINE )
            {
                tmpint = line_flip( iCurObject, 1 );
                SetStatusMsg( "%d line(s) flipped (sides and vertices)",
                    tmpint );
                if( tmpint )
                    post_fullupdate();
            }
            break;

/*-------------------------------------------------------------------------*
 * Ctrl+P: Paint !                                                         *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLP:
            if( current_op != OP_NOTHING )
                break;

            if( iObjectType == MODE_VERTEX )
            {
                message_box( MB_OK, st_dcksays,
                 "The format painter doesn't work in vertex mode." );
                break;
            }

            if( !obj_nmarked() )    /* Can't do it w/o marked objects */
            {
                message_box( MB_OK, "Notice",
                             "You must mark some objects before using the\n"
                             "format painter." );
                break;
            }

            current_op = OP_FPAINTER;
            SetStatusMsg( "Select source object, then "
                                "[LButton] to Set; [RButton] to abort" );

            if( iCurObject != -1 )
            {
                /* Do it */
                tmpint = fpainter( editmode, iCurObject );

                current_op = OP_NOTHING;

                if( tmpint )
                    post_fullupdate();

                SetStatusMsg( "%d copy(s) made", tmpint );
            }

            break;

/*-------------------------------------------------------------------------*
 * CTRL-J: Join lines (del vertex)                                         *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLJ:
            if( current_op != OP_NOTHING )
                break;

            if( iObjectType == MODE_VERTEX )
            {
                tmpint = obj_delvjoinline( csr_object );
                SetStatusMsg( "%d line(s) joined", tmpint );
                if( tmpint )
                    post_fullupdate();
            }
            else if( iObjectType == MODE_LINE )
            {
                tmpint = obj_delljoinsectors( iCurObject );
                SetStatusMsg( "%d sector(s) joined", tmpint );
                if( tmpint )
                    post_fullupdate();
            }
            break;

/*-------------------------------------------------------------------------*
 * Ctrl+T: Set tags.                                                       *
 *-------------------------------------------------------------------------*/
        case CTRL+'T':
        case CMD_TAGTOTHING:
            if( current_op != OP_NOTHING )
                break;
            if( !(editmode & (MODE_LINE | MODE_SECTOR | MODE_THING)) )
            {
                message_box( MB_OK, "Notice",
                    "You must be in Sector, Item or Line mode to use the\n"
                    "Set Tags function." );
                break;
            }

            if( !obj_nmarked() && iCurObject == -1 )
            {
                message_box( MB_OK, st_dcksays,
                    "You have to mark some objects before using the\n"
                    "Set Tags function." );
                break;
            }

            unmarkobj = 0;
            obj_marked = iCurObject;

            if( obj_marked != -1 && !obj_ismarked( obj_marked ) )
                obj_mark( obj_marked ), unmarkobj = 1; /* So we set this one too. */

            current_op = OP_TAGSET;
            SetStatusMsg( "Set tags: Move to the object to tag to, then"
                                  " [LButton] to Set; [RButton] to abort." );
            /* Switch modes. */
            iTagDestMode = editmode;

            if( editmode == MODE_SECTOR )
                SetEditMode( MODE_LINE );
            else if( editmode == MODE_THING && fHexen )
                SetEditMode( MODE_LINE );
            else if( editmode == MODE_LINE )
            {
                if( ch == CMD_TAGTOTHING )
                    SetEditMode( MODE_THING );
                else
                    SetEditMode( MODE_SECTOR );
            }
            else
            {
                current_op = OP_NOTHING;
                SetStatusMsg( st_blank );
            }
            break;

/*-------------------------------------------------------------------------*
 * Ctrl+B: Combine lines.                                                  *
 *-------------------------------------------------------------------------*/
        case CTRL+'B':
            if( editmode != MODE_LINE ) /* Line mode, duh. */
                break;
            if( current_op == OP_NOTHING )
            {
                if( !obj_lstartcombine( iCurObject ) )
                    break;
                SetStatusMsg( "Move to the other line, then [Ctrl+B] to Combine; [ESC] to Abort" );
                current_op = OP_COMBLINE;
            }
            else if( current_op == OP_COMBLINE )
            {
                undo_init();
                Object obj( obj_lfinishcombine( iCurObject ), MODE_LINE );
                obj_delete( MODE_LINE, obj, 0 );
                map_purge( MODE_ALL );
                undo_setvars();
                current_op = OP_NOTHING;
            }
            break;

/*-------------------------------------------------------------------------*
 * ',' and '.' .. decrease/increase leniency                               *
 *-------------------------------------------------------------------------*/
        case ',':
        case '.':
            sel_lenience += (ch == ',' ? -1 : 1);
            SetStatusMsg( "Leniency variance: %d", sel_lenience );
            break;

/*-------------------------------------------------------------------------*
 * ALT-1: Edit sidedef 1                                                   *
 *-------------------------------------------------------------------------*/
        case KEY_ALT1:
        case KEY_ALT2:
            if( current_op != OP_NOTHING )
                break;
            if( editmode == MODE_LINE )
                LinedefEdit( iCurObject, (ch == KEY_ALT1) ? 1 : 2 );
            break;

/*-------------------------------------------------------------------------*
 * B: Show clipboard rectangle                                             *
 *-------------------------------------------------------------------------*/
        case 'B':
            if( !cb_isempty() )
            {
                tog_showclip = !tog_showclip;
                if( tog_showclip )
                    cb_draw( cursorx, cursory );
                else
                    cb_draw( -1, -1 );      /* erase it if turned off */
            }
            else
                SetStatusMsg( "Clipboard is empty" );
            break;

/*-------------------------------------------------------------------------*
 * menu jumps
 *-------------------------------------------------------------------------*/
        case KEY_ALTF:
            mnu_main->Run( 0 );
            break;
        case KEY_ALTD:
            mnu_main->Run( 1 );
            break;
        case KEY_ALTO:
            mnu_main->Run( 2 );
            break;
        case KEY_ALTI:
            mnu_main->Run( 3 );
            break;
        case KEY_ALTT:
            keystuff( KEY_ENTER );
            mnu_main->Run( 4 );
            break;
        case KEY_ALTL:
            mnu_main->Run( 5 );
            break;
        case KEY_ALTH:
            mnu_main->Run( 6 );
            break;

/*-------------------------------------------------------------------------*
 * ALT-N: Configuration
 *-------------------------------------------------------------------------*/
        case KEY_ALTN:
            getconfig();
            break;

/*-------------------------------------------------------------------------*
 * ALT-M: Motifs!                                                          *
 *-------------------------------------------------------------------------*/
        case 0x3200: // case KEY_ALTM:
            motif_pick();
            break;

/*-------------------------------------------------------------------------*
 * Q and W: number of sides for polygon
 *-------------------------------------------------------------------------*/
        case 'Q':
        case 'W':
            if( editmode != MODE_LINE || iCreateObject != coPolygon )
                break;

            if( ch == 'Q' && pr_npoints > 3 )
                --pr_npoints;
            else if( ch == 'W' && pr_npoints < 64 )
                ++pr_npoints;

            /* update this */
            if( current_op == OP_POLYGON )
                pr_stat = GS_UPDATE;

            //si_str( siToolInfo, "%d sides", pr_npoints );
            //sb_setupdate( sbMain, 1 );

            break;

/*-------------------------------------------------------------------------*
 * A and E: angle of polygon
 *-------------------------------------------------------------------------*/
        case 'A':
        case 'E':
            if( current_op != OP_POLYGON )
                break;

            tmpint = tog_fine ? 1 : 15;

            if( ch == 'A' )
                poly_startangle -= tmpint;
            else if( ch == 'E' )
                poly_startangle += tmpint;

            if( poly_startangle < 0 )
                poly_startangle += 360;
            else if( poly_startangle > 359 )
                poly_startangle -= 360;

            pr_stat = GS_UPDATE;

            break;

/*-------------------------------------------------------------------------*
 * ALT-R: Replace                                                          *
 *-------------------------------------------------------------------------*/
        case KEY_ALTR:
            if( current_op != OP_NOTHING )
                break;

            if( editmode == MODE_LINE )
            {
                tmpint = side_replacetex( iCurObject );
                SetStatusMsg( "%d texture(s) replaced", tmpint );
            }
            break;

/*-------------------------------------------------------------------------*
 * `: Toggle fine move                                                     *
 *-------------------------------------------------------------------------*/
        case '`':
            //sb_setupdate( sbMain, 1 );
            tog_fine = !tog_fine;
            SetStatusMsg( "Fine mode %s", st_onoff( tog_fine ) );
            break;

/*-------------------------------------------------------------------------*
 * ENTER: Multi-purpose                                                    *
 *-------------------------------------------------------------------------*/
        case KEY_ENTER:
            if( current_op != OP_NOTHING )
            {
                switch( current_op )
                {
                case OP_ROTATE:
                case OP_SCALE:
                    obj_exitmanip( 1 );
                    current_op = OP_NOTHING;
                    post_fullupdate();
                    break;
                }
                SetStatusMsg( "" );
                break;
            }
            if( csr_object != -1 || obj_nmarked() )
            {
                if( editmode == MODE_THING )
                {
                    iLastThingEdited = csr_object;
                    thing_edit( csr_object, 1 );
                }
                else if( editmode == MODE_LINE )
                    LinedefEdit( iCurObject, -1 );
                else if( editmode == MODE_SECTOR )
                    sector_edit( csr_object, 1 );
                donothing = 0;
            }
            break;

/*-------------------------------------------------------------------------*
 * J: Join mode on/off                                                     *
 *-------------------------------------------------------------------------*/
        case 'J':
            //sb_setupdate( sbMain, 1 );
            tog_join = !tog_join;
            SetStatusMsg( "Autojoin mode %s", st_onoff( tog_join ) );
            break;

/*-------------------------------------------------------------------------*
 * CTRLX/Y: Align on axis                                                  *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLX:
        case KEY_CTRLY:
            if( current_op != OP_NOTHING )
                break;
            tmpint = obj_align( csr_object, ch == KEY_CTRLY );
            SetStatusMsg( "%d object(s) aligned on the %c axis",
                        tmpint, ch );
            post_fullupdate();
            break;

/*-------------------------------------------------------------------------*
 * CTRL-D: make door from sector                                           *
 * CTRL+L: Lift                                                            *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLL:
            if( current_op != OP_NOTHING || editmode != MODE_SECTOR )
                break;

            CreateLiftFromSector( csr_object );

            break;
        case CTRL+'D':
            if( current_op != OP_NOTHING || editmode != MODE_SECTOR )
                break;

            CreateDoorFromSector( csr_object );

            break;

/*-------------------------------------------------------------------------*
 * CTRL-E: Set dEfault textures on lines.                                  *
 *-------------------------------------------------------------------------*/
        case CTRL+'E':
            if( current_op != OP_NOTHING )
                break;

            if( obj_nmarked() > 1 )
            {
                if( message_box( MB_YESNO, st_dcksays,
                    "Are you sure you want to apply the current motif to all the\n"
                    "marked objects?  This process is not reversible." ) == MB_NO )
                    break;
            }

            if( editmode == MODE_LINE )
                tmpint = obj_linesetmotif( iCurObject );
            else if( editmode == MODE_SECTOR )
                tmpint = obj_sectsetmotif( csr_object );
            else
                break;
            SetStatusMsg( "%d texture(s) replaced", tmpint );
            break;

/*-------------------------------------------------------------------------*
 * 8-*: Cursor size toggle                                                 *
 *-------------------------------------------------------------------------*/
        case '*':
            tog_csize = !tog_csize;
            ch_stat = (ch_stat == GS_ON) ? GS_LAST : GS_FIRST;
            break;

/*-------------------------------------------------------------------------*
 * V/L/S/T: Toggle edit mode                                               *
 *-------------------------------------------------------------------------*/
        //case 'V':
        //    tmpint = MODE_VERTEX;
        //    goto _changemode;
        case 'L':
            tmpint = MODE_LINE;
            goto _changemode;
        case 'S':
            tmpint = MODE_SECTOR;
            goto _changemode;
        case 'R':
            tmpint = MODE_RECTANGLE;
            goto _changemode;
        case 'N':
            tmpint = MODE_POLYGON;
            goto _changemode;
        case 'T':
        case 'I':
            if( editmode == MODE_THING )
            {
                /* maybe just toggle thing graphics */
                if( tog_tgraph || config.tgr_default )
                {
                    tog_tgraph = !tog_tgraph;
                    post_fullupdate();
                    break;
                }
            }

            tmpint = MODE_THING;
_changemode:
            if( current_op != OP_NOTHING )
            {
                SetStatusMsg( "Can't switch edit modes while %s; "
                                      "finish first!", op_strings[current_op] );
                break;
            }

            // si_str( siToolInfo, st_blank );

            /* translate marked objects to the new edit mode */
            if( config.tog_markxlat )
                obj_markxlate( editmode, tmpint );
            else
                obj_clearmarks( iCurObject, 0 );

            SetEditMode( tmpint );

            break;

/*-------------------------------------------------------------------------*
 * ESC KEY                                                                 *
 *-------------------------------------------------------------------------*/
        case KEY_ESC:
            if( current_op != OP_NOTHING )
            {
                short abortable = 1;

                /* Abort the current operation */
                switch( current_op )
                {
                case OP_RECTANGLE:
                case OP_POLYGON:
                    pr_stat = GS_LAST;
                    break;
                case OP_SCALE:
                case OP_ROTATE:
                    obj_drawmaniplist( -1, -1, rotate_angle, scale_factor );
                    obj_exitmanip( 0 );
                    csr_object = -1;
                    tog_drag = 0;
                    break;
                case OP_CREATELINE:
                    lc_abort();
                    post_fullupdate();
                    break;
                case OP_TAGSET:
                    SetEditMode( editmode == MODE_LINE ?
                                  MODE_SECTOR : MODE_LINE );
                    break;
                case OP_COMBLINE:
                    _lcomb_first = -1;
                    break;
                case OP_DRAG:
                    obj_drawmaniplist( -1, -1, -1, -1 );
                    obj_exitmanip( 0 );
                    tog_drag = 0;
                    break;
                default:
                    /* none of the above?  can't abort with ESC */
                    abortable = 0;
                    break;
                }

                if( abortable )
                {
                    current_op = OP_NOTHING;
                    SetStatusMsg( "" );
                }
            }
            else
            {
                /* The ESC key brings up the main menu system if nothing else
                 * is going on. */
                mnu_main->Run();

                if( fFullScreen )
                    mnu_main->Erase();
            }
            break;

/*-------------------------------------------------------------------------*
 * C: Clear marks                                                          *
 *-------------------------------------------------------------------------*/
        case 'C':
            if( current_op != OP_NOTHING )
                break;
            SetStatusMsg( st_omclear );
            obj_clearmarks( csr_object, 0 );
            post_fullupdate();
            break;

/*-------------------------------------------------------------------------*
 * G: Toggle Grid                                                          *
 *-------------------------------------------------------------------------*/
        case 'G':
            ++tog_grid;
            if( tog_grid == 3 )
                tog_grid = 0;
            // SetStatusMsg( "Grid mode: %s" );
            /* Full update */
            post_fullupdate();
            break;

/*-------------------------------------------------------------------------*
 * [ / ]: Grid decrease/increase                                           *
 *-------------------------------------------------------------------------*/
        case ']':
        case '[':
            if( ch == ']' && gridstep < 512 )
                gridstep += gridstep;
            else if( ch == '[' && gridstep > 2 )
                gridstep -= (gridstep / 2);
            SetStatusMsg( "Grid spacing: %d", gridstep );
            //sb_setupdate( sbMain, 1 );
            /* Full update */
            post_fullupdate();
            break;

/*-------------------------------------------------------------------------*
 * P: Snap to grid                                                         *
 *-------------------------------------------------------------------------*/
        case 'P':
            tog_snap = (tog_snap < 2) ? (tog_snap + 1) : 0;
            SetStatusMsg( "Snap to grid mode: %s", vstring[tog_snap] );
            //sb_setupdate( sbMain, 1 );
            /* If we're doing something, post a full update because most
             * likely what we're doing will be affected by the snap
             * decision (HAHAHAHHA snap decision!  get it?!) */
            if( current_op != OP_NOTHING )
                post_fullupdate();
            break;

/*-------------------------------------------------------------------------*
 * + / -: Increase/decrease zoom                                           *
 *-------------------------------------------------------------------------*/
        case '=':   /* so shift doesn't have to be pressed */
            ch = '+';
        case '+':                   /* Increase */
        case '-':                   /* Decrease! */
            if( (ch == '+' && xscale > 125) ||
                (ch == '-' && xscale < 20000) )
            {
                /* don't set it if -1 later */
                tmpint = -1;

                if( !tog_fine )
                {
                    if( (ch == '-' && xscale >= 2000) || xscale > 2000 )
                        tmpint = ch == '+' ? -1000 : 1000;
                    else if( xscale > 125 )
                        tmpint = ch == '+' ? -250 : 250;
                    else
                        ZoomAbs( 250 );
                }
                else
                {
                    if( xscale > 125 )
                        tmpint = ch == '+' ? -250 : 250;
                    else
                        ZoomAbs( 250 );
                }

                if( tmpint != -1 )
                    ZoomRelative( tmpint );
            }
            break;

/*-------------------------------------------------------------------------*
 * Z: Super-ZOOM!  Dun-daa!                                                *
 *-------------------------------------------------------------------------*/
        case 'Z':
            tmpint = abs( config._zoom_low - config._zoom_high ) / 3;

            if( xscale > tmpint )
                tmpint = config._zoom_low;
            else
                tmpint = config._zoom_high;

            ZoomAbs( tmpint );

            break;

/*-------------------------------------------------------------------------*
 * >/<: Multi-purpose.  Used in scaling and rotating.                      *
 *-------------------------------------------------------------------------*/
        case '>':
        case '<':
            /* Well, we aren't going to do it if we're not doing either
             * of the functions it's associated with. */
            if( current_op != OP_SCALE && current_op != OP_ROTATE )
                break;

            /* Erase the old list */
            obj_drawmaniplist( -1, -1, rotate_angle, scale_factor );

            /* Scaling! */
            if( current_op == OP_SCALE )
            {
                /* Adjust the scale factor and.. */
                if( tog_fine )
                    scale_factor += (ch == '>') ? 0.025 : -0.025;
                else
                    scale_factor += (ch == '>') ? 0.1 : -0.1;
            }
            /* Rotating! */
            else
            {
                if( tog_fine )
                    rotate_angle += (ch == '>') ? -1.0 : 1.0;
                else
                    rotate_angle += (ch == '>') ? -10.0 : 10.0;

                if( rotate_angle < 0 )
                    rotate_angle = 360 + rotate_angle;
                if( rotate_angle > 360 )
                    rotate_angle -= 360;
            }

_updatescalerotate:
            /* Draw the list with the new factors */
            if( current_op == OP_ROTATE )
                SetStatusMsg( "%s Rotation angle: %.1f", st_use_sr, rotate_angle );
            else
                SetStatusMsg( "%s Scale factor: %.3f", st_use_sr, scale_factor );
            obj_drawmaniplist( -1, -1, rotate_angle, scale_factor );

            break;

/*-------------------------------------------------------------------------*
 * Split a line in the middle                                              *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLC:      /* Ctrl-S w/mouse driver screws up?? */
            if( current_op != OP_NOTHING || editmode != MODE_LINE )
                break;
            if( (tmpint = obj_splitlines( iCurObject )) > 0 )
                post_fullupdate();
            SetStatusMsg( "%d lines split/created", tmpint );
            csr_object = -1;
            break;

/*-------------------------------------------------------------------------*
 * F1: Context-sensitive Help                                              *
 * (well, it will be =)                                                    *
 *-------------------------------------------------------------------------*/
        case KEY_F1:
            if( current_op == OP_NOTHING )
                run_help_menu();
            else switch( current_op )
            {
            case OP_PASTENEXTCLICK:
                help_disp( "PASTENEXTCLICK", 10 );
                break;
            case OP_DRAG:
                help_disp( "DRAG", 10 );
                break;
            case OP_CREATELINE:
                help_disp( "DESIGNING", 10 );
                break;
            case OP_SCALE:
                help_disp( "SCALEROTATE", 10 );
                break;
            case OP_MARKBOX:
                help_disp( "MARKBOX", 10 );
                break;
            case OP_COMBLINE:
                help_disp( "COMBLINE", 10 );
                break;
            case OP_FPAINTER:
                help_disp( "FPAINTER", 10 );
                break;
            case OP_TAGSET:
                help_disp( "TAGSET", 10 );
                break;
            case OP_RECTANGLE:
            case OP_POLYGON:
                help_disp( "RECTPOLY", 10 );
                break;
            }
            break;

/*-------------------------------------------------------------------------*
 * F9: Run and test the map.                                               *
 *-------------------------------------------------------------------------*/
        case KEY_F9:
            map_test();
            break;

/*-------------------------------------------------------------------------*
 * number-zoom                                                             *
 *-------------------------------------------------------------------------*/
        case '1':
            ZoomAbs( 10000 );
            break;
        case '2':
            ZoomAbs( 7500 );
            break;
        case '3':
            ZoomAbs( 5000 );
            break;
        case '4':
            ZoomAbs( 3500 );
            break;
        case '5':
            ZoomAbs( 2500 );
            break;
        case '6':
            ZoomAbs( 2000 );
            break;
        case '7':
            ZoomAbs( 1500 );
            break;
        case '8':
            ZoomAbs( 750 );
            break;
        case '9':
            ZoomAbs( 250 );
            break;
        case '0':
            ZoomAbs( 125 );
            break;

/*-------------------------------------------------------------------------*
 * CTRL-A/CTRL-O: Scale/rotate marked objects                              *
 *-------------------------------------------------------------------------*/
        case KEY_CTRLA:
        case KEY_CTRLO:
            if( (current_op == OP_ROTATE && ch == KEY_CTRLO) ||
                (current_op == OP_SCALE && ch == KEY_CTRLA) )
            {
                /* Erase the old list */
                obj_drawmaniplist( -1, -1, rotate_angle, scale_factor );
                /* Rotate/scale dialog box */
                GetScaleRotateValues( &scale_factor, &rotate_angle );
                goto _updatescalerotate;
            }
            if( current_op != OP_NOTHING )
                break;
            /* Initialize duplicate mode and set the current operation.. */
            objCursor.iNumber = csr_object;
            objCursor.iType = iObjectType;
            if( !obj_initmanip( editmode, cursorx, cursory, objCursor, 0, 0 ) )
                break;
            tog_drag = 1;
            current_op = ch == KEY_CTRLA ? OP_SCALE : OP_ROTATE;
            scale_factor = 1.0;
            rotate_angle = 0;
            goto _updatescalerotate;

            // break;

/*-------------------------------------------------------------------------*
 * Ctrl+H: Height chnage in sector mode                                    *
 *-------------------------------------------------------------------------*/
        case CTRL+'H':
            if( current_op != OP_NOTHING || editmode != MODE_SECTOR )
                break;

            tmpint = sect_changeheight( csr_object );
            SetStatusMsg( "%d object(s) changed", tmpint );

            break;

/*-------------------------------------------------------------------------*
 * DEL: Delete marked objects                                              *
 *-------------------------------------------------------------------------*/
        case KEY_DEL:
            if( current_op != OP_NOTHING )
                break;
            if( editmode == MODE_SECTOR && (biostime( 0, 0L )-_lastdel) > secdel_tics )
            {
                tmpint = message_box( MB_YESNO, "Confirm",
                             "Delete sector(s)?    " );
                if( tmpint == MB_NO )
                    break;
            }

            if( iObjectType == MODE_LINE && !obj_nmarked() && iCurObject != -1 )
            {
                /* join two-sided lines */
                if( linelist[iCurObject].lsdef != -1 && linelist[iCurObject].rsdef != -1 )
                {
                    obj_delljoinsectors( iCurObject );
                    SetStatusMsg( "The line was deleted and sectors joined" );
                    post_fullupdate();
                    break;
                }
            }
            else if( iObjectType == MODE_VERTEX && csr_object != -1 )
            {
                keystuff( KEY_CTRLJ );
                break;
            }

            _lastdel = biostime( 0, 0L );

            /* This might take awhile */
            SetStatusMsg( "Working..." );
            UpdateMessageBar();
            if( (tmpint = obj_delete( editmode, objCursor, 1 )) )
                post_fullupdate();
            SetStatusMsg( "%d object(s) deleted", tmpint );
            break;

/*-------------------------------------------------------------------------*
 * ALT-S: Save!                                                            *
 *-------------------------------------------------------------------------*/
        case KEY_ALTS:
            if( current_op != OP_NOTHING )
                break;
            file_save( FALSE, FALSE );
            break;

        case KEY_ALTA:
            if( current_op != OP_NOTHING )
                break;
            file_save( TRUE, FALSE );
            break;

/*-------------------------------------------------------------------------*
 * ALT-X: Exit!                                                            *
 *-------------------------------------------------------------------------*/
        case KEY_ALTX:        /* Alt-X */
            if( exit_confirm() == MA_EXIT )
                goto done;
            break;

/*-------------------------------------------------------------------------*
 * ALT-E: Edit a new map                                                   *
 *-------------------------------------------------------------------------*/
        case KEY_ALTE:
            if( check_save() && map_get() )
            {
                rvl = 1;
                goto done;
            }
            break;

/*-------------------------------------------------------------------------*
 * Movement                                                                *
 *-------------------------------------------------------------------------*/
        case KEY_LEFT:
        case KEY_RIGHT:
        case KEY_UP:
        case KEY_DOWN:
            MoveMapTo( ch );
            post_fullupdate();
            break;

/*-------------------------------------------------------------------------*
 * Mouse                                                                   *
 *-------------------------------------------------------------------------*/
        case -1:        /* Check the mouse ... */
            mouseop = mouse_event( &scx, &scy );
            donothing = 1;

/*-------------------------------------------------------------------------*
 * Moving has happened.  Just get the new coordinates of                   *
 * the mouse and stick them in the global cursor variables (the axes saved *
 * depends on which "stick" keys are held down, if any.  Stick keys        *
 * restrain movement to a particular axis -- CTRL for the Y axis, and      *
 * SHIFT for the X axis.                                                   *
 *-------------------------------------------------------------------------*/
            if( mouseop != MOUSE_NOTHING )
            {
                if( mouseop != MOUSE_MOVE )
                    lShiftDownTime = -1;

                globtomap( &cursorx, &cursory, scx, scy );

                /* Check for button bar and/or menu mouse presses.. */
                if( mouseop == MOUSE_LDOWN &&
                    scy < mypstart && current_op == OP_NOTHING )
                {
                    if( !fFullScreen &&
                        (tmpint = mnu_main->HitTest( Point( scx, scy ) )) != -1 )
                    {
                        if( current_op == OP_NOTHING )
                            mnu_main->Run( tmpint );
                    }
                    break;
                }

                //mouse_obj = FindObjectAt( cursorx, cursory, &iObjectType );
                //int iCurObject = mouse_obj;
                //
                //if( iObjectType != editmode )
                //    iCurObject = -1;
            }
            else
            {
                mouse_obj = -1;

                if( !mouse_zoom && config.tog_scrolly )
                {
                    donothing = 1;
                    scx = scursorx;
                    scy = scursory;

                    if( ((tmplong = _tics) - _lastscroll) > _scrollwait )
                    {
                        fScrolling = FALSE;

                        if( _firstscroll && (tmplong - _firstscroll) > getduration( 50 ) )
                            _scrollwait = getduration( 5 );

                        if( scx == 0 )
                            donothing = 0;
                        else if( scx >= sxlen )
                            donothing = 0;
                        if( scy == 0 )
                            donothing = 0;
                        else if( scy >= sylen )
                            donothing = 0;

                        /* if no scroll is happening, and it's at least a sec since
                         * the last scroll, reset it to "slow" */
                        if( donothing && (tmplong - _lastscroll) > getduration( 100 ) )
                        {
                            _firstscroll = 0;
                            _scrollwait = getduration( 10 );
                        }
                        else if( !donothing )
                        {
                            if( !_firstscroll )
                                _firstscroll = tmplong;
                            _lastscroll = tmplong;
                        }

                        /* now do the scrollies */
                        if( scx == 0 )
                        {
                            MoveMapTo( KEY_LEFT );
                            post_fullupdate();
                            fScrolling = TRUE;
                        }
                        else if( scx >= sxlen )
                        {
                            MoveMapTo( KEY_RIGHT );
                            post_fullupdate();
                            fScrolling = TRUE;
                        }
                        if( scy == 0 )
                        {
                            MoveMapTo( KEY_UP );
                            post_fullupdate();
                            fScrolling = TRUE;
                        }
                        else if( scy >= sylen )
                        {
                            MoveMapTo( KEY_DOWN );
                            post_fullupdate();
                            fScrolling = TRUE;
                        }
                    }
                }
            }

            if( isshift() && editmode == MODE_SECTOR )
            {
                if( !lastshift )
                    /* to force update.. eh. */
                    mouseop = MOUSE_MOVE;
                lastshift = 1;
                iCurObject = mouse_obj = -1;
                objCursor.SetInvalid();
            }
            else
            {
                if( lastshift )  /* force update */
                    mouseop = MOUSE_MOVE;
                lastshift = 0;
                mouse_obj = FindObjectAt( cursorx, cursory, objCursor );

                iObjectType = objCursor.iType;
                iCurObject = objCursor.iNumber;

                if( objCursor.iType != editmode )
                    iCurObject = -1;
            }

/*-------------------------------------------------------------------------*
 * Right mouse button                                                      *
 * The right mouse button, in all cases, edits the selected + marked       *
 * objects by calling the appropriate edit function for the edit mode      *
 * currently in use.  The edit function is responsible for re-storing the  *
 * data in each of the selected/marked objects.                            *
 *-------------------------------------------------------------------------*/
            if( mouseop == MOUSE_RUP )
            {
                mouse_obj = -1;
                SetStatusMsg( "" );

                if( current_op == OP_PASTENEXTCLICK ||
                    current_op == OP_FPAINTER )
                {
                    /* Abort it.. */
                    current_op = OP_NOTHING;
                    break;
                }
                else if( current_op == OP_CREATELINE )
                {
                    /* Save the current line, but don't start a new one
                     * (0). */
                    if( !lc_save( 0 ) )
                    {
                        /* We're done!  turn it off and redraw. */
                        current_op = OP_NOTHING;
                    }

                    post_fullupdate();

                    break;
                }
                else if( current_op == OP_TAGSET )
                {
                    SetEditMode( editmode == MODE_LINE ? MODE_SECTOR : MODE_LINE );
                    current_op = OP_NOTHING;
                    break;
                }
                else if( current_op == OP_NOTHING )
                {
                    if( editmode == MODE_THING )
                    {
                        /* Edit the cursor item or marked items */
                        iLastThingEdited = objCursor.iNumber;
                        thing_edit( objCursor.iNumber, 1 );
                        donothing = 0;
                    }
                    else if( editmode == MODE_LINE )
                    {
                        LinedefEdit( iCurObject, -1 );
                        donothing = 0;
                    }
                    else if( editmode == MODE_SECTOR )
                    {
                        sector_edit( objCursor.iNumber, 1 );
                        donothing = 0;
                    }
                }
            }

/*-------------------------------------------------------------------------*
 * left mouse button / single click                                        *
 *-------------------------------------------------------------------------*/
            else if( mouseop == MOUSE_LUP )
            {
                if( current_op == OP_PASTENEXTCLICK )
                {
                    current_op = OP_NOTHING;
                    keystuff( KEY_AGSTAR );
                    SetStatusMsg( st_blank );
                    break;
                }
                /* Creating a line?  Save it */
                else if( current_op == OP_CREATELINE )
                {
                    /* Start a new one by saving the current one with '1' */
                    if( !lc_save( 1 ) )
                    {
                        /* We're done!  turn it off and redraw. */
                        current_op = OP_NOTHING;
                        SetStatusMsg( st_blank );
                        post_fullupdate();
                    }
                    break;
                }
                /* Left click .. Mark/unmark object if on one. */
                else if( current_op == OP_NOTHING && iCreateObject == -1 )
                {
                    if( isctrl() && editmode == MODE_THING )
                    {
                        if( iLastThingEdited == -1 )
                            goto _AddThing;
                        else
                        {
                            // add last edited thing
                            _scr2map( &tmpx, &tmpy, cursorx, cursory );
                            int iNewThing = obj_insert( MODE_THING, iLastThingEdited,
                                tmpx, tmpy );
                        }

                        post_fullupdate();
                    }
                    else if( isctrl() && objCursor.iType == MODE_LINE )
                    {
                        // add a vertex here

                        // Create map coordinates from the cursor position..
                        // don't use the _scr2map() function because it uses snap
                        tmpx = xstart + _scrx2map( cursorx );
                        tmpy = ystart + _scry2map( cursory );

                        short s_nvertex = nvertex;

                        tmpint = _vc_lean;
                        _vc_lean = _scrx2map( 5 );

                        obj_insert( MODE_VERTEX, VTX_CREATEJOIN, tmpx, tmpy );

                        if( _vcj_line == -1 )   /* means it wasn't created on a line */
                        {
                            SetStatusMsg( "There's no line close by" );

                            /* delete it if one was created - obj_insert might have
                            * returned an existing vertex because of vtx_createjoin */
                            if( nvertex > s_nvertex )
                                --nvertex;

                            UpdateMessageBar();
                        }
                        else
                        {
                            SetStatusMsg( "Vertex created" );
                            post_fullupdate();
                        }

                        _vc_lean = tmpint;
                    }
                    else if( objCursor.IsValid() )
                    {
                        cursor_put( CSR_ERASE );

                        if( !obj_ismarked( objCursor ) )
                        {
                            // if shift is not down, and it has to be
                            // to add to the marks, then clear the current
                            // marks
                            if( !isshift() && config.tog_shiftadd )
                                obj_clearmarks( objCursor.iNumber, 1 );
                            obj_mark( objCursor );
                        }
                        else
                        {
                            obj_unmark( objCursor );
                        }

                        if( objCursor != objHighlighted )
                            obj_highlight( objCursor, COLOR_MARKED );
                        cursor_put( CSR_PUT );
                        donothing = 0;
                    }
                    else if( config.tog_shiftadd && obj_nmarked() )
                    {
                        SetStatusMsg( st_omclear );
                        obj_clearmarks( -1, 0 );
                        post_fullupdate();
                        donothing = 0;
                    }
                }
                else if( current_op == OP_COMBLINE && iCurObject != -1 )
                {
                    obj_lfinishcombine( iCurObject );
                    SetStatusMsg( st_blank );
                    current_op = OP_NOTHING;
                }
                else if( current_op == OP_TAGSET )
                {
                    if( iCurObject != -1 )
                    {
                        tmpint = obj_tagset( iCurObject, editmode, iTagDestMode );
                        current_op = OP_NOTHING;
                        if( tmpint )
                            post_fullupdate();
                        SetStatusMsg( "%d tag(s) set" );
                        SetEditMode( iTagDestMode );
                        SetStatusMsg( st_blank );

                        if( unmarkobj )
                            obj_unmark( obj_marked );
                    }
                    else
                        message_box( MB_OK, "Tag-setting help",
                         "Move to the object to share the tags with and click\n"
                         "the left mouse button.  To abort, click the right\n"
                         "button or press ESC." );
                }
                else if( current_op == OP_FPAINTER )
                {
                    /* Do it */
                    tmpint = fpainter( editmode, iCurObject );

                    current_op = OP_NOTHING;

                    if( tmpint )
                        post_fullupdate();

                    SetStatusMsg( "%d copy(s) made", tmpint );
                }
                mouseop = MOUSE_NOTHING;
            }

/*-------------------------------------------------------------------------*
 * Left mouse button double click                                          *
 *-------------------------------------------------------------------------*/
            else if( mouseop == MOUSE_LDBLCLICK )
            {
                if( current_op != OP_NOTHING )
                    break;

                /* Create map coordinates from the cursor position */
_AddThing:
                _scr2map( &tmpx, &tmpy, cursorx, cursory );

                if( editmode == MODE_THING )
                {
                    /* Create a new thing, of course! */
                    tmpint = obj_insert( MODE_THING, iLastThingEdited, tmpx, tmpy );

                    if( thinglist[tmpint].angle % 45 )
                        thinglist[tmpint].angle = 0;

                    thing_autosetangle( tmpint );
                    if( !thing_edit( tmpint, 0 ) )
                        --nthing;
                    else
                    {
                        iLastThingEdited = tmpint;
                        post_fullupdate();
                    }
                }
                else if( editmode == MODE_LINE )
                {
                    /* turn off highlight if there is one */
                    if( csr_object != -1 )
                        obj_highlight( iObjectType, csr_object, COLOR_CSRSEL );

                    /* Start a line at this position */
                    lc_start( tmpx, tmpy );
                    SetStatusMsg( "Line creation: [LButton] to continue;"
                                          " [RButton] to save and create region" );
                    current_op = OP_CREATELINE;
                }
            }
            else if( mouseop == MOUSE_RDBLCLICK && current_op == OP_NOTHING )
            {
                tmpint = MenuPop( 0, " Edit mode ", NULL,
                                  " Line     ", NULL,
                                  " Vertex   ", NULL,
                                  " Thing    ", NULL,
                                  " Sector   ", NULL,
                                  NULL );
                if( tmpint == 0 )           keystuff( 'L' );
                else if( tmpint == 1 )      keystuff( 'V' );
                else if( tmpint == 2 )      keystuff( 'T' );
                else if( tmpint == 3 )      keystuff( 'S' );
            }

/*-------------------------------------------------------------------------*
 * Middle-zoom
 *-------------------------------------------------------------------------*/
            else if( mouseop == MOUSE_MDOWN &&
                (!fDemoPlaying && !fDemoRecording) )
            {
                mouse_zoom = 1;
                fScrolling = TRUE;
                mouse_setlock( 1, 1 );
            }
            else if( !mou_mdown && mouse_zoom )
            {
                mouse_setlock( 0, 0 );
                fScrolling = FALSE;
                mouse_zoom = 0;
            }

/*-------------------------------------------------------------------------*
 * Initialize dragging                                                     *
 * Action depends on cursor object :--                                     *
 *  1) If there's something there, calls obj_initmanip() to move all the   *
 *     marked objects.                                                     *
 *  2) Otherwise, a mark box is started with the coordinates of the        *
 *     cursor.                                                             *
 *-------------------------------------------------------------------------*/
            else if( mouseop == MOUSE_RDOWN && mou_rdown == 1 && current_op == OP_NOTHING )
            {
                if( runcursorpad() == 0 )
                {
                    /* this is very bad, but no matter */
                    mouseop = MOUSE_RDRAGINIT;
                    mou_rdrag = 1;
                    goto _domarkbox;
                }
            }

            else if( mouseop == MOUSE_LDRAGINIT ||
                     mouseop == MOUSE_LDBLDRAGINIT )
            {
                /* Can't do this while doing something else */
                if( current_op != OP_NOTHING )
                    break;

                if( editmode == MODE_LINE && iCreateObject == coPolygon )
                {
                    if( objHighlighted.IsValid() )
                    {
                        obj_highlight( objHighlighted, COLOR_CSRSEL );
                        if( obj_ismarked( objHighlighted ) )
                            obj_highlight( objHighlighted, COLOR_MARKED );
                    }

                    /* start creating that thar polygon */
                    current_op = OP_POLYGON;
                    pr_stat = GS_FIRST;
                    //sb_setupdate( sbMain, 1 );
                    UpdateMessageBar();
                    SetStatusMsg( "Release [LButton] to finish; hold down [RButton] to move the polygon." );
                    pr_x1 = xstart + _scrx2map( cursorx );
                    pr_y1 = ystart + _scry2map( cursory );
                    pr_x2 = xstart + _scrx2map( cursorx );
                    pr_y2 = ystart + _scry2map( cursory );
                    break;
                }
                else if( editmode == MODE_LINE && iCreateObject == coRectangle )
                {
                    if( objHighlighted.IsValid() )
                    {
                        obj_highlight( objHighlighted, COLOR_CSRSEL );
                        if( obj_ismarked( objHighlighted ) )
                            obj_highlight( objHighlighted, COLOR_MARKED );
                    }

                    /* start creating that thar region */
                    current_op = OP_RECTANGLE;
                    pr_stat = GS_FIRST;
                    //sb_setupdate( sbMain, 1 );
                    UpdateMessageBar();
                    SetStatusMsg( "Release [LButton] to finish; hold down [RButton] to move the rectangle." );
                    pr_x1 = xstart + _scrx2map( cursorx );
                    pr_y1 = ystart + _scry2map( cursory );
                    pr_x2 = xstart + _scrx2map( cursorx );
                    pr_y2 = ystart + _scry2map( cursory );
                    break;
                }

                if( scy < mypstart )
                    break;

                if( objCursor.IsValid() )
                {
                    if( objHighlighted.IsValid() )
                    {
                        obj_highlight( objHighlighted, COLOR_CSRSEL );
                        if( obj_ismarked( objHighlighted ) )
                            obj_highlight( objHighlighted, COLOR_MARKED );
                    }

                    /* This might take some time on slow computers..
                     * and it does!  :) */
                    SetStatusMsg( "Please wait: Building manipulation list..." );
                    tog_drag = 1;
                    current_op = OP_DRAG;
                    //sb_setupdate( sbMain, 1 );
                    UpdateMessageBar();
                    obj_initmanip( editmode, cursorx, cursory, objCursor,
                                   mouseop == MOUSE_LDBLDRAGINIT, 1 );
                    obj_drawmaniplist( cursorx, cursory, -1, -1 );
                    SetStatusMsg( "Object %s mode ON: Release left"
                                          " button to drop object(s)",
                                          mouseop == MOUSE_LDRAGINIT ?
                                          "drag" : "copy" );
                }
                else
                {
_domarkbox:
                    /* Set mode (mark/unmark) */
                    mbox_mode = (mouseop == MOUSE_LDRAGINIT) ? MBOX_MARK : MBOX_UNMARK;

                    /* Create initial coordinates */
                    SetStatusMsg( "Block %smark mode ON: Release left"
                             " button to turn it off.",
                             mbox_mode == MBOX_MARK ? "" : "un" );
                    mb_x1 = xstart + _scrx2map( cursorx );
                    mb_y1 = ystart + _scry2map( cursory );
                    current_op = OP_MARKBOX;
                    mb_stat = GS_FIRST;
                }
                donothing = 0;
            }

/*-------------------------------------------------------------------------*
 * Exit dragging                                                           *
 * Action depends on what is being performed with the mouse (set at        *
 * MOUSE_LDRAGINIT) :--                                                    *
 *  1) dragging objects: the objects are "saved" in memory at their new    *
 *     location and obj_exitmanip() is called.                             *
 *  2) mark box: the mark box is "saved" by calling obj_markbox() with     *
 *     the coordinates of the mark box.                                    *
 *-------------------------------------------------------------------------*/
            else if( mouseop == MOUSE_LDRAGEXIT ||
                     mouseop == MOUSE_RDRAGEXIT )
            {
                if( current_op == OP_DRAG )
                {
                    /* Drag mode: Save drag changes */
                    obj_exitmanip( 1 );
                    if( csr_object != -1 && obj_nmarked() == 1 )
                        obj_unmark( csr_object );
                    SetStatusMsg( "" );
                    /* Full update */
                    post_fullupdate();
                    tog_drag = 0;
                    current_op = OP_NOTHING;
                    SetStatusMsg( st_blank );
                }
                else if( current_op == OP_MARKBOX )
                {
                    if( objHighlighted.IsValid() )
                    {
                        obj_highlight( objHighlighted, COLOR_CSRSEL );
                        if( obj_ismarked( objHighlighted ) )
                            obj_highlight( objHighlighted, COLOR_MARKED );
                    }

                    /* MBox mode: mark in the box */
                    if( mbox_mode == MBOX_MARK )
                    {
                        if( config.tog_shiftadd && !isshift() )
                            obj_clearmarks( -1, 1 );
                    }

                    tmpint = obj_markbox( mb_x1, mb_y1, mb_x2, mb_y2,
                        mbox_mode );

                    if( mbox_mode == MBOX_MARK )
                    {
                        if( !tmpint && config.tog_shiftadd && obj_nmarked() )
                        {
                            SetStatusMsg( st_omclear );
                            obj_clearmarks( -1, 0 );
                            post_fullupdate();
                        }
                    }

                    SetStatusMsg( "%d object(s) %smarked", tmpint,
                                mbox_mode == MBOX_MARK ? "" : "un" );

                    // turn back on highlight
                    if( objHighlighted.IsValid() )
                    {
                        obj_highlight( objHighlighted, COLOR_CSRSEL );
                        if( obj_ismarked( objHighlighted ) )
                            obj_highlight( objHighlighted, COLOR_MARKED );
                    }

                    //sb_setupdate( sbMain, 1 );
                    mb_stat = GS_LAST;
                    current_op = OP_NOTHING;
                }
                else if( current_op == OP_SOMETHING )
                {
                    SetStatusMsg( st_blank );
                    current_op = OP_NOTHING;
                }
                else if( mouseop == MOUSE_LDRAGEXIT && (current_op == OP_POLYGON || current_op == OP_RECTANGLE) )
                {
                    PolyScr2Map( pmPoints, current_op == OP_RECTANGLE ? 4 : pr_npoints, tog_snap, TRUE );
                    CreatePolyRegion( pmPoints, current_op == OP_RECTANGLE ? 4 : pr_npoints );
                    SetStatusMsg( st_blank );
                    current_op = OP_NOTHING;
                    pr_stat = GS_OFF;
                    post_fullupdate();  /* fulllllll update. */
                }
                mouseop = MOUSE_NOTHING;
                donothing = 0;
            }
            else if( mouseop == MOUSE_MOVE )
            {
                cso_tmp = mouse_obj;
                donothing = 0;
            }

            if( mouse_zoom && mou_deltay )
            {
                if( ZoomRelative( mou_deltay * 20 ) )
                {
                    cso_tmp = -1;
                    post_fullupdate();
                    fScrolling = TRUE;
                }
            }

            break;
        }
    }

done:
    if( rvl == 0 && iExitLev == EL_EXIT )
    {
        /* Display exit message */
        gwnp = wn_def( Centered, Centered, 27, 3, C_GRAY3, C_GRAY1,
                       WF_POPUP | WF_3D, st_dcksays );
        wn_disp( gwnp );
        cursor_put( CSR_ERASE );
        wn_xytext( gwnp, Centered, 1, "Shutting Down" );
        cursor_put( CSR_PUT );
        wn_free( gwnp );    /* but leave it up */
    }

    tlb_st = tlb_st;    /* oh well */

    first_init = 0;
    return rvl;
}
