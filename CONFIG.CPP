/*-------------------------------------------------------------------------*
 * CONFIG.C:
 * Functions for reading configuration files.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"

/*-------------------------------------------------------------------------*
 *
 * Definitions for this file
 *
 *-------------------------------------------------------------------------*/

typedef int (* LOADFP)(uchar*);     /* load function */

typedef struct
{
    uchar       *sect;      /* Section name */
    LOADFP      loadfunc;   /* load function for the section */

} BLOCKINFO;

typedef struct
{
    uchar       *sig;       /* type of config file */
    BLOCKINFO   *inf;       /* block information (load blocks) */

} FILEDEF;

/*-------------------------------------------------------------------------*
 *
 * Globals for this file
 *
 *-------------------------------------------------------------------------*/

int _load_things( uchar* );
int _load_tinfo( uchar* );
int _load_actions( uchar* );
int _load_sectors( uchar* );
int _load_general( uchar* );
int _load_popup( uchar* );
int _load_texwads( uchar* );
int _load_lnotag( uchar* );
int _load_angles( uchar* );
int _load_done( uchar* );
int _load_thingargs( uchar* );
int _load_tidactions( uchar* );

BLOCKINFO   _gamedat_info[] =
{
    { "Things", (LOADFP) _load_things },
    { "Thing Info", (LOADFP) _load_tinfo },
    { "Thing Args", (LOADFP) _load_thingargs },
    { "Actions", (LOADFP) _load_actions },
    { "Sectors", (LOADFP) _load_sectors },
    { "General", (LOADFP) _load_general },
    { "Texture Files", (LOADFP) _load_texwads },
    { "Actions with TID", (LOADFP) _load_tidactions },
    { "Actions without Tag", (LOADFP) _load_lnotag },
    { "Actions with Tag", (LOADFP) _load_lnotag },
    { "Popup", (LOADFP) _load_popup },
    { "Angles", (LOADFP) _load_angles },
    { "*Done", (LOADFP) _load_done },
    { NULL, (LOADFP) NULL }
};

FILEDEF     file_defs[] =
{
    { "GAMEDATA", (BLOCKINFO *) &_gamedat_info },
    { NULL, NULL }
};

#define MAXLINE 128

static  FILE *fp;
static  uchar _getline_mode;
int     config_lineno;
uchar   *config_fn; /* current config filename */

/*-------------------------------------------------------------------------*
 *
 * Code begins here
 *
 *-------------------------------------------------------------------------*/

void parseline_cfg( uchar *buf, uchar *val1, ... )
/*-------------------------------------------------------------------------*
 * Parses a semicolon-delimited line
 *-------------------------------------------------------------------------*/
{
    va_list vl;
    char    *ptr = val1, *tokptr = buf, dest[128], *dptr;

    va_start( vl, val1 );

    while( ptr != NULL )
    {
        dptr = dest;

        while( *tokptr != ';' && *tokptr )
            *dptr++ = *tokptr++;

        if( *tokptr == ';' )
            ++tokptr;   /* advance past that eh */

        *dptr = 0;

        strcvt( ptr, dest, RALEADBLANKS | RTRBLANKS, 128 );
        ptr = va_arg( vl, char * );
    }

    va_end( vl );
}

void parsesecline_cfg( uchar *buf, uchar *secname, uchar *additional )
/*-------------------------------------------------------------------------*
 * parses a [Section] or [Section: Additional] line into its components.
 *-------------------------------------------------------------------------*/
{
    uchar *p;

    *additional = 0;    /* blank that out - might not be in buf */

    strccpy( secname, buf+1, "]:" );     /* Section */
    strcvt( secname, secname, RTRBLANKS, 128 );

    if( (p = strchr( (char*) buf, ':' )) )
    {
        strccpy( additional, p+1, "]" );
        strcvt( additional, additional, RALEADBLANKS, 128 );
    }
}

void countline_cfg( int *nLines, int *nCols )
{
    long pos = ftell( fp );
    char buf[128];
    int iMaxLines = 0, iMaxCols = 0;

    while( 1 )
    {
        if( fgets( buf, MAXLINE, fp ) == NULL )
            break;        /* no lines to get :( */

        if( buf[0] == '[' )
            break;

        int iTmp = strlen( buf );

        if( iTmp > iMaxCols )
            iMaxCols = iTmp;

        ++iMaxLines;
    }

    fseek( fp, pos, SEEK_SET );

    nLines[0] = iMaxLines;
    nCols[0] = iMaxCols;
}

static  uchar return_this[128];     /* if nonblank: copy, set blank, return */

uchar *getline_cfg( uchar *buf )
/*-------------------------------------------------------------------------*
 * Gets the next line from the currently open configuration file.  If
 * _getline_mode == 0, gets *every line*.  Otherwise, only returns
 * the line if it is not a "section" line (so that load functions can
 * call this as well.)
 * Comment lines and blank lines are universally ignored.
 *-------------------------------------------------------------------------*/
{
    uchar   *p, blank;

    if( *return_this )      /* like it says */
    {
        strcpy( buf, return_this );
        *return_this = 0;
        return buf;
    }

    while( 1 )
    {
        if( fgets( buf, MAXLINE, fp ) == NULL )
            return NULL;        /* no lines to get :( */

        ++config_lineno;

        blank = 1;
        p = buf;

        while( *p )
        {
            if( *p == '\r' || *p == '\n' || *p == '#' )
            {
                *p = 0;     /* all done here */
                break;
            }
            else if( *p != 32 && *p != 9 )
                blank = 0;
            p++;
        }

        if( !(*buf) || blank )     /* blank lines - fuk that, man */
            continue;

        /* we now have a pristine line .. let's see what it is */
        if( *buf == '[' )       /* oooh, a section! */
        {
            if( _getline_mode == 0 )
                return buf;
            else
            {
                /* silly load function, sections are for load_cfg()! */
                strcpy( return_this, buf );
                return NULL;
            }
        }
        else
            break;  /* return at end of function */
    }

    return buf;
}

short load_cfg( uchar *fn )
/*-------------------------------------------------------------------------*
 * Loads a configuration file.
 * The first line of the file is a signature which must match one of the
 * entries in the file_defs array.  When it is found, a subsidiary function
 * is called to parse each of the sections and load each.
 *-------------------------------------------------------------------------*/
{
    uchar       buf[128];
    uchar       section[128], additional[128];
    short       i;
    LOADFP      sec_fp;
    BLOCKINFO   *inf = NULL;
    uchar       reason[128];

    uchar       *lconfig_fn;
    FILE        *lfp;
    uchar       l_getline_mode;
    int         llineno;

    fp = fopen( fn, "rt" ); /* open the file */

    return_this[0] = 0;
    config_lineno = 0;
    config_fn = fn;

    if( fp == NULL )        /* oops! */
    {
        error( "can't open file '%s' for read access", fn );
        return 0;
    }

    setvbuf( fp, NULL, _IOFBF, 4096 );

    _getline_mode = 0;      /* in load_cfg() */

    strcpy( reason, "No signature present." );

    /* first, get the signature */
    if( getline_cfg( buf ) == NULL )
    {
        /* error! */
invalid_file:
        error( "configuration file '%s' is invalid\n(%s)", fn, reason );
        fclose( fp );
        return 0;
    }

    /* find the signature */
    for( i = 0; file_defs[i].sig; i++ )
    {
        if( !strcmp( file_defs[i].sig, buf ) )
        {
            inf = file_defs[i].inf;
            break;
        }
    }

    if( !inf )  /* if it's still null, we didn't find a matching sig */
    {
        sprintf( reason, "unknown signature '%s'", buf );
        goto invalid_file;
    }

    while( 1 )
    {
        /* read each section, call its load function, whoop de doo */
        _getline_mode = 0;
        if( getline_cfg( buf ) == NULL )
            break;      /* all done */

        /* check for include statement */
        if( !strncmpi( buf, "[include", 8 ) )
        {
            /* save current status, call load_cfg again, then restore
             * status. */
            lconfig_fn = config_fn;
            lfp = fp;
            l_getline_mode = _getline_mode;
            llineno = config_lineno;
            strccpy( section, buf+9, "]" );
            if( !load_cfg( section ) )
            {
                fclose( fp );
                return 0;
            }
            config_fn = lconfig_fn;
            fp = lfp;
            _getline_mode = l_getline_mode;
            config_lineno = llineno;
            continue;
        }

        if( *buf != '[' )       /* this shouldn't happen */
        {
            sprintf( reason, "line %d should have a section header", config_lineno );
            goto invalid_file;
        }

        /* find out which section it is */
        parsesecline_cfg( buf, section, additional );

        /* find the section - we hope */
        sec_fp = NULL;
        for( i = 0; inf[i].sect; i++ )
        {
            if( !strcmpi( inf[i].sect, section ) )
            {
                sec_fp = inf[i].loadfunc;
                break;
            }
        }

        if( !sec_fp )   /* error! */
        {
            sprintf( reason, "line %d's section header (%s) is invalid", config_lineno, section );
            goto invalid_file;
        }

        _getline_mode = 1;  /* in load function - so get_line knows not to get sections */
        switch( (*sec_fp)(additional) )
        {
        case FALSE:
            fclose( fp );
            return 0;
        case -1:
            fclose( fp );
            return 1;   // ignore this file, but return success
        }
    }

    /* close the file */
    fclose( fp );

    /* find the section - we hope */
    sec_fp = NULL;
    for( i = 0; inf[i].sect; i++ )
    {
        if( !strcmpi( inf[i].sect, "*Done" ) )
        {
            sec_fp = inf[i].loadfunc;
            break;
        }
    }

    if( sec_fp )
        (*sec_fp)(NULL);

    return 1;
}

/*-------------------------------------------------------------------------*
 *
 * Configuration dialog box and other
 *
 *-------------------------------------------------------------------------*/

void setconfig( void )
{
    /* set options */
    if( !xscale )
        xscale = config.xscale;

    tog_grid = config.tog_grid;
    tog_join = config.tog_join;
    tog_tgraph = (config.tgr_default == TGR_PRIMARY);

    _PopHelp.SetActive( config.tog_pophelp );

    post_fullupdate();
}

void saveconfig( void )
{
    short handle = z_creat( _ABBREV_"32.CFG", 0 );
    z_write( handle, _ABBREV_" Configuration File (you can't edit this.)\r\n\x1a", 50 );
    z_write( handle, &file_version, sizeof file_version );
    z_write( handle, &config, sizeof config );
    z_close( handle );
}

void readconfig( void )
/*-------------------------------------------------------------------------*
 * OPens and reads the file dck32.cfg.  if the file does not exist, it
 * sets a global "get configuration" flag, which means that the user must
 * configure dck when they get in it.  before doing this, it also sets
 * the defaults for a new configuration.
 *-------------------------------------------------------------------------*/
{
    short   handle, version;

    void ReadTools();
    ReadTools();

    handle = z_open( _ABBREV_"32.CFG", O_BINARY | O_RDONLY );
    if( handle != -1 )
    {
        lseek( handle, 50L, SEEK_SET );
        z_read( handle, &version, sizeof version );

        if( version == file_version )
        {
            z_read( handle, &config, sizeof config );

            z_close( handle );
            setconfig();
            return;
        }

        z_close( handle );
        remove( _ABBREV_"32.CFG" );
    }

    fFindGames = 1;
    fGetConfig = 1;
    config.xscale = 6000;
    config._zoom_high = 6000;
    config._zoom_low = 1000;
    config.tog_redocc = 0;
#ifdef REGISTERED
    config.tog_welcome = 0;
#else
    config.tog_welcome = 1;
#endif
    config.tog_grid = 1;
    config.tog_shiftadd = 0;
    config.tog_join = 1;
    config.tog_unpeg = 1;
    config.tog_combineexact = 0;
    config.tog_markxlat = 1;
    config.tog_snap = S_NORMAL;
    config.tog_scrolly = 1;
    config.tog_savesrch = 1;
    config.tog_entersave = 0;
    config.tog_tiletex = 1;
    config.tog_backup = BKUP_ALWAYS;
    config.tog_runparm = 1;
    config.tog_pophelp = 1;
    config.tog_showtips = 1;

#ifdef REGISTERED
    config.tgr_default = TGR_SECONDARY;
#else
    config.tgr_default = TGR_NEVER;
#endif
    config.tgr_size1 = 3500;
    config.tgr_size2 = 1250;
    config.tgr_enemies = 0;

    config.fTexShowNames = FALSE;
    config.iTexFloorSize = 64;
    config.iTexWallSize = 128;

    strcpy( config.szEditor, "edit.com" );

    setconfig();
}

void getconfig( void )
{
    VarTransfer_t vtConfig[] =
    {
        { "HighZoom", NULL, (PTR) &config._zoom_high,
            "Enter the zoomed-out value of the AutoZoom feature" },
        { "LowZoom", NULL, (PTR) &config._zoom_low,
            "Enter the zoomed-in value of the AutoZoom feature" },
        { "BkupNever", NULL, (PTR) &config.tog_backup,
            "Never make backups" },
        { "BkupFirst", NULL, (PTR) NULL,
            "Backup the old version when you first save a newly loaded file" },
        { "BkupEvery", NULL, (PTR) NULL,
            "Backup the old version every time you save a file" },
        { "OptRedoCC", NULL, (PTR) &config.tog_redocc,
            NULL },
        { "OptWelcome", NULL, (PTR) &config.tog_welcome,
            NULL },
        { "OptShowTips", NULL, (PTR) &config.tog_showtips,
            "Display tips when DCK starts up." },
        { "OptShiftAdd", NULL, (PTR) &config.tog_shiftadd,
            "Requires that SHIFT be held to keep the current marks" },
        { "OptCombineExact", NULL, (PTR) &config.tog_combineexact,
            "Combines exact vertices automatically" },
        { "OptMarkXlate", NULL, (PTR) &config.tog_markxlat,
            "Marks lines in a marked sector, and vertices in a marked line" },
        { "OptUnpeg", NULL, (PTR) &config.tog_unpeg,
            "This makes windows and archways look a whole lot better" },
        { "OptScrolly", NULL, (PTR) &config.tog_scrolly,
            "Scrolls the map when the cursor reaches the screen edge" },
        { "OptRunparms", NULL, (PTR) &config.tog_runparm,
            "Asks for skill level parameters when you press F9" },
        { "OptPopHelp", NULL, (PTR) &config.tog_pophelp,
            "Displays popup help in dialog boxes" },
        { "DefGrid", NULL, (PTR) &config.tog_grid,
            "Displays the grid by default" },
        { "DefJoin", NULL, (PTR) &config.tog_join,
            "Turns on Auto-Join mode by default" },
        { "TGRNever", NULL, (PTR) &config.tgr_default,
            "Never displays Item graphics" },
        { "TGRPrimary", NULL, (PTR) NULL,
            "First display mode: Graphics; Second display mode: Squares" },
        { "TGRSecondary", NULL, (PTR) NULL,
            "First display mode: Squares; Second display mode: Graphics" },
        { "TGRSmall", NULL, (PTR) &config.tgr_size1,
            "Small graphics are displayed when the zoom scale is less than" },
        { "TGRBig", NULL, (PTR) &config.tgr_size2,
            "Large graphics are displayed when the zoom scale is less than" },
        { "TGRCharacters", NULL, (PTR) &config.tgr_enemies,
            "Check this to display enemies and player starts as graphics" },
        { "Editor", NULL, (PTR) config.szEditor,
            "Enter the DOS command to edit Hexen script files" },

        // end marker
        { NULL, NULL, NULL, NULL }
    };

    Dialog* pDialog = new Dialog;
    pDialog->Create( szDSFile, "ConfigDlg" );
    pDialog->VarSetup( vtConfig );

    Point ptScreen( 0, Centered );
    ptScreen.x = (iScreenWidth - pDialog->ScreenRect().Width());
    ptScreen.x = (ptScreen.x / 3) * 2;

    pDialog->MoveTo( ptScreen );

    int iRvl = pDialog->Process();
    delete pDialog;

    /* save it */
    saveconfig();

    if( iRvl == daExitSave )
        setconfig();
}
