/*-------------------------------------------------------------------------*
 * MAPMEM.C:                                                               *
 * Functions for loading, saving and editing a map.                        *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#pragma inline +memcpy +strcpy +memset

#include "gui\menu.hpp"
#include "gui\dialog.hpp"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

short   loadedvcount, its_loaded, tagnum[10];
VERTEX  *lvlist;
short   preview( short xpos, short ypos );
short   addblanktexture( char *name );

void map_freedata( short data )
/*-------------------------------------------------------------------------*
 * Frees map data.
 *-------------------------------------------------------------------------*/
{
    if( data == F_ALL )
    {
        /* frees everything */
        nvertex = 0;
        nline = 0;
        nside = 0;
        nsector = 0;
        nthing = 0;
        if( pBehavior ) memfree( pBehavior );
        pBehavior = NULL;
        if( lvlist )    memfree( lvlist );
        lvlist = NULL;
        loadedvcount = 0;
        memset( &MapOptions, 0, sizeof MapOptions );
        MapOptions.uSig = MO_SIG;
    }

    if( data == F_ALL || data == F_PRECALC )
    {
        /* frees precalc data */
        if( ssectors )
        {
            memfree( ssectors );
            ssectors = NULL;
            num_ssectors = 0;
        }
        if( psegs )
        {
            memfree( psegs );
            psegs = NULL;
            num_psegs = 0;
        }
        if( pnodes )
        {
            memfree( pnodes );
            pnodes = NULL;
            num_pnodes = 0;
        }

        if( reject )    memfree( reject );
        if( blocklists )memfree( blocklists );
        if( blockptrs ) memfree( blockptrs );
        blockmap_size = blockptrs_size = 0;
        blocklists = NULL;
        blockptrs = NULL;
        reject = NULL;
    }
}

void map_new( short newgame )
/*-------------------------------------------------------------------------*
 * Creates a new map.  Just kills all the objects.                         *
 *-------------------------------------------------------------------------*/
{
    void SetEditMode( short mode );

    /* close current wad */
    closecurwad( newgame );
    game_switch( newgame );

    strcpy( szCurMapTag, pgdCurGame->newtag );    /* new tag, dude. */
    map_freedata( F_ALL );              /* free all data */
    nMadeChanges = nMapChanges = 0;     /* set these */
    post_fullupdate();                  /* post a full update */
    memset( marklist, 0, sizeof marklist ); /* clear marks */

    SetEditMode( MODE_LINE );          /* set editmode to line */
    gridstep = 64;                      /* set gridstep */
    xscale = config.xscale;             /* and all this stuff */
    tog_grid = config.tog_grid;
    tog_join = config.tog_join;

    obj_clearmarks( -1, FALSE );

    pwdCurWad = wad_new( "UNTITLED.WAD" );
    fNewMap = 1;
    its_loaded = 1;

    broadcast( BC_CLEANFILE );
}

#define nDSetting 5

uchar *dsettings[nDSetting][5] =
{
    { "Run like hell",
      "Shine on you crazy diamond",
      "Learning to fly",
      "One of these days",
      "Dogs of war" },

    { "Crawl away",
      "Intolerance",
      "Sweat",
      "Cold and ugly",
      "Disgustipated" },

    { "Time to relax",
      "Gotta get away",
      "Come out and play",
      "Genocide",
      "Killboy powerhead" },

    { "Finding my way",
      "Distant early warning",
      "Red sector A",
      "Manhattan project",
      "Force ten" },

    { "So gently we go",
      "Levitate",
      "Not quite sonic",
      "Basketball",
      "The universe in you" }
};

uchar *mt_dialog( void )
/*-------------------------------------------------------------------------*
 * Returns a string to concatenate on the run command, or NULL if aborted.
 *-------------------------------------------------------------------------*/
{
    extern  int     iMapSkill;
    extern  int     iMapClass;
    extern  uchar   fMapNoMonsters;

    static  uchar   parmbuf[128];
    static  uchar   first = 1;
    static  uchar   **ds_array;
    static  short   last_ds = -1;

    Field   *pfSkill1, *pfSkill2, *pfSkill3, *pfSkill4, *pfSkill5;

    byte    fStopAsking = 0;

    last_ds = rand() % nDSetting;

    ds_array = dsettings[last_ds];

    if( !config.tog_runparm )
        return " ";

    VarTransfer_t vtMapRun[] =
    {
        { "Skill1", (Field**) &pfSkill1, (PTR) &iMapSkill, NULL },
        { "Skill2", (Field**) &pfSkill2, NULL, NULL },
        { "Skill3", (Field**) &pfSkill3, NULL, NULL },
        { "Skill4", (Field**) &pfSkill4, NULL, NULL },
        { "Skill5", (Field**) &pfSkill5, NULL, NULL },

        { "Fighter", NULL, (PTR) &iMapClass, NULL },
        { "Nomonsters", NULL, (PTR) &fMapNoMonsters, NULL },
        { "Dontask", NULL, (PTR) &fStopAsking, NULL },

        // end marker
        { NULL, NULL, NULL, NULL }
    };

    Dialog *pDialog = new Dialog;
    pDialog->Create( szDSFile, fHexen ? "HexenMapRunDlg" : "DoomMapRunDlg" );
    pDialog->VarSetup( vtMapRun );

    ((stText*)pfSkill1->GetPrompt())->SetText( ds_array[0] );
    ((stText*)pfSkill2->GetPrompt())->SetText( ds_array[1] );
    ((stText*)pfSkill3->GetPrompt())->SetText( ds_array[2] );
    ((stText*)pfSkill4->GetPrompt())->SetText( ds_array[3] );
    ((stText*)pfSkill5->GetPrompt())->SetText( ds_array[4] );

    int iRvl = pDialog->Process();

    delete pDialog;

    if( fStopAsking )
    {
        config.tog_runparm = 0;
        saveconfig();
    }

    if( iRvl == daExitSave )
    {
        if( fHexen )
        {
            sprintf( parmbuf, "-skill %d -class %d %s", iMapSkill + 1,
                iMapClass, fMapNoMonsters ? "-nomonsters" : "" );
        }
        else
        {
            sprintf( parmbuf, "-skill %d %s", iMapSkill + 1,
                fMapNoMonsters ? "-nomonsters" : "" );
        }
        first = 0;

        return parmbuf;
    }

    return NULL;
}

void map_test( void )
{
    char    tmpbuf[128], tmpbuf2[128], curdir[128], szRespFile[128];
    char    runbuf[128], tmp = 0, *pszOurParams, *ptr;
    extern  uchar   fDoNodes;
    short   i, curdisk = getdisk(),
            ok = (!nMapChanges && !nMadeChanges && isnodeexist());
    FILE    *fp, *fpResponse;

    pszOurParams = mt_dialog();

    if( !pszOurParams )
        return;

    if( !ok )
    {
        fDoNodes = 1;     /* need ze nodez */

        if( !file_save( FALSE, TRUE ) )
        {
            return;
        }
    }

    // we are now in text-mode..

    fDoNodes = 0;     /* zet zis awf! */

    getcwd( curdir, 128 );          /* so we can change back later */

    fp = fopen( "_PLAY.BAT", "wt" );
    /* line 1: echo off */
    fprintf( fp, "@echo off\ncls\n" );

    strccpy( tmpbuf, pgdCurGame->run, " " );
    strcpy( runbuf, tmpbuf );

    /* line 2: change to the drive */
    fnchop( tmpbuf2, tmpbuf, FNC_DRIVE );
    fprintf( fp, "%s\n", tmpbuf2 );

    /* line 3: change to the dir */
    fnchop( tmpbuf2, tmpbuf, FNC_DIRECTORY );
    fprintf( fp, "cd %s\n", tmpbuf2 );

    // create response file
    fnchop( szRespFile, pgdCurGame->run, FNC_DRIVE | FNC_DIRECTORY );
    strcat( szRespFile, "\\dckrun.rsp" );
    fpResponse = fopen( szRespFile, "wb" );

    ptr = strchr( (char*) pgdCurGame->run, '-' );
    setfilertparms();
    i = 0;

_MoreParms:
    while( ptr )
    {
        strcpy( tmpbuf, "-" );
        strccpy( tmpbuf+1, ptr+1, "-\r\n" );
        rt_replace( tmpbuf );
        fprintf( fpResponse, "%s\r\n", tmpbuf );
        ptr = strchr( ptr+1, '-' );
    }

    if( !i )
    {
        i = 1;
        ptr = pszOurParams;
        goto _MoreParms;
    }

    fprintf( fpResponse, "-file " );
    for( i = 1; pgdCurGame->texwads[i]; i++ )
    {
        if( !samefile( pgdCurGame->texwads[i], pwdCurWad->fn ) )
            fprintf( fpResponse, "%s ", pgdCurGame->texwads[i] );
    }
    // add our wad at the end
    fprintf( fpResponse, "%s\r\n", pwdCurWad->fn );
    fclose( fpResponse );

    strcat( runbuf, " @" );
    strcat( runbuf, szRespFile );
    strlwr( runbuf );

    /* line 4: echo cmd-line */
    fprintf( fp, "echo [Command line: %s]\n", runbuf );

    /* line 5: run it */
    fprintf( fp, "echo. | %s\n", runbuf );

    /* line 6: back to this drive/dir */
    fprintf( fp, "%c:\n", ((char) curdisk + 'A') );
    fprintf( fp, "cd %s\n", curdir );

    fclose( fp );

    /* get_out is checked in input.c and exits the main input loop if it's
     * set. */
    get_out = 1;
    iExitLev = EL_PLAY;      /* wheeeeeeeeeeeeeeeee */
}

int EditMapOptions()
{
    // edits the map options structure
    Dialog *pDialog = new Dialog;
    pDialog->Create( szDSFile, "MapOptionsDlg" );

    VarTransfer_t vtMapOptions[] =
    {
        { "Script", NULL, (PTR) &MapOptions.szScriptName, 0 },

        // end-marker
        { NULL, NULL, NULL, NULL }
    };

    pDialog->VarSetup( vtMapOptions );
    if( pDialog->Process() == daExitSave )
        nMapChanges++;

    delete pDialog;

    return maExit;
}

static void loaderr( char *loadwhat )
{
    error( "loading %s from %s - can't find %s", szCurMapTag, pwdCurWad->fn, loadwhat );
}

char szGameVersion[] = "version 2.3";

void ReadMapOptions( FILE* fp )
{
    char szBuf[128];

    while( 1 )
    {
        fgets( szBuf, 128, fp );

        // end keyword?
        if( !strcmp( szBuf, "[End]" ) )
            break;

        if( !strcmp( szBuf, "[General]" ) )
            break;
    }
}

void map_load( void )
/*-------------------------------------------------------------------------*
 * Loads the map tag in "szCurMapTag" from curwad.
 *-------------------------------------------------------------------------*/
{
    ulong   size;
    FILE*   handle;
    LINEDEF *ldptr;
    short   n, m, nsdd10, lv_val = 1;
    SIDEDEF sd;
    SIDEDEFM *sdmp;
    THING   *tptr;
    WADENTRY *weptr;
    short   *vused;
    short   i, nblank;
    long    *lvp, *vlp;
    GWINDOW *loadwnp;
    char    buf[128];
    int     iLoadCount = 1;
    int     nLoadTotal = 6;

    its_loaded = 0;

/*-------------------------------------------------------------------------*
 * Jump to the map resources                                               *
 *-------------------------------------------------------------------------*/
    if( !wad_find( pwdCurWad, szCurMapTag ) )
        return;

    loadwnp = MAKESTATUSWINDOW( "Loading Map", 2 );
    wn_disp( loadwnp );
    sprintf( buf, " %s in %s", szCurMapTag, pwdCurWad->fn );
    wn_xytext( loadwnp, 1, 5, buf );

    map_freedata( F_ALL );  /* free all data */

    // load up our data section
#if 0
    handle = wad_e2h( pwdCurWad, (WADENTRY*) (pwdCurWad->curdir->data) );
    fseek( handle, strlen( szGameVersion ) + 1, SEEK_CUR );    // go past that

    size = dirgimmesize( pwdCurWad );
    if( size > strlen( szGameVersion ) + 1 )
        ReadMapOptions( handle );

    fclose( handle );
#endif

/*-------------------------------------------------------------------------*
 * Read in lines                                                           *
 *-------------------------------------------------------------------------*/
    wn_statstr( loadwnp, "Loading linedefs" );
    wad_findCurMap( pwdCurWad, "LINEDEFS" );
    wn_stat( loadwnp, iLoadCount++, nLoadTotal );

    if( fHexen )
    {
        dirload( pwdCurWad, &size, (uchar *) linelist );
        nline = (short) (size / sizeof( LINEDEF ));
    }
    else
    {
        byte *pLineData = (byte*) dirload( pwdCurWad, &size, NULL );
        nline = (short) (size / sizeof( DoomLine ));

        DoomLine *dlp = (DoomLine*) pLineData;
        ldptr = linelist;

        // convert doom lines to hexen lines
        for( i = 0; i < nline; i++ )
        {
            memcpy( ldptr, dlp, 8 );

            ldptr->sectag = dlp->sectag;
            ldptr->rsdef = dlp->rsdef;
            ldptr->lsdef = dlp->lsdef;

            ldptr++;
            dlp++;
        }

        memfree( pLineData );
    }

    if( nline > maxline )
        error( "maximum linedef count exceeded in loading %s", szCurMapTag );

/*-------------------------------------------------------------------------*
 * Read in vertices                                                        *
 *-------------------------------------------------------------------------*/
    wn_statstr( loadwnp, "Loading vertices" );
    wad_findCurMap( pwdCurWad, "VERTEXES" );
    lvlist = (VERTEX*) dirload( pwdCurWad, &size, NULL );
    loadedvcount = nvertex = (short) (size / sizeof( VERTEX ));
    wn_stat( loadwnp, iLoadCount++, nLoadTotal );

    if( nvertex > maxvertex )
        error( "maximum vertex count exceeded in loading %s", szCurMapTag );

    _memstr = "vertex use list";
    vused = new short[nvertex];
    memset( vused, 0, sizeof( short ) * nvertex );

    for( ldptr = linelist, n = 0; n < nline; n++, ldptr++ )
    {
        vused[ldptr->vfrom] = 1;
        vused[ldptr->vend] = 1;
    }

    for( n = nvertex = 0, lvp = (long*)lvlist, vlp = (long*)vlist;
         n < loadedvcount; n++, lvp++ )
    {
        if( vused[n] )
        {
            *vlp++ = *lvp;
            ++nvertex;
        }
    }

    /* update the vertex numbers in the linedefs */
    for( i = 0, m = 0; i < loadedvcount; i++ )
        if( vused[i] )
            vused[i] = m++;
    for( i = 0; i < nline; i++ )
    {
        if( linelist[i].vfrom != vused[linelist[i].vfrom] )
        {
            lv_val = 0;
            linelist[i].vfrom = vused[linelist[i].vfrom];
        }
        if( linelist[i].vend != vused[linelist[i].vend] )
        {
            lv_val = 0;
            linelist[i].vend = vused[linelist[i].vend];
        }
    }

    if( !lv_val )
    {
        memfree( lvlist );
        lvlist = NULL;
    }

    delete vused;

/*-------------------------------------------------------------------------*
 * Read things                                                             *
 *-------------------------------------------------------------------------*/
    wn_statstr( loadwnp, "Loading things" );
    wn_stat( loadwnp, iLoadCount++, nLoadTotal );
    wad_findCurMap( pwdCurWad, "THINGS" );

    if( fHexen )
    {
        dirload( pwdCurWad, &size, (uchar *) thinglist );
        nthing = (short) (size / sizeof( THING ));
    }
    else
    {
        byte *pThingData = (byte*) dirload( pwdCurWad, &size, NULL );

        if( !pThingData )
        {
            nthing = 0;
        }
        else
        {
            nthing = (short) (size / sizeof( DoomThing ));

            DoomThing *dtp = (DoomThing*) pThingData;
            tptr = thinglist;

            // convert doom things to hexen things
            for( i = 0; i < nthing; i++ )
            {
                tptr->x = dtp->x;
                tptr->y = dtp->y;
                tptr->attribs = dtp->attribs;
                tptr->type = dtp->type;
                tptr->angle = dtp->angle;

                tptr++;
                dtp++;
            }

            memfree( pThingData );
        }
    }

    if( nthing > maxthing )
        error( "maximum thing count exceeded in loading %s", szCurMapTag );

/*-------------------------------------------------------------------------*
 * Read sectors                                                            *
 *-------------------------------------------------------------------------*/
    wn_statstr( loadwnp, "Loading sectors" );
    wad_findCurMap( pwdCurWad, "SECTORS" );
    wn_stat( loadwnp, iLoadCount++, nLoadTotal );

    dirload( pwdCurWad, &size, (uchar *) sectorlist );
    nsector = (short) (size / sizeof( SECTOR ));

    if( nsector > maxsector )
        error( "maximum sector count exceeded in loading %s", szCurMapTag );

/*-------------------------------------------------------------------------*
 * Read sidedefs                                                           *
 *-------------------------------------------------------------------------*/
    wn_statstr( loadwnp, "Loading sidedefs" );
    wn_stat( loadwnp, iLoadCount++, nLoadTotal );
    weptr = wad_findCurMap( pwdCurWad, "SIDEDEFS" );
    size = weptr->length;
    handle = wad_e2h( pwdCurWad, weptr );
    nside = (short) (size / sizeof( SIDEDEF ));

    setvbuf( handle, NULL, _IOFBF, 4096 );

    if( nside > maxside )
        error( "maximum sidedef count exceeded in loading %s", szCurMapTag );

    nsdd10 = nside / 10;

    if( !nsdd10 )
        nsdd10 = 1;

    nblank = 0;

    /* Load sides in a special kind of way.  Woo woo! */
    for( i = 0, sdmp = sidelist; i < nside; i++, sdmp++ )
    {
        fread( &sd, sizeof( SIDEDEF ), 1, handle );

        /* Translate it into memory */
        if( (sdmp->utxidx = texture_toint( sd.utxname )) == -1 )
            if( (sdmp->utxidx = texture_toint_ic( sd.utxname )) == -1 )
                sdmp->utxidx = addblanktexture( sd.utxname ), nblank++;

        if( (sdmp->ltxidx = texture_toint( sd.ltxname )) == -1 )
            if( (sdmp->ltxidx = texture_toint_ic( sd.ltxname )) == -1 )
                sdmp->ltxidx = addblanktexture( sd.ltxname ), nblank++;

        if( (sdmp->ftxidx = texture_toint( sd.ftxname )) == -1 )
            if( (sdmp->ftxidx = texture_toint_ic( sd.ftxname )) == -1 )
                sdmp->ftxidx = addblanktexture( sd.ftxname ), nblank++;

        sdmp->sector = sd.sector;
        sdmp->wxofs = sd.wxofs;
        sdmp->wyofs = sd.wyofs;
    }

    fclose( handle );

    if( nblank )
    {
        message_box( MB_OK, st_dcksays,
         "%d textures were in the map that are not in the list of\n"
         "valid textures - they've been added to the list, but you\n"
         "won't be able to view them unless you tell "_ABBREV_" where to\n"
         "find their descriptors.", nblank );
    }

/*-------------------------------------------------------------------------*
 * Load boring stuff
 *-------------------------------------------------------------------------*/
    if( !lv_val )
    {
        /* doomed sucks */
        message_box( MB_OK, st_dcksays,
         "This map has been loaded without its precalculations because it was\n"
         "saved by an editor that is partly incompatible with "_ABBREV_".  This\n"
         "is not a serious problem - it just means the BSP (see the manual) will\n"
         "have to be rebuilt when you save the map." );
    }
    else
    {
        wn_statstr( loadwnp, "Loading Precalculations" );
        wn_stat( loadwnp, iLoadCount++, nLoadTotal );

        if( wad_findCurMap( pwdCurWad, "BEHAVIOR" ) )
        {
            pBehavior = (char*) dirload( pwdCurWad, &size, NULL );
            lBehaviorSize = size;
        }
        else
        {
            lBehaviorSize = 0;
            pBehavior = NULL;
        }

        wad_findCurMap( pwdCurWad, "NODES" );
        pnodes = (Pnode*) dirload( pwdCurWad, &size, NULL );
        num_pnodes = (size / sizeof( Pnode ));

        wad_findCurMap( pwdCurWad, "SEGS" );
        psegs = (Pseg*) dirload( pwdCurWad, &size, NULL );
        num_psegs = (size / sizeof( Pseg ));

        wad_findCurMap( pwdCurWad, "SSECTORS" );
        ssectors = (SSector*) dirload( pwdCurWad, &size, NULL );
        num_ssectors = (size / sizeof( SSector ));

        weptr = wad_findCurMap( pwdCurWad, "BLOCKMAP" );

        /* load the blockmap */
        if( weptr->length > 0 )
        {
            handle = wad_e2h( pwdCurWad, weptr );
            fread( &blockhead, sizeof( Block ), 1, handle );
            blockptrs_size = (blockhead.xblocks*blockhead.yblocks)*2;
            blockptrs = (short*) memget( blockptrs_size );
            fread( blockptrs, blockptrs_size, 1, handle );
            blockmap_size = weptr->length - blockptrs_size - sizeof( Block );
            blocklists = (short*) memget( blockmap_size );
            fread( blocklists, blockmap_size, 1, handle );
            fclose( handle );
        }
        else
        {
            blocklists = NULL;
            blockmap_size = 0;
            blockptrs_size = 0;
            blockptrs = NULL;
            memset( &blockhead, 0, sizeof blockhead );
        }

        wad_findCurMap( pwdCurWad, "REJECT" );
        reject = (char*) dirload( pwdCurWad, &size, NULL );
        reject_size = size;
    }

/*-------------------------------------------------------------------------*
 * Close and cleanup                                                       *
 *-------------------------------------------------------------------------*/
    memset( marklist, 0, sizeof marklist );

    wn_undisp( loadwnp );
    wn_free( loadwnp );

    broadcast( BC_NEWFILE );
    nMadeChanges = nMapChanges = 0;
    its_loaded = 1;
    fNewMap = 0;
}

short ismaptag_menu( uchar *name, WAD* wptr )
/*-------------------------------------------------------------------------*
 * If only one game matches 'name', returns that.  otherwise makes a menu
 * of all matching games and returns the user's choice of the menu.
 *-------------------------------------------------------------------------*/
{
    short       rvl;
    short       iarray[10];
    const char  *array[10];
    short       idx = 0;
    short       i, digcount;
    GAMEDATA    *pgdGame = pgdGameArray;
    uchar       *s, *np;
    bool        fWaitForHexen = FALSE;  // there goes modularity

    if( wad_find( wptr, "BEHAVIOR" ) )
        fWaitForHexen = TRUE;

    for( i = 0, pgdGame = pgdGameArray; pgdGame != NULL; i++, pgdGame++ )
    {
        np = name;
        s = pgdGame->tag;

        if( !s[0] )    /* empty game data */
            break;

        do {

            if( *s == '\'' )
            {
                digcount = (* ++s) - '0';    /* decplace */
                ++s;                         /* advance past it */
                while( digcount-- )
                {
                    if( !isdigit( *np++ ) )    /* lookin' for a digit */
                        goto next_gd;
                }
            }
            else
            {
                if( *np++ != *s++ )     /* check for a match on normal char */
                    goto next_gd;
            }

        } while( *np || *s );

        if( !strcmpi( pgdGame->cfgname, "HEXEN" ) )
        {
            if( fWaitForHexen )
                return i;
            else            // not a hexen wad
                goto next_gd;
        }

        /* got here .. it's a match. */
        iarray[idx] = i;
        array[idx++] = (const char*) pgdGame->name;

next_gd:
    }

    if( fWaitForHexen )
    {
        // if we got here, they don't have hexen
        message_box( MB_CANCEL, st_dcksays,
            "This map is for Hexen, and you don't have Hexen installed." );
        return -1;
    }

    array[idx] = NULL;

    if( !idx )
        return -1;
    if( idx == 1 )  /* only one matching game */
        return iarray[0];

    rvl = MenuPopArray( 0, " Load for which game? ", NULL, array );

    if( rvl == -1 )
        return -1;

    return iarray[rvl];
}

short ismaptag( uchar *name, int iMatchGame )
/*-------------------------------------------------------------------------*
 * Checks to see if 'name' follows any of the pgdCurGame array map tag
 * formats
 *-------------------------------------------------------------------------*/
{
    short       i, digcount, dignum = 0;
    GAMEDATA    *pgdGame = pgdGameArray;
    uchar       *s, *np, digbuf[10], *dbptr;

    if( iMatchGame != -1 )
        i = iMatchGame;
    else
        i = 0;

    for( ; pgdGame != NULL; i++, pgdGame++ )
    {
        np = name;
        s = pgdGame->tag;

        if( ! (*s) )    /* empty game data */
            break;

        do {

            if( *s == '\'' )
            {
                digcount = (* ++s) - '0';       /* decplace */
                ++s;        /* advance past it */
                dbptr = digbuf;
                while( digcount-- )
                {
                    if( !isdigit( *np ) )   /* lookin' for a digit */
                        goto next_gd;
                    *dbptr++ = *np++;
                }
                *dbptr = 0;
                tagnum[dignum++] = atoi( digbuf );
            }
            else
            {
                if( *np++ != *s++ )     /* check for a match on normal char */
                    goto next_gd;
            }

        } while( *np || *s );

        /* got here .. it's a match. */
        if( iMatchGame != -1 )
            return TRUE;

        return i;

next_gd:
        if( iMatchGame != -1 )
            return FALSE;
    }

    return -1;  /* no match */
}

short findgamename( uchar *name )
{
    short   i;

    for( i = 0; i < iMaxGame; i++ )
    {
        if( !strcmpi( pgdGameArray[i].cfgname, name ) )
            return i;
    }

    return -1;
}

void map_edit( uchar *load_fn, uchar *load_tag, uchar *load_game )
/*-------------------------------------------------------------------------*
 * Edits the currently selected map by loading it, going into graphics     *
 * mode if not already there, and calling the input loop.                  *
 *-------------------------------------------------------------------------*/
{
    GWINDOW *gwnp;
    short   tmpint, gamenum = -1;

    gr_vidmode( MODE_GFX );       /* graphics mode! */
    _PopHelp.Init( "pophelp.hlp" );

    setfillstyle( CLOSE_DOT_FILL, C_GRAY4 );
    bar( 0, 0, sxlen, sylen );

    //setcolor( C_WHITE );
    //gr_fillcircle( sxlen/2, sylen/2, 20 );
    //getch();
    //return;

// /* TEST
    /* put a piccy while we're waiting for the mouse */
    gwnp = wn_def( Centered, Centered, 27, 3, C_GRAY3, C_GRAY1,
                    WF_POPUP | WF_3D, st_dcksays );
    wn_disp( gwnp );
    wn_xytext( gwnp, Centered, 1, "Starting Up" );
    wn_free( gwnp );    /* but leave it up */

    mouse_init();
    mouse_show();

    if( !ismouse )      /* no mouse */
        return;

    strupr( load_fn );
    strupr( load_tag );

    tmpint = -1;

    if( *load_game )
        gamenum = findgamename( load_game );

    if( *load_fn )
    {
        /* got a game number; switch to the game and load the .wad */
        pwdCurWad = wad_open( load_fn, WL_COMPLETE );

        /* get game of load tag */
        if( pwdCurWad && *load_tag && tmpint == -1 )
        {
            if( wad_find( pwdCurWad, load_tag ) == NULL )
            {
                wad_close( pwdCurWad );
                pwdCurWad = NULL;
                goto _cantload;
            }

            if( gamenum == -1 )
            {
                // get the game to load
                tmpint = ismaptag_menu( load_tag, pwdCurWad );

                if( tmpint == -1 )  /* aborted.. */
                    return;
                else
                   gamenum = tmpint;   // else assign the game
            }
        }

        if( pwdCurWad )
        {
            if( *load_tag && gamenum != -1 )
            {
                game_switch( gamenum );
                strcpy( szCurMapTag, load_tag );
                map_load();

                if( !its_loaded )
                {
                    wad_close( pwdCurWad );
                    pwdCurWad = NULL;
                    goto _cantload;
                }
            }
            else if( tmpint != -1 )
                map_new( tmpint );

            if( pwdCurWad )
                addtexwad( pwdCurWad );
        }
        else
        {
_cantload:
            if( !load_tag[0] )
            {
                message_box( MB_OK, st_dcksays,
                    "I couldn't find the file:    \n%s",
                    load_fn );
            }
            else
            {
                message_box( MB_OK, st_dcksays,
                    "I couldn't load %s from the file:\n%s",
                    load_tag, load_fn );
            }
        }
    }
    else
    {
        if( gamenum < 0 )
            gamenum = 0;

        map_new( gamenum );
    }

    while( 1 )
    {
        /* le_input() returns 1 to continue editing (ie: recall me), or
         * 0 to exit the editor. */
        if( !le_input() )
            break;
    }
}

short purgelist( uchar *base, size_t size, short nelem, short *uselist )
/*-------------------------------------------------------------------------*
 * Purges an array.  uselist[] is an array of integers.  All to-be-purged  *
 * elements in 'base' should have their corresponding elements in uselist  *
 * set to zero.                                                            *
 *-------------------------------------------------------------------------*/
{
    short     i;
    uchar   *ptr = NULL, *ptr2;
    short     saved = 0;

    for( i = 0; i < nelem; i++ )
    {
        if( !uselist[i] && !ptr )
            ptr = base + (i * size);
        else if( uselist[i] && ptr )
        {
            ptr2 = base + (i * size);
            for( ; uselist[i] && i < nelem; i++ )
            {
                memcpy( ptr, ptr2, size );
                ptr += size;
                ptr2 += size;
                ++saved;
            }
        }
        else if( uselist[i] )
            ++saved;
    }

    return saved;
}

void map_purge( short purgewhat )
/*-------------------------------------------------------------------------*
 * Way this works is simple: Goes through all the                          *
 * structures that depend on other structures (vertices->linedefs,         *
 * sidedefs->linedefs, etc) and sets up boolean lists of which are used    *
 * and which aren't.  Then, goes through and reorganizes the lists to      *
 * delete unused parts (ie: vertices and sidedefs that aren't used are     *
 * removed from the list.)  Of course, the lists must be renumbered, etc,  *
 * but who cares.                                                          *
 *                                                                         *
 * This function makes the map "changed."                                  *
 *-------------------------------------------------------------------------*/
{
    short     *uselist;
    short     i, i2, tmp;
    LINEDEF *ldp;
    SIDEDEFM *sdp;
    void    line_delvsame( void );

    /* delete lines with same vertex references */
    line_delvsame();

/*-------------------------------------------------------------------------*
 * purge vertices                                                          *
 *-------------------------------------------------------------------------*/
    if( purgewhat & MODE_VERTEX )
    {
        uselist = new short[maxvertex];
        memset( uselist, 0, maxvertex * sizeof( short ) );

        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            uselist[ldp->vfrom] = 1;
            uselist[ldp->vend] = 1;
        }

        /* Reorganize them... */
        for( i = 0, i2 = 1; i < nvertex; i++ )
        {
            if( uselist[i] )
                uselist[i] = i2++;
        }

        nvertex = purgelist( (uchar *) vlist, sizeof( VERTEX ), nvertex, uselist );

        /* Now reorganize the lines */
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            ldp->vfrom = uselist[ldp->vfrom]-1;
            ldp->vend = uselist[ldp->vend]-1;
        }

        delete uselist;
    }
/*-------------------------------------------------------------------------*
 * purge sidedefs                                                          *
 *-------------------------------------------------------------------------*/
    if( purgewhat & MODE_SIDEDEF )
    {
        uselist = new short[maxside];
        memset( uselist, 0, maxside * sizeof( short ) );

        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( ldp->rsdef != -1 )
                uselist[ldp->rsdef] = 1;
            if( ldp->lsdef != -1 )
                uselist[ldp->lsdef] = 1;
        }

        /* Reorganize them... */
        for( i = 0, i2 = 1; i < nside; i++ )
        {
            if( uselist[i] )
                uselist[i] = i2++;
        }

        nside = purgelist( (uchar *) sidelist, sizeof( SIDEDEFM ), nside, uselist );

        /* Now reorganize them */
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( ldp->rsdef != -1 )
                ldp->rsdef = uselist[ldp->rsdef]-1;
            if( ldp->lsdef != -1 )
                ldp->lsdef = uselist[ldp->lsdef]-1;
        }

        delete uselist;
    }

/*-------------------------------------------------------------------------*
 * purge sectors                                                           *
 *-------------------------------------------------------------------------*/
    if( purgewhat & MODE_SECTOR )
    {
        uselist = new short[maxsector];
        memset( uselist, 0, maxsector * sizeof( short ) );

        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( ldp->rsdef != -1 )
            {
                sdp = &sidelist[ldp->rsdef];

                if( sdp->sector == 160 )
                    sdp->sector = 160;

                if( sdp->sector != -1 )
                    uselist[sdp->sector] = 1;
            }
            if( ldp->lsdef != -1 )
            {
                sdp = &sidelist[ldp->lsdef];

                if( sdp->sector == 160 )
                    sdp->sector = 160;

                if( sdp->sector != -1 )
                    uselist[sdp->sector] = 1;
            }
        }

        /* Reorganize them... */
        for( i = 0, i2 = 1; i < nsector; i++ )
        {
            if( uselist[i] )
                uselist[i] = i2++;
        }

        nsector = purgelist( (uchar *) sectorlist, sizeof( SECTOR ), nsector, uselist );

        /* Now reorganize them */
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( ldp->rsdef != -1 )
            {
                sdp = &sidelist[ldp->rsdef];
                if( (tmp = sdp->sector) != -1 )
                    sdp->sector = uselist[tmp]-1;
            }
            if( ldp->lsdef != -1 )
            {
                sdp = &sidelist[ldp->lsdef];
                if( (tmp = sdp->sector) != -1 )
                    sdp->sector = uselist[tmp]-1;
            }
        }

        delete uselist;
    }

    nMapChanges++;
    nMadeChanges++;
}
