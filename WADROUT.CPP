/*-------------------------------------------------------------------------*
 * .WAD routines                                                           *
 *-------------------------------------------------------------------------*/

typedef char    WADNAME[8];

/* Header at the beginning of a WAD file                                   */
typedef struct
{
    uchar   sig[4];     /* Signature of wad [IWAD/PWAD] */
    ulong   nentry;     /* Number of entries in this WAD's directory */
    ulong   dirstart;   /* The offset where the directory begins */

} WADHEADER;

/* Each entry in the WAD's directory is as follows...                      */
typedef struct
{
    ulong   offset;     /* Offset of the data */
    ulong   length;     /* Data length */
    WADNAME name;       /* Name of the data */

} WADENTRY;

typedef struct
{
    char        fn[128];    /* filename */
    WADENTRY    *dir;       /* directory list */
    short         ndir;       /* number of directory entries */
    short         maxdir;     /* maximum number of directory entries (sizeof dir) */

} WAD;

#define DIRMAX  4000

WAD *wad_load( uchar *fn )
/*-------------------------------------------------------------------------*
 * Loads a .WAD.                                                           *
 *-------------------------------------------------------------------------*/
{
    FILE    *infp = fopen( fn, "rb" );
    WAD     *wad;
    WADENTRY    *we;
    WADHEADER   hdr;
    short     i;

    if( !infp )   /* can't open */
        return NULL;    /* error */

    wad = malloc( sizeof( WAD ) );
    strcpy( wad->fn, fn );
    wad->maxdir = DIRMAX;
    wad->dir = malloc( sizeof( WADENTRY ) * wad->maxdir );
    wad->ndir = 0;      /* no entries yet */

    /* read in the header */
    fread( &hdr, sizeof hdr, 1, infp );
    fseek( infp, hdr.dirstart, SEEK_SET );

    for( i = 0, we = wad->dir; i < hdr.nentry; i++, we++ )
        fread( we, sizeof( WADENTRY ), 1, infp );

    wad->ndir = hdr.nentry;

    /* Close the .wad */
    fclose( infp );

    return wad;
}

FILE *wad_e2h( WAD *wad, WADENTRY *we )
/*-------------------------------------------------------------------------*
 * Opens a .wad, seeks to the entry's offset, and returns the file handle. *
 *-------------------------------------------------------------------------*/
{
    FILE    *fp = fopen( wad->fn, "rb" );

    if( !fp )
        return NULL;

    fseek( fp, we->offset, SEEK_SET );

    return fp;
}

WADENTRY *wad_find( WAD *wad, uchar *entry )
/*-------------------------------------------------------------------------*
 * Finds the entry specified by .. guess!                                  *
 *-------------------------------------------------------------------------*/
{
    WADENTRY    *we = wad->dir;
    short         i, ndir = wad->ndir, eslen = strlen( entry );

    if( eslen > 8 ) eslen = 8;

    for( i = 0; i < ndir; i++, we++ )
    {
        if( !memicmp( entry, we->name, eslen ) )
            return we;
    }

    return NULL;
}

void *wad_loadentry( WAD *wad, WADENTRY *we )
{
    FILE *fp = fopen( wad->fn, "rb" );
    char *buf;

    if( !fp )
        return NULL;

    fseek( fp, we->offset, SEEK_SET );
    buf = malloc( we->length );

    if( !buf )
    {
        fclose( fp );
        return NULL;
    }

    fread( buf, we->length, 1, fp );

    fclose( fp );

    return buf;
}

void wad_close( WAD *wad )
/*-------------------------------------------------------------------------*
 * Free a .wad                                                             *
 *-------------------------------------------------------------------------*/
{
    /* free the list */
    free( wad->dir );
    free( wad );
}

