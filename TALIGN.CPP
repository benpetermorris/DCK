/*-------------------------------------------------------------------------*
 * TALIGN.C:
 * Texture alignment functions.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "fields.hpp"

/*-------------------------------------------------------------------------*
 *
 * Globals begin here.
 *
 *-------------------------------------------------------------------------*/

extern  short       _objhigh_dotagged;
static  short       mode;           /* 0 = horiz, 1 = vert, 2 = both */
static  SIDEDEFM    *sdprev;        /* last aligned side */
static  TEXTURE     *texprev;       /* previous texture used for alignment */

/*-------------------------------------------------------------------------*
 *
 * Code begins here.
 *
 *-------------------------------------------------------------------------*/

#define ldp2num(ldp)        ((size_t) (((size_t)ldp-(size_t)linelist) / sizeof( LINEDEF )))

static short get_amode( void )
{
    short rvl = MenuPop( mode, " Alignment Mode ", "ALIGNMODE",
                         " Horizontal  ", "Align textures by line length (left-right)",
                         " Vertical    ", "Align textures by sector height (top-down)",
                         " Both        ", "Align textures both ways",
                         NULL );
    SetViewPort( mxpstart, mypstart-1, mxpstart+mxplen, mypstart+myplen+1, 1 );

    if( rvl == -1 )
        return -1;

    else return mode = rvl;
}

static void t_error( LINEDEF *ldp, unsigned sd )
{
    message_box( MB_OK, "Error",
        "Alignment problem occured at linedef %d,\n"
        "sidedef %d.", ldp2num( ldp ), sd );
}

static short ld_len( LINEDEF *ldp )
{
    VERTEX  *v = &vlist[ldp->vfrom],
            *v2 = &vlist[ldp->vend];
    return line_len( v->x, v->y, v2->x, v2->y );
}

short t_align( LINEDEF *lprev, LINEDEF *lcur, short sector, short whichside )
/*-------------------------------------------------------------------------*
 * Operates on lcur.
 * Returns 1 : ok, 0 : not ok.
 *-------------------------------------------------------------------------*/
{
    SIDEDEFM    *sdp;
    TEXTURE     *t = NULL;
    SECTOR      *this_sec, *prev_sec, *opp_sec;
    short       sd1, sd2;
    short       lprevlen;
    short       x, y;
    short       height_diff;

    /* determine which sidedef to work with */
    if( sector == -1 )      /* not working with sectors, working with sides */
    {
        if( !whichside && lcur->rsdef != -1 ) /* recurse! */
        {
            sdp = &sidelist[lcur->rsdef];
            opp_sec = lcur->lsdef == -1 ? NULL : &sectorlist[sidelist[lcur->lsdef].sector];
        }
        else if( lcur->lsdef != -1 && whichside )
        {
            sdp = &sidelist[lcur->lsdef];
            opp_sec = lcur->rsdef == -1 ? NULL : &sectorlist[sidelist[lcur->rsdef].sector];
        }
        else    /* done */
            return 0;
    }
    else
    {
        /* find the side on the line with the sector */
        if( sidelist[lcur->rsdef].sector == sector )
        {
            sdp = &sidelist[lcur->rsdef];
            opp_sec = lcur->lsdef == -1 ? NULL : &sectorlist[sidelist[lcur->lsdef].sector];
        }
        else if( lcur->lsdef != -1 && sidelist[lcur->lsdef].sector == sector )
        {
            sdp = &sidelist[lcur->lsdef];
            opp_sec = lcur->rsdef == -1 ? NULL : &sectorlist[sidelist[lcur->rsdef].sector];
        }
        else
        {
            t_error( lcur, -1 );
            return 0;
        }
    }

    /* check for same textures.. for now. */
    if( sdp->ftxidx )
        t = &texlist[sdp->ftxidx];
    else if( sdp->ltxidx )
        t = &texlist[sdp->ltxidx];
    else if( sdp->utxidx )
        t = &texlist[sdp->utxidx];

    if( lprev == NULL || sdprev == NULL || t == NULL )
    {
_a_begin:
        /* begin alignment. */
        sdp->wxofs = 0;
        sdp->wyofs = 0;

        sdprev = sdp;
        texprev = t;

        return 1;
    }

    this_sec = &sectorlist[sdp->sector];
    prev_sec = &sectorlist[sdprev->sector];

    /* not the same texture, so begin again */
    if( t != texprev )
        goto _a_begin;

    /* get the last alignment */
    x = sdprev->wxofs;
    y = sdprev->wyofs;

    /* get the previous line's length */
    lprevlen = ld_len( lprev );

    /* X ALIGNMENT */
    x += lprevlen;      /* add previous line's length */
    /* check for overflow */
    if( x >= t->width )
        x = x % t->width;

    if( mode != 1 )
        sdp->wxofs = x;

    /* Y ALIGNMENT */
    height_diff = this_sec->cheight - prev_sec->cheight;
    y -= height_diff;

    if( mode != 0 )
        sdp->wyofs = y;

    if( config.tog_unpeg &&
        !(this_sec->sectag) && opp_sec && !(opp_sec->sectag) )
    {
        if( sdp->utxidx )
            lcur->attribs |= LA_UUNPEGGED;
        if( sdp->ltxidx )
            lcur->attribs |= LA_LUNPEGGED;
    }

    sdprev = sdp;
    texprev = t;

    ++nMadeChanges;

    return 1;
}

short *st_llist, *stllp;

LINEDEF *findpath( LINEDEF *lprev, short sector )
/*-------------------------------------------------------------------------*
 * Finds the next line connected to lprev that has a sidedef referencing
 * "sector."
 *-------------------------------------------------------------------------*/
{
    static  short llist[512];
    static  short vf_idx;
    short   theline, *llp;
    LINEDEF *ldp;

    if( lprev == NULL )  /* find first line for this alignment */
    {
        /* search for a start line.. doesn't really matter what number it
         * is, as long as it has a side connected to "sector".  make sure
         * it's not in the list of startlines before we use it. */
        theline = obj_linefind( LF_BEGIN,
                                LF_SIDEWITHSEC, sector,
                                LF_END );

        while( theline != -1 )
        {
            /* search for it in startlines list */
            for( llp = st_llist; llp != stllp; llp++ )
            {
                if( *llp == theline )
                    break;
            }

            if( llp == stllp )     /* this one's ok */
                break;

            theline = obj_linefind( LF_CONTINUE );  /* nope !  next one please */
        }

        if( theline == -1 )
            return NULL;

        *stllp++ = theline;

        llp = llist;
        *llp++ = theline;
        *llp = -1;

        goto _returnit;
    }

    /* find a line connected to the previous line */
    theline = obj_linefind( LF_BEGIN,
                            LF_SIDEWITHSEC, sector,
                            LF_VANY, vf_idx,
                            LF_END );
    if( theline != -1 )
    {
        while( theline != -1 )
        {
            /* see if the line is already there */
            for( llp = llist; *llp != -1; llp++ )
            {
                if( theline == *llp )
                    break;  /* oops .. already done this line. */
            }

            if( *llp == -1 )   /* if so, it's not been done yet. */
            {
                *stllp++ = theline; /* can't start here either */
                *llp++ = theline;
                *llp = -1;       /* add it to the list, and terminator */
                goto _returnit;
            }

            theline = obj_linefind( LF_CONTINUE );
        }
    }

    return NULL;    /* no more */

_returnit:
    /* got a line number in theline .. post-find processing here. */
    ldp = &linelist[theline];

    /* tell it which vertex to look for next time */
    if( sidelist[ldp->rsdef].sector == sector )
        vf_idx = ldp->vend;
    else
        vf_idx = ldp->vfrom;

    return ldp;  /* return it */
}

LINEDEF *findlinepathmarked( LINEDEF *lprev, short eos, short *whichside )
/*-------------------------------------------------------------------------*
 * Finds the next line connected to lprev that's not in the list of
 * already found lines .. the line must be marked too, of course.
 *
 * eos is "end or start" : the vertex from lprev to search for.
 *
 * if lprev is null, this function searches for a line that is connected
 * to only one other marked line - this line might be used as the start
 * of the alignment - IF it's connected to a line going in the same
 * direction.  In either case, the line is added to the st_llist array,
 * which is checked for when lprev is null to make sure that we don't
 * align a set of lines twice.
 *-------------------------------------------------------------------------*/
{
    static  short llist[128];
    static  short vf_idx;
    static  LINEDEF *ldp;
    LINEDEF *ldp2;
    short   *llp;
    short   theline, i, theline2;

    if( lprev != NULL )
    {
        if( !eos )
            vf_idx = lprev->vend;    /* only valid here eh */
        else
            vf_idx = lprev->vfrom;
        llp = llist;
        *llp++ = ldp2num( lprev );
        *llp = -1;
        return NULL;
    }

    if( eos == 2 )
    {
        /* try to find the start of the next series of lines and return it
         * without doing any alignment.  the line must be */
        for( i = 0; i < nline; i++ )
        {
            if( obj_ismarked( i ) )
            {
                /* check to see if this line has been used already */
                for( llp = st_llist; llp != stllp; llp++ )
                {
                    if( *llp == i )
                        break;
                }

                /* this line is okay */
                if( llp == stllp )
                {
                    ldp = &linelist[i];

                    /* check to see if it's a valid startline - a marked
                     * line must be attached to its end vertex, and no
                     * marked line must be attached to its start. */
                    theline = obj_linefind( LF_BEGIN, LF_VANY, ldp->vend, LF_END );
                    while( theline != -1 )
                    {
                        if( theline != i && obj_ismarked( theline ) )
                            break;
                        theline = obj_linefind( LF_CONTINUE );
                    }

                    if( theline != -1 )
                    {
                        /* make sure no marked line is attached to its start line */
                        theline = obj_linefind( LF_BEGIN, LF_VANY, ldp->vfrom, LF_END );
                        while( theline != -1 )
                        {
                            if( theline != i && obj_ismarked( theline ) )
                                break;
                            theline = obj_linefind( LF_CONTINUE );
                        }

                        /* it's a valid startpoint: add it to the list and return it */
                        if( theline == -1 )
                        {
                            *whichside = 0;
                            *stllp++ = i;
                            return ldp;
                        }
                    }
                }
            }
        }

        /* if we've reached here, there are no more series of lines. */
        return NULL;
    }

    theline = obj_linefind( LF_BEGIN, LF_VANY, vf_idx, LF_END );

    while( theline != -1 )
    {
        if( obj_ismarked( theline ) )
        {
            for( llp = llist; *llp != -1; llp++ )
            {
                if( *llp == theline )
                    break;
            }

            if( *llp == -1 )
            {
                /* okidokie. */
                *llp++ = theline;
                *llp = -1;      /* terminatuh */

                ldp = &linelist[theline];

                if( ldp->vfrom == vf_idx )
                {
                    *whichside = 0;
                    vf_idx = ldp->vend;
                }
                else
                {
                    *whichside = 1;
                    vf_idx = ldp->vfrom;
                }

                return ldp;
            }
        }

        theline = obj_linefind( LF_CONTINUE );
    }

    return NULL;
}

void line_align( short object )
/*-------------------------------------------------------------------------*
 * Aligns the marked lines, like.
 *-------------------------------------------------------------------------*/
{
    LINEDEF *lprev = NULL, *ldp = NULL;
    short   i, whichside, direc;

    if( get_amode() == -1 )
        return;

    if( !st_llist )
        st_llist = (short*) memget( maxline * sizeof( short ) );
    stllp = st_llist;

    _objhigh_dotagged = 0;

    if( object != -1 )
    {
        /* effectively mark this one */
        obj_highlight( MODE_LINE, object, COLOR_CSRSEL );

        if( obj_ismarked( object ) )
            obj_highlight( MODE_LINE, object, COLOR_MARKED );
    }

    /* loop through both sides */
    while( 1 )
    {
        /* find the start of another series */
        ldp = findlinepathmarked( NULL, 2, &whichside );

        if( ldp == NULL )   /* nothing, eh. */
            break;

        direc = 0;  /* reset this */

        while( direc < 2 )
        {
            findlinepathmarked( ldp, direc, &whichside );

            texprev = NULL;
            sdprev = NULL;    /* init */
            lprev = NULL;

            while( ldp )
            {
                /* unhighlight previous line */
                if( lprev )
                {
                    obj_highlight( MODE_LINE, ldp2num( lprev ),
                        COLOR_MARKED ^ C_LIGHTMAGENTA );
                }

                if( !t_align( lprev, ldp, -1, whichside ) )
                    break;

                /* highlight this one */
                obj_highlight( MODE_LINE, ldp2num( ldp ),
                    COLOR_MARKED ^ C_LIGHTMAGENTA );

                delay( 30 );

                lprev = ldp;
                ldp = findlinepathmarked( NULL, 0, &whichside );    /* parms don't matter */
            }

            /* unhighlight last one */
            if( !ldp && lprev )
            {
                obj_highlight( MODE_LINE, ldp2num( lprev ),
                    COLOR_MARKED ^ C_LIGHTMAGENTA );
            }

            direc++;  /* do left side now */
            ldp = lprev;
        }
    }

    if( object != -1 )
    {
        if( obj_ismarked( object ) )
            obj_highlight( MODE_LINE, object, COLOR_MARKED );
        obj_highlight( MODE_LINE, object, COLOR_CSRSEL );
    }

    _objhigh_dotagged = 1;
}

void sector_align( short object )
/*-------------------------------------------------------------------------*
 * Aligns the textures in a sector.
 *-------------------------------------------------------------------------*/
{
    LINEDEF *lprev = NULL, *ldp;

    if( get_amode() == -1 )
        return;

    if( !st_llist )
        st_llist = (short*) memget( maxline * sizeof( short ) );
    stllp = st_llist;

    _objhigh_dotagged = 0;

    obj_highlight( MODE_SECTOR, object, COLOR_CSRSEL );

    while( 1 )
    {
        texprev = NULL;
        sdprev = NULL;    /* init */
        lprev = NULL;

        ldp = findpath( NULL, object ); /* start searchin' */

        if( ldp == NULL )       /* no more start lines */
            break;

        while( ldp != NULL )
        {
            /* unhighlight previous line */
            if( lprev )
                obj_highlight( MODE_LINE, ldp2num( lprev ), C_LIGHTMAGENTA );

            /* highlight this one */
            obj_highlight( MODE_LINE, ldp2num( ldp ), C_LIGHTMAGENTA );

            /* align it */
            if( !t_align( lprev, ldp, object, -1 ) )
                break;

            nMadeChanges ++;

            delay( 30 );

            lprev = ldp;    /* set previous to current */
            ldp = findpath( lprev, object );
        }

        /* unhighlight last one */
        obj_highlight( MODE_LINE, ldp2num( lprev ), C_LIGHTMAGENTA );
    }

    obj_highlight( MODE_SECTOR, object, COLOR_CSRSEL );

    _objhigh_dotagged = 1;
}

/*-------------------------------------------------------------------------*
 * Interactive texture alignment
 *-------------------------------------------------------------------------*/

static fldString *pfXOffset, *pfYOffset;
static stText *plbXSize, *plbYSize;

class fldAlign : public fldWallTex
{
public:
    fldAlign( int iTexture, bool fTopOrigin ) : fldWallTex()
        {
            ptLastTL = ptLastBR = Point( -1, -1 );
            ptTL = Point( 0, 0 );
            ptBR = Point( 255, 127 );
            SetWall( iTexture );
            this->fTopOrigin = fTopOrigin;
            iDragging = 0;

            pUnderBufs[0] = new char[imagesize( 0, 0, 256, 10 )];
            pUnderBufs[1] = new char[imagesize( 0, 0, 256, 10 )];
            pUnderBufs[2] = new char[imagesize( 0, 0, 10, 256 )];
            pUnderBufs[3] = new char[imagesize( 0, 0, 10, 256 )];
        }
    ~fldAlign()
        {
            delete pUnderBufs[0];
            delete pUnderBufs[1];
            delete pUnderBufs[2];
            delete pUnderBufs[3];
        }

private:
    void Draw();
    int Yxlate( int y );
    void Transfer( int iDirection )
        {
            if( pvTransferBuf == NULL )
                return;

            int* ipStore = (int*) pvTransferBuf;

            if( iDirection == transFromField )
            {
                ipStore[0] = ptTL.x;
                ipStore[1] = Yxlate( ptTL.y );
                ipStore[2] = ptBR.x - ptTL.x;
                ipStore[3] = ptBR.y - ptTL.y;
            }
            else
            {
                ptTL.x = ipStore[0];
                ptTL.y = Yxlate( ipStore[1] );
                ptBR.x = ptTL.x + ipStore[2];
                ptBR.y = ptTL.y + ipStore[3];
            }
        }
    int Event( Event_t& event );
    Point ptLastTL;
    Point ptTL;
    Point ptLastBR;
    Point ptBR;
    char *pUnderBufs[4];
    int iDragging;
    bool fTopOrigin;    // is y == 0 at the top, or the bottom?
};

int fldAlign::Yxlate( int y )
{
    // if fTopOrigin, just returns y.  otherwise, returns
    // texture height - y.  this is used for drawing only.
    if( fTopOrigin )
        return y;

    return pTex->height - y;
}

void fldAlign::Draw()
{
    bool fUpdateText = TRUE;

    Rect rcField = rcDlgPixels;
    rcField.MoveRel( pdlgParent->ScreenClientRect().TopLeft() );
    rcField.Inflate( -1, -1 );
    int Xmax = rcField.Width();
    int Ymax = rcField.Height();

    if( IsFirstDraw() )
    {
        // draw texture
        fldWallTex::Draw();
    }

    if( !(ptTL != ptLastTL || ptBR != ptLastBR) )
        return;

    SetViewPort( rcField.left, rcField.top, rcField.right, rcField.bottom );

    // erase old lines
    if( !IsFirstDraw() )
    {
        // replace underbufs
        putimage( 0, ptLastTL.y-4, pUnderBufs[0], COPY_PUT );
        putimage( ptLastTL.x-4, 0, pUnderBufs[1], COPY_PUT );
    }

    if( !IsFirstDraw() )
    {
        // replace underbufs
        putimage( 0, ptLastBR.y-4, pUnderBufs[2], COPY_PUT );
        putimage( ptLastBR.x-4, 0, pUnderBufs[3], COPY_PUT );
    }

    // draw new lines

    // save image
    getimage( 0, ptTL.y-4, 256, ptTL.y+4, pUnderBufs[0] );
    getimage( ptTL.x-4, 0, ptTL.x+4, 128, pUnderBufs[1] );

    getimage( 0, ptBR.y-4, 256, ptBR.y+4, pUnderBufs[2] );
    getimage( ptBR.x-4, 0, ptBR.x+4, 128, pUnderBufs[3] );

    setcolor( C_LIGHTGREEN );
    line( ptTL.x, 0, ptTL.x, Ymax );
    line( 0, ptTL.y, Xmax, ptTL.y );
    rectangle( ptTL.x-4, ptTL.y-4, ptTL.x+4, ptTL.y+4 );
    ptLastTL = ptTL;

    setcolor( C_LIGHTRED );
    line( ptBR.x, 0, ptBR.x, Ymax );
    line( 0, ptBR.y, Xmax, ptBR.y );
    rectangle( ptBR.x-4, ptBR.y-4, ptBR.x+4, ptBR.y+4 );
    ptLastBR = ptBR;

    setwritemode( COPY_PUT );

    pdlgParent->SetActive();

    if( fUpdateText )
    {
        char szBuf[128];

        itoa( ptTL.x, szBuf, 10 );
        pfXOffset->SetText( szBuf );
        itoa( Yxlate( ptTL.y ), szBuf, 10 );
        pfYOffset->SetText( szBuf );

        itoa( ptBR.x-ptTL.x, szBuf, 10 );
        plbXSize->SetText( szBuf );
        itoa( Yxlate( ptBR.y-ptTL.y ), szBuf, 10 );
        plbYSize->SetText( szBuf );
    }
}

int fldAlign::Event( Event_t& event )
{
    Rect rcField = GetRectInParent();
    bool fShifted = isshift();
    int iMaxX = rcField.Width();
    int iMaxY = rcField.Height();
    Rect rcTL( ptTL, Size( 0, 0 ) );
    Rect rcBR( ptBR, Size( 0, 0 ) );
    rcTL.Inflate( 5, 5 );
    rcBR.Inflate( 5, 5 );

    rcField.MoveTo( Point( 0, 0 ) );

    // rcField is now in the field only, eh?!

    switch( event.type )
    {
    case etKey:
        switch( event.iCode )
        {
        case KEY_LEFT:
            if( fShifted && ptBR.x )
                --ptBR.x;
            if( !fShifted && ptTL.x )
                --ptTL.x;
            break;
        case KEY_RIGHT:
            if( fShifted && ptBR.x < iMaxX )
                ++ptBR.x;
            if( !fShifted && ptTL.x < iMaxX )
                ++ptTL.x;
            break;
        case KEY_UP:
            if( fShifted && ptBR.y )
                --ptBR.y;
            if( !fShifted && ptTL.y )
                --ptTL.y;
            break;
        case KEY_DOWN:
            if( fShifted && ptBR.y < iMaxY )
                ++ptBR.y;
            if( !fShifted && ptTL.y < iMaxY )
                ++ptTL.y;
            break;
        }
        OnDraw();
        break;
    case etMouse:
        switch( event.iCode )
        {
        case MOUSE_LDOWN:
            // drag an origin?
            if( rcTL.IsPtInside( event.ptPoint ) )
            {
                // in top left
                iDragging = 1;
            }
            else if( rcBR.IsPtInside( event.ptPoint ) )
            {
                // in bottom right
                iDragging = 2;
            }
            break;
        case MOUSE_MOVE:
            // drag dein pointen
            if( !iDragging )
                break;
            if( iDragging == 1 )
            {
                ptTL = event.ptPoint;
                if( ptTL.x < 0 ) ptTL.x = 0;
                else if( ptTL.x >= iMaxX ) ptTL.x = iMaxX;
                if( ptTL.y < 0 ) ptTL.y = 0;
                else if( ptTL.y >= iMaxY ) ptTL.y = iMaxY;
            }
            else
            {
                ptBR = event.ptPoint;
                ptBR = event.ptPoint;
                if( ptBR.x < 0 ) ptBR.x = 0;
                else if( ptBR.x >= iMaxX ) ptBR.x = iMaxX;
                if( ptBR.y < 0 ) ptBR.y = 0;
                else if( ptBR.y >= iMaxY ) ptBR.y = iMaxY;
            }

            OnDraw();
            break;
        case MOUSE_LUP:
            iDragging = 0;  // no more dragging
            break;
        }
    }

    return daContinue;
}

int TexAlign( int iTexture, int *iXpos, int *iYpos, bool fTopOrigin )
{
    Dialog *pDialog = new Dialog;
    pDialog->Create( szDSFile, "AlignDlg" );   // load resource

    // fldWallTex *pAlignTex = new fldWallTex;

    // get fields
    pfXOffset = (fldString*) pDialog->GetField( "XOffset" );
    pfYOffset = (fldString*) pDialog->GetField( "YOffset" );
    plbXSize = (stText*) pDialog->GetLabel( "XSize" );
    plbYSize = (stText*) pDialog->GetLabel( "YSize" );

    // replace region field with align field
    fldAlign *pAlignTex = new fldAlign( iTexture, fTopOrigin );
    pDialog->REPLACEREGION( "MainTexture", pAlignTex );

    int iAlignment[4];
    pAlignTex->SetTransferBuf( iAlignment );

    // set up transfer to field
    iAlignment[0] = iXpos[0];
    iAlignment[1] = iYpos[0];
    iAlignment[2] = 256;
    iAlignment[3] = 128;

    int iRvl = pDialog->Process();

    if( iRvl == daExitSave )
    {
        iXpos[0] = iAlignment[0];
        iYpos[0] = iAlignment[1];
    }

    delete pDialog;

    return (iRvl == daExitSave);
}
