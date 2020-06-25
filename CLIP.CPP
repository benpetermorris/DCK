/*-------------------------------------------------------------------------*
 * CLIP.C:                                                                 *
 * Functions for copying to/pasting from/deleting the clipboard.           *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals et al                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static  uchar *st_error = "Error!";
static  uchar *st_nomem =
        "Oops!  There isn't enough memory to allocate the clipboard\n"
        "buffers.  See \"managing memory\" in the "_ABBREV_" manual.";

short   nthingc;
THING   *thingclip;
short   nvertexc;
VERTEX  *vclip;
short   nlinec;
LINEDEF *lineclip;
short   nsidec;
SIDEDEFM *sideclip;
short   nsectorc;
SECTOR  *sectorclip;

static  short clippsector;
short   clipmaxx, clipminx;
short   clipmaxy, clipminy;
short   clipcenterx, clipcentery;
short   cliplevel;  /* for map elements  - what was clipped? */
static  int clipmode = -1;

static void _cb_cliplines( uchar *cliplist, short n, short *seccliplist );
static void _cb_clipsectors( uchar *cliplist, short n );
static short *_cb_clipsides( void );
static short *_cb_clipvertexes( uchar *cliplist );
static void _cb_toclip( short *vcliplist, short *scliplist, short *seccliplist );
static void _cb_fromclip( short lbase, short vbase, short sbase, short secbase );

void SetEditMode( short mode );

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static void _cb_nomem( void )
{
    message_box( MB_OK, st_error, st_nomem );
}

void cb_free( short type )
/*-------------------------------------------------------------------------*
 * Frees the clipboard buffers.                                            *
 *-------------------------------------------------------------------------*/
{
    if( type == MODE_VERTEX )
    {
        /* Free any previous allocations */
        if( nvertexc )
        {
            memfree( vclip );
            nvertexc = 0;
        }
        if( nsectorc )
        {
            memfree( sectorclip );
            nsectorc = 0;
        }
        if( nlinec )
        {
            memfree( lineclip );
            nlinec = 0;
        }
        if( nsidec )
        {
            memfree( sideclip );
            nsidec = 0;
        }
    }
    else if( type == MODE_THING )
    {
        if( nthingc )
        {
            memfree( thingclip );
            nthingc = 0;
        }
    }
}

short cb_clip( short obj_type, short csr_object )
/*-------------------------------------------------------------------------*
 * Copies objects of obj_type to the clipboard.                            *
 * Fairly simple operation:--                                              *
 *                                                                         *
 * Checks the mark list for all the marked objects, and copies them to     *
 * the appropriate array depending on the value of obj_type.               *
 * Returns the number of objects copied to the clipboard.                  *
 *-------------------------------------------------------------------------*/
{
    short   i, nmark, mpos;
    ushort  needed = 0;
    THING   *t;
    VERTEX  *v;

    if( csr_object != -1 )
        obj_mark( csr_object );

    /* Nothing marked? */
    if( !(nmark = obj_nmarked()) )
        return 0;

    if( editmode != MODE_THING )
        clipmode = editmode;

    /* Do we already have some stuff marked here? */
    switch( obj_type )
    {
    case MODE_VERTEX:
        message_box( MB_OK, st_dcksays, "Vertices can't be clipped." );
        return 0;
    case MODE_SECTOR:
        cliplevel = 2;
    case MODE_LINE:
        cliplevel = 1;
        needed = MODE_VERTEX;
        break;
    case MODE_THING:
        needed = MODE_THING;
        break;
    }

    /* Free existing stuff */
    cb_free( needed );

    clipminx = clipminy = 32000;
    clipmaxx = clipmaxy = -32000;

    switch( obj_type )
    {
/*-------------------------------------------------------------------------*
 * Clipping things (this is easy :)                                        *
 *-------------------------------------------------------------------------*/
    case MODE_THING:
        /* Allocate some memory */
        thingclip = (THING*) malloc( sizeof( THING ) * nmark );
        if( thingclip == NULL )
        {
            /* Oops! */
            _cb_nomem();
            return 0;
        }

        for( i = 0, mpos = 0; i < MAXOBJECT; i++ )
        {
            if( obj_ismarked( i ) )
            {
                /* Copy this object to the clipboard */
                t = &thinglist[i];
                /* First: Get the extent of the marked things so we can
                 * center placement later */
                if( t->x < clipminx )   clipminx = t->x;
                if( t->x > clipmaxx )   clipmaxx = t->x;
                if( t->y < clipminy )   clipminy = t->y;
                if( t->y > clipmaxy )   clipmaxy = t->y;

                /* Like I said: Copy the object */
                memcpy( &thingclip[mpos++], t, sizeof( THING ) );
                ++nthingc;
            }
        }
        break;
/*-------------------------------------------------------------------------*
 * Clippling lines                                                         *
 *-------------------------------------------------------------------------*/
    case MODE_LINE:
        /* Saves lines, sidedefs and vertexes */
        _cb_cliplines( linemarklist, nmark, NULL );
        break;
/*-------------------------------------------------------------------------*
 * Sectors                                                                 *
 *-------------------------------------------------------------------------*/
    case MODE_SECTOR:
        /* Saves sectors, lines, sidedefs, and then vertexes */
        _cb_clipsectors( marklist, nmark );
        break;
    }

    /* Now, loop again and make all coordinates relative to the
     * center of the clip area */
    clipcenterx = (clipmaxx + clipminx) / 2;
    clipcentery = (clipmaxy + clipminy) / 2;
    switch( needed )
    {
    case MODE_THING:
        /* Counts for thing edit mode */
        for( i = 0; i < nthingc; i++ )
        {
            t = &thingclip[i];
            t->x -= clipcenterx;
            t->y -= clipcentery;
        }
        break;
    case MODE_VERTEX:
        /* Counts for all the other edit modes */
        for( i = 0; i < nvertexc; i++ )
        {
            v = &vclip[i];
            v->x -= clipcenterx;
            v->y -= clipcentery;
        }
        break;
    }

    /* Now turn the centers into screen rectangle halfwidth and halfheight */
    clipcenterx = abs( (clipmaxx - clipminx) / 2 );
    clipcentery = abs( (clipmaxy - clipminy) / 2 );

    return nmark;
}

static void _cb_clipsectors( uchar *cliplist, short nclip )
/*-------------------------------------------------------------------------*
 * Saves sectors.  Check out the comments in the function.                 *
 *-------------------------------------------------------------------------*/
{
    short   i, theline, n;
    SECTOR  *secp;
    uchar   *lclipbool;
    short   *seccliplist;

    /* Allocate space for the clipboard */
    sectorclip = (SECTOR*) malloc( sizeof( SECTOR ) * nclip );
    nsectorc = nclip;

    /* seccliplist is a conversion buffer that's passed to _cb_toclip() */
    seccliplist = new short[nsector];
    /* lclipbool: linedef clip boolean array.  If a linedef is clipped,
     * its index in this array is set to 1. */
    lclipbool = new uchar[nline];
    memset( lclipbool, 0, nline );  /* all 0 at first: none clipped yet */
    nlinec = 0;

    /* Save the sectors */
    for( i = 0, secp = sectorclip, n = 0; i < nsector; i++ )
    {
        if( cliplist[i] )   /* Save this sector? */
        {
            /* Find all the lines with this sector */
            theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, i, LF_END );
            while( theline != -1 )
            {
                /* Save this line */
                if( !lclipbool[theline] )   ++nlinec;
                lclipbool[theline] = 1;
                theline = obj_linefind( LF_CONTINUE );
            }

            /* Now save the sector itself */
            memcpy( secp++, &sectorlist[i], sizeof( SECTOR ) );
            /* negative number to indicate it's a clipped sector.  works. */
            seccliplist[i] = -(++n);
        }
        else
            seccliplist[i] = i;
    }

    /* Now save the lines.  Does all the other stuff too. */
    _cb_cliplines( lclipbool, nlinec, seccliplist );
    delete lclipbool;
    delete seccliplist;
}

static void _cb_cliplines( uchar *cliplist, short nclip, short *seccliplist )
/*-------------------------------------------------------------------------*
 * Saves the lines.  WOOO WOO.                                             *
 *-------------------------------------------------------------------------*/
{
    short   i;
    LINEDEF *ldp;
    short   *vcliplist, *scliplist;
    uchar   *vclipbool;

    /* Allocate space for the clipboard */
    lineclip = (LINEDEF*) malloc( sizeof( LINEDEF ) * nclip );
    nlinec = nclip;

    /* vclipbool: vertex clipped boolean array.  if a vertex has to be
     * clipped, its index in this array is 1. */
    vclipbool = new uchar[nvertex];
    memset( vclipbool, 0, nvertex );    /* all 0 to begin with */
    nvertexc = 0;

    for( i = 0, ldp = lineclip; i < nline; i++ )
    {
        if( cliplist[i] )       /* Save this line? */
        {
            /* Yep!  So save it and determine if its vertexes need
             * to be saved (always, unless they already are.) */
            memcpy( ldp, &linelist[i], sizeof( LINEDEF ) );
            if( !vclipbool[ldp->vfrom] )    ++nvertexc;
            if( !vclipbool[ldp->vend] )     ++nvertexc;
            vclipbool[ldp->vfrom] = 1;
            vclipbool[ldp->vend] = 1;
            ldp++;
        }
    }

    /* Save vertexes */
    vcliplist = _cb_clipvertexes( vclipbool );

    /* Save sidedefs */
    scliplist = _cb_clipsides();

    /* Convert the list.  seccliplist might be NULL, but _cb_toclip()
     * handles that. */
    _cb_toclip( vcliplist, scliplist, seccliplist );
    /* Free the conversion buffers */
    free( scliplist );
    free( vcliplist );
    delete vclipbool;
}

static short *_cb_clipsides( void )
/*-------------------------------------------------------------------------*
 * Uses the list of clipped lines to build a list of clipped sides.        *
 * Returns a conversion buffer that should be passed to _cb_toclip.        *
 *-------------------------------------------------------------------------*/
{
    short     i, n;
    SIDEDEFM  *sptr;
    LINEDEF   *ldp;
    short     *icliplist = (short*) malloc( sizeof( *icliplist ) * nside );

    /* Conversion buffer */
    memset( icliplist, 0, sizeof( *icliplist ) * nside );

    /* Allocate enough space for worst-case scenario */
    sideclip = (SIDEDEFM*) malloc( sizeof( SIDEDEFM ) * (nlinec * 2) );
    nsidec = 0;

    /* Store them */
    for( i = 0, sptr = sideclip, ldp = lineclip, n = 0; i < nlinec; i++, ldp++ )
    {
        if( ldp->rsdef != -1 )      /* Save this side? */
        {
            memcpy( sptr++, &sidelist[ldp->rsdef], sizeof( SIDEDEFM ) );
            ++nsidec;
            icliplist[ldp->rsdef] = n++;
        }
        if( ldp->lsdef != -1 )      /* Save this side? */
        {
            memcpy( sptr++, &sidelist[ldp->lsdef], sizeof( SIDEDEFM ) );
            ++nsidec;
            icliplist[ldp->lsdef] = n++;
        }
    }

    /* use "realloc" to resize the buffer to the real size */
    sideclip = (SIDEDEFM*) realloc( sideclip, sizeof( SIDEDEFM ) * nsidec );

    return icliplist;
}

static short *_cb_clipvertexes( uchar *cliplist )
/*-------------------------------------------------------------------------*
 * Saves a list of vertexes in the global list.  Returns a pointer to a    *
 * conversion buffer that should be passed to _cb_toclip.                  *
 *-------------------------------------------------------------------------*/
{
    short     i, n;
    VERTEX  *vptr;
    short     *icliplist = (short*) malloc( sizeof( *icliplist ) * nvertex );

    /* Allocate space for it */
    vclip = (VERTEX*) malloc( sizeof( VERTEX ) * nvertexc );

    /* Store them */
    for( i = 0, vptr = vclip, n = 0; i < nvertex; i++ )
    {
        if( cliplist[i] )
        {
            memcpy( vptr, &vlist[i], sizeof( VERTEX ) );
            icliplist[i] = n++;
            if( vptr->x < clipminx )   clipminx = vptr->x;
            if( vptr->x > clipmaxx )   clipmaxx = vptr->x;
            if( vptr->y < clipminy )   clipminy = vptr->y;
            if( vptr->y > clipmaxy )   clipmaxy = vptr->y;
            ++vptr;
        }
        else
            icliplist[i] = 0;
    }

    return icliplist;
}

static void _cb_toclip( short *vcliplist, short *scliplist, short *seccliplist )
/*-------------------------------------------------------------------------*
 * Converts all the vertex references in the clipboard line list to        *
 * references to the vclip[] buffer.  Same with sidedefs.                  *
 * Also converts all sector references in the side list to the new         *
 * sectors.                                                                *
 *-------------------------------------------------------------------------*/
{
    short         i;
    LINEDEF     *ldp;
    SIDEDEFM    *sdp;

    for( i = 0, ldp = lineclip; i < nlinec; i++, ldp++ )
    {
        ldp->vfrom = vcliplist[ldp->vfrom];
        ldp->vend = vcliplist[ldp->vend];
        if( ldp->lsdef != -1 )
            ldp->lsdef = scliplist[ldp->lsdef];
        if( ldp->rsdef != -1 )
            ldp->rsdef = scliplist[ldp->rsdef];
    }

    if( seccliplist )
        for( i = 0, sdp = sideclip; i < nsidec; i++, sdp++ )
            sdp->sector = seccliplist[sdp->sector];
}

static void _cb_fromclip( short lbase, short vbase, short sbase, short secbase )
/*-------------------------------------------------------------------------*
 * Converts all vertex/sidedef references in the clipboard list to new     *
 * objects by adding the appropriate base.                                 *
 *-------------------------------------------------------------------------*/
{
    short       i, i2;
    LINEDEF     *ldp;
    SIDEDEFM    *sdp;

    for( i = lbase, ldp = linelist+lbase; i < nline; i++, ldp++ )
    {
        ldp->vfrom += vbase;
        ldp->vend += vbase;

        if( ldp->rsdef != -1 )
            ldp->rsdef += sbase;

        if( ldp->lsdef != -1 )
            ldp->lsdef += sbase;
        else if( clippsector != -1 )
        {
            /* we're pasting inside another sector: if this side's opposite
             * side faces that sector, make it two-sided */
            if( obj_getphyssector( i, 1 ) == clippsector )
            {
                ldp->lsdef = obj_insert( MODE_SIDEDEF, -1, 0, 0 );
                sidelist[ldp->lsdef].sector = clippsector;
                /* make the line two-sided */
                ldp->attribs &= ~LA_IMPASS;
                ldp->attribs |= LA_TSIDE;
            }
        }
    }

    for( i = sbase, sdp = sidelist+sbase; i < nside; i++, sdp++ )
    {
        if( sdp->sector < 0 )
        {
            sdp->sector = -(sdp->sector) - 1;
            sdp->sector += secbase;
        }
        else if( editmode == MODE_SECTOR )
        {
            /* this sidedef's sector reference is not within the clip buffer.
             * if clippsector is not -1, just set the reference to clippsector.
             * otherwise, delete the sidedef, make its linedef single-sided. */

            if( clippsector != -1 )     /* set it to the parent sector */
                sdp->sector = clippsector;
            else
            {
                /* find lines that reference this sidedef and delete
                 * the reference */
                for( i2 = 0, ldp = linelist+lbase; i2 < nlinec; i2++, ldp++ )
                {
                    if( ldp->lsdef == i )
                        ldp->lsdef = -1;
                    else if( ldp->rsdef == i )
                    {
                        if( ldp->lsdef != -1 )
                        {
                            /* flip the line around */
                            swap2b( ldp->lsdef, ldp->rsdef );
                            swap2b( ldp->vfrom, ldp->vend );
                            ldp->lsdef = -1;
                        }
                    }
                    else
                        continue;

                    ldp->attribs &= ~LA_TSIDE;
                    ldp->attribs |= LA_IMPASS;
                }
            }
        }
    }

    for( i = lbase; i < nline; i++ )
    {
        /* fix textures */
        _WFfixtexnames( i, 0, 0 );
        _WFfixtexnames( i, 1, 0 );
    }

}

short cb_isempty( void )
/* Clipboard empty? */
{
    return !(nthingc || nvertexc || nlinec || nsectorc || nsidec);
}

static short _cb_pastevertexes( short x, short y )
/*-------------------------------------------------------------------------*
 * Pastes vertexes.  Pretty easy stuff.                                    *
 *-------------------------------------------------------------------------*/
{
    short     i;
    VERTEX  *vtxp;

    for( i = 0, vtxp = vlist+nvertex; i < nvertexc; i++, vtxp++ )
    {
        memcpy( vtxp, &vclip[i], sizeof( VERTEX ) );
        vtxp->x += x;
        vtxp->y += y;
    }

    nvertex += nvertexc;
    return nvertexc;
}

static short _cb_pastelines( short x, short y, bool fPastingSectors )
/*-------------------------------------------------------------------------*
 * Pastes lindedefs, sidedefs and vertexes.                                *
 *-------------------------------------------------------------------------*/
{
    short     npaste = 0;

    /* Paste the linedefs first. */
    memcpy( linelist+nline, lineclip, nlinec * sizeof( LINEDEF ) );
    nline += nlinec;
    npaste += nlinec;

    /* Now, paste sidedefs. */
    memcpy( sidelist+nside, sideclip, nsidec * sizeof( SIDEDEFM ) );
    nside += nsidec;

    /* Now vertexes.  (this function increments the number of vertexes
     * so we don't have to.) */
    npaste += _cb_pastevertexes( x, y );

    /* Convert the vertex/side references. */
    _cb_fromclip( nline-nlinec, nvertex-nvertexc, nside-nsidec,
                  fPastingSectors ? nsector-nsectorc : 0 );

    return npaste;
}

static short _cb_pastesectors( short x, short y )
/*-------------------------------------------------------------------------*
 * Pastes sectors.                                                         *
 * ALso pastes linedefs, sidedefs and vertexes.                            *
 *-------------------------------------------------------------------------*/
{
    short npaste = 0;

    /* This is easy.  Just copy the clipped items to the end of the
     * real list and up the number of sectors. */
    memcpy( sectorlist+nsector, sectorclip, nsectorc * sizeof( SECTOR ) );
    nsector += nsectorc;
    npaste += nsectorc;

    /* Paste linedefs now (which also pastes vertexes and sidedefs.) */
    npaste += _cb_pastelines( x, y, TRUE );
    return npaste;
}

short cb_paste( short obj_type, short x, short y )
/*-------------------------------------------------------------------------*
 * Pastes all clipped objects relative to x, y (assumed to be the desired  *
 * center of the paste area.)  Returns the number of objects pasted.       *
 *                                                                         *
 * Note: the clipboard is NOT modified by this routine.                    *
 *-------------------------------------------------------------------------*/
{
    short     i, tmp, npaste = 0;
    THING   *t;

    undo_init();

    if( editmode != MODE_THING && editmode != clipmode && clipmode != -1 )
    {
        SetEditMode( clipmode );
        obj_type = clipmode;
    }
    else
        post_fullupdate();

    /* Snap the coordinates */
    _mapsnap( &x, &y, x, y );

    obj_clearmarks( -1, 0 );
    clippsector = -1;

    switch( obj_type )
    {
/*-------------------------------------------------------------------------*
 * Paste some things.                                                      *
 *-------------------------------------------------------------------------*/
    case MODE_THING:
        memset( marklist+nthing, 1, nthingc );
        /* Simply loop through the things and make them all relative to the
         * cursor coordinates.. */
        for( i = 0; i < nthingc; i++ )
        {
            t = &thingclip[i];

            /* Add it to the thing list */
            tmp = obj_insert( MODE_THING, -1, 0, 0 );
            memcpy( &thinglist[tmp], t, sizeof( THING ) );
            t = &thinglist[tmp];
            t->x += x;
            t->y += y;
        }

        npaste = nthingc;
        break;
/*-------------------------------------------------------------------------*
 * Paste other objects.                                                    *
 *-------------------------------------------------------------------------*/
    case MODE_SECTOR:
        clippsector = obj_get( MODE_SECTOR, x, y, x, y );
        memset( marklist+nsector, 1, nsectorc );
        npaste = _cb_pastesectors( x, y );
        if( npaste )
            nMapChanges++;
        break;
    case MODE_LINE:
        memset( marklist+nline, 1, nlinec );
        npaste = _cb_pastelines( x, y, FALSE );
        if( npaste )
            nMapChanges++;
        break;
    }

    if( npaste )
        nMadeChanges++;

    undo_setvars();

    return npaste;
}

short cb_draw( short x, short y )
/*-------------------------------------------------------------------------*
 * Draws the clipboard rectangle.                                          *
 * X and Y are screen cursor coordinates.                                  *
 *-------------------------------------------------------------------------*/
{
    static  short myx, myy;
    short     sx, sy;

    if( x != -1 )  /* Snap the coordinates */
    {
        _mapsnap( &myx, &myy, xstart + _scrx2map( x ), ystart + _scry2map( y ) );
        myx = _mapx2scr( myx - xstart );
        myy = _mapy2scr( myy - ystart );
    }

    sx = _mapx2scr( clipcenterx );
    sy = _absmapy2scr( clipcentery );

    cursor_put( CSR_ERASE );
    gr_setwritemode( SWM_XOR );
    gr_setlinestyle( SLS_SOLID, 1 );
    gr_box( myx-sx, myy-sy, myx+sx, myy+sy, C_WHITE );
    gr_setwritemode( SWM_COPY );
    cursor_put( CSR_PUT );

    return 1;
}
