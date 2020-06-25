/*-------------------------------------------------------------------------*
 * LOADSAVE.C:
 * DIalog boxes for loading and saving .WADs.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"

/*-------------------------------------------------------------------------*
 *
 * Code starts here
 *
 *-------------------------------------------------------------------------*/

static fldListBox   *pfDirList, *pfFileList, *pfMapList;
static fldString    *pfFilename, *pfTag;
static fldRegion    *pfPreview;
static fldCheckBox  *pfDoNodes;
static fldButton    *pfBtnNew;
static Dialog       *pDialog;

static StrList  *stlDirs, *stlFiles, *stlMaps;
static bool     fNoMaps, fNoFiles, fFirstUpd, fMapValid;
static short    newmap, fldno_maplist, fldno_filelist;
static short    pscale = 500, fg_mode, pxofs = 0, pyofs = 0;
static uchar    curdir[128], thetag[9], fszFilename[128], fszTag[10], fg_ext[10],
                fnjump[128];    /* when this has something in it, _upd_files
                                 * goes to filename */
// static short   _upd_files( FIELD *notused );
static int _upd_files( Field* );
static WAD     *ls_wptr;

bool   fDoNodes = 0; /* don't redo nodes by default */

void fg_freewad( void );

short preview( WAD *wptr, uchar *_ltag, short xpos, short ypos,
               short width, short height, short scale, short xofs, short yofs );
void FreePreviewData();

static int _upd_preview( Field* )
/*-------------------------------------------------------------------------*
 * Displays a preview of the currently highlighted map in the currently
 * highlighted wad in a little window south of the border
 *-------------------------------------------------------------------------*/
{
    Rect rcRect;

    if( !pfPreview )
        return daContinue;

    /* clear the old one */
    rcRect = pfPreview->GetRectInParent();
    bool fHidden = mouse_hide( rcRect );
    pfPreview->OnDraw();

    if( !fNoMaps )
    {
        // map: draw it
        fMapValid = preview( ls_wptr, (uchar*) pfMapList->GetText(), rcRect.left+1,
                 rcRect.top+1, rcRect.Width()-2, rcRect.Height()-2,
                 pscale, pxofs, pyofs );
        if( pfTag &&
            ismaptag( (char*) pfMapList->GetText(), iCurGame ) )  /* update tag field? */
        {
            pfTag->SetText( pfMapList->GetText() );
        }
    }

    if( fNoMaps || !fMapValid )
    {
        /* no maps .. display that. */
        gr_setcolor( C_WHITE );
        outtextxy( rcRect.left+1, rcRect.top+1, "No preview available" );
    }

    mouse_show( fHidden );

    return daContinue;
}

static int _upd_maps( Field* pField )
/*-------------------------------------------------------------------------*
 * Updates the list of maps from the current file.
 * if notused is not null, this is being called from the dialog box.
 * otherwise, it's being passed-down from one of the other functions.
 *-------------------------------------------------------------------------*/
{
    static  char thefile[128];
    short   pos;
    NODEPTR ndp;
    WADENTRY *weptr;

    pxofs = pyofs = 0;
    pscale = 500;

    if( !(fg_mode & FG_SAVE) )
    {
        sprintf( thefile, "%s%s", curdir, pfFileList->GetText() );
        strlwr( thefile );
        pfFilename->SetText( thefile );
    }
    else
    {
        /* we have to account for the situation where the
         * user enters a filename that does not exist.
         * however, if pField is not null, it means the user is
         * scrolling the list and so we should update the filename
         * field. */
        const char* pszFilename = pfFilename->GetText();
        if( pField != NULL ||
            (!strchr( pszFilename, '.' ) && strchr( pszFilename, '\\' )) )
        {
            /* no filename in the field, so use the file from the
             * file list */
            sprintf( thefile, "%s%s", curdir, pfFileList->GetText() );
            strlwr( thefile );
            pfFilename->SetText( thefile );
        }
        else    /* filename in the field, eh */
            strcpy( thefile, pszFilename );
    }

    if( fg_mode & FG_NOMAPS )
        return daContinue;

    /* clear previous map list */
    stlMaps->DeleteAll();

    /* open the file */
    fg_freewad();
    ls_wptr = wad_open( thefile, WL_MAPONLY );

    if( ls_wptr != NULL )
    {
        fNoMaps = 1;

        ndp = NodeFirst( ls_wptr->dir_list );

        while( ndp != ls_wptr->dir_list )
        {
            weptr = (WADENTRY*) ndp->data;
            if( ismaptag( weptr->name ) != -1 )
            {
                stlMaps->AddTail( (char*) WN2Str( weptr->name ) );
                fNoMaps = 0; /* no no maps */
            }
            ndp = NodeNext( ndp );
        }
    }

    if( ls_wptr == NULL || fNoMaps )
    {
        stlMaps->AddTail( "No maps." );
        fNoMaps = 1;
    }

    stlMaps->Sort();
    pfMapList->ReInit();

    if( *fszTag )
    {
        pos = stlMaps->Find( fszTag );
        if( pos != -1 )
            pfMapList->SetCurItem( pos );
        else
            pfMapList->SetCurItem( 0 );
    }
    else
        pfMapList->SetCurItem( 0 );

    _upd_preview( NULL );

    return daContinue;
}

static int _selthis( Field* )
/*-------------------------------------------------------------------------*
 * When the user makes their choice of a map
 *-------------------------------------------------------------------------*/
{
    if( !fNoMaps || (fg_mode & FG_NOMAPS) )
        return daExitSave;

    return daContinue;
}

static int ffNewMap( Field* )
/*-------------------------------------------------------------------------*
 * New!
 *-------------------------------------------------------------------------*/
{
    short   rvl;

    if( (rvl = game_choose()) != -1 )
    {
        newmap = rvl;
        return daExitSave;
    }

    return daContinue;
}

static int _changedir( Field* )
/*-------------------------------------------------------------------------*
 * Changes to the current dir in the dir field and updates the lists again
 *-------------------------------------------------------------------------*/
{
    const char *pszDir = pfDirList->GetText();

    if( pszDir[2] == ':' )  /* set disk */
    {
        setdisk( pszDir[1]-'a' );
        if( getdisk() != pszDir[1]-'a' )
            goto _inv;
        getcwd( curdir, 128 );
        _upd_files( NULL );
    }
    else if( chdir( pszDir ) != -1 )
    {
        getcwd( curdir, 128 );
        _upd_files( NULL );
    }
    else
    {
_inv:
        message_box( MB_OK, st_dcksays,
            "I can't change to that drive or directory." );
    }

    return daContinue;
}

static int _upd_files( Field* )
/*-------------------------------------------------------------------------*
 * Updates the files and dirs lists
 *-------------------------------------------------------------------------*/
{
    char    thefile[128];
    short   done, gotone = 0, thedisk = getdisk(), diskidx, fnpos;
    struct  ffblk ff;

    getcwd( curdir, 128 );

    stlFiles->DeleteAll();
    stlDirs->DeleteAll();

    /* disks */

    for( diskidx = 2; diskidx < 26; diskidx++ )
    {
        setdisk( diskidx );
        if( getdisk() == diskidx )
        {
            /* it's there, add it */
            sprintf( thefile, "-%c:-", 'a' + diskidx );
            stlDirs->AddTail( thefile );
        }
    }

    setdisk( thedisk );
    chdir( curdir );

    if( lastchar( curdir ) != '\\' )
        strcat( curdir, "\\" );

    /* dirs */
    sprintf( thefile, "%s*.*", curdir );

    done = findfirst( thefile, &ff, FA_DIREC );

    /* add the parent directory */
    if( strlen( curdir ) > 3 )
        stlDirs->AddTail( ".." );

    while( !done )
    {
        strlwr( ff.ff_name );
        if( ff.ff_attrib & FA_DIREC && ff.ff_name[0] != '.' )
            stlDirs->AddTail( ff.ff_name );
        done = findnext( &ff );
    }

    stlDirs->Sort();
    pfDirList->ReInit();
    pfDirList->SetCurItem( 0 );

    /* files */
    sprintf( thefile, "%s*.%s", curdir, fg_ext );
    done = findfirst( thefile, &ff, FA_ARCH );

    while( !done )
    {
        strlwr( ff.ff_name );
        stlFiles->AddTail( ff.ff_name );
        done = findnext( &ff );
        gotone = 1;
    }

    if( !gotone )
    {
        stlFiles->AddTail( "No files." );
        fNoFiles = 1;
    }
    else
        fNoFiles = 0;

    stlFiles->Sort();
    pfFileList->ReInit();

    if( *fnjump )
    {
        fnpos = stlFiles->Find( fnjump );
        if( fnpos > -1 )
        {
            pfFileList->SetCurItem( fnpos );
            if( !fFirstUpd && !(fg_mode & FG_NOMAPS) )
                pDialog->GoToItem( pfMapList );
        }
        else
            pfFileList->SetCurItem( 0 );
        *fnjump = 0;
    }
    else
        pfFileList->SetCurItem( 0 );

    _upd_maps( NULL );

    return daContinue;
}

static int _nothing( Field* )
{
    pxofs = pyofs = 0;
    pscale = 500;

    _upd_preview( NULL );

    message_box( MB_OK, st_dcksays,
     "Use the Map Control button to move around the map preview:\n"
     "  - Arrow keys move it around\n"
     "  - Use + and - to zoom in on the center\n" );

    return daContinue;
}

static int _mapup( Field* )
{
    pyofs += 100;
    _upd_preview( NULL );
    return daContinue;
}

static int _mapdown( Field* )
{
    pyofs -= 100;
    _upd_preview( NULL );
    return daContinue;
}

static int _mapright( Field* )
{
    pxofs += 100;
    _upd_preview( NULL );
    return daContinue;
}

static int _mapleft( Field* )
{
    pxofs -= 100;
    _upd_preview( NULL );
    return daContinue;
}

static int _zoomout( Field* )
{
    if( pscale > 100 )
        pscale -= 200;
    _upd_preview( NULL );
    return daContinue;
}

static int _zoomin( Field* )
{
    if( pscale < 4000 )
        pscale += 200;
    _upd_preview( NULL );
    return daContinue;
}

static int _updwithfn()
/*-------------------------------------------------------------------------*
 * takes the data from the dir/filename string, and tries to
 * change to the drive and dir specified.  and sets the current file to
 * the one specified, if there was one.  eh.
 *-------------------------------------------------------------------------*/
{
    char    dir[80];
    char    *p;
    char    szFilename[128];
    int     tmpint;

    strcpy( szFilename, pfFilename->GetText() );

    /* change drives? */
    if( szFilename[1] == ':' )
        setdisk( szFilename[0]-'a' );
    if( fnchop( dir, szFilename, FNC_DIRECTORY ) )
        chdir( dir );

    if( lastchar( szFilename ) == '\\' )
        lastchar( szFilename ) = 0;

    *fnjump = 0;

    tmpint = _rtl_chmod( szFilename, 0 );

    /* change to a directory? */
    if( tmpint != -1 && (tmpint & FA_DIREC) )
    {
        chdir( szFilename );
        if( !fFirstUpd )
            pDialog->GoToItem( pfFileList );
    }
    else
    {
        if( tmpint != -1 )
        {
            p = strrchr( szFilename, '\\' );
            if( !p ) p = szFilename;
            else     ++p;
            strccpy( fnjump, p, "." );
        }
    }

    getcwd( curdir, 128 );
    _upd_files( NULL );

    fFirstUpd = 0;

    return daContinue;
}

static int ffFileKeyEnter()
{
    if( fg_mode & FG_SAVE ) // we have a winner!
        return daExitSave;

    return _updwithfn();
}

void fg_freewad( void )
{
    if( ls_wptr )
    {
        wad_close( ls_wptr );
        ls_wptr = NULL;
    }
}

static int dvValidateChoice()
{
    if( newmap == -1 && (fNoMaps || !fMapValid) && !(fg_mode & (FG_NOMAPS | FG_SAVE)) )
    {
        if( fNoMaps )
            message_box( MB_OK, "Hey!", "You can't load a nonexistent map, smartypants." );
        else if( !fMapValid )
            message_box( MB_OK, "Hey!", "That's not a Hexen map." );

        return valNotOkay;
    }

    const char *pFilename = pfFilename->GetText();

    if( strchr( pFilename, ' ' ) )
    {
        message_box( MB_OK, st_dcksays, "That is an invalid filename." );
        return valNotOkay;
    }

    return valOkay;
}

short file_get( short mode, uchar *fn, uchar *ext )
/*-------------------------------------------------------------------------*
 * Dialog box for saving or loading files.
 *-------------------------------------------------------------------------*/
{
    Field   *pField;
    char    origdir[128], title[128];
    short   origdisk = getdisk(), rvl, colq = 60, rowq = 26;
    short   mode_fNoMaps;
    static  char lastdir[128];
    static  short lastdisk;

    fFirstUpd = 1;

    preview( NULL, NULL, 0, 0, 0, 0, 0, 0, 0 );

    getcwd( curdir, 128 );   /* get current dir */
    strcpy( origdir, curdir );
    strcpy( fszFilename, fn );
    strlwr( fszFilename );
    strcpy( fg_ext, ext );

    fg_mode = mode;
    mode_fNoMaps = (fg_mode & FG_NOMAPS) ? 1 : 0;
    fMapValid = FALSE;

    stlFiles = new StrList( 1024, 13, "Files" );  /* 1024 files */
    stlMaps = new StrList( 64, 10, "Maps" );  /* 64 maps in one file */
    stlDirs = new StrList( 256, 13, "Dirs" ); /* 256 dirs */

    sprintf( title, (fg_mode & FG_LOAD) ? "Load a %s" : "Save a %s",
                    (fg_mode & FG_NOMAPS) ? "file" : "map" );

    char szDlgName[32];
    if( mode_fNoMaps )
        strcpy( szDlgName, "FileDlg" );
    else
        strcpy( szDlgName, "FileMapDlg" );

    pDialog = new Dialog;
    pDialog->Create( szDSFile, szDlgName );

    // get the fields
    pfFilename = (fldString*) pDialog->GetField( "Name" );
    pfTag = (fldString*) pDialog->GetField( "Tag" );
    pfDirList = (fldListBox*) pDialog->GetField( "Dirs" );
    pfFileList = (fldListBox*) pDialog->GetField( "Files" );
    pfMapList = (fldListBox*) pDialog->GetField( "Maps" );
    pfPreview = (fldRegion*) pDialog->GetField( "Preview" );
    pfDoNodes = (fldCheckBox*) pDialog->GetField( "DoNodes" );
    pfBtnNew = (fldButton*) pDialog->GetField( "BtnNew" );

    // set transfer
    pfFilename->SetTransferBuf( (PTR) fszFilename );
    if( pfTag )
        pfTag->SetTransferBuf( (PTR) fszTag );
    if( pfDoNodes )
        pfDoNodes->SetTransferBuf( (PTR) &fDoNodes );

    pfDirList->SetItemList( stlDirs );
    pfFileList->SetItemList( stlFiles );
    if( pfMapList )
        pfMapList->SetItemList( stlMaps );

    // set some key functions
    pfFilename->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) ffFileKeyEnter );

    // if not saving..
    if( !(fg_mode & FG_SAVE) )
    {
        if( pfTag )
            pfTag->SetStyle( csInvisible | csSkip, TRUE );
        if( pfDoNodes )
            pfDoNodes->SetStyle( csInvisible | csSkip, TRUE );
        if( pfBtnNew )
            pfBtnNew->SetFunction( (FFUNC) ffNewMap );
    }
    else
    {
        if( pfBtnNew )
            pfBtnNew->SetStyle( csInvisible | csSkip, TRUE );
    }

    pfDirList->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) _changedir );
    pfDirList->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _changedir );

    pfFileList->SetOnChange( (FFUNC) _upd_maps );
    pfFileList->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _selthis );

    if( !mode_fNoMaps )
    {
        pfMapList->SetOnChange( (FFUNC) _upd_preview );
        pfMapList->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) _selthis );
        pfMapList->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _selthis );
    }
    else
        pfMapList = NULL;

    pDialog->SetOnEnter( (DFUNC) _updwithfn );
    pDialog->SetOnValidate( (DVFUNC) dvValidateChoice );

    /* change to last accessed drive / dir */
    if( lastdisk )
    {
        setdisk( lastdisk );
        chdir( lastdir );
        getcwd( curdir, 128 );   /* get current dir */
    }

    /* init vars that might be changed by the time we exit */
    *thetag = 0;
    newmap = -1;
    ls_wptr = NULL;
    pxofs = pyofs = 0;
    pscale = 500;

    rvl = pDialog->Process();

    if( pfMapList )
        strcpy( thetag, pfMapList->GetText() );

    delete pDialog;

    delete stlFiles;
    delete stlDirs;
    delete stlMaps;

    // get the last directory and disk for the future
    getcwd( lastdir, 128 );
    lastdisk = getdisk();

    if( rvl != daExitSave )
        fg_freewad();

    setdisk( origdisk );
    chdir( origdir );

    strcpy( fn, fszFilename );

    FreePreviewData();

    return (rvl == daExitSave);
}

short map_get( void )
/*-------------------------------------------------------------------------*
 * Gets a new map and file.
 *-------------------------------------------------------------------------*/
{
    short rvl, gamenum;

    if( pwdCurWad && nline /* make sure there is data */ )
    {
        strcpy( fszFilename, pwdCurWad->fn );
        strcpy( fszTag, szCurMapTag );
    }
    else
    {
        strcpy( fszFilename, pgdCurGame->dir );
        strlwr( fszFilename );
        if( lastchar( fszFilename ) != '\\' )
            strcat( fszFilename, "\\" );
        *fszTag = 0;
    }

    rvl = file_get( FG_LOAD, fszFilename, "WAD" );

    if( rvl )
    {
        if( newmap != -1 )
        {
            /* newmap contians game number when not -1 .. which it isn't */
            map_new( newmap );

            return 1;
        }

        if( fNoFiles || fNoMaps )
        {
            /* can't do dat */
            fg_freewad();
            return 0;
        }

        gamenum = ismaptag_menu( thetag, ls_wptr );
        if( gamenum == -1 )
        {
            fg_freewad();
            return 0;
        }

        if( !pwdCurWad || !samefile( pwdCurWad->fn, ls_wptr->fn ) )
        {
            /* close the previous wad */
            closecurwad( gamenum );
            game_switch( gamenum );
            wad_reload( ls_wptr, WL_COMPLETE );
            addtexwad( ls_wptr );
            pwdCurWad = ls_wptr;
        }
        else
            game_switch( gamenum );

        strcpy( szCurMapTag, thetag );
        map_load();

        return 1;
    }

    return 0;
}

short file_save( bool fDoDialog, bool fTestMap )
/*-------------------------------------------------------------------------*
 * Pops up save dialog.  If want, save.                                    *
 *-------------------------------------------------------------------------*/
{
    short   tmp;
    char    fn[128];

    if( !nline )
    {
        message_box( MB_OK, st_dcksays, "There's no data to save." );
        return 0;
    }

_again:
    if( pwdCurWad && !fNewMap )
    {
        strcpy( fszFilename, pwdCurWad->fn );
    }
    else
    {
        fszFilename[0] = 0;
        fDoDialog = TRUE;   // new wad to save, so get the dialog box
    }

    strcpy( fszTag, szCurMapTag );

    if( (!pwdCurWad || fDoDialog) && file_get( FG_SAVE, fszFilename, "WAD" ) )
    {
        if( striinc( pgdCurGame->pwdIWAD, fszFilename ) )   /* No pwdIWAD, dude! */
        {
            message_box( MB_OK, "What are you doing, Dave?",
                "As a safeguard, "_ABBREV_" will not let you save to the\n"
                "IWAD ('%s'.)", pgdCurGame->pwdIWAD );
            fg_freewad();
            goto _again;
        }

/*
        if( ls_wptr && samefile( ls_wptr->fn, fszFilename ) &&
            wad_find( ls_wptr, fszTag ) &&
            (fNewMap || !samefile( ls_wptr->fn, pwdCurWad->fn )) )
        {
            /* only if it's a different file, of course */
            tmp = message_box( MB_YESNOCANCEL, "Warning!",
                "The file you have selected to save to already has a map\n"
                "in '%s'.  Do you want to continue?", fszTag );

            fg_freewad();

            if( tmp == MB_NO )
                goto _again;
            else if( tmp == MB_CANCEL )
                return 0;
        }
*/

        fg_freewad();       /* we never want this */

        tmp = ismaptag( fszTag );

        if( tmp < 0 )
        {
            tmp = message_box( MB_YESNO, st_dcksays,
                         "I don't recognize which game the tag '%s'\n"
                         "belongs to.  Do you want to try again?", fszTag );

            if( tmp == MB_YES )
                goto _again;
            else return 0;
        }

        fnchop( fn, fszFilename, FNC_FILE | FNC_DRIVE | FNC_DIRECTORY );
        strcat( fn, ".WAD" );
        int iMode;
        if( fTestMap )
            iMode = wsTestMap;
        else if( fDoNodes )
            iMode = wsComplete;
        else
            iMode = wsDataOnly;
        if( WadSave( fn, fszTag, iMode ) )
        {
            fNewMap = 0;
            nMadeChanges = 0;
            nMapChanges = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
    else if( !fDoDialog )
    {
        if( striinc( pgdCurGame->pwdIWAD, pwdCurWad->fn ) )   /* No pwdIWAD, dude! */
        {
            message_box( MB_OK, "What are you doing, Dave?",
                "As a safeguard, "_ABBREV_" will not let you save to the\n"
                "IWAD ('%s'.)", pgdCurGame->pwdIWAD );
            fDoDialog = TRUE;
            goto _again;
        }

        int iMode;

        if( fTestMap )
            iMode = wsTestMap;
        else
            iMode = wsDataOnly;

        if( WadSave( fszFilename, fszTag, iMode ) )
        {
            fNewMap = 0;
            nMadeChanges = 0;
            nMapChanges = 0;
            return TRUE;
        }
        else
            return FALSE;
    }

    return 0;
}
