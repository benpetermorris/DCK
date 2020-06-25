/*-------------------------------------------------------------------------*
 * WADMGMT.C:
 * Functions for managing .WAD files (ie: adding entries, deleting them,
 * and so on.)
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"

/*-------------------------------------------------------------------------*
 *
 * Code begins here.
 *
 *-------------------------------------------------------------------------*/

short preview( WAD *wptr, uchar *_ltag, short xpos, short ypos,
               short width, short height, short scale, short xofs,
               short yofs );

void copybytes( FILE *inhandle, FILE *outfp, ulong nbytes );
WADENTRY *wad_adde( WAD *wptr, FILE *handle, char *name, long size );

enum
{
    WB_FILE,            /* entry is a file from disk */
    WB_MEMORY,          /* entry is a buffer in memory */
    WB_WADENTRY         /* entry is an entry in another .wad */
};

typedef struct
{
    uchar   type;       /* WB_xxx */
    uchar   fn[81];     /* used for WB_WADENTRY and WB_FILE */
    uchar   entry[9];   /* used for WB_WADENTRY */
    long    offset;     /* used for WB_WADENTRY */
    long    size;       /* used for WB_WADENTRY and WB_MEMORY */
    uchar   *ptr;       /* used for WB_MEMORY */

    uchar   sv_entry[9];        /* save this entry as? */

} WADBlock;

static  NODEPTR     dst_list;      /* of WADBlock */
static  uchar       dst_changed;
static  uchar       dst_fn[128];
static  uchar       dst_sig[4]; /* PWAD or IWAD */

static  WAD*        src_wptr;
static  uchar       src_dir[128];

static  fldCheckBox *pfReplace;
static  stText      *plbSrcPos, *plbSrcItem;
static  StrList     *stlSource, *stlDest;
static  fldListBox  *pfSource, *pfDest;
static  Dialog      *pWadDialog;

static  short       origdisk;
static  char        origdir[128];

static int pb_putall();
static int pb_wmplace();
static int pb_wmdremove();
static int pb_wmdsave();
static int pb_wmdload();
static int pb_wmdnew();
static int pb_wmview( fldListBox* );

static int _listitemmod( Field *fldp );

static int wm_chkdsave( void );
static int wm_EditDest( WADBlock *wbp );
static void setwtitle( void );

#define fld_getcheck(fldp)      (((char*)((fldp)->data))[0])

static NODEPTR wm_FindEntry( char *entry )
{
    NODEPTR ndp = NodeFirst( dst_list );
    WADBlock *wbp;

    while( ndp != dst_list )
    {
        wbp = (WADBlock*) ndp->data;

        if( !strcmp( entry, wbp->sv_entry ) )
            return ndp;

        ndp = NodeNext( ndp );
    }

    return NULL;
}

static WADBlock* wm_CreateDest_WE( WAD* wptr, WADENTRY* weptr )
/*-------------------------------------------------------------------------*
 * Creates a destination entry from "weptr" in wptr.. wowza.
 *-------------------------------------------------------------------------*/
{
    WADBlock    *wbp = (WADBlock*) (NodeInsert( NodeLast( dst_list ),
                sizeof( WADBlock ) )->data);
    memset( wbp, 0, sizeof *wbp );

    wbp->type = WB_WADENTRY;
    strcpy( wbp->fn, wptr->fn );
    strcpy( wbp->entry, WN2Str( weptr->name ) );
    strcpy( wbp->sv_entry, WN2Str( weptr->name ) );
    wbp->offset = weptr->offset;
    wbp->size = weptr->length;

    dst_changed = 1;

    return wbp;
}

static WADBlock* wm_CreateDest_FILE( char *dir, char *fn )
/*-------------------------------------------------------------------------*
 * Creates a new destination entry from "file".
 *-------------------------------------------------------------------------*/
{
    WADBlock    *wbp = (WADBlock*) (NodeInsert( NodeLast( dst_list ),
                sizeof( WADBlock ) )->data);
    short       handle;

    memset( wbp, 0, sizeof *wbp );
    wbp->type = WB_FILE;
    sprintf( wbp->fn, "%s%s%s", dir, lastchar( dir ) == '\\' ? "" : "\\", fn );
    strccpy( wbp->sv_entry, fn, "." );
    handle = z_open( wbp->fn, O_BINARY | O_RDONLY );
    wbp->size = filelength( handle );
    z_close( handle );

    dst_changed = 1;

    return wbp;
}

static int _wm_newdest()
/*-------------------------------------------------------------------------*
 * New destination.
 *-------------------------------------------------------------------------*/
{
    char buf[128];

    if( !wm_chkdsave() )
        return daContinue;

    stlDest->DeleteAll();
    NukeLinkedList( dst_list );
    dst_list = InitList();
    dst_changed = 0;
    strcpy( dst_fn, "UNTITLED.WAD" );
    memcpy( dst_sig, "PWAD", sizeof dst_sig );

    pfDest->OnDraw();

    setwtitle();

    return daContinue;
}

static int wm_UpdateDestwithWAD( char *fn )
{
    WAD*        wptr;
    NODEPTR     ndp;
    WADENTRY*   weptr;

    /* load ze wad */
    wptr = wad_open( fn, WL_COMPLETE );

    if( !wptr )
        return 0;

    memcpy( dst_sig, wptr->hdr.sig, sizeof dst_sig );

    stlDest->DeleteAll();
    NukeLinkedList( dst_list );
    dst_list = InitList();

    strcpy( dst_fn, fn );

    /* update the list */
    ndp = NodeFirst( wptr->dir_list );

    while( ndp != wptr->dir_list )
    {
        weptr = (WADENTRY*) ndp->data;

        if( !islevelresource( weptr->name ) && weptr->name[0] )
        {
            stlDest->AddTail( WN2Str( weptr->name ) );
            wm_CreateDest_WE( wptr, weptr );
        }

        ndp = NodeNext( ndp );
    }

    dst_changed = 0;

    setwtitle();

    pfDest->ReInit();
    pfDest->SetCurItem( 0 );

    return 1;
}

static int _wm_loaddest()
/*-------------------------------------------------------------------------*
 * Loads an existing .wad as a destination file.
 *-------------------------------------------------------------------------*/
{
    char        fn[128];

    if( !wm_chkdsave() )
        return daContinue;

    strcpy( fn, dst_fn );

    /* get a file */
    if( !file_get( FG_LOAD | FG_NOMAPS, fn, "WAD" ) )
        return daContinue;

    wm_UpdateDestwithWAD( fn );

    return daContinue;
}

static int _wm_removedest()
/*-------------------------------------------------------------------------*
 * Removes the current destination entry from the dst_list list .. eh.
 *-------------------------------------------------------------------------*/
{
    short   ditem = pfDest->GetCurItem();
    NODEPTR ndp = NodeNumtoPtr( ditem, dst_list );

    /* nothing to delete */
    if( !(ditem || stlDest->GetCount()) )
        return daContinue;

    NodeDelete( ndp );
    stlDest->Delete( ditem );

    dst_changed = 1;

    pfDest->ReInit();

    if( ditem == stlDest->GetCount() )
        pfDest->SetCurItem( ditem-1 );
    else
        pfDest->OnDraw();

    _listitemmod( pfDest );

    return daContinue;
}

static WADBlock* getDestEntry( void )
/*-------------------------------------------------------------------------*
 * Returns a pointer to the wadblock structure in the current destination
 * entry.
 *-------------------------------------------------------------------------*/
{
    WADBlock* wbRvl = NULL;

    if( NodeNext( dst_list ) != dst_list )  /* only if there's something there */
        wbRvl = (WADBlock*) NodeNumtoPtr( pfDest->GetCurItem(), dst_list )->data;

    return wbRvl;
}

static int _wm_putindest()
/*-------------------------------------------------------------------------*
 * Puts the current thing from the source thing into the destination
 * thing.
 *-------------------------------------------------------------------------*/
{
    WADENTRY*   weptr;
    WADBlock*   wbp;
    short       sitem = pfSource->GetCurItem(), mbrvl, exist_pos;
    NODEPTR     ndp;
    bool        fAskForName;
    char        *p = (char*) (*stlSource)[sitem];

    if( src_wptr )
    {
        /* source is a wad. */
        fAskForName = FALSE;
        weptr = wad_find( src_wptr, p );
        wbp = wm_CreateDest_WE( src_wptr, weptr );
    }
    else
    {
        if( p[0] == '<' || p[0] == '-' )
            return daContinue; /* uh dunt think suh. */

        /* source is a directory. */
        fAskForName = TRUE;
        wbp = wm_CreateDest_FILE( src_dir, p );
    }

_editit:
    /* edit it */
    if( fAskForName &&
        !wm_EditDest( (WADBlock*) NodeLast( dst_list )->data ) )
    {
_cancelit:
        /* aborted .. delete it. */
        NodeDelete( NodeLast( dst_list ) );
        return daContinue;
    }

    /* replace it? */
    if( (ndp = wm_FindEntry( wbp->sv_entry )) != NodeLast( dst_list ) )
    {
        if( !pfReplace->GetState() )
        {
            mbrvl = message_box( MB_YESNOCANCEL, st_dcksays,
                "The entry '%s' already exists in the destination WAD!\n"
                "Do you want to replace it (select 'Yes'), rename it\n"
                "(select 'No'), or not add it at all (select 'Cancel')?", wbp->sv_entry );
            if( mbrvl == MB_NO )
                goto _editit;
            else if( mbrvl == MB_CANCEL )
                goto _cancelit;
        }

        exist_pos = NodePtrtoNum( ndp, dst_list );

        /* free existing data */
        memfree( ndp->data );

        /* replace it with new data */
        ndp->data = wbp;

        /* delete last node */
        ndp = NodeLast( dst_list );
        ndp->data = NULL;
        NodeDelete( ndp );

        pfDest->ReInit();
        pfDest->SetCurItem( exist_pos );
    }
    else
    {
        stlDest->AddTail( wbp->sv_entry );
        /* set to the last item and display the field */
        pfDest->ReInit();
        pfDest->SetCurItem( 4096 );
    }

    return daContinue;
}

static void _wm_updsrc( void )
/*-------------------------------------------------------------------------*
 * Depending on the current status of the source variables, updates
 * the source list:
 *  if src_wad is null, updates the list based on the current directory.
 *  otherwise updates it based on src_wad.
 *-------------------------------------------------------------------------*/
{
    NODEPTR     ndp;
    WADENTRY*   weptr;
    short       done, thedisk = getdisk(), diskidx;
    struct      ffblk ff;
    char        buf[128];

    stlSource->DeleteAll();

    if( src_wptr )
    {
        /* update the list */
        ndp = NodeFirst( src_wptr->dir_list );

        stlSource->AddTail( "<Directory>" );

        while( ndp != src_wptr->dir_list )
        {
            weptr = (WADENTRY*) ndp->data;

            if( !islevelresource( weptr->name ) && weptr->name[0] )
                stlSource->AddTail( WN2Str( weptr->name ) );

            ndp = NodeNext( ndp );
        }
    }
    else
    {
        for( diskidx = 2; diskidx < 26; diskidx++ )
        {
            setdisk( diskidx );
            if( getdisk() == diskidx )
            {
                /* it's there, add it */
                sprintf( buf, "-%c:-", 'A' + diskidx );
                stlSource->AddTail( buf );
            }
        }

        setdisk( thedisk );
        chdir( src_dir );

        done = findfirst( "*.*", &ff, FA_ARCH | FA_RDONLY | FA_DIREC );

        if( strlen( src_dir ) > 3 )
            stlSource->AddTail( "<..>" );

        while( !done )
        {
            if( (ff.ff_attrib & FA_DIREC) && ff.ff_name[0] != '.' )
            {
                sprintf( buf, "<%s>", ff.ff_name );
                stlSource->AddTail( buf );
            }

            done = findnext( &ff );
        }

        done = findfirst( "*.*", &ff, FA_ARCH | FA_RDONLY );

        while( !done )
        {
            if( !(ff.ff_attrib & FA_DIREC) )
                stlSource->AddTail( ff.ff_name );

            done = findnext( &ff );
        }
    }

    stlSource->Sort();

    pfSource->ReInit();
    pfSource->SetCurItem( 0 );

    if( src_wptr )
        sprintf( buf, "%-38s", src_wptr->fn );
    else
        sprintf( buf, "%-38s", src_dir );

    plbSrcPos->SetText( buf );
}

static WAD* _wm_wadlist[16];

static WAD* _wm_openwad( uchar *fn )
{
    short   i;

    for( i = 0; _wm_wadlist[i]; i++ )
    {
        if( samefile( fn, _wm_wadlist[i]->fn ) )
            return _wm_wadlist[i];
    }

    return (_wm_wadlist[i] = wad_open( fn, WL_COMPLETE ));
}

static int _wm_savelist( NODEPTR wlist, uchar *fn )
/*-------------------------------------------------------------------------*
 * Saves all the objects in "wlist" to "fn."
 *-------------------------------------------------------------------------*/
{
    FILE        *outfp, *infp;
    WADBlock    *wbp;
    NODEPTR     ndp;
    WADHEADER   hdr;
    WADENTRY*   weptr, *n_weptr;
    char        tmpname[128], buf[128];
    short       n, total = NodeTotal( wlist ), current = 0;
    WAD         *inwad, *outwad;    /* temp wad storage */
    GWINDOW     *wnp = MAKESTATUSWINDOW( "Saving File", 0 );

    wn_disp( wnp );

    /* Create the new file */
    fnchop( tmpname, fn, FNC_DRIVE | FNC_DIRECTORY );
    if( lastchar( tmpname ) != '\\' )
        strcat( tmpname, "\\" );
    strcat( tmpname, "$TMP.WAD" );

    outwad = wad_new( "$TMP.WAD" );
    outfp = fopen( tmpname, "wb" );
    memcpy( outwad->hdr.sig, dst_sig, sizeof dst_sig );
    fwrite( &(outwad->hdr), sizeof( WADHEADER ), 1, outfp );

    ndp = NodeFirst( wlist );

    while( ndp != wlist )
    {
        wbp = (WADBlock*) ndp->data;

        sprintf( buf, "Saving %s...", wbp->sv_entry );
        wn_statstr( wnp, buf );
        wn_stat( wnp, current, total );

        current++;

        n_weptr = wad_adde( outwad, outfp, wbp->sv_entry, wbp->size );

        switch( wbp->type )
        {
        case WB_FILE:
            infp = fopen( wbp->fn, "rb" );
            copybytes( infp, outfp, wbp->size );
            fclose( infp );
            break;
        case WB_WADENTRY:
            /* normal entry? */
            infp = fopen( wbp->fn, "rb" );
            if( ismaptag( wbp->entry ) < 0 )
            {
                fseek( infp, wbp->offset, SEEK_SET );
                copybytes( infp, outfp, wbp->size );
            }
            else
            {
                inwad = _wm_openwad( wbp->fn );

                /* write out the tag, and all the data */
                if( wad_find( inwad, "BEHAVIOR" ) )
                    n = 11;
                else
                    n = 10;

                weptr = wad_find( inwad, wbp->entry );

                while( n-- )
                {
                    /* next level entry */
                    inwad->curdir = NodeNext( inwad->curdir );
                    weptr = (WADENTRY*) inwad->curdir->data;

                    /* check it.  word.  to your mother. */
                    wad_adde( outwad, outfp, weptr->name, weptr->length );
                    fseek( infp, weptr->offset, SEEK_SET );
                    copybytes( infp, outfp, weptr->length );
                }
            }

            fclose( infp );

            break;
        }

        wbp->type = WB_WADENTRY;
        strcpy( wbp->fn, fn );
        strcpy( wbp->entry, wbp->sv_entry );
        wbp->offset = n_weptr->offset;
        wbp->size = n_weptr->length;

        ndp = NodeNext( ndp );
    }

    outwad->hdr.dirstart = ftell( outfp );

    /* write out the directory list now.. wow. */
    ndp = NodeFirst( outwad->dir_list );

    while( ndp != outwad->dir_list )
    {
        weptr = (WADENTRY*) ndp->data;
        fwrite( weptr, sizeof *weptr, 1, outfp );
        ndp = NodeNext( ndp );
    }

    fseek( outfp, 0L, SEEK_SET );
    fwrite( &(outwad->hdr), sizeof( WADHEADER ), 1, outfp );
    fclose( outfp );

    remove( fn );
    rename( tmpname, fn );

    checktexupdate( outwad );
    wad_close( outwad );

    for( n = 0; _wm_wadlist[n]; n++ )
    {
        wad_close( _wm_wadlist[n] );
        _wm_wadlist[n] = NULL;
    }

    wn_undisp( wnp );
    wn_free( wnp );

    dst_changed = 0;

    return 1;
}

static int _wm_savedest()
/*-------------------------------------------------------------------------*
 * Saves the destination list to a file.
 *-------------------------------------------------------------------------*/
{
    char        fn[128];
    short       rvl;

_getagain:
    strcpy( fn, dst_fn );

    if( !file_get( FG_SAVE | FG_NOMAPS, fn, "WAD" ) )
        return daContinue;

    if( !strchr( fn, '.' ) )
        strcat( fn, ".WAD" );

    if( !samefile( dst_fn, fn ) && access( fn, 0 ) != -1 )
    {
        rvl = message_box( MB_YESNOCANCEL, st_dcksays,
            "The file '%s' exists!  Saving to it will overwrite\n"
            "the existing contents.  Are you sure you want to?", fn );

        if( rvl == MB_CANCEL )
            return daContinue;

        if( rvl == MB_NO )
            goto _getagain;
    }

    _fullpath( dst_fn, fn, 128 );

    /* save it. */
    _wm_savelist( dst_list, dst_fn );

    setwtitle();

    return daContinue;
}

static int wm_chkdsave( void )
/*-------------------------------------------------------------------------*
 * If the file has been modified, asks the user whether they want to save
 * it.
 *
 * Returns 1 on "continue" (yes/no), 0 on "cancel" (cancel)
 *-------------------------------------------------------------------------*/
{
    short rvl;

    if( !dst_changed )
        return TRUE;

    /* save it */
_askagain:
    rvl = message_box( MB_YESNOCANCEL, st_dcksays,
        "The WADfile you're working with has changed: do you want\n"
        "to save the changes?" );

    if( rvl == MB_CANCEL )
        return FALSE;

    if( rvl == MB_YES )
        _wm_savedest();
    else
        return TRUE;       /* MB_NO */

    if( dst_changed )
        goto _askagain;

    return TRUE;
}

static int _wm_checkexit()
/*-------------------------------------------------------------------------*
 * When ESC or F10 is pressed.
 *-------------------------------------------------------------------------*/
{
    if( !wm_chkdsave() )
        return FALSE;

    return TRUE;
}

static int wm_EditDest( WADBlock *wbp )
/*-------------------------------------------------------------------------*
 * Allows the user to edit a destination object.
 *-------------------------------------------------------------------------*/
{
    Dialog  *pDialog;
    Field   *pField;
    char    title[128];
    char    fromfile[128];
    char    save_as[10];

    sprintf( title, "Editing entry '%s'", wbp->sv_entry );

    pDialog = new Dialog;
    pDialog->Create( Point( Centered, Centered ), Size( 200, 48 ), title );

    if( wbp->type == WB_WADENTRY )
        sprintf( fromfile, "File: %s [%s]", wbp->fn, wbp->entry );
    else
        sprintf( fromfile, "File: %s", wbp->fn );

    new stText( pDialog, Point( 4, 4 ), fromfile );
    fldString* pString = new fldString( pDialog, Point( 4, 16 ), 40,
        ftString, "Save as:", csPromptBeside | csUpper, 8 );
    pString->SetTransferBuf( (PTR) wbp->sv_entry );

    pDialog->AddDefaultButtons();

    int iRvl = pDialog->Process();

    delete pDialog;

    if( iRvl == daExitSave )
        dst_changed = TRUE;

    return (iRvl == daExitSave);
}

static int _wm_destedit()
/*-------------------------------------------------------------------------*
 * Allows the user to edit a destination object.
 *-------------------------------------------------------------------------*/
{
    WADBlock *wbp;

    if( stlDest->GetCount() )
    {
        wbp = getDestEntry();

        if( wm_EditDest( wbp ) )
        {
            stlDest->SetAt( pfDest->GetCurItem(), wbp->sv_entry );
            pfDest->ReInit();
            pfDest->OnDraw();
        }
    }

    return daContinue;
}

static int _listitemmod( Field *fldp )
/*-------------------------------------------------------------------------*
 * Called when the user moves around the src/dest list.
 *-------------------------------------------------------------------------*/
{
    WADENTRY    *weptr = NULL;
    WADBlock    *wbp;
    short       item = pfSource->GetCurItem();
    short       handle;
    char        buf[128], *sdata = (char*) pfSource->GetText();
    char        *p = (char*) (*stlSource)[item];

    if( fldp == pfSource )
    {
        if( src_wptr )
        {
            weptr = wad_find( src_wptr, p );
            if( weptr )
            {
                if( ismaptag( weptr->name ) < 0 )
                    sprintf( buf, "%8lu bytes", weptr->length );
                else
                    sprintf( buf, "Map data for %s", WN2Str( weptr->name ) );
            }
            else
                strcpy( buf, "Go back to the file list" );
        }
        else
        {
            if( sdata[0] == '-' )
                sprintf( buf, "Change to drive %c:", sdata[1] );
            else if( sdata[0] == '<' )
                strcpy( buf, "Change to this directory" );
            else
            {
                handle = z_open( sdata, O_BINARY | O_RDONLY );
                sprintf( buf, "%8lu bytes", filelength( handle ) );
                z_close( handle );
            }
        }
    }
    else
    {
        wbp = getDestEntry();

        if( wbp )
        {
            if( wbp->type == WB_WADENTRY )
                sprintf( buf, "%s from %s", wbp->entry, wbp->fn );
            else
                sprintf( buf, "Data from %s", wbp->fn );
        }
        else
            *buf = 0;
    }

    char szText[128];
    sprintf( szText, "%-38s", buf );
    plbSrcItem->SetText( szText );

    return daContinue;
}

static short showpic( short x, short y, short xw, short yw, uchar *fn, ulong ofs )
{
    short handle = open( fn, O_BINARY | O_RDONLY ), s2p_rvl;
    extern uchar *picbuf;
    extern SRECT prb;
    extern uchar sp2pb_reterr;       /* return on error in spr2picbuf()? */
    short spr2picbuf( short x, short y, short handle, long offset, short center, int );

    prb.left = 0;
    prb.right = xw;
    prb.top = 0;
    prb.bottom = yw;

    mkpicbuf( xw, yw );

    /* centered */
    sp2pb_reterr = 1;
    s2p_rvl = spr2picbuf( 0, 0, handle, ofs, 1, 100 );
    sp2pb_reterr = 0;
    close( handle );

    if( !s2p_rvl )
    {
        message_box( MB_OK, st_dcksays, "That's not a game picture!" );
        return 0;
    }

    putimage( x, y, picbuf, COPY_PUT );

    return 1;
}

static void showbinary( uchar *fn, ulong offset, long length )
/*-------------------------------------------------------------------------*
 * Guess.
 *-------------------------------------------------------------------------*/
{
    uchar   **txt;
    short   nline = 0;
    short   numchars = 16, i, curdisk = getdisk(), slen = 0;
    long    nread = 0;
    extern  int dckfont;
    uchar   c;
    uchar   charbuf[16], buf[128], *bufp;
    FILE    *fp = fopen( fn, "rb" );
    char    *hcp = "0123456789ABCDEF";

    if( !fp )
        return;

    fseek( fp, offset, SEEK_SET );

    if( length == -1 )
        length = filelength( fileno( fp ) );

    txt = (uchar**) memget( sizeof( uchar * ) * 2048 );

    /* read in the bufferZz */
    while( numchars == 16 )
    {
        if( nread + numchars > length )
            numchars = length - nread;

        fread( charbuf, numchars, 1, fp );

        sprintf( buf, " %04X: ", (short) nread );
        bufp = buf+6;

        for( i = 0; i < numchars; i++ )
        {
            c = charbuf[i];
            *bufp++ = hcp[c/16];
            *bufp++ = hcp[c%16];
            *bufp++ = 32;
        }

        for( ; i < 16; i++ )
        {
            /* padding */
            *bufp++ = 32;
            *bufp++ = 32;
            *bufp++ = 32;
        }

        /* actual chars */
        *bufp++ = 32;

        for( i = 0; i < numchars; i++ )
            *bufp++ = charbuf[i] ? charbuf[i] : 32;

        *bufp++ = 32;
        *bufp = 0;

        if( !slen )
            slen = strlen( buf ) + 1;

        strcpy( txt[nline++] = (uchar*) memget( slen ), buf );

        if( nline == 2047 )
        {
            message_box( MB_OK, st_dcksays,
              "The data is too big to display entirely; Only the first\n"
              "32k can be shown." );
            break;
        }

        nread += numchars;
    }

    /* change to dck location */
    setdisk( origdisk );
    chdir( origdir );

    settextstyle( DEFAULT_FONT, HORIZ_DIR, 1 );
    int iOldFW = _fontwidth, iOldFH = _fontheight;
    _fontwidth = 8;
    _fontheight = 8;
    readtext( "Displaying data", (char**) txt, nline, 32 );
    settextstyle( dckfont, HORIZ_DIR, 1 );
    _fontwidth = iOldFW;
    _fontheight = iOldFH;

    /* change back to source location */
    setdisk( curdisk );
    chdir( src_dir );

    for( i = 0; i < nline; i++ )
        memfree( txt[i] );

    memfree( txt );
}

static int _wm_view( fldListBox* fldp )
/*-------------------------------------------------------------------------*
 * View the current entry.
 *-------------------------------------------------------------------------*/
{
    GWINDOW*    wnp = NULL;
    WADENTRY*   weptr;
    short       rvl, x, y;
    ulong       ofs, len = -1;
    char        *pszFile;
    uchar*      viewtag;
    char        *pszSel = (char*) fldp->GetText();
    WAD*        view_wptr = src_wptr;
    WADBlock*   wbp;

    static      short lastview;

    if( fldp == pfSource )
    {
        if( src_wptr )
        {
            weptr = wad_find( src_wptr, pszSel );

            if( !weptr )
                return daContinue;

            ofs = weptr->offset;
            len = weptr->length;
            pszFile = src_wptr->fn;
        }
        else
        {
            ofs = 0;
            pszFile = pszSel;
        }
    }
    else
    {
        wbp = getDestEntry();

        ofs = wbp->offset;
        len = wbp->size;
        pszFile = wbp->fn;
    }

    if( access( pszFile, 0 ) == -1 )
        return daContinue;

    if( ismaptag( pszSel ) < 0 )
    {
        /* view type */
        rvl = MenuPop( lastview, "View as...", "MN_VIEWDATA",
                     " Binary     ", "View the data as a binary dump",
                     " Picture    ", "View the data as a game picture",
                     NULL );

        if( rvl == -1 )
            return daContinue;
        else if( rvl == 1 )
        {
            cursor_put( CSR_ERASE );

            wnp = wn_def( Centered, pWadDialog->ScreenRect().top + _fontheight * 3,
                          264 / _fontwidth + 1,
                          128 / _fontheight + 1,
                          C_GRAY3, C_GRAY1, WF_POPUP | WF_3D, "Viewing picture" );
            wn_disp( wnp );

            gr_rshadrect( 4, 4, 261, 134, 0 );
            if( !showpic( 5, 5, 256, 128, pszFile, ofs ) )
                keystuff( KEY_ESC );        /* to get rid of window */

            cursor_put( CSR_PUT );
        }
        else
            showbinary( pszFile, ofs, len );

        lastview = rvl;
    }
    else
    {
        if( fldp == pfDest )
        {
            /* not supported */
            if( wbp->type != WB_WADENTRY )
                return daContinue;

            /* since this is the destination, we have to open the file that htis
             * entry in the destination is getting its data from. */
            view_wptr = wad_open( pszFile, WL_MAPONLY );
            viewtag = wbp->entry;
        }
        else
            viewtag = pszSel;

        cursor_put( CSR_ERASE );

        wnp = wn_def( Centered, Centered, 264 / _fontwidth + 1, 264 / _fontheight + 1,
                    C_GRAY3, C_GRAY1, WF_POPUP | WF_3D, "Viewing map" );
        wn_disp( wnp );

        gr_rshadrect( 4, 4, 261, 261, 0 );
        preview( view_wptr, viewtag, 5, 5, 256, 256, 500, 0, 0 );

        if( fldp == pfDest )
            wad_close( view_wptr );

        cursor_put( CSR_PUT );
    }

    if( wnp )
    {
        while( 1 )
        {
            rvl = mouse_event( &x, &y );

            if( keycheck() || rvl == MOUSE_LUP || rvl == MOUSE_RUP )
                break;
        }

        if( keycheck() )
            keyget();

        wn_undisp( wnp );
        wn_free( wnp );
    }

    return daContinue;
}

static int _wm_saveentry( Field *fldp )
/*-------------------------------------------------------------------------*
 * Save an entry into a file .. like it says.
 *-------------------------------------------------------------------------*/
{
    WADBlock    *wbp;
    WADENTRY    *weptr;
    long        ofs, len;
    char        *srcfn, dstfn[128];
    FILE        *infp, *outfp;

    dstfn[0] = 0;

    if( fldp == pfDest )
    {
        wbp = getDestEntry();

        if( ismaptag( wbp->entry ) >= 0 )
        {
_cantsavemapdata:
            message_box( MB_OK, st_dcksays,
              "I can't save map data to a raw file." );
            return daContinue;
        }

        if( wbp->type == WB_FILE )  /* no point in saving that. */
            goto _whysavethat;

        srcfn = wbp->fn;
        ofs = wbp->offset;
        len = wbp->size;

        strcpy( dstfn, WN2Str( wbp->sv_entry ) );
    }
    else
    {
        if( src_wptr )
        {
            weptr = wad_find( src_wptr, (char*) pfSource->GetText() );
            if( !weptr )
                return daContinue;
            srcfn = src_wptr->fn;
            ofs = weptr->offset;
            len = weptr->length;

            if( ismaptag( weptr->name ) >= 0 )
                goto _cantsavemapdata;

            strcpy( dstfn, WN2Str( weptr->name ) );
        }
        else
        {
_whysavethat:
            message_box( MB_OK, st_dcksays, "That already IS a file!" );
            return daContinue;
        }
    }

    /* get the filename */
    if( !file_get( FG_SAVE | FG_NOMAPS, dstfn, "*" ) )
        return daContinue;

    /* save it */
    infp = fopen( srcfn, "rb" );
    fseek( infp, ofs, SEEK_SET );
    outfp = fopen( dstfn, "wb" );
    copybytes( infp, outfp, len );
    fclose( infp );
    fclose( outfp );

    return daContinue;
}

static int _wm_srcchange()
/*-------------------------------------------------------------------------*
 * Looks at the current source field entry and "changes" to it
 *-------------------------------------------------------------------------*/
{
    short   sitem = pfSource->GetCurItem();
    char    *pszEntry = (char*) pfSource->GetText();
    char    buf[128];
    short   handle;

    if( src_wptr )
    {
        if( !sitem )
        {
            /* back to directory */
            wad_close( src_wptr );
            src_wptr = NULL;
            _wm_updsrc();
        }
        else
        {
            /* view it */
            _wm_view( pfSource );
        }
    }
    else
    {
        if( pszEntry[0] == '<' )
        {
            /* change to directory */
            strccpy( buf, pszEntry+1, ">" );
            chdir( buf );
            getcwd( src_dir, 128 );
            _wm_updsrc();
        }
        else if( pszEntry[0] == '-' )
        {
            setdisk( pszEntry[1] - 'A' );
            getcwd( src_dir, 128 );
            _wm_updsrc();
        }
        else
        {
            /* check the file: is it a .wad? */
            handle = z_open( pszEntry, O_RDONLY | O_BINARY );
            z_read( handle, buf, 4 );
            z_close( handle );

            if( !memcmp( buf, "PWAD", 4 ) || !memcmp( buf, "IWAD", 4 ) )
            {
                /* yAh! */
                src_wptr = wad_open( pszEntry, WL_COMPLETE );
                _wm_updsrc();
            }
            else
            {
                /* view it. */
                _wm_view( pfSource );
            }
        }
    }

    _listitemmod( pfSource );

    return daContinue;
}

static int _wm_dbup()
{
    if( !pwdCurWad )
        _wm_newdest();
    else
        wm_UpdateDestwithWAD( pwdCurWad->fn );

    /* get the current directory into the source list */
    _wm_updsrc();
    _listitemmod( pfSource );

    return daContinue;
}

static void setwtitle( void )
{
    uchar   buf[128];

    sprintf( buf, "WAD Management [%s]", dst_fn );
    pWadDialog->SetTitle( buf );
}

void wad_mgmt( void )
/*-------------------------------------------------------------------------*
 * Creates the wad management dialog box and allows the user to do all
 * the management operations.  This is all very exciting.
 *
 * There is always a destination and a source.  The destination is a linked
 * list of entries, and the source is a directory list or a .wad file.
 *
 * When a destination is loaded, all its entries are loaded into the dst_list
 * linked list.  Entries can be added or removed.  On closing the destination,
 * each entry is checked and saved.
 *
 * A Note: Map resources other than the map tags are not displayed OR stored
 * in the string table lists or the destination lists or anything.  However,
 * when a map tag entry is operated on (ie: saved, or viewed, or whatever),
 * the fact that it is a map tag is taken into account .. y'know?
 *-------------------------------------------------------------------------*/
{
#ifndef REGISTERED
    Unregistered();
#else
    static  char srcbuf[128], srcbuf2[128];
    Field   *fldp;

    *srcbuf = 0;
    *srcbuf2 = 0;
    dst_changed = 0;

    getcwd( origdir, 128 );
    origdisk = getdisk();
    strcpy( src_dir, origdir );

    /* initialize source and destination listboxes and linked lists */
    stlSource = new StrList( 5120, 13, "SourceList" );
    stlDest = new StrList( 5120, 13, "DestList" );
    dst_list = InitList();

    pWadDialog = new Dialog;
    pWadDialog->Create( szDSFile, "WadmgmtDlg" );

    VarTransfer_t vtWadMgmt[] =
    {
        { "SourceList",     (Field**) &pfSource,    NULL,
            "Press ENTER to view this item" },
        { "DestList",       (Field**) &pfDest,      NULL,
            "Press ENTER to view this item" },
        { "Replace",        (Field**) &pfReplace,   NULL,
            "Check this to replace entries automatically" },

        // end marker
        { NULL, NULL, NULL, NULL }
    };


    pWadDialog->VarSetup( vtWadMgmt );

    // get text items
    plbSrcPos = (stText*) pWadDialog->GetLabel( "SrcPos" );
    plbSrcItem = (stText*) pWadDialog->GetLabel( "SrcItem" );

    pfSource->SetItemList( stlSource );
    pfDest->SetItemList( stlDest );

    pfReplace->SetState( FALSE );

    pfSource->SetOnChange( (FFUNC) _listitemmod );
    pfSource->SetOnEnter( (FFUNC) _listitemmod );

    pfSource->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) _wm_srcchange );
    pfSource->SetUserEvent( etKey, KEY_CTRLP, (FUFUNC) _wm_putindest );
    pfSource->SetUserEvent( etKey, KEY_CTRLL, (FUFUNC) _wm_loaddest );
    pfSource->SetUserEvent( etKey, KEY_CTRLS, (FUFUNC) _wm_savedest );
    pfSource->SetUserEvent( etKey, KEY_CTRLX, (FUFUNC) _wm_saveentry );
    pfSource->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _wm_srcchange );

    // set button functions
    pWadDialog->SETBUTTONFUNC( "BtnPut", (FFUNC) pb_wmplace );
    pWadDialog->SETBUTTONFUNC( "BtnViewSrc", (FFUNC) pb_wmview );
    pWadDialog->SETBUTTONFUNC( "BtnViewDest", (FFUNC) pb_wmview );
    pWadDialog->SETBUTTONFUNC( "BtnRemove", (FFUNC) pb_wmdremove );
    pWadDialog->SETBUTTONFUNC( "BtnSave", (FFUNC) pb_wmdsave );
    pWadDialog->SETBUTTONFUNC( "BtnLoad", (FFUNC) pb_wmdload );
    pWadDialog->SETBUTTONFUNC( "BtnNew", (FFUNC) pb_wmdnew );

    // set dest parameters
    pfDest->SetOnChange( (FFUNC) _listitemmod );
    pfDest->SetOnEnter( (FFUNC) _listitemmod );
    pfDest->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) _wm_view );
    pfDest->SetUserEvent( etKey, KEY_DEL, (FUFUNC) _wm_removedest );
    pfDest->SetUserEvent( etKey, KEY_CTRLS, (FUFUNC) _wm_savedest );
    pfDest->SetUserEvent( etKey, KEY_CTRLE, (FUFUNC) _wm_destedit );
    pfDest->SetUserEvent( etKey, KEY_CTRLL, (FUFUNC) _wm_loaddest );
    pfDest->SetUserEvent( etKey, KEY_CTRLX, (FUFUNC) _wm_saveentry );
    pfDest->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _wm_view );

    pWadDialog->SetOnEnter( (DFUNC) _wm_dbup );
    pWadDialog->SetOnExit( (DFUNC) _wm_checkexit );

    pWadDialog->Process();

    delete pWadDialog;

    if( dst_list )
        NukeLinkedList( dst_list );

    delete stlDest;
    delete stlSource;

    if( src_wptr )
    {
        wad_close( src_wptr );
        src_wptr = NULL;
    }

    setdisk( origdisk );
    chdir( origdir );
#endif
}

static int pb_wmdnew()
{
    return _wm_newdest();
}

static int pb_wmdload()
{
    return _wm_loaddest();
}

static int pb_wmview( fldListBox* pField )
{
    if( pField == pfSource )
        return _wm_srcchange();
    else if( pField == pfDest )
        return _wm_view( pField );
}

static int pb_wmdsave()
{
    return _wm_savedest();
}

static int pb_wmdremove()
{
    return _wm_removedest();
}

static int pb_wmplace()
{
    return _wm_putindest();
}

static int pb_putall()
{
    return _wm_putindest();
}
