/*-------------------------------------------------------------------------*
 * TEXTURE.C:                                                              *
 * Functions for selecting / displaying textures                           *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"

// #pragma option -O2
#pragma inline +memset +memcpy +memcmp

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#define MaxColSize      2048     /* Maximum column size */
#define MaxColCount     1025
#define MaxPatchBytes   0x20000
#define MaxPicBufX      256
#define MaxPicBufY      128
#define MaxTexX         128

#define texWidth(t)     ((t)->width <= MaxTexX ? (t)->width : MaxTexX)
#define texHeight(t)    ((t)->height <= MaxPicBufY ? (t)->height : MaxPicBufY)

static  ulong   colofs[MaxColCount];        /* for the current picture */
static  uchar   floorbuf[4096]; /* for floors/ceilings (64x64) */
static  uchar   *patchbuf;

uchar   sp2pb_reterr;       /* return on error in spr2picbuf()? */

/* picture rectangle boundaries */
SRECT    prb;

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static int _ts_func( const void *a, const void *b )
{
    return memcmp( a, b, sizeof( TEXTURE ) );
}

static int _wes_func( const void *a, const void *b )
{
    WADENTRY *_a = (WADENTRY *) a, *_b = (WADENTRY *) b;
    return memcmp( _a->name, _b->name, sizeof( WADNAME ) );
}

static short texstrlen( char *buf )
{
    short i = 0;

    while( buf[i] != 32 && buf[i] != 0 )
        ++i;

    return i;
}

short texture_toint( WADNAME texname )
{
    short i;

    for( i = 0; i < ntex; i++ )
    {
        if( !wncmp( texlist[i].name, texname ) )
            return i;
    }

    return -1;
}

short texture_toint_ic( WADNAME texname )
{
    short i;

    for( i = 0; i < ntex; i++ )
    {
        if( !wncmpi( texlist[i].name, texname ) )
            return i;
    }

    return -1;
}

char *texture_tostr( char *str, short texno )
{
    if( texno == -1 )
    {
        memset( str, 0, sizeof( WADNAME ) + 1 );
        return str;
    }

    str[8] = 0;
    memcpy( str, texlist[texno].name, sizeof( WADNAME ) );

    return str;
}

short floor_toint( char *floorname )
{
    short i;

    for( i = 0; i < nfloor; i++ )
    {
        if( !wncmp( floorlist[i].name, floorname ) )
            return i;
    }

    return -1;       /* '-' if not found.. */
}

img_hdr     *pbhdr;
char        *picbuf;
unsigned    pblen;

void mkpicbuf( short width, short height )
{
    if( picbuf == NULL )
        picbuf = (char*) memget( pblen = imagesize( 0, 0, MaxPicBufX+1, MaxPicBufY+1 ) );
    else
        memset( picbuf, 0, pblen );

    pbhdr = (img_hdr*) picbuf;

    pbhdr->width = pbhdr->w2 = width;
    pbhdr->height = pbhdr->h2 = height;
    pbhdr->notused = 0;
    pbhdr->widthpadded = (width+2) - ((width+2) % 2);
    pbhdr->depth = 8;
    pbhdr->planes = 1;
}

#if 0
short spr2picbuf( short x, short y, short handle, long offset, short center,
        int iMagnify )  // iMagnify: 100 is 1x, 200 is 2x, etc.
/*-------------------------------------------------------------------------*
 * Converts a patch picture at file:handle:offset into the picture buffer,
 * centering it if necessary.
 *-------------------------------------------------------------------------*/
{
    short   curx, cury, i, realy;
    uchar   npixel;
    uchar   *colptr, *pbptr, *pFirstExpCol;
    short   widthpad = pbhdr->widthpadded;
    static  ulong lastpofs = -1;
    static  PIC_HDR header;

    int     nPixInCol;
    int     iDoExpand, iDoContract, iDoContract2, iDoExpand2;
    int     iExpand, iContract;

    iExpand = iMagnify / 100;
    iContract = 100 / iMagnify;

    if( iMagnify > 100 )
        iMagnify = 1;
    else if( iMagnify < 100 )
        iMagnify = -1;
    else
        iMagnify = 0;

    if( offset == -1 )  /* can't display this */
        return 1;

    if( !patchbuf ) /* 256x128 is pretty big eh ? */
        patchbuf = (uchar*) memget( MaxPatchBytes ); /* well whatever. */

    if( lastpofs != offset )
    {
        /* Jump to the resource */
        lseek( handle, offset, SEEK_SET );

        /* Read in the header */
        _read( handle, &header, sizeof header );

        if( header.width < 0 || header.width > MaxColCount )
        {
            if( sp2pb_reterr )  return 0;
            error( "spr2picbuf: MaxColCount exceeded by %d", header.width-MaxColCount );
        }

        /* Read in the offsets */
        _read( handle, colofs, header.width * 4 );
        colofs[header.width] = colofs[header.width-1] + MaxColSize;

        if( colofs[header.width] > MaxPatchBytes )
        {
            if( sp2pb_reterr )  return 0;
            error( "spr2picbuf: MaxPatchBytes exceeded by %d", colofs[header.width]-MaxPatchBytes );
        }

        lseek( handle, offset, SEEK_SET );
        _read( handle, patchbuf, colofs[header.width] );
    }

    if( iMagnify == 1 )
    {
        header.height *= iExpand;
        header.width *= iExpand;
    }
    else if( iMagnify == -1 )
    {
        header.height /= iContract;
        header.width /= iContract;
    }

    if( center )
    {
        y = prb.top + (prb.bottom - prb.top) / 2 - header.height / 2;
        x = prb.left + (prb.right - prb.left) / 2 - header.width / 2;
    }

    curx = x;

    lastpofs = offset;

    iDoContract = iContract;

    /* read in the data and display it post by post */
    for( i = 0; i < header.width && curx < prb.right; i++ )
    {
        if( iDoContract > 1 )
        {
            --iDoContract;
            continue;
        }

        iDoContract = iContract;
        iDoExpand = iExpand;

        while( iDoExpand )
        {
            if( iDoExpand < iExpand )
            {
                int i = nPixInCol;

                pbptr = picbuf+sizeof(img_hdr)+(y*widthpad)+curx;

                // copy last column over ..
                while( i-- )
                {
                    *pbptr = *pFirstExpCol;
                    pbptr += widthpad;
                    pFirstExpCol += widthpad;
                }

                goto _NextColumn;
            }
            else
                pFirstExpCol = picbuf+sizeof(img_hdr)+(y*widthpad)+curx;

            /* pointer to column = patch buffer[ column offset [ cur_column ] ]*/
            colptr = patchbuf + colofs[i];

            if( curx < prb.left )      /* No point in doing anything here.. */
                goto _NextColumn;

            nPixInCol = y;

            /* Display this column */
            iDoContract2 = iContract;

            while( 1 )
            {
                if( *colptr == 0xFF )       /* End of column? */
                    break;

                cury = *colptr++;       /* Get y start of column */

                realy = y + cury;       /* "realy" is the screen y-pos */
                pbptr = picbuf+sizeof(img_hdr)+(realy*widthpad)+curx;

                npixel = *colptr++;     /* Get number of pixels */
                ++colptr;               /* first pixel is junk */

                while( realy < prb.bottom && npixel )
                {
                    iDoExpand2 = iExpand;

                    if( iExpand > 1 )
                    {
                        while( iDoExpand2-- )
                        {
                            if( realy >= prb.top )  /* write pixel in column format (row 1, row 2, etc) */
                                *pbptr = *colptr;

                            pbptr += widthpad;
                            realy++;
                        }
                    }
                    else    // no expansion
                    {
                        // check for contraction
                        if( iDoContract2 > 1 )
                        {
                            --iDoContract2;
                        }
                        else
                        {
                            iDoContract2 = iDoContract;

                            if( realy >= prb.top )  /* write pixel in column format (row 1, row 2, etc) */
                                *pbptr = *colptr;

                            pbptr += widthpad;
                            realy++;
                        }
                    }

                    ++colptr;
                    npixel--;
                }

                colptr += npixel;       /* any pixels left over? */
                ++colptr;               /* last pixel junk */
            }

            nPixInCol = realy-nPixInCol;

_NextColumn:
            --iDoExpand;
            ++curx;
        }
    }

    return 1;
}
#else
short spr2picbuf( short x, short y, short handle, long offset, short center,
        int iMagnify )  // iMagnify: 100 is 1x, 200 is 2x, etc.
/*-------------------------------------------------------------------------*
 * Converts a patch picture at file:handle:offset into the picture buffer,
 * centering it if necessary.
 *-------------------------------------------------------------------------*/
{
    short   curx, cury, i, realy;
    uchar   npixel;
    uchar   *colptr, *pbptr;
    short   widthpad = pbhdr->widthpadded;
    static  ulong lastpofs = -1;
    static  PIC_HDR header;

    if( offset == -1 )  /* can't display this */
        return 1;

    if( !patchbuf ) /* 256x128 is pretty big eh ? */
        patchbuf = (uchar*) memget( MaxPatchBytes ); /* well whatever. */

    if( lastpofs != offset )
    {
        /* Jump to the resource */
        lseek( handle, offset, SEEK_SET );

        /* Read in the header */
        _read( handle, &header, sizeof header );

        if( header.width < 0 || header.width > MaxColCount )
        {
            if( sp2pb_reterr )  return 0;
            error( "spr2picbuf: MaxColCount exceeded by %d", header.width-MaxColCount );
        }

        /* Read in the offsets */
        _read( handle, colofs, header.width * 4 );
        colofs[header.width] = colofs[header.width-1] + MaxColSize;

        if( colofs[header.width] > MaxPatchBytes )
        {
            if( sp2pb_reterr )  return 0;
            error( "spr2picbuf: MaxPatchBytes exceeded by %d", colofs[header.width]-MaxPatchBytes );
        }

        lseek( handle, offset, SEEK_SET );
        _read( handle, patchbuf, colofs[header.width] );
    }

    if( center )
    {
        y = prb.top + (prb.bottom - prb.top) / 2 - header.height / 2;
        x = prb.left + (prb.right - prb.left) / 2 - header.width / 2;
    }

    curx = x;

    lastpofs = offset;

    /* read in the data and display it post by post */
    for( i = 0; i < header.width && curx < prb.right; i++, curx++ )
    {
        /* pointer to column = patch buffer[ column offset [ cur_column ] ]*/
        colptr = patchbuf + colofs[i];

        if( curx < prb.left )      /* No point in doing anything here.. */
            continue;

        /* Display this column */
        while( 1 )
        {
            if( *colptr == 0xFF )       /* End of column? */
                break;

            cury = *colptr++;       /* Get y start of column */

            npixel = *colptr++;     /* Get number of pixels */
            ++colptr;               /* first pixel is junk */

            realy = y + cury;       /* "realy" is the screen y-pos */
            pbptr = picbuf+sizeof(img_hdr)+(realy*widthpad)+curx;

            while( realy < prb.bottom && npixel )
            {
                if( realy >= prb.top )  /* write pixel in column format (row 1, row 2, etc) */
                    *pbptr = *colptr;

                pbptr += widthpad;
                ++colptr;
                npixel--;
                realy++;
            }

            colptr += npixel;       /* any pixels left over? */
            ++colptr;               /* last pixel junk */
        }
    }

    return 1;
}

#endif

void we_disppic( short x, short y, short xw, short yw, WAD* wptr, WADENTRY *weptr )
/*-------------------------------------------------------------------------*
 * Displays a picture from the iwad.
 *-------------------------------------------------------------------------*/
{
    short handle = open( wptr->fn, O_BINARY | O_RDONLY );

    prb.left = 0;
    prb.right = xw;
    prb.top = 0;
    prb.bottom = yw;

    mkpicbuf( xw, yw );

    /* centered */
    spr2picbuf( 0, 0, handle, weptr->offset, 1, 100 );

    putimage( x, y, picbuf, COPY_PUT );

    close( handle );
}

void texture_disp( short x, short y, short xw, short yw,
                   short xofs, short yofs,
                   TEXTURE *t, short xtile )
/*-------------------------------------------------------------------------*
 * Displays a texture, man
 *-------------------------------------------------------------------------*/
{
    TEXDESCRIP  *tdp;
    PNRef       *pnp;
    short       i, handle = -1, lasttexwad = -1, xtpos = 0, ytpos = 0;

    /* if there's an x or y offset, they come from a sidedef.  since the
     * picture buffer is put together with an offset of zero:
     *
     * on positive offsets: the ?tpos coordinate has to be the negative of
     * the offset.
     *
     * on negative offsets: the ?tpos coordinate has to be the texture's
     * dimension minus the offset.
     */

    if( xofs < 0 )
        xtpos = -(t->width + xofs);
    if( yofs < 0 )
        ytpos = -(t->height + yofs);

    if( xofs > 0 )
        xtpos = -xofs;
    if( yofs > 0 )
        ytpos = -yofs;

    if( xw > MaxPicBufX )
        xw = MaxPicBufX;
    if( yw > MaxPicBufY )
        yw = MaxPicBufY;

    if( !xtile )
    {
        if( xw > t->width )
            xw = t->width;
    }

    prb.left = x;
    prb.top = y;
    prb.right = x + xw;
    prb.bottom = y + yw;

    if( !(t->ndescrip) )
    {
        /* display a big X */
        gr_setlinestyle( SLS_SOLID, 3 );
        gr_line( prb.left + 16, prb.top + 16, prb.right-16, prb.bottom-16, C_RED );
        gr_line( prb.right - 16, prb.top + 16, prb.left + 16, prb.bottom-16, C_RED );
        gr_setlinestyle( SLS_SOLID, 1 );
        return;
    }

    DEBUG_INIT( "Displaying texture %s (#%d)", WN2Str( t->name ), (t-texlist)/sizeof(TEXTURE) );

    prb.left = prb.top = 0;
    prb.right = xw;
    prb.bottom = yw;

    mkpicbuf( xw, yw );

_redisp:
    for( i = 0, tdp = t->dlist; i < t->ndescrip; i++, tdp++ )
    {
        pnp = &pgdCurGame->pnames[tdp->pentry];

        if( handle != -1 && lasttexwad != pnp->wadnumber )
            z_close( handle ), handle = -1;

        if( handle == -1 )
        {
            handle = z_open( pgdCurGame->texwads[pnp->wadnumber], O_BINARY | O_RDONLY );
            lasttexwad = pnp->wadnumber;
        }

        if( xtpos + tdp->xofs > xw )
            continue;
        if( ytpos + tdp->yofs > yw )
            continue;

        spr2picbuf( xtpos + tdp->xofs, ytpos + tdp->yofs, handle, pnp->offset, 0, 200 );
    }

    xtpos += texWidth( t );

    if( xtile && xtpos < xw )
        goto _redisp;

    putimage( x, y, picbuf, COPY_PUT );

    if( handle != -1 )
        z_close( handle );

    DEBUG_EXIT();
}

void _fdisp( short x, short y, char *buf )
{
    short   i, i2;
    char    *pbptr;

    mkpicbuf( 64, 64 );

    for( i = 0; i < 64; i++ )
    {
        pbptr = picbuf + sizeof( img_hdr ) + (i * pbhdr->widthpadded);
        for( i2 = 0; i2 < 64; i2++ )
            *pbptr++ = *buf++;
    }

    putimage( x, y, picbuf, COPY_PUT );
}

void floor_disp( short x, short y, short floorno )
{
    WADENTRY    *we;
    short       handle;
    static int  iLastFloor = -1;

    if( floorno == -1 )
    {
        gr_rect( x, y, x + 63, y + 63, C_BLACK );
        setlinestyle( SOLID_LINE, 0, 2 );
        setcolor( C_RED );
        line( x + 4, y + 4, x + 60, y + 60 );
        line( x + 60, y + 4, x + 4, y + 60 );
        setlinestyle( SOLID_LINE, 0, 1 );
        return;
    }

    if( iLastFloor != floorno )
    {
        handle = z_open( pwdIWAD->fn, O_BINARY | O_RDONLY );
        we = &floorlist[floorno];
        lseek( handle, we->offset, SEEK_SET );
        z_read( handle, floorbuf, 4096U );
        z_close( handle );
    }

    _fdisp( x, y, floorbuf );

    iLastFloor = floorno;
}

static int ffFinishSearch()
{
    return daExitSave;
}

short texfind_dlg( char *str )
{
    strupr( str );

    Dialog *pDialog = new Dialog;
    pDialog->Create( Point( Centered, Centered ), Size( 56, 32 ), "Search for" );
    fldString* pfldStr = new fldString( pDialog, Point( 4, 4 ), 48, ftString,
        "Search for:", csUpper, 8 );
    pfldStr->SetTransferBuf( (PTR) str );
    pfldStr->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) ffFinishSearch );

    int iRvl = pDialog->Process();
    delete pDialog;

    return (iRvl == daExitSave);
}

short texture_find( short startpos, short direc, uchar *texname )
/*-------------------------------------------------------------------------*
 * Searches for a texture in the list from "startpos" using "direc"..      *
 * direc is 1 or -1, for forward or reverse.  uses 'texname' as the        *
 * search string and does an inclusive, not absolute, search.  returns     *
 * the found texture's index number or BEEPS and returns 'startpos' if     *
 * not found.                                                              *
 *-------------------------------------------------------------------------*/
{
    short     i;
    uchar    nbuf[9];

    if( !texname[0] )
        return startpos;

    nbuf[8] = 0;

    for( i = startpos+direc; i >= 0 && i < ntex; i += direc )
    {
        memcpy( nbuf, texlist[i].name, 8 );

        if( strstr( (char*) nbuf, (char*) texname ) != NULL )
            return i;
    }

    sound( 100 );
    delay( 100 );
    nosound();

    return startpos;
}

short floor_find( short startpos, short direc, uchar *floorname )
/*-------------------------------------------------------------------------*
 * See texture_find().                                                     *
 *-------------------------------------------------------------------------*/
{
    short     i;
    uchar    nbuf[9];

    if( !floorname[0] )
        return startpos;

    nbuf[8] = 0;

    for( i = startpos+direc; i >= 0 && i < nfloor; i += direc )
    {
        memcpy( nbuf, floorlist[i].name, 8 );

        if( strstr( (char*) nbuf, (char*) floorname ) != NULL )
            return i;
    }

    sound( 100 );
    delay( 100 );
    nosound();

    return startpos;
}

/*-------------------------------------------------------------------------*
 *
 * Texture browser functions
 *
 *-------------------------------------------------------------------------*/

// stub
short texdlg( short curtex, short dofloor ) { return curtex; }

class fldTextureList : public fldRegion
{
public:
    fldTextureList( Dialog* pDialog, Rect& rcRect, int _iMaxTextures,
                stText* _ptxtInfo, bool _fFloors ) :
      fldRegion( pDialog, rcRect, 0 ),
        iCurTex(0), iTopTex(0), iLastCur(-1), iLastTop(-1),
        iMaxTextures(_iMaxTextures)
    {
        ptxtInfo = _ptxtInfo;
        fFloors = _fFloors;
        fNames = FALSE;
        iCaptionHeight = _YUnitsToPixels( YUPC );
        szSearch[0] = 0;

        SetDisplaySize( fFloors ? 64 : 128 );
    }

    void SetCurTexture( int iTexture );
    void SetDisplaySize( int iPixels = -1 );
    void SetNamesOnly( bool );
    inline int GetDisplayCount() { return iXCount * iYCount; }
    inline int GetCurTexture() { return iCurTex; }
    inline int GetDisplaySize()
        { return iXPixels; }
    inline bool GetNamesOnly()
        { return fNames; }
    inline bool GetMode()
        { return fFloors + 1; }

    void Draw();
    int Event( Event_t& );

private:
    Rect TexRects[512];
    bool fTexDisplayed[512];

    bool fFloors;
    bool fNames;
    int iLastDisplaySize;
    int iXCount;
    int iYCount;
    int iXPixels;
    int iYPixels;
    int iMaxTextures;
    int iCurTex;
    int iTopTex;
    int iLastCur;
    int iLastTop;
    int iCaptionHeight;
    int iXTexPadding;      // pixels
    int iYTexPadding;      // pixels
    stText* ptxtInfo;
    char szSearch[10];
};

void fldTextureList::SetNamesOnly( bool fNames )
{
    this->fNames = fNames;

    if( fNames )
    {
        iXPixels = 64;
        iYPixels = iCaptionHeight;

        SetDisplaySize();
    }
    else
        SetDisplaySize( iLastDisplaySize );
}

void fldTextureList::SetDisplaySize( int iPixels )
{
    if( iPixels != -1 )
    {
        fNames = FALSE;

        iXPixels = iPixels;
        iYPixels = iPixels;

        if( iYPixels > 128 )
            iYPixels = 128;

        if( iPixels >= 64 )
            iYPixels += iCaptionHeight;

        iLastDisplaySize = iXPixels;
    }

    Rect rcRect = GetRectInParent();
    rcRect.Inflate( -1, -1 );

    int iTotalHeight = rcRect.Height();

    if( fNames )
        iTotalHeight -= 136;

    iXCount = rcRect.Width() / (iXPixels + 2);
    iYCount = iTotalHeight / (iYPixels + 2);

    iXTexPadding = (rcRect.Width() - (iXCount * iXPixels)) / iXCount;
    iYTexPadding = (iTotalHeight - (iYCount * iYPixels)) / iYCount;

    iXTexPadding /= 2;
    iYTexPadding /= 2;

    if( iXTexPadding < 2 )
        iXTexPadding = 2;
    if( iYTexPadding < 2 )
        iYTexPadding = 2;

    iXCount = rcRect.Width() / (iXPixels + iXTexPadding*2);
    iYCount = iTotalHeight / (iYPixels + iYTexPadding*2);

    //iXCount = rcRect.Width() / (iXPixels + iTexPadding*2);
    //iYCount = rcRect.Height() / (iYPixels + iTexPadding*2);

    SetFirstDraw(); // force l'update
    SetCurTexture( iCurTex );
}

int fldTextureList::Event( Event_t& event )
{
    int iSearchDirec;

    switch( event.type )
    {
    case etKey:
        switch( event.iCode )
        {
        case '/':       // search
        case '\\':
            if( !szSearch[0] )
            {
        case KEY_ALTF:
                if( !texfind_dlg( szSearch ) )
                    break;
            }

            if( event.iCode == '\\' )
                iSearchDirec = -1;
            else
                iSearchDirec = 1;

            if( fFloors )
                SetCurTexture( floor_find( iCurTex, iSearchDirec, szSearch ) );
            else
                SetCurTexture( texture_find( iCurTex, iSearchDirec, szSearch ) );
            break;
        case '5':
        case 'N':
        case 'n':
            SetNamesOnly( !fNames );
            break;
        case '-':
            if( (iXPixels > 32 && !fFloors) ||
                (iXPixels > 64 && fFloors) )
                SetDisplaySize( iXPixels / 2 );
            break;
        case '+':
            if( (iXPixels < 256 && !fFloors) ||
                (iXPixels < 128 && fFloors) )
                SetDisplaySize( iXPixels * 2 );
            break;
        case KEY_LEFT:
            SetCurTexture( iCurTex-1 );
            break;
        case KEY_RIGHT:
            SetCurTexture( iCurTex+1 );
            break;
        case KEY_PGDN:
            SetCurTexture( iCurTex+GetDisplayCount() );
            break;
        case KEY_PGUP:
            SetCurTexture( iCurTex-GetDisplayCount() );
            break;
        case KEY_UP:
            SetCurTexture( iCurTex-iXCount );
            break;
        case KEY_DOWN:
            SetCurTexture( iCurTex+iXCount );
            break;
        case KEY_HOME:
            SetCurTexture( 0 );
            break;
        case KEY_END:
            SetCurTexture( iMaxTextures-1 );
            break;
        }
        break;
    case etMouse:
        // find out where the mouse event occured
        int iMouseTex = -1;
        int nDisplayed = GetDisplayCount();
        if( nDisplayed > iMaxTextures )
            nDisplayed = iMaxTextures;
        for( int i = 0; i < nDisplayed; i++ )
        {
            if( TexRects[i].IsPtInside( event.ptPoint ) )
                iMouseTex = i + iTopTex;
        }

        if( iMouseTex == -1 )
            break;

        switch( event.iCode )
        {
        case MOUSE_LUP:
        case MOUSE_MOVE:
            // select a new texture
            SetCurTexture( iMouseTex );
            break;
        case MOUSE_LDBLCLICK:
            // select this texture and exit
            SetCurTexture( iMouseTex );
            return daExitSave;
        }

        break;
    }

    return daContinue;
}

void fldTextureList::SetCurTexture( int iTexture )
{
    bool fDiff1 = abs( iTexture - iCurTex );
    int nDisplayed = GetDisplayCount();

    iCurTex = iTexture;

    if( iCurTex >= iMaxTextures )
        iCurTex = iMaxTextures - 1;
    if( iCurTex < 0 )
        iCurTex = 0;

    while( iCurTex >= iTopTex + nDisplayed )
        iTopTex += nDisplayed;

    while( iCurTex < iTopTex )
        iTopTex -= nDisplayed;  // modify this, maybe

    if( iTopTex + nDisplayed > iMaxTextures )
        iTopTex = iMaxTextures - nDisplayed;

    if( iTopTex < 0 )
        iTopTex = 0;

    OnDraw();
}

void fldTextureList::Draw()
{
    // draw the texture list, starting at the top, left of the region.
    // draw a maximum of 128 high, even if the texture is taller.
    // if the last top texture is the same as this top texture, unmark
    //   the last current texture and mark the new current texture.
    // if the last top texture is different from the new top texture,
    //   redraw the entire list.

    bool fDisplayOne = FALSE;

    Rect rcRect = GetRectInParent();
    Point ptListTopLeft = rcRect.TopLeft();
    rcRect.Inflate( -1, -1 );
    rcRect.Inflate( -iXTexPadding, -iYTexPadding );
    Point ptPoint = rcRect.TopLeft();

    if( !fTexDisplayed[iCurTex-iTopTex] && !fNames )
        fDisplayOne = TRUE;

    if( iLastTop != iTopTex || IsFirstDraw() )
    {
        memset( fTexDisplayed, 0, sizeof fTexDisplayed );
        fDisplayOne = FALSE;
    }

    if( iLastTop != iTopTex || IsFirstDraw() || fDisplayOne )
    {
        int iRow = 1, iCol = 1;

        if( !fDisplayOne )
        {
            if( fNames )
            {
                // to separate for the texture
                setcolor( C_WHITE );
                line( rcRect.left + 2, rcRect.bottom - 136, rcRect.right - 2,
                    rcRect.bottom - 136 );
            }

            fldRegion::Draw();

            // don't unhighlight current texture
            iLastCur = -1;
        }

        bool fNoPictures = FALSE;

        // draw entire texture list
        int i, iTexture;
        for( i = 0, iTexture = iTopTex; iTexture < iMaxTextures; iTexture++, i++ )
        {
            if( keycheck() && !fNames )
                fNoPictures = TRUE;

            // create rectangle for drawing texture/floor
            if( !fDisplayOne || (fDisplayOne && iTexture == iCurTex) )
            {
                TexRects[i].SetRect( ptPoint - ptListTopLeft,
                                Size( iXPixels, iYPixels ) );

                TEXTURE* pTex = &texlist[iTexture];
                char* pszName;

                if( fFloors )
                    pszName = WN2Str( floorlist[iTexture].name );
                else
                    pszName = WN2Str( pTex->name );

                if( fNames || iXPixels >= 64 )
                {
                    int iNameOfsY = iYPixels - iCaptionHeight;

                    if( fNames )
                        iNameOfsY = 0;

                    setfillstyle( SOLID_FILL, C_BLUE );
                    bar( ptPoint.x, ptPoint.y + iNameOfsY, ptPoint.x + iXPixels,
                        ptPoint.y + iNameOfsY + iCaptionHeight );
                    setcolor( C_WHITE );
                    outtextxy( ptPoint.x + 2, ptPoint.y + iNameOfsY, pszName );
                }

                if( !fNames && !fNoPictures )
                {
                    if( !fFloors )
                        texture_disp( ptPoint.x, ptPoint.y, iXPixels, iXPixels,
                                    0, 0, pTex, FALSE );
                    else
                    {
                        floor_disp( ptPoint.x, ptPoint.y, iTexture );
                        if( iXPixels > 64 )
                        {
                            // draw four of 'em
                            floor_disp( ptPoint.x+64, ptPoint.y, iTexture );
                            floor_disp( ptPoint.x, ptPoint.y+64, iTexture );
                            floor_disp( ptPoint.x+64, ptPoint.y+64, iTexture );
                        }
                    }

                    fTexDisplayed[i] = TRUE;
                }
            }

            ptPoint.x += iXPixels + iXTexPadding * 2;
            ++iCol;

            if( iCol > iXCount )
            {
                // next line
                ptPoint.x = rcRect.left;
                iCol = 1;

                ptPoint.y += iYPixels + iYTexPadding * 2;
                iRow++;

                if( iRow > iYCount )
                {
                    ++i;
                    break;  // outta here
                }
            }

            if( fDisplayOne && iCurTex == iTexture )
                break;
        }
    }

    // unhighlight last current texture
    if( iLastCur != -1 && iLastCur != iCurTex )
    {
        Rect rcTexture( TexRects[iLastCur - iTopTex] );
        rcTexture.Inflate( 1, 1 );
        pdlgParent->FocusRect( rcTexture, C_WHITE, FALSE );
    }

    // highlight current texture
    if( iCurTex != -1 && iLastCur != iCurTex )
    {
        Rect rcTexture( TexRects[iCurTex - iTopTex] );
        rcTexture.Inflate( 1, 1 );
        pdlgParent->FocusRect( rcTexture, C_WHITE, FALSE );

        // update texture info
        char szBuf[128];
        if( !fFloors )
        {
            TEXTURE *pTex = &texlist[iCurTex];
            sprintf( szBuf, "%3lu of %-3lu %-8s  %3lux%-3lu  %2d patches  %s",
                    (long) iCurTex+1, (long) iMaxTextures, WN2Str( pTex->name ),
                    (long) pTex->width, (long) pTex->height, pTex->ndescrip,
                    pTex->ndescrip < 2 ? "(Transparent)" : "             " );

            if( fNames )
            {
                // display the texture
                Rect rcErase( rcRect.left + 4, rcRect.bottom - 132, rcRect.left + 300,
                              rcRect.bottom - 4 );
                pdlgParent->FillRect( rcErase, C_BLACK );
                texture_disp( rcRect.left + 4, rcRect.bottom - 132, 256, 128,
                              0, 0, pTex, TRUE );
            }
        }
        else
        {
            WADENTRY *pFloor = &floorlist[iCurTex];
            strcpy( szBuf, WN2Str( pFloor->name ) );

            if( fNames )
            {
                // display the floor
                floor_disp( rcRect.left + 4,    rcRect.bottom - 132, iCurTex );
                floor_disp( rcRect.left + 68,   rcRect.bottom - 132, iCurTex );
                floor_disp( rcRect.left + 4,    rcRect.bottom - 68, iCurTex );
                floor_disp( rcRect.left + 68,   rcRect.bottom - 68, iCurTex );
            }
        }

        ptxtInfo->SetText( szBuf );
    }

    iLastCur = iCurTex;
    iLastTop = iTopTex;
}

static int pbTFNames()
{
    keystuff( 'N' );
    return daContinue;
}

static int pbTFZoomIn()
{
    keystuff( '+' );
    return daContinue;
}

static int pbTFZoomOut()
{
    keystuff( '-' );
    return daContinue;
}

static int pbTFPageUp()
{
    keystuff( KEY_PGUP );
    return daContinue;
}

static int pbTFPageDown()
{
    keystuff( KEY_PGDN );
    return daContinue;
}

static int pbSaveDefault( fldTextureList* pTexList )
{
    // set defaults in config file
    if( pTexList->GetMode() == 2 )
        config.iTexFloorSize = pTexList->GetDisplaySize();
    else
        config.iTexWallSize = pTexList->GetDisplaySize();
    config.fTexShowNames = pTexList->GetNamesOnly();
    saveconfig();

    return daContinue;
}

int TexBrowser( int *iSelected, bool fFloors )
/*-------------------------------------------------------------------------*
 * Displays a texture browser, and jumps to "iSelected."  if they hit ESC,
 * returns iSelected.  ENTER returns the current texture, and cursor keys
 * move.
 *-------------------------------------------------------------------------*/
{
    int iHeight;
    int iWidth;
    int iTexPadding = 4;

    static int iLastWallSize = -1;
    static int iLastFloorSize = -1;
    static bool fLastNames;

    if( iLastWallSize == -1 )
    {
        // set default
        iLastWallSize = config.iTexWallSize;
        iLastFloorSize = config.iTexFloorSize;
        fLastNames = config.fTexShowNames;
    }

    iHeight = 128 + GetFontHeight()/10 + iTexPadding * 2;
    iWidth = 128 + iTexPadding * 2;

    Size szDialog( iScreenWidth - 32, iScreenHeight - 16 );

    szDialog.height -= szDialog.height % iHeight;
    szDialog.width -= szDialog.width % iWidth;

    if( iScreenHeight - szDialog.height < 80 )
        szDialog.height -= iHeight;

    szDialog.PixelsToUnits();
    szDialog.height += 36;

    Size szTextureList = szDialog;
    szTextureList.height -= 36;

    // create dialog
    Dialog* pDialog = new Dialog;
    pDialog->Create( Point( Centered, Centered ), szDialog,
                     "Texture / flats browser" );

    // texture info
    stText* pText = new stText( pDialog, Point( 4, szTextureList.height + 4 ),
        "" );

    // add texture field
    fldTextureList* pTextureList = new fldTextureList( pDialog,
            Rect( Point( 0, 0 ), szTextureList ), fFloors ? nfloor : ntex,
            pText, fFloors );

    if( fFloors )
        pTextureList->SetDisplaySize( iLastFloorSize );
    else
        pTextureList->SetDisplaySize( iLastWallSize );

    pTextureList->SetNamesOnly( fLastNames );
    pTextureList->SetCurTexture( iSelected[0] );
    pTextureList->SetMessage(
        "Use the arrow keys to move the selection, and ENTER to pick. The +"
        " and - keys zoom In/Out." );

    // add other buttons
    new fldButton( pDialog, Rect( 4, szDialog.height - 20, 34, szDialog.height - 8 ), "Names",
                   (FFUNC) pbTFNames, csMouseButton );
    new fldButton( pDialog, Rect( 38, szDialog.height - 20, 72, szDialog.height - 8 ), "Zoom In",
                   (FFUNC) pbTFZoomIn, csMouseButton );
    new fldButton( pDialog, Rect( 76, szDialog.height - 20, 110, szDialog.height - 8 ), "Zoom Out",
                   (FFUNC) pbTFZoomOut, csMouseButton );

    // add set default button
    new fldButton( pDialog, Rect( 114, szDialog.height - 20, 184, szDialog.height - 8 ), "Save as Default",
                   (FFUNC) pbSaveDefault, csMouseButton );

    new fldButton( pDialog, Rect( 200, szDialog.height - 20, 216, szDialog.height - 8 ), "<<",
                   (FFUNC) pbTFPageUp, csMouseButton );
    new fldButton( pDialog, Rect( 220, szDialog.height - 20, 236, szDialog.height - 8 ), ">>",
                   (FFUNC) pbTFPageDown, csMouseButton );


    // add ok/cancel
    pDialog->AddDefaultButtons();

    int iRvl = pDialog->Process();

    if( iRvl == daExitSave )
        iSelected[0] = pTextureList->GetCurTexture();

    // save these for next time
    fLastNames = pTextureList->GetNamesOnly();
    if( fFloors )
        iLastFloorSize = pTextureList->GetDisplaySize();
    else
        iLastWallSize = pTextureList->GetDisplaySize();

    delete pDialog;

    return iRvl == daExitSave;
}

short addblanktexture( char *name )
{
    TEXTURE *t = &pgdCurGame->texlist[pgdCurGame->ntex++];

    ntex = pgdCurGame->ntex;

    t->width = 64;
    t->height = 128;
    t->ndescrip = 0;
    t->dlist = NULL;
    wncpy( t->name, name );

    return pgdCurGame->ntex - 1;
}

static  short n_newpatch, n_updpatch, n_patchnotfound, n_patchpic,
              n_newtex, n_updtex;

short updatepnames( WAD *wptr, short texnum )
/*-------------------------------------------------------------------------*
 * Searches for all pnames that reference texture wad 'texnum' and updates
 * their offsets by searching "wptr" for the names.
 *-------------------------------------------------------------------------*/
{
    PNRef       *pnp = pgdCurGame->pnames;
    WADENTRY    *we;
    short       i, npname = pgdCurGame->npname, ngot = 0;

    n_patchnotfound = n_patchpic = 0;

    for( i = 0; i < npname; i++, pnp++ )
    {
        if( pnp->wadnumber == texnum )
        {
            we = wad_find( wptr, pnp->name );

            if( we )
            {
                pnp->offset = we->offset;
                ngot++;
                n_patchpic++;
            }
            else
            {
                we = wad_find( pwdIWAD, pnp->name );
                if( we )
                {
                    pnp->offset = we->offset;
                    pnp->wadnumber = 0;
                }
                else
                {
                    pnp->offset = -1;
                    n_patchnotfound++;
                }
            }
        }
    }

    return ngot;
}

uchar* fixwadnam( uchar *s );

short updatetextures( WAD *wptr, short texnum )
/*-------------------------------------------------------------------------*
 * Updates the texture information. (texnum is wptr's texture wad number)
 *   If wptr contains a PNAMES reference, updates the gamedata pnames
 *   list by recreating the entire thing from scratch.
 *
 *   If wptr contains a TEXTUREx lump, updates the pgdCurGame textures
 *   list:
 *      - Searches current texture list for each entry in new texture
 *        lump
 *      - Found? (same name, different data)  Updates the existing
 *        texture with the new data.
 *      - Not found?  Adds it to the end of the texture list.
 *   For all other entries:
 *      - Loops through the pnames reference; if it finds a match,
 *        sets the pnames entry to point to the new .wad and offset of the
 *        entry.
 *
 * If no texture things were found, returns 0.
 *-------------------------------------------------------------------------*/
{
    NODEPTR     ndp, dir_list = wptr->dir_list;
    WADENTRY    *we, *flp;
    char        *texlump, name[10], fn[128];
    char        *pnames, *newpnp, foundtex = 0;
    PNRef       *pnp;
    TEXMETA     *newtexp;
    TEXTURE     *curtexp;
    uchar       *newdlist;
    ulong       numnewpnames, numnewtex, i, *texofs;
    short       version;
    size_t      newdescrip_size;
    short       npname, curtexnum, gotsomething = 0;
    bool        fGotPNAMES = 0;
    short       curflat, curtexlump, handle;
    struct ftime ft1, ft2;

    DEBUG_INIT( "Updating texture lists with '%s' (#%d)", wptr->fn, texnum );

    n_updpatch = 0;
    n_newpatch = 0;
    n_updtex = 0;
    n_newtex = 0;

    if( wad_find( wptr, "PNAMES" ) )
    {
        pnames = newpnp = (char*) dirload( wptr, NULL, NULL );
        fGotPNAMES = gotsomething = 1;

        npname = pgdCurGame->npname = ((ulong*)pnames)[0];

        newpnp += sizeof( ulong );
        pnp = pgdCurGame->pnames;
        i = 0;

        // reload list of pnames
        for( i = 0; i < npname; i++, pnp++, newpnp += sizeof( WADNAME ) )
        {
            n_newpatch++;
            wncpy( pnp->name, newpnp );
            fixwadnam( pnp->name );
            if( texnum )    // if not loading IWAD..
            {
                // find it in the IWAD
                we = wad_find( pwdIWAD, pnp->name );
                if( we )    // if it's found, set its offset
                    pnp->offset = we->offset;
                else        // if it's not found here, it's found below (check for -1)
                    pnp->offset = -1;
            }
            pnp->wadnumber = 0; // set it to the IWAD - if its offset is -1, code below
                                // searches for it in the new file

            if( pgdCurGame->npname == MaxPNAME )
            {
                error( "PNAMES limit exceeded (That's a lot of PNAMES!)\n"
                       "The limit is %d; there are %d too many.", MaxPNAME,
                       numnewpnames - i );
            }
        }

        memfree( pnames );
    }

    /* search for texture lumps */
    for( curtexlump = 1; curtexlump <= 2; curtexlump++ )
    {
        sprintf( name, "TEXTURE%d", curtexlump );

        if( wad_find( wptr, name ) )
        {
            gotsomething = 1;
            foundtex = 1;
            texlump = (char*) dirload( wptr, NULL, NULL );

            numnewtex = ((ulong*)texlump)[0];
            texofs = (ulong*) (texlump + sizeof( ulong ));

            for( i = 0; i < numnewtex; i++ )
            {
                /* set texture pointer */
                newtexp = (TEXMETA*) (texlump + texofs[i]);

                /* only check if not loading the pwdIWAD */
                if( texnum && (curtexnum = texture_toint( newtexp->name )) != -1 )
                {
                    /* existing texture name */
                    curtexp = &pgdCurGame->texlist[curtexnum];

                    newdescrip_size = newtexp->ndescrip * sizeof( TEXDESCRIP );
                    newdlist = (uchar*) (newtexp + 1);

                    /* make sure they're different */
                    if( curtexp->width == newtexp->width &&
                        curtexp->height == newtexp->height &&
                        curtexp->ndescrip == newtexp->ndescrip )
                    {
                        /* now check to see if the descriptors are the same */
                        if( !memcmp( curtexp->dlist, newdlist, newdescrip_size ) )
                            continue;
                    }

                    n_updtex++;

                    curtexp->width = newtexp->width;
                    curtexp->height = newtexp->height;
                    curtexp->wadnumber = texnum;

                    if( newtexp->ndescrip != curtexp->ndescrip )
                    {
                        curtexp->ndescrip = newtexp->ndescrip;
                        if( curtexp->dlist )
                            memfree( curtexp->dlist );
                        curtexp->dlist = (TEXDESCRIP*) memget( newdescrip_size );
                    }

                    /* copy in the patch descriptors */
                    memcpy( curtexp->dlist, newdlist, newdescrip_size );
                }
                else
                {
                    if( pgdCurGame->ntex == MaxTEXTURE )
                    {
                        error( "Textures limit exceeded!\n"
                               "The limit is %d - there are %d too many.", MaxTEXTURE,
                               numnewtex - i );
                    }

                    n_newtex++;

                    /* new texture name */
                    curtexp = &pgdCurGame->texlist[pgdCurGame->ntex++];

                    wncpy( curtexp->name, newtexp->name );
                    curtexp->ndescrip = newtexp->ndescrip;
                    curtexp->width = newtexp->width;
                    curtexp->height = newtexp->height;
                    curtexp->dlist = (TEXDESCRIP*) memget(
                        sizeof( TEXDESCRIP ) * curtexp->ndescrip );
                    curtexp->wadnumber = texnum;

                    /* copy in the patch descriptors */
                    memcpy( curtexp->dlist, texlump + texofs[i] + sizeof( *newtexp ),
                            sizeof( TEXDESCRIP ) * curtexp->ndescrip );
                }
            }

            memfree( texlump );
        }
    }

    if( foundtex )
        qsort( pgdCurGame->texlist, pgdCurGame->ntex, sizeof( TEXTURE ), _ts_func );

    /* find wad entries that match references in the pnames array - only
     * if not loading the iwad. */
    if( texnum )
    {
        ndp = NodeFirst( dir_list );
        npname = pgdCurGame->npname;

        while( ndp != dir_list )
        {
            we = (WADENTRY*) ndp->data;

            /* search for it in the pnames list */
            for( i = 0, pnp = pgdCurGame->pnames; i < npname; i++, pnp++ )
            {
                if( !wncmp( we->name, pnp->name ) )
                {
                    gotsomething = 1;
                    pnp->wadnumber = texnum;
                }
            }

            ndp = NodeNext( ndp );
        }
    }

    /* create floor list */
    if( !texnum )
    {
        nfloor = 0;
        flp = pgdCurGame->floorlist;

        for( curflat = 1; ; curflat++ )
        {
            /* find the starter */
            sprintf( name, "F%d_START", curflat );

            if( (we = wad_find( wptr, name )) == NULL )
                break;      /* done */

            /* the end marker */
            sprintf( name, "F%d_END", curflat );

            ndp = wptr->curdir;

            for( ;; )
            {
                ndp = NodeNext( ndp );
                we = (WADENTRY*) ndp->data;

                if( !memcmp( we->name, name, 6 ) )
                    break;

                ++nfloor;
                memcpy( flp++, we, sizeof( WADENTRY ) );
            }
        }

        pgdCurGame->nfloor = nfloor;
        qsort( pgdCurGame->floorlist, pgdCurGame->nfloor, sizeof( WADENTRY ), _wes_func );

        we = pgdCurGame->floorlist;
        name[8] = 0;

        if( pgdCurGame->stlFlats )
            delete pgdCurGame->stlFlats;

        pgdCurGame->stlFlats = new StrList( nfloor+1, 9, "FloorList" );

        for( i = 0; i < nfloor; i++, we++ )
        {
            memcpy( name, we->name, 8 );
            pgdCurGame->stlFlats->AddTail( name, -1 );
        }
    }

    ntex = pgdCurGame->ntex;

    /* update the pnames list offsets */
    if( !texnum )
    {
        sprintf( fn, "%s.PLT", pgdCurGame->cfgname );
        npname = pgdCurGame->npname;

        handle = z_open( pwdIWAD->fn, O_BINARY | O_RDONLY );
        getftime( handle, &ft1 );
        z_close( handle );

        if( access( fn, 0 ) == -1 )
        {
makeplt:
            updatepnames( wptr, texnum );
            handle = z_creat( fn, O_BINARY | S_IWRITE );
            z_write( handle, _ABBREV_" Patch Lookup Table\r\n\x1a", 50 );
            z_write( handle, &file_version, sizeof file_version );
            z_write( handle, &ft1, sizeof ft1 );
            z_write( handle, &npname, sizeof npname );
            z_write( handle, pgdCurGame->pnames, sizeof( PNRef ) * npname );
            z_close( handle );
        }
        else
        {
            handle = z_open( fn, O_BINARY | O_RDONLY );
            lseek( handle, 50L, SEEK_SET );
            z_read( handle, &version, sizeof version );

            if( version != file_version )
            {
                z_close( handle );
                goto makeplt;
            }

            z_read( handle, &ft2, sizeof ft2 );

            if( memcmp( &ft1, &ft2, sizeof ft1 ) )
            {
                /* different times .. etc. */
                z_close( handle );
                goto makeplt;
            }

            z_read( handle, &npname, sizeof npname );
            z_read( handle, pgdCurGame->pnames, sizeof( PNRef ) * npname );
            z_close( handle );
            pgdCurGame->npname = npname;
        }
    }
    else
    {
        if( updatepnames( wptr, texnum ) )
            gotsomething = 1;
    }

    DEBUG_EXIT();

    return gotsomething;
}

short istexwad( char *fn )
/*-------------------------------------------------------------------------*
 * Guess.
 *-------------------------------------------------------------------------*/
{
    short i;

    for( i = 0; pgdCurGame->texwads[i]; i++ )
    {
        if( !strcmpi( fn, pgdCurGame->texwads[i] ) )
            return i;
    }

    return -1;
}

void checktexupdate( WAD *wptr )
/*-------------------------------------------------------------------------*
 * if wptr is one of the texture .wads, calls updatepnames() with the
 * wad number.
 *-------------------------------------------------------------------------*/
{
    short texnum = istexwad( wptr->fn );

    if( texnum >= 0 )
        updatepnames( wptr, texnum );
}

void init_textures( void )
/*-------------------------------------------------------------------------*
 * Initializes textures: loads all the .wads in gamedat's texwads[] array.
 *-------------------------------------------------------------------------*/
{
    short   i;
    WAD*    wptr;

    if( !pgdCurGame->floorlist )
    {
        pgdCurGame->floorlist = (WADENTRY*) memget( MaxFLOOR * sizeof( WADENTRY ) );
        pgdCurGame->texlist = (TEXTURE*) memget( MaxTEXTURE * sizeof( TEXTURE ) );
        pgdCurGame->pnames = (PNRef*) memget( MaxPNAME * sizeof( PNRef ) );
    }
    else
    {
        pgdCurGame->nfloor = nfloor = 0;
        pgdCurGame->ntex = ntex = 0;
        pgdCurGame->npname = 0;
    }

    /* add the "-" entry */
    addblanktexture( "-" );

    for( i = 0; pgdCurGame->texwads[i]; i++ )
    {
        if( i ) /* no sense in reopening the pwdIWAD */
            wptr = wad_open( pgdCurGame->texwads[i], WL_COMPLETE );
        else
            wptr = pwdIWAD;

        if( wptr )
            updatetextures( wptr, i );

        if( i && wptr )
            wad_close( wptr );
    }
}

short addtexwad( WAD* wptr )
{
    short   i, rvl;
    char    *p;

    for( i = 0; pgdCurGame->texwads[i]; i++ )
    {
        if( !strcmpi( wptr->fn, pgdCurGame->texwads[i] ) )     /* already there */
            return 2;
    }

    p = pgdCurGame->texwads[i] = (uchar*) memget( 128 );

    strcpy( p, wptr->fn );
    rvl = updatetextures( wptr, i );

    if( !rvl )
    {
        /* get rid of it */
        memfree( pgdCurGame->texwads[i] );
        pgdCurGame->texwads[i] = NULL;
    }

    return rvl;
}

void removetexwad( short texnum )
{
    short i;

    /* since there's no real easy way to remove a texture wad, just
     * remove it from the list and reload the entire list.  Good thing
     * this doesn't take too long... */
    memfree( pgdCurGame->texwads[texnum] );

    for( i = texnum; pgdCurGame->texwads[i]; i++ )
        pgdCurGame->texwads[i] = pgdCurGame->texwads[i+1];

    pgdCurGame->texwads[i] = NULL;

    init_textures();
}

/*-------------------------------------------------------------------------
 * Interface: Texture Management
 *-------------------------------------------------------------------------
 *
 * On saving the textures to a .WAD file, a list of textures to save is
 * referenced and only those textures are saved.  The list is automatically
 * updated with the names of the textures that are modified by the user,
 * but can optionally be filled with other data, such as: 1) all the textures
 * defined in the current .wad file, or 2) a user-defined list.
 *
 * From the savelist of textures is created a list of PNAMES that must be
 * saved as well; this list is created by considering each referenced patch
 * and saving it if it is not found in the pwdIWAD (patch->wadnumber != 0.)
 *
 * The textures are then converted to reference patch NAMES, and not numbers,
 * in preparation for the next step.
 *
 *  * All saved textures go in the TEXTURE2 lump.
 *
 * When saving, the destination file is checked for existing PNAMES or
 * TEXTURE2 lumps.  If they exist, each entry is
 *
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Interface: texture file management
 *--------------------------------------------------------------------------
 * Allows:
 *   - Adding a texture file to the active list
 *   - Removing an active texture file
 *-------------------------------------------------------------------------*/

static  fldListBox* pfTexFiles;
static  StrList     *stlTexFiles;
static  char  **dtext;
static  short dnline;

void ConvertSidedefs( bool fToNames )
{
    static SIDEDEF *pSideNames, *pNamePtr;
    SIDEDEFM *pIndexPtr;

    if( !nside )
        return;

    if( fToNames )
    {
        // convert current indices list into names list
        if( pSideNames )
        {
            memfree( pSideNames );
            pSideNames = NULL;
        }

        _memstr = "sidedef conversion buffer";
        pSideNames = pNamePtr = (SIDEDEF*) memget( nside * sizeof( SIDEDEF ) );
        pIndexPtr = sidelist;

        for( int i = 0; i < nside; i++ )
        {
            // no need to copy all info
            wncpy( pNamePtr->utxname, texlist[pIndexPtr->utxidx].name );
            wncpy( pNamePtr->ltxname, texlist[pIndexPtr->ltxidx].name );
            wncpy( pNamePtr->ftxname, texlist[pIndexPtr->ftxidx].name );

            ++pNamePtr;
            ++pIndexPtr;
        }
    }
    else
    {
        // convert saved names list into current indices list

        pNamePtr = pSideNames;
        pIndexPtr = sidelist;

        for( int i = 0; i < nside; i++ )
        {
            /* Translate it into memory */
            if( (pIndexPtr->utxidx = texture_toint( pNamePtr->utxname )) == -1 )
                if( (pIndexPtr->utxidx = texture_toint_ic( pNamePtr->utxname )) == -1 )
                    pIndexPtr->utxidx = addblanktexture( pNamePtr->utxname );

            if( (pIndexPtr->ltxidx = texture_toint( pNamePtr->ltxname )) == -1 )
                if( (pIndexPtr->ltxidx = texture_toint_ic( pNamePtr->ltxname )) == -1 )
                    pIndexPtr->ltxidx = addblanktexture( pNamePtr->ltxname );

            if( (pIndexPtr->ftxidx = texture_toint( pNamePtr->ftxname )) == -1 )
                if( (pIndexPtr->ftxidx = texture_toint_ic( pNamePtr->ftxname )) == -1 )
                    pIndexPtr->ftxidx = addblanktexture( pNamePtr->ftxname );

            ++pNamePtr;
            ++pIndexPtr;
        }

        memfree( pSideNames );
        pSideNames = NULL;
    }
}

static void daddtext( char *fmt, ... )
{
    va_list vl;
    char    buf[128];

    va_start( vl, fmt );
    vsprintf( buf, fmt, vl );
    va_end( vl );

    dtext[dnline] = (char*) memget( 128 );
    strcpy( dtext[dnline++], buf );
}

static int pbTFDetails()
/*-------------------------------------------------------------------------*
 * Details of the current texture file.
 *-------------------------------------------------------------------------*/
{
    short   curtexwad = pfTexFiles->GetCurItem();
    char    buf[128];
    short   i, f_npname = 0, f_ntex = 0, npname = pgdCurGame->npname, nb;
    PNRef   *pnp = pgdCurGame->pnames;

    dnline = 0;
    buf[8] = 0;

    /* get some text */
    dtext = (char**) memget( 2048 * sizeof( char * ) );

    daddtext( "" );
    daddtext( "|F1Details for %s:", pfTexFiles->GetText() );
    daddtext( "" );

    for( i = 0; i < ntex; i++ )
    {
        if( texlist[i].wadnumber == curtexwad )
            f_ntex++;
    }

    if( f_ntex )
    {
        nb = 0;
        buf[0] = 0;

        daddtext( "  Contains %d (of %d) texture definitions: ", f_ntex, ntex );

        for( i = 0; i < ntex; i++ )
        {
            if( texlist[i].wadnumber == curtexwad )
            {
                sprintf( buf + strlen( buf ), "  %-8s  %3d x %-3d",
                         WN2Str( texlist[i].name ), texlist[i].width, texlist[i].height );
                ++nb;

                if( nb == 2 )
                {
                    nb = 0;
                    daddtext( buf );
                    buf[0] = 0;
                }
            }
        }

        if( nb )
            daddtext( buf );
    }
    else
        daddtext( "  Contains no texture definitions." );

    daddtext( "" );

    for( i = 0; i < npname; i++ )
    {
        if( pnp[i].wadnumber == curtexwad )
            f_npname++;
    }

    /* add pnames info */
    if( f_npname )
    {
        nb = 0;
        buf[0] = 0;

        daddtext( "  Contains %d (of %d) PNAMES entries: ", f_npname, npname );

        for( i = 0; i < npname; i++ )
        {
            if( pnp[i].wadnumber == curtexwad )
            {
                sprintf( buf + strlen( buf ), "  %-8s", WN2Str( pnp[i].name ) );

                ++nb;

                if( nb == 3 )
                {
                    nb = 0;
                    daddtext( buf );
                    buf[0] = 0;
                }
            }
        }

        if( nb )
            daddtext( buf );

        daddtext( "" );
    }
    else
        daddtext( "  Contains no PNAMES entries." );

    dtext[dnline] = NULL;

    /* read the text */
    readtext( "Texture Details", dtext, dnline, 20 );

    for( i = 0; dtext[dnline]; i++ )
        memfree( dtext[i] );

    memfree( dtext );

    return daContinue;
}

static int pbTFRemove()
/*-------------------------------------------------------------------------*
 * Takes awhile, but..
 *-------------------------------------------------------------------------*/
{
    short   curtexwad = pfTexFiles->GetCurItem(), i;

    if( !curtexwad )
    {
        message_box( MB_OK, "Like, duh.",
          "You can't remove the IWAD from the list." );
        return daContinue;
    }

    ConvertSidedefs( TRUE );

    /* remove it, jah. */
    removetexwad( curtexwad );

    if( curtexwad == pfTexFiles->GetItemList()->GetCount()-1 )
        pfTexFiles->SetCurItem( curtexwad-1 );

    pfTexFiles->GetItemList()->DeleteAll();
    for( i = 0; pgdCurGame->texwads[i]; i++ )
        pfTexFiles->GetItemList()->AddTail( pgdCurGame->texwads[i] );

    pfTexFiles->ReInit();

    if( curtexwad == stlTexFiles->GetCount() )
        pfTexFiles->SetCurItem( curtexwad-1 );
    else
        pfTexFiles->OnDraw();

    ConvertSidedefs( FALSE );

    return daContinue;
}

static int pbTFAdd()
/*-------------------------------------------------------------------------*
 * Add a file to the list
 *-------------------------------------------------------------------------*/
{
    uchar   fn[128];
    uchar   *dv_st1 = st_blank, *dv_st2 = st_blank;

    strcpy( fn, pfTexFiles->GetText() );

    /* get a file ? */
    if( file_get( FG_LOAD | FG_NOMAPS, fn, "WAD" ) )
    {
        ConvertSidedefs( TRUE );

        if( !addtexwad( wad_open( fn, WL_COMPLETE ) ) )
            message_box( MB_OK, st_dcksays,
              "I couldn't find any textures in that file." );
        else
        {
            message_box( MB_OK, st_dcksays,
                         "%s loaded:\n"
                         " %d new PNAMES entries\n"
                         " %d existing PNAMES entries updated\n"
                         " %d patch pictures\n"
                         " %d new textures\n"
                         " %d existing textures updated\n", fn,
                         n_newpatch, n_updpatch, n_patchpic, n_newtex, n_updtex );
            pfTexFiles->GetItemList()->AddTail( fn );
            pfTexFiles->ReInit();
            pfTexFiles->OnDraw();

            if( n_patchnotfound )
            {
                if( n_patchnotfound > 10 )
                {
                    dv_st1 = "(This PWAD might have been\n";
                    dv_st2 = "intended for a different game.)";
                }

                message_box( MB_OK, st_dcksays,
                 "In the PNAMES resource, there were %d patches named\n"
                 "that were not found in the IWAD (%s)\n"
                 "or the loaded PWAD.  %s%s", n_patchnotfound, pwdIWAD->fn,
                 dv_st1, dv_st2 );
            }
        }

        ConvertSidedefs( FALSE );
    }

    return daContinue;
}

static int pbTFBrowser()
{
    int iSel = 0;
    TexBrowser( &iSel, FALSE );
    return daContinue;
}

void texfile_mgmt( void )
/*-------------------------------------------------------------------------*
 * Interface for adding / removing texture files from the list.
 *-------------------------------------------------------------------------*/
{
    Dialog      *pDialog;

    stlTexFiles = new StrList( 32, 64, "TexFiles" );

    for( int i = 0; pgdCurGame->texwads[i]; i++ )
        stlTexFiles->AddTail( pgdCurGame->texwads[i] );

    VarTransfer_t vtTexMgmt[] =
    {
        { "TexFiles", (Field**) &pfTexFiles, NULL, NULL },
        { "BtnAdd", NULL, (PTR) pbTFAdd, NULL },
        { "BtnRemove", NULL, (PTR) pbTFRemove, NULL },
        { "BtnDetails", NULL, (PTR) pbTFDetails, NULL },
        { "BtnBrowser", NULL, (PTR) pbTFBrowser, NULL },

        // end marker
        { NULL, NULL, NULL, NULL }
    };

    pDialog = new Dialog;
    pDialog->Create( szDSFile, "TexMgmtDlg" );
    pDialog->VarSetup( vtTexMgmt );

    pfTexFiles->SetUserEvent( etKey, KEY_INS, (FUFUNC) pbTFAdd );
    pfTexFiles->SetUserEvent( etKey, KEY_DEL, (FUFUNC) pbTFRemove );
    pfTexFiles->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) pbTFDetails );
    pfTexFiles->SetItemList( stlTexFiles );
    pfTexFiles->SetCurItem( 0 );

    pDialog->Process();

    delete pDialog;
}
