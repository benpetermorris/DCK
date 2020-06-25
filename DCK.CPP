/*-------------------------------------------------------------------------*
 * DCK.C:                                                                  *
 * Issa doom editor!                                                       *
 *-------------------------------------------------------------------------*/

#define _MAINMODULE

#pragma hdrstop
#define _DCK_C      /* some header files use this */
#include <std.h>
#include "gui\gui.hpp"
#include "zlib.h"
#include "defs.h"

#ifdef REGISTERED
#include "dckendrg.h"
#else
#include "dckendsh.h"
#endif

/*-------------------------------------------------------------------------*
 *                                                                         *
 * String definitions                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

char *st_ok   =    "   Ok   ";         /* for ok buttons */
char *st_cancel =  " Cancel ";         /* for cancel buttons */
char *st_blank =   "";                 /* blank, dude. */
char *st_version = "3.62";              /* version string */
char *st_dcksays = _ABBREV_ " Says";         /* dck says. */

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

extern int ismouse;

short mnu_pop( short x, short y, short firstitem, uchar *title, uchar *help, ... ) {return 0;}

uchar status_restore( uchar );
void fixgdfiles( void );
void renamegdfiles( void );
void status_save( void );
void saveconfig( void );
short ad_compile( char *inf );

void    _ExceptInit( void ) {}

/* File variables                                                          */
uchar       szCurMapTag[9];     /* current level tag */
WAD         *pwdCurWad;         /* current .wad */
WAD         *pwdIWAD;           /* current pwdIWAD for the game */
GAMEDATA    *pgdGameArray;
short       iMaxGame;
GAMEDATA    *pgdCurGame;        /* current game data */
short       iCurGame;           /* current game data number */
MAPOPTIONS  MapOptions;
bool        fHexen;             // hexen game?
int         nMapEntryCount;     // entrycount for each map

/* Misc. variables                                                         */
short   cursorx, cursory;       /* x and y of cursor location */
uchar   *_memstr;               /* What we're doing with memory for debugging */
uchar   fFullUpdate;            /* Perform a full update? */
ushort  nMadeChanges;           /* Made changes on current file? */
ushort  nMapChanges;            /* map changes on current file? */
short   iLastThingEdited = -1;        /* Last thing so we can duplicate it */
short   torigin_x = -1, torigin_y;   /* thing origin */
uchar   restore = 0xFF;         /* restoring last edit? */
bool    fNewMap;               // new file?
short   _fontheight;            /* guess :) */
short   _fontwidth;
short   _vradius;
short   fGetConfig;             /* does DCK32.CFG *not* exist? */
short   fFindGames;             /* find games? */
short   iExitLev = EL_EXIT;
short   get_out;                /* if 1, exit dck from input.c */
TFlags  td_flags = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0 };        /* thing display flags */

/* Defaults                                                                */
short   _def_ftxidx;            /* default full texture index */
short   _def_utxidx;            /* default upper texture index */
short   _def_ltxidx;            /* default lower texture index */
short   _def_stairtxidx;        /* default stair texture index */
short   _def_doorbig;
short   _def_doorsmall;
short   _def_doortrak;
short   _def_light;
WADNAME _def_ceiltex;
WADNAME _def_liftfloor;
WADNAME _def_floortex;
Config  config;                 /* configuration array */

/* Map test variables                                                      */
int     iMapSkill = 2;
byte    fMapNoMonsters;
int     iMapClass = 0;

/* Graphics variables                                                      */
short   lastvidmode;            /* To prevent unnecessary switching */
short   lastcolor;              /* To prevent unnecessary color changes */
short   sxlen, sylen;           /* X and Y size of the viewport */
short   mxplen, myplen;         /* X and Y size of the map area */
short   mxpstart, mypstart;     /* X and Y start of the map area */
short   xstart, ystart;         /* MAP x and y coords that appear at top
                                 * left of viewport. */
short   xscale;                 /* Scale (zoom) factor.  This is used on
                                 * all sc_xxx functions, which take virtual
                                 * coordinates and not screen coordinates. */
short   yscale;
long    sc_x, sc_y;             /* Precalculated variables at each update */
long    sc_x_r, sc_y_r;         /* Precalcs (screen->map) */
short   sb_y;                   /* status bar Y begin */
short   gridstep = 64;
short   editmode = MODE_LINE;   /* Edit mode is line mode to begin with */
uchar   marklist[MAXOBJECT];    /* boolean index array of marked objects */
short   file_version = 0x362;   /* file versions .. woo woo ooo ooo */
uchar   *loadtexwads[16];       /* load these texture wads. */
short   sel_lenience;           /* selection lenience */
char    szDSFile[128];          // dialog script filename

/* map variables                                                         */
VERTEX  *vlist;                 /* List of vertices in current map */
short   nvertex = 0;            /* Number of vertices in current map */
short   maxvertex;
LINEDEF *linelist;              /* Lines in current map */
short   nline = 0;              /* Number of lines */
short   maxline;
THING   *thinglist;             /* Things in current map */
short   nthing = 0;             /* Number of things in map */
short   maxthing;
SECTOR  *sectorlist;            /* Sectors in current map */
short   nsector = 0;            /* Number of sectors */
short   maxsector;
SIDEDEFM *sidelist;             /* Sidedefs in current map */
short   nside = 0;              /* Number of sides */
short   maxside;
TEXTURE *texlist;               /* Texture list */
short   ntex = 0;               /* Number of textures */
WADENTRY *floorlist;            /* List of floors */
short   nfloor = 0;             /* Number of floors */
SSector *ssectors;
long    num_ssectors = 0;
Pseg    *psegs = NULL;
long    num_psegs = 0;
Pnode   *pnodes = NULL;
long    num_pnodes = 0;
Block   blockhead;
short   *blockptrs;
short   *blocklists = NULL;
long    blockptrs_size;
long    blockmap_size;
char    *reject = NULL;
long    reject_size;
long    lBehaviorSize;
char    *pBehavior;

/* Misc. toggles                                                           */
uchar   tog_fine = 0;           /* Fine mode on? */
uchar   tog_grid = 1;           /* Grid mode on? */
uchar   tog_drag = 0;           /* Drag mode on? */
uchar   tog_mbox = 0;           /* mark box mode on? */
uchar   tog_snap = 1;           /* snap mode on? */
uchar   tog_key = 0;            /* key box (more info) mode on? ( Not used ) */
uchar   tog_join = 0;           /* autojoin mode on? */
uchar   tog_csize = 0;          /* cursor size (0 = small) */
uchar   tog_vradius = 0;
uchar   tog_quiet = 0;
uchar   tog_tgraph = 0;
uchar   tog_poly = 0;
uchar   tog_rect = 0;

/* current game pointer aliases                                            */
StrList *stlThingSections;
StrList *stlThingTypes;
StrList *stlThingArgs;
StrList *stlActSections;
StrList *stlActTypes;
StrList *stlActArgs;
StrList *stlSectorTypes;
StrList *stlAngles;
StrList *stlFlats;

// higher gray numbers mean darker

#if 0
short guito256[][4] =
{
    { 0, 0, 0, 0 },          //BLACK
    { 0, 0, 100, 1 },        //BLUE
    { 0, 170, 0, 2 },        //GREEN
    { 0, 170, 170, 3 },      //CYAN
    { 170, 0, 0, 4 },        //RED
    { 170, 0, 170, 5 },      //MAGENTA
    { 170, 85, 0, 6 },       //BROWN
    { 220, 220, 220, 7 },    //GRAY1
    { 160, 160, 160, 8 },    //GRAY3
    { 85, 85, 255, 9 },      //LIGHTBLUE
    { 85, 255, 85, 10 },     //LIGHTGREEN
    { 85, 255, 255, 11 },    //LIGHTCYAN
    { 255, 85, 85, 12 },     //LIGHTRED
    { 255, 85, 255, 13 },    //LIGHTMAGENTA
    { 255, 255, 85, 14 },    //YELLOW
    { 255, 255, 255, 15 },   //WHITE
    { 210, 210, 210, 94 },   //GRAY2
    { 115, 115, 115, 106 },  //GRAY4
    { 35, 35, 35, 6 },       //GRAY5
    { 0, 171, 171, 194 },  //ILINE
    { 0, 0, 255, 200 },      //PLINE
    { 30, 30, 100, 240 },    //GRID
    { 255, 255, 255, 168 },  //WHITE2
    { -1, -1, -1, -1 }       //DUH2
};
#else
short   guito256[][4] =
{
    { 0, 0, 0, 0 },          //BLACK
    { 0, 0, 100, 1 },        //BLUE
    { 0, 170, 0, 2 },        //GREEN
    { 0, 170, 170, 3 },      //CYAN
    { 170, 0, 0, 4 },        //RED
    { 170, 0, 170, 5 },      //MAGENTA
    { 170, 85, 0, 6 },       //BROWN
    { 190, 190, 190, 7 },    //GRAY1
    { 130, 130, 130, 8 },    //GRAY3
    { 85, 85, 255, 9 },      //LIGHTBLUE
    { 85, 255, 85, 10 },     //LIGHTGREEN
    { 85, 255, 255, 11 },    //LIGHTCYAN
    { 255, 85, 85, 12 },     //LIGHTRED
    { 255, 85, 255, 13 },    //LIGHTMAGENTA
    { 255, 255, 85, 14 },    //YELLOW
    { 255, 255, 255, 15 },   //WHITE
    { 180, 180, 180, 94 },   //GRAY2
    { 80, 80, 80, 106 },     //GRAY4
    { 50, 50, 50, 6 },       //GRAY5
    { 171, 171, 255, 194 },
    { 0, 0, 255, 200 },
    { 30, 30, 100, 240 },    //GRID
    { 255, 255, 255, 168 },  //WHITE2
    { 0, 0, 95, 0 },         //GRIDLINE
    { -1, -1, -1, -1 }       //DUH2
};
#endif

/* For this module only                                                    */
short   motif_num;              /* Last motif selected. */
uchar   load_fn[128];           /* to load from dck.sta */
uchar   load_tag[128];          /* load tag */
uchar   load_game[128];         /* game filename */
short   ntw;                    /* for loading texture wads */
ulong   resourcemax;            /* Number of bytes for resources */

void helpme( void )
{
printf(
" Options are:      (* specifies a default option, append '-' to turn it off)\n"
"   file [map]          edits specified file, and optional map\n"
"   /game xxx           switches to specified game\n"
" * /r                  restores previous edit state\n"
"   /vidmode            reconfigure video modes\n"
"   /t file1 [file2..]  load texture files\n"
"   /findgames          find new games, or re-find existing ones\n"
"\n"
" By default, "_ABBREV_" restores the previous edit state if you run it with\n"
" no file or map parameters.  This includes the loaded map, the current\n"
" position and zoom factors, and edit mode.\n"
);
    exit( EL_EXIT );
}

ushort dataseg;

#include "object.hpp"
#include "proto.h"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

void * operator new( size_t Type_size )
{
    if( !_memstr )
        _memstr = "Single new object";
    return memget( Type_size );
}

void * operator new[]( size_t Type_size )
{
    if( !_memstr )
        _memstr = "Multiple new objects";
    return memget( Type_size );
}

void operator delete( void *Type_ptr )
{
    memfree( Type_ptr );
}

#ifndef __FLAT__
unsigned short mycs, myip;

void far div0proc( void )
{
    asm     pop bp;
    asm     pop myip;
    asm     pop mycs;
    asm     push mycs;
    asm     push myip;

    printf( "Divide Error at %04X:%04X\n", mycs, myip );
    exit( EL_EXIT );
}

void setdiv0( void )
{
    struct  SREGS   sregs;
    union   REGS    regs;

    regs.x.ax = 0x2500;
    segread( &sregs );
    sregs.ds = FP_SEG( div0proc );
    regs.x.dx = FP_OFF( div0proc );
    int86x( 0x21, &regs, &regs, &sregs );
}
#endif

char *d_strs[] =
{
    "ATI Wonder (XL)",
    "ATI Graphics Ultra Pro",
    "VESA Driver",
    "Compaq Qvision and Advanced VGA",
    "Tseng 3000 chipset",
    "Tseng 4000 chipset",
    "Genoa 5000",
    "Genoa 6000",
    "Oak OTI-067",
    "Western Digital Paradise",
    "Tecmar VGA AD",
    "Trident Impact VGA",
    "Video 7 VRAM, Fastwrite, 1024i",
    "Video 7 VRAM II",
    "S3-based card",
    NULL
};

char *r_strs[] =
{
    "640x480",
    "800x600",
    "1024x768",
    "1280x1024",
    NULL
};

short cfg_vidmode( void )
/*-------------------------------------------------------------------------*
 * Configures the video mode.
 *-------------------------------------------------------------------------*/
{
    short   driver, mode;

    driver = bar_menu( d_strs, " Choose a video driver ", config.driver,
                    Centered, Centered );
    if( driver == -1 )
        return 0;

    mode = bar_menu( r_strs, " Choose a resolution ", config.mode,
                    Centered, Centered );
    if( mode == -1 )
        return 0;

    config.driver = driver;
    config.mode = mode;

    return 1;
}

void resource_alloc( void )
/* Read in the maximums and allocate storage for the lists                 */
{
    resourcemax = 0L;

    maxvertex = 8192;
    _memstr = "vertex list";
    vlist = (VERTEX*) memget( maxvertex * sizeof( VERTEX ) );
    resourcemax += (long) (maxvertex * sizeof( VERTEX ));

    maxline = 6144;
    _memstr = "line list";
    linelist = (LINEDEF*) memget( maxline * sizeof( LINEDEF ) );
    resourcemax += (long) (maxline * sizeof( LINEDEF ));

    maxside = 8192;
    _memstr = "side list";
    sidelist = (SIDEDEFM*) memget( maxside * sizeof( SIDEDEFM ) );
    resourcemax += (long) (maxside * sizeof( SIDEDEFM ));

    maxsector = 3072;
    _memstr = "sector list";
    sectorlist = (SECTOR*) memget( maxsector * sizeof( SECTOR ) );
    resourcemax += (long) (maxsector * sizeof( SECTOR ));

    maxthing = 3072;
    _memstr = "thing list";
    thinglist = (THING*) memget( maxthing * sizeof( THING ) );
    resourcemax += (long) (maxthing * sizeof( THING ));
}

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Entry point                                                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/

void qprintf( char *fmt, ... )
{
    va_list vl;

    va_start( vl, fmt );
    if( !tog_quiet )
        vprintf( fmt, vl );
    va_end( vl );
}

uchar   *dplayfn = NULL, *drecfn = NULL;

extern short me_proc;
extern long me_total;
extern ulong me_nothing_e;
extern ulong me_nothing_d;

void CommitStack()
// NOTE: You must compile with the -N switch to enable stack overflow
// checking before this function can work (Borland uses the stack
// check prologue to commit locals that exceed 4K)
{
   long stackgrow[8192];  // 32K bytes - commit a big chunk of stack space

   stackgrow[8191] = 0;   // to force the stack to grow and become committed
                          // we must touch a location near the end.
}

static void _dckmain( int argc, char *argv[] );

void main( int argc, char *argv[] )
{
    // reset clock
_asm {
    push ax
    mov al, 0x46
    out 0x43, al
    mov ax, 0xffff
    out 0x40, al
    mov al, ah
    out 0x40, al
}

    struct blahType
    {
    char a[329];         // tweeking with this number gets it to blow up
    }b;
    b.a[0] = 'a';
    CommitStack();
    _dckmain( argc, argv );
}

static void _dckmain( int argc, char *argv[] )
{
    short   i, done, cfgv = 0;
    struct  ffblk ff;
    uchar   parm[80], switchch;       /* 0 = '-' / 1 = '+' */

#ifndef __FLAT__
    setdiv0();
#else
    setvbuf( stdout, NULL, _IONBF, 0 );
#endif

    randomize();
    strcpy( szDSFile, argv[0] );
    char *ptr = strrchr( szDSFile, '\\' );
    strcpy( ptr, "\\DCK.DS" );

    if( argc > 1 && !strcmpi( argv[1], "/quiet" ) )
        tog_quiet = 1;

    qprintf( _FULL_" v%s, (c) Copyright 1994-96 Zerius Development\n"
             " Usage: "_ABBREV_" [options].  "_ABBREV_" /? for help.\n\n", st_version );
//    qprintf( "*** BETA VERSION - DO NOT DISTRIBUTE ***\n\n" );

/* Check command-line arguments                                            */
    for( i = 1; i < argc; i++ )
    {
        strupr( argv[i] );

        if( *argv[i] == '-' )
            *argv[i] = '/';

        if( strchr( argv[i], '?' ) != NULL )
            helpme();

        /* Get the real parm string */
        strccpy( parm, argv[i]+1, " -+" );
        strupr( parm );
        switchch = lastchar( argv[i] ) == '-' ? 0 : 1;

        /* Restore last status? */
        if( !strcmpi( parm, "R" ) || !strcmpi( parm, "RESTORE" ) )
            restore = switchch ? 0xFF : R_MOTIF;
        else if( !strcmpi( parm, "FINDGAMES" ) )
            fFindGames = switchch;
        else if( !strcmpi( parm, "T" ) )
        {
            ntw = 0;
            ++i;
            while( i < argc && !strchr( "/-", argv[i][0] ) )
                loadtexwads[ntw++] = argv[i++];
            --i;
        }
        else if( !strcmpi( parm, "quiet" ) )
            continue;   /* ignore this */
        else if( !strcmpi( parm, "record" ) )
        {
            if( argv[++i] )
            {
                drecfn = argv[i];
                printf( "þ Recording demo %s\n", argv[i] );
                restore = 0;
            }
        }
        else if( !strcmpi( parm, "play" ) )
        {
            if( argv[++i] )
            {
                dplayfn = argv[i];
                printf( "þ Playing demo %s\n", argv[i] );
                restore = 0;
            }
        }
        else if( !strcmpi( parm, "GAME" ) )
        {
            if( argv[++i] )
                strcpy( load_game, argv[i] );
            else
                helpme();
            restore = R_MOTIF;
        }
        else if( !strcmpi( parm, "V" ) || !strcmpi( parm, "VIDMODE" ) )
            cfgv = switchch;
        else if( *argv[i] == '/' )
            helpme();
        else
        {
            /* assume file and map come next */
            if( !load_fn[0] )
                strcpy( load_fn, argv[i] );
            else if( !load_tag[0] )
                strcpy( load_tag, argv[i] );
            else
                helpme();
            restore = R_MOTIF;  /* restore motif only */
        }
    }

    readconfig();

    /* need to configure? */
    if( cfgv || access( _ABBREV_"32.CFG", 0 ) == -1 )
    {
        if( !cfg_vidmode() )
        {
            qprintf( "You must select a video driver and mode before you can run "_ABBREV_".\n" );
            exit( EL_EXIT );
        }

        if( cfgv )
            saveconfig();
    }

    if( drecfn )
        dInitRecord( drecfn );
    else if( dplayfn )
        dInitPlayback( dplayfn );

    /* allocate resources for level data */
    resource_alloc();

    qprintf( "þ Allocated %lu bytes for resources\n", resourcemax );

    /* Load help file */
    qprintf( "þ Loading help file\n" );
    help_init( _ABBREV_".HLP" );

    if( restore )
    {
        qprintf( "þ Using restart information\n" );
        restore = status_restore( restore );
    }
    else
        qprintf( "þ Ignoring restart information\n" );

    qprintf( "þ Loading game data files\n" );

    done = findfirst( "*.GD", &ff, FA_ARCH | FA_RDONLY );
    while( !done )
    {
        if( !load_cfg( ff.ff_name ) )
            exit( EL_EXIT );
        done = findnext( &ff );
    }

    if( !iMaxGame )
        error( "There are no active gamedata (GD) files!\n"
               "Run "_ABBREV_" with the /findgames parameter to search for games." );

    /* some of them might need to be rewritten */
    fixgdfiles();
    renamegdfiles();

    /* Kill mouse at exit */
    atexit( mouse_kill );

    /* kill demo at exit */
    //atexit( dShutdown );

    qprintf( "þ Memory available: %lu bytes\n", coreleft() );

#ifndef __FLAT__
    /* Since we all hate this interrupt, let's get rid of it!  Thanks so much
     * to Bill Gates for putting it in.  Wow, what a guy.
     */
    _old_int_1b = getvect( 0x1b );
    setvect( 0x1b, _new_int_1b );
    atexit( restore_int1b );
#endif

    /* Initialize broadcast functions */
    bc_register( (BFUNC) _bf_consist );
    bc_register( (BFUNC) _bf_map );

    /* Jump to the editor */
    qprintf( "þ Launching graphics mode" );

#ifdef HEXEN
    strcpy( load_game, "HEXEN" );
#endif

    map_edit( load_fn, load_tag, load_game );

    /* Aaaand we're outta here */
    status_save();

    gr_vidmode( MODE_TEXT );

    if( iExitLev == EL_EXIT )
    {
        puttext( 1, 1, 80, DCKEND_LENGTH / 160, DCKEND );
        gotoxy( 1, DCKEND_LENGTH/160+1 );
    }

    if( !ismouse )  /* DCK jumps right to this if there's no mouse */
        printf( "þ "_ABBREV_" requires a mouse, and I can't find one!\n" );

    if( !dplayfn )
        saveconfig();

/*
    printf( "\nme_proc = %d\n", me_proc );
    printf( "me_total = %d\n", me_total );
    printf( "me_nothing_e = %u\n", me_nothing_e );
    printf( "me_nothing_d = %u\n", me_nothing_d );
*/

    exit( iExitLev );
}

enum
{
    coPolygon,
    coRectangle
};

void status_save( void )
/*-------------------------------------------------------------------------*
 * Saves the status of the editor for later retrieval.                     *
 *-------------------------------------------------------------------------*/
{
    FILE    *fp;
    short   idx;
    char    fn[128], status[128];
    extern  int iCreateObject;

    fnchop( fn, _argv[0], FNC_DRIVE | FNC_DIRECTORY );
    strcat( fn, "\\"_ABBREV_".STA" );

    fp = fopen( fn, "wb" );
    if( fp == NULL )    return;

    fprintf( fp, "; "_ABBREV_" Status File (don't modify me!)\r\n" );

    fprintf( fp, "%d\r\n", file_version );

    if( !fNewMap && pwdCurWad )
    {
        fprintf( fp, "%s\r\n", pgdCurGame->cfgname );
        fprintf( fp, "%s\r\n", pwdCurWad->fn );
        fprintf( fp, "%s\r\n", szCurMapTag );
    }
    else
        fprintf( fp, "(none)\r\n(none)\r\n(none)\r\n" );

    /* Write out misc map things */
    fprintf( fp, "%d\r\n%d\r\n%d\r\n", xstart, ystart, xscale );
    fprintf( fp, "%d\r\n", gridstep );

    /* Edit mode */
    if( editmode == MODE_LINE )
    {
        if( iCreateObject == coPolygon )
            editmode = MODE_POLYGON;
        else if( iCreateObject == coRectangle )
            editmode = MODE_RECTANGLE;
    }

    fprintf( fp, "%d\r\n", editmode );

    /* Write out status bytes */
    idx = 0;
    status[idx++] = tog_fine + '0';
    status[idx++] = tog_grid + '0';
    status[idx++] = tog_snap + '0';
    status[idx++] = tog_join + '0';
    status[idx++] = tog_csize + '0';
    status[idx++] = tog_tgraph + '0';
    status[idx] = 0;
    fprintf( fp, "%s\r\n", status );

    /* last motif number */
    fprintf( fp, "%d\r\n", motif_num );

    /* skill / nomonsters */
    fprintf( fp, "%d\r\n", iMapSkill );
    fprintf( fp, "%d\r\n", (int) fMapNoMonsters );
    fprintf( fp, "%d\r\n", iMapClass );

    /* thing display flags (all chars :) */
    status[sizeof( TFlags )] = 0;
    for( idx = 0; idx < sizeof( TFlags ); idx++ )
        status[idx] = ((char*)&td_flags)[idx] + '0';

    fprintf( fp, "%s\r\n", status );

    /* texture wads */
    if( pgdCurGame )
    {
        for( ntw = 1; pgdCurGame->texwads[ntw]; ntw++ );
        fprintf( fp, "%d\r\n", ntw-1 );

        for( idx = 1; pgdCurGame->texwads[idx]; idx++ )
            fprintf( fp, "%s\r\n", pgdCurGame->texwads[idx] );
    }
    else
        fprintf( fp, "0\r\n" );

    /* selection lenience */
    fprintf( fp, "%d\r\n", sel_lenience );

    fclose( fp );
}

uchar status_restore( uchar what )
{
    FILE    *fp;
    short   tmp, idx;
    char    fn[128], status[128];

    fnchop( fn, _argv[0], FNC_DRIVE | FNC_DIRECTORY );
    strcat( fn, "\\"_ABBREV_".STA" );

    fp = fopen( fn, "rb" );
    if( fp == NULL )    return 0;

    tmp = getcfgint( fp );

    /* must match version .. */
    if( tmp != file_version )
    {
        fclose( fp );
        remove( fn );
        return 0;
    }

    getcfgline( status, fp );
    if( what & R_MAP )      strcpy( load_game, status );
    getcfgline( status, fp );
    if( what & R_MAP )      strcpy( load_fn, status );
    getcfgline( status, fp );
    if( what & R_MAP )      strcpy( load_tag, status );

    if( *load_game == '(' ) *load_game = 0;       /* "(none)" */
    if( *load_fn == '(' )   *load_fn = 0;       /* "(none)" */
    if( *load_tag == '(' )  *load_tag = 0;

    /* Read in misc map things */
    tmp = getcfgint( fp );  if( tmp == -32000 ) goto _done;
    if( what & R_MAP ) xstart = tmp;
    tmp = getcfgint( fp );  if( tmp == -32000 ) goto _done;
    if( what & R_MAP ) ystart = tmp;
    tmp = getcfgint( fp );  if( tmp == -32000 ) goto _done;
    if( what & R_MAP ) xscale = tmp;

    /* grid step.. awooga */
    tmp = getcfgint( fp );
    if( what & R_MAP )
        gridstep = tmp;

    /* Edit mode */
    tmp = getcfgint( fp );
    if( what & R_MAP )
        editmode = tmp;

    /* Read in status bytes */
    getcfgline( status, fp );
    if( what & R_TOGGLES )
    {
        tog_fine = status[0] - '0';
        tog_grid = status[1] - '0';
        tog_snap = status[2] - '0';
        tog_join = status[3] - '0';
        tog_csize = status[4] - '0';
        tog_tgraph = status[5] - '0';
    }

    /* motif number */
    tmp = getcfgint( fp );
    if( what & R_MOTIF )
        motif_num = tmp;

    tmp = getcfgint( fp );
    if( what & R_MAP )
        iMapSkill = tmp;
    tmp = getcfgint( fp );
    if( what & R_MAP )
        fMapNoMonsters = tmp;
    tmp = getcfgint( fp );
    if( what & R_MAP )
        iMapClass = tmp;

    /* thing display flags (all chars :) */
    getcfgline( status, fp );
    for( idx = 0; idx < sizeof( TFlags ); idx++ )
        ((char*)&td_flags)[idx] = status[idx] - '0';

    /* get me some texture wads */
    tmp = getcfgint( fp );
    for( idx = 0; idx < tmp; idx++ )
    {
        getcfgline( status, fp );
        if( (what & R_MAP) )
            strcpy( loadtexwads[ntw++] = new uchar[128], status );
    }

    /* selection lenience */
    tmp = getcfgint( fp );
    if( what & R_MAP )  sel_lenience = tmp;

_done:
    fclose( fp );

    return what;
}
