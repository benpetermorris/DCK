/*-------------------------------------------------------------------------*
 * GFX.C:                                                                  *
 * Functions for drawing onscreen in graphics mode.                        *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\gui.hpp"

#include <math.h>
#include <limits.h>

/*-------------------------------------------------------------------------*
 *
 * Globals begin here
 *
 *-------------------------------------------------------------------------*/

/* colors */
#define C_THING_OM  C_GRAY1

int d_ints[] =  /* drivers */
{
    ATI256,
    ATIGUP,
    VESA256,
    COMPAQ,
    TSENG3256,
    TSENG4256,
    GENOA5,
    GENOA6,
    OAK,
    PARADIS256,
    TECMAR,
    TRIDENT256,
    VIDEO7,
    VIDEO7II,
    S3,
    DETECTX
};

int r_ints[] =  /* resolutions */
{
    RES640x480,
    RES800x600,
    RES1024x768,
    RES1280x1024
};

int     dckfont;
bool    fNewGfxMode;
static  short gr_initok = 0, tog_doompal = 0;
static  uchar pal256[256][3];

/*-------------------------------------------------------------------------*
 *
 * Code begins here
 *
 *-------------------------------------------------------------------------*/

void gr_checkerr( void )
{
    short result = graphresult();
    if( result != grOk )
        error( "Graphics error %d: %s", result, grapherrormsg( result ) );
}

void gr_closegraph( void )
{
    if( gr_initok )
    {
        if( lastvidmode != MODE_TEXT )
            restorecrtmode();
        closegraph();
        gr_initok = 0;
    }
}

void gr_vidmode( short mode )
/*-------------------------------------------------------------------------*
 * Sets the video mode to 'mode' and sets curvidmode = mode.               *
 *-------------------------------------------------------------------------*/
{
    int     gdriver, gmode;
    static  short first = 1;

    if( mode == lastvidmode )
        return;

    if( mode == MODE_GFX )
    {
        fNewGfxMode = TRUE;

        gdriver = d_ints[config.driver];
        gmode = r_ints[config.mode];

        initgraph( &gdriver, &gmode, NULL );
        gr_checkerr();

        dckfont = installuserfont( _ABBREV_".FNT" );
        gr_checkerr();
        settextstyle( dckfont, HORIZ_DIR, 1 );
        _fontheight = 15;
        _fontwidth = 7;

        InitGUI();
        SetGuiColor( gcDlgText, BLACK );
        SetGuiColor( gcMenuText, BLACK );
        SetGuiColor( gcMenuItemText, BLACK );
        SetGuiColor( gcFldFocus, BLACK );
        SetGuiColor( gcLboxFocus, LIGHTGREEN );

        if( !pgdCurGame )
            loadpalette( _ABBREV_".pal" );
        else
            loadpalette( pgdCurGame->palette );

        sxlen = iScreenWidth;
        sylen = iScreenHeight;

        gr_initok = 1;

        fNewGfxMode = FALSE;
    }
    else if( mode == MODE_TEXT && !first )
    {
        mouse_kill();
        restorecrtmode();
        gr_closegraph();
    }

    first = 0;
    lastvidmode = mode;
}

uchar findrgb( short r, short g, short b )
{
    ulong    curidx = -1, curdist = ULONG_MAX, dist;
    short   i, rd, gd, bd;

    for( i = 0; i < 256; i++ )
    {
        rd = r-(long)pal256[i][0];
        gd = g-(long)pal256[i][1];
        bd = b-(long)pal256[i][2];

        dist = ((rd*rd)+(gd*gd)+(bd*bd));

        if( dist < curdist )
        {
            curdist = dist;
            curidx = i;
        }
    }

    return curidx;
}

void convertcolortable( short table[][4] )
{
    short   c;
    long    r, g, b;

    for( c = 0; table[c][0] != -1; c++ )
    {
        r = table[c][0];
        g = table[c][1];
        b = table[c][2];

        table[c][3] = findrgb( r, g, b );
    }
}

typedef struct tagRGBQUAD
{
    byte    rgbBlue;
    byte    rgbGreen;
    byte    rgbRed;
    byte    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFOHEADER
{
    ulong   biSize;
    long    biWidth;
    long    biHeight;
    unint    biPlanes;
    unint    biBitCount;
    ulong   biCompression;
    ulong   biSizeImage;
    long    biXPelsPerMeter;
    long    biYPelsPerMeter;
    ulong   biClrUsed;
    ulong   biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER
{
    ushort    bfType;
    ulong   bfSize;
    ushort    bfReserved1;
    ushort    bfReserved2;
    ulong   bfOffBits;
} BITMAPFILEHEADER;

void SaveShot( void )
{
    int i;
    long _size;

    RGBQUAD pal[256];
	BITMAPFILEHEADER bmpFh;
	BITMAPINFOHEADER bmih;

    struct viewporttype vp;
    getviewsettings( &vp );
    SetViewPort( 0, 0, iScreenWidth, iScreenHeight, 0 );

	bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = iScreenWidth + 1;
    bmih.biHeight = iScreenHeight + 1;
	bmih.biPlanes = 1;
    bmih.biBitCount = 8;
    bmih.biCompression = 0;
	bmih.biSizeImage = _size = bmih.biHeight *
                                bmih.biWidth;
    bmih.biClrUsed = 256;
    bmih.biClrImportant = 256;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;

	bmpFh.bfType = 0x4d42;
    bmpFh.bfSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + _size + sizeof( pal );
	bmpFh.bfReserved1 = bmpFh.bfReserved2 = 0;
    bmpFh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER ) + sizeof( pal );

    char szFn[128];

    for( i = 0; i < 9999; i++ )
    {
        sprintf( szFn, _ABBREV_"%04d.BMP", i );
        if( access( szFn, 0 ) == -1 )
            break;
    }

    // convert palette
    for( i = 0; i < 256; i++ )
    {
        pal[i].rgbBlue = pal256[i][2];
        pal[i].rgbGreen = pal256[i][1];
        pal[i].rgbRed = pal256[i][0];
        pal[i].rgbReserved = 0;
    }

    int iHandle = _creat( szFn, O_BINARY | S_IWRITE );

    _write( iHandle, &bmpFh, sizeof bmpFh );
    _write( iHandle, &bmih, sizeof bmih );
    _write( iHandle, &pal, sizeof pal );

    // write screen data now
    int x, y;
    char *linebuf = new char[iScreenWidth+1];
    for( y = iScreenHeight; y >= 0; y-- )
    {
        i = 0;
        for( x = 0; x <= iScreenWidth; x++ )
            linebuf[i++] = getpixel( x, y );
        _write( iHandle, linebuf, iScreenWidth+1 );
    }
    delete linebuf;

    close( iHandle );

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom, 1 );
}



void gr_line( short x1, short y1, short x2, short y2, short color )
{
    gr_setcolor( color );
    line( x1, y1, x2, y2 );
}

void gr_rect( short x1, short y1, short x2, short y2, short color )
{
    gr_setcolor( color );
    bar( x1, y1, x2, y2 );
}

void gr_box( short x1, short y1, short x2, short y2, short color )
{
    gr_setcolor( color );
    rectangle( x1, y1, x2, y2 );
}

void gr_setlinestyle( short style, short thickness )
{
    setlinestyle( style, 0, thickness );
}

void gr_clearviewport( void )
{
    clearviewport();
}

void gr_setcolor( short color )
{
    setfillstyle( SOLID_FILL, color );
    setcolor( color );
}

void gr_setwritemode( short mode )
{
    setwritemode( mode );
}

void gr_setvisualpage( short page )
{
}

void gr_setactivepage( short page )
{
}

short gr_textwidth( char *text )
{
    return textwidth( text );
}

void sc_line( short x1, short y1, short x2, short y2 )
/*-------------------------------------------------------------------------*
 * Draws a scaled line from x1, y1 to x2, y2 using the scale/zoom factor.  *
 *-------------------------------------------------------------------------*/
{
    short     sx1, sy1, sx2, sy2;

    /* Scale the coordinates into screen coordinates */
    sx1 = _mapx2scr( x1-xstart );
    sx2 = _mapx2scr( x2-xstart );
    sy1 = _mapy2scr( y1-ystart );
    sy2 = _mapy2scr( y2-ystart );

    line( sx1, sy1, sx2, sy2 );
}

void sc_plotx( short x1, short y1, short color )
/*-------------------------------------------------------------------------*
 * Plots a small 'x' at x1, y1 using color.                                *
 *-------------------------------------------------------------------------*/
{
    short sx1, sy1;

    /* Make the coordinates relative to the topleft of the viewport */
    x1 -= xstart;
    y1 -= ystart;

    gr_setcolor( color );
    gr_setwritemode( SWM_XOR );
    gr_setlinestyle( SLS_SOLID, 1 );

    /* Scale the coordinates into screen coordinates */
    sx1 = _mapx2scr( x1 );
    sy1 = _mapy2scr( y1 );

    /* Draw the first line */
    line( sx1-10, sy1-10, sx1+10, sy1+10 );

    /* Draw the second line */
    line( sx1+10, sy1-10, sx1-10, sy1+10 );
}

void _cursor_put( short mode )
{
    if( mode == CSR_ERASE )
        mouse_hide();
    if( mode == CSR_PUT )
        mouse_show();
}

void gr_shadrect( short x1, short y1, short x2, short y2, short color )
/*-------------------------------------------------------------------------*
 * Draws a "shadowed" (3d) rectangle at the specified coordinates and fills*
 * it in.  (popping UP)                                                    *
 *-------------------------------------------------------------------------*/
{
    short     dk, lt;

    dk = C_GRAY4;
    lt = C_GRAY2;

    if( color == C_GRAY4 )
        dk = C_GRAY5;

    if( color != -1 )
        gr_rect( x1+1, y1+1, x2-1, y2-1, color );

    gr_line( x1, y2, x2, y2, dk );
    gr_line( x2, y1, x2, y2, dk );

    gr_line( x1, y1, x2, y1, lt );
    gr_line( x1, y1, x1, y2, lt );
}

void gr_rshadrect( short x1, short y1, short x2, short y2, short color )
/*-------------------------------------------------------------------------*
 * Draws a "shadowed" (3d) rectangle at the specified coordinates and fills*
 * it in. (reverse!)                                                       *
 *-------------------------------------------------------------------------*/
{
    short dk, lt;

    dk = C_GRAY4;
    lt = C_GRAY2;

    if( color == C_GRAY4 )
        dk = C_BLACK;

    swap2b( dk, lt );

    if( color != -1 )
        gr_rect( x1+1, y1+1, x2-1, y2-1, color );

    gr_line( x1, y2, x2, y2, dk );
    gr_line( x2, y1, x2, y2, dk );

    gr_line( x1, y1, x2, y1, lt );
    gr_line( x1, y1, x1, y2, lt );
}

#define rad_deg 57.29583
#define deg_rad 0.0174532

void sc_linewithendmarks( short x1, short y1, short x2, short y2 )
{
    short     sx1, sy1, sx2, sy2, nx, ny;
    double  m, dx, dy;

    /* Scale the coordinates into screen coordinates */
    sx1 = _mapx2scr( x1-xstart );
    sx2 = _mapx2scr( x2-xstart );
    sy1 = _mapy2scr( y1-ystart );
    sy2 = _mapy2scr( y2-ystart );

    line( sx1, sy1, sx2, sy2 );

    /* Make the end-notches */
    dx = sx2-sx1;
    dy = sy2-sy1;
    if( dy == 0.0 )
    {
        nx = 0;
        ny = 8;
    }
    else
    {
        m = dx/dy;
        nx = (short) (8.0 * cos( -m * rad_deg ));
        ny = (short) (8.0 * sin( -m * rad_deg ));
    }

    line( sx1-nx, sy1-ny, sx1+nx, sy1+ny );
    line( sx2-nx, sy2-ny, sx2+nx, sy2+ny );
}

void drawdirection( short x1, short y1, short x2, short y2, short angle )
{
    short xmid, ymid;

    x1+=2; y1+=2;
    x2-=2; y2-=2;

    xmid = (x1+x2)/2;
    ymid = (y1+y2)/2;

    clmou;
    #undef lineto
    #undef moveto

    setcolor( C_BLACK );

    switch( angle )
    {
    case 90:    /* north */
        moveto( x1, ymid );
        lineto( xmid, y1 );
        lineto( x2, ymid );
        lineto( x1, ymid );
        break;
    case 270:   /* south */
        moveto( x1, ymid );
        lineto( xmid, y2 );
        lineto( x2, ymid );
        lineto( x1, ymid );
        break;
    case 0:     /* east */
        moveto( xmid, y1 );
        lineto( x2, ymid );
        lineto( xmid, y2 );
        lineto( xmid, y1 );
        break;
    case 180:   /* west */
        moveto( xmid, y1 );
        lineto( x1, ymid );
        lineto( xmid, y2 );
        lineto( xmid, y1 );
        break;
    case 45:    /* northeast */
        moveto( x1, y1 );
        lineto( x2, y1 );
        lineto( x2, y2 );
        lineto( x1, y1 );
        break;
    case 135:   /* northwest */
        moveto( x1, y1 );
        lineto( x2, y1 );
        lineto( x1, y2 );
        lineto( x1, y1 );
        break;
    case 315:   /* southeast */
        moveto( x1, y2 );
        lineto( x2, y2 );
        lineto( x2, y1 );
        lineto( x1, y2 );
        break;
    case 225:   /* southwest */
        moveto( x1, y1 );
        lineto( x2, y2 );
        lineto( x1, y2 );
        lineto( x1, y1 );
        break;
    }

    stmou;
    #include "gdef.h"
}

short ts_draw( short x, short y, short type );

void sc_drawthing( short tnumber )
/*-------------------------------------------------------------------------*
 * Draws tlist[tnumber]                                                    *
 *-------------------------------------------------------------------------*/
{
    THING   *t = &thinglist[tnumber];
    ushort  attribs = t->attribs;
    short   sx1, sy1, r, c, mr = thing_mapradius( t->type );
    char    dblack = 0;
    TInfo   *tipi;
    extern  short xend, yend;

    /* Test radius */
    if( !inrange( t->x, xstart-mr, xstart+xscale+mr ) ||
        !inrange( t->y, ystart-mr, ystart+yscale+mr ) )
        return;

    /* check to see if we should display this */
    if( td_flags.use )
        if( !td_check( attribs ) )
            dblack = 1;

    tipi = &(pgdCurGame->thinginfo[pgdCurGame->ttype2idx[t->type]]);

    /* Scale the coordinates into screen coordinates */
    sx1 = _mapx2scr( t->x-xstart );
    sy1 = _mapy2scr( t->y-ystart );

    /* if editmode is thing type, and thing graphics are on,
     * and the xscale is less than the scale to display the small size
     * at, and we're NOT drawing a thing with angles.. */
    if( !dblack && editmode == MODE_THING && tog_tgraph &&
        xscale <= config.tgr_size1 && !((tipi->dangle) && !config.tgr_enemies) )
    {
        /* might return 0 */
        if( ts_draw( sx1, sy1, t->type ) )
            return;
    }

    r = thing_radius( t->type );

    if( !dblack && editmode == MODE_THING )
    {
        /* Draw the thing */
        short x1 = sx1-r, y1 = sy1-r,
              x2 = sx1+r, y2 = sy1+r;

        c = thing_color( t->type );

        setfillstyle( SOLID_FILL, c );
        bar( x1+1, y1+1, x2-1, y2-1 );
        /* Bright lines */
        setcolor( C_WHITE );
        rectangle( x1, y1, x2, y2 );
        //moveto( x1, y2 );
        //lineto( x1, y1 );
        //lineto( x2, y1 );
        /* dark lines */
        //setcolor( C_GRAY3 );
        //lineto( x2, y2 );
        //lineto( x1+1, y2 );

        if( tipi->dangle )
            drawdirection( x1, y1, x2, y2, t->angle );
    }
    else
    {
        gr_setcolor( C_THING_OM );
        /* Draw the liiine, maaaan */
        line( sx1-r, sy1, sx1+r, sy1 );
        line( sx1, sy1-r, sx1, sy1+r );
    }
}

uchar *ipi2img( uchar *src, size_t buflen, img_hdr *dest )
{
    uchar   palcvt[256];
    uchar   *p, *srcp, *dstp;
    short   i;

    /* palette first */
    p = src;

    for( i = 0; i < 256; i++ )
        palcvt[i] = findrgb( *p++, *p++, *p++ );

    if( dest == NULL )
        dest = (img_hdr*) memget( buflen - 768 );

    memcpy( dest, src+768, sizeof( img_hdr ) );

    /* now copy the rest */
    i = 768 + sizeof( img_hdr );
    srcp = src + i;
    dstp = ((uchar*) dest) + sizeof( img_hdr );

    for( ; i < buflen; i++ )
        *dstp++ = palcvt[*srcp++];

    return (uchar*) dest;
}

void loadpalette( char *fn )
{
    int     i;
    int     r, g, b;
    FILE    *fp;
    static  char lastpal[128];

    if( strcmpi( fn, lastpal ) && !fNewGfxMode )
    {
        SetViewPort( 0, 0, sxlen, sylen, 1 );
        clearviewport();
        fFullUpdate = 2;
        strcpy( lastpal, fn );
    }

    fp = fopen( fn, "rb" );
    fread( pal256, 768, 1, fp );

    tog_doompal = 1;

    for( i = 0; i < 256; i++ )
    {
        r = (int) pal256[i][0];
        g = (int) pal256[i][1];
        b = (int) pal256[i][2];
        setrgbpalette( i, r >> 2, g >> 2, b >> 2 );
    }

    convertcolortable( guito256 );

    fclose( fp );
}

/*-------------------------------------------------------------------------*
 *
 * Putimage with background sensitivity.
 *
 *-------------------------------------------------------------------------*/

#pragma opt -O2
#pragma inline +memset +memcpy

#define pfMaxWidth  256
#define pfMaxHeight 200

void putimage_bg( short x, short y, uchar *buf, short picdiv )
{
    static      uchar *pfget, *pfput;
    static      unsigned pflen;

    unsigned    clen, cpos, cpos2;
    short       height, width, wpad, bx, by, bwpad;
    uchar       *gptr, *pptr, *bptr;
    img_hdr     *hdr = (img_hdr*) buf;

    if( !pfget )
    {
        pflen = imagesize( 0, 0, pfMaxWidth+2, pfMaxHeight+2 );
        pfget = new uchar[pflen];
        pfput = new uchar[pflen];
    }

    width = hdr->width;
    height = hdr->height;
    bwpad = hdr->widthpadded;

    if( height > pfMaxHeight || width > pfMaxWidth )
        error( "putimage transparency limit exceeded (%d x %d)", width, height );

    if( picdiv > 1 )
    {
        getimage( x, y, x+width/2-1, y+height/2-1, pfget );

        clen = imagesize( 0, 0, width/2, height/2 );

        /* copy header */
        memset( pfput, 0, clen );
        memcpy( pfput, pfget, sizeof( img_hdr ) );
        wpad = ((img_hdr*)pfput)->widthpadded;

        bwpad *= picdiv;

        for( by = cpos = cpos2 = 0; by < height; by++, cpos += wpad, cpos2 += bwpad )
        {
            pptr = pfput + sizeof( img_hdr ) + cpos;
            gptr = pfget + sizeof( img_hdr ) + cpos;
            bptr = buf   + sizeof( img_hdr ) + cpos2;

            for( bx = 0; bx < width; bx++ )
            {
                if( *bptr )
                    *pptr = *bptr;
                else
                    *pptr = *gptr;
                ++pptr;
                ++gptr;
                bptr += picdiv;
            }
        }
    }
    else
    {
        getimage( x, y, x+width-1, y+height-1, pfget );

        clen = imagesize( 0, 0, width, height );

        /* copy header */
        memset( pfput, 0, clen );
        memcpy( pfput, pfget, sizeof( img_hdr ) );
        wpad = ((img_hdr*)pfput)->widthpadded;

        /* copy data */
        for( by = cpos = cpos2 = 0; by < height; by++, cpos += wpad, cpos2 += bwpad )
        {
            pptr = pfput + sizeof( img_hdr ) + cpos;
            gptr = pfget + sizeof( img_hdr ) + cpos;
            bptr = buf   + sizeof( img_hdr ) + cpos2;

            for( bx = 0; bx < width; bx++ )
            {
                if( *bptr )
                    *pptr = *bptr;
                else
                    *pptr = *gptr;
                ++pptr;
                ++gptr;
                ++bptr;
            }
        }
    }

    putimage( x, y, pfput, COPY_PUT );
}

/*-------------------------------------------------------------------------*
 * Circle fill functions
 *-------------------------------------------------------------------------*/

#undef line
#define hline(x1,x2,y)      line(_x+x1, _y+y, _x+x2, _y+y)
#include "gdef.h"   /* redefine line */

void gr_fillcircle( short _x, short _y, short radius )
{
#if 0
    clmou;
    fillellipse( _x, _y, radius, radius );
    stmou;
#else
    short x, y, d, DeltaE, DeltaSE;

    x = 0;
    y = radius;
    d = 1 - radius;
    DeltaE = 3;
    DeltaSE = -2 * radius + 5;

    clmou;

    hline( x, -x, y );
    hline( y, -y, x );
    hline( y, -y, -x );
    hline( x, -x, -y );

    while( y > x )
    {
        if( d < 0 )
        {
            d += DeltaE;
            DeltaE += 2;
            DeltaSE += 2;
            x ++;
        }
        else
        {
            d += DeltaSE;
            DeltaE += 2;
            DeltaSE += 4;
            x ++;
            y --;
        }

        hline( x, -x, y );
        hline( y, -y, x );
        hline( y, -y, -x );
        hline( x, -x, -y );
    }

    stmou;
#endif
}

#undef putpixel
#define hline(x1,x2,y)  { putpixel(_x+x1, _y+y, clr ); putpixel( _x+x2, _y+y, clr ); }
#include "gdef.h"   /* redefine putpixel */

void gr_drawcircle( short _x, short _y, short radius )
{
#if 0
    circle( _x, _y, radius );
#else
    short x, y, d, DeltaE, DeltaSE, clr = getcolor();

    x = 0;
    y = radius;
    d = 1 - radius;
    DeltaE = 3;
    DeltaSE = -2 * radius + 5;

    clmou;

    hline( x, -x, y );
    hline( y, -y, x );
    hline( y, -y, -x );
    hline( x, -x, -y );

    while( y > x )
    {
        if( d < 0 )
        {
            d += DeltaE;
            DeltaE += 2;
            DeltaSE += 2;
            x ++;
        }
        else
        {
            d += DeltaSE;
            DeltaE += 2;
            DeltaSE += 4;
            x ++;
            y --;
        }

        hline( x, -x, y );
        hline( y, -y, x );
        hline( y, -y, -x );
        hline( x, -x, -y );
    }

    stmou;
#endif
}
