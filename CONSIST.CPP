/*-------------------------------------------------------------------------*
 * CONSIST.C:                                                              *
 * A consistency checker for DOOM maps.                                    *
 *                                                                         *
 * Here's how it works.  When a consistency error is found, cs_log         *
 * is called with specific information about the error:                    *
 *  - its type (index to the cs_table array)                               *
 *  - the object in question                                               *
 * When the checks are complete, cs_display() is called to display the     *
 * consistency log and get the user's response to each error: a bar can    *
 * be moved up and down the list of errors, and the user can choose to     *
 * fix any of them.                                                        *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "things.h"
#include "gui\dialog.hpp"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

bool LineUsesTID( int iLine );

uchar *st_odeleted = "Object was deleted.";
//uchar *st_notfix = "This error cannot be fixed automatically.";

/* Prototypes for module functions                                         */
void _csc_line( void );
void _csc_side( void );
void _csc_sector( void );
void _csc_path( void );
void _csc_thing( void );
void _csc_misc( void );
void _csc_fix( void );
void _csc_udtex( void );

uchar *_csf_line( short );
uchar *_csf_side( short );
uchar *_csf_thing( short );
uchar *_csf_sector( short );
uchar *_csf_fix( short );

#define NUMCHECK    8

typedef struct
{
    char    *desc;          /* Description of this check */
    uchar   doit;           /* Set the dialog box options */
    void    (*fp)(void);    /* Function that does this check */

} CS_OPTTABLE;

CS_OPTTABLE cs_opttable[NUMCHECK] =
{
    { "Line (structure) errors", 1, _csc_line },
    { "Side (wall) errors", 1, _csc_side },
    { "Sector: closed paths", 0, _csc_path },
    { "Sector: other errors", 1, _csc_sector },
    { "Item errors", 1, _csc_thing },
    { "Miscellaneous errors", 1, _csc_misc },
    { "Undefined textures", 0, _csc_udtex },
    { "Fix functions (slow, rare use)", 0, _csc_fix }
};

#define MAXERROR        256

#define CSDNUM          10      /* Display 10 items at a time */
#define CSD_REDO        0
#define CSD_CONTINUE    1

/* Takes the number of the error in the array */
typedef uchar * (* CSFUNC)(short);

typedef struct
{
    uchar   *sdesc;     /* Short description for the error */
    uchar   *help;      /* help keyword for the error */
    CSFUNC  fp;         /* function to fix the error */

} CSTABLE;

CSTABLE cs_table[] =
{
    { "Line is 2S but has no second side.", "CS_L_2SNOSD2", _csf_line },
    { "Line is NOT 2S but has a second side.", "CS_L_NO2SSD2", _csf_line },
    { "Line's vertex references are the same.", "CS_L_VSAME", _csf_line },
    { "Line has no first sidedef.", "CS_L_NOSD1", _csf_line },
    { "Line has a \"type\" but no Sector Tag.", "CS_L_TYPENOTAG", _csf_line },
    { "Line has a sector tag but no \"type\".", "CS_L_TAGNOTYPE", _csf_line },
    { "Line's tag is not shared by a Sector.", "CS_L_TAGNOSEC", _csf_line },
    { "Line's TID is not shared by a Thing.", "CS_L_TIDNOTHING", _csf_line },
    { "Side %d is missing upper/lower textures.", "CS_SD_MISSTEX", _csf_side },
    { "Side %d is missing full texture.", "CS_SD_MISSTEX", _csf_side },
    { "Side %d's full tex not \"transparent-able.\"", "CS_SD_FULLNOTTRANS", _csf_side },
    { "Side %d has a bad texture.", "CS_SD_BADTEX", _csf_side },
    { "Side %d's transparent full tex too high", "CS_SD_FTOOHIGH", _csf_side },
    { "Side %d's texture is undefined.", "CS_SD_UDTEX", _csf_side },
    { "Sector is not closed (ends at vertex %d)", "CS_S_NOTCLOSED", NULL },
    { "Sector's ceiling/floor difference is greater than 1000.", "CS_S_G1000", _csf_sector },
    { "Sector's tag is not shared by a Line.", "CS_S_TAGNOLINE", _csf_sector },
    { "Sector's floor texture is invalid.", "CS_S_BADTEX", _csf_sector },
    { "Sector's ceiling texture is invalid.", "CS_S_BADTEX", _csf_sector },
    { "Item is not in a sector.", "CS_T_NOSECTOR", _csf_thing },
    { "Item is too high for its Sector.", "CS_T_TOOHIGH", _csf_thing },
    { "Item collides with other things.", "CS_T_COLLIDE", _csf_thing },
    { "Item collides with a line.", "CS_T_INWALL", _csf_thing },
    { "Unknown Item type.", "CS_T_UNKNOWN", _csf_thing },
    { "PolyObject Anchor is duplicated with Item %d", "CS_T_POLYDUP", _csf_thing },
    { "There is no PolyObject StartSpot for this Anchor", "CS_T_POLYNOSTART", _csf_thing },
    { "No player 1 start.", "CS_M_NOP1", NULL },
    { "No player 1 start with a zeroed first argument.", "CS_M_NOP1ZERO", NULL },
    { "Too few player starts.", "CS_M_NOP234", NULL },
    { "Too few deathmatch starts.", "CS_M_NODM", NULL },
    { "Missing an exit line.", "CS_M_NOEXIT", NULL },
    { "Tele-line's tagged sector has no Tele-exit.", "CS_M_NOTEXIT", NULL },
    { "at the same point as %d", "CS_F_VSAME", _csf_fix }
};

enum cst_defs       /* Must match cs_table[] */
{
    CS_L_2SNOSD2,
    CS_L_NO2SSD2,
    CS_L_VSAME,
    CS_L_NOSD1,
    CS_L_TYPENOTAG,
    CS_L_TAGNOTYPE,
    CS_L_TAGNOSEC,
    CS_L_TIDNOTHING,
    CS_SD_MISSTEXUL,
    CS_SD_MISSTEXF,
    CS_SD_FULLNOTTRANS,
    CS_SD_BADTEX,
    CS_SD_FTOOHIGH,
    CS_SD_UDTEX,
    CS_S_NOTCLOSED,
    CS_S_G1000,
    CS_S_TAGNOLINE,
    CS_S_BADFTEX,
    CS_S_BADCTEX,
    CS_T_NOSECTOR,
    CS_T_TOOHIGH,
    CS_T_COLLIDE,
    CS_T_INWALL,
    CS_T_UNKNOWN,
    CS_T_POLYDUP,
    CS_T_POLYNOSTART,
    CS_M_NOP1,
    CS_M_NOP1ZERO,
    CS_M_NOP234,
    CS_M_NODM,
    CS_M_NOEXIT,
    CS_M_NOTEXIT,
    CS_F_VSAME
};

typedef struct
{
    short     code;     /* Code for the error (into cs_table) */
    short     object;   /* Object in error */
    short     object2;  /* Object #2..  Not used all the time. */
    uchar     *actstr;  /* Action string: What can be done or what has been done. */

} CSENTRY;

static  CSENTRY     cse_array[MAXERROR];    /* Array for the errors logged (MAXERROR in size) */
static  fldListBox  *pfErrorLog;
static  fldButton   *pfGoto, *pfFix;
static  StrList     *stlErrorLog;
static  short       nerror;                 /* Number of errors */
static  short       redoflag, lastitem;
static  uchar       nopdelete;
static  GWINDOW     *statwnp;
/* To check if we need to do it again */
static  short       nmadec = -1, nmapc = -1;
static int _upd_cbuttons();

void SetStatusMsg( char *fmt, ... );
void UpdateMessageBar();

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static void cs_log( short code, short object, short object2 )
/*-------------------------------------------------------------------------*
 * Creates a new log entry for the consistency checker in the cse_array    *
 * array.                                                                  *
 *-------------------------------------------------------------------------*/
{
    if( nerror < MAXERROR )
    {
        cse_array[nerror].code = code;
        cse_array[nerror].object = object;
        cse_array[nerror].object2 = object2;
        /* NULL: it's figured out in the display loop. */
        cse_array[nerror].actstr = NULL;
    }
    ++nerror;

    /* Display the number of errors to the user */
    wn_xytext( statwnp, 19, 5, "%d", nerror );
}

static short checkdel( void )
{
    short rvl;

    if( nopdelete )
        return 1;

    rvl = message_box( MB_YESNOALWAYS, st_dcksays,
     "Fixing this error deletes the object - are you sure\n"
     "you want to do that?" );

    if( rvl == MB_ALWAYS )
    {
        nopdelete = 1;
        return 1;
    }
    else if( rvl == MB_YES )
        return 1;
    else if( rvl == MB_NO )
        return 0;
}

static uchar *cs_errstr( short entrynum )
{
    static  uchar str[128], str2[128];

    if( cse_array[entrynum].object != -1 )
    {
        sprintf( str2, cs_table[cse_array[entrynum].code].sdesc,
                 cse_array[entrynum].object2 );

        sprintf( str, "Object %d: %s", cse_array[entrynum].object, str2 );
    }
    else
        strcpy( str, cs_table[cse_array[entrynum].code].sdesc );

    return str;
}

static int _cs_go()
/*-------------------------------------------------------------------------*
 * "Goes" to the object assoicated with the error in entrynum.             *
 * 1. Sets the appropriate edit mode                                       *
 * 2. Puts the error message on the status bar                             *
 *-------------------------------------------------------------------------*/
{
    short  entrynum = pfErrorLog->GetCurItem();
    extern Object go_obj;

    /* Go to this object (handled in input.c) */
    go_obj.iNumber = cse_array[entrynum].object;
    go_obj.iType = editmode;

    if( !go_obj.IsValid() )      /* can't go to no object. */
        return daContinue;

    switch( cse_array[entrynum].code )
    {
    case CS_M_NOTEXIT:
    case CS_L_2SNOSD2:
    case CS_L_NO2SSD2:
    case CS_L_VSAME:
    case CS_L_NOSD1:
    case CS_L_TYPENOTAG:
    case CS_L_TAGNOTYPE:
    case CS_L_TAGNOSEC:
    case CS_SD_MISSTEXUL:
    case CS_SD_MISSTEXF:
    case CS_SD_FULLNOTTRANS:
    case CS_SD_BADTEX:
    case CS_SD_FTOOHIGH:
    case CS_SD_UDTEX:
        go_obj.iType = MODE_LINE;
        break;
    case CS_S_G1000:
    case CS_S_TAGNOLINE:
    case CS_S_BADFTEX:
    case CS_S_BADCTEX:
        go_obj.iType = MODE_SECTOR;
        break;
    case CS_T_NOSECTOR:
    case CS_T_TOOHIGH:
    case CS_T_COLLIDE:
    case CS_T_INWALL:
        go_obj.iType = MODE_THING;
        break;
    case CS_S_NOTCLOSED:    /* go to vertex */
        go_obj.iNumber = cse_array[entrynum].object2;
    case CS_F_VSAME:
        go_obj.iType = MODE_VERTEX;
        break;
    }

    SetStatusMsg( cs_errstr( entrynum ) );
    UpdateMessageBar();

    return daExitSave;
}

static int _cs_fix()
/*-------------------------------------------------------------------------*
 * Invokes the "fix" function, dude.
 *-------------------------------------------------------------------------*/
{
    short item = pfErrorLog->GetCurItem();
    CSFUNC fp;
    char buf[128], *p;
    //void strtbl_rpl( STRTABLE *st, short pos, uchar *s, short ident );

    if( cse_array[item].actstr == st_odeleted )
        return daContinue;

    fp = cs_table[cse_array[item].code].fp;
    if( fp && !cse_array[item].actstr )
    {
        cse_array[item].actstr = p = (*fp)( item );
        if( p )
        {
            nMadeChanges++;
            if( cse_array[item].object != -1 )
                sprintf( buf, "Object %d: %s", cse_array[item].object, p );
            else
                strcpy( buf, p );
            stlErrorLog->SetAt( item, buf );
            //strtbl_rpl( stlErrorLog, item, buf, -1 );
            pfErrorLog->ReInit();
            pfErrorLog->SetCurItem( item+1 );
            //_fld_setpos( pfErrorLog, item+1 );
            // pfErrorLog->OnDraw();
            _upd_cbuttons();
        }
        else
        {
            message_box( MB_OK, st_dcksays,
                "I can't fix this error automatically." );
        }
    }

    /* do this, lak. */

    return daContinue;
}

static int _cs_help()
/*-------------------------------------------------------------------------*
 * Help!
 *-------------------------------------------------------------------------*/
{
    short item = pfErrorLog->GetCurItem();
    char *p = cs_table[cse_array[item].code].help;

    help_disp( p, 8 );

    return daContinue;
}

static int _cs_redo()
/*-------------------------------------------------------------------------*
 * Sets the redo flag and exits the dialog box.
 *-------------------------------------------------------------------------*/
{
    redoflag = 1;
    return daExitSave;
}

static int _upd_cbuttons()
/*-------------------------------------------------------------------------*
 * Updates the error-specific buttons
 *-------------------------------------------------------------------------*/
{
    short item = pfErrorLog->GetCurItem();

    /* fix button */
    if( cs_table[cse_array[item].code].fp != NULL &&
        cse_array[item].actstr == NULL )
        pfFix->SetStyle( csSkip, FALSE );
    else
        pfFix->SetStyle( csSkip, TRUE );

    pfFix->OnDraw();

    /* goto button */
    if( cse_array[item].object != -1 )
        pfGoto->SetStyle( csSkip, FALSE );
    else
        pfGoto->SetStyle( csSkip, TRUE );

    pfGoto->OnDraw();

    return daContinue;
}

short cs_display( short newlist )
/*-------------------------------------------------------------------------*
 * Displays the list of consistency errors.
 * Allows the user to move around a bar that highlights the "current"
 * error.
 *-------------------------------------------------------------------------*/
{
    VarTransfer_t vtConsist[] =
    {
        { "ErrorLog", (Field**) &pfErrorLog, NULL,
            "Press F1 to get help, F to fix the error or G to go to it" },
        { "BtnGoto", (Field**) &pfGoto, (PTR) _cs_go, NULL },
        { "BtnFix", (Field**) &pfFix, (PTR) _cs_fix, NULL },
        { "BtnRedo", (Field**) NULL, (PTR) _cs_redo, NULL },

        // end marker
        { NULL, NULL, NULL, NULL }
    };

    Dialog  *pDialog = new Dialog;
    pDialog->Create( szDSFile, "ConsistDlg" );
    pDialog->VarSetup( vtConsist );

    stlErrorLog = new StrList( MAXERROR, 60, "ErrorLog" );
    for( int i = 0; i < nerror && i < MAXERROR; i++ )
        stlErrorLog->AddTail( cs_errstr( i ) );

    pfErrorLog->SetItemList( stlErrorLog );
    pfErrorLog->SetOnChange( (FFUNC) _upd_cbuttons );
    pfErrorLog->SetUserEvent( etKey, 'G', (FUFUNC) _cs_go );
    pfErrorLog->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) _cs_go );
    pfErrorLog->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _cs_go );
    pfErrorLog->SetUserEvent( etKey, 'F', (FUFUNC) _cs_fix );
    pfErrorLog->SetUserEvent( etKey, 'R', (FUFUNC) _cs_redo );
    pfErrorLog->SetUserEvent( etKey, KEY_F1, (FUFUNC) _cs_help );

    redoflag = 0;

    if( !newlist )  /* not a new list ? jump to last pos in list */
        pfErrorLog->SetCurItem( lastitem );

    pDialog->SetOnEnter( (DFUNC) _upd_cbuttons );
    pDialog->Process();

    lastitem = pfErrorLog->GetCurItem();

    delete pDialog;

    if( redoflag )
        return CSD_REDO;

    return CSD_CONTINUE;
}

void _bf_consist( short msg )
{
    switch( msg )
    {
    case BC_NEWFILE:
    case BC_CLEANFILE:
        nmadec = nmapc = -1;
        nerror = 0;
        break;
    }
}

short cs_run( short force )
/*-------------------------------------------------------------------------*
 * Runs the consistency checker.                                           *
 * Displays a dialog box and prompts the user for the options to check.    *
 * When the dialog box is taken down and saved, begins a loop and          *
 * processes each choice, running the appropriate check function for each. *
 * At the end of the loop, if there were any errors, calls cs_display().   *
 * Returns the number of errors found.                                     *
 *-------------------------------------------------------------------------*/
{
    short   i, nline;

    /* See if we're gonna run the check */
    if( !force )
    {
        if( (config.tog_redocc && nMadeChanges == nmadec && nMapChanges == nmapc && nerror)
            || (!config.tog_redocc && nerror) )
        {
            /* no need, i guess. */
            if( cs_display( 0 ) != CSD_REDO )
                return nerror;
        }
    }

_redo:
    nopdelete = 0;
    nmadec = -1;

    /* Pop up the options dialog box. */
    nline = 1+NUMCHECK+3;

    Dialog *pDialog = new Dialog;
    pDialog->Create( Point( Centered, Centered ),
        Size( 128, nline * YUPC ), "Consistency Checker Options" );

    for( i = 0; i < NUMCHECK; i++ )
    {
        fldCheckBox* pCheck = new fldCheckBox( pDialog, Point( 4, 4 + i*YUPC ),
            cs_opttable[i].desc );
        pCheck->SetTransferBuf( (PTR) &(cs_opttable[i].doit) );
    }

    pDialog->AddDefaultButtons();

    /* Run the dialog box */
    int iRvl = pDialog->Process();

    delete pDialog;

    if( iRvl == daExitNoSave )
    {
        nmadec = -1;
        return 0;     /* BOO */
    }

    /* Ok, now run all the consistency checks. */
    if( nMapChanges )
        map_purge( MODE_ALL );  /* May as well do this if it's already modified */
    nerror = 0;

    /* Stick up a box with some status stuff in it. */
    statwnp = MAKESTATUSWINDOW( "Performing Consistency Check", 2 );
    wn_disp( statwnp );
    wn_xytext( statwnp, 1, 5, "Number of errors:" );

    for( i = 0; i < NUMCHECK; i++ )
    {
        /* Run this one? */
        if( cs_opttable[i].doit )
        {
            /* Yep ! */
            wn_statstr( statwnp, cs_opttable[i].desc );
            (*cs_opttable[i].fp)();
        }
    }

    wn_undisp( statwnp );
    wn_free( statwnp );

    /* Done.. now, if there were any errors. */
    if( !nerror )
        message_box( MB_OK, "Great news", "No errors were found!" );
    else
        if( cs_display( 1 ) == CSD_REDO )
            goto _redo;

    nmadec = nMadeChanges;
    nmapc = nMapChanges;

    return nerror;
}

static void _csc_misc( void )
/*-------------------------------------------------------------------------*
 * Checks misc. things!                                                    *
 *-------------------------------------------------------------------------*/
{
    short     i;
    short     t_p1 = 0, t_npstart = 0, t_p1zero = 0;
    short     t_ndstart = 0;
    short     theline, ttc = 0, ndone = 0;
    THING   *tptr;
    SECTOR  *secp;

    /* Check the things */
    for( i = 0, tptr = thinglist; i < nthing; i++, tptr++ )
    {
        if( !(i % 25) )
            wn_stat( statwnp, i, nthing );

        if( tptr->type == THING_PLAYER1 )
        {
            t_p1 = 1;
            if( tptr->arg0 == 0 )
                t_p1zero = 1;
        }
        else if( tptr->type < 5 )
            t_npstart++;
        else if( tptr->type == THING_DEATHMATCH )
            t_ndstart++;
    }

    if( !t_p1 )
        cs_log( CS_M_NOP1, -1, -1 );
    if( !t_p1zero )
        cs_log( CS_M_NOP1ZERO, -1, -1 );
    if( t_npstart < 3 )
        cs_log( CS_M_NOP234, -1, -1 );
    if( t_ndstart < 4 )
        cs_log( CS_M_NODM, -1, -1 );

    if( !fHexen )
    {
        /* Now check for exit lines */
        if( obj_linefind( LF_BEGIN, LF_TYPE, 11, LF_END ) == -1 &&
            obj_linefind( LF_BEGIN, LF_TYPE, 52, LF_END ) == -1 &&
            obj_linefind( LF_BEGIN, LF_TYPE, 124, LF_END ) == -1 &&
            obj_linefind( LF_BEGIN, LF_TYPE, 51, LF_END ) == -1 )
            cs_log( CS_M_NOEXIT, -1, -1 );
    }
    else
    {
        if( obj_linefind( LF_BEGIN, LF_TYPE, 74, LF_END ) == -1 &&
            obj_linefind( LF_BEGIN, LF_TYPE, 75, LF_END ) == -1 )
            cs_log( CS_M_NOEXIT, -1, -1 );
    }

/*-------------------------------------------------------------------------
 * Check teleport lines for tagged sectors with teleporter exits.
 * (First pass, checks for type #97.  Second pass, checks for #39.)
 *-------------------------------------------------------------------------*/
    theline = obj_linefind( LF_BEGIN, LF_TYPE, 97, LF_END );

_search:
    while( !fHexen && theline != -1 )
    {
        wn_stat( statwnp, ndone + theline, nline * 2 );

        /* Ok.. now find a sector with the same sector tag */
        for( i = 0, secp = sectorlist; i < nsector; i++, secp++ )
        {
            if( secp->sectag && secp->sectag == linelist[theline].sectag )
            {
                /* This sector is the destination for the xport */
                short     i2;

                /* Ok, now search for all xport exit things */
                for( i2 = 0, tptr = thinglist; i2 < nthing; i2++, tptr++ )
                {
                    if( tptr->type == THING_TELEPORT )
                        if( obj_get( MODE_SECTOR, tptr->x, tptr->y, tptr->x, tptr->y ) == i )
                            break;
                }

                if( i2 == nthing )
                    cs_log( CS_M_NOTEXIT, theline, -1 );

                break;
            }
        }

        /* Get the next line */
        theline = obj_linefind( LF_CONTINUE );
    }

    if( !fHexen && !ttc )      /* Searching for two types and only done the first? */
    {
        ttc = 1;
        ndone = nline * ttc;
        theline = obj_linefind( LF_BEGIN, LF_TYPE, 39, LF_END );
        goto _search;
    }
}

bool lineneedstag( int iType )
{
    short idx;
    short *pshNoTag = pgdCurGame->lines_notag;

    if( !iType )
        return FALSE;

    /* search game data structure */
    for( idx = 0; idx < 64; idx++, pshNoTag++ )
    {
        if( iType == pshNoTag[0] )
            return fHexen ? TRUE : FALSE;
    }

    return fHexen ? FALSE : TRUE;
}

static void _csc_line( void )
/*-------------------------------------------------------------------------*
 * Check for line errors                                                   *
 *-------------------------------------------------------------------------*/
{
    LINEDEF *ldp;
    short     i;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        /* Display the status */
        if( !(i % 50) )
            wn_stat( statwnp, i, nline );

        /* Here, we check for errors. */
        if( (ldp->attribs & LA_TSIDE) && ldp->lsdef == -1 )
            cs_log( CS_L_2SNOSD2, i, -1 );  /* 2-sided, no second sidedef */
        if( !(ldp->attribs & LA_TSIDE) && ldp->lsdef != -1 )
            cs_log( CS_L_NO2SSD2, i, -1 );  /* not 2-sided, but second sidedef */
        if( ldp->vfrom == ldp->vend )
            cs_log( CS_L_VSAME, i, -1 );    /* vertex references the same */
        if( ldp->rsdef == -1 )
            cs_log( CS_L_NOSD1, i, -1 );    /* no first sidedef */

        if( lineneedstag( ldp->special ) )
        {
            if( !lSecTag( ldp ) )      /* Type but no tag */
            {
                if( (fHexen && !inrange( ldp->special, 10, 13 )) || !fHexen )
                    cs_log( CS_L_TYPENOTAG, i, -1 );
            }
            else
            /* Find a sector with the sector tag */
            {
                short     i2;
                SECTOR  *secp;
                int     iLineTag = lSecTag( ldp );

                for( i2 = 0, secp = sectorlist; i2 < nsector; i2++, secp++ )
                {
                    if( secp->sectag == iLineTag )
                        break;
                }

                if( i2 == nsector )     /* true if none were found */
                    cs_log( CS_L_TAGNOSEC, i, -1 );
            }
        }

        else if( LineUsesTID( i ) )
        {
            // find a thing with matching TID
            THING *thingp;
            int i2;
            int iLineTID = ldp->arg0;

            for( i2 = 0, thingp = thinglist; i2 < nthing; i2++, thingp++ )
            {
                if( thingp->tid == iLineTID )
                    break;
            }

            if( i2 == nthing )
                cs_log( CS_L_TIDNOTHING, i, -1 );   // oops, none!
        }

        else if( !fHexen && lSecTag( ldp ) )      /* No type, but tag. */
            cs_log( CS_L_TAGNOTYPE, i, -1 );
    }

    wn_stat( statwnp, nline, nline );
}

static short bothsky( short sd1, short sd2 )
{
    SECTOR  *secp;

    if( sd1 == -1 || sd2 == -1 )
        return 0;

    secp = &sectorlist[sidelist[sd1].sector];

    if( memcmp( secp->ctxname, "F_SKY", 5 ) )
        return 0;

    secp = &sectorlist[sidelist[sd2].sector];

    if( memcmp( secp->ctxname, "F_SKY", 5 ) )
        return 0;

    return 1;
}

static void _csc_side( void )
/*-------------------------------------------------------------------------*
 * Side checker: for things like missing textures, etc.                    *
 *-------------------------------------------------------------------------*/
{
    LINEDEF *ldp;
    short     i, e, needu, needl, needf, noutx;
    SIDEDEFM *sdp;
    SECTOR  *secp;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        /* Display the status */
        if( !(i % 50) )
            wn_stat( statwnp, i, nline );

        noutx = bothsky( ldp->rsdef, ldp->lsdef );

        e = 0;

        /* Check this side */
        if( ldp->rsdef != -1 )
        {
            sdp = &sidelist[ldp->rsdef];
            side_getneedtex( i, 0, &needu, &needf, &needl );
            if( (needu && !sdp->utxidx && !noutx) || needl && !sdp->ltxidx )
                cs_log( CS_SD_MISSTEXUL, i, 1 ), e = 1;
            if( needf && !sdp->ftxidx )
                cs_log( CS_SD_MISSTEXF, i, 1 ), e = 1;
            if( !needf && sdp->ftxidx && texlist[sdp->ftxidx].ndescrip > 1 )
                cs_log( CS_SD_FULLNOTTRANS, i, 1 ), e = 1;

            if( !inrange( sdp->ftxidx, 0, ntex ) ||
                !inrange( sdp->utxidx, 0, ntex ) ||
                !inrange( sdp->ltxidx, 0, ntex ) )
                cs_log( CS_SD_BADTEX, i, 1 ), e = 1;

            if( !needf && sdp->ftxidx && texlist[sdp->ftxidx].height >
                ((secp = &sectorlist[sdp->sector])->cheight - secp->fheight) &&
                sdp->sector == sidelist[ldp->lsdef].sector )
                cs_log( CS_SD_FTOOHIGH, i, 1 );
        }

        if( e )     /* Don't do both: not worth it */
            continue;

        if( ldp->lsdef != -1 )
        {
            sdp = &sidelist[ldp->lsdef];
            side_getneedtex( i, 1, &needu, &needf, &needl );
            if( (needu && !sdp->utxidx && !noutx) || needl && !sdp->ltxidx )
                cs_log( CS_SD_MISSTEXUL, i, 2 );
            if( needf && !sdp->ftxidx )
                cs_log( CS_SD_MISSTEXF, i, 2 );
            if( !needf && sdp->ftxidx && texlist[sdp->ftxidx].ndescrip > 1 )
                cs_log( CS_SD_FULLNOTTRANS, i, 2 );
            if( !inrange( sdp->ftxidx, 0, ntex ) ||
                !inrange( sdp->utxidx, 0, ntex ) ||
                !inrange( sdp->ltxidx, 0, ntex ) )
                cs_log( CS_SD_BADTEX, i, 2 );

            if( !needf && sdp->ftxidx && texlist[sdp->ftxidx].height >
                ((secp = &sectorlist[sdp->sector])->cheight - secp->fheight) &&
                sdp->sector == sidelist[ldp->rsdef].sector )
                cs_log( CS_SD_FTOOHIGH, i, 2 );
        }
    }

    wn_stat( statwnp, nline, nline );
}

static void _csc_udtex( void )
/*-------------------------------------------------------------------------*
 * Checks for blank textures in sides, not including "-"
 *-------------------------------------------------------------------------*/
{
    LINEDEF *ldp;
    short   e, i;
    SIDEDEFM *sdp;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        /* Display the status */
        if( !(i % 50) )
            wn_stat( statwnp, i, nline );

        e = 0;

        /* Check this side */
        if( ldp->rsdef != -1 )
        {
            sdp = &sidelist[ldp->rsdef];

            if( (sdp->utxidx && texlist[sdp->utxidx].ndescrip == 0) ||
                (sdp->ftxidx && texlist[sdp->ftxidx].ndescrip == 0) ||
                (sdp->ltxidx && texlist[sdp->ltxidx].ndescrip == 0) )
                cs_log( CS_SD_UDTEX, i, 1 ), e = 1;
        }

        if( e )     /* Don't do both: not worth it */
            continue;

        if( ldp->lsdef != -1 )
        {
            sdp = &sidelist[ldp->lsdef];

            if( (sdp->utxidx && texlist[sdp->utxidx].ndescrip == 0) ||
                (sdp->ftxidx && texlist[sdp->ftxidx].ndescrip == 0) ||
                (sdp->ltxidx && texlist[sdp->ltxidx].ndescrip == 0) )
                cs_log( CS_SD_UDTEX, i, 1 ), e = 1;
        }
    }

    wn_stat( statwnp, nline, nline );
}

void _csc_sector( void )
/*-------------------------------------------------------------------------*
 * Checks sectors for bad stuff.                                           *
 *-------------------------------------------------------------------------*/
{
    short     i;
    SECTOR  *secp;

    for( i = 0, secp = sectorlist; i < nsector; i++, secp++ )
    {
        /* Check for matching linedef tag */
        if( !fHexen && secp->sectag && secp->sectag != 999 && secp->sectag != 666 )
        {
            if( obj_linefind( LF_BEGIN, LF_SECTAG, secp->sectag, LF_END ) == -1 )
                cs_log( CS_S_TAGNOLINE, i, -1 );    /* Yep! Set the error. */
        }
        if( floor_toint( secp->ftxname ) == -1 )
            cs_log( CS_S_BADFTEX, i, -1 );
        if( floor_toint( secp->ctxname ) == -1 )
            cs_log( CS_S_BADCTEX, i, -1 );
    }
}

void _csc_fix( void )
/*-------------------------------------------------------------------------*
 * Fixy things.                                                            *
 *-------------------------------------------------------------------------*/
{
    VERTEX  *vtxp, *vtp2;
    short     i, i2, x, y;

    for( i = 0, vtxp = vlist; i < nvertex; i++, vtxp++ )
    {
        wn_stat( statwnp, i, nvertex );

        x = vtxp->x;
        y = vtxp->y;

        for( i2 = 0, vtp2 = vlist; i2 < nvertex; i2++, vtp2++ )
        {
            if( i2 != i && x == vtp2->x && y == vtp2->y )
                cs_log( CS_F_VSAME, i, i2 );
        }
    }
}

uchar islinedefinside( short object, short x1, short y1, short x2, short y2 );

short appear_same( THING *t1, THING *t2 )
{
    unsigned a1 = t1->attribs, a2 = t2->attribs;

    /* mask out all but the first three bits of each attribute short, and
     * then and the results together */
    if( fHexen )
    {
        if( (a1 & 7) & (a2 & 7) )
        {
            if( (a1 & 448) & (a2 & 448) )
            {
                if( (a1 & 3584) & (a2 & 3584) )
                    return 1;
            }
        }
    }
    else
    {
        if( (a1 & 7) & (a2 & 7) )
        {
            if( (a1 & TA_MPLAYER) & (a2 & TA_MPLAYER) )
                return 1;
        }
    }

    return 0;
}

void _csc_thing( void )
/*-------------------------------------------------------------------------*
 * Checks thing errors.                                                    *
 *-------------------------------------------------------------------------*/
{
    short   i, sector, l, rad1, tidx, i2, rad2;
    short   x1, y1, x2, y2;
    THING   *tptr, *tptr2;
    LINEDEF *ldp;
    SECTOR  *secp;

    for( i = 0, tptr = thinglist; i < nthing; i++, tptr++ )
    {
        if( !(i % 40) )     /* Display status */
            wn_stat( statwnp, i, nthing );

        tidx = (short) (pgdCurGame->ttype2idx[tptr->type]);

        if( tidx == -1 )
        {
            cs_log( CS_T_UNKNOWN, i, -1 );
            continue;
        }

        if( fHexen && inrange( tptr->type, 3000, 3002 ) )
        {
            // polyobject checks
            if( tptr->type == 3000 )
            {
                // anchor: check for startspot
                for( i2 = 0, tptr2 = thinglist; i2 < nthing; i2++, tptr2++ )
                {
                    if( (tptr2->type == 3001 || tptr2->type == 3002) &&
                        tptr2->angle == tptr->angle )
                        break;
                }

                if( i2 == nthing )
                {
                    // error
                    cs_log( CS_T_POLYNOSTART, i, -1 );
                }

                // check for duplicated anchor
                for( i2 = 0, tptr2 = thinglist; i2 < nthing; i2++, tptr2++ )
                {
                    if( i2 != i && tptr2->type == 3000 && tptr2->angle == tptr->angle )
                    {
                        cs_log( CS_T_POLYDUP, i, i2 );
                        break;
                    }
                }
            }

            continue;
        }

        /* Make sure it's in a sector */
        sector = obj_get( MODE_SECTOR, tptr->x, tptr->y, tptr->x, tptr->y );

        if( sector == -1 )      /* Not in a sector! */
            cs_log( CS_T_NOSECTOR, i, -1 );
        else        /* Check its height */
        {
            secp = &sectorlist[sector];
            if( thing_height( tptr->type ) > abs( secp->cheight-secp->fheight ) )
                cs_log( CS_T_TOOHIGH, i, -1 );      /* Too high! */
        }

        // check for collision with wall
        // this only matters for moving things, which all have angles.
        if( pgdCurGame->thinginfo[tidx].dangle )
        {
            rad1 = thing_mapradius( tptr->type );
            rad1 -= (rad1 / 4);

            x1 = tptr->x - rad1;
            y1 = tptr->y - rad1;
            x2 = tptr->x + rad1;
            y2 = tptr->y + rad1;

            /* check to see if it's in a wall */
            for( l = 0, ldp = linelist; l < nline; l++, ldp++ )
            {
                if( ldp_isimpass( ldp ) )
                    if( islinedefinside( l, x1, y1, x2, y2 ) )
                    {
                        cs_log( CS_T_INWALL, i, -1 );       /* in wall! */
                        break;
                    }
            }
        }

        // check for collision with other items
        if( pgdCurGame->thinginfo[tidx].clash )
        {
            // IF (ABS(x1-x2) =< (r1+r2)) AND (ABS(y1-y2) =< (r1+r2)) THEN *collision*
            x1 = tptr->x;
            y1 = tptr->y;

            /* check to see if it collides with other things */
            for( i2 = 0, tptr2 = thinglist; i2 < nthing; i2++, tptr2++ )
            {
                if( i2 != i )
                {
                    if( pgdCurGame->thinginfo[pgdCurGame->ttype2idx[tptr2->type]].clash )
                    {
                        rad2 = thing_mapradius( tptr2->type );
                        rad2 -= (rad2 / 4);

                        x2 = tptr2->x;
                        y2 = tptr2->y;

                        if( abs( x1-x2 ) <= ( rad1+rad2 ) && abs( y1-y2 ) <= ( rad1+rad2 ) )
                        {
                            if( appear_same( tptr, tptr2 ) )
                                cs_log( CS_T_COLLIDE, i, -1 );
                        }
                    }
                }
            }
        }
    }

    wn_stat( statwnp, nthing, nthing );
}

void _csc_path( void )
/*-------------------------------------------------------------------------*
 * Checks all sectors for closed paths.                                    *
 *-------------------------------------------------------------------------*/
{
    SECTOR  *secp;
    LINEDEF *ldp;
    short     i, l, *llist, *llp, *ip, i2, lcount;
    short     vidx, vfirst;

    llist = new short[nline];

    for( i = 0, secp = sectorlist; i < nsector; i++, secp++ )
    {
        if( !(i % 20) )
            wn_stat( statwnp, i, nsector );

        char buf[128];

        sprintf( buf, "sector %d", i );
        wn_statstr( statwnp, buf );

        llp = llist;
        lcount = 0;

        if( i == 254 )
            i = 254;

        for( vidx = -1, vfirst = -2; ; )
        {
            for( l = 0, ldp = linelist; l < nline; l++, ldp++ )
            {
                if( ldp->rsdef >= 0 && (ldp->vfrom == vidx || vidx == -1) &&
                    sidelist[ldp->rsdef].sector == i )

                {
                    // this line's right side references the sector
                    for( i2 = 0, ip = llist; i2 < lcount; i2++, ip++ )
                    {
                        if( *ip == l )  // already stored this line
                            goto _nope;
                    }

                    // don't count lines with both sides that reference the
                    // sector
                    if( ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == i )
                        goto _nope;

                    if( vidx == -1 )    // is this the first line we've found?
                        vfirst = ldp->vend;
                    else
                    {
                        // add this line to the list
                        *llp++ = l;
                        ++lcount;
                    }

                    // since the right side references the sector, the next line
                    // we find should be connected to the end of the line
                    vidx = ldp->vend;
                    break;
                }

                if( ldp->lsdef >= 0 && (ldp->vend == vidx || vidx == -1) &&
                    sidelist[ldp->lsdef].sector == i )
                {
                    for( i2 = 0, ip = llist; i2 < lcount; i2++, ip++ )
                    {
                        if( *ip == l )
                            goto _nope;
                    }

                    if( ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == i )
                        goto _nope;

                    if( vidx == -1 )
                        vfirst = ldp->vfrom;
                    else
                    {
                        *llp++ = l;
                        ++lcount;
                    }
                    vidx = ldp->vfrom;
                    break;
                }
_nope:
            }

            if( vfirst == vidx && lcount > 1 )
                break;

            // started on this sector, but finished the "find sector lines"
            // loop without finding the end of the sector ..
            if( vidx != -1 && l == nline )
            {
                cs_log( CS_S_NOTCLOSED, i, vidx );
                break;
            }
            if( vidx == -1 && l == nline )
            {
                // this sector does not exist.
                break;
            }
        }
    }

    delete llist;
}

uchar *_csf_line( short errnum )
{
    short     code = cse_array[errnum].code;
    short     object = cse_array[errnum].object;
    LINEDEF *ldp = &linelist[object];

    switch( code )
    {
    case CS_L_VSAME:        /* Delete the fuckerzzzzz */
        linelist[object].vfrom = linelist[object].vend = 0;
        nMapChanges++;
        return st_odeleted;
    case CS_L_2SNOSD2:       /* 2-sided, no sd2 */
        ldp->attribs |= LA_IMPASS;
        ldp->attribs &= ~LA_TSIDE;
        return "Fixed: Removed 2S and added Impassable flags.";
    case CS_L_NO2SSD2:
        ldp->attribs |= LA_TSIDE;
        return "Fixed: Added 2S flag.";
    case CS_L_NOSD1:        /* no first sidedef */
        if( ldp->lsdef != -1 &&
            !(ldp->attribs & LA_TSIDE) &&
            (ldp->attribs & LA_IMPASS) )
        {
            ldp->rsdef = ldp->lsdef;
            ldp->lsdef = -1;
            return "Sides were reversed.";
        }
        return NULL;
    case CS_L_TYPENOTAG:
        ldp->special = 0;      /* normal line */
        return "Line type was set to Normal.";
    case CS_L_TAGNOTYPE:
        if( fHexen )
            return NULL;
        ldp->sectag = 0;    /* no sector tag */
        return "Line's sector tag was removed.";
    case CS_L_TAGNOSEC:
        lSecTag( ldp ) = 0;
        ldp->special = 0;
        return "Line was set to Normal / No Tag.";
    }

    return NULL;
}

uchar *_csf_thing( short errnum )
{
    short code = cse_array[errnum].code;
    short object = cse_array[errnum].object;

    switch( code )
    {
    case CS_T_NOSECTOR:
    case CS_T_COLLIDE:
    case CS_T_INWALL:
    case CS_T_TOOHIGH:
        if( checkdel() )
        {
            Object obj( object, MODE_THING );
            obj_delete( MODE_THING, obj, 0 );
            return st_odeleted;
        }
    }

    return NULL;
}

uchar *_csf_side( short errnum )
{
    short         code = cse_array[errnum].code;
    short         object = cse_array[errnum].object;
    short         whichside = cse_array[errnum].object2-1;
    SIDEDEFM    *sdp = &sidelist[whichside ? linelist[object].lsdef : linelist[object].rsdef];
    uchar       *whereptr;
    static      uchar buf[80];

    switch( code )
    {
    case CS_SD_UDTEX:     /* undef texture */
        sdp->utxidx = sdp->ftxidx = sdp->ltxidx = 0;
        _WFfixtexnames( object, whichside, 0 );
        return "Side was retextured with defaults.";
    case CS_SD_MISSTEXUL:     /* MIssing textures */
    case CS_SD_MISSTEXF:
        _WFfixtexnames( object, whichside, 0 );
        return "Required textures were added.";
    case CS_SD_FULLNOTTRANS:
    case CS_SD_FTOOHIGH:
        sdp->ftxidx = 0;
        return "The full texture was removed.";
    case CS_SD_BADTEX:
        whereptr = "No";
        if( !inrange( sdp->utxidx, 0, ntex ) )
            sdp->utxidx = 0, whereptr = "Upper";
        if( !inrange( sdp->ftxidx, 0, ntex ) )
            sdp->ftxidx = 0, whereptr = "Full";
        if( !inrange( sdp->ltxidx, 0, ntex ) )
            sdp->ltxidx = 0, whereptr = "Lower";
        _WFfixtexnames( object, whichside, 0 );
        sprintf( buf, "%s texture was fixed", whereptr );
        return buf;
    }

    return NULL;
}

uchar *_csf_sector( short errnum )
{
    short     code = cse_array[errnum].code;
    short     object = cse_array[errnum].object;
    SECTOR  *secp = &sectorlist[object];
    static  char buf[80];

    switch( code )
    {
    case CS_S_G1000:
        secp->cheight = secp->fheight + 999;
        sprintf( buf, "Sector's ceiling height was set to %d.", secp->cheight );
        return buf;
    case CS_S_TAGNOLINE:
        secp->sectag = 0;
        return "Sector's tag was set to 0.";
    case CS_S_BADFTEX:
        wncpy( secp->ftxname, _def_floortex );
        sprintf( buf, "Sector's floor texture was set to " );
        strncat( buf, _def_floortex, 8 );
        return buf;
    case CS_S_BADCTEX:
        wncpy( secp->ctxname, _def_ceiltex );
        sprintf( buf, "Sector's ceiling texture was set to " );
        strncat( buf, _def_ceiltex, 8 );
        return buf;
    }

    return NULL;
}

uchar *_csf_fix( short errnum )
{
    short     code = cse_array[errnum].code;
    short     object = cse_array[errnum].object;
    static  char buf[80];

    switch( code )
    {
    case CS_F_VSAME:
        vertex_replace( object, cse_array[errnum].object2 );
        sprintf( buf, "Vertex %d was replaced with %d", object,
                 cse_array[errnum].object2 );
        return buf;
    }

    return NULL;
}

