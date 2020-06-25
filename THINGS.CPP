/*-------------------------------------------------------------------------*
 * THINGS.C:                                                               *
 * Stuff for things.                                                       *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

static short fEditMarked;

#pragma inline

#include "gui\dialog.hpp"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

short thing_color( short type )
{
    short   n = pgdCurGame->ttype2idx[type];

    if( n == 0xFF )
        return C_WHITE;

    return guito256[pgdCurGame->thinginfo[n].color][3];
}

short thing_height( short type )
/*-------------------------------------------------------------------------*
 * Returns a thing's height                                                *
 *-------------------------------------------------------------------------*/
{
    short   n = pgdCurGame->ttype2idx[type];

    if( n == 0xFF )
        return 20;

    return pgdCurGame->thinginfo[n].height;
}

short thing_mapradius( short type )
/*-------------------------------------------------------------------------*
 * Returns radius for a thing in map coords.                               *
 *-------------------------------------------------------------------------*/
{
    short   n = pgdCurGame->ttype2idx[type];

    if( n == 0xFF )
        return 16;

    return pgdCurGame->thinginfo[n].radius;
}

short thing_radius( short type )
/*-------------------------------------------------------------------------*
 * Returns radius for a thing in pixels.                                   *
 *-------------------------------------------------------------------------*/
{
    short   n = pgdCurGame->ttype2idx[type];

    if( n == 0xFF )
        return _mapx2scr( 16 );

    return _mapx2scr( pgdCurGame->thinginfo[n].radius );
}

char *thing_visstring( short when )
{
    static char temp[40];
    char n = 0;

    strcpy( temp, "" );
    if( when & 0x01 )
        strcat( temp, "1-2" ), ++n;
    if( when & 0x02 )
    {
        if( n ) strcat( temp, "-" );
        strcat( temp, "3" ), ++n;
    }
    if( when & 0x04 )
    {
        if( n ) strcat( temp, "-" );
        strcat( temp, "4-5" );
    }

    if( when & 0x10 )
        strcat( temp, " [multi]" );

    if (when & 0x08)
        strcat( temp, " [deaf]" );
    return temp;
}

short thing_markpattern( short object )
/*-------------------------------------------------------------------------*
 * Marks a pattern of things.                                              *
 * Uses the same dialog box as the original, but instead: if object is not *
 * -1, copies search attributes from there..                               *
 *-------------------------------------------------------------------------*/
{
#if 0       /* MODIFY !!!!!!!!!!!!!!! */
    THING   *t;
    short     newangle, newtype, i, nmarked;

    if( object != -1 )
    {
        t = &thinglist[object];
        fAmbush = t->attribs & TA_DEAF    ? 1 : 0;     /* set the attributes */
        fEasy = t->attribs & TA_SKILL12 ? 1 : 0;
        fNormal   = t->attribs & TA_SKILL3  ? 1 : 0;
        fHard = t->attribs & TA_SKILL45 ? 1 : 0;
        fDeathmatch = t->attribs & TA_GDEATHMATCH ? 1 : 0;
        strcpy( fszThingType, ThgTypeToStr( t->type ) );
    }
    else
    {
        fAmbush = fEasy = fNormal = fHard = fDeathmatch = 2;
        t_angle = -1;
        *fszThingType = 0;
    }

    _create_tdbox( 1 );     /* multiple .. well, it sets 3-way checkboxes */

    dlg_setorglines( tdlg, -1, -1, 2 );
    dlg_settitle( tdlg, "Thing search; enter pattern" );
    dlg_setxy( tdlg, Centered, Centered );

    if( dlg_proc( 0, tdlg ) == DF_NOSAVE )
        return 0;

    newtype = *fszThingType ? ThgStrToType( fszThingType ) : -1;

    /* Search for things and mark 'em */
    for( i = 0, t = thinglist, nmarked = 0; i < nthing; i++, t++ )
    {
        /* don't check for angles.. waste of time. */

        if( newtype != -1 && t->type != newtype )
            continue;       /* no match */
        if( fAmbush != 2 && ((t->attribs & TA_DEAF) ? 1 : 0) != fAmbush )
            continue;       /* no match */
        if( fEasy != 2 && ((t->attribs & TA_SKILL12) ? 1 : 0) != fEasy )
            continue;       /* no match */
        if( fNormal != 2 && ((t->attribs & TA_SKILL3) ? 1 : 0) != fNormal )
            continue;       /* no match */
        if( fHard != 2 && ((t->attribs & TA_SKILL45) ? 1 : 0) != fHard )
            continue;       /* no match */
        if( fDeathmatch != 2 && ((t->attribs & TA_GDEATHMATCH) ? 1 : 0) != fDeathmatch )
            continue;       /* no match */

        if( !obj_ismarked( i ) && object != i )    /* Hilite it */
            obj_highlight( MODE_THING, i, COLOR_MARKED );

        obj_mark( i );
        ++nmarked;
    }

    return nmarked;
#endif

    return 0;
}

void thing_autosetangle( short object )
/*-------------------------------------------------------------------------*
 * Using the angle of the angle system, angles the angular angle from      *
 * the thing origin to the thing coordinates.                              *
 *-------------------------------------------------------------------------*/
{
    short   angle, tmpint;
    THING   *tptr = &thinglist[object];
    extern  short   xend, yend;     /* x and y opposite coordinates of xstart and ystart */

    if( torigin_x == -1 )   /* no origin set */
        return;

    if( !inrange( torigin_x, xstart, xend ) ||
        !inrange( torigin_y, ystart, yend ) )
        return;

    /* get the angle from the thing to the origin */
    angle = _angle( tptr->x, tptr->y, torigin_x, torigin_y );

    tmpint = angle % 45;
    angle -= tmpint;    /* round down */

    if( tmpint >= 23 )
        angle += 45;    /* round up if needed */

    angle %= 360;

    tptr->angle = angle;
}

short td_check( ushort attribs )
// returns 1 on "active", 0 on "inactive"
{
#ifdef REGISTERED
    if( fHexen )
    {
    if( !bittest( attribs, td_flags.skill1, TA_SKILL12 ) ||
        !bittest( attribs, td_flags.skill2, TA_SKILL3 ) ||
        !bittest( attribs, td_flags.skill3, TA_SKILL45 ) ||

        !bittest( attribs, td_flags.fighter, TA_FIGHTER ) ||
        !bittest( attribs, td_flags.cleric, TA_CLERIC ) ||
        !bittest( attribs, td_flags.mage, TA_MAGE ) ||

        !bittest( attribs, td_flags.gsingle, TA_GSINGLE ) ||
        !bittest( attribs, td_flags.gcoop, TA_GCOOP ) ||
        !bittest( attribs, td_flags.gdeath, TA_GDEATHMATCH ) ||

        !bittest( attribs, td_flags.ambush, TA_AMBUSH ) ||
        !bittest( attribs, td_flags.dormant, TA_DORMANT ) )
        return 0;
    }
    else
    {
    if( !bittest( attribs, td_flags.skill1, TA_SKILL12 ) ||
        !bittest( attribs, td_flags.skill2, TA_SKILL3 ) ||
        !bittest( attribs, td_flags.skill3, TA_SKILL45 ) ||

        !bittest( attribs, td_flags.ambush, TA_AMBUSH ) ||
        !bittest( attribs, td_flags.multiplayer, TA_MPLAYER ) )
        return 0;
    }
#endif
    return 1;
}

short td_getflags( void )
/*-------------------------------------------------------------------------*
 * Gets thing display flags.
 *-------------------------------------------------------------------------*/
{
#ifndef REGISTERED
    Unregistered();
    return FALSE;
#else
    VarTransfer_t vtThingMask[] =
    {
        { "OnEasy", NULL, (PTR) &td_flags.skill1, NULL },
        { "OnNormal", NULL, (PTR) &td_flags.skill2, NULL },
        { "OnHard", NULL, (PTR) &td_flags.skill3, NULL },
        { "OnFighter", NULL, (PTR) &td_flags.fighter, NULL },
        { "OnCleric", NULL, (PTR) &td_flags.cleric, NULL },
        { "OnMage", NULL, (PTR) &td_flags.mage, NULL },
        { "OnSingle", NULL, (PTR) &td_flags.gsingle, NULL },
        { "OnCoop", NULL, (PTR) &td_flags.gcoop, NULL },
        { "OnDeath", NULL, (PTR) &td_flags.gdeath, NULL },
        { "Ambush", NULL, (PTR) &td_flags.ambush, NULL },
        { "Dormant", NULL, (PTR) &td_flags.dormant, NULL },

        // end marker
        { NULL, NULL, NULL, NULL }
    };

    Dialog* pDialog = new Dialog;
    pDialog->Create( szDSFile, fHexen ? "HexenThingMaskDlg" :
        "DoomThingMaskDlg" );
    pDialog->VarSetup( vtThingMask );

    int iRvl = pDialog->Process();

    delete pDialog;

    return (iRvl == daExitSave);
#endif
}

/*-------------------------------------------------------------------------*
 *
 * Thing picture sprite fileZz
 *
 *-------------------------------------------------------------------------*/

extern  img_hdr     *pbhdr;
extern  char        *picbuf;
extern  unsigned    pblen;

static  PIC_HDR     ts_hdr;
byte    ts_loaded;
byte    ts_reload;
char    szThingFile[128];

#define nImgSize        2
#define nMaxImage       255
#define TS_ENDMARKER    "*TS_END*"

short spr2picbuf( short x, short y, short handle, long offset, short center, int );

/* array of images loaded from disk - corresponds to the thinginfo
 * array in the current gamedata structure. */
typedef struct
{
    unsigned imglen;
    short    x, y;
    img_hdr  *picture;

} imgRec;

imgRec images[nMaxImage];

short ts_mkpic( WAD* wptr, char *name )
/*-------------------------------------------------------------------------*
 * Displays a picture from the iwad.
 *-------------------------------------------------------------------------*/
{
    short       handle = z_open( wptr->fn, O_BINARY | O_RDONLY );
    WADENTRY    *weptr = wad_find( wptr, name );

    extern      SRECT prb;

    if( !weptr )
    {
        z_close( handle );
        return 0;
    }

    lseek( handle, weptr->offset, SEEK_SET );
    z_read( handle, &ts_hdr, sizeof ts_hdr );
    mkpicbuf( ts_hdr.width, ts_hdr.height );

    prb.left = 0;
    prb.right = ts_hdr.width;
    prb.top = 0;
    prb.bottom = ts_hdr.height;

    spr2picbuf( 0, 0, handle, weptr->offset, 1, 100 );

    z_close( handle );

    return 1;
}

void ts_make( void )
/*-------------------------------------------------------------------------*
 * Using the current gamedata structure, makes a thing sprite file
 * from all the sprites in the thinginfo array.
 *-------------------------------------------------------------------------*/
{
    FILE        *outfp;
    short       i, imgnum;
    unsigned    tnumpos;
    TInfo       *tiptr;
    WAD*        wptr;
    GWINDOW     *wnp;
    char        spname[10];
    imgRec      rec, *irptr;
    short       nimg_game = 0, nimg = 0, sum = 0;
    STRTABLE    *stbl = strtbl_init( "ThingSprites", 512, 9 );

    sprintf( szThingFile, "%s.TS", pgdCurGame->cfgname );

    wnp = wn_def( Centered, Centered, 30, 3, C_GRAY3, C_GRAY1, WF_POPUP | WF_3D,
                  "Creating Item Graphics" );
    wn_disp( wnp );

    outfp = fopen( szThingFile, "wb" );
    fwrite( "Thing Sprite File\r\n\x1A", 50, 1, outfp );
    fwrite( &file_version, sizeof( short ), 1, outfp );

    wptr = wad_open( pgdCurGame->texwads[0], WL_COMPLETE );

    /* reset the number of images in this game */
    nimg_game = 0;
    tnumpos = ftell( outfp );

    fwrite( &nimg_game, sizeof( short ), 1, outfp );
    memset( pgdCurGame->tnum2img, 0xFF, sizeof pgdCurGame->tnum2img );
    fwrite( pgdCurGame->tnum2img, sizeof pgdCurGame->tnum2img, 1, outfp );

    tiptr = pgdCurGame->thinginfo;

    spname[8] = 0;

    for( ; tiptr->type != -1; tiptr++ )
    {
        memcpy( spname, tiptr->sprite, 8 );

        imgnum = strtbl_find( stbl, spname );

        if( spname[0] != '*' && spname[0] )
        {
            if( imgnum == -1 )
            {
                /* only add it if it's not there eh. */
                if( ts_mkpic( wptr, spname ) )
                {
                    strtbl_add( stbl, spname, 1 );

                    rec.imglen = pbhdr->widthpadded * pbhdr->height + sizeof( *pbhdr );
                    rec.x = ts_hdr.width;
                    rec.y = ts_hdr.height;

                    irptr = &images[nimg];

                    memcpy( irptr, &rec, sizeof rec );
                    irptr->picture = (img_hdr*) memget( rec.imglen );
                    memcpy( irptr->picture, picbuf, rec.imglen );

                    fwrite( &rec, sizeof rec - sizeof( img_hdr* ), 1, outfp );
                    fwrite( picbuf, rec.imglen, 1, outfp );

                    nimg_game++;
                }
                else
                    imgnum = -2;
            }

            if( imgnum != -2 )
            {
                if( !(nimg % 10) )
                    wn_xytext( wnp, Centered, 1, "Processed: %-03d", nimg );

                if( imgnum == -1 )
                    imgnum = nimg++;

                /* set the thing number to the image number stored */
                pgdCurGame->tnum2img[tiptr->type] = imgnum;
            }
        }
    }

    /* rewrite this info */
    fseek( outfp, tnumpos, SEEK_SET );
    fwrite( &nimg_game, sizeof( short ), 1, outfp );
    fwrite( pgdCurGame->tnum2img, sizeof pgdCurGame->tnum2img, 1, outfp );
    fseek( outfp, 0L, SEEK_END );

    wad_close( wptr );

    /* write out end marker and close the file */
    fclose( outfp );

    ts_loaded = 1;
    ts_reload = 0;

    strtbl_free( stbl );

    wn_undisp( wnp );
    wn_free( wnp );
}

void ts_load( void )
/*-------------------------------------------------------------------------*
 * Loads the TS file.
 *-------------------------------------------------------------------------*/
{
    short   i, nimg_game, sum, version;
    char    name[50];
    imgRec  *irptr;
    GWINDOW *wnp;

    sprintf( szThingFile, "%s.TS", pgdCurGame->cfgname );
    FILE *fp = fopen( szThingFile, "rb" );

    if( ts_loaded )
    {
        // kill current list
        for( i = 0; i < nMaxImage; i++ )
        {
            if( images->picture )
                memfree( images->picture );
            images->picture = NULL;
        }
    }

    if( fp == NULL )
    {
_make_it:
        ts_make();
        return;
    }

    setvbuf( fp, NULL, _IOFBF, 16384 );

    fseek( fp, 50L, SEEK_SET );
    fread( &version, sizeof( short ), 1, fp );

    if( version != file_version )
    {
        fclose( fp );
        goto _make_it;
    }

    wnp = wn_def( Centered, Centered, 30, 3, C_GRAY3, C_GRAY1, WF_POPUP | WF_3D,
                  st_dcksays );
    wn_disp( wnp );
    wn_xytext( wnp, Centered, 1, "Loading Sprites File" );

    irptr = images;

    /* read in number of images & image conversion */
    fread( &nimg_game, sizeof( short ), 1, fp );
    fread( pgdCurGame->tnum2img, sizeof pgdCurGame->tnum2img, 1, fp );

    for( i = 0; i < nimg_game; i++, irptr++ )
    {
        if( fread( irptr, sizeof( imgRec ) - sizeof( img_hdr* ), 1, fp ) < 1 )
            break;
        irptr->picture = (img_hdr*) memget( irptr->imglen );
        fread( irptr->picture, irptr->imglen, 1, fp );
    }

    fclose( fp );

    ts_loaded = 1;
    ts_reload = 0;

    wn_undisp( wnp );
    wn_free( wnp );
}

short ts_draw( short x, short y, short type )
/*-------------------------------------------------------------------------*
 * Draws a thing sprite.
 *-------------------------------------------------------------------------*/
{
#ifndef REGISTERED
    return FALSE;
#else
    uchar   imgnum;
    imgRec  *irptr;
    struct  viewporttype vp;

    static  short   lxscale, picdiv, picdivt2;
    void    putimage_bg( short x, short y, uchar *buf, short picdiv );

    if( !ts_loaded || ts_reload )
    {
        getviewsettings( &vp );
        ts_load();
        /* window functions in ts_load change viewport.. sigh ! */
        SetViewPort( vp.left, vp.top, vp.right, vp.bottom );
    }

    imgnum = pgdCurGame->tnum2img[type];

    /* not a valid thing */
    if( imgnum == 0xFF )
        return 0;

    irptr = &images[imgnum];

    if( lxscale != xscale )
    {
        picdiv = (xscale <= config.tgr_size2) ? 1 : 2;
        picdivt2 = picdiv * 2;
    }

    putimage_bg( x-irptr->x/picdivt2, y-irptr->y/picdivt2,
                 (uchar*) irptr->picture, picdiv );

    return 1;
#endif
}

