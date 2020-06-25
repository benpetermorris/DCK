/*-------------------------------------------------------------------------*
 * GAMEDAT.C:
 * Functions for loading game types, determining the type of a certain
 * tag, etc.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop
#include <assert.h>

/*-------------------------------------------------------------------------*
 *
 * Globals begin here
 *
 *-------------------------------------------------------------------------*/

static char NOGAMEDIR[] = "Directory; [Undefined]";

static short nthingtype;
static short nlinetype;

void load_motifs( void );

extern      uchar *config_fn;
extern      int config_lineno;

static char *colors[] =
{
    "", "BLUE", "GREEN", "CYAN", "RED", "MAGENTA", "BROWN", "GREY", "DARKGREY",
    "LIGHTBLUE", "LIGHTGREEN", "LIGHTCYAN", "LIGHTRED", "LIGHTMAGENTA",
    "YELLOW", "WHITE", NULL
};

/*-------------------------------------------------------------------------*
 *
 * Functions begin here
 *
 *-------------------------------------------------------------------------*/

static short clrstr2int( char *s )
{
    short i;

    strupr( s );

    for( i = 0; colors[i]; i++ )
    {
        if( !strcmp( s, colors[i] ) )
            return i;
    }

    /* only used here, so wtf */
    error( "invalid color '%s' in %s.GD", s, pgdCurGame->cfgname );

    /* never gets here.. BUT BORLAND WILL COMPLAIN! */
    return -1;
}

static  short   newdirline[8];

void writelinefile( uchar *fn, char *new_fn, short line, uchar *linebuf )
/*-------------------------------------------------------------------------*
 * Replaces line 'line' in 'fn' with 'linebuf'.  optionally creates a new
 * file called new_fn.
 *-------------------------------------------------------------------------*/
{
    FILE    *fp = fopen( fn, "rb" );
    FILE    *fp2 = fopen( new_fn ? new_fn : "$tmp.fil", "wb" );
    short   i = 0;
    char    buf[256];

    assert( fp2 != NULL );

    for( i = 1; i < line; i++ )
    {
        fgets( buf, 255, fp );
        fputs( buf, fp2 );
    }

    /* skip the line in the original */
    fgets( buf, 255, fp );
    /* write out the new line in the new one */
    fprintf( fp2, "%s\r\n", linebuf );

    i++;

    while( 1 )
    {
        if( fgets( buf, 255, fp ) == NULL )
            break;
        fputs( buf, fp2 );
    }

    fclose( fp );
    fclose( fp2 );

    if( !new_fn )
    {
        remove( fn );
        rename( "$tmp.fil", fn );
    }
}

void fixgdfiles( void )
{
    short   i;
    char    buf[128], fn[128];

    for( i = 0; i < iMaxGame; i++ )
    {
        if( newdirline[i] != -1 )
        {
            sprintf( buf, "Directory; %s", pgdGameArray[i].dir );
            sprintf( fn, "%s.GD", pgdGameArray[i].cfgname );
            writelinefile( fn, NULL, newdirline[i], buf );
        }
    }
}

void renamegdfiles( void )
{
    struct ffblk ff;
    int done;
    char buf[128];

    done = findfirst( "*.!GD", &ff, FA_ARCH );

    while( !done )
    {
        strccpy( buf, ff.ff_name, "." );
        strcat( buf, ".GD" );
        remove( buf );
        rename( ff.ff_name, buf );
        done = findnext( &ff );

    }
}

static int dbx, dby;

static short __ff( char *file, char *destbuf )
{
    struct  ffblk ff;
    short   done;
    static  char dirbuf[128];
    static  int dbfind = 0;

/*
    static  short   twirly_idx;
    static  char    twirlies[] = "|/-\\";

    printf( "%c\b", twirlies[twirly_idx++] );
    if( twirly_idx == 4 )
        twirly_idx = 0;
*/

    getcwd( dirbuf, 128 );

    if( access( file, 0 ) != -1 )
    {
        strcpy( destbuf, dirbuf );
        return 1;
    }

    if( dbfind++ == 10 )
    {
        dbfind = 0;
        printf( "%-50s", dirbuf );
        gotoxy( dbx, dby );
    }

    done = findfirst( "*.*", &ff, FA_DIREC );

    while( !done )
    {
        if( ff.ff_attrib & FA_DIREC && ff.ff_name[0] != '.' )
        {
            chdir( ff.ff_name );
            if( __ff( file, destbuf ) )
                return 1;
        }
        done = findnext( &ff );
    }

    chdir( ".." );

    return 0;
}

short file_find( char *file, char *destbuf )
{
    short   curdrive = getdisk(), diskidx;
    char    dir[128];

    getcwd( dir, 128 );

    for( diskidx = 2; diskidx < 26; diskidx++ )
    {
        setdisk( diskidx );
        if( getdisk() == diskidx )
        {
            chdir( "\\" );
            if( __ff( file, destbuf ) )
            {
                setdisk( curdrive );
                chdir( dir );
                return 1;
            }
        }
    }

    setdisk( curdrive );
    chdir( dir );

    return 0;
}

short findgamefiles( GAMEDATA *pgdGame )
{
    char    dir[64], buf[128];
    short   diskidx, curdrive = getdisk();

    printf( "þ Scanning for %s: ", pgdGame->name );

    strcpy( dir, pgdGame->dir + 3 );

    for( diskidx = 2; diskidx < 26; diskidx++ )
    {
        setdisk( diskidx );
        if( getdisk() == diskidx )
        {
            /* it's there, check it */
            sprintf( buf, "%c:\\%s\\%s", 'A' + diskidx, dir, pgdGame->pwdIWAD );

            if( access( buf, 0 ) != -1 )
            {
                /* found it eh. */
                sprintf( pgdGame->dir, "%c:\\%s", 'A' + diskidx, dir );
                setdisk( curdrive );
                printf( "Found in '%s'.\n", pgdGame->dir );
                return 1;
            }
        }
    }

    setdisk( curdrive );

    buf[0] = 0;
    printf( "\rþ If you have the game %s, enter its directory below.  If\n"
            "  you'd prefer to have "_ABBREV_" search for the game, just press ENTER.\n"
            "  ** If you don't have the game (or it's the shareware version),\n"
            "     press ESC:\n  ", pgdGame->name );

    if( editdir( buf, wherex(), wherey(), WHITE_BLUE, 70 ) == 0 )
        return 0;
    if( buf[0] )
    {
        strcpy( pgdGame->dir, buf );
        return -1;  // re find that game
    }

    printf( "þ Searching for %s: ", pgdGame->name );
    dbx = wherex();
    dby = wherey();

    /* now.. find it using standard methods */
    if( file_find( pgdGame->pwdIWAD, buf ) )
    {
        strcpy( pgdGame->dir, buf );
        printf( "Found in '%s'.\n", pgdGame->dir );
        return 1;
    }

    printf( "Not found.\n" );

    return 0;
}

static bool fNoLoad;

int _load_general( uchar *notused )
/*-------------------------------------------------------------------------*
 * Creates a new pgdCurGame entry and sets "pgdCurGame" to point to it.. after
 * loading all the information.
 *-------------------------------------------------------------------------*/
{
    GAMEDATA    *pgdNewGame;
    uchar       buf[128], lvalue[64], rvalue[81];
    int         tmpint;
    bool        fWriteDat = FALSE;

    fNoLoad = FALSE;
    newdirline[iMaxGame] = -1;

    /* have to init it */
    if( pgdGameArray == NULL )
    {
        /* four is probably fine. */
        pgdGameArray = (GAMEDATA*) memget( sizeof( GAMEDATA ) * 8 );
    }

    pgdNewGame = &pgdGameArray[iMaxGame++];
    memset( pgdNewGame, 0, sizeof( GAMEDATA ) );

    /* copy in everything but the extension - no dir, 'cause it's in the
     * current dir. */
    strccpy( pgdNewGame->cfgname, config_fn, "." );

    /* read in the config */
    while( getline_cfg( buf ) != NULL )
    {
        parseline_cfg( buf, lvalue, rvalue, NULL );

        /* get the keyword and copy the rvalue into the right place */
        if( !strcmpi( lvalue, "Name" ) )
            strcpy( pgdNewGame->name, rvalue );
        else if( !strcmpi( lvalue, "Directory" ) )
        {
            strcpy( pgdNewGame->dir, rvalue );
            if( lastchar( pgdNewGame->dir ) == '\\' )     /* can't have that, eh. */
                lastchar( pgdNewGame->dir ) = 0;
            newdirline[iMaxGame-1] = config_lineno;
        }
        else if( !strcmpi( lvalue, "IWAD" ) )
            strcpy( pgdNewGame->pwdIWAD, rvalue );
        else if( !strcmpi( lvalue, "Tag" ) )
            strcpy( pgdNewGame->tag, rvalue );
        else if( !strcmpi( lvalue, "New Map" ) )
            strcpy( pgdNewGame->newtag, rvalue );
        else if( !strcmpi( lvalue, "Run" ) )
            strcpy( pgdNewGame->run, rvalue );
        else if( !strcmpi( lvalue, "Nodes" ) )
            strcpy( pgdNewGame->szNodes, rvalue );
        else if( !strcmpi( lvalue, "Palette" ) )
            strcpy( pgdNewGame->palette, rvalue );
        else if( !strcmpi( lvalue, "LargeData" ) )
            pgdNewGame->fLargeData = atoi( rvalue );
    }

    /* find the directory.. we hope */

_redo_look:
    errno = 0;
    tmpint = _chmod( pgdNewGame->dir, 0 );

    // if the directory doesn't exist, or it's not a directory ..
    if( tmpint == -1 || !(tmpint & FA_DIREC) )
    {
        /* search for games? */
        if( tmpint == -1 && fFindGames )
        {
            /* try to find it */
            while( 1 )
            {
                tmpint = findgamefiles( pgdNewGame );
                if( tmpint == 0 )   // not found
                    break;
                if( tmpint == 1 )   // found
                    goto _found_game;
                if( tmpint == -1 )  // redo
                {
                    fWriteDat = TRUE;
                    goto _redo_look;
                }
            }
        }

        if( strcmp( pgdNewGame->dir, "[Undefined]" ) )
        {
            printf( "* Notice: Could not find the game '%s.'\n"
                    "  (The directory '%s' does not exist.)\n", pgdNewGame->name, pgdNewGame->dir );
            sprintf( buf, "%s.!GD", pgdNewGame->cfgname );
            writelinefile( config_fn, buf, newdirline[iMaxGame-1], NOGAMEDIR );
        }

        iMaxGame--; /* next one will overwrite this one */
        fNoLoad = TRUE;
    }
    else if( !fWriteDat )
    {
        /* don't update gd file.. no need to */
        newdirline[iMaxGame-1] = -1;
    }

_found_game:
    /* add pwdIWAD to the texture list */
    sprintf( pgdNewGame->texwads[0] = (uchar*) memget( 128 ), "%s\\%s", pgdNewGame->dir, pgdNewGame->pwdIWAD );

    // see if we can find the IWAD
    if( !fNoLoad && access( pgdNewGame->texwads[0], 0 ) == -1 )
    {
        printf( "* Notice: Could not find %s's IWAD.\n"
                "  (You will not be able to create levels for this game.)\n", pgdNewGame->name );
        sprintf( buf, "%s.!GD", pgdNewGame->cfgname );
        writelinefile( config_fn, buf, newdirline[iMaxGame-1], NOGAMEDIR );
        iMaxGame--; /* next one will overwrite this one */
        fNoLoad = TRUE;
    }

    if( fNoLoad )
        return -1;  // ignore this one

    /* allocate space for tables */
    memset( pgdNewGame->ttype2idx, 0xff, sizeof pgdNewGame->ttype2idx );

    pgdNewGame->stlThingSections = new StrList( 32, 50, "ThingSec" );
    pgdNewGame->stlThingTypes = new StrList( 384, 50, "ThingTypes" );
    pgdNewGame->stlActSections = new StrList( 16, 50, "ActSec" );
    pgdNewGame->stlActTypes = new StrList( 1024, 50, "ActTypes" );
    pgdNewGame->stlSectorTypes = new StrList( 128, 70, "SecTypes" );
    pgdNewGame->stlAngles = new StrList( 10, 32, "Angles" );

    pgdNewGame->stlThingSections->AddTail( "(All)" );
    pgdNewGame->stlActSections->AddTail( "(All)" );

    if( pgdNewGame->fLargeData )
    {
        pgdNewGame->stlThingArgs = new StrList( 32, 50, "ThingArgs" );
        pgdNewGame->stlActArgs = new StrList( 256, 50, "ActArgs" );
    }

    nthingtype = nlinetype = 0;

    pgdCurGame = pgdNewGame;  /* set the global */

    if( strchr( (char*) pgdNewGame->run, '\\' ) == NULL )
    {
        sprintf( buf, "%s\\%s", pgdNewGame->dir, pgdNewGame->run );
        strcpy( pgdNewGame->run, buf );
    }
    /* do some checks for it now */
    strccpy( buf, pgdNewGame->run, " /" );
    if( access( buf, 0 ) == -1 )
        printf( "* Warning: %s.GD points to an invalid \"run\" parameter:\n  '%s'", pgdNewGame->cfgname, buf );
    if( pgdNewGame->newtag[0] == 0 )
        error( "%s.GD does not have a \"New Map\" entry", pgdNewGame->cfgname );
    if( pgdNewGame->palette[0] == 0 )
        error( "%s.GD does not have a \"Palette\" entry", pgdNewGame->cfgname );
}

int _load_done( uchar *notused )
{
    if( !pgdCurGame->stlThingTypes->GetCount() )
        error( "%s.GD does not have any Thing Types defined", pgdCurGame->cfgname );
    if( !pgdCurGame->stlActTypes->GetCount() )
        error( "%s.GD does not have any Action Types defined", pgdCurGame->cfgname );
    if( !pgdCurGame->stlSectorTypes->GetCount() )
        error( "%s.GD does not have any Sector Types defined", pgdCurGame->cfgname );
    if( !pgdCurGame->stlAngles->GetCount() )
        error( "%s.GD does not have any Angles defined", pgdCurGame->cfgname );

    return TRUE;
}

int _load_tidactions( uchar* )
{
    short   idx = 0;
    uchar   buf[128];

    memset( &(pgdCurGame->ActionsWithTID), 0xFF, sizeof (pgdCurGame->ActionsWithTID) );

    while( getline_cfg( buf ) != NULL )
        pgdCurGame->ActionsWithTID[idx++] = atoi( buf );

    return TRUE;
}

int _load_thingargs( uchar *pszName )
/*-------------------------------------------------------------------------*
 * Loads thing args.
 *-------------------------------------------------------------------------*/
{
    char    buf[128], args[128];
    int     id;

    int iLines, iCols;
    countline_cfg( &iLines, &iCols );

    StrList *stlThingArgs;
    pgdCurGame->stlThingArgs = stlThingArgs =
        new StrList( iLines+1, iCols+1, (char*) pszName );

    while( getline_cfg( buf ) != NULL )
    {
        id = atoi( buf );

        char *ptr = strchr( buf, ';' ) + 1;
        strcvt( args, ptr, RABLANKS, 80 );
        stlThingArgs->AddTail( args, id );
    }

    return TRUE;
}

int _load_popup( uchar *pszName )
/*-------------------------------------------------------------------------*
 * loads a popup list.
 *-------------------------------------------------------------------------*/
{
    uchar   buf[128];
    char    nam[64], idstr[64];

    int iLines, iCols;
    countline_cfg( &iLines, &iCols );

    StrList *stlPopup;
    pgdCurGame->stlPopups[pgdCurGame->nPopups++] = stlPopup =
        new StrList( iLines+1, iCols+1, (char*) pszName );

    while( getline_cfg( buf ) != NULL )
    {
        parseline_cfg( buf, nam, idstr, NULL );
        stlPopup->AddTail( nam, atoi( idstr ) );
    }

    return TRUE;
}

int _load_texwads( uchar *notused )
/*-------------------------------------------------------------------------*
 * Loads the list of texture .WADs that go with this game data file
 *-------------------------------------------------------------------------*/
{
    uchar   buf[128];
    short   idx = 1;    /* 1, not 0 .. pwdIWAD is already there. */

    while( getline_cfg( buf ) != NULL )
        strcpy( pgdCurGame->texwads[idx++] = (uchar*) memget( 128 ), buf );

    return TRUE;
}

int _load_lnotag( uchar *notused )
/*-------------------------------------------------------------------------*
 * Loads the list of line numbers that don't require tags,
 * or HEXEN: line numbers that do require tags in arg0.
 *-------------------------------------------------------------------------*/
{
    short   idx = 0;
    uchar   buf[128];

    memset( &(pgdCurGame->lines_notag), 0xFF, sizeof (pgdCurGame->lines_notag) );

    while( getline_cfg( buf ) != NULL )
        pgdCurGame->lines_notag[idx++] = atoi( buf );

    return TRUE;
}

int _load_tinfo( uchar *notused )
/*-------------------------------------------------------------------------*
 * Loads the thing information section, eh.
 * Each one contains a number - the thing number, its radius, its height.
 * Only replaces info for those things that are specified.
 *-------------------------------------------------------------------------*/
{
    uchar   buf[128];
    uchar   type[64], radius[64], height[64], dangle[32];
    TInfo   *tip = pgdCurGame->thinginfo, *tipi;
    short   thingno;

    while( getline_cfg( buf ) != NULL )
    {
        parseline_cfg( buf, type, radius, height, dangle, NULL );

        thingno = pgdCurGame->ttype2idx[atoi( type )];

        if( thingno == -1 )
            continue;

        tipi = &pgdCurGame->thinginfo[thingno];

        tipi->dangle = (dangle[0] == '1' || dangle[0] == '3') ? 1 : 0;
        tipi->clash  = (dangle[0] == '2' || dangle[0] == '3') ? 1 : 0;
        tipi->height = atoi( height );
        tipi->radius = atoi( radius );
    }

    return TRUE;
}

int _load_things( uchar *secname )
/*-------------------------------------------------------------------------*
 * Loads a "things" section from the current config file into the current
 * game data.  TInfo must remain synchronous with thingtype strtable.
 *-------------------------------------------------------------------------*/
{
    uchar   buf[128];
    uchar   nam[64], idstr[64], sprite[128];
    TInfo   *tip = pgdCurGame->thinginfo, *tipi;
    short   *tt2idxp = pgdCurGame->ttype2idx;
    short   curcolor = 7;

    /* add the section name */
    pgdCurGame->stlThingSections->AddTail( secname, nthingtype );

    /* make a keyword */
    sprintf( buf, "*%s", secname );
    pgdCurGame->stlThingTypes->AddTail( buf, 32000 );
    tip[nthingtype].type = 0;
    ++nthingtype;

    /* now add all the things */
    while( getline_cfg( buf ) != NULL )
    {
        parseline_cfg( buf, nam, idstr, sprite, NULL );

        if( !strcmpi( nam, "Color" ) )
        {
            curcolor = clrstr2int( idstr );
            continue;
        }

        pgdCurGame->stlThingTypes->AddTail( nam, nthingtype );

        tipi = &tip[nthingtype];

        tipi->type = atoi( idstr );
        tt2idxp[tipi->type] = nthingtype;
        tipi->color = curcolor;
        tipi->dangle = 0;
        tipi->height = 16;
        tipi->radius = 16;
        strcpy( tipi->sprite, sprite );

        ++nthingtype;
    }

    tip[nthingtype].type = -1;  /* end of table */

    return TRUE;
}

int _load_actions( uchar *secname )
/*-------------------------------------------------------------------------*
 * Loads a "actions" section from the current config file into the current
 * game data structure.
 *-------------------------------------------------------------------------*/
{
    char   buf[128];
    char   nam[64], idstr[64];
    char    args[80];
    bool    fHexen = pgdCurGame->fLargeData;

    /* add the section name */
    pgdCurGame->stlActSections->AddTail( secname, nlinetype );

    /* make a keyword */
    sprintf( buf, "*%s", secname );
    pgdCurGame->stlActTypes->AddTail( buf, 32000 );
    ++nlinetype;

    /* now add all the lines */
    while( getline_cfg( buf ) != NULL )
    {
        parseline_cfg( buf, nam, idstr, NULL );

        if( fHexen )
        {
            char *ptr = strchr( buf, ';' ) + 1;
            ptr = strchr( ptr, ';' ) + 1;
            while( *ptr == 32 ) ++ptr;
            strcvt( args, ptr, RABLANKS, 80 );
            pgdCurGame->stlActArgs->AddTail( args, atoi( idstr ) );
        }

        pgdCurGame->stlActTypes->AddTail( nam, atoi( idstr ) );
        ++nlinetype;
    }

    return TRUE;
}

int _load_sectors( uchar *notused )
/*-------------------------------------------------------------------------*
 * Loads the sector types
 *-------------------------------------------------------------------------*/
{
    uchar   buf[128];
    uchar   nam[64], idstr[64];

    /* add all the lines */
    while( getline_cfg( buf ) != NULL )
    {
        parseline_cfg( buf, nam, idstr, NULL );
        pgdCurGame->stlSectorTypes->AddTail( nam, atoi( idstr ) );
    }

    return TRUE;
}

int _load_angles( uchar *notused )
/*-------------------------------------------------------------------------*
 * Loads the angles
 *-------------------------------------------------------------------------*/
{
    uchar   buf[128];
    uchar   nam[64], idstr[64];

    /* add all the lines */
    while( getline_cfg( buf ) != NULL )
    {
        parseline_cfg( buf, nam, idstr, NULL );
        pgdCurGame->stlAngles->AddTail( nam, atoi( idstr ) );
    }

    return TRUE;
}

void game_switch( int gamenum )
/*-------------------------------------------------------------------------*
 * Switches global data pointers to a new game data
 *-------------------------------------------------------------------------*/
{
    extern  short motif_num;      /* dck.c -- used for status save/restore */
    extern  byte  ts_reload;

    short   idx, numtexwads;
    WAD*    wptr;
    GWINDOW *gwnp = NULL;

    static short first = 1, lastgame = -1;

    if( !first || !(restore & R_MAP) )
        motif_num = 0;  /* not valid anymore */

    pgdCurGame = &pgdGameArray[gamenum];
    iCurGame = gamenum;

    if( gamenum != lastgame )
    {
        lastgame = gamenum;
        gwnp = wn_def( Centered, Centered, 27, 3, C_GRAY3, C_GRAY1,
                        WF_POPUP | WF_3D, st_dcksays );
        wn_disp( gwnp );
        cursor_put( CSR_ERASE );
        wn_xytext( gwnp, Centered, 1, "Switching to %s", pgdCurGame->name );
        cursor_put( CSR_PUT );

        ts_reload = 1;
    }

    /* open the pwdIWAD */
    if( pwdIWAD )
        wad_close( pwdIWAD );

    /* first is always the pwdIWAD */
    pwdIWAD = wad_open( pgdCurGame->texwads[0], WL_COMPLETE );

    if( !pwdIWAD )
        error( "couldn't open the IWAD ('%s')", pgdCurGame->texwads[0] );

    if( !wad_find( pwdIWAD, "F2_END" ) )
        error( "your copy of the game is not registered." );

    /* make sure textures are loaded */
    if( !(pgdCurGame->texlist) )
        init_textures();

    /* set global variables to gamedat's variables to save time */
    ntex = pgdCurGame->ntex;
    nfloor = pgdCurGame->nfloor;
    texlist = pgdCurGame->texlist;
    floorlist = pgdCurGame->floorlist;

    load_motifs();

    motif_set( motif_num );

    /* add these texture wads - only the first time though. */
    if( first )
    {
        for( idx = 0; loadtexwads[idx]; idx++ )
        {
            wptr = wad_open( loadtexwads[idx], WL_COMPLETE );
            if( wptr )
            {
                addtexwad( wptr );
                wad_close( wptr );
            }
        }
    }

    if( gwnp )
    {
        wn_undisp( gwnp );
        wn_free( gwnp );
    }

    loadpalette( pgdCurGame->palette );

    stlThingSections = pgdCurGame->stlThingSections;
    stlThingTypes    = pgdCurGame->stlThingTypes;
    stlThingArgs     = pgdCurGame->stlThingArgs;
    stlActSections   = pgdCurGame->stlActSections;
    stlActTypes      = pgdCurGame->stlActTypes;
    stlActArgs       = pgdCurGame->stlActArgs;
    stlSectorTypes   = pgdCurGame->stlSectorTypes;
    stlAngles        = pgdCurGame->stlAngles;
    stlFlats         = pgdCurGame->stlFlats;

    fHexen           = pgdCurGame->fLargeData;

    nMapEntryCount   = 11 + fHexen;

    first = 0;
}
