/*-------------------------------------------------------------------------*
 * STAIRS.C:                                                               *
 * Functions for creating stairs.  Wowo wowoow.                            *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include <math.h>

#include "gui\dialog.hpp"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

short line_a, line_b;         /* The lines. */
short ivstarta, ivstartb;
short ivenda, ivendb;
short direc_a, direc_b;
VERTEX *vstarta, *vstartb;
VERTEX *venda, *vendb;
LINEDEF *ldp_a, *ldp_b;
float len_a, len_b;         /* Lengths of the lines */
float inc_a, inc_b;         /* Incrementation values to create each stair */
short nstair = 6;                 /* Number of stairs to create */

static void getstairpos( short *x, short *y, short stairnum, short aorb );
static short getstairsector( short x1, short y1, short x2, short y2 );
static short dostairdlg( void );

struct
{
    int     nos;
    int     sh;
    int     fsf;
    int     lsf;
    int     fsc;
    int     lsc;
    uchar   reverse;
    uchar   ceil;
    uchar   hidden;

} sinfo =
{
    8,
    16,
    0,
    0,
    0,
    0,
    0,
    1,
    0
};

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

void drawoneline( short lineno );

void stair_do( void )
/*-------------------------------------------------------------------------*
 * "Does" the stairs.                                                      *
 *-------------------------------------------------------------------------*/
{
    short     curstairline;        /* Line inxed for current stair */
    short     x1, y1, x2, y2;     /* Line for current stair */
    LINEDEF *ldp, *sl_a, *sl_b;
    SECTOR  *secp;
    short     newv_a, newv_b, theline;
    short     shortline_a, shortline_b;
    short     othersector, origsector;
    short     curstairsector;      /* Current stair sector */
    short     curstairheight = 0, incstairheight = 0;
    short     i, tmpline, laststairexist, firststairexist, lastceil;
    short     curstepdist_a, curstepdist_b;       /* for alignment only */

    if( editmode != MODE_LINE )
    {
        message_box( MB_OK, st_dcksays,
                     "You must be in line mode to create stairs.\n" );
        return;
    }
    if( obj_nmarked() != 2 )
    {
        message_box( MB_OK, st_dcksays,
                     "You must mark exactly two lines before\n"
                     "creating stairs." );
        return;
    }

    /* Find the border lines */
    line_a = line_b = -1;
    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        if( obj_ismarked( i ) )
        {
            if( line_a == -1 )
                line_a = i;
            else
            {
                /* Done ! */
                line_b = i;
                break;
            }
        }
    }

    undo_init();

    ldp_a = &linelist[line_a];
    ldp_b = &linelist[line_b];

    curstairsector = origsector = obj_get( MODE_SECTOR,
                (vlist[ldp_a->vfrom].x + vlist[ldp_a->vend].x) / 2,
                (vlist[ldp_a->vfrom].y + vlist[ldp_a->vend].y) / 2,
                (vlist[ldp_b->vfrom].x + vlist[ldp_b->vend].x) / 2,
                (vlist[ldp_b->vfrom].y + vlist[ldp_b->vend].y) / 2 );
    secp = &sectorlist[curstairsector];
    sinfo.fsf = secp->fheight;
    sinfo.lsf = sinfo.fsf + sinfo.nos * sinfo.sh;
    sinfo.fsc = secp->cheight;
    sinfo.lsc = sinfo.fsc + sinfo.nos * sinfo.sh;

    /* Okay, now do the dialog box. */
    if( !dostairdlg() )     /* aborted .. returns 0. */
        return;

    /* Increment nstair if there's no line between the end vertexes */
    tmpline = obj_linefind( LF_BEGIN, LF_VANY, ivenda, LF_END );

    laststairexist = 0;

    while( tmpline != -1 )
    {
        ldp = &linelist[tmpline];
        if( ldp->vfrom == ivenda && ldp->vend == ivendb )
        {
            laststairexist = 1;
            break;
        }
        if( ldp->vend == ivenda && ldp->vfrom == ivendb )
        {
            laststairexist = 1;
            break;
        }
        tmpline = obj_linefind( LF_CONTINUE );
    }

    /* Check for a line between the start vertexes of the lines */
    tmpline = obj_linefind( LF_BEGIN, LF_VANY, ivstarta, LF_END );

    firststairexist = 0;

    while( tmpline != -1 )
    {
        ldp = &linelist[tmpline];
        if( ldp->vfrom == ivstarta && ldp->vend == ivstartb )
        {
            firststairexist = 1;
            break;
        }
        if( ldp->vend == ivstarta && ldp->vfrom == ivstartb )
        {
            firststairexist = 1;
            break;
        }
        tmpline = obj_linefind( LF_CONTINUE );
    }

/*-------------------------------------------------------------------------*
 * Here's the basic algorithm:                                             *
 * 1. Create a line for the step.                                          *
 * 2. Split the remaining sector with this line.                           *
 * 3. Set the floor / ceiling heights (and fix textures.)                  *
 *-------------------------------------------------------------------------*/
    ++nstair;
    lastceil = sinfo.fsc;        /* ceiling heights for texture alignment */

    for( i = 0, curstairheight = sinfo.fsf; i < nstair; i++ )
    {
        delay( 10 );

        getstairpos( &x1, &y1, i, 0 );
        getstairpos( &x2, &y2, i, 1 );

        if( i == nstair-1 && laststairexist )
            break;

        if( !(!i && firststairexist) )
        /* Determines the vertexes for the next stair */
        {
            if( !i )        /* first stair - already has vertexes */
            {
                newv_a = ivstarta;
                newv_b = ivstartb;
                sl_a = ldp_a;
                sl_b = ldp_b;
            }
            else if( i == nstair-1 )    /* last stair - already has vertexes */
            {
                newv_a = ivenda;
                newv_b = ivendb;
                sl_a = ldp_a;       /* these are for sector detection below. */
                sl_b = ldp_b;
            }
            else        /* middle stair - create new vertexes */
            {
                newv_a = obj_insert( MODE_VERTEX, VTX_CREATE, x1, y1 );
                newv_b = obj_insert( MODE_VERTEX, VTX_CREATE, x2, y2 );

                /* Create the two lines by splitting lines a and b at
                 * x1, y1 and x2, y2 respectively. */
                shortline_a = obj_insert( MODE_LINE, line_a, 0, 0 );
                shortline_b = obj_insert( MODE_LINE, line_b, 0, 0 );
                sl_a = &linelist[shortline_a];
                sl_b = &linelist[shortline_b];

                if( !direc_a )
                {
                    /* Going UP */
                    sl_a->vfrom = ldp_a->vfrom;
                    sl_a->vend = newv_a;
                    ldp_a->vfrom = newv_a;
                }
                else
                {
                    /* Going DOWN */
                    sl_a->vend = ldp_a->vend;
                    sl_a->vfrom = newv_a;
                    ldp_a->vend = newv_a;
                }

                if( !direc_b )
                {
                    /* Going UP */
                    sl_b->vfrom = ldp_b->vfrom;
                    sl_b->vend = newv_b;
                    ldp_b->vfrom = newv_b;
                }
                else
                {
                    /* Going DOWN */
                    sl_b->vend = ldp_b->vend;
                    sl_b->vfrom = newv_b;
                    ldp_b->vend = newv_b;
                }
            }

            /* this is for _WFsplitsector() -- it requires psector set./ */
            setpsector( (othersector =
        getstairsector( (vlist[sl_a->vfrom].x + vlist[sl_a->vend].x) / 2,
                        (vlist[sl_a->vfrom].y + vlist[sl_a->vend].y) / 2,
                        (vlist[sl_b->vfrom].x + vlist[sl_b->vend].x) / 2,
                        (vlist[sl_b->vfrom].y + vlist[sl_b->vend].y) / 2 ) ) );

            /* Create the stair line */
            curstairline = obj_insert( MODE_LINE, -1, 0, 0 );
            ldp = &linelist[curstairline];

            ldp->vfrom = newv_a;
            ldp->vend = newv_b;

            drawoneline( curstairline );

            /* Do the actual sector split now. */
            _WFsplitsector( curstairline );
        }
        else
            othersector = -1;

        if( i )
        {
            /* Horizontal texture alignment */
            curstepdist_a = i * inc_a;
            curstepdist_b = i * inc_b;

            if( !direc_a )
                sidelist[sl_a->rsdef].wxofs = curstepdist_a;
            else
                sidelist[sl_a->rsdef].wxofs = len_a - curstepdist_a;

            if( !direc_b )
                sidelist[sl_b->rsdef].wxofs = curstepdist_b;
            else
                sidelist[sl_b->rsdef].wxofs = len_b - curstepdist_b;
        }

        /* Okay, now we have two sectors: the remaining portion to be "staired",
         * and the sector we just created.  What we're interested in is the
         * latter; here's where we get it and set the height.
         *
         * Get the sector at the centerpoint of the line between the centerpoints
         * of the two remaining lines. */
        curstairsector = obj_get( MODE_SECTOR,
                 (vlist[ldp_a->vfrom].x + vlist[ldp_a->vend].x) / 2,
                 (vlist[ldp_a->vfrom].y + vlist[ldp_a->vend].y) / 2,
                 (vlist[ldp_b->vfrom].x + vlist[ldp_b->vend].x) / 2,
                 (vlist[ldp_b->vfrom].y + vlist[ldp_b->vend].y) / 2 );
        if( curstairsector == othersector )
            curstairsector = nsector-1;

        secp = &sectorlist[curstairsector];
        secp->fheight = curstairheight;
        if( sinfo.ceil )
            secp->cheight = sinfo.fsc + incstairheight;
        else
            secp->cheight = sinfo.fsc;

        if( i == 0 && sinfo.hidden )    /* Get a tag for the sector */
            secp->sectag = firstfreetag();

        theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, curstairsector, LF_END );

        while( theline != -1 )
        {
            if( linelist[theline].attribs & LA_TSIDE )
            {
                /* set both lower textures to the stair texture */
                sidelist[linelist[theline].rsdef].ltxidx = _def_stairtxidx;
                sidelist[linelist[theline].lsdef].ltxidx = _def_stairtxidx;
            }

            _WFfixtexnames( theline, 2, 2 );

            theline = obj_linefind( LF_CONTINUE );
        }

        if( sinfo.hidden )
            sectorlist[curstairsector].fheight = sinfo.fsf;

        if( i != (nstair-1) )
        {
            curstairheight += sinfo.sh;
            incstairheight += sinfo.sh;
        }

        if( i )
        {
            /* Vertical texture alignment */
            sidelist[sl_a->rsdef].wyofs = lastceil - secp->cheight;
            sidelist[sl_b->rsdef].wyofs = lastceil - secp->cheight;
        }
    }

    /* this is a kludge, a patch, a bandaid, and i don't care :P */
    if( sinfo.reverse && !sinfo.hidden )
    {
        secp = &sectorlist[origsector];

        secp->fheight = curstairheight;
        if( sinfo.ceil )
            secp->cheight = sinfo.fsc + incstairheight;
        else
            secp->cheight = sinfo.fsc;

        theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, origsector, LF_END );

        while( theline != -1 )
        {
            if( linelist[theline].attribs & LA_TSIDE )
            {
                /* set both lower textures to the stair texture */
                sidelist[linelist[theline].rsdef].ltxidx = _def_stairtxidx;
                sidelist[linelist[theline].lsdef].ltxidx = _def_stairtxidx;
            }

            _WFfixtexnames( theline, 2, 2 );

            theline = obj_linefind( LF_CONTINUE );
        }
    }

    obj_unmark( line_a );
    obj_unmark( line_b );

    post_fullupdate();      /* I'd say so.  :) */
    nMadeChanges++;
    nMapChanges++;

    undo_setvars();
}

static short getstairsector( short x1, short y1, short x2, short y2 )
/*-------------------------------------------------------------------------*
 * Returns the sector number that the next stair will be created in.       *
 *-------------------------------------------------------------------------*/
{
    return( obj_get( MODE_SECTOR, x1, y1, x2, y2 ) );
}

/*-------------------------------------------------------------------------*
 *
 * Stair dialog functions
 *
 *-------------------------------------------------------------------------*/

static Dialog     *pStairDlg;
static fldString  *pfnos, *pffsf, *pffsc, *pflsf, *pflsc, *pfsh;
static fldCheckBox *pfceil;

class fldStairPreview : public fldRegion
{
public:
    fldStairPreview( int _iAngle ) : fldRegion(), iAngle(_iAngle) {}

    void SetReverse( bool fReverse )
        { this->fReverse = fReverse; OnDraw(); }
    void Draw();

private:
    int iAngle;
    bool fReverse;
};

void fldStairPreview::Draw()
{
    float angle = (float) (iAngle + (fReverse ? 180 : 0)) * 0.0174533;
    int x1, y1, x2, y2;

    fldRegion::Draw();

    Rect rcRect = GetRectInParent();

    int iRadius = min( rcRect.Height(), rcRect.Width() ) / 2 - 6;

    x1 = rcRect.CenterX() - (iRadius * cos( angle ));
    y1 = rcRect.CenterY() + (iRadius * sin( angle ));
    x2 = rcRect.CenterX() + (iRadius * cos( angle ));
    y2 = rcRect.CenterY() - (iRadius * sin( angle ));

    setcolor( C_WHITE );
    line( x1, y1, x2, y2 );

    setcolor( C_GREEN );
    gr_fillcircle( x1, y1, 4 );
    setcolor( C_RED );
    gr_fillcircle( x2, y2, 4 );
}

static int ffOnReverseChanged( fldCheckBox *pField )
{
    fldStairPreview *pPreview = (fldStairPreview*) pField->GetParent()->GetField( "Preview" );
    pPreview->SetReverse( pField->GetState() );
    return daContinue;
}

static int ffUpdateStairFields( fldString* pField )
/*-------------------------------------------------------------------------*
 * based on the new info in fldp->data, updates the other stair fields.
 *-------------------------------------------------------------------------*/
{
    short       first, last, count, each;

    /* number of stairs changed?  update absolute vars */
    if( pField == pfnos || pField == pfsh || pField == pfceil )
    {
        first = pffsf->GetInteger();    /* first stair floor */
        last = first + pfnos->GetInteger() * pfsh->GetInteger();
        pflsf->SetInteger( last );

        if( pfceil->GetState() )  /* if do ceiling */
        {
            first = pffsc->GetInteger();    /* first stair ceil */
            last = first + pfnos->GetInteger() * pfsh->GetInteger();
            pflsc->SetInteger( last );
        }
        else
        {
            first = pffsc->GetInteger();    /* first stair ceil */
            pflsc->SetInteger( pffsc->GetInteger() );
        }
    }
    else if( pField == pffsf )
    {
	// first stair floor
        first = pffsf->GetInteger();
        last = first + pfnos->GetInteger() * pfsh->GetInteger();
        pflsf->SetInteger( last );
    }
    else if( pField == pflsf )
    {
	// last stair floor
        last = pflsf->GetInteger();
        first = last + pfnos->GetInteger() * pfsh->GetInteger();
        pffsf->SetInteger( last );
    }
    else if( pField == pffsc )
    {
        first = pffsc->GetInteger();    /* get first height */
        last = first + pfnos->GetInteger() * pfsh->GetInteger();
        pflsc->SetInteger( last );
    }

    return daContinue;
}

static short dostairdlg( void )
/*-------------------------------------------------------------------------*
 * Displays the stair dialog box, and then sets the global values.         *
 *-------------------------------------------------------------------------*/
{
    short   rvl;
    short   dist1, dist2, x, y;
    VERTEX  *vtmp;
    fldCheckBox* pfreverse;

    vstarta = &vlist[ivstarta = linelist[line_a].vfrom];
    vstartb = &vlist[ivstartb = linelist[line_b].vfrom];
    venda = &vlist[ivenda = linelist[line_a].vend];
    vendb = &vlist[ivendb = linelist[line_b].vend];

    /* get the stair angle man */
    fldStairPreview *pPreview = new fldStairPreview( obj_lineangle( line_a ) );

    pStairDlg = new Dialog;
    pStairDlg->Create( szDSFile, "StairDlg" );

    VarTransfer_t vtStairs[] =
    {
        { "NOS", (Field**)&pfnos, (PTR) &sinfo.nos,
            "Enter the number of stairs to create" },
        { "FSF", (Field**)&pffsf, (PTR) &sinfo.fsf,
            "Enter the height of the first stair's floor" },
        { "FSC", (Field**)&pffsc, (PTR) &sinfo.fsc,
            "Enter the height of the first stair's ceiling" },
        { "LSF", (Field**)&pflsf, (PTR) &sinfo.lsf,
            "Enter the height of the last stair's floor" },
        { "LSC", (Field**)&pflsc, (PTR) &sinfo.lsc,
            "Enter the height of the last stair's ceiling" },
        { "SH", (Field**)&pfsh, (PTR) &sinfo.sh,
            "Enter the height of each stair here" },
        { "DoCeiling", (Field**)&pfceil, (PTR) &sinfo.ceil, NULL },
        { "Reverse", (Field**)&pfreverse, (PTR) &sinfo.reverse, NULL },
        { "DoHidden", NULL, (PTR) &sinfo.hidden, NULL },

        // end marker
        { NULL, NULL, NULL, NULL }
    };

    pStairDlg->VarSetup( vtStairs );
    pStairDlg->REPLACEREGION( "Preview", pPreview );

    pfnos->SetOnChange( (FFUNC) ffUpdateStairFields );
    pffsf->SetOnChange( (FFUNC) ffUpdateStairFields );
    pffsc->SetOnChange( (FFUNC) ffUpdateStairFields );
    pflsf->SetOnChange( (FFUNC) ffUpdateStairFields );
    pflsc->SetOnChange( (FFUNC) ffUpdateStairFields );
    pfsh->SetOnChange( (FFUNC) ffUpdateStairFields );
    pfceil->SetOnChange( (FFUNC) ffUpdateStairFields );
    pfreverse->SetOnChange( (FFUNC) ffOnReverseChanged );

    rvl = pStairDlg->Process();

    nstair = sinfo.nos;
    delete pStairDlg;

    if( rvl == daExitNoSave )
        return 0;       /* Aborted. */

    /* Now, set the global values. */
    len_a = obj_linelen( line_a );
    len_b = obj_linelen( line_b );

    inc_a = (len_a / (float) nstair);
    inc_b = (len_b / (float) nstair);

    direc_a = direc_b = 0;

    /* Figure out the directions for each line. */
    dist1 = (abs( vstarta->x - vstartb->x ) + abs( vstarta->y - vstartb->y ));
    dist2 = (abs( vstarta->x - vendb->x ) + abs( vstarta->y - vendb->y ));

    /* Set the start / end points for the stairs based on the direction
     * of the two lines and the sinfo.reverse flag.  We always use line A
     * as being in the "right" direction. */
    if( dist1 > dist2 )
    {
        vtmp = vstartb;
        vstartb = vendb;
        vendb = vtmp;
        swap2b( ivstartb, ivendb );
        direc_b = 1;
    }

    if( sinfo.reverse )
    {
        /* reverse both of them. */
        vtmp = vstarta;
        vstarta = venda;
        venda = vtmp;
        swap2b( ivstarta, ivenda );
        direc_a = !direc_a;

        vtmp = vstartb;
        vstartb = vendb;
        vendb = vtmp;
        swap2b( ivstartb, ivendb );
        direc_b = !direc_b;
    }

    /* Done. */
    return 1;
}

static void getstairpos( short *x, short *y, short stairnum, short aorb )
/*-------------------------------------------------------------------------*
 * aorb: 0 = A, 1 = B.                                                     *
 * Returns the X and Y position of the stair's vertex on the appropriate   *
 * line.                                                                   *
 * x = start X + (wanted distance / total distance) * (end X - start X)    *
 *-------------------------------------------------------------------------*/
{
    float   wanted, len;
    VERTEX  *vstart, *vend;

    if( !aorb )
    {
        /* A. */
        wanted = inc_a * stairnum;
        len = len_a;
        vstart = vstarta;
        vend = venda;
    }
    else
    {
        /* B. */
        wanted = inc_b * stairnum;
        len = len_b;
        vstart = vstartb;
        vend = vendb;
    }

    *x = (short) (vstart->x + (wanted / len) * (float)(vend->x - vstart->x));
    *y = (short) (vstart->y + (wanted / len) * (float)(vend->y - vstart->y));
}

