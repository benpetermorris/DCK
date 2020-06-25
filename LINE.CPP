/*-------------------------------------------------------------------------*
 * LINE.C:                                                                 *
 * Just functions for editing linedefs with dialog boxes.                  *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#ifdef USE_MATRIX
#include "pmatrix.h"
#endif

#include "join.h"
#include <math.h>

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#define bittobool(val,bitval)   (((val) & (bitval)) ? 1 : 0)

static short _WFdelvjoinline( short vertex );
void obj_pop( short mode );
void obj_push( short mode );

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#if 0
void _fld_showtex( FIELD *fldp, short where )
/*-------------------------------------------------------------------------*
 * Converts fldp's string into a texture integer, then displays that
 * texture below and 8 to the right of the field .. I think.
 *-------------------------------------------------------------------------*/
{
    short   x1 = fldp->x1 - (_fontwidth * 7);
    short   y1 = fldp->y1 + _fontheight + 4;
    short   x2 = x1 + 129;
    short   y2 = y1 + 129;
    short   texno = texture_toint( (char*) fldp->data );
    short   xofs = 0, yofs = 0, xw = 128, yw = 128, uh, fh, lh;
    short   sd2_valid;
    TEXTURE* texp = &texlist[texno];
    short   xtile = config.tog_tiletex && texp->width > 32;
    SECTOR  *secp1, *secp2;
    FIELD   *sd1sec, *sd2sec, *tsecp, *afldp;
    DIALOG  *dlgp = fldp->dlgp;

    if( where == 1 || !(dlgp->name) || strcmp( dlgp->name, "LineDialog" ) )
    {
        x1 = fldp->x1;
        x2 = x1 + 129;
    }

    strcpy( tfbuf, fldp->data );

    cursor_put( CSR_ERASE );
    gr_rshadrect( x1, y1, x2, y2, C_BLACK );

    if( config.tog_tiletex == 2 &&
        dlgp->name && !strcmpi( dlgp->name, "LineDialog" ) && texno > 0 )
    {
        sd2_valid = ((short*)_sd2_valfld->sparedata)[0];

        /* editing the line dialog box - get this side's x and y offsets
         * and pass them to the texture display function */
        if( !sd_enabled )
        {
            xofs = atoi( dlg_namptr( dlgp, "SD1X" )->data );
            yofs = atoi( dlg_namptr( dlgp, "SD1Y" )->data );
        }
        else
        {
            xofs = atoi( dlg_namptr( dlgp, "SD2X" )->data );
            yofs = atoi( dlg_namptr( dlgp, "SD2Y" )->data );
        }

        xtile = 1;

        if( _leditlen != 1 )
            xw = _leditlen;

        sd1sec = dlg_namptr( dlgp, "Sd1Sector" );
        sd2sec = dlg_namptr( dlgp, "Sd2Sector" );

        /* if this is sidedef 2, reverse the sector pointers */
        if( sd_enabled )
        {
            tsecp = sd1sec;
            sd1sec = sd2sec;
            sd2sec = tsecp;
        }

        /* get the y height. */
        if( sd2_valid && ((char*)sd1sec->data)[0] && ((char*)sd2sec->data)[0] )
        {
            /* two-sided line */
            secp1 = &sectorlist[atoi( sd1sec->data )];
            secp2 = &sectorlist[atoi( sd2sec->data )];

            uh = secp1->cheight - secp2->cheight;
            lh = secp2->fheight - secp1->fheight;
            fh = secp1->cheight - secp1->fheight;

            /* upper? (+3 : "Sdx") */
            if( !strcmp( fldp->name+3, "Upper" ) )
            {
                /* uppers are drawn bottom-up */
                yw = uh;

                afldp = dlg_namptr( dlgp, "A_UUnpeg" );
                if( fld_getcheck( afldp ) )
                    yofs += (texp->height - yw);
            }
            else if( !strcmp( fldp->name+3, "Lower" ) )
            {
                yw = lh;

                /* doesn't start at the top? */
                if( yw < 128 )
                {
                    y1 += (128 - yw);
                    refline1 = 1;
                    refline2 = 1;
                }

                afldp = dlg_namptr( dlgp, "A_LUnpeg" );

                /* calculate yofs based on lower unpegged setting */
                if( fld_getcheck( afldp ) )
                {
                    /* not checked, so draw top-down. */
                    if( yw < 128 )
                        refline2 = 1;
                }
                else
                {
                    /* draw from the bottom-up.  if the y height (yw)
                     * is less than 128, add the difference to y1.
                     * otherwise, add the difference of (128-t->height)
                     * to yofs. */
                    if( yw > 128 )      /* less than 128,  */
                    {
                        yofs += (128-texp->height);
                    }
                }
            }
            else if( !strcmp( fldp->name+3, "Full" ) )
            {
                yw = (fh - lh - uh);
                yw -= (yw % 128);

                afldp = dlg_namptr( dlgp, "A_LUnpeg" );
                if( fld_getcheck( afldp ) )
                    yofs += (texp->height - yw);

                y1 += (128 - yw);
            }
        }
        else if( ((char*)sd1sec->data)[0] && !strcmp( fldp->name+3, "Full" ) )
        {
            secp1 = &sectorlist[atoi( sd1sec->data )];
            yw = secp1->cheight - secp1->fheight;

            if( !yw )
                yw = 128;
        }
        else
            yw = 0;

        if( yw < 0 )
            yw = 0;

        if( xw > 128 )
            xw = 128;
        if( yw > 128 )
            yw = 128;
    }

    if( texno != -1 )
        texture_disp( x1+1, y1+1, xw, yw, xofs, yofs, texp, xtile );
    else
    {
        gr_setcolor( C_WHITE );
        if( _lnedit < 2 || fldismod( fldp ) )
            outtextxy( x1+2, y1+2, "Invalid texture." );
    }

    cursor_put( CSR_PUT );
}
#endif

void MakeWallField( Field* pField );

short side_replacetex( short csr_object )
/*-------------------------------------------------------------------------*
 * Displays a dialog box and allows the user to select search/replace      *
 * textures for all three types of texture.                                *
 *-------------------------------------------------------------------------*/
{
    uchar   suptex[10], smidtex[10], slotex[10];
    uchar   ruptex[10], rmidtex[10], rlotex[10];
    int     dowhat = obj_nmarked() ? 1 : 0;
    short     rvl, nc = 0, i;
    short     supidx, smididx, sloidx;
    short     rupidx, rmididx, rloidx;
    LINEDEF *ldp;
    SIDEDEFM    *sdp;
    GWINDOW *wnp;
    uchar   unmarkobj = 0;

    *suptex = *smidtex = *slotex = 0;
    *ruptex = *rmidtex = *rlotex = 0;

    VarTransfer_t vtReplaceTex[] =
    {
        { "UpperOld", NULL, suptex, NULL },
        { "UpperNew", NULL, ruptex, NULL },
        { "MiddleOld", NULL, smidtex, NULL },
        { "MiddleNew", NULL, rmidtex, NULL },
        { "LowerOld", NULL, slotex, NULL },
        { "LowerNew", NULL, rlotex, NULL },
        { "ReplaceIn", NULL, &dowhat, NULL },

        // end-marker
        { NULL, NULL, NULL, NULL }
    };

    Dialog* pDialog = new Dialog;
    pDialog->Create( _ABBREV_".DS", "ReplaceTexDlg" );

    pDialog->VarSetup( vtReplaceTex );

    MakeWallField( pDialog->GetField( "UpperOld" ) );
    MakeWallField( pDialog->GetField( "UpperNew" ) );
    MakeWallField( pDialog->GetField( "MiddleOld" ) );
    MakeWallField( pDialog->GetField( "MiddleNew" ) );
    MakeWallField( pDialog->GetField( "LowerOld" ) );
    MakeWallField( pDialog->GetField( "LowerNew" ) );

_dodlg:
    rvl = pDialog->Process();

    if( rvl == daExitNoSave )
    {
        delete pDialog;
        return 0;
    }

    if( csr_object != -1 && !obj_ismarked( csr_object ) )
        obj_mark( csr_object ), unmarkobj = 1;

    /* Get all the textures */
    supidx = texture_toint( (char*) suptex );
    smididx = texture_toint( (char*) smidtex );
    sloidx = texture_toint( (char*) slotex );
    rupidx = texture_toint( (char*) ruptex );
    rmididx = texture_toint( (char*) rmidtex );
    rloidx = texture_toint( (char*) rlotex );

    if( supidx == -1 && *suptex ||
        smididx == -1 && *smidtex ||
        sloidx == -1 && *slotex ||
        rupidx == -1 && *ruptex ||
        rmididx == -1 && *rmidtex ||
        rloidx == -1 && *rlotex )
    {
        message_box( MB_OK, "Notice",
                     "One or more of the textures you entered was\n"
                     "invalid!" );
        goto _dodlg;
    }

    delete pDialog;

    /* Make a status window */
    wnp = MAKESTATUSWINDOW( "Please Wait", 0 );
    wn_disp( wnp );
    wn_statstr( wnp, "Replacing textures..." );

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        if( !(i % 40) )
        {
            delay( 5 );        /* looks nice.  So shoot me. */
            wn_stat( wnp, i, nline );
        }

        /* is it marked?  OR.. are we doing all of them? */
        if( obj_ismarked( i ) || !dowhat )
        {
            if( ldp->rsdef != -1 )
            {
                /* Get the side and replace the textures */
                sdp = &sidelist[ldp->rsdef];
                if( supidx != -1 && sdp->utxidx == supidx )
                    { sdp->utxidx = rupidx; ++nc; }
                if( smididx != -1 && sdp->ftxidx == smididx )
                    { sdp->ftxidx = rmididx; ++nc; }
                if( sloidx != -1 && sdp->ltxidx == sloidx )
                    { sdp->ltxidx = rloidx; ++nc; }
            }
            if( ldp->lsdef != -1 )
            {
                /* Get the side and replace the textures */
                sdp = &sidelist[ldp->lsdef];
                if( supidx != -1 && sdp->utxidx == supidx )
                    { sdp->utxidx = rupidx; ++nc; }
                if( smididx != -1 && sdp->ftxidx == smididx )
                    { sdp->ftxidx = rmididx; ++nc; }
                if( sloidx != -1 && sdp->ltxidx == sloidx )
                    { sdp->ltxidx = rloidx; ++nc; }
            }
        }
    }

    delay( 20 );
    wn_stat( wnp, nline, nline );

    if( nc )
        nMadeChanges++;

    wn_undisp( wnp );
    wn_free( wnp );

    if( unmarkobj )
        obj_unmark( csr_object );

    return nc;
}

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Other code:                                                             *
 * Special code for creating lines, etc.  This is called from INPUT.C      *
 * when a series of lines is to be created, or when it's done, or when     *
 * it's been aborted .. silly stuff like that.                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static short _lc_vtx1, _lc_vtx2;  /* start/end vertices of line being created */
static short _lc_line = -1;       /* current line being created */
static short _lc_count = 0;       /* count of lines created since and including first */
static short _lc_first;           /* index of the first line created */
static short _lc_vcount;          /* number of vertices created this time */
static short _lc_vfirst;          /* number of vertices BEFORE it was started */
static short psector;             /* sector we're creating these things in */
static uchar old_join;

void drawoneline( short lineno );

static void _lc_deletecreated( void )
{
    undo_undo();
}

void setpsector( short num )
{
    psector = num;
}

short lc_inprogress( void )
{
    return (_lc_line == -1) ? 0 : 1;
}

short lc_inrange( short obj )
{
    if( lc_inprogress() && inrange( obj, _lc_first, (_lc_first + _lc_count - 1) ) )
        return 1;
    return 0;
}

void lc_abort( void )
/*-------------------------------------------------------------------------*
 * Aborts the line currently being created.                                *
 *-------------------------------------------------------------------------*/
{
    obj_drawmaniplist( -1, -1, -1, -1 );
    obj_exitmanip( 0 );
    obj_clearmarks( -1, 0 );

    _lc_deletecreated();

    tog_drag = 0;
    tog_join = old_join;
    _lc_line = -1;
    _lc_count = 0;
}

void lc_start( short x, short y )
/*-------------------------------------------------------------------------*
 * Starts creating a new line.  If _lc_line is not -1, that means we're    *
 * creating another line attached to _lc_line, so do that.                 *
 *-------------------------------------------------------------------------*/
{
    /* Test: If _lc_line is not -1, we're starting a new line from the end
     * of the line that was just created.  So .. use its ending vertex.
     */
    if( _lc_line != -1 )
    {
        _lc_vtx1 = linelist[_lc_line].vend;
        _lc_line = obj_insert( MODE_LINE, -1, 0, 0 );
    }
    else
    {
        /* saveitol */
        undo_init();

/*-------------------------------------------------------------------------*
 * Creating the vertices for the line: We don't try to create autojoins    *
 * here because the user might abort the line.  Instead, we take the more  *
 * sensible approach of attempting the joins at the confirmation of the    *
 * line's creation.                                                        *
 *-------------------------------------------------------------------------*/
        _lc_vtx1 = obj_insert( MODE_VERTEX, VTX_CREATE, x, y );
        _lc_line = obj_insert( MODE_LINE, -1, 0, 0 );
        _lc_count = 0;
        _lc_vcount = 1;
        old_join = tog_join;

void drawonevertex( VERTEX *vptr );

        drawonevertex( &vlist[_lc_vtx1] );

     //   tog_join = 0;
    }

    ++_lc_vcount;
    _lc_vtx2 = obj_insert( MODE_VERTEX, VTX_CREATE,
                           vlist[_lc_vtx1].x,
                           vlist[_lc_vtx1].y );

/*-------------------------------------------------------------------------*
 * Insert a new line: the way this is done is a bit..  silly.  Create two  *
 * vertices *AND* a linedef to go between them.  One of the vertices is the*
 * double-click point, and the other travels with the cursor. Mark the     *
 * cursor vertex. Call obj_initmanip() to start the moving process.  At the*
 * next single lclick save the changes.  ESC or rclick aborts.             *
 *-------------------------------------------------------------------------*/
    /* First line in this creation series */
    if( !_lc_count )
    {
        _lc_vfirst = nvertex - _lc_vcount;
        _lc_first = _lc_line;
    }

    /* Assign the vertices */
    linelist[_lc_line].vfrom = _lc_vtx1;
    linelist[_lc_line].vend  = _lc_vtx2;

    /* Clear the current marks */
    obj_clearmarks( -1, 1 );

    /* Drag it... */
    Object objVertex2( _lc_vtx2, MODE_VERTEX );
    obj_initmanip( MODE_VERTEX, cursorx, cursory, objVertex2, 0, 1 );
    obj_drawmaniplist( cursorx, cursory, -1, -1 );
    tog_drag = 1;
}

int GetLenience( int iObjectType );

short vmatch( short v1idx, short v2idx )
{
    VERTEX *v1 = &vlist[v1idx];
    VERTEX *v2 = &vlist[v2idx];

    if( v1idx == v2idx )
        return 0;

    int iLenience = GetLenience( MODE_VERTEX );

    return( abs( v2->x - v1->x ) <= iLenience && abs( v2->y - v1->y ) <= iLenience );
}

short lc_save( short startnew )
/*-------------------------------------------------------------------------*
 * Saves the line currently being created by adding it to the line list    *
 * and makign sure that the vertices are properly placed.                  *
 * If startnew == 1, calls lc_start() to start yet another one.            *
 * Returns 1 or 0 -- continue or don't.                                    *
 *-------------------------------------------------------------------------*/
{
    /* ignore this: and return.  yes I know this code is macroed and will
     * be expanded to about twice its size.  but do I look like I care?  do
     * I?  No, I don't.  Get lost. */

    int iLenience = GetLenience( MODE_VERTEX );

    if( startnew &&
        inrange( xstart + _scrx2map( cursorx ), vlist[_lc_vtx1].x - iLenience, vlist[_lc_vtx1].x + iLenience ) &&
        inrange( ystart + _scry2map( cursory ), vlist[_lc_vtx1].y - iLenience, vlist[_lc_vtx1].y + iLenience ) )
        return 1;

    /* Update coords */
    obj_drawmaniplist( -1, -1, -1, -1 );
    obj_drawmaniplist( cursorx, cursory, -1, -1 );

    cursor_put( CSR_ERASE );
    /* Erase the line */

    obj_drawmaniplist( -1, -1, -1, -1 );
    /* Done .. save it all */
    obj_exitmanip( 1 );
    /* Clear the marks */
    obj_clearmarks( -1, 0 );

    /* draw the one line */
    if( startnew )
        drawoneline( _lc_line );

    cursor_put( CSR_PUT );
    tog_drag = 0;

    ++_lc_count;        /* Number of lines created.. */

    if( startnew == 1 )
    {
        /* Wait.. are we on the first vertex? */
        if( vmatch( linelist[nline-1].vend, linelist[_lc_first].vfrom ) )
        {
            startnew = 0;
            goto _saveit;
        }

_keepgoing:
        lc_start( 0, 0 );   /* Don't need coords on continuing lines */
        return 1;
    }
    else
    {
_saveit:
        /* Go for it! */
        if( startnew == 0 )
        {
            switch( _WFcompleteregion( _lc_first, nline ) )
            {
            case 0:     /* Done. */
                break;
            case 1:     /* Continue creating lines! */
                goto _keepgoing;
            }
        }

        /* Destroy all unused vertices */
        map_purge( MODE_VERTEX );

        /* Done creating this series of lines. */
        _lc_line = -1;
        _lc_count = 0;
        tog_join = old_join;

        undo_setvars();

        return 0;
    }
}

void side_getneedtex( short line_no, short whichside,
                      short *upper, short *full, short *lower )
/*-------------------------------------------------------------------------*
 * Uses simple logic to determine which textures a sidedef requires.       *
 *-------------------------------------------------------------------------*/
{
    SIDEDEFM    *sdp, *sdp2;
    SECTOR      *secp, *secp2;
    LINEDEF     *ldp = &linelist[line_no];

    sdp = (!whichside ? (&sidelist[ldp->rsdef]) : (&sidelist[ldp->lsdef]));
    sdp2 = (whichside ? (&sidelist[ldp->rsdef]) : (&sidelist[ldp->lsdef]));

    /* Need full texture? */
    *full = (ldp->attribs & LA_TSIDE) ? 0 : 1;
    *upper = *lower = 0;

    if( (whichside && ldp->lsdef == -1) || (!whichside && ldp->rsdef == -1) )
        return;

    if( !(*full) )     /* Logic here */
    {
        secp = &sectorlist[sdp->sector];
        secp2 = &sectorlist[sdp2->sector];

        /* Can't do dat */
        if( sdp2->sector == -1 )
            return;

        /* Check for upper texture */
        if( secp2->cheight < secp->cheight )
        {
            if( strcmpi( secp->ctxname, "F_SKY1" ) ||
                strcmpi( secp2->ctxname, "F_SKY1" ) )
                *upper = 1;
        }
        if( secp2->fheight > secp->fheight )
            *lower = 1;
    }
}

void sect_fixtex( short obj_num, short remove )
/*-------------------------------------------------------------------------*
 * Finds all sidedefs with obj_num as their sector and calls _WFfixtexnames*
 * for each.                                                               *
 *-------------------------------------------------------------------------*/
{
    short   theline;

    theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, obj_num, LF_END );

    while( theline != -1 )
    {
        _WFfixtexnames( theline, 0, remove );
        _WFfixtexnames( theline, 1, remove );

        theline = obj_linefind( LF_CONTINUE );
    }
}

void _WFfixborderlines( short sector )
/*-------------------------------------------------------------------------*
 * Finds all lines that border sector and another sector and fixes their   *
 * textures.                                                               *
 *-------------------------------------------------------------------------*/
{
    short     i;
    LINEDEF *ldp;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        if( ((ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == sector) &&
             (ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector >= 0)) ||
            ((ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == sector) &&
             (ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector >= 0)) )
        {
            /* This is a borderline! */
            _WFfixtexnames( i, 0, 0 );
            _WFfixtexnames( i, 1, 0 );
        }
    }
}

short sect_gethighestparent( short sector )
/*-------------------------------------------------------------------------*
 * Uses floor differences.                                                 *
 *-------------------------------------------------------------------------*/
{
    short   theline, thesect, curheight = -32000, cursect = -1;
    LINEDEF *ldp;

    theline = obj_linefind( LF_BEGIN,
                            LF_ATTRIBS, LA_TSIDE,
                            LF_SIDEWITHSEC, sector, LF_END );

    while( theline != -1 )
    {
        ldp = &linelist[theline];

        if( (thesect = sidelist[ldp->lsdef].sector) != sector ||
            (thesect = sidelist[ldp->rsdef].sector) != sector )
            {
                if( thesect == sector )
                    goto _nextline;

                if( sectorlist[thesect].fheight > curheight )
                    curheight = sectorlist[thesect].fheight, cursect = thesect;
            }

_nextline:
        theline = obj_linefind( LF_CONTINUE );
    }

    return cursect;
}

#define LIFT_WALK       88
#define LIFT_SPACE      62

#define LA_SETSPAC(attribs,ac)  (attribs &= 0x3ff, attribs |= ((ac)<<10))

bool GetLiftType( int& iSpeed, int& iDelay )
{
    int iValues[][2] =
    {
        { 5, 35 },
        { 10, 35 },
        { 20, 35 },
        { 30, 35 }
    };

    int iRvl = MenuPop( 0, "Select a type of lift:", NULL,
        "Very Slow", NULL,
        "Slow", NULL,
        "Moderately Fast", NULL,
        "Fast!", NULL,
        NULL );

    if( iRvl == -1 )
        return FALSE;

    iSpeed = iValues[iRvl][0];
    iDelay = iValues[iRvl][1];

    return TRUE;
}

void CreateLiftFromSector( int iSector )
/*-------------------------------------------------------------------------*
 * Creates a lift from a sector.                                           *
 * Processes each line that's connected to the sector:                     *
 *  1) Impassable lines get NOTHING.  HEAR ME ?!                           *
 *  2) 2sLines that border on the chosen "upper sector" get the "trigger   *
 *     lift down" type                                                     *
 *  3) 2sLines that border on any sectors lower than the CUS get the "button *
 *     lift down" type and must face outside the lift sector.              *
 *-------------------------------------------------------------------------*/
{
    short   theline, upsect, thesect, sectag;
    LINEDEF *ldp;

    if( obj_nmarked() > 1 )
    {
        if( message_box( MB_YESNO, st_dcksays,
            "Turn all marked sectors into lifts?" ) == MB_NO )
            return;
    }

    int iSpeed, iDelay;

    if( !GetLiftType( iSpeed, iDelay ) )
        return;

    bool unmarkobj = FALSE;

    if( iSector != -1 && !obj_ismarked( iSector ) )
        obj_mark( iSector ), unmarkobj = TRUE;

    for( int iThisSector = 0; iThisSector < MAXOBJECT; iThisSector++ )
    {
        if( !obj_ismarked( iThisSector ) )
            continue;

        upsect = sect_gethighestparent( iThisSector );
        theline = obj_linefind( LF_BEGIN,
                                LF_ATTRIBS, LA_TSIDE,
                                LF_SIDEWITHSEC, iThisSector, LF_END );

        sectag = firstfreetag();
        sectorlist[iThisSector].sectag = sectag;

        while( theline != -1 )
        {
            ldp = &linelist[theline];

            if( (thesect = sidelist[ldp->lsdef].sector) != iThisSector ||
                (thesect = sidelist[ldp->rsdef].sector) != iThisSector )
            {
                /* Make sure it faces outward */
                if( sidelist[ldp->rsdef].sector == iThisSector )
                {
                    nMapChanges ++;
                    swap2b( ldp->vfrom, ldp->vend );   /* gottaswap */
                    swap2b( ldp->lsdef, ldp->rsdef );
                }

                if( !fHexen )
                {
                    if( thesect == upsect )
                        ldp->special = LIFT_WALK;
                    if( sectorlist[thesect].fheight < sectorlist[upsect].fheight )
                        ldp->special = LIFT_SPACE;
                    else
                        ldp->special = LIFT_WALK;

                    if( ldp->special == LIFT_WALK )
                        sidelist[ldp->lsdef].ltxidx = _def_ltxidx;
                    else
                        sidelist[ldp->rsdef].ltxidx = _def_ltxidx;

                    lSecTag( ldp ) = sectag;
                }
                else
                {
                    ldp->special = 62;      // plat_downwaitupstay
                    ldp->arg1 = iSpeed;
                    ldp->arg2 = iDelay;

                    if( sectorlist[thesect].fheight < sectorlist[upsect].fheight )
                    {
                        LA_SETSPAC( ldp->attribs, 1 );
                        sidelist[ldp->rsdef].ltxidx = _def_ltxidx;
                    }
                    else
                    {
                        LA_SETSPAC( ldp->attribs, 0 );
                        sidelist[ldp->lsdef].ltxidx = _def_ltxidx;
                    }

                    lSecTag( ldp ) = sectag;
                }
            }

            theline = obj_linefind( LF_CONTINUE );
        }

        nMadeChanges ++;

        sectorlist[iThisSector].fheight = sectorlist[upsect].fheight;

        if( sectorlist[iThisSector].cheight < sectorlist[upsect].cheight )
            sectorlist[iThisSector].cheight = sectorlist[upsect].cheight;

        wncpy( sectorlist[iThisSector].ftxname, _def_liftfloor );

        post_fullupdate();
    }

    if( unmarkobj )
        obj_unmark( iSector );
}

enum doortypes
{
    D_NORMAL = 1,
    D_BLUE = 26,
    D_YELLOW = 27,
    D_RED = 28,
    D_STAY = 31,
    D_BLUESTAY = 32,
    D_REDSTAY = 33,
    D_YELLOWSTAY = 34,
    D_REMOTE = 0,
    D_SEKRIT = -1,
    D_NORMALB = 117,
    D_STAYB = 118
};

short doortypes[] =
{
    D_NORMAL,
    D_NORMALB,
    D_STAY,
    D_STAYB,
    0,
    D_BLUE,
    D_YELLOW,
    D_RED,
    D_BLUESTAY,
    D_REDSTAY,
    D_YELLOWSTAY,
    0,
    D_REMOTE,
    D_SEKRIT
};

#include "fields.hpp"

int _ffTexBrowser( fldString* pString );
int _ffShowChangedTex( fldString* pString );

static int _fvTexName( const char* pszString, fldString* pString )
{
    if( texture_toint( (char*) pszString ) == -1 )
    {
        pString->SetValMsg( "This is not a valid texture." );
        return valNotOkay;
    }

    return valOkay;
}

static fldCheckBox *pfStayOpen;
static fldListBox *pfKeys;

static int ffKeyChanged()
{
    if( pfKeys->GetCurItem() > 0 )
        pfStayOpen->SetState( OFF );

    return daContinue;
}

static int ffStayOpenChanged()
{
    if( pfStayOpen->GetState() )
        pfKeys->SetCurItem( 0 );

    return daContinue;
}

static struct
{
    int iType;

    int iSpeed;
    int iDelay;
    int iKey;
    bool fSecret;
    bool fStayOpen;
    bool fRepeatable;
    int iTexture;

} DoorInfo =
{
    12,

    15,
    35,
    0,
    0,
    0,
    1,
    -1
};

bool GetDoorType()
{
    if( !fHexen )
    {
        int iRvl = MenuPop( 0, "Choose a door type", "MN_MKDOOR",
                        " Open / Close               ", NULL,
                        " Open / Close Blazing  ", NULL,
                        " Open / Stay           ", NULL,
                        " Open / Stay Blazing   ", NULL,
                        "-", NULL,
                        " Open / Close (Blue)   ", NULL,
                        " Open / Close (Yellow) ", NULL,
                        " Open / Close (Red)    ", NULL,
                        " Open / Stay (Blue)    ", NULL,
                        " Open / Stay (Red)     ", NULL,
                        " Open / Stay (Yellow)  ", NULL,
                        "-", NULL,
                        " Remote door (Normal Lines) " , NULL,
                        " Open / Close (Secret) ", NULL,
                        NULL );

        if( iRvl == -1 )
            return FALSE;

        DoorInfo.iType = doortypes[iRvl];

        if( DoorInfo.iType == D_SEKRIT )    // -1
        {
            DoorInfo.iType = D_NORMAL;
            DoorInfo.fSecret = TRUE;
        }

        return TRUE;
    }


    static char fszTexName[128];
    char fszKey[128];
    fldString *pfTexName;
    int _iSpeed, _iDelay;

    if( DoorInfo.iTexture == -1 )
        DoorInfo.iTexture = _def_doorbig;

    VarTransfer_t vtDoor[] =
    {
        { "Speed", NULL, (PTR) &DoorInfo.iSpeed, NULL },
        { "Delay", NULL, (PTR) &DoorInfo.iDelay, NULL },
        { "Secret", NULL, (PTR) &DoorInfo.fSecret, NULL },
        { "StayOpen", (Field**) &pfStayOpen, (PTR) &DoorInfo.fStayOpen, NULL },
        { "Repeatable", NULL, (PTR) &DoorInfo.fRepeatable, NULL },
        { "Key", (Field**) &pfKeys, (PTR) fszKey, NULL },
        { "TexName", (Field**) &pfTexName, (PTR) fszTexName, NULL },

        // end-marker
        { NULL, NULL, NULL, NULL }
    };

    Dialog *pDialog = new Dialog;
    pDialog->Create( szDSFile, "DoorDlg" );

    pDialog->VarSetup( vtDoor );

    fldWallTex *pfTexture = new fldWallTex;
    pDialog->REPLACEREGION( "Texture", pfTexture );
    pfTexName->pUserData = (PTR) pfTexture;
    pfTexName->SetOnChange( (FFUNC) _ffShowChangedTex );
    pfTexName->SetUserEvent( etKey, KEY_F2, (FUFUNC) _ffTexBrowser );
    pfTexName->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _ffTexBrowser );
    pfTexName->SetOnValidate( (FVFUNC) _fvTexName );
    texture_tostr( fszTexName, DoorInfo.iTexture );
    pfTexture->SetWall( fszTexName );

    // create key list
    StrList *stlKeys = new StrList( 16, 30 );
    stlKeys->AddTail( "Unlocked", 0 );
    stlKeys->AddTail( "Steel key", 1 );
    stlKeys->AddTail( "Cave key", 2 );
    stlKeys->AddTail( "Axe key", 3 );
    stlKeys->AddTail( "Fire key", 4 );
    stlKeys->AddTail( "Emerald key", 5 );
    stlKeys->AddTail( "Dungeon key", 6 );
    stlKeys->AddTail( "Silver key", 7 );
    stlKeys->AddTail( "Rusty key", 8 );
    stlKeys->AddTail( "Horn key", 9 );
    stlKeys->AddTail( "Swamp key", 10 );
    stlKeys->AddTail( "Gold key", 11 );
    pfKeys->SetItemList( stlKeys );
    stlKeys->IdToStr( DoorInfo.iKey, fszKey );

    // these are related
    pfKeys->SetOnChange( (FFUNC) ffKeyChanged );
    pfStayOpen->SetOnChange( (FFUNC) ffStayOpenChanged );

    int iRvl = pDialog->Process();

    delete pDialog;

    if( iRvl == daExitNoSave )
        return FALSE;

    DoorInfo.iKey = stlKeys->StrToId( fszKey );

    if( DoorInfo.iKey )
        DoorInfo.iType = 13;  // key door type
    else if( DoorInfo.fStayOpen )
        DoorInfo.iType = 11;    // stay open type
    else
        DoorInfo.iType = 12;    // regular, boring door type

    DoorInfo.iTexture = pfTexture->GetWall();

    return TRUE;
}

void CreateDoorFromSector( int iSector )
/*-------------------------------------------------------------------------*
 * Creates a door.                                                         *
 * Processes each line that's connected to the sector:                     *
 *  1) Impassable lines get the doortrack texture (the default one) and    *
 *     are set to "lower unpegged."                                        *
 *  2) Other lines are made "facing outward", and have the door function   *
 *     set to them.                                                        *
 *-------------------------------------------------------------------------*/
{
    LINEDEF     *ldp;
    SIDEDEFM    *sdp;

    SECTOR      *secp;
    short       setdtm1 = 0;
    short       theline, linelen = 0, tmpint;
    short       parsector;
    SECTOR      *psecp;

    int         i;

    if( !GetDoorType() )
        return;

    bool unmarkobj = FALSE;

    if( iSector != -1 && !obj_ismarked( iSector ) )
        obj_mark( iSector ), unmarkobj = TRUE;

    for( int iThisSector = 0; iThisSector < MAXOBJECT; iThisSector++ )
    {
        if( !obj_ismarked( iThisSector ) )
            continue;

        secp = &sectorlist[iThisSector];
        parsector = sect_gethighestparent( iThisSector );
        psecp = &sectorlist[parsector];

        theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, iThisSector, LF_END );

        while( theline != -1 )
        {
            ldp = &linelist[theline];

            /* Impassable? */
            if( ldp->attribs & LA_IMPASS )
            {
                /* Door track texture.  Lower unpegged.  DUUDE. */
                sdp = &sidelist[ldp->rsdef];
                sdp->ftxidx = _def_doortrak;
                ldp->attribs |= LA_LUNPEGGED;
            }
            else
            {
                /* Make sure it's facing outside the sector.  If iTexture
                * is still -1, decide what texture to used based upon the
                * length of the longest line. */

                /* Check to see: Make sure line_num is facing OUT of the sector */
                if( sidelist[ldp->rsdef].sector == iThisSector )
                {
                    swap2b( ldp->vfrom, ldp->vend );
                    swap2b( ldp->rsdef, ldp->lsdef );
                }

                /* Clear textures */
                sdp = &sidelist[ldp->rsdef];
                sdp->ftxidx = sdp->ltxidx = sdp->utxidx = 0;
                sdp = &sidelist[ldp->lsdef];
                sdp->ftxidx = sdp->ltxidx = sdp->utxidx = 0;

                tmpint = obj_linelen( theline );
                if( tmpint > linelen )
                    linelen = tmpint;
            }

            theline = obj_linefind( LF_CONTINUE );
        }

        /* Set the sector's ceiling height to its floor height */
        secp->cheight = secp->fheight = psecp->fheight;

        /* Figure out the length and set the appropriate door texture */
        if( !fHexen )
        {
            if( linelen <= 64 ) DoorInfo.iTexture = _def_doorsmall;
            else                DoorInfo.iTexture = _def_doorbig;
        }

        /* Set some textures now */
        theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, iThisSector, LF_END );
        while( theline != -1 )
        {
            ldp = &linelist[theline];

            /* Set texture */
            if( !(ldp->attribs & LA_IMPASS) )
            {
                /* don't do dat! */
                ldp->attribs &= ~LA_UUNPEGGED;

                sdp = &sidelist[ldp->rsdef];

                sdp->utxidx = DoorInfo.iTexture;
                ldp->special = DoorInfo.iType;

                if( fHexen )
                {
                    ldp->arg0 = 0;
                    ldp->arg1 = DoorInfo.iSpeed;
                    ldp->arg2 = DoorInfo.iDelay;
                    ldp->arg3 = DoorInfo.iKey;

                    LA_SETSPAC( ldp->attribs, 1 );
                    bitset( ldp->attribs, DoorInfo.fRepeatable, LA_REPEAT );
                }

                if( DoorInfo.fSecret )
                {
                    secp = &sectorlist[sdp->sector];

                    /* set the secret flag */
                    ldp->attribs |= LA_SECRET;

                    sdp->utxidx = sdp->ftxidx = sdp->ltxidx = 0;
                    _WFfixtexnames( theline, 0, 0 );    /* to get surrounding textures */
                    _WFfixtexnames( theline, 1, 0 );

                    if( texlist[sdp->utxidx].height )
                        sdp->wyofs = (secp->cheight - secp->fheight) % (texlist[sdp->utxidx].height);
                }
            }

            theline = obj_linefind( LF_CONTINUE );
        }
    }

    if( unmarkobj )
        obj_unmark( iSector );

    post_fullupdate();
    nMadeChanges++;
    nMapChanges++;
}

void _WFfixtexnames( short obj_num, short whichside, short remove )
/*-------------------------------------------------------------------------*
 * Fixes textures.                                                         *
 * Pretty simple stuff -- checks to see if obj_num is attached to any      *
 * other lines and duplicates their textures where needed.  Uses default   *
 * textures (_def_?txidx) where required.  Logic:                          *
 *                                                                         *
 *  - Upper/Lower texture:                                                 *
 *      1. obj_num's opposite texture                                      *
 *      2. Connected line's same texture                                   *
 *      3. obj_num's normal texture                                        *
 *      4. connected line's normal texture                                 *
 *      5. obj_num's opposite sidedef's opposite texture                   *
 *      6. default upper/lower texture                                     *
 *  - Normal texture:                                                      *
 *      1. Connected line's normal texture                                 *
 *      2. Connected line's upper/lower texture                            *
 *      3. Default normal texture                                          *
 *    Normal texture is applied only if the line is NOT 2S.                *
 *-------------------------------------------------------------------------*/
{
    short       i;
    LINEDEF     *ldp, *ldp2;
    SIDEDEFM    *sdp, *sdp2, *opsdp;
    short       ftex, utex, ltex, sectag2;
    short       needutex, needltex, needftex;
    short       vfrom, vend, sector, _lastline = -1, direc = 0;

    if( remove == 2 )   /* do both */
    {
        /* remove and then don't remove */
        _WFfixtexnames( obj_num, whichside, 1 );
        remove = 0;
    }

    if( whichside == 2 )
    {
        _WFfixtexnames( obj_num, 0, remove );
        whichside = 1;
    }

    ldp = &linelist[obj_num];

    if( (!whichside && ldp->rsdef == -1) || (whichside && ldp->lsdef == -1) )
        return;

    vfrom = ldp->vfrom;
    vend = ldp->vend;
    sdp = (!whichside ? (&sidelist[ldp->rsdef]) : (&sidelist[ldp->lsdef]));
    if( (whichside && ldp->rsdef >= 0) || (!whichside && ldp->lsdef >= 0) )
        opsdp = (whichside ? (&sidelist[ldp->rsdef]) : (&sidelist[ldp->lsdef]));
    else
        opsdp = NULL;
    sector = sdp->sector;

    ftex = _def_ftxidx;
    utex = _def_utxidx;
    ltex = _def_ltxidx;

    side_getneedtex( obj_num, whichside, &needutex, &needftex, &needltex );

    if( remove )
    {
        /* This is simple enough */
        if( !needftex )   sdp->ftxidx = 0;
        if( !needltex )   sdp->ltxidx = 0;
        if( !needutex )   sdp->utxidx = 0;
        nMadeChanges++;
        return;
    }
    else
    {
        sectag2 = (opsdp ? sectorlist[opsdp->sector].sectag : 0);

        /* "Smart" remove */
        if( !needutex && !sectorlist[sdp->sector].sectag && !sectag2 )
            sdp->utxidx = 0;
        if( !needutex && !sectorlist[sdp->sector].sectag && !sectag2 )
            sdp->utxidx = 0;
    }

    if( needftex && sdp->ftxidx )   needftex = 0;
    if( needutex && sdp->utxidx )   needutex = 0;
    if( needltex && sdp->ltxidx )   needltex = 0;

    /* Find a connected line.  We're going to try to center the
     * search around the line, so start AT the line and move upwards.
     * If nothing's found, repeat the loop going DOWN.
     */
_doitagain:
    for( ldp2 = linelist+obj_num, i = obj_num; i != _lastline; )
    {
        /* Check to see if it's connected.. */
        if( i != obj_num &&
            (ldp2->vfrom == vfrom || ldp2->vend == vend ||
             ldp2->vfrom == vend || ldp2->vend == vfrom) )
        {
            /* It is!  Now find the logical sidedef..
             * (ie: the one that's in the same sector ) */
            if( ldp2->rsdef >= 0 && sidelist[ldp2->rsdef].sector == sector )
                sdp2 = &sidelist[ldp2->rsdef];
            else if( ldp2->lsdef >= 0 && sidelist[ldp2->lsdef].sector == sector )
                sdp2 = &sidelist[ldp2->lsdef];
            else
                sdp2 = NULL;

            /* if sdp2 != NULL, then we've found a sidedef that's in the
             * same sector.  Assign the needed textures and break.  */
            if( sdp2 )
            {
                ftex = sdp->ftxidx;

                if( needftex )
                {
                    if( sdp2->ftxidx )  ftex = sdp2->ftxidx;
                    else if( sdp2->utxidx ) ftex = sdp2->utxidx;
                    else if( sdp2->ltxidx ) ftex = sdp2->ltxidx;
                    else goto _next;
                    sdp->ftxidx = ftex;
                    needftex = 0;
                }
                if( needutex )
                {
                    if( sdp->ltxidx )           utex = sdp->ltxidx;
                    else if( sdp2->utxidx )     utex = sdp2->utxidx;
                    else if( ftex )             utex = ftex;
                    else if( sdp2->ftxidx )     utex = sdp2->ftxidx;
                    else if( opsdp && opsdp->ltxidx ) utex = opsdp->ltxidx;
                    else goto _next;

                    if( !sdp->utxidx && config.tog_unpeg )
                        ldp->attribs |= LA_UUNPEGGED;

                    sdp->utxidx = utex;
                    needutex = 0;
                }
                if( needltex )
                {
                    if( sdp->utxidx )           ltex = sdp->utxidx;
                    else if( sdp2->ltxidx )     ltex = sdp2->ltxidx;
                    else if( ftex )             ltex = ftex;
                    else if( sdp2->ftxidx )     ltex = sdp2->ftxidx;
                    else if( opsdp && opsdp->utxidx ) ltex = opsdp->utxidx;
                    else goto _next;

                    if( !sdp->ltxidx && config.tog_unpeg )
                        ldp->attribs |= LA_LUNPEGGED;

                    sdp->ltxidx = ltex;
                    needltex = 0;
                }
            }
        }

_next:
        if( direc )
        {
            i++;
            ldp2++;
        }
        else
        {
            i--;
            ldp2--;
        }
    }

    /* If true, we haven't found anything.. go forwards in the list
     * and see if there's anything else there */
    if( !direc && (needftex || needutex || needltex) )
    {
        _lastline = nline;
        direc = 1;
        goto _doitagain;
    }

    if( needftex )  sdp->ftxidx = _def_ftxidx;
    if( needutex )  sdp->utxidx = _def_utxidx;
    if( needltex )  sdp->ltxidx = _def_ltxidx;
    nMadeChanges++;
}

void linefixwithmatrix( short secSearch, short secReplace )
/*-------------------------------------------------------------------------*
 * Finds all the sidedefs that reference secSearch, and if they're         *
 * contianed within the polygon matrix, replaces their sector references   *
 * with secReplace.                                                        *
 *-------------------------------------------------------------------------*/
{
#ifdef USE_MATRIX
    LINEDEF     *ldp;
    SIDEDEFM    *sdp;
    short         i;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        if( ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == secSearch )
            if( mtxCheckLinedef( i ) )
                sidelist[ldp->rsdef].sector = secReplace;
        if( ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == secSearch )
            if( mtxCheckLinedef( i ) )
                sidelist[ldp->lsdef].sector = secReplace;
    }
#endif
}

/* Set by the following routine. */
short _ss_origsec, _ss_newsec;

void fixsectorrefs( short newsector, short psector )
/*-------------------------------------------------------------------------*
 * Sectors that might have been changed ..
 *-------------------------------------------------------------------------*/
{
    short   i, sd, sec;
    LINEDEF *ldp;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        sd = ldp->rsdef;
        if( sd >= 0 && sidelist[sd].sector == psector )
        {
            sec = obj_getphyssector( i, 0 );
            if( sec == newsector )
                sidelist[sd].sector = newsector;
        }
        sd = ldp->lsdef;
        if( sd >= 0 && sidelist[sd].sector == psector )
        {
            sec = obj_getphyssector( i, 1 );
            if( sec == newsector )
                sidelist[sd].sector = newsector;
        }
    }
}

short _WFsplitsector( short splitline )
/*-------------------------------------------------------------------------*
 * Splits a sector with splitline.                                         *
 * Returns 1 or 0 .. ok or not.                                            *
 *-------------------------------------------------------------------------*/
{
    short     llist[128];
    short     i2, *ip;
    short     *lptr = llist, done = 0, sd, sec;
    uchar     reason[80];
    LINEDEF   *ldp = &linelist[splitline];
    SIDEDEFM  *sdp;
    short     vidx, v1idx = ldp->vfrom, v2idx = ldp->vend, i = 0, lcount = 0;
    short     newsector;
#ifdef USE_MATRIX
    short     mtxllist[128];
    short     mtxlcount = 0;
#endif

    _ss_origsec = psector;
    ldp = linelist;

    /* Create a closed path here */
    for( vidx = v1idx; vidx != v2idx; )
    {
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( i == splitline )
                continue;

            sdp = &sidelist[ldp->rsdef];
            if( ldp->rsdef >= 0 && sdp->sector == psector && ldp->vfrom == vidx )
            {
                for( i2 = 0, ip = llist; i2 < lcount; i2++, ip++ )
                {
                    if( *ip == i )
                        goto _nope;
                }

                vidx = ldp->vend;
                *lptr++ = i;
                ++lcount;
                break;
            }

            sdp = &sidelist[ldp->lsdef];
            if( ldp->lsdef >= 0 && sdp->sector == psector && ldp->vend == vidx )
            {
                for( i2 = 0, ip = llist; i2 < lcount; i2++, ip++ )
                {
                    if( *ip == i )
                        goto _nope;
                }

                vidx = ldp->vfrom;
                *lptr++ = i;
                ++lcount;
                break;
            }
_nope:

        }

        if( i == nline )
        {
            /* Couldn't find a closed path! */
            if( vidx == v1idx )
                sprintf( reason, "Vertex %d doesn't have a sidedef that references this sector.",
                                 v1idx );
            else
                strcpy( reason, "You should make sure that the sector is closed first." );

            if( message_box( MB_YESNO, "Notice",
              " I can't complete this region because there isn't a closed       \n"
              "path on this sector between the two points of the split line.\n"
              "(%s)\n"
              "Do you want to keep the line you created anyway?", reason ) == MB_NO )
              _lc_deletecreated();
            return 0;
        }

        if( vidx == v1idx )
        {
            message_box( MB_OK, "Notice",
                         "The two vertices are not in the same sector." );
            _lc_deletecreated();
            return 0;
        }
    }

    /* Create the new sector */
    _ss_newsec = newsector = obj_insert( MODE_SECTOR, psector, 0, 0 );
    sectorlist[newsector].sectag = 0;   /* NO SHARE THIS UNKA */
    ldp = &linelist[splitline];
    ldp->attribs = LA_TSIDE;
    /* Create left sidedef -- already have a right one */
    ldp->lsdef = obj_insert( MODE_SIDEDEF, -1, 0, 0 );
    sidelist[ldp->rsdef].sector = psector;
    sidelist[ldp->rsdef].ftxidx = 0;
    sidelist[ldp->lsdef].sector = newsector;
    sidelist[ldp->lsdef].ftxidx = 0;

    for( lptr = llist, i = 0; i < lcount; lptr++, i++ )
    {
        ldp = &linelist[*lptr];
        sdp = &sidelist[ldp->rsdef];
        if( ldp->rsdef < 0 || sdp->sector != psector )
            sdp = &sidelist[ldp->lsdef];
        sdp->sector = newsector;
    }

    fixsectorrefs( newsector, psector );

    nMadeChanges++;
    nMapChanges++;
    return 1;
}

static short ranged_value( short val, short min, short max )
/*-------------------------------------------------------------------------*
 * if val is in >= min and < max, all's coo.
 * if val is < min, returns max-(min-val).
 * if val is >= max, returns min+(max-val).
 *-------------------------------------------------------------------------*/
{
    if( val >= min && val < max )
        return val;
    else if( val < min )
        return max-(min-val);
    else if( val >= max )
        return min+(max-val);

    return val; /* oops?! */
}

static short isregioninward( short lstart, short lend )
/*-------------------------------------------------------------------------*
 * Determines whether the "region" between and including the lines lstart
 * and lend is facing in on itself.
 *-------------------------------------------------------------------------*/
{
    VERTEX  *v, *vprev, *vnext, *vmid;
    short   vminx[3], vmaxx[3], vminy[3], vmaxy[3], *vchosen = NULL;
    LINEDEF *ldp, *lprev, *lnext;
    short   i, minx = 32000, miny = -32000, maxy = 32000, maxx = -32000;
    short   angle1, angle2, angle3;

    for( ldp = linelist + lstart, i = lstart; i < lend; ldp++, i++ )
    {
        v = &vlist[ldp->vfrom];
        lprev = &linelist[ranged_value( i-1, lstart, lend )];
        lnext = &linelist[ranged_value( i+1, lstart, lend )];

        if( v->x < minx || (minx != 32000 && v->x == minx && v->y > vlist[vminx[1]].y) )
        {
            minx = v->x;
            vminx[0] = lprev->vfrom;
            vminx[1] = ldp->vfrom;
            vminx[2] = ldp->vend;
        }
        if( v->y > miny || (miny != -32000 && v->y == miny && v->x < vlist[vminx[1]].x) )
        {
            miny = v->y;
            vminy[0] = lprev->vfrom;
            vminy[1] = ldp->vfrom;
            vminy[2] = ldp->vend;
        }
        if( v->x > maxx || (maxx != -32000 && v->x == maxx && v->y < vlist[vminx[1]].y) )
        {
            maxx = v->x;
            vmaxx[0] = lprev->vfrom;
            vmaxx[1] = ldp->vfrom;
            vmaxx[2] = ldp->vend;
        }
        if( v->y < maxy || (maxy != 32000 && v->y == maxy && v->x > vlist[vminx[1]].x) )
        {
            maxy = v->y;
            vmaxy[0] = lprev->vfrom;
            vmaxy[1] = ldp->vfrom;
            vmaxy[2] = ldp->vend;
        }

        v = &vlist[ldp->vend];
        if( v->x < minx || (minx != 32000 && v->x == minx && v->y > vlist[vminx[1]].y) )
        {
            minx = v->x;
            vminx[0] = ldp->vfrom;
            vminx[1] = ldp->vend;
            vminx[2] = lnext->vend;
        }
        if( v->y > miny || (miny != -32000 && v->y == miny && v->x < vlist[vminx[1]].x) )
        {
            miny = v->y;
            vminy[0] = ldp->vfrom;
            vminy[1] = ldp->vend;
            vminy[2] = lnext->vend;
        }
        if( v->x > maxx || (maxx != -32000 && v->x == maxx && v->y < vlist[vminx[1]].y) )
        {
            maxx = v->x;
            vmaxx[0] = ldp->vfrom;
            vmaxx[1] = ldp->vend;
            vmaxx[2] = lnext->vend;
        }
        if( v->y < maxy || (maxy != 32000 && v->y == maxy && v->x > vlist[vminx[1]].x) )
        {
            maxy = v->y;
            vmaxy[0] = ldp->vfrom;
            vmaxy[1] = ldp->vend;
            vmaxy[2] = lnext->vend;
        }
    }

    if( minx != 32000 )
        vchosen = vminx;
    else if( maxx != -32000 )
        vchosen = vmaxx;
    else if( miny != 32000 )
        vchosen = vminy;
    else if( maxy != -32000 )
        vchosen = vmaxy;

    if( !vchosen )
        return 1;

    vprev = &vlist[vchosen[0]];
    vmid = &vlist[vchosen[1]];
    vnext = &vlist[vchosen[2]];

    angle1 = _angle( vprev->x, vprev->y, vmid->x, vmid->y );
    angle2 = _angle( vmid->x, vmid->y, vnext->x, vnext->y );
    angle3 = 180 - angle1 + angle2;

    if( angle3 > 360 )
        angle3 -= 360;
    else if( angle3 < 0 )
        angle3 += 360;

    return (angle3 < 180);
}

short vertex_checkjoin( short vertex );

void PolyScr2Map( int points[][2], short npoints, short snap, bool fSimple = FALSE )
{
    short   i, x, y;
    short   osnap = tog_snap;

    tog_snap = snap;

    for( i = 0; i < npoints; i++ )
    {
        _mapsnap( &x, &y,
                  xstart + _scrx2map( points[i][0] ),
                  ystart + _scry2map( points[i][1] ), fSimple );
        points[i][0] = x;
        points[i][1] = y;
    }

    tog_snap = osnap;
}

void PolyMap2Scr( int points[][2], short npoints, short snap )
{
    short   i, x, y;
    short   osnap = tog_snap;

    tog_snap = snap;

    for( i = 0; i < npoints; i++ )
    {
        _mapsnap( &x, &y, points[i][0], points[i][1], TRUE );
        points[i][0] = _mapx2scr( x - xstart );
        points[i][1] = _mapy2scr( y - ystart );
    }

    tog_snap = osnap;
}

uchar islinedefinside( short object, short x1, short y1, short x2, short y2 );

short SplitLineAtVertex( short lobj, short vertex )
/*-------------------------------------------------------------------------*
 * Returns the new line
 *-------------------------------------------------------------------------*/
{
    short new_line;

    void find_slope_point( short *dx, short *dy, short xpos, short ypos,
                       VERTEX *vfrom, VERTEX *vend );

    /* Only split a line if we're joining... */
    find_slope_point( &(vlist[vertex].x), &(vlist[vertex].y),
                        vlist[vertex].x, vlist[vertex].y,
                        &vlist[linelist[lobj].vfrom],
                        &vlist[linelist[lobj].vend] );

    /* Create a new line and join the two with our new vertex. */
    new_line = obj_insert( MODE_LINE, lobj, 0, 0 );

    linelist[lobj].vend = vertex;
    linelist[new_line].vfrom = vertex;

    return new_line;
}

void lineSetI2S( LINEDEF *ldp )
/*-------------------------------------------------------------------------*
 * Sets a line's impassable / 2-sided attributes based on its sidedefs
 *-------------------------------------------------------------------------*/
{
    if( ldp->lsdef == -1 )
    {
        ldp->attribs |= LA_IMPASS;
        ldp->attribs &= ~LA_TSIDE;
    }
    else
    {
        ldp->attribs |= LA_TSIDE;
        ldp->attribs &= ~LA_IMPASS;
    }
}

short lineCreatewithSplits( short lidx, short v1idx, short v2idx, short s1idx, short s2idx )
/*-------------------------------------------------------------------------*
 * Creates a new line from the given coordinates, splitting existing
 * lines if necessary.  if the line then overlaps an existing line,
 * fixes the sector references .. yah that's it.
 *
 * returns a number of the line it split, or -1.
 *-------------------------------------------------------------------------*/
{
    LINEDEF *ldp, *ldp2;
    extern  short _vcj_line, _vc_lean;
    short   split, ovc = _vc_lean;

    _vc_lean = _scrx2map( 3 );

    ldp = &linelist[lidx];

    vertex_createjoin( v1idx, &v1idx );
    split = _vcj_line;

    vertex_createjoin( v2idx, &v2idx );
    if( _vcj_line != -1 )
        split = _vcj_line;

    _vc_lean = ovc;

    ldp->vfrom = v1idx;
    ldp->vend = v2idx;

    sidelist[ldp->rsdef].sector = s1idx;

    if( s2idx != -1 )
    {
        ldp->lsdef = obj_insert( MODE_SIDEDEF, -1, 0, 0 );
        sidelist[ldp->lsdef].sector = s2idx;
    }

    return _vcj_line;
}

void CreatePolyRegion( int points[][2], short npoints )
/*-------------------------------------------------------------------------*
 * Uses the extern polygon list points to create a new set of lines.
 *-------------------------------------------------------------------------*/
{
    short       rg_inside, tmp, rsdef, lsdef, copyline = -1;
    short       i, v1idx, psector = -1, lstart, lend, sectidx, isinward, ncheck = 0, lidx, i2, v2idx, direc;
    short       spx = -1, spy, vr_first = nvertex, x_same = -32000, y_same = -32000, combline;
    short       varray[64];
    short       rg_minx = 32000, rg_miny = 32000,
                rg_maxx = -32000, rg_maxy = -32000;
    short       llist[128], *llp = llist, llcount = 0;
    short       llist2[128], *llp2 = llist2, llcount2 = 0;
    LINEDEF     *ldp, *ldp2;
    VERTEX      *v;
    extern      uchar *og_ignorethese;
    void        line_combinesame( short *llist, short llcount );

    /* find the first vertex that doesn't touch any lines */
    for( i = 0; i < npoints; i++ )
    {
        if( points[i][0] > rg_maxx )    rg_maxx = points[i][0];
        if( points[i][1] > rg_maxy )    rg_maxy = points[i][1];
        if( points[i][0] < rg_minx )    rg_minx = points[i][0];
        if( points[i][1] < rg_miny )    rg_miny = points[i][1];

        if( x_same == -32000 )
            x_same = points[i][0];
        if( y_same == -32000 )
            y_same = points[i][1];

        /* so they're NOT the same */
        if( points[i][0] != x_same )
            x_same = -32001;
        if( points[i][1] != y_same )
            y_same = -32001;

        if( spx == -1 &&
            obj_get( MODE_LINE, points[i][0]-2, points[i][1]-2, points[i][0]+2, points[i][1]+2 ) == -1 )
        {
            spx = points[i][0];
            spy = points[i][1];
        }
    }

    if( x_same != -32001 || y_same != -32001 )
    {
        /* they're all on one of these axes */
        return;
    }

    undo_init();

    /* if no point doesn't touch any lines, make it the first centerpoint of a line
     * that doesn't touch existling lines (this suuuuuuucks.) */
    if( spx == -1 )
    {
        for( i = 0; i < npoints; i++ )
        {
            i2 = ranged_value( i+1, 0, npoints );

            if( obj_get( MODE_LINE, points[i][0], points[i][1], points[i2][0], points[i2][1] ) == -1 )
            {
                spx = (points[i][0]+points[i2][0])/2;
                spy = (points[i][1]+points[i2][1])/2;
                break;
            }
        }
    }

    if( spx == -1 )
    {
        /* center of the bleedin' region then. */
        spx = (rg_minx+rg_maxx)/2;
        spy = (rg_miny+rg_maxy)/2;
    }

    /* get the parent sector */
    psector = obj_get( MODE_SECTOR, spx, spy, spx, spy );

    /* create the new sector */
    sectidx = obj_insert( MODE_SECTOR, psector, 0, 0 );

    /* create vertices */
    for( i = 0; i < npoints; i++ )
        varray[i] = obj_insert( MODE_VERTEX, -1, points[i][0], points[i][1] );

    og_ignorethese = (uchar*) memget( MAXOBJECT );

    lstart = nline;

    for( i = 0; i < npoints; i++ )  /* create lines */
        og_ignorethese[obj_insert( MODE_LINE, -1, 0, 0 )] = 1;

    lend = nline;

    for( i = 0; i < npoints; i++ )
    {
        /* create liness */
        i2 = ranged_value( i+1, 0, npoints );
        tmp = lineCreatewithSplits( lstart+i, varray[i], varray[i2], sectidx, psector );

        if( tmp != -1 )
            copyline = tmp;
    }

    memfree( og_ignorethese );
    og_ignorethese = NULL;

    /* make sure the region is facing in the right direction */
    isinward = isregioninward( lstart, lend );

    if( !isinward )
    {
        /* need to reverse it */
        for( i = lstart, ldp = linelist+lstart; i < lend; i++, ldp++ )
            swap2b( ldp->vfrom, ldp->vend );
    }

    for( i = lstart; i < nline; i++ )
    {
        v1idx = linelist[i].vfrom;
        v2idx = linelist[i].vend;

        *llp++ = i;
        ++llcount;

        for( i2 = 0, v = vlist; i2 < vr_first; i2++, v++ )
        {
            if( i2 == v1idx || i2 == v2idx )
                continue;

            if( islinedefinside( i, v->x-2, v->y-2, v->x+2, v->y+2 ) )
                SplitLineAtVertex( i, i2 );
        }
    }

    for( i = 0, llp = llist; i < llcount; i++, llp++ )
    {
        ldp = &linelist[*llp];

        /* check for overlaps and delete existing lines */
        combline = -1;

        v1idx = ldp->vfrom;
        v2idx = ldp->vend;

        for( i2 = 0, ldp2 = linelist; i2 < nline; i2++, ldp2++ )
        {
            if( ldp2->vfrom == v1idx && ldp2->vend == v2idx && i2 != *llp )
            {
                direc = 0;
                combline = i2;
                break;
            }
            else if( ldp2->vfrom == v2idx && ldp2->vend == v1idx && i2 != *llp )
            {
                direc = 1;
                combline = i2;
                break;
            }
        }

        if( combline != -1 )
        {
            if( copyline == -1 )
                copyline = combline;

            /* new sector is facing INWARD */

            if( !direc )
            {
                if( ldp2->lsdef == -1 )
                {
                    /* same direction overlap of a one-sided line.  make sure
                     * this line is one-sided. */
                    ldp->lsdef = -1;
                }
                else
                {
                    /* same direction overlap of a two-sided line.  set the
                     * second side's sector to its second side's. */
                    sidelist[ldp->lsdef].sector = sidelist[ldp2->lsdef].sector;
                }
            }
            else
            {
                /* overlap a line, in the opposite direction.  set new
                 * second sidedef to its first sidedef.
                 */
                if( ldp->lsdef == -1 )
                    ldp->lsdef = obj_insert( MODE_SIDEDEF, ldp->rsdef, 0, 0 );
                sidelist[ldp->lsdef].sector = sidelist[ldp2->rsdef].sector;
            }

            linelist[combline].vfrom = linelist[combline].vend = 0;
        }
    }

    /* do sector attributes */
    if( psector != -1 )
        memcpy( &sectorlist[sectidx], &sectorlist[psector], sizeof( SECTOR ) );
    else if( copyline != -1 )
    {
        ldp = &linelist[copyline];
        if( ldp->rsdef > -1 )
            memcpy( &sectorlist[sectidx], &sectorlist[sidelist[ldp->rsdef].sector], sizeof( SECTOR ) );
        else if( ldp->lsdef > -1 )
            memcpy( &sectorlist[sectidx], &sectorlist[sidelist[ldp->lsdef].sector], sizeof( SECTOR ) );
    }

    for( i = 0, llp = llist; i < llcount; i++, llp++ )
    {
        ldp = &linelist[*llp];

        /* set its attributes */
        lineSetI2S( ldp );

        _WFfixtexnames( *llp, 0, 2 );
        _WFfixtexnames( *llp, 1, 2 );
    }

    /* fix sector references */
    fixsectorrefs( sectidx, psector );

    map_purge( MODE_ALL );

    nMadeChanges++;
    nMapChanges++;

    undo_setvars();
}

short _WFcompleteregion( short lstart, short lend )
/*-------------------------------------------------------------------------*
 * Completes a region bounded by the lines from lstart to lend.            *
 * See documentation for WADassist|Complete Created Regions for full       *
 * explanation of what happens here.                                       *
 *-------------------------------------------------------------------------*/
{
    uchar   rg_indep;       /* 1 if independent */
    uchar   rg_inside;      /* 1 if inside */
    short   rg_cx, rg_cy;     /* center x and y */
    short   rg_minx = 32000, rg_maxx = -32000,
            rg_miny = 32000, rg_maxy = -32000;
    short   i, tmp, v1idx, v2idx, contactline, newsector;
    short   _copyftx, isinward, oldvclean;
    extern  short _vc_lean;
    VERTEX  *v1, *v2;
    LINEDEF *ldp, ld_split, ld_dummy;
    SIDEDEFM *sdp;
    extern  byte *og_ignorethese;

    /* Destroy all unused vertices */
    map_purge( MODE_VERTEX );

    /* The first thing we have to do is determine what kind of region
     * this is: independent outside, independent inside, connected
     * outside, connected inside.  What this means:
     *
     * "independent" means the region is composed entirely of vertices
     * not connected to any lines out-of-range of "lstart" and "lend".
     * "connected" is the opposite.
     *
     * "inside" means the region is contained entirely inside an
     * existing sector.  this affects which sidedefs are created when
     * the region is completed.  "outside" is the opposite.
     *
     */

    /* Find the centerpoint of the region */
    for( i = lstart; i < lend; i++ )
    {
        v1 = &vlist[linelist[i].vfrom];
        v2 = &vlist[linelist[i].vend];
        if( v1->x > rg_maxx )   rg_maxx = v1->x;
        if( v2->x > rg_maxx )   rg_maxx = v2->x;
        if( v1->y > rg_maxy )   rg_maxy = v1->y;
        if( v2->y > rg_maxy )   rg_maxy = v2->y;
        if( v1->x < rg_minx )   rg_minx = v1->x;
        if( v2->x < rg_minx )   rg_minx = v2->x;
        if( v1->y < rg_miny )   rg_miny = v1->y;
        if( v2->y < rg_miny )   rg_miny = v2->y;
    }

    rg_cx = (rg_minx + rg_maxx) / 2;
    rg_cy = (rg_miny + rg_maxy) / 2;

    /* Figure out if it's connected */
    rg_indep = 1;

    /* determine which sector it's in, if any.  if it's IN a sector, all of the
     * created lines & vertices will be in the sector too, so just choose any
     * one of them (like the start one) to figure it out. */
    tmp = linelist[lstart].vend;
    nline -= _lc_count;    /* THIS IS TERRIBLE */
    if( _lc_count > 1 && rg_indep )
        psector = obj_get( MODE_SECTOR, vlist[tmp].x, vlist[tmp].y, vlist[tmp].x, vlist[tmp].y );
    else
        psector = obj_get( MODE_SECTOR, rg_cx, rg_cy, rg_cx, rg_cy );
    nline += _lc_count;    /* but oh well. */

    /* Set the type flags */
    rg_inside = ((psector == -1) ? 0 : 1);

    tmp = linelist[lstart].vfrom;
    oldvclean = _vc_lean;
    //_vc_lean = gridstep / 2;
    tog_join = 1;

    JoinInfo ji;

    if( CheckJoin( tmp, ji ) &&
        !(ji.fJoinType == jiVertex && ji.iVertex >= _lc_vfirst) )
    {
        // one is connected - make sure the other is if there's more than one line
        tmp = linelist[lend-1].vend;
        i = CheckJoin( tmp, ji );
        _vc_lean = oldvclean;
        tog_join = old_join;

        if( !i && _lc_count > 1 )
        {
            tmp = message_box( MB_YESNO, st_dcksays,
                " I can't complete this region because only one vertex is\n"
                "connected to an existing vertex. Exactly two connected\n"
                "vertices are required in order to create a region like\n"
                "this. Do you want to continue creating lines?" );
            if( tmp == MB_YES )
                return 1;
            else
            {
                _lc_deletecreated();
                return 0;
            }
        }

        /* done, eh. */
        //_vc_lean = gridstep / 2;
        tog_join = 1;

        v1idx = linelist[lstart].vfrom;
        v2idx = linelist[lend-1].vend;

        if( _lc_count == 1 )
        {
            memcpy( &ld_split, &linelist[lstart], sizeof( LINEDEF ) );
            --nline;
        }

        vertex_createjoin( v1idx, &v1idx );
        vertex_createjoin( v2idx, &v2idx );

        if( _lc_count == 1 )
        {
            memcpy( &linelist[nline++], &ld_split, sizeof( LINEDEF ) );
            lstart = nline-1;
            lend = nline;
        }

        rg_indep = 0;

        linelist[lstart].vfrom = v1idx;
        linelist[lend-1].vend = v2idx;
    }

    tog_join = old_join;
    _vc_lean = oldvclean;

    /* Check for number of lines */
    if( !rg_inside && _lc_count < 2 )
    {
        tmp = message_box( MB_YESNO, st_dcksays,
          " I can't complete this region because there aren't enough\n"
          "lines to do so (more than one is required if you're not\n"
          "creating a sector split.)  Do you want to continue\n"
          "creating lines?" );
        if( tmp == MB_YES )
            return 1;
        else
        {
            _lc_deletecreated();
            return 0;
        }
    }

    if( _lc_count == 1 )
    {
        int dotype;
        bool fGetType = TRUE;

        if( fGetType )
        {
            /* Does the user want to create a split line or a normal line? */
            dotype = MenuPop( 0, " Create...", "MN_ONELINE",
                          " Sector split       ", NULL,
                          " 2-sided line       ", NULL,
                          NULL );
        }
        else
            dotype = 1;

        if( dotype == -1 )
        {
            _lc_deletecreated();
            return 0;
        }

        if( !dotype )
        {
            if( _WFsplitsector( lstart ) )       /* Yessplit */
            {
                _WFfixtexnames( lstart, 0, 0 );
                _WFfixtexnames( lstart, 1, 0 );
            }
            else
            {
                if( _lc_count )
                    goto _mk2side;
            }
        }
        else
        {
_mk2side:
            ldp = &linelist[lstart];
            sdp = &sidelist[ldp->rsdef];

            /* Modify the line to indicate two-sided, passable */
            ldp->attribs = LA_TSIDE;
            /* Modify the right side to indicate no textures.. it's
             * a transparent line. */
            sdp->sector = psector;
            sdp->utxidx = sdp->ltxidx = sdp->ftxidx = 0;    /* Notex */
            ldp->lsdef = obj_insert( MODE_SIDEDEF, ldp->rsdef, 0, 0 );
        }

        return 0;
    }

    /* Independent. */
    if( rg_indep )
    {
        /* Connect the last two vertices */
        linelist[lend-1].vend = linelist[lstart].vfrom;
        isinward = isregioninward( lstart, lend );
    }
    else
    {
        contactline = -1;

        /* Check: the first line's first vertex must already be connected
         * to the last line's ending vertex.  v1idx and v2idx are set
         * up earlier to avoid conflicts when flipping linedefs. */
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( !inrange( i, lstart, lend-1 ) )
            {
                if( (ldp->vfrom == v1idx && ldp->vend == v2idx) ||
                    (ldp->vend == v1idx && ldp->vfrom == v2idx) )
                {
                    contactline = i;
                    break;
                }
            }
        }

        if( contactline == -1 )
        {
            if( message_box( MB_YESNO, "Notice",
              " I can't complete this region because its start and end\n"
              "vertexes aren't connected by a linedef.  Would you like\n"
              "to delete the created lines before aborting?" ) == MB_YES )
                _lc_deletecreated();
            return 0;
        }

        /* HERE: The region lines' right sidedef points to the new sector. */
        memcpy( &ld_dummy, &linelist[lend], sizeof( LINEDEF ) );
        linelist[lend].vfrom = v2idx;   /* dummy */
        linelist[lend].vend = v1idx;   /* dummy */
        isinward = isregioninward( lstart, lend+1 );
        memcpy( &linelist[lend], &ld_dummy, sizeof( LINEDEF ) );
    }

    /* Create a new sector if it's a region split or a new sector
     * is being created. */
    newsector = obj_insert( MODE_SECTOR, psector, 0, 0 );

    /* Modify sidedefs for each line to say they're in our new sector */
    for( i = lstart; i < lend; i++ )
        sidelist[linelist[i].rsdef].sector = newsector;

    if( !isinward )
    {
        for( i = lstart; i < lend; i++ )
        {
            tmp = linelist[i].vfrom;
            linelist[i].vfrom = linelist[i].vend;
            linelist[i].vend = tmp;
        }
    }

    /* HERE: The region lines all face inward, and their right sidedefs
     * are set to the new sector. */

    if( rg_indep )
    {
        if( rg_inside )
        {
            for( i = lstart, ldp = linelist+lstart; i < lend; i++, ldp++ )
            {
                /* We're creating a separate sector, so we have to create
                 * left sidedefs too. */
                ldp->lsdef = obj_insert( MODE_SIDEDEF, -1, 0, 0 );
                sdp = &sidelist[ldp->lsdef];
                sdp->sector = psector;
                sdp->ftxidx = sdp->utxidx = sdp->ltxidx = 0; /* No textures dude */
                ldp->attribs = LA_TSIDE;

                /* aaaand get rid of the middle texture on the right sidedef */
                sidelist[ldp->rsdef].ftxidx = 0;
            }
        }
    }
    else
    {
        ldp = &linelist[contactline];

        if( !rg_inside )        /* external new regions */
        {
            memcpy( &sectorlist[newsector],
                    &sectorlist[sidelist[ldp->rsdef].sector],
                    sizeof( SECTOR ) );

            sectorlist[newsector].sectag = 0;   /* NO SHARE THIS UNKA */

            ldp->attribs |= LA_TSIDE;
            ldp->attribs &= ~LA_IMPASS;

            /* See if we can get some new full textures */
            _copyftx = sidelist[ldp->rsdef].ftxidx;
            if( !_copyftx ) _copyftx = _def_ftxidx;

            /* use linelist[lstart].rsdef as the source because it'll be
             * facing inwards.. don't forget to fix this line with
             * _WFfixtexnames().. */
            if( ldp->lsdef == -1 )
                ldp->lsdef = obj_insert( MODE_SIDEDEF, linelist[lstart].rsdef, 0, 0 );

            /* Don't want any full textures on the connecting line */
            sidelist[ldp->lsdef].ftxidx = sidelist[ldp->rsdef].ftxidx = 0;
            _WFfixtexnames( contactline, 0, 0 );
            _WFfixtexnames( contactline, 1, 0 );

            /* Copy the full textures over to the new lines */
            for( i = lstart, ldp = linelist+lstart; i < lend; i++, ldp++ )
                sidelist[ldp->rsdef].ftxidx = _copyftx;
        }
        else
        {
            if( sidelist[ldp->rsdef].sector == psector )
                sidelist[ldp->rsdef].sector = newsector;
            else if( ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == psector )
                sidelist[ldp->lsdef].sector = newsector;

            for( i = lstart, ldp = linelist+lstart; i < lend; i++, ldp++ )
            {
                ldp->lsdef = obj_insert( MODE_SIDEDEF, -1, 0, 0 );
                sdp = &sidelist[ldp->lsdef];
                sdp->sector = psector;
                sdp->ftxidx = sdp->utxidx = sdp->ltxidx = 0; /* No textures dude */
                ldp->attribs = LA_TSIDE;

                /* aaaand get rid of the middle texture on the right sidedef */
                sidelist[ldp->rsdef].ftxidx = 0;
            }
        }
    }

    if( rg_inside )
    {
        /* reverse the lines - they all face inward at the moment, but
         * sectors-in-sectors should face outward for trigger purposes
         * and all that. */
        for( i = lstart; i < lend; i++ )
        {
            /* have to do this too */
            swap2b( linelist[i].vfrom, linelist[i].vend );
            swap2b( linelist[i].rsdef, linelist[i].lsdef );
        }
    }

    nMadeChanges++;
    nMapChanges++;
    return 0;
}
