/*-------------------------------------------------------------------------*
 * WAD.C:                                                                  *
 * Functions for loading/saving WAD files                                  *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#pragma inline

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static  NODEPTR wad_list;   /* list of open .wads */
static  short   _writeerr = 0;

#include "statbars.h"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

WAD *wad_new( char *wadfn )
{
    WAD       *wptr;
    uchar     *fn;

    wptr = (WAD*) memget( sizeof( WAD ) );
    fn = wptr->fn;

    memcpy( wptr->hdr.sig, "PWAD\0\0\0\0", 8 );
    wptr->hdr.nentry = 0;
    wptr->hdr.dirstart = 0;

    _fullpath( fn, wadfn, 128 );

    wptr->dir_list = InitList();

    return wptr;
}

WAD *wad_open( char *wadfn, short mode )
/*-------------------------------------------------------------------------*
 * 1. Reads all level data from the new wad and updates level handles as   *
 *    is appropriate.                                                      *
 *-------------------------------------------------------------------------*/
{
    short     handle;
    WAD       *wptr;
    WADHEADER *wadhdr;
    uchar     *fn;

    wptr = wad_new( wadfn );
    wadhdr = &wptr->hdr;

    fn = wptr->fn;

    handle = z_open( fn, O_BINARY | O_RDONLY );

    if( handle == -1 )
    {
        strcat( fn, ".WAD" );
        handle = z_open( fn, O_BINARY | O_RDONLY );

        if( handle == -1 )
        {
            fnchop( fn, pgdCurGame->dir, FNC_DRIVE | FNC_DIRECTORY );
            strcat( fn, "\\" );
            strcat( fn, wadfn );
            handle = z_open( fn, O_BINARY | O_RDONLY );
            if( handle == -1 )
            {
                strcat( fn, ".WAD" );
                handle = z_open( fn, O_BINARY | O_RDONLY );
                if( handle == -1 )
                    return NULL;
            }
        }
    }

    /* Read the header of the WAD file */
    z_read( handle, wadhdr, sizeof *wadhdr );

    /* Check the validity */
    if( !memcmp( wadhdr->sig, "IWAD", 4 ) )      /* is "IWAD" */
        wptr->typ = 0;
    else if( !memcmp( wadhdr->sig, "PWAD", 4 ) )  /* not "PWAD" */
        wptr->typ = 1;
    else
        error( "wadopen: file '%s' isn't a WAD file.", fn );

    wptr->dir_list = InitList();

    /* Close it because that's all we're doing with wadopen */
    z_close( handle );

    /* Read the WAD map data */
    wad_load( wptr, mode );

    /* add this to the list of wads */
    if( !wad_list ) wad_list = InitList();
    NodeInsert( NodeLast( wad_list ), 0L )->data = wptr;

    return wptr;
}

void wad_close( WAD *wptr )
/*-------------------------------------------------------------------------*
 * Closes a wad - frees its memory, removes it from the list of open wads.
 *-------------------------------------------------------------------------*/
{
    NODEPTR ndp;

    if( !wptr )
        return;

    ndp = NodeFindData( wad_list, wptr );

    if( ndp )
    {
        ndp->data = NULL;   /* this comes later */
        NodeDelete( ndp );
    }

    NukeLinkedList( wptr->dir_list );
    free( wptr );
}

uchar* fixwadnam( uchar *s )
{
    uchar   *p = s, ch, n = 8;

    while( n-- )
    {
        ch = *p;
        *p++ = _mtoupper( ch );
    }

    return s;
}

void wad_reload( WAD* wptr, short mode )
/*-------------------------------------------------------------------------*
 * Reloads a .WAD's dir with a new mode.
 *-------------------------------------------------------------------------*/
{
    DestroyList( wptr->dir_list );
    wptr->dir_list = InitList();
    wad_load( wptr, mode );
}

short wad_load( WAD *wptr, short mode )
/*-------------------------------------------------------------------------*
 * loads up wptr's dir_list with the directory.                            *
 *-------------------------------------------------------------------------*/
{
    FILE        *handle;
    WADENTRY    we;
    ushort      storecount = 0, nentry;
    NODEPTR     dir_list = wptr->dir_list, ndp;

    /* open the file */
    if( (handle = fopen( wptr->fn, "rb" )) == NULL )
        error( "wad_load: can't open wadfile %s", wptr->fn );

    /* Let's have a 4k buffer, shall we? */
    setvbuf( handle, NULL, _IOFBF, 4096 );

    fread( &(wptr->hdr), sizeof( WADHEADER ), 1, handle );
    nentry = wptr->hdr.nentry;

    /* Jump to the directory */
    fseek( handle, wptr->hdr.dirstart, SEEK_SET );

    wptr->mode = mode;
    wptr->hdr.nentry = 0;

    while( nentry-- )
    {
        fread( &we, sizeof( WADENTRY ), 1, handle );

        if( mode == WL_MAPONLY )     /* store only maps? */
        {
            if( !storecount && !wncmp( we.name, "BEHAVIOR" ) )
                storecount = 1;
            else if( !storecount && ismaptag( we.name ) != -1 )
                storecount = 11;
        }

        if( (mode == WL_COMPLETE || storecount) && we.name[0] )
        {
            ndp = NodeInsert( NodeLast( dir_list ), sizeof( WADENTRY ) );
            wptr->hdr.nentry++;

            /* upper */
            fixwadnam( we.name );

            memcpy( ndp->data, &we, sizeof( WADENTRY ) );
            if( storecount )
                --storecount;
        }
    }

    fclose( handle );

    return 1;
}

WADENTRY* wad_findCurMap( WAD *wptr, uchar *_name )
/*-------------------------------------------------------------------------*
 * Finds the name following the global szCurMapTag[].
 *-------------------------------------------------------------------------*/
{
    short   i, iCount = 12;
    NODEPTR ndp, dir_list = wptr->dir_list;

    if( !wad_find( wptr, szCurMapTag ) )
        return NULL;

    ndp = wptr->curdir;

    /* Find the thing. */
    while( ndp != dir_list && iCount-- )
    {
        if( !wncmp( ((WADENTRY *) ndp->data)->name, _name ) )
        {
            /* Found! */
            wptr->curdir = ndp;
            return (WADENTRY*) wptr->curdir->data;  /* return the entry, not node */
        }

        ndp = NodeNext( ndp );
    }

    /* Not found! */
    wptr->curdir = NULL;
    return NULL;
}

WADENTRY* wad_find( WAD *wptr, uchar *_name )
/*-------------------------------------------------------------------------*
 * Searches the directory forward from 'curdir' for 'name' and returns 1   *
 * if found, 0 otherwise.  If found, sets curdir to the associated node,   *
 * otherwise restores curdir to its original value.  Call dirfind( NULL )  *
 * to reset curdir to the first entry.                                     *
 *-------------------------------------------------------------------------*/
{
    NODEPTR dir_list = wptr->dir_list, ndp = NodeFirst( dir_list );
    short     i;

    /* Find the thing. */
    while( ndp != dir_list )
    {
        if( !wncmp( ((WADENTRY *) ndp->data)->name, _name ) )
        {
            /* Found! */
            wptr->curdir = ndp;
            return (WADENTRY*) wptr->curdir->data;  /* return the entry, not node */
        }

        ndp = NodeNext( ndp );
    }

    /* Not found! */
    wptr->curdir = NULL;
    return NULL;
}

void *dirload( WAD *wptr, ulong *size, uchar *buf )
/*-------------------------------------------------------------------------*
 * Loads the entry pointed to by 'curdir' by allocating memory for the     *
 * resource and returning a pointer to it.                                 *
 * If buf is NULL, allocates memory for the data and returns a pointer to  *
 * it.  Otherwise, stores it in buf and returns buf.                       *
 *-------------------------------------------------------------------------*/
{
    FILE        *fp;
    WADENTRY    *weptr = (WADENTRY*) wptr->curdir->data;
    void        *ptr;

    if( wptr->curdir == NULL )
    {
        /* meep! */
        error( "dirload: curdir == NULL" );
    }

    /* open the file */
    if( (fp = fopen( wptr->fn, "rb" )) == NULL )
        error( "dirload: can't open wadfile %s", wptr->fn );

    /* seek to the right position */
    fseek( fp, weptr->offset, SEEK_SET );

#ifndef __FLAT__
    if( weptr->length > 0xFFFAL )
        error( "dirload: attempt to read > 64k resource" );
#endif

    /* allocate some memory */
    if( buf == NULL )
    {
        _memstr = "dirload() buffer";
        ptr = memget( weptr->length );
    }
    else
        ptr = buf;

    /* read in the resource */
    fread( ptr, weptr->length, 1, fp );

    if( size != NULL )
        *size = weptr->length;

    /* Close the file */
    fclose( fp );

    return ptr;
}

FILE* wad_e2h( WAD *wptr, WADENTRY *weptr )
/*-------------------------------------------------------------------------*
 * opens wptr, seeks to weptr, and returns the handle.
 *-------------------------------------------------------------------------*/
{
    FILE* handle;

    if( weptr == NULL )
        weptr = (WADENTRY*) wptr->curdir->data;

    handle = fopen( wptr->fn, "rb" );

    if( handle == NULL )
        return NULL;

    fseek( handle, weptr->offset, SEEK_SET );

    return handle;
}

ulong dirgimmesize( WAD *wptr )
/*-------------------------------------------------------------------------*
 * Returns the size of the 'pointed-to' directory entry                    *
 *-------------------------------------------------------------------------*/
{
    if( wptr->curdir == NULL )
        error( "dirgimmesize: curdir == NULL" );
    return( ((WADENTRY *) wptr->curdir->data)->length );
}

WADENTRY *wad_adde( WAD *wptr, FILE *handle, char *name, long size )
/*-------------------------------------------------------------------------*
 * Adds an entry to a .WAD's directory in memory.
 *-------------------------------------------------------------------------*/
{
    WADENTRY    *weptr;
    NODEPTR     ndp;

    ndp = NodeInsert( NodeLast( wptr->dir_list ), sizeof( WADENTRY ) );
    weptr = (WADENTRY*) ndp->data;

    if( size )
        weptr->offset = ftell( handle );
    else
        weptr->offset = 0L;

    ++(wptr->hdr.nentry);

    weptr->length = size;

    wncpy( weptr->name, name );

    return weptr;
}

void copybytes( FILE *inhandle, FILE *outfp, ulong nbytes )
/*-------------------------------------------------------------------------*
 * Copies bytes from one handle to another.                                *
 *-------------------------------------------------------------------------*/
{
    uchar   *buf;
    size_t  maxread = 64000U;
    size_t  toread;

    _memstr = "copybytes() buffer";
    buf = (uchar*) memgetmax( &maxread );

    while( nbytes )
    {
        toread = maxread;
        if( toread > nbytes )
            toread = (size_t) nbytes;
        fread( buf, toread, 1, inhandle );
        if( fwrite( buf, toread, 1, outfp ) < 1 )
            error( "copybytes() failed on write" );
        nbytes -= toread;
    }

    memfree( buf );
}

void myfwrite( void *buf, size_t size, size_t n, FILE *fp )
{
    if( fwrite( buf, size, n, fp ) != n )
        _writeerr = 1;
}

bool UpdateScriptData( bool );
void DrawScreen();

void RunNodeBuilder()
{
    char szRunBuf[128];

    setfilertparms();
    strcpy( szRunBuf, pgdCurGame->szNodes );
    rt_replace( szRunBuf );
    DosCommand( szRunBuf );
}

int WadSave( const char *pszFile, const char *pszMapTag, int iMode )
{
    short       i, hasnodes = 0, tmpint, nostore = 0;
    NODEPTR     ndp;
    SIDEDEF     sdef;
    SIDEDEFM    *sdp;
    WAD         *new_wptr;
    WAD         *copy_wptr;
    WADENTRY    *weptr;
    char        tmpname[128], tmpbuf[128];
    FILE        *fp, *copyfp;
    GWINDOW     *statwnp;
    bool        fGfxMode = TRUE;
    bool        fComplete = (iMode > wsDataOnly);
    extern      short loadedvcount;
    extern      VERTEX *lvlist;
    int         iRvl = TRUE;

    _writeerr = 0;

    if( fHexen )
        UpdateScriptData( !fComplete );

    if( fComplete )
    {
        gr_vidmode( MODE_TEXT );
        fGfxMode = FALSE;

        if( !isnodeexist() )
        {
            map_purge( MODE_ALL );  // building nodes, so purge stuff
            hasnodes = FALSE;
        }
    }
    else
    {
        hasnodes = isnodeexist();
    }

    if( !hasnodes )
    {
        if( lvlist )
        {
            memfree( lvlist );
            lvlist = NULL;
        }
        loadedvcount = 0;
    }

    /* Create the new file */
    fnchop( tmpname, (uchar*) pszFile, FNC_DRIVE | FNC_DIRECTORY );
    if( lastchar( tmpname ) != '\\' )
        strcat( tmpname, "\\" );
    strcat( tmpname, "$TMP.WAD" );

    /* processing:
     * - if the .wad we're saving to is a new wad, nothing happens in this step.
     * - if the .wad we're saving to is the currently open .wad,
     *   copy its directory into the new .wad and save all the entries as we go.
     * - if the .wad we're saving to is an existing .wad, open it in a
     *   temporary pointer and do the above. */
    if( access( pszFile, 0 ) != -1 )
    {
        copy_wptr = pwdCurWad;

        if( !samefile( pwdCurWad->fn, (uchar*) pszFile ) )
            copy_wptr = wad_open( (char*) pszFile, WL_COMPLETE );
        else   /* reload the current .wad in its entirety */
            wad_reload( pwdCurWad, WL_COMPLETE );
    }
    else
        copy_wptr = NULL;

    new_wptr = wad_new( tmpname );

    fp = fopen( tmpname, "wb" );
    myfwrite( &(new_wptr->hdr), sizeof( WADHEADER ), 1, fp );

    if( fGfxMode )
    {
        sprintf( tmpbuf, "Saving %s", pszFile );
        statwnp = MAKESTATUSWINDOW( tmpbuf, 0 );
        wn_disp( statwnp );
    }

    /* copy anything? */
    if( copy_wptr )
    {
        if( fGfxMode )
        {
            wn_statstr( statwnp, "Copying old file..." );
        }
        else
        {
            printf( "Copying old file: " );
        }

        tmpint = copy_wptr->hdr.nentry;

        if( wad_find( copy_wptr, szCurMapTag ) )
            tmpint -= 10;

        i = 0;
        nostore = 0;

        copyfp = fopen( copy_wptr->fn, "rb" );

        /* copy everything except the current level tag and its data */
        ndp = NodeFirst( copy_wptr->dir_list );

        while( ndp != copy_wptr->dir_list )
        {
            if( fGfxMode )
            {
                wn_stat( statwnp, i++, tmpint );
            }
            else
            {
                printf( "%02d%c\b\b\b", i++ * 100 / tmpint, '%' );
            }
            weptr = (WADENTRY*) ndp->data;

            if( !wncmp( weptr->name, (char*) pszMapTag ) )
                nostore = nMapEntryCount;

            if( !nostore )
            {
                wad_adde( new_wptr, fp, weptr->name, weptr->length );
                fseek( copyfp, weptr->offset, SEEK_SET );
                copybytes( copyfp, fp, weptr->length );
            }
            else
                --nostore;

            ndp = NodeNext( ndp );
        }

        if( copy_wptr != pwdCurWad )   /* heh heh heh heh heh heh heh */
            wad_close( copy_wptr );
        fclose( copyfp );

        if( !fGfxMode )
            printf( "100%c\n", '%' );
    }

    /* map identifier */
    extern char szGameVersion[];

    WADENTRY *pweMapTag = wad_adde( new_wptr, fp, (char*) pszMapTag,
                    sizeof MapOptions + strlen( szGameVersion ) + 1 );
    myfwrite( szGameVersion, strlen( szGameVersion ) + 1, 1, fp );
    myfwrite( &MapOptions, sizeof MapOptions, 1, fp );

    if( fGfxMode )
    {
        wn_stat( statwnp, 1, nMapEntryCount );
    }
    else
    {
        printf( "\nSaving to %s: [          %s]\b\b\b\b\b\b\b\b\b\b\b%s", pszFile,
                fHexen?" ":"", fHexen?"\b":"" );
    }

    /* Write out things */
    if( fGfxMode )
    {
        wn_statstr( statwnp, "Saving things..." );
    }
    else
    {
        printf( "*" );
    }

    if( fHexen )
    {
        wad_adde( new_wptr, fp, "THINGS", (long) nthing * (long) sizeof( THING ) );
        myfwrite( thinglist, sizeof( THING ), nthing, fp );
    }
    else
    {
        wad_adde( new_wptr, fp, "THINGS", (long) nthing * (long) sizeof( DoomThing ) );

        DoomThing dt;
        THING* tptr = thinglist;

        // convert hexen things to doom things
        for( i = 0; i < nthing; i++ )
        {
            dt.x = tptr->x;
            dt.y = tptr->y;
            dt.attribs = tptr->attribs;
            dt.type = tptr->type;
            dt.angle = tptr->angle;

            myfwrite( &dt, sizeof( DoomThing ), 1, fp );

            tptr++;
        }
    }

    if( fGfxMode )
    {
        wn_stat( statwnp, 2, nMapEntryCount );
    }

    /* Write out linedefs */
    if( fGfxMode )
    {
        wn_statstr( statwnp, "Saving linedefs..." );
    }
    else
    {
        printf( "*" );
    }

    if( fHexen )
    {
        wad_adde( new_wptr, fp, "LINEDEFS", (long)nline * (long)sizeof( LINEDEF ) );
        myfwrite( linelist, sizeof( LINEDEF ), nline, fp );
    }
    else
    {
        wad_adde( new_wptr, fp, "LINEDEFS", (long)nline * (long)sizeof( DoomLine ) );
        DoomLine dl;
        LINEDEF *ldptr = linelist;

        // convert hexen lines to doom lines
        for( i = 0; i < nline; i++ )
        {
            memcpy( &dl, ldptr, 6 );
            dl.sectag = ldptr->sectag;
            dl.special = ldptr->special;
            dl.rsdef = ldptr->rsdef;
            dl.lsdef = ldptr->lsdef;
            myfwrite( &dl, sizeof( DoomLine ), 1, fp );

            ldptr++;
        }
    }

    if( fGfxMode )
    {
        wn_stat( statwnp, 3, nMapEntryCount );
    }

    /* Write out sidedefs */
    if( fGfxMode )
    {
        wn_statstr( statwnp, "Saving sidedefs..." );
    }
    else
    {
        printf( "*" );
    }
    wad_adde( new_wptr, fp, "SIDEDEFS", (long)nside * (long)sizeof( SIDEDEF ) );
    for( i = 0, sdp = sidelist; i < nside; i++, sdp++ )
    {
        sdef.wxofs = sdp->wxofs;
        sdef.wyofs = sdp->wyofs;
        sdef.sector = sdp->sector;

        wncpy( sdef.utxname, texlist[sdp->utxidx].name );
        wncpy( sdef.ltxname, texlist[sdp->ltxidx].name );
        wncpy( sdef.ftxname, texlist[sdp->ftxidx].name );

        myfwrite( &sdef, sizeof( SIDEDEF ), 1, fp );
    }
    if( fGfxMode )
    {
        wn_stat( statwnp, 4, nMapEntryCount );
    }

    /* Write out vertices */
    if( fGfxMode )
    {
        wn_statstr( statwnp, "Saving vertexes..." );
    }
    else
    {
        printf( "*" );
    }
    if( lvlist == NULL )
    {
        wad_adde( new_wptr, fp, "VERTEXES", (long) nvertex * (long) sizeof( VERTEX ) );
        myfwrite( vlist, sizeof( VERTEX ), nvertex, fp );
    }
    else
    {
        wad_adde( new_wptr, fp, "VERTEXES", (long) loadedvcount * (long) sizeof( VERTEX ) );
        myfwrite( lvlist, sizeof( VERTEX ), loadedvcount, fp );
    }
    if( fGfxMode )
    {
        wn_stat( statwnp, 5, nMapEntryCount );
    }

    if( !hasnodes )
    {
        wad_adde( new_wptr, fp, "SEGS", 0 );
        wad_adde( new_wptr, fp, "SSECTORS", 0 );
        wad_adde( new_wptr, fp, "NODES", 0 );

        if( !fGfxMode )
            printf( "***" );
    }
    else
    {
        if( fGfxMode )
        {
            wn_statstr( statwnp, "Saving segs..." );
        }
        else
        {
            printf( "*" );
        }
        wad_adde( new_wptr, fp, "SEGS", (long) num_psegs * (long) sizeof( Pseg ) );
        myfwrite( psegs, sizeof( Pseg ), num_psegs, fp );
        if( fGfxMode )
        {
            wn_stat( statwnp, 6, nMapEntryCount );
        }

        if( fGfxMode )
        {
            wn_statstr( statwnp, "Saving sub-sectors..." );
        }
        else
        {
            printf( "*" );
        }
        wad_adde( new_wptr, fp, "SSECTORS", (long) num_ssectors * (long) sizeof( SSector ) );
        myfwrite( ssectors, sizeof( SSector ), num_ssectors, fp );
        if( fGfxMode )
        {
            wn_stat( statwnp, 7, nMapEntryCount );
        }

        if( fGfxMode )
        {
            wn_statstr( statwnp, "Saving nodes..." );
        }
        else
        {
            printf( "*" );
        }
        wad_adde( new_wptr, fp, "NODES", (long) num_pnodes * (long) sizeof( Pnode ) );
        myfwrite( pnodes, sizeof( Pnode ), num_pnodes, fp );
        if( fGfxMode )
        {
            wn_stat( statwnp, 8, nMapEntryCount );
        }
    }

    /* Write out sectors */
    if( fGfxMode )
    {
        wn_statstr( statwnp, "Saving sectors..." );
    }
    else
    {
        printf( "*" );
    }
    wad_adde( new_wptr, fp, "SECTORS", (long) nsector *
              (long) sizeof( SECTOR ) );
    myfwrite( sectorlist, sizeof( SECTOR ), nsector, fp );
    if( fGfxMode )
    {
        wn_stat( statwnp, 9, nMapEntryCount );
    }

    if( !hasnodes )
    {
        wad_adde( new_wptr, fp, "REJECT", 0 );
        wad_adde( new_wptr, fp, "BLOCKMAP", 0 );

        if( !fGfxMode )
            printf( "**" );
    }
    else
    {
        if( fGfxMode )
        {
            wn_statstr( statwnp, "Saving reject..." );
        }
        else
        {
            printf( "*" );
        }
        wad_adde( new_wptr, fp, "REJECT", reject_size );
        myfwrite( reject, reject_size, 1, fp );
        if( fGfxMode )
        {
            wn_stat( statwnp, 10, nMapEntryCount );
        }

        if( fGfxMode )
        {
            wn_statstr( statwnp, "Saving blockmap..." );
        }
        else
        {
            printf( "*" );
        }
        wad_adde( new_wptr, fp, "BLOCKMAP", (long) sizeof( Block ) +
                  blockptrs_size + blockmap_size );
        myfwrite( &blockhead, sizeof( Block ), 1, fp );
        myfwrite( blockptrs, blockptrs_size, 1, fp );
        myfwrite( blocklists, blockmap_size, 1, fp );
        if( fGfxMode )
        {
            wn_stat( statwnp, 11, nMapEntryCount );
        }
    }

    if( fHexen )
    {
        if( pBehavior )
        {
            wad_adde( new_wptr, fp, "BEHAVIOR", lBehaviorSize );
            myfwrite( pBehavior, lBehaviorSize, 1, fp );
        }
        else
        {
            wad_adde( new_wptr, fp, "BEHAVIOR", 16 );
            myfwrite( "ACS\0\b\0\0\0\0\0\0\0\0\0\0\0", 16, 1, fp );
        }

        if( !fGfxMode )
            printf( "*" );
    }

#if 0
    pweMapTag->offset = ftell( fp );
    myfwrite( &MapOptions, sizeof MapOptions, 1, fp );
#endif

    printf( "\n" );

    new_wptr->hdr.dirstart = ftell( fp );

    ndp = NodeFirst( new_wptr->dir_list );

    while( ndp != new_wptr->dir_list )
    {
        myfwrite( ndp->data, sizeof( WADENTRY ), 1, fp );
        ndp = NodeNext( ndp );
    }

    fseek( fp, 0L, SEEK_SET );
    myfwrite( &new_wptr->hdr, sizeof( WADHEADER ), 1, fp );
    fclose( fp );

    if( fGfxMode )
    {
        wn_undisp( statwnp );
        wn_free( statwnp );
    }

    if( pszFile[1] == ':' )
        sprintf( tmpbuf, "%c:\\$TMP2.WAD", (char) pszFile[0] );
    else
        strcpy( tmpbuf, "$TMP2.WAD" );

    /* shouldn't be there, but. */
    remove( tmpbuf );

    if( !_writeerr )
        rename( pszFile, tmpbuf );

    if( _writeerr || rename( tmpname, pszFile ) )
    {
        rename( tmpbuf, pszFile );
        if( fGfxMode )
        {
            message_box( MB_OK, "Warning!",
                        "I can't %s to the file '%s'.                 \n"
                        "Your work is UNSAVED.  Please choose a different\n"
                        "filename and try again.", _writeerr ? "write" : "rename", pszFile );
        }
        else
        {
            printf( "** WARNING: I can't %s the file '%s'.\n"
                    "   Your work is UNSAVED.  Please chooce a different\n"
                    "   filename and save again.", _writeerr ? "write to" : "rename", pszFile );
        }
        wad_close( new_wptr );
        iRvl = FALSE;
        goto Done;
    }

    hasnodes = FALSE;   /* pnodes != NULL && nMapChanges != 1 */

    wad_close( pwdCurWad );
    strcpy( new_wptr->fn, pszFile ); /* right now it's "tmp.wad" or whatever */
    _fullpath( new_wptr->fn, pszFile, 128 );
    pwdCurWad = new_wptr;
    checktexupdate( pwdCurWad );
    strcpy( szCurMapTag, pszMapTag );

    if( config.tog_backup )
    {
        /* backup the old file */
        strcpy( tmpname, pszFile );
        strchangestr( tmpname, "WAD", "BAK" );
        remove( tmpname );
        rename( tmpbuf, tmpname );
    }
    else
        remove( tmpbuf );       /* or.. remove the old file. */

    if( fGfxMode )
    {
        SetStatusMsg( "%s saved.", pszFile );
        UpdateMessageBar();
    }

    // do node builder

    if( fComplete )
    {
        RunNodeBuilder();
    }

Done:
    if( iRvl == FALSE || (!fGfxMode && iMode != wsTestMap) )
    {
        // restore graphics mode
        gr_vidmode( MODE_GFX );
        mouse_init();
        DrawScreen();
        mouse_show();
        fGfxMode = TRUE;

        if( fComplete )
        {
            // reload map because nodes have changed
            wad_reload( pwdCurWad, WL_MAPONLY );
            map_load();
        }
    }

    return iRvl;
}

void closecurwad( short newgame )
/*-------------------------------------------------------------------------*
 * Closes the wad with the current map in it - if the wad is a texture
 * wad, asks the user if they want to remove it from the list.
 *-------------------------------------------------------------------------*/
{
    short   texnum;

    if( pwdCurWad && (newgame == -1 || newgame == iCurGame) )
    {
        if( (texnum = istexwad( pwdCurWad->fn )) > 0 )
        {
            if( message_box( MB_YESNO, st_dcksays,
                "%s contains texture information.  Do you want to\n"
                "keep this information in memory (pick Yes) or unload\n"
                "it now (pick No)?",
                pwdCurWad->fn ) == MB_NO )
                removetexwad( texnum );
        }
        wad_close( pwdCurWad );
    }

    pwdCurWad = NULL;
}
