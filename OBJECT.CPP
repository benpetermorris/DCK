/*-------------------------------------------------------------------------*
 * OBJECT.C:                                                               *
 * Functions for manipulating objects, etc.                                *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include <math.h>

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

/* Colors */
#define C_DRAG      C_WHITE2     /* another white */
#define C_TAGGED    C_LIGHTRED

// status definitions for xor operations:
enum
{
    GS_OFF = -1,
    GS_FIRST = 0,
    GS_ON = 1,
    GS_UPDATE = 2,
    GS_LAST = 3
};

uchar   _objhigh_dotagged = 1;  /* highlight tagged objects on obj_highlight ? */

uchar   _vertex_align;
short   _va_from, _va_to;
short   _vcj_line = -1;     /* vertex_createjoin() sets this if it created a line */
short   _vcj_line2 = -1;     /* vertex_createjoin() sets this if it created a line */
VERTEX  *_va_addr;
short   _vc_lean = 2;
byte    *og_ignorethese;       /* of MAXOBJECT when not NULL.  when examining series
                                * of objects in certain functions, indeces in this
                                * are set to 1 to ignore them for the function */

short   lc_inprogress( void );  /* line.c */
void    line_delvsame( void );
void    line_combinesame( short *llist, short llcount );
short   lc_inrange( short obj );

void    SetStatusMsg( char *fmt, ... );
void    UpdateMessageBar();

void    obj_shiftmarksdown( Object& obj );

#define vtx_isdeleted(v)    ((v).x == -32000)
#define fixdirec(a,b)       { short tmp; if( b < a ) { tmp = a; a = b; b = tmp; } }

#pragma option -G   /* fast, man.. fast !!! */

class ObjEnum
{
public:
    Object& Start( int iObjectType, int iStartAt = 0 );
    Object& Next();
    void PauseOne();

private:
    int iNumber, iMax;
    Object obj;
};

uchar islinedefinside( short object, short x1, short y1, short x2, short y2 )
/*-------------------------------------------------------------------------*
 * Check to see if part of linelist[object] is in the passed square.       *
 *-------------------------------------------------------------------------*/
{
    short lx1 = vlist[linelist[object].vfrom].x;
    short ly1 = vlist[linelist[object].vfrom].y;
    short lx2 = vlist[linelist[object].vend].x;
    short ly2 = vlist[linelist[object].vend].y;
    short i;

    /* is the line totally on one side of the box? */
    if( (lx2 > x2 && lx1 > x2) ||
        (lx2 < x1 && lx1 < x1) ||
        (ly2 > y2 && ly1 > y2) ||
        (ly2 < y1 && ly1 < y1) )
        return 0;

    if( lx1 >= x1 && lx1 <= x2 && ly1 >= y1 && ly1 <= y2 )
        return 1; /* the LineDef vfrom is entirely inside the square */
    if( lx2 >= x1 && lx2 <= x2 && ly2 >= y1 && ly2 <= y2 )
        return 1; /* the LineDef vend is entirely inside the square */
    if( (ly1 > y1) != (ly2 > y1) )
    {
        i = lx1 + (short) ( (long) (y1 - ly1) * (long) (lx2 - lx1) / (long) (ly2 - ly1));
        if( i >= x1 && i <= x2 )
            return 1; /* the LineDef crosses the y1 side (left) */
    }
    if( (ly1 > y2) != (ly2 > y2))
    {
        i = lx1 + (short) ( (long) (y2 - ly1) * (long) (lx2 - lx1) / (long) (ly2 - ly1));
        if( i >= x1 && i <= x2 )
            return 1; /* the LineDef crosses the y2 side (right) */
    }
    if( (lx1 > x1) != (lx2 > x1))
    {
        i = ly1 + (short) ( (long) (x1 - lx1) * (long) (ly2 - ly1) / (long) (lx2 - lx1));
        if( i >= y1 && i <= y2 )
            return 1; /* the LineDef crosses the x1 side (down) */
    }
    if( (lx1 > x2) != (lx2 > x2))
    {
        i = ly1 + (short) ( (long) (x2 - lx1) * (long) (ly2 - ly1) / (long) (lx2 - lx1));
        if( i >= y1 && i <= y2 )
            return 1; /* the LineDef crosses the x2 side (up) */
    }

    /* The linedef is not inside the square. */
    return 0;
}

short obj_getphyssector( short line_num, short side )
/*-------------------------------------------------------------------------*
 * Gets the sector number that line_num's side is facing.  Does not
 * use references.  Uses math.
 *-------------------------------------------------------------------------*/
{
   short x0, y0, dx0, dy0;
   short x1, y1, dx1, dy1;
   short x2, y2, dx2, dy2;
   short ld2, dist;
   short bestld, bestdist, bestmdist;
   LINEDEF *ldp = &linelist[line_num];

   /* get the coords for this line */
   x0  = vlist[ldp->vfrom].x;
   y0  = vlist[ldp->vfrom].y;
   dx0 = vlist[ldp->vend].x - x0;
   dy0 = vlist[ldp->vend].y - y0;

   /* find the normal vector for this LineDef */
   x1 = (dx0 + x0 + x0) / 2;
   y1 = (dy0 + y0 + y0) / 2;

   if( side == 0 )
   {
        dx1 = dy0;
        dy1 = -dx0;
   }
   else
   {
        dx1 = -dy0;
        dy1 = dx0;
   }

   bestld = -1;
   /* use a parallel to an axis instead of the normal vector (faster method) */
   if (abs( dy1) > abs( dx1))
   {
      if (dy1 > 0)
      {
	 /* get the nearest LineDef in that direction (increasing Y's: North) */
	 bestdist = 32767;
	 bestmdist = 32767;
     for (ld2 = 0; ld2 < nline; ld2++)
        if (ld2 != line_num && ((vlist[ linelist[ ld2].vfrom].x > x1) != (vlist[ linelist[ ld2].vend].x > x1)))
	    {
           x2  = vlist[ linelist[ ld2].vfrom].x;
           y2  = vlist[ linelist[ ld2].vfrom].y;
           dx2 = vlist[ linelist[ ld2].vend].x - x2;
           dy2 = vlist[ linelist[ ld2].vend].y - y2;
           dist = y2 + (short) ((long) (x1 - x2) * (long) dy2 / (long) dx2);
	       if (dist > y1 && (dist < bestdist || (dist == bestdist && (y2 + dy2 / 2) < bestmdist)))
	       {
		  bestld = ld2;
		  bestdist = dist;
		  bestmdist = y2 + dy2 / 2;
	       }
	    }
      }
      else
      {
	 /* get the nearest LineDef in that direction (decreasing Y's: South) */
	 bestdist = -32767;
	 bestmdist = -32767;
     for (ld2 = 0; ld2 < nline; ld2++)
        if (ld2 != line_num && ((vlist[ linelist[ ld2].vfrom].x > x1) != (vlist[ linelist[ ld2].vend].x > x1)))
	    {

           x2  = vlist[ linelist[ ld2].vfrom].x;
           y2  = vlist[ linelist[ ld2].vfrom].y;
           dx2 = vlist[ linelist[ ld2].vend].x - x2;
           dy2 = vlist[ linelist[ ld2].vend].y - y2;
           dist = y2 + (short) ((long) (x1 - x2) * (long) dy2 / (long) dx2);
	       if (dist < y1 && (dist > bestdist || (dist == bestdist && (y2 + dy2 / 2) > bestmdist)))
	       {
		  bestld = ld2;
		  bestdist = dist;
		  bestmdist = y2 + dy2 / 2;
	       }
	    }
      }
   }
   else
   {
      if (dx1 > 0)
      {
	 /* get the nearest LineDef in that direction (increasing X's: East) */
	 bestdist = 32767;
	 bestmdist = 32767;
     for (ld2 = 0; ld2 < nline; ld2++)
        if (ld2 != line_num && ((vlist[ linelist[ ld2].vfrom].y > y1) != (vlist[ linelist[ ld2].vend].y > y1)))
	    {
           x2  = vlist[ linelist[ ld2].vfrom].x;
           y2  = vlist[ linelist[ ld2].vfrom].y;
           dx2 = vlist[ linelist[ ld2].vend].x - x2;
           dy2 = vlist[ linelist[ ld2].vend].y - y2;
           dist = x2 + (short) ((long) (y1 - y2) * (long) dx2 / (long) dy2);
	       if (dist > x1 && (dist < bestdist || (dist == bestdist && (x2 + dx2 / 2) < bestmdist)))
	       {
		  bestld = ld2;
		  bestdist = dist;
		  bestmdist = x2 + dx2 / 2;
	       }
	    }
      }
      else
      {
	 /* get the nearest LineDef in that direction (decreasing X's: West) */
	 bestdist = -32767;
	 bestmdist = -32767;
     for (ld2 = 0; ld2 < nline; ld2++)
        if (ld2 != line_num && ((vlist[ linelist[ ld2].vfrom].y > y1) != (vlist[ linelist[ ld2].vend].y > y1)))
	    {
           x2  = vlist[ linelist[ ld2].vfrom].x;
           y2  = vlist[ linelist[ ld2].vfrom].y;
           dx2 = vlist[ linelist[ ld2].vend].x - x2;
           dy2 = vlist[ linelist[ ld2].vend].y - y2;
           dist = x2 + (short) ((long) (y1 - y2) * (long) dx2 / (long) dy2);
	       if (dist < x1 && (dist > bestdist || (dist == bestdist && (x2 + dx2 / 2) > bestmdist)))
	       {
		  bestld = ld2;
		  bestdist = dist;
		  bestmdist = x2 + dx2 / 2;
	       }
	    }
      }
   }

   /* no intersection: the LineDef was pointing outwards! */
   if (bestld < 0)
      return -1;

   /* now look if this LineDef has a SideDef bound to one sector */
   if (abs( dy1) > abs( dx1))
   {
      if ((vlist[ linelist[ bestld].vfrom].x < vlist[ linelist[ bestld].vend].x) == (dy1 > 0))
     x0 = linelist[ bestld].rsdef;
      else
     x0 = linelist[ bestld].lsdef;
   }
   else
   {
      if ((vlist[ linelist[ bestld].vfrom].y < vlist[ linelist[ bestld].vend].y) != (dx1 > 0))
     x0 = linelist[ bestld].rsdef;
      else
     x0 = linelist[ bestld].lsdef;
   }

   /* there is no SideDef on this side of the LineDef! */
   if (x0 < 0)
      return -1;

   /* OK, we got it -- return the Sector number */
   return sidelist[x0].sector;
}

#pragma option -G-

#define HTHICK  3

bool MatchLineWithTID( int iLine, int iTID )
{
    LINEDEF &Line = linelist[iLine];

    if( Line.special && Line.arg0 == iTID )
    {
        int i;
        short *pS;
        for( i = 0, pS = pgdCurGame->ActionsWithTID; i < 16;
             i++, pS++ )
        {
            if( pS[0] == Line.special )
                return TRUE;
            if( pS[0] == -1 )
                return FALSE;
        }
    }

    return FALSE;
}

bool LineUsesTID( int iLine )
{
    LINEDEF &Line = linelist[iLine];

    if( Line.special )
    {
        int i;
        short *pS;
        for( i = 0, pS = pgdCurGame->ActionsWithTID; i < 16;
             i++, pS++ )
        {
            if( pS[0] == Line.special )
                return TRUE;
        }
    }

    return FALSE;
}

void _MarkSectorLine( int iLine, int iSector, int iColor, bool fWatchOtherSide )
{
    int m;
    LINEDEF *ldp = &linelist[iLine];

    if( fWatchOtherSide )
    {
        m = ldp->rsdef;
        if( m != -1 && sidelist[m].sector == iSector )
        {
            m = ldp->lsdef;

            if( m == -1 || !obj_ismarked( sidelist[m].sector,
                    MODE_SECTOR ) )
            {
                obj_highlight( MODE_LINE, iLine, iColor );
            }
        }
        else if( m == -1 || !obj_ismarked( sidelist[m].sector,
                    MODE_SECTOR ) )
        {
            m = ldp->lsdef;
            if( m != -1 && sidelist[m].sector == iSector )
                obj_highlight( MODE_LINE, iLine, iColor );
        }
    }
    else
    {
        m = ldp->rsdef;
        if( m != -1 && sidelist[m].sector == iSector )
        {
            obj_highlight( MODE_LINE, iLine, iColor );
        }
        else
        {
            m = ldp->lsdef;
            if( m != -1 && sidelist[m].sector == iSector )
                obj_highlight( MODE_LINE, iLine, iColor );
        }
    }
}

short obj_highlight( short obj_type, short object, short color )
/*-------------------------------------------------------------------------*
 * Highlights obj_type[object] with color using XOR writes.                *
 *-------------------------------------------------------------------------*/
{
    short   sx1, sy1, sx2, sy2, r, i, m, cx, cy;
    LINEDEF *ldp;
    VERTEX  *v1, *v2;
    THING   *pThing;
    bool    fWatchOtherSide = FALSE;

    static  short dlsector = 0, drawnormal = 0, sh_notthisline = -1;

    cursor_put( CSR_ERASE );

    /* Set XOR mode */
    setwritemode( XOR_PUT );
    gr_setcolor( color );

    /* Depends on the kind of object ... */
    switch( obj_type )
    {
/* Vertex                                                                  */
    case MODE_VERTEX:
        /* Calculate screen coordinates */
        sx1 = _mapx2scr( vlist[object].x-xstart );
        sy1 = _mapy2scr( vlist[object].y-ystart );
        /* Draw/erase box around it (XOR) */
        clmou;
        setlinestyle( SOLID_LINE, 0, HTHICK );
        rectangle( sx1-2, sy1-2, sx1+2, sy1+2 );
        stmou;
        break;
/* Linedef                                                                 */
    case MODE_LINE:
        /* Get the coordinates */
        v1 = &vlist[linelist[object].vfrom];
        v2 = &vlist[linelist[object].vend];
        sx1 = _mapx2scr( v1->x-xstart );
        sy1 = _mapy2scr( v1->y-ystart );
        sx2 = _mapx2scr( v2->x-xstart );
        sy2 = _mapy2scr( v2->y-ystart );

        /* A thick line */
        clmou;
        setlinestyle( SOLID_LINE, 0, HTHICK );
        setcolor( color );
        line( sx1, sy1, sx2, sy2 );
        stmou;

        if( (editmode == MODE_LINE && !dlsector) || drawnormal )
        {
            cx = (sx1+sx2)/2;
            cy = (sy1+sy2)/2;
            clmou;
            setlinestyle( SOLID_LINE, 0, 1 );
            setcolor( color );
            line( cx, cy, cx + (sy1 - sy2) / 4, cy + (sx2 - sx1) / 4 );
            stmou;
        }

        if( editmode == MODE_LINE && _objhigh_dotagged && color != C_TAGGED )
        {
            int iSecTag = lSecTag( &linelist[object] );
            int iTID = linelist[object].arg0;

            if( iSecTag > 0 && lineneedstag( linelist[object].special ) )
            {
                /* draw all the tagged sectors (highlight them anyway) */
                for( m = 0; m < nsector; m++ )
                {
                    if( sectorlist[m].sectag == iSecTag )
                    {
                        /* don't highlight this line - screws up colors */
                        sh_notthisline = object;

                        ++dlsector;
                        obj_highlight( MODE_SECTOR, m, C_TAGGED );
                        --dlsector;

                        sh_notthisline = -1;
                    }
                }
            }
            else if( iTID && LineUsesTID( object ) )
            {
                for( i = 0, pThing = thinglist; i < nthing; i++, pThing++ )
                {
                    if( pThing->tid == iTID )
                        obj_highlight( MODE_THING, i, C_TAGGED );
                }
            }
        }

        break;
/* Thing                                                                   */
    case MODE_THING:
        /* Calculate screen coordinates */
        pThing = &thinglist[object];
        sx1 = _mapx2scr( pThing->x-xstart );
        sy1 = _mapy2scr( pThing->y-ystart );
        r = thing_radius( pThing->type );
        /* Draw/erase box around it (XOR) */
        clmou;
        gr_setwritemode( SWM_XOR );
        gr_setlinestyle( SLS_SOLID, HTHICK );
        gr_box( sx1-r-1, sy1-r-1, sx1+r+1, sy1+r+1, color );
        stmou;

        // check if it's got a TID, and ..
        if( pThing->tid && color != C_TAGGED )
        {
            for( ldp = linelist, i = 0; i < nline; i++, ldp++ )
            {
                if( ldp->special && MatchLineWithTID( i, pThing->tid ) )
                    obj_highlight( MODE_LINE, i, C_TAGGED );
            }
        }

        break;
/* Sector                                                                  */
    case MODE_SECTOR:
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( i == sh_notthisline )
                continue;

            _MarkSectorLine( i, object, color, FALSE );
        }
        if( _objhigh_dotagged && color != C_TAGGED && sectorlist[object].sectag > 0 )
        {
            short sectag = sectorlist[object].sectag;

            for( ldp = linelist, i = 0; i < nline; i++, ldp++ )
            {
                if( lSecTag( ldp ) == sectag && lineneedstag( ldp->special ) )
                {
                    /* if the line was highlighted with selection color (it's
                     * part of the highlighted sector), unhighlight it so
                     * it'll come out red when marked as the tagline */
                    _MarkSectorLine( i, object, color, FALSE );

                    drawnormal = 1;
                    obj_highlight( MODE_LINE, i, C_TAGGED );
                    drawnormal = 0;
                }
            }
        }
        break;
    }

    cursor_put( CSR_PUT );

    /* Reset to normal mode */
    gr_setwritemode( SWM_COPY );
    gr_setlinestyle( SLS_SOLID, 1 );

    return 1;
}

short obj_get( short obj_type, short x1, short y1, short x2, short y2,
               int *iDistance )
/*-------------------------------------------------------------------------*
 * Checks to see if there's something inside the passed box.               *
 * Returns the number of the object, or -1.                                *
 * obj_type is the requested type of the objects to check.                 *
 *-------------------------------------------------------------------------*/
{
    short       i, tmp, cur, n, m, rad;
    short       midx, midy, curx, dist = 32000, curdist, curitem = -1, lrad = 32000;
    short       lx1, ly1, lx2, ly2, xl, yl, curxl, curyl;
    THING       *t;
    VERTEX      *v, *v1, *v2;
    LINEDEF     *ldp;
    TInfo       *tip = pgdCurGame->thinginfo;
    short       *tt2idx = pgdCurGame->ttype2idx;

    if( y2 < y1 )
    {
        tmp = y2;
        y2 = y1;
        y1 = tmp;
    }
    if( x2 < x1 )
    {
        tmp = x2;
        x2 = x1;
        x1 = tmp;
    }

    midx = (x1 + x2) / 2;
    midy = (y1 + y2) / 2;

    xl = abs( x1 - midx );
    yl = abs( y1 - midy );

    /* Different checks for different object types */
    switch( obj_type )
    {
/* Check vertices                                                          */
    case MODE_VERTEX:
        for( i = 0, v = vlist; i < nvertex; i++, v++ )
        {
            //if( og_ignorethese && og_ignorethese[i] )
            //    continue;

            curxl = midx - v->x;
            curyl = midy - v->y;
            curxl = abs( curxl );
            curyl = abs( curyl );

            /* Check this vertex, maaaan */
            curdist = curxl + curyl;
            if( curdist < dist && curxl <= xl && curyl <= yl )
                curitem = i, dist = curdist;
        }
        if( iDistance )
            iDistance[0] = dist;
        return curitem;
/* Check linelist                                                          */
    case MODE_LINE:
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( og_ignorethese && og_ignorethese[i] )
                continue;

            /* Check this linedef.  Is it partially inside the box?
             * Code generously "donated" by Raphael Quinet/DEU. */
            if( islinedefinside( i, x1, y1, x2, y2 ) )
            {
                if( lc_inrange( i ) )
                    continue;

                v1 = &vlist[ldp->vfrom];
                v2 = &vlist[ldp->vend];
                xl = (v1->x + v2->x) / 2; /* x center of line */
                yl = (v1->y + v2->y) / 2; /* y center of line */
                curdist = abs( xl - midx ) + abs( yl - midy );
                if( curdist < dist )
                    curitem = i, dist = curdist;
            }
        }
        if( iDistance )
            iDistance[0] = dist;
        return curitem;
/* Check things                                                            */
    case MODE_THING:
        for( i = 0, t = thinglist; i < nthing; i++, t++ )
        {
            /* Check this thing. */
            curxl = midx - t->x;
            curyl = midy - t->y;
            curxl = abs( curxl );
            curyl = abs( curyl );

            rad = tt2idx[t->type];
            if( rad == -1 ) rad = 16;
            else rad = tip[rad].radius;

            curdist = curxl + curyl;
            if( ((curdist < dist) || (curdist == dist && rad < lrad)) &&
                curxl <= rad && curyl <= rad )
            {
                if( td_flags.use && !td_check( t->attribs ) )
                    continue;

                lrad = rad;
                curitem = i;
                dist = curdist;
            }
        }
        return curitem;
/* Case sectors                                                            */
    case MODE_SECTOR:
        /* Hack, hack...  I look for the first LineDef crossing a horizontal
         * half-line drawn from the cursor */
        curx = 32000;
        cur = -1;
        for( n = 0, ldp = linelist; n < nline; n++, ldp++ )
        {
            if( (vlist[ldp->vfrom].y > midy) != (vlist[ldp->vend].y > midy) )
            {
                lx1 = vlist[ldp->vfrom].x;
                ly1 = vlist[ldp->vfrom].y;
                lx2 = vlist[ldp->vend].x;
                ly2 = vlist[ldp->vend].y;

                if( ly1 == midy || ly2 == midy )
                    m = lx1 + (short) ((long) ((midy+1) - ly1) * (long) (lx2 - lx1) / (long) (ly2 - ly1));
                else
                    m = lx1 + (short) ((long) (midy - ly1) * (long) (lx2 - lx1) / (long) (ly2 - ly1));


                // m = lx1 + (midy - ly1) * (lx2 - lx1) / (ly2 - ly1);


                if( m >= midx && m < curx )
                {
                    curx = m;
                    cur = n;
                }
            }
        }

        /* now look if this LineDef has a SideDef bound to one sector */
        if( cur >= 0 )
        {
            if( vlist[linelist[cur].vfrom].y > vlist[linelist[cur].vend].y )
                cur = linelist[cur].rsdef;
            else
                cur = linelist[cur].lsdef;
            if( cur >= 0 )
                cur = sidelist[cur].sector;
            else
                cur = -1;
        }
        else
            cur = -1;
        return( cur );
    }

    return -1;
}

static void     *maniplist;
static short    *rdlinelist;
static short    inmanip;
static short    nmanip;
static short    cursormapx, cursormapy, manip_cx, manip_cy;
static short    manip_objtype;
static double   manip_angle, manip_scale;
static short    manip_refx, manip_refy;
static short    manip_relative;
static uchar    *m_marklist;
static short    mlastx, mlasty;

short           manip_addx, manip_addy;
short           manip_startx, manip_starty;
short           manip_copy;
short           m_showlength = GS_OFF;
short           len_num;
short           len_colors[32];
short           len_lengths[32];

#define obj_ismanip(n)      m_marklist[n]

short obj_manipupd( short x, short y )
/*-------------------------------------------------------------------------*
 * Determines if we need to update the manip list display.                 *
 *-------------------------------------------------------------------------*/
{
    return( mlastx != x || mlasty != y );
}

short obj_initmanip( int obj_type, short coordx, short coordy, Object& objCursor,
                     short copy, short relative )
/*-------------------------------------------------------------------------*
 * Initializes dragging operations.                                        *
 * The way dragging works is pretty simple.  This initialization function  *
 * goes through the entire marklist of objects and creates a new "drag     *
 * list" of objects based on which objects are selected (that is, it       *
 * creates another structure for each marked object and copies the         *
 * original data into that object.)  THIS list of objects is the one       *
 * that's changed and drawn as the user "drags" the cursor around.  When   *
 * the user confirms the new location of the objects, the new locations    *
 * are copied back into the original structure and the temporary list      *
 * deleted from memory.                                                    *
 *                                                                         *
 * If copy is one, the object(s) get tacked on to the end of the list      *
 * instead of being "put back in."                                         *
 *-------------------------------------------------------------------------*/
{
    short     i, n, i2, m;
    short     rdpos;
    VERTEX  *v;
    THING   *t;
    LINEDEF *ldp;
    short     refidx, refdist = 32000, dist;
    bool    fUnmarkObj = FALSE;
    size_t  objsize;

    manip_copy = copy;
    manip_objtype = obj_type;
    manip_relative = relative;
    _memstr = "m_marklist buffer";
    m_marklist = (uchar*) memget( MAXOBJECT );

    cursormapx = xstart + _scrx2map( coordx );
    cursormapy = ystart + _scry2map( coordy );

    /* Mark csr_object */
    if( objCursor.iNumber != -1 && !obj_ismarked( objCursor ) )
        obj_mark( objCursor ), fUnmarkObj = TRUE;

    memset( m_marklist, 0, MAXOBJECT );

    if( manip_objtype != MODE_THING && manip_objtype != MODE_VERTEX )
    {
        /* Mark all the vertices .. */
        switch( manip_objtype )
        {
        case MODE_LINE:
            // copy over marked vertices ..
            memcpy( m_marklist, marklist, MAXOBJECT );

            // now copy over marked lines
            for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
            {
                if( obj_ismarked( i, MODE_LINE ) )
                {
                    m_marklist[ldp->vfrom] = 1;
                    m_marklist[ldp->vend] = 1;
                }
            }
            break;
        case MODE_SECTOR:
            for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
            {
                m = ldp->rsdef;
                if( m >= 0 && obj_ismarked( sidelist[m].sector ) )
                {
                    m_marklist[ldp->vfrom] = 1;
                    m_marklist[ldp->vend] = 1;
                }
                m = ldp->lsdef;
                if( m >= 0 && obj_ismarked( sidelist[m].sector ) )
                {
                    m_marklist[ldp->vfrom] = 1;
                    m_marklist[ldp->vend] = 1;
                }
            }
            break;
        }
    }
    else
        memcpy( m_marklist, marklist, MAXOBJECT );

    /* Count number of marked objects */
    for( i = 0, nmanip = 0; i < MAXOBJECT; i++ )
        if( obj_ismanip( i ) )
            ++nmanip;

    if( objCursor.iNumber != -1 && objCursor.iType == MODE_VERTEX && nmanip == 1 )
        _vertex_align = 1;

    _va_from = _va_to = -1;

    /* No selected objects? */
    if( !nmanip )
        return 0;

    if( manip_objtype == MODE_THING )       objsize = sizeof( THING );
    else                                    objsize = sizeof( VERTEX );

    /* Allocate some memory for the drag list */
    if( (maniplist = malloc( nmanip * objsize )) == NULL )
    {
_nomem:
        message_box( MB_OK, "Notice",
                     "There is not enough free memory to perform this operation." );
        return 0;
    }

    rdlinelist = NULL;

    /* Copy the marked vertices into the drag list */
    if( manip_objtype != MODE_THING )
    {
        /* Set up the lines to ignore: all those affected by the manipulation. */
        _memstr = "ignorethese buffer";
        og_ignorethese = (uchar*) memget( MAXOBJECT );
        memset( og_ignorethese, 0, MAXOBJECT );

        if( manip_relative )
        {
            /* Find the marked vertex closest to the cursor */
            for( i = 0; i < MAXOBJECT; i++ )
            {
                if( obj_ismanip( i ) )
                {
                    dist = abs( vlist[i].x-cursormapx ) + abs( vlist[i].y-cursormapy );
                    if( dist < refdist )
                    {
                        refidx = i;
                        refdist = dist;
                    }
                }
            }

            manip_refx = manip_startx = vlist[refidx].x;
            manip_refy = manip_starty = vlist[refidx].y;
        }

        _memstr = "redraw-line list";
        rdlinelist = (short*) memget( nline * (sizeof( short ) * 2) );

        if( rdlinelist == NULL )
        {
            free( maniplist );
            goto _nomem;
        }

        /* Init the line list */
        for( i = 0; i < nline * 2; i++ )
            rdlinelist[i] = MAXOBJECT;

        for( i = 0, n = 1; i < MAXOBJECT; i++ )
        {
            if( obj_ismanip( i ) )
            {
                /* Find the line that uses this vertex */
                for( i2 = 0, ldp = linelist, rdpos = 0; i2 < nline; i2++, rdpos += 2, ldp++ )
                {

/* Man, I can't believe how kludged this is.  Oh well, it hardly matters.
 * The thing works :).  The way this works is: When a line is found that uses
 * a "dragged" vertex, the "redraw line list" array at the line's position
 * is updated with the NEGATIVE value of the marked vertex's place in the
 * maniplist array (-n).
 * Then, its sibling vertex (from or to) *in the RDL list* is checked for an
 * uninitialized value and, if true, is updated with the value of the
 * "real" corresponding vertex in the line we're checking.
 */
                    if( ldp->vfrom == i )
                    {
                        rdlinelist[rdpos] = -n;
                        if( rdlinelist[rdpos+1] == MAXOBJECT )
                            rdlinelist[rdpos+1] = ldp->vend;
                        _va_to = ldp->vend;
                        og_ignorethese[i2] = 1;
                    }
                    if( ldp->vend == i )
                    {
                        rdlinelist[rdpos+1] = -n;
                        if( rdlinelist[rdpos] == MAXOBJECT )
                            rdlinelist[rdpos] = ldp->vfrom;
                        _va_from = ldp->vfrom;
                        og_ignorethese[i2] = 1;
                    }
                }

                v = &((VERTEX *) maniplist)[n-1];
                _va_addr = v;
                ++n;
                memcpy( v, &vlist[i], sizeof( VERTEX ) );
                if( manip_relative )
                {
                    /* Change coordinates to cursor offsets */
                    v->x -= manip_refx;
                    v->y -= manip_refy;
                }
            }
        }
    }
    else
    {
        if( manip_relative )
        {
            /* Find the marked thing closest to the cursor */
            for( i = 0; i < MAXOBJECT; i++ )
            {
                if( obj_ismanip( i ) )
                {
                    dist = abs( thinglist[i].x-cursormapx ) +
                           abs( thinglist[i].y-cursormapy );
                    if( dist < refdist )
                    {
                        refidx = i;
                        refdist = dist;
                    }
                }
            }

            manip_refx = manip_startx = thinglist[refidx].x;
            manip_refy = manip_starty = thinglist[refidx].y;
        }

        for( i = 0, n = 0; i < MAXOBJECT; i++ )
        {
            if( obj_ismanip( i ) )
            {
                t = &((THING *) maniplist)[n++];
                memcpy( t, &thinglist[i], sizeof( THING ) );
                if( manip_relative )
                {
                    /* Change coordinates to cursor offsets */
                    t->x -= manip_refx;
                    t->y -= manip_refy;
                }
            }
        }
    }

    manip_refx -= cursormapx;
    manip_refy -= cursormapy;

    obj_findcenter( &manip_cx, &manip_cy );

    if( _va_from == -1 || _va_to == -1 )
        _vertex_align = 0;

    if( obj_type != MODE_THING )
        m_showlength = GS_FIRST;

    inmanip = TRUE;

    /* Mark csr_object */
    if( fUnmarkObj )
        obj_unmark( objCursor );

    return 1;
}

void lineSplitandCombine( uchar *checklist, short ncheck )
/*-------------------------------------------------------------------------*
 * Splits lines with vertices in "checklist" and combines the resultant
 * lines, if any.
 *-------------------------------------------------------------------------*/
{
    short   firstjoin = 1, i, NewVertex, tmpint;
    short   *llist = (short*) memget( MAXOBJECT * sizeof( *llist ) ), *llp = llist;
    short   llcount = 0;

    /* if we're creating lines then yes, join vertexes. */
    if( lc_inprogress() || !inmanip )
        firstjoin = 0;

    /* Save the new locations of the objects */
    if( tog_join || !firstjoin )
    {
        for( i = 0; i < ncheck; i++ )
        {
            if( checklist[i] )
            {
                vertex_createjoin( i, &NewVertex );

                if( _vcj_line != -1 )   /* it created a line */
                {
                    *llp++ = _vcj_line;
                    ++llcount;
                }

                if( _vcj_line2 != -1 )   /* it created a line */
                {
                    *llp++ = _vcj_line2;
                    ++llcount;
                }

                if( NewVertex != i )
                {
                    if( config.tog_combineexact &&
                        vlist[NewVertex].x == vlist[i].x &&
                        vlist[NewVertex].y == vlist[i].y )
                    {
                        vertex_replace( i, NewVertex );
                        continue;
                    }

                    if( firstjoin )
                    {
                        tmpint = message_box( MB_YESNO, st_dcksays,
                                   "Join close vertices?             " );
                        if( tmpint == MB_NO )
                            break;
                        firstjoin = 0;
                    }

                    /* All lines that DID use this one must now use
                     * the new one. */
                    vertex_replace( i, NewVertex );
                }
            }
        }

        /* try to join lines.  og_ignorethese[] is an array of
         * lines to be ignored for "getting", but it's also a useful
         * indicator of which lines are affected.  here, we build
         * a list of these lines and pass it to the combine lines
         * function, which is ONLY useful from this exact point. */
        if( og_ignorethese )
        {
            for( i = 0; i < MAXOBJECT; i++ )
            {
                if( og_ignorethese[i] )
                    *llp++ = i, ++llcount;
            }
        }

        line_combinesame( llist, llcount );
    }

    memfree( llist );
}

void obj_exitmanip( short save )
/*-------------------------------------------------------------------------*
 * Exits the drag function.                                                *
 * 1) if( save ), saves the new locations of the objects.                  *
 * 2) deletes the drag list                                                *
 *-------------------------------------------------------------------------*/
{
    short   i, n, tmp;
    VERTEX  *v;
    THING   *t;

    _vertex_align = 0;

    bool fSimple = isctrl() ? FALSE : TRUE;

    if( nmanip == 1 )
    {
        _mapsnap( &manip_addx, &manip_addy, cursormapx, cursormapy, fSimple );
    }
    else
    {
        _mapsnap( &manip_addx, &manip_addy, cursormapx+manip_refx,
            cursormapy+manip_refy, fSimple );
    }

    if( save )
    {
        if( !lc_inprogress() )
            undo_init();

        /* Save the new locations of the objects */
        for( i = 0, n = 0; i < MAXOBJECT; i++ )
        {
            if( obj_ismanip( i ) )
            {
                /* Copy in vertices */
                if( manip_objtype != MODE_THING )
                {
                    v = &((VERTEX *) maniplist)[n++];
                    if( manip_relative )
                    {
                        /* Re-create proper coordinates */
                        v->x += manip_addx;
                        v->y += manip_addy;
                    }
                    else if( manip_angle != -1 )
                    {
                        rs_coords( &(v->x), &(v->y), v->x-manip_cx, v->y-manip_cy,
                                manip_angle, manip_scale );
                        /* Because rotation/scaling operations are
                         * based on an central origin (ie: each entry in the
                         * move list is relative to the center of the "moved"
                         * objects), we use the manip_c# variables to return
                         * them to an absolute position instead of the
                         * manip_add.. variables. */
                        v->x += manip_cx;
                        v->y += manip_cy;
                    }
                    memcpy( &vlist[i], v, sizeof( VERTEX ) );
                }
                else
                {
                    t = &((THING *) maniplist)[n++];
                    if( manip_relative )
                    {
                        /* Re-create proper coordinates */
                        t->x += manip_addx;
                        t->y += manip_addy;
                    }
                    else if( manip_angle != -1 )
                    {
                        rs_coords( &(t->x), &(t->y), t->x-manip_cx, t->y-manip_cy,
                                   manip_angle, manip_scale );
                        t->x += manip_cx;
                        t->y += manip_cy;
                    }
                    if( !manip_copy )
                    {
                        memcpy( &thinglist[i], t, sizeof( THING ) );
                        thing_autosetangle( i );
                    }
                    else
                    {
                        /* Copy them instead by tacking them on to the
                         * end of the list.
                         */
                        tmp = obj_insert( MODE_THING, -1, 0, 0 );
                        memcpy( &thinglist[tmp], t, sizeof( THING ) );
                        thing_autosetangle( tmp );
                    }
                }
            }
        }

        nMadeChanges++;

        if( manip_objtype != MODE_THING && !lc_inprogress() )
        {
            nMapChanges++;

            /* combine dragged vertices with existing lines.. */
            lineSplitandCombine( m_marklist, MAXOBJECT );

            map_purge( MODE_ALL );
        }

        if( !lc_inprogress() )
            undo_setvars();
    }

    m_showlength = GS_OFF;

    /* Delete the mark list and other variables */
    if( og_ignorethese )
    {
        memfree( og_ignorethese );
        og_ignorethese = NULL;
    }
    memfree( m_marklist );
    memfree( maniplist );

    if( rdlinelist )        /* For non-thing moving */
        memfree( rdlinelist );

    inmanip = 0;
    maniplist = NULL;
}

void obj_findcenter( short *x, short *y )
/*-------------------------------------------------------------------------*
 * Finds the centerpoint of marked objects.                                *
 *-------------------------------------------------------------------------*/
{
    short   n, minx = 32000, maxx = -32000,
            miny = 32000, maxy = -32000;
    THING   *t;
    VERTEX  *v;

/*-------------------------------------------------------------------------*
 * Things                                                                  *
 *-------------------------------------------------------------------------*/
    if( manip_objtype == MODE_THING )
    {
        n = 0;
        while( n < nmanip )
        {
            t = &((THING *)maniplist)[n++];
            if( t->x < minx )   minx = t->x;
            if( t->x > maxx )   maxx = t->x;
            if( t->y < miny )   miny = t->y;
            if( t->y > maxy )   maxy = t->y;
        }
    }
/*-------------------------------------------------------------------------*
 * Other things (vertices/linedefs/etc)                                    *
 *-------------------------------------------------------------------------*/
    else
    {
        n = 0;
        while( n < nmanip )
        {
            v = &((VERTEX *)maniplist)[n++];
            if( v->x < minx )   minx = v->x;
            if( v->x > maxx )   maxx = v->x;
            if( v->y < miny )   miny = v->y;
            if( v->y > maxy )   maxy = v->y;
        }
    }

    *x = minx + (maxx-minx) / 2;
    *y = miny + (maxy-miny) / 2;
}

void obj_center( short editmode, short obj_num, short *x, short *y )
/*-------------------------------------------------------------------------*
 * Gets the center of an object and returns it.                            *
 *-------------------------------------------------------------------------*/
{
    short     minx, miny, maxx, maxy, i;
    VERTEX  *v1, *v2;
    LINEDEF *ldp;

    minx = miny = 32000;
    maxx = maxy = -32000;

    switch( editmode )
    {
    case MODE_VERTEX:
        *x = vlist[obj_num].x;
        *y = vlist[obj_num].y;
        return;
    case MODE_THING:
        *x = thinglist[obj_num].x;
        *y = thinglist[obj_num].y;
        return;
    case MODE_LINE:
        *x = (vlist[linelist[obj_num].vfrom].x + vlist[linelist[obj_num].vend].x) / 2;
        *y = (vlist[linelist[obj_num].vfrom].y + vlist[linelist[obj_num].vend].y) / 2;
        break;
    case MODE_SECTOR:
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( (ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == obj_num) ||
                (ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == obj_num) )
            {
                v1 = &vlist[ldp->vfrom];
                v2 = &vlist[ldp->vend];
                if( v1->x < minx )  minx = v1->x;
                if( v2->x < minx )  minx = v2->x;
                if( v1->y < miny )  miny = v1->y;
                if( v2->y < miny )  miny = v2->y;
                if( v1->x > maxx )  maxx = v1->x;
                if( v2->x > maxx )  maxx = v2->x;
                if( v1->y > maxy )  maxy = v1->y;
                if( v2->y > maxy )  maxy = v2->y;
            }
        }
        *x = (minx + maxx) / 2;
        *y = (miny + maxy) / 2;
        return;
    }
}

#define LENMAX  12

struct
{
    int x1, y1, x2, y2;

} ShowLineLen[LENMAX];

static int iFont6x10 = -1;
extern int dckfont;

class LineLengthBox
{
public:
    LineLengthBox()
    {
        //if( iFont6x10 == -1 )
        //    iFont6x10 = installuserfont( "6x10.fnt" );
        pImageSave = new char[imagesize( 0, 0, iWidth = GetFontWidth()/10*5+4,
            iHeight = GetFontHeight()/10+2 )];
        fDrawn = FALSE;
    }
    ~LineLengthBox()
    {
        delete pImageSave;
    }

    void DrawForLine( int x1, int y1, int x2, int y2 );

    void Erase()
    {
        if( fDrawn )
            putimage( iLeft, iTop, pImageSave, COPY_PUT );
        fDrawn = FALSE;
    }

private:
    char*   pImageSave;
    int     iLeft, iTop;
    int     iWidth, iHeight;
    bool    fDrawn;

};

void LineLengthBox::DrawForLine( int x1, int y1, int x2, int y2 )
{
    //settextstyle( iFont6x10, HORIZ_DIR, 1 );

    int sx1 = _mapx2scr( x1 - xstart );
    int sx2 = _mapx2scr( x2 - xstart );
    int sy1 = _mapy2scr( y1 - ystart );
    int sy2 = _mapy2scr( y2 - ystart );

    if( line_len( sx1, sy1, sx2, sy2 ) < 40 )
    {
        fDrawn = FALSE;
        return;
    }

    iLeft = (sx1+sx2)/2 - iWidth / 2;
    iTop =  (sy1+sy2)/2 - iHeight / 2;

    getimage( iLeft, iTop, iLeft + iWidth, iTop + iHeight, pImageSave );
    setfillstyle( SOLID_FILL, C_WHITE );
    bar( iLeft, iTop, iLeft + iWidth, iTop + iHeight );
    int iLength = line_len( x1, y1, x2, y2 );
    char buf[10];
    setcolor( C_BLACK );
    outtextxy( iLeft + 2, iTop + 1, itoa( iLength, buf, 10 ) );
    //settextstyle( dckfont, HORIZ_DIR, 1 );

    fDrawn = TRUE;
}

static LineLengthBox *pLineLenBoxes[LENMAX];

void obj_drawmaniplist( short coordx, short coordy, double angle, double scale )
/*-------------------------------------------------------------------------*
 * if( update_coords ), get the new cursor coordinates into the globals    *
 * and redraw the list.  Otherwise, just use the old coordinates.          *
 *-------------------------------------------------------------------------*/
{
    short   i, sx1, sy1, r;
    uchar   v1a, v2a;
    short   x1, x2, y1, y2;
    VERTEX  *v1, *v2;
    THING   *t;
    short   *rdptr = rdlinelist;
    short   max, may;

    cursor_put( CSR_ERASE );

    /* Set the viewport */
    SetViewPort( mxpstart, mypstart, mxpstart+mxplen, mypstart+myplen );

    if( m_showlength != GS_OFF && !pLineLenBoxes[0] )
    {
        // create line length boxes
        for( i = 0; i < LENMAX; i++ )
            pLineLenBoxes[i] = new LineLengthBox;
    }

    /* Like I said. */
    if( manip_relative && coordx != -1 )
    {
        mlastx = coordx;
        mlasty = coordy;

        cursormapx = xstart + _scrx2map( coordx );
        cursormapy = ystart + _scry2map( coordy );

        /* Check: If we're dragging, and it's vertex mode, and there
         * is only one vertex marked, and the alt key is held down,
         * slide it along its neighboring vertices (this is all
         * stored in _vertex_align and _va_ld1, and _va_ld2 */
        if( _vertex_align && isalt() )
        {
            find_slope_point( &(_va_addr->x), &(_va_addr->y),
                              cursormapx, cursormapy,
                              &vlist[_va_from], &vlist[_va_to] );
            _va_addr->x -= cursormapx;
            _va_addr->y -= cursormapy;
        }
    }

    if( coordx == -1 &&
        (m_showlength == GS_ON || m_showlength == GS_LAST) )
    {
        // erase old boxes
        for( i = len_num-1; i >= 0; i-- )
            pLineLenBoxes[i]->Erase();
    }

    if( m_showlength == GS_FIRST )
        m_showlength = GS_ON;
    if( m_showlength == GS_LAST )
        m_showlength = GS_OFF;

    len_num = 0;

    if( manip_relative )
    {
        static bool fSimple;

        if( coordx != -1 )
        {
            // not redrawing, so take notice of this
            fSimple = isctrl() ? FALSE : TRUE;
        }

        if( nmanip == 1 )
            _mapsnap( &manip_addx, &manip_addy, cursormapx, cursormapy, fSimple );
        else
            _mapsnap( &manip_addx, &manip_addy, cursormapx+manip_refx, cursormapy+manip_refy, fSimple );
    }
    else
    {
        manip_angle = angle;
        manip_scale = scale;

        manip_addx = 0;
        manip_addy = 0;
    }

    max = manip_addx;
    may = manip_addy;

    if( manip_objtype != MODE_THING )
    {
        /* Update the lines */
        setcolor( C_DRAG );
        gr_setlinestyle( SLS_SOLID, 1 );
        gr_setwritemode( SWM_XOR );

        for( i = 0; i < nline; i++, rdptr += 2 )
        {
            /* Find a "redraw line" with an initialized value.  These are
             * the only ones we draw.  MAXOBJECT is used as an "uninitialized"
             * value. */
            if( rdptr[0] != MAXOBJECT )
            {
                /* Uses a "dragged" vertex, so get its pointer from the
                 * "dragged" vertex list. */
                if( rdptr[0] < 0 )
                {
                    v1a = 1;        /* tell it to make the vertex absolute */
                    v1 = &((VERTEX *)maniplist)[(-rdptr[0])-1];
                }
                /* Uses a "real" vertex, so get its pointer from the "real"
                 * vertex list. */
                else
                {
                    v1a = 0;        /* "real" vertices are absolute */
                    v1 = &vlist[rdptr[0]];
                }

                if( rdptr[1] < 0 )
                {
                    v2a = 1;
                    v2 = &((VERTEX *)maniplist)[(-rdptr[1])-1];
                }
                else
                {
                    v2a = 0;
                    v2 = &vlist[rdptr[1]];
                }

                /* The flags v1a and v2a signify the type of each
                 * vertex.  When 0, it means it's a relative (dragged)
                 * vertex, and therefore must be added to the coordinates
                 * of the cursor before drawing it.  These four cases
                 * are done for speed, not good-looking code :) */

                if( !v1a && !v2a )
                {
                    /* This really should never happen */
                    x1 = v1->x; x2 = v2->x;
                    y1 = v1->y; y2 = v2->y;
                }
                else if( v1a && !v2a )
                {
                    x1 = v1->x+max;
                    y1 = v1->y+may;
                    x2 = v2->x;
                    y2 = v2->y;
                }
                else if( v1a && v2a )
                {
                    x1 = v1->x+max;
                    y1 = v1->y+may;
                    x2 = v2->x+max;
                    y2 = v2->y+may;
                }
                else if( !v1a && v2a )
                {
                    x1 = v1->x;
                    y1 = v1->y;
                    x2 = v2->x+max;
                    y2 = v2->y+may;
                }

                if( angle != -1 )
                {
                    if( v1a )
                    {
                        rs_coords( &x1, &y1, x1-manip_cx, y1-manip_cy,
                                angle, scale );
                        x1 += manip_cx;
                        y1 += manip_cy;
                    }
                    if( v2a )
                    {
                        rs_coords( &x2, &y2, x2-manip_cx, y2-manip_cy,
                                angle, scale );
                        x2 += manip_cx;
                        y2 += manip_cy;
                    }
                }

                sc_line( x1, y1, x2, y2 );

                if( m_showlength != GS_OFF && !(v1a && v2a) && len_num < LENMAX )
                {
                    ShowLineLen[len_num].x1 = x1;
                    ShowLineLen[len_num].y1 = y1;
                    ShowLineLen[len_num].x2 = x2;
                    ShowLineLen[len_num].y2 = y2;
                    ++len_num;
                }
            }
        }
    }
    else if( manip_objtype == MODE_THING )
    {
        gr_setlinestyle( SLS_SOLID, 1 );
        gr_setwritemode( SWM_XOR );

        for( i = 0, t = (THING *) maniplist; i < nmanip; i++, t++ )
        {
            /* Check this one .. */
            r = thing_radius( t->type );

            /* Redraw it */
            x1 = max + t->x;
            y1 = may + t->y;

            if( angle != -1 )
            {
                rs_coords( &x1, &y1, x1-manip_cx, y1-manip_cy, angle, scale );
                x1 += manip_cx;
                y1 += manip_cy;
            }

            sx1 = _mapx2scr( x1 - xstart );
            sy1 = _mapy2scr( y1 - ystart );

            /* Draw the thing */
            gr_setcolor( thing_color( t->type ) );
            rectangle( sx1-r, sy1-r, sx1+r, sy1+r );
        }
    }

    if( manip_relative )
        sc_plotx( max, may, C_LIGHTGREEN );

    if( m_showlength == GS_ON && coordx != -1 )
    {
        // draw new boxes
        for( i = 0; i < len_num; i++ )
            pLineLenBoxes[i]->DrawForLine( ShowLineLen[i].x1, ShowLineLen[i].y1,
                ShowLineLen[i].x2, ShowLineLen[i].y2 );
    }

    gr_setlinestyle( SLS_SOLID, 1 );
    gr_setwritemode( SWM_COPY );

    cursor_put( CSR_PUT );
}

byte linemarklist[MAXOBJECT];

void obj_clearmarks( short csr_object, short update )
/*-------------------------------------------------------------------------*
 * Clears all marked objects and updates the screen if update is 1.        *
 *-------------------------------------------------------------------------*/
{
    short     i;

    if( !update )
    {
        memset( marklist, 0, MAXOBJECT );
        memset( linemarklist, 0, MAXOBJECT );
        return;
    }

    /* I'm going to do this in reverse order because it'll leave less
     * garbage than forward order, most of the time. */
    cursor_put( CSR_ERASE );

    if( editmode != MODE_LINE )
    {
        for( i = MAXOBJECT-1; i >= 0; i-- )
        {
            if( obj_ismarked( i ) )
            {
                if( csr_object != i )
                    obj_highlight( editmode, i, COLOR_MARKED );
                obj_unmark( i );
            }
        }
    }
    else
    {
        for( i = MAXOBJECT-1; i >= 0; i-- )
        {
            if( obj_ismarked( i, MODE_LINE ) )
            {
                if( csr_object != i )
                    obj_highlight( MODE_LINE, i, COLOR_MARKED );
                obj_unmark( i );
            }
            if( obj_ismarked( i, MODE_VERTEX ) )
            {
                if( csr_object != i )
                    obj_highlight( MODE_VERTEX, i, COLOR_MARKED );
                obj_unmark( i, MODE_VERTEX );
            }
        }
    }
    cursor_put( CSR_PUT );
}

short obj_nmarked( int iObjectType )
/*-------------------------------------------------------------------------*
 * Returns the number of marked objects                                    *
 *-------------------------------------------------------------------------*/
{
    short   i = MAXOBJECT, n = 0;
    uchar   *p = marklist;

    if( iObjectType == -1 )
        iObjectType = editmode;

    if( iObjectType == MODE_LINE )
        p = linemarklist;

    while( i-- )
        if( *p++ )
            ++n;

    return n;
}

void obj_mark( Object& obj )
{
    if( obj.iType != MODE_LINE )
    {
        marklist[obj.iNumber] = 1;
    }
    else
    {
        linemarklist[obj.iNumber] = 1;
    }
}

void obj_unmark( Object& obj )
{
    if( obj.iType != MODE_LINE )
    {
        marklist[obj.iNumber] = 0;
    }
    else
    {
        linemarklist[obj.iNumber] = 0;
    }
}

bool obj_ismarked( Object& obj )
{
    if( obj.iType != MODE_LINE )
    {
        return marklist[obj.iNumber];
    }
    else
    {
        return linemarklist[obj.iNumber];
    }
}

void obj_shiftmarksdown( Object& obj )
{
    if( obj.iType != MODE_LINE )
    {
        memcpy( marklist+obj.iNumber, marklist+obj.iNumber+1,
            MAXOBJECT-obj.iNumber );
        marklist[MAXOBJECT-1] = 0;
    }
    else
    {
        memcpy( linemarklist+obj.iNumber, linemarklist+obj.iNumber+1,
            MAXOBJECT-obj.iNumber );
        linemarklist[MAXOBJECT-1] = 0;
    }
}

/*-------------------------------------------------------------------------*
 * Like I care if you see this macro.                                      *
 *-------------------------------------------------------------------------*/

#define mysectormarkmacro( m, sdp )     \
            if( m >= 0 && (sdp)->sector >= 0 )  { \
                if( mode == MBOX_MARK && marklist[sdp->sector] == 1 ) \
                    obj_unmark( sdp->sector );  \
                else if( mode == MBOX_UNMARK && marklist[sdp->sector] == 2 ) \
                    marklist[sdp->sector] = 1;  }

short obj_markbox( short x1, short y1, short x2, short y2, short mode )
/*-------------------------------------------------------------------------*
 * Marks all objects in a box.                                             *
 * Returns the number of objects marked.                                   *
 *-------------------------------------------------------------------------*/
{
    short     i, tmp, nobj = 0, m;
    LINEDEF *ldp;
    SIDEDEFM    *sdp;

    /* Fix coordinates if we need to */
    if( y1 > y2 )
    {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    if( x1 > x2 )
    {
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }

    cursor_put( CSR_ERASE );

    switch( editmode )
    {
    case MODE_LINE:
        /* Mark the lines.  Both their vertices must be in the box. */
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            if( inrange( vlist[ldp->vfrom].x, x1, x2 ) &&
                inrange( vlist[ldp->vend].x, x1, x2 ) &&
                inrange( vlist[ldp->vfrom].y, y1, y2 ) &&
                inrange( vlist[ldp->vend].y, y1, y2 ) )
            {
                if( mode == MBOX_MARK )
                {
                    nobj ++;
                    if( !obj_ismarked( i ) )
                    {
                        obj_highlight( MODE_LINE, i, COLOR_MARKED );
                        obj_mark( i );
                    }
                }
                else if( mode == MBOX_UNMARK && obj_ismarked( i ) )
                {
                    obj_highlight( MODE_LINE, i, COLOR_MARKED );
                    obj_unmark( i );
                    nobj ++;
                }
            }
        }

        /* Mark the vertices cuz I love god */
        for( i = 0; i < nvertex; i++ )
        {
            if( inrange( vlist[i].x, x1, x2 ) &&
                inrange( vlist[i].y, y1, y2 ) &&
                !vtx_isdeleted( vlist[i] ) )
            {
                if( mode == MBOX_MARK )
                {
                    nobj ++;
                    if( !obj_ismarked( i, MODE_VERTEX ) )
                    {
                        obj_highlight( MODE_VERTEX, i, COLOR_MARKED );
                        obj_mark( i, MODE_VERTEX );
                    }
                }
                else if( mode == MBOX_UNMARK && obj_ismarked( i, MODE_VERTEX ) )
                {
                    obj_highlight( MODE_VERTEX, i, COLOR_MARKED );
                    obj_unmark( i, MODE_VERTEX );
                    nobj ++;
                }
            }
        }

        break;
    case MODE_THING:
        /* Mark the things. */
        for( i = 0; i < nthing; i++ )
        {
            if( inrange( thinglist[i].x, x1, x2 ) &&
                inrange( thinglist[i].y, y1, y2 ) )
            {
                if( td_flags.use && !td_check( thinglist[i].attribs ) )
                    continue;

                if( mode == MBOX_MARK )
                {
                    nobj ++;
                    if( !obj_ismarked( i ) )
                    {
                        obj_highlight( MODE_THING, i, COLOR_MARKED );
                        obj_mark( i );
                    }
                }
                else if( mode == MBOX_UNMARK && obj_ismarked( i ) )
                {
                    obj_highlight( MODE_THING, i, COLOR_MARKED );
                    obj_unmark( i );
                    nobj ++;
                }
            }
        }
        break;
    case MODE_SECTOR:
        /* Mark all of the sectors ... */
        if( mode == MBOX_MARK )
        {
            for( i = 0; i < nsector; i++ )
                ++marklist[i];
        }
        else
        {
            for( i = 0; i < nsector; i++ )
                if( marklist[i] )
                    marklist[i] = 2;
        }
        /* ... then remove the unwanted ones from the list */
        for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
        {
            m = ldp->vfrom;
            if( vlist[m].x < x1 || vlist[m].x > x2 || vlist[m].y < y1 || vlist[m].y > y2 )
            {
                sdp = &sidelist[m = ldp->rsdef];
                mysectormarkmacro( m, sdp );
                sdp = &sidelist[m = ldp->lsdef];
                mysectormarkmacro( m, sdp );
                continue;
            }
            m = ldp->vend;
            if( vlist[m].x < x1 || vlist[m].x > x2 || vlist[m].y < y1 || vlist[m].y > y2 )
            {
                sdp = &sidelist[m = ldp->rsdef];
                mysectormarkmacro( m, sdp );
                sdp = &sidelist[m = ldp->lsdef];
                mysectormarkmacro( m, sdp );
                continue;
            }
        }

        for( i = 0, nobj = 0; i < nsector; i++ )
        {
            /* Now do the actual marking */
            if( (marklist[i] == 1 && mode == MBOX_MARK) ||
                (marklist[i] == 2 && mode == MBOX_UNMARK) )
                obj_highlight( MODE_SECTOR, i, COLOR_MARKED ), ++nobj;

            if( marklist[i] == 2 )
                marklist[i] = (mode == MBOX_MARK) ? 1 : 0;
        }

        break;
    }

    cursor_put( CSR_PUT );

    return nobj;
}

void find_slope_point( short *dx, short *dy, short xpos, short ypos,
                       VERTEX *vfrom, VERTEX *vend )
/*-------------------------------------------------------------------------*
 * Finds the point on the slope from vfrom to vend using coordinates xpos  *
 * and ypos.                                                               *
 *-------------------------------------------------------------------------*/
{
    float   slope, y_intr;
    float   rise, run;
    short   x1 = _mapx2scr( vfrom->x-xstart );
    short   y1 = _mapy2scr( vfrom->y-ystart );
    short   x2 = _mapx2scr( vend->x-xstart );
    short   y2 = _mapy2scr( vend->y-ystart );
    short   x, y;

    rise = (y2-y1);
    run = (x2-x1);

    if( !rise )     rise = 1;
    if( !run )      run = 1;

    if( run == 1 )
    {
        *dx = vfrom->x;
        *dy = ypos;
        return;
    }
    if( rise == 1 )
    {
        *dy = vfrom->y;
        *dx = xpos;
        return;
    }

    slope = rise / run;
    if( !slope ) slope = 1;

    y_intr = -(slope * x1 - y1);

    xpos = _mapx2scr( xpos - xstart );
    ypos = _mapy2scr( ypos - ystart );

    // determine if the line is more horizontal or more vertical, and use
    // a slope equation based on the most usable coordinate

    if( abs(rise) > abs(run) )
    {
        // more vertical; use ypos to determine x, y
        x = (short) ((ypos - y_intr) / slope);
        y = (short) ((slope * x) + y_intr);
    }
    else
    {
        // more horizontal; use xpos to determine x, y
        y = (short) ((slope * xpos) + y_intr);
        x = (short) ((y - y_intr) / slope);
    }

    //x = (short) ((((float)xpos / slope) + (float)ypos - y_intr) / (slope + 1 / slope));
    //y = (short) (x * slope + y_intr);

    *dx = xstart + _scrx2map( x );
    *dy = ystart + _scry2map( y );
}

void vertex_replace( short search, short replace )
/*-------------------------------------------------------------------------*
 * Replaces all vertex occurances of 'search' to 'replace.'                *
 *-------------------------------------------------------------------------*/
{
    short     i;
    LINEDEF *ldp;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        if( ldp->vfrom == search )
            ldp->vfrom = replace;
        if( ldp->vend == search )
            ldp->vend = replace;
    }
}

#include "join.h"

short vertex_createjoin( short vertex, short *newvertex )
/*-------------------------------------------------------------------------*
 * Creates a join between "vertex", any other close vertices, or a line
 * that vertex resides on.
 *
 * Returns 1 if a "line split" happened.
 *-------------------------------------------------------------------------*/
{
    _vcj_line = -1;
    _vcj_line2 = -1;

    *newvertex = vertex;

    JoinInfo ji;

    CheckJoin( vertex, ji, jiVertex | (tog_join ? jiLine : 0) );

    if( ji.fJoinType == jiLine )
    {
        /* Create a new line and join the two with our vertex. */
        int new_line = obj_insert( MODE_LINE, ji.iLine, 0, 0 );

        _vcj_line = new_line;
        _vcj_line2 = ji.iLine;

        linelist[ji.iLine].vend = vertex;
        linelist[new_line].vfrom = vertex;

        return 1;
    }

    *newvertex = ji.iVertex;

    return 0;
}

#define MODE_LINEONLY 0x80

int FindObjectAtMap( short x, short y, Object& obj, int iObjMode = editmode );

int GetLenience( int iObjectType );

bool CheckJoin( int iVertex, JoinInfo& ji, int iDistance, unsigned uFlags )
{
    short   x1, y1, x2, y2, x = vlist[iVertex].x, y = vlist[iVertex].y, i;
    short   lobj, curdist, curitem = -1;
    short   curxl, curyl;
    bool    fGotLine = FALSE;
    int     iLineDistance, iLenience = GetLenience( MODE_VERTEX );
    VERTEX  *v;

    // set default data
    ji.x = vlist[iVertex].x;
    ji.y = vlist[iVertex].y;
    ji.fJoinType = jiVertex;
    ji.iVertex = iVertex;

    if( uFlags & jiLine )
    {
        Object obj;

        lobj = FindObjectAtMap( x, y, obj, MODE_LINEONLY );

        if( lobj != -1 )
        {
            /* Is it near this line's vertices? */
            if( iVertex == linelist[lobj].vfrom ||
                iVertex == linelist[lobj].vend )
            {
                goto GetVertices;
            }

            if( vmatch( iVertex, linelist[lobj].vfrom ) )
            {
                curitem = linelist[lobj].vfrom;
                goto ReturnVertex;
            }
            if( vmatch( iVertex, linelist[lobj].vend ) )
            {
                curitem = linelist[lobj].vend;
                goto ReturnVertex;
            }

            find_slope_point( &x1, &y1, x, y,
                            &vlist[linelist[lobj].vfrom],
                            &vlist[linelist[lobj].vend] );

            ji.fJoinType = jiLine;
            ji.iLine = lobj;
            ji.x = x1;
            ji.y = y1;

            fGotLine = TRUE;
            iLineDistance = abs( x1-x ) + abs( y1-y ) + 4;
        }
    }

    if( !(uFlags & jiVertex) )
    {
        if( fGotLine )
            return TRUE;
        return FALSE;
    }

GetVertices:
    // try to join it to existing vertices ..
    for( i = 0, v = vlist; i < nvertex; i++, v++ )
    {
        if( i == iVertex || (tog_drag && obj_ismanip( i )) )
            continue;

        if( !tog_join && config.tog_combineexact )
        {
            if( v->x == x && v->y == y )
            {
                curitem = i;
                break;
            }
        }
        else
        {
            curxl = x - v->x;
            curyl = y - v->y;
            curxl = abs( curxl );
            curyl = abs( curyl );

            // is this vertex closer than our current distance?
            curdist = curxl + curyl;
            if( curdist < iDistance && curxl <= iLenience && curyl <= iLenience )
                curitem = i, iDistance = curdist;
        }
    }

    if( curitem != -1 && !(fGotLine && iLineDistance < curdist) )
    {
        // found a vertex to join to
ReturnVertex:
        ji.fJoinType = jiVertex;
        ji.iVertex = curitem;
        ji.x = vlist[curitem].x;
        ji.y = vlist[curitem].y;
        return TRUE;
    }

    if( fGotLine )
    {
        return TRUE;    // ji is already filled out
    }

    return FALSE; // just nothing at all
}

short obj_insert( short objtype, short source, short x, short y )
/*-------------------------------------------------------------------------*
 * Inserts a new object at x, y using the current defaults.  Does nothing  *
 * else.  Returns the number of the new object.                            *
 *-------------------------------------------------------------------------*/
{
    short       rsdef, lsdef;       /* For creating a new line */
    short       newvertex;
    LINEDEF     *newld;
    THING       *t;
    SIDEDEFM    *sdp;
    SECTOR      *secp;

    switch( objtype )
    {
/*-------------------------------------------------------------------------*
 * Add a new vertex.  Simply creates a new entry in the vlist table and    *
 * returns the number of the new vertex.                                   *
 *-------------------------------------------------------------------------*/
    case MODE_VERTEX:
        vlist[nvertex].x = x;
        vlist[nvertex].y = y;

        /* If source is .. we'll try to put this vertex on a line.
         * Yes, this means searching all the lines for a match :) */
        if( source == VTX_CREATEJOIN )
        {
            /* don't care about the return value */
            vertex_createjoin( nvertex, &newvertex );
            if( newvertex != nvertex )
                return newvertex;
        }

        ++nvertex;
        ++nMadeChanges;
        ++nMapChanges;

        return nvertex-1;
/*-------------------------------------------------------------------------*
 * Add a new line.  Creates a new line and returns its number, and copies  *
 * the stuff from 'source' if required.                                    *
 *-------------------------------------------------------------------------*/
    case MODE_LINE:
        newld = &linelist[nline];
        if( source >= 0 )
        {
            memcpy( newld, &linelist[source], sizeof( LINEDEF ) );
            rsdef = linelist[source].rsdef;
            lsdef = linelist[source].lsdef;
        }
        else
        {
            /* Set default attributes */
            newld->attribs = LA_IMPASS;
            newld->lsdef = -1;
            newld->special = 0;
            rsdef = -1;
            lsdef = -1;
        }

        newld->rsdef = obj_insert( MODE_SIDEDEF, rsdef, 0, 0 );
        if( lsdef != -1 )
            newld->lsdef = obj_insert( MODE_SIDEDEF, lsdef, 0, 0 );

        ++nMadeChanges;
        ++nMapChanges;

        ++nline;
        return nline-1;
/*-------------------------------------------------------------------------*
 * Add a new sidedef.                                                      *
 *-------------------------------------------------------------------------*/
    case MODE_SIDEDEF:
        sdp = &sidelist[nside];
        if( source >= 0 )
            memcpy( sdp, &sidelist[source], sizeof( SIDEDEFM ) );
        else
        {
            /* Copy in defaults for sidedef */
            memset( sdp, 0, sizeof( SIDEDEFM ) );
            sdp->utxidx = sdp->ltxidx = 0;
            sdp->ftxidx = _def_ftxidx;
            sdp->sector = -1;
        }
        ++nside;
        return nside-1;
/*-------------------------------------------------------------------------*
 * Add a new thing!                                                        *
 *-------------------------------------------------------------------------*/
    case MODE_THING:
        nMadeChanges++;
        t = &thinglist[nthing];
        if( source >= 0 && source < nthing )
            memcpy( t, &thinglist[source], sizeof( THING ) );
        else
        {
            t->x = x;
            t->y = y;
            thing_autosetangle( nthing );
            t->type = 1;
            t->attribs = TA_SKILL12 | TA_SKILL3 | TA_SKILL45;

            if( fHexen )
            {
                t->attribs |= TA_FIGHTER | TA_CLERIC | TA_MAGE |
                             TA_GSINGLE | TA_GCOOP | TA_GDEATHMATCH;
            }
        }

        t->tid = 0;
        t->x = x;
        t->y = y;
        ++nthing;
        return nthing-1;
/*-------------------------------------------------------------------------*
 * Add a new sector                                                        *
 *-------------------------------------------------------------------------*/
    case MODE_SECTOR:
        nMadeChanges++;
        secp = &sectorlist[nsector];
        if( source >= 0 )
            memcpy( secp, &sectorlist[source], sizeof( SECTOR ) );
        else
        {
            secp->fheight = 0;
            secp->cheight = 128;
            memcpy( secp->ftxname, _def_floortex, 8 );
            memcpy( secp->ctxname, _def_ceiltex, 8 );
            secp->light = _def_light;
            secp->type = 0;
        }

        secp->sectag = 0;

        ++nsector;
        return nsector-1;
    }

    return -1;
}

short _linesplit( short lineno, short vertexno )
{
    short new_line = obj_insert( MODE_LINE, lineno, 0, 0 );
    linelist[lineno].vend = vertexno;
    linelist[new_line].vfrom = vertexno;
    lSecTag( &linelist[new_line] ) = lSecTag( &linelist[lineno] );
    return new_line;
}

short obj_splitlines( short csr_object )
/*-------------------------------------------------------------------------*
 * Splits all marked lines AND the line in csr_object if not -1 and        *
 * returns the number of lines split.                                      *
 *-------------------------------------------------------------------------*/
{
    VERTEX  *v1, *v2;
    short     i, new_vertex, new_line, nsplit = 0, unmarkobj = 0;
    short     nlinesentry = nline;       /* cuz we're adding some too */

    if( csr_object != -1 && !obj_ismarked( csr_object ) )
        obj_mark( csr_object ), unmarkobj = 1;

    /* Split the lines */
    for( i = 0; i < nlinesentry; i++ )
    {
        /* Marked?  Split it. */
        if( obj_ismarked( i ) )
        {
            ++nsplit;
            v1 = &vlist[linelist[i].vfrom];
            v2 = &vlist[linelist[i].vend];
            new_vertex = obj_insert( MODE_VERTEX, VTX_CREATE,
                                     v1->x + (v2->x - v1->x) / 2,
                                     v1->y + (v2->y - v1->y) / 2 );
            new_line = _linesplit( i, new_vertex );

            if( i != csr_object || (i == csr_object && !unmarkobj) )
                obj_mark( new_line );
        }
    }

    nMapChanges++;
    nMadeChanges++;

    if( unmarkobj )
        obj_unmark( csr_object );

    return nsplit;
}

#define MINVADIST   7

void _mapsnap( short *dx, short *dy, short sx, short sy, bool fSimple )
/*-------------------------------------------------------------------------*
 * Snaps sx, sy to dx, dy.  Both are DOOM MAP coordinates.                 *
 *-------------------------------------------------------------------------*/
{
    short       t1, t2, x = sx, y = sy;
    VERTEX      *v;
    short       i, dist, curdist, curitem = -1, curxl, curyl;

    dx[0] = sx;
    dy[0] = sy;

    if( tog_snap != S_NONE )  /* well, only if we HAVE to snap. */
    {
        t1 = abs( sx % gridstep );
        t2 = abs( sy % gridstep );

        if( sx > 0 )
        {
            if( t1 >= gridstep / 2 )
                x += (gridstep-t1);
            else
                x -= t1;
        }
        else
        {
            if( t1 >= gridstep / 2 )
                x -= (gridstep-t1);
            else
                x += t1;
        }

        if( sy > 0 )
        {
            if( t2 >= gridstep / 2 )
                y += (gridstep-t2);
            else
                y -= t2;
        }
        else
        {
            if( t2 >= gridstep / 2 )
                y -= (gridstep-t2);
            else
                y += t2;
        }

        sx = x;
        sy = y;
    }

    if( editmode != MODE_THING && fSimple != 2 )
    {
        // dummy vertex
        vlist[nvertex].x = dx[0];
        vlist[nvertex].y = dy[0];
        JoinInfo ji;

        // distance starts out as being the distance to the closest gridpoint.
        // only if we find a closer vertex do we snap anywhere else ..
        int curxl = sx - (*dx);
        int curyl = sy - (*dy);
        curxl = abs( curxl );
        curyl = abs( curyl );

        int dist = curxl + curyl;

        int iMode = (tog_snap == S_VERTEX ? jiVertex : 0);

        if( !fSimple )
            iMode |= jiLine;

        if( CheckJoin( nvertex, ji, dist, iMode ) )
        {
            *dx = ji.x;
            *dy = ji.y;
            return;
        }
    }

    dx[0] = sx;
    dy[0] = sy;
}

void _scr2map( short *dx, short *dy, short cx, short cy )
{
    short tx, ty;

    if( tog_snap )
    {
        tx = xstart + _scrx2map( cx );
        ty = ystart + _scry2map( cy );
        _mapsnap( dx, dy, tx, ty, TRUE );
    }
    else
    {
        *dx = xstart + _scrx2map( cx );
        *dy = ystart + _scry2map( cy );
    }
}

short obj_align( short csr_object, short axis )
/*-------------------------------------------------------------------------*
 * Aligns all the marked objects along the (X = 0) axis.                   *
 *-------------------------------------------------------------------------*/
{
    short     nmoved = 0, i;
    short     xpos, ypos;

    if( editmode != MODE_VERTEX && editmode != MODE_THING )
        return 0;

    _scr2map( &xpos, &ypos, cursorx, cursory );

    if( csr_object != -1 )
        obj_mark( csr_object );

    switch( editmode )
    {
    case MODE_THING:
        for( i = 0; i < MAXOBJECT; i++ )
        {
            if( obj_ismarked( i ) )
            {
                if( axis )
                    thinglist[i].y = ypos;
                else
                    thinglist[i].x = xpos;
                ++nmoved;
            }
        }
        break;
    case MODE_VERTEX:
        for( i = 0; i < MAXOBJECT; i++ )
        {
            if( obj_ismarked( i ) )
            {
                if( axis )
                    vlist[i].y = ypos;
                else
                    vlist[i].x = xpos;
                ++nmoved;
            }
        }
        break;
    }

    if( nmoved )
    {
        nMapChanges++;
        nMadeChanges++;
    }

    if( csr_object != -1 )
        obj_unmark( csr_object );

    return nmoved;
}

short obj_delete( short editmode, Object& objDelete, short delmarked )
/*-------------------------------------------------------------------------*
 * Deletes all marked objects                                              *
 *-------------------------------------------------------------------------*/
{
    short   i, ndeleted = 0, theline, allgone;
    LINEDEF *ldp;
    Object  obj;
    ObjEnum oe;

    if( objDelete.IsValid() )
        obj_mark( objDelete );

    if( delmarked )
        undo_init();

    switch( editmode )
    {
/*-------------------------------------------------------------------------*
 * Delete vertices                                                         *
 *-------------------------------------------------------------------------*/
    case MODE_VERTEX:
        ndeleted = nvertex;
        map_purge( MODE_VERTEX );
        ndeleted -= nvertex;
        break;
/*-------------------------------------------------------------------------*
 * Delete lines                                                            *
 *-------------------------------------------------------------------------*/
    case MODE_LINE:
        obj = oe.Start( MODE_LINE );
        while( obj.IsValid() )
        {
            if( (delmarked && obj_ismarked( obj )) ||
                (!delmarked && obj == objDelete) )
            {
                i = obj.iNumber;
                memcpy( linelist+i, linelist+i+1, sizeof( LINEDEF ) *
                        (nline-i) );
                obj_shiftmarksdown( obj );
                ++ndeleted;
                oe.PauseOne();

                if( !delmarked )
                    break;
            }

            obj = oe.Next();
        }

        // cut out these lines
        nline -= ndeleted;

        if( ndeleted )
           map_purge( MODE_ALL );

        break;
/*-------------------------------------------------------------------------*
 * Delete things                                                           *
 *-------------------------------------------------------------------------*/
    case MODE_THING:
        obj = oe.Start( MODE_THING );

        while( obj.IsValid() )
        {
            // delete this thing?
            if( (delmarked && obj_ismarked( obj )) ||
                (!delmarked && obj == objDelete) )
            {
                i = obj.iNumber;
                memcpy( thinglist+i, thinglist+i+1, sizeof( THING ) *
                        (nthing-i) );
                obj_shiftmarksdown( obj );
                ++ndeleted;
                oe.PauseOne();

                if( !delmarked )
                    break;
            }

            obj = oe.Next();
        }
        nthing -= ndeleted;
        break;
/*-------------------------------------------------------------------------*
 * Delete sectors                                                          *
 *-------------------------------------------------------------------------*/
    case MODE_SECTOR:
        obj = oe.Start( MODE_SECTOR );

        while( obj.IsValid() )
        {
            if( (delmarked && obj_ismarked( obj )) ||
                (!delmarked && obj == objDelete) )
            {
                i = obj.iNumber;
_redo:
                allgone = 1;

                /* Delete all the lines.. */
                theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, i, LF_END );

                while( theline != -1 )
                {
                    /* Now.. we should check this line for two-sidedness.  If the
                     * line is two-sided, just delete one of its sides (the relevant one)
                     * and make it one-sided / impassable. */
                    ldp = &linelist[theline];
                    if( ldp->lsdef >= 0 && ldp->rsdef >= 0 )
                    {
                        /* Kill the flags man */
                        ldp->attribs &= ~(LA_TSIDE);
                        ldp->attribs |= LA_IMPASS;

                        /* Delete the appropriate sidedef (well, set it to -1.. then
                         * call map_purge() later.) */
                        if( ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == i )
                            ldp->lsdef = -1;
                        else if( ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == i )
                        {
                            /* Special handling for this because we have to flip the
                             * sidedefs. */
                            ldp->rsdef = ldp->lsdef;
                            ldp->lsdef = -1;
                            swap2b( ldp->vfrom, ldp->vend );
                        }

                        _WFfixtexnames( theline, 0, 0 );
                        allgone = 0;
                        theline = obj_linefind( LF_CONTINUE );
                    }
                    else
                    {
                        Object obj( theline, MODE_LINE );
                        obj_delete( MODE_LINE, obj, 0 );
                        /* don't do a continue search because the search is now invalid..
                         * we deleted a line. */
                        theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, i, LF_END );
                    }
                }

                if( !allgone )
                    goto _redo;

                ++ndeleted;

                if( !delmarked )
                    break;
            }

            obj = oe.Next();
        }

        map_purge( MODE_ALL );
        break;
    }

    if( ndeleted )
    {
        post_fullupdate();
        obj_clearmarks( -1, FALSE );

        nMadeChanges++;
        if( editmode != MODE_THING )
            nMapChanges++;
    }

    if( delmarked )
        undo_setvars();

    return ndeleted;
}

short line_flip( short csr_object, uchar flipvertex )
/*-------------------------------------------------------------------------*
 * Flips all marked lines (including csr_object) by reversing their        *
 * sidedefs.  If flipvertex is TRUE, flips their vertices too.             *
 *-------------------------------------------------------------------------*/
{
    short     i, tmp, nflipped = 0;
    LINEDEF *ldp;

    if( csr_object != -1 )
        obj_mark( csr_object );

    for( i = 0, ldp = linelist; i < MAXOBJECT; i++, ldp++ )
    {
        if( obj_ismarked( i ) )
        {
            /* Flip sidedefs */
            if( !(ldp->rsdef != -1 && ldp->lsdef == -1) )
            {
                tmp = ldp->rsdef;
                ldp->rsdef = ldp->lsdef;
                ldp->lsdef = tmp;
            }
            /* Flip vertices? */
            if( flipvertex )
            {
                tmp = ldp->vfrom;
                ldp->vfrom = ldp->vend;
                ldp->vend = tmp;
            }
            ++nflipped;
        }
    }

    if( csr_object != -1 )
        obj_unmark( csr_object );

    if( nflipped )
    {
        nMadeChanges++;
        if( flipvertex )
            nMapChanges++;
    }

    return nflipped;
}

ushort   lf_startpos;

short obj_linefind( ushort command, ... )
/*-------------------------------------------------------------------------*
 * Searches the list of lines for lines matching certain attributes.       *
 * If command is LF_BEGIN, searches from the beginning.  LF_CONTINUE,      *
 * searches from last found position+1.  Returns the first found line.     *
 *-------------------------------------------------------------------------*/
{
    va_list vl;
    LINEDEF *ldp;
    short     i;
    ushort   lftype, lfarg;
    static  ushort   vfrom, vend, vany;
    static  ushort   attribs;
    static  ushort   sectag;
    static  ushort   sidewithsec;
    static  ushort   type;

    va_start( vl, command );

    if( command == LF_BEGIN || command == LF_SETPARM )
    {
        /* "Don't search for" value is 0xFFFFU */
        if( command == LF_BEGIN )
        {
            vfrom = vend = vany = attribs = sectag = sidewithsec = type = 0xFFFFU;
            lf_startpos = 0;
        }
        else
            ++lf_startpos;

        i = 0;

        while( i++ < 8 )
        {
            /* Get the command */
            lftype = va_arg( vl, ushort );

            /* Get the parameter */
            if( lftype != LF_END )
                lfarg = va_arg( vl, ushort );
            else
                break;

            switch( lftype )
            {
            case LF_VFROM:  /* find from vertex */
                vfrom = lfarg;
                break;
            case LF_VEND:   /* find end vertex */
                vend = lfarg;
                break;
            case LF_VANY:   /* find any vertex */
                vany = lfarg;
                break;
            case LF_ATTRIBS:    /* find some attribs (inclusive) */
                attribs = lfarg;
                break;
            case LF_SECTAG: /* find a sector tag */
                sectag = lfarg;
                break;
            case LF_SIDEWITHSEC:    /* find a side with sector */
                sidewithsec = lfarg;
                break;
            case LF_TYPE:   /* find a type */
                type = lfarg;
                break;
            }
        }

        if( i >= 8 )
            error( "obj_linefind(): LF_END not found before 8 specs (problem with call)" );
    }
    else
        ++lf_startpos;

    va_end( vl );

    /* Begin the search */
    for( i = lf_startpos, ldp = linelist+lf_startpos; i < nline; i++, ldp++ )
    {
        if( vfrom != 0xFFFFU && ldp->vfrom != vfrom )
            continue;
        if( vend != 0xFFFFU && ldp->vend != vend )
            continue;
        if( vany != 0xFFFFU && (ldp->vend != vany && ldp->vfrom != vany) )
            continue;
        if( attribs != 0xFFFFU && ((ldp->attribs & attribs) != attribs)  )
            continue;
        if( sectag != 0xFFFFU && lSecTag( ldp ) != sectag )
            continue;
        if( sidewithsec != 0xFFFFU )
        {
            if( !((ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == sidewithsec) ||
                  (ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == sidewithsec)) )
                continue;
        }
        if( type != 0xFFFFU && ldp->special != type )
            continue;

        lf_startpos = i;
        return i;
    }

    return -1;
}

short obj_linelen( short object )
{
    VERTEX  *v = &vlist[linelist[object].vfrom],
            *v2 = &vlist[linelist[object].vend];
    return line_len( v->x, v->y, v2->x, v2->y );
}

short obj_delljoinsectors( short csr_object )
/*-------------------------------------------------------------------------*
 * Deletes a line and joins the two sectors it borders:                    *
 * All sidedefs whose sector setting is that of the line's second          *
 * sidedef's get changed to the first sidedef's sector.                    *
 *-------------------------------------------------------------------------*/
{
    LINEDEF *ldp, *ldp2;
    short     i, ndel = 0, theline;
    short     sec1, sec2, unmarkobj = 0;

    if( csr_object != -1 && !obj_ismarked( csr_object ) )
        obj_mark( csr_object ), unmarkobj = 1;

    undo_init();

    ObjEnum oe;
    Object obj;

    for( obj = oe.Start( MODE_LINE ); obj.IsValid(); obj = oe.Next() )
    {
        if( !obj_ismarked( obj ) )
            continue;

        i = obj.iNumber;
        ldp = (LINEDEF*) obj;

        /* Check for two sides */
        if( ldp->lsdef == -1 || ldp->rsdef == -1 )
            continue;

        sec1 = sidelist[ldp->rsdef].sector;
        sec2 = sidelist[ldp->lsdef].sector;

        /* Find all lines with this sector reference and set them
            * to the other one.  Yeah. */
        theline = obj_linefind( LF_BEGIN,
                                LF_SIDEWITHSEC, sec2,
                                LF_END );
        while( theline != -1 )
        {
            ldp2 = &linelist[theline];
            if( sidelist[ldp2->rsdef].sector == sec2 )
                sidelist[ldp2->rsdef].sector = sec1;
            if( sidelist[ldp2->lsdef].sector == sec2 )
                sidelist[ldp2->lsdef].sector = sec1;
            theline = obj_linefind( LF_CONTINUE );
        }

        memcpy( linelist+i, linelist+i+1, sizeof( LINEDEF ) * (nline-i) );
        --nline;
        obj_shiftmarksdown( obj );
        oe.PauseOne();

        nMapChanges++;
        nMadeChanges++;
        ++ndel;
    }

    undo_setvars();

    if( unmarkobj )
        obj_unmark( csr_object );

    return ndel;
}

short obj_delvjoinline( short csr_object )
/*-------------------------------------------------------------------------*
 * Attempts to delete all marked vertexes and join the lines they          *
 * separate.                                                               *
 *-------------------------------------------------------------------------*/
{
    LINEDEF *ldp;
    short     i, i2, l1, l2, ndel = 0;

    if( csr_object != -1 )
        obj_mark( csr_object, MODE_VERTEX );

    undo_init();

    for( i = 0; i < nvertex; i++ )
    {
        if( obj_ismarked( i, MODE_VERTEX ) )
        {
            l1 = -1; l2 = -1;

            for( i2 = 0, ldp = linelist; i2 < nline; i2++, ldp++ )
            {
                if( ldp->vend == i )
                {
                    l1 = i2;
                    if( l2 != -1 )
                        break;
                }
                if( ldp->vfrom == i )
                {
                    l2 = i2;
                    if( l1 != -1 )
                        break;
                }
            }

            /* can't do this vertex? */
            if( l1 == -1 || l2 == -1 )
                continue;

            /* Otherwise.. */
            linelist[l1].vend = linelist[l2].vend;
            memcpy( linelist+l2, linelist+l2+1, (nline-l2+1) * sizeof( LINEDEF ) );
            ndel++;
            --nline;
            nMapChanges++;
            nMadeChanges++;

            obj_unmark( i, MODE_VERTEX );
        }
    }

    /* Remove unused vertices */
    map_purge( MODE_VERTEX );

    undo_setvars();

    return ndel;
}

void line_delvsame( void )
/*-------------------------------------------------------------------------*
 * Deletes all lines whose vertex references are the same.                 *
 *-------------------------------------------------------------------------*/
{
    short     i;
    LINEDEF *ldp;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        if( ldp->vfrom == ldp->vend )
        {
            /* This one's invalid */
            memcpy( linelist+i, linelist+i+1, (nline-i) * sizeof( LINEDEF ) );
            --nline;
            --ldp;
            --i;
            nMapChanges ++;
        }
    }
}

void line_combinesame( short *llist, short llcount )
/*-------------------------------------------------------------------------*
 * Searches for congruent lines and combines them.                         *
 *-------------------------------------------------------------------------*/
{
    short     i = 0, i2;
    short     vfrom, vend, *llp = llist, deleted = -1;
    LINEDEF   *ldp, *ldp2;

    SetStatusMsg( "Combining lines..." );
    UpdateMessageBar();

_redoloop:
    if( deleted != -1 )
    {
        i++;
        llp++;

        linelist[deleted].vfrom = linelist[deleted].vend = 0;
    }

    for( ; i < llcount; i++, llp++ )
    {
        ldp = &linelist[*llp];

        vfrom = ldp->vfrom;
        vend = ldp->vend;

        for( i2 = 0, ldp2 = linelist; i2 < nline; i2++, ldp2++ )
        {
            if( ldp2->vfrom == vfrom && ldp2->vend == vend && i2 != *llp )
            {
                obj_lstartcombine( *llp );
                deleted = obj_lfinishcombine( i2 );
                goto _redoloop;
            }
            else if( ldp2->vfrom == vend && ldp2->vend == vfrom && i2 != *llp )
            {
                obj_lstartcombine( *llp );
                deleted = obj_lfinishcombine( i2 );
                goto _redoloop;
            }
        }
    }

    nMadeChanges ++;
    nMapChanges ++;

    SetStatusMsg( st_blank );
    UpdateMessageBar();
}

short obj_snapmarked( Object& objCursor )
/*-------------------------------------------------------------------------*
 * Snaps all marked objects to grid.                                       *
 *-------------------------------------------------------------------------*/
{
    uchar   *dolist = new uchar[MAXOBJECT];
    short   i, unmarkobj = 0, theline, oldsnap = tog_snap, nsnap = 0;
    VERTEX  *v;
    THING   *t;
    SECTOR  *secp;

    if( objCursor.IsValid() && !obj_ismarked( objCursor ) )
        obj_mark( objCursor ), unmarkobj = 1;

    tog_snap = S_NORMAL;

    memset( dolist, 0, MAXOBJECT );

    switch( editmode )
    {
    case MODE_SECTOR:
        for( i = 0, secp = sectorlist; i < nsector; i++, secp++ )
        {
            if( obj_ismarked( i ) )
            {
                nsnap++;

                /* Marked sector; now find all its lines */
                theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, i, LF_END );

                while( theline != -1 )
                {
                    /* Mark its vertexes */
                    dolist[linelist[theline].vfrom] = 1;
                    dolist[linelist[theline].vend] = 1;
                    theline = obj_linefind( LF_CONTINUE );
                }
            }
        }
        break;
    case MODE_LINE:
        memcpy( dolist, marklist, MAXOBJECT );
        nsnap = obj_nmarked( MODE_VERTEX );
        for( i = 0; i < nline; i++ )
        {
            if( obj_ismarked( i, MODE_LINE ) )
            {
                nsnap++;
                dolist[linelist[i].vfrom] = TRUE;
                dolist[linelist[i].vend] = TRUE;
            }
        }
        break;
    case MODE_THING:
        memcpy( dolist, marklist, MAXOBJECT );
        nsnap = obj_nmarked( MODE_THING );
        break;
    }

    if( nsnap )
    {
        nMadeChanges++;

        undo_init();

        /* Now that we have the list of things we want to snap, snap them. */
        switch( editmode )
        {
        case MODE_THING:
            for( i = 0, t = thinglist; i < nthing; i++, t++ )
            {
                if( dolist[i] )
                    _mapsnap( &(t->x), &(t->y), t->x, t->y, 2 );
            }
            break;
        case MODE_VERTEX:
        case MODE_LINE:
        case MODE_SECTOR:
            for( i = 0, v = vlist; i < nvertex; i++, v++ )
            {
                if( dolist[i] )
                    _mapsnap( &(v->x), &(v->y), v->x, v->y, 2 );
            }
            nMapChanges++;
            break;
        }
    }

    delete dolist;

    if( unmarkobj )
        obj_unmark( objCursor );

    tog_snap = oldsnap;

    undo_setvars();

    return nsnap;
}

short obj_linesetmotif( short object )
/*-------------------------------------------------------------------------*
 * Sets all marked sidedefs (including object's) to use the current motif  *
 * textures.                                                               *
 *-------------------------------------------------------------------------*/
{
    short     i, unmarkobj = 0, nrep = 0;
    LINEDEF *ldp;
    SIDEDEFM *sdp;

    if( editmode == MODE_LINE )
    {
        if( object != -1 && !obj_ismarked( object ) )
            obj_mark( object ), unmarkobj = 1;
    }
    else
    {
        /* not in line mode; only do "object" */
        if( object == -1 )
            return 0;

        i = nline;      /* to break out of loop cuz we jump */
        ldp = &linelist[object];
        goto _dorep;
    }

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        if( obj_ismarked( i ) )
        {
_dorep:
            /* Do this one. */
            if( ldp->lsdef >= 0 )
            {
                sdp = &sidelist[ldp->lsdef];

                if( sdp->ftxidx )
                    sdp->ftxidx = _def_ftxidx, nrep++;
                if( sdp->utxidx )
                    sdp->utxidx = _def_utxidx, nrep++;
                if( sdp->ltxidx )
                    sdp->ltxidx = _def_ltxidx, nrep++;
            }

            if( ldp->rsdef >= 0 )
            {
                sdp = &sidelist[ldp->rsdef];

                if( sdp->ftxidx )
                    sdp->ftxidx = _def_ftxidx, nrep++;
                if( sdp->utxidx )
                    sdp->utxidx = _def_utxidx, nrep++;
                if( sdp->ltxidx )
                    sdp->ltxidx = _def_ltxidx, nrep++;
            }
        }
    }

    if( unmarkobj )
        obj_unmark( object );

    if( nrep )
        nMadeChanges ++;

    return nrep;
}

short obj_sectsetmotif( short object )
/*-------------------------------------------------------------------------*
 * Sets a sector's line's sisdededss to the current motif.                 *
 *-------------------------------------------------------------------------*/
{
    short       theline;
    SECTOR      *secp;
    LINEDEF     *ldp;
    SIDEDEFM    *sdp;
    short       unmarkobj = 0;
    short       nrep = 0, i;

    if( object != -1 && !obj_ismarked( object ) )
        obj_mark( object ), unmarkobj = 1;

    for( i = 0, secp = sectorlist; i < nsector; i++, secp++ )
    {
        if( obj_ismarked( i ) )
        {
            /* Copy over the sector attributes */
            memcpy( secp->ftxname, _def_floortex, sizeof( WADNAME ) );
            memcpy( secp->ctxname, _def_ceiltex, sizeof( WADNAME ) );
            secp->light = _def_light;

            /* Set all the lines */
            theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, i, LF_END );

            while( theline != -1 )
            {
                ldp = &linelist[theline];

                if( ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == i )
                    sdp = &sidelist[ldp->lsdef];
                else if( ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == i )
                    sdp = &sidelist[ldp->rsdef];

                if( sdp->ftxidx )
                    sdp->ftxidx = _def_ftxidx, nrep++;
                if( sdp->utxidx )
                    sdp->utxidx = _def_utxidx, nrep++;
                if( sdp->ltxidx )
                    sdp->ltxidx = _def_ltxidx, nrep++;

                theline = obj_linefind( LF_CONTINUE );
            }
        }
    }

    if( unmarkobj )
        obj_unmark( object );

    if( nrep )
        nMadeChanges ++;

    return nrep;
}

short _lcomb_first = -1;

short obj_lstartcombine( short csr_object )
/*-------------------------------------------------------------------------*
 * Starts the line-combine operation.  If csr_object == -1, returns 0.     *
 *-------------------------------------------------------------------------*/
{
    if( csr_object == -1 )
        return 0;
    _lcomb_first = csr_object;
    return 1;
}

short obj_lfinishcombine( short csr_object )
/*-------------------------------------------------------------------------*
 * Completes the line-combine operation.                                   *
 *-------------------------------------------------------------------------*/
{
    short       _lcomb_second = csr_object;
    LINEDEF     *combline = &linelist[_lcomb_second];
    LINEDEF     *dr_line = &linelist[_lcomb_first];    /* Death row line */
    VERTEX      *v1, *dr_v1, *dr_v2;
    short       dist1, dist2, tmp;

    if( dr_line->vfrom == dr_line->vend ||
        combline->vfrom == combline->vend )
        return _lcomb_second;

    if( csr_object == -1 )
    {
        _lcomb_first = -1;
        return -1;
    }

    /* Get the vertexes */
    v1 = &vlist[combline->vfrom];
    dr_v1 = &vlist[dr_line->vfrom];
    dr_v2 = &vlist[dr_line->vend];

    dist1 = (abs( v1->x - dr_v1->x ) + abs( v1->y - dr_v1->y ));
    dist2 = (abs( v1->x - dr_v2->x ) + abs( v1->y - dr_v2->y ));

    if( dist1 < dist2 )
    {
        /* clVertex1 = drVertex1 */
        vertex_replace( dr_line->vfrom, combline->vfrom );
        vertex_replace( dr_line->vend, combline->vend );
    }
    else
    {
        vertex_replace( dr_line->vfrom, combline->vend );
        vertex_replace( dr_line->vend, combline->vfrom );
    }

    /* fix sector references */
    combline->rsdef = obj_insert( MODE_SIDEDEF, combline->rsdef, 0, 0 );
    combline->lsdef = obj_insert( MODE_SIDEDEF, combline->rsdef, 0, 0 );

    tmp = obj_getphyssector( _lcomb_second, 0 );
    if( tmp == -1 )
    {
        swap2b( combline->vfrom, combline->vend );
        tmp = obj_getphyssector( _lcomb_second, 0 );
    }
    sidelist[combline->rsdef].sector = tmp;

    tmp = obj_getphyssector( _lcomb_second, 1 );
    if( tmp == -1 )
        combline->lsdef = -1;
    else
        sidelist[combline->lsdef].sector = tmp;

    /* Set the proper attributes */
    if( combline->lsdef != -1 )
    {
        combline->attribs &= ~LA_IMPASS;
        combline->attribs |= LA_TSIDE;
    }
    else
    {
        combline->attribs |= LA_IMPASS;
        combline->attribs &= ~LA_TSIDE;
    }

    _WFfixtexnames( _lcomb_second, 0, 1 );
    _WFfixtexnames( _lcomb_second, 1, 1 );

    _WFfixtexnames( _lcomb_second, 0, 0 );
    _WFfixtexnames( _lcomb_second, 1, 0 );

    nMadeChanges++;
    nMapChanges++;
    post_fullupdate();

    /* return the line number to delete */
    tmp = _lcomb_first; /* so we can return it */
    _lcomb_first = -1;

    return tmp;        /* this one was deleted */
}

short obj_tagset( short source, short sourcemode, short destmode )
/*-------------------------------------------------------------------------*
 * Source is THE ONE.  sourcemode is the ONE's type.                       *
 * The opposite is the NOT sourcemode.                                     *
 *-------------------------------------------------------------------------*/
{
    short   i;
    short   tag;
    short   ntagged = 0;

    short firstfreetid();

    if( sourcemode == MODE_LINE )
        tag = lSecTag( &linelist[source] );
    else if( sourcemode == MODE_SECTOR )
        tag = sectorlist[source].sectag;
    else
        tag = thinglist[source].tid;

    if( !tag )
    {
        if( destmode != MODE_THING )
            tag = firstfreetag();
        else
            tag = firstfreetid();
    }
    else
    {
        i = MenuPop( 0, " Set tags", NULL,
            " Use clicked object's tag ", "The clicked object already has a tag - this selection will use it",
            " Find new tag",              "Sets a new tag in the objects",
            NULL );
        if( i == -1 )
            return 0;
        else if( i == 1 )
        {
            if( destmode != MODE_THING )
                tag = firstfreetag();
            else
                tag = firstfreetid();
        }
    }

    if( sourcemode == MODE_LINE )
    {
        lSecTag( &linelist[source] ) = tag;

        if( destmode == MODE_SECTOR )
        {
            for( i = 0; i < nsector; i++ )
            {
                if( obj_ismarked( i, MODE_SECTOR ) )
                    sectorlist[i].sectag = tag, ++ntagged;
            }
        }
        else
        {
            for( i = 0; i < nthing; i++ )
            {
                if( obj_ismarked( i, MODE_THING ) )
                    thinglist[i].tid = tag, ++ntagged;
            }
        }
    }
    else
    {
        if( sourcemode == MODE_SECTOR )
        {
            sectorlist[source].sectag = tag;
        }
        else
        {
            thinglist[source].tid = tag;
        }

        for( i = 0; i < nline; i++ )
        {
            if( obj_ismarked( i, MODE_LINE ) )
                lSecTag( &linelist[i] ) = tag, ++ntagged;
        }
    }

    if( ntagged )
        nMadeChanges ++;

    return ntagged;
}

short obj_linenormalize( short object )
/*-------------------------------------------------------------------------*
 * "Normalizes" all marked lines (removes sectags and types.)              *
 *-------------------------------------------------------------------------*/
{
    short     i, unmarkobj = 0, nnorm = 0;
    LINEDEF *ldp;

    if( !obj_ismarked( object ) && object != -1 )
        obj_mark( object ), unmarkobj = 1;

    for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
    {
        if( obj_ismarked( i ) )
        {
            ldp->special = 0;
            ldp->arg0 = 0;
            ldp->arg1 = 0;
            ldp->arg2 = 0;
            ldp->arg3 = 0;
            ldp->arg4 = 0;
            ++ nnorm;
        }
    }

    if( unmarkobj )
        obj_unmark( object );

    if( nnorm )
        nMadeChanges ++;

    return nnorm;
}

short obj_thingnormalize( short object )
/*-------------------------------------------------------------------------*
 * "Normalizes" all marked things (removes specials and arguments.)
 *-------------------------------------------------------------------------*/
{
    short   i, unmarkobj = 0, nnorm = 0;
    THING   *tptr;

    if( !obj_ismarked( object ) && object != -1 )
        obj_mark( object ), unmarkobj = 1;

    for( i = 0, tptr = thinglist; i < nthing; i++, tptr++ )
    {
        if( obj_ismarked( i ) )
        {
            tptr->special = 0;
            tptr->arg0 = 0;
            tptr->arg1 = 0;
            tptr->arg2 = 0;
            tptr->arg3 = 0;
            tptr->arg4 = 0;
            ++ nnorm;
        }
    }

    if( unmarkobj )
        obj_unmark( object );

    if( nnorm )
        nMadeChanges ++;

    return nnorm;
}

short obj_sectnormalize( short object )
/*-------------------------------------------------------------------------*
 * "Normalizes" all marked sectors (removes sectags.)                      *
 *-------------------------------------------------------------------------*/
{
    short     i, unmarkobj = 0, nnorm = 0;
    SECTOR  *secp;

    if( !obj_ismarked( object ) && object != -1 )
        obj_mark( object ), unmarkobj = 1;

    for( i = 0, secp = sectorlist; i < nsector; i++, secp++ )
    {
        if( obj_ismarked( i ) )
        {
            secp->sectag = 0;
            ++ nnorm;
        }
    }

    if( unmarkobj )
        obj_unmark( object );

    if( nnorm )
        nMadeChanges ++;

    return nnorm;
}

void obj_markxlate( short oldmode, short newmode )
/*-------------------------------------------------------------------------*
 * Translates the object marks between the old mode and the new mode.      *
 *-------------------------------------------------------------------------*/
{
    short   i, theline;

    if( oldmode != MODE_SECTOR || newmode != MODE_LINE )
    {
        memset( linemarklist, 0, sizeof linemarklist );
        memset( marklist, 0, sizeof marklist );
        return;
    }

    uchar   *omarklist = new uchar[sizeof marklist];

    memcpy( omarklist, marklist, sizeof marklist );
    memset( linemarklist, 0, sizeof linemarklist );
    memset( marklist, 0, sizeof marklist );

    for( i = 0; i < MAXOBJECT; i++ )
    {
        if( omarklist[i] )
        {
            /* find all lines associated with this sector */
            theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, i, LF_END );
            while( theline != -1 )
            {
                linemarklist[theline] = 1;
                theline = obj_linefind( LF_CONTINUE );
            }
        }
    }

_done:
    delete omarklist;
}

/*-------------------------------------------------------------------------*
 * undo stuff here.
 *-------------------------------------------------------------------------*/

static  short snline = -1, snvertex = -1, snside = -1, snsector = -1, snthing = -1;
static  LINEDEF *slinelist;
static  VERTEX *svlist;
static  SECTOR *sseclist;
static  SIDEDEFM *ssidelist;
static  THING *sthinglist;

void obj_push( short mode )
{
    if( mode & MODE_LINE )
    {
        if( slinelist ) memfree( slinelist );
        slinelist = (LINEDEF*) memget( sizeof( LINEDEF ) * nline );
        memcpy( slinelist, linelist, sizeof( LINEDEF ) * nline );
        snline = nline;
    }
    if( mode & MODE_VERTEX )
    {
        if( svlist ) memfree( svlist );
        svlist = (VERTEX*) memget( sizeof( VERTEX ) * nvertex );
        memcpy( svlist, vlist, sizeof( VERTEX ) * nvertex );
        snvertex = nvertex;
    }
    if( mode & MODE_SIDEDEF )
    {
        if( ssidelist ) memfree( ssidelist );
        ssidelist = (SIDEDEFM*) memget( sizeof( SIDEDEFM ) * nside );
        memcpy( ssidelist, sidelist, sizeof( SIDEDEFM ) * nside );
        snside = nside;
    }
    if( mode & MODE_SECTOR )
    {
        if( sseclist ) memfree( sseclist );
        sseclist = (SECTOR*) memget( sizeof( SECTOR ) * nsector );
        memcpy( sseclist, sectorlist, sizeof( SECTOR ) * nsector );
        snsector = nsector;
    }
    if( mode & MODE_THING )
    {
        if( sthinglist ) memfree( sthinglist );
        sthinglist = (THING*) memget( sizeof( THING ) * nthing );
        memcpy( sthinglist, thinglist, sizeof( THING ) * nthing );
        snthing = nthing;
    }
}

void obj_pop( short mode )
{
    if( mode & MODE_LINE && snline != -1 )
    {
        nline = snline;
        if( slinelist )
        {
            memcpy( linelist, slinelist, sizeof( LINEDEF ) * nline );
            memfree( slinelist );
            slinelist = NULL;
        }
        snline = -1;
    }
    if( mode & MODE_SECTOR && snsector != -1 )
    {
        nsector = snsector;
        if( sseclist )
        {
            memcpy( sectorlist, sseclist, sizeof( SECTOR ) * nsector );
            memfree( sseclist );
            sseclist = NULL;
        }
        snsector = -1;
    }
    if( mode & MODE_VERTEX && snvertex != -1 )
    {
        nvertex = snvertex;
        if( svlist )
        {
            memcpy( vlist, svlist, sizeof( VERTEX ) * nvertex );
            memfree( svlist );
            svlist = NULL;
        }
        snvertex = -1;
    }
    if( mode & MODE_SIDEDEF && snside != -1 )
    {
        nside = snside;
        if( ssidelist )
        {
            memcpy( sidelist, ssidelist, sizeof( SIDEDEFM ) * nside );
            memfree( ssidelist );
            ssidelist = NULL;
        }
        snside = -1;
    }
    if( mode & MODE_THING && snthing != -1 )
    {
        nthing = snthing;
        if( sthinglist )
        {
            memcpy( thinglist, sthinglist, sizeof( THING ) * nthing );
            memfree( sthinglist );
            sthinglist = NULL;
        }
        snthing = -1;
    }
}

static short undo_mapchanges = -2, undo_madechanges = -2;

void undo_init( void )
/*-------------------------------------------------------------------------*
 * Saves all the objects.
 *-------------------------------------------------------------------------*/
{
    undo_mapchanges = undo_madechanges = -1;

    obj_push( MODE_ALL );
}

void undo_setvars( void )
/*-------------------------------------------------------------------------*
 * Set variables for undo..
 *-------------------------------------------------------------------------*/
{
    undo_mapchanges = nMapChanges;
    undo_madechanges = nMadeChanges;
}

void undo_undo( void )
/*-------------------------------------------------------------------------*
 * Undoooooooooooooo
 *-------------------------------------------------------------------------*/
{
    if( undo_mapchanges == -1 )
    {
        obj_pop( MODE_ALL );
        post_fullupdate();
    }
    else if( undo_mapchanges == nMapChanges && undo_madechanges == nMadeChanges )
    {
        obj_pop( MODE_ALL );
        post_fullupdate();
    }
    else
    {
        message_box( MB_OK, st_dcksays, "Can't Undo." );
    }

    undo_mapchanges = -2;
}

int obj_flip( Object& objCursor, int iDirection )
// flips (mirrors) the marked objects (vertices!)
{
    manip_objtype = editmode;

    short centerX, centerY;
    bool unmarkobj = FALSE;
    int i, nflipped, m;
    LINEDEF *ldp;

    if( objCursor.IsValid() && !obj_ismarked( objCursor ) )
        obj_mark( objCursor ), unmarkobj = 1;

    m_marklist = (uchar*) memget( MAXOBJECT );
    memset( m_marklist, 0, MAXOBJECT );
    char * m_linelist = new char[MAXOBJECT];
    memset( m_linelist, 0, MAXOBJECT );

    manip_objtype = editmode;

    if( manip_objtype != MODE_THING )
    {
        /* Mark all the vertices .. */
        switch( manip_objtype )
        {
        case MODE_LINE:
            memcpy( m_marklist, marklist, sizeof marklist );
            for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
            {
                if( obj_ismarked( i ) )
                {
                    m_marklist[ldp->vfrom] = 1;
                    m_marklist[ldp->vend] = 1;
                    m_linelist[i] = 1;
                }
            }
            break;
        case MODE_SECTOR:
            for( i = 0, ldp = linelist; i < nline; i++, ldp++ )
            {
                m = ldp->rsdef;
                if( m >= 0 && obj_ismarked( sidelist[m].sector ) )
                {
                    m_marklist[ldp->vfrom] = 1;
                    m_marklist[ldp->vend] = 1;
                    m_linelist[i] = 1;
                }
                m = ldp->lsdef;
                if( m >= 0 && obj_ismarked( sidelist[m].sector ) )
                {
                    m_marklist[ldp->vfrom] = 1;
                    m_marklist[ldp->vend] = 1;
                    m_linelist[i] = 1;
                }
            }
            break;
        }
    }
    else
    {
        // do things
        memcpy( m_marklist, marklist, sizeof marklist );
    }

    int     minx = 32000, maxx = -32000,
            miny = 32000, maxy = -32000;
    THING   *t;
    VERTEX  *v;

    bool fNoneMarked = TRUE;

    if( manip_objtype == MODE_THING )
    {
        for( i = 0; i < MAXOBJECT; i++ )
        {
            if( m_marklist[i] )
            {
                t = &thinglist[i];
                if( t->x < minx )   minx = t->x;
                if( t->x > maxx )   maxx = t->x;
                if( t->y < miny )   miny = t->y;
                if( t->y > maxy )   maxy = t->y;
                fNoneMarked = FALSE;
            }
        }
    }
    else
    {
        for( i = 0; i < MAXOBJECT; i++ )
        {
            if( m_marklist[i] )
            {
                v = &vlist[i];
                if( v->x < minx )   minx = v->x;
                if( v->x > maxx )   maxx = v->x;
                if( v->y < miny )   miny = v->y;
                if( v->y > maxy )   maxy = v->y;
                fNoneMarked = FALSE;
            }
        }
    }

    if( fNoneMarked )
        goto Done;

    centerX = (maxx+minx) / 2;
    centerY = (maxy+miny) / 2;

    undo_init();

    // now flip the vertices/things
    int x, y;
    for( i = 0; i < MAXOBJECT; i++ )
    {
        if( m_marklist[i] )
        {
            if( editmode == MODE_THING )
            {
                if( !iDirection )
                {
                    x = thinglist[i].x - centerX;
                    thinglist[i].x = centerX - x;
                }
                else
                {
                    y = thinglist[i].y - centerY;
                    thinglist[i].y = centerY - y;
                }
            }
            else
            {
                if( !iDirection )
                {
                    x = vlist[i].x - centerX;
                    vlist[i].x = centerX - x;
                }
                else
                {
                    y = vlist[i].y - centerY;
                    vlist[i].y = centerY - y;
                }
            }
        }
    }

    // now flip the lines
    if( editmode != MODE_THING )
    {
        for( i = 0; i < MAXOBJECT; i++ )
        {
            if( m_linelist[i] )
            {
                int v = linelist[i].vend;
                linelist[i].vend = linelist[i].vfrom;
                linelist[i].vfrom = v;
            }
        }
    }

    if( editmode != MODE_THING )
        nMapChanges++;
    nMadeChanges++;
    post_fullupdate();

    undo_setvars();

Done:
    memfree( m_marklist );
    memfree( m_linelist );

    if( unmarkobj )
        obj_unmark( objCursor );

    return nflipped;
}

Object& ObjEnum::Start( int iObjectType, int iStartAt )
{
    iNumber = iStartAt;
    obj.iType = iObjectType;

    switch( iObjectType )
    {
    case MODE_LINE:
        iMax = nline;
        break;
    case MODE_SECTOR:
        iMax = nsector;
        break;
    case MODE_THING:
        iMax = nthing;
        break;
    case MODE_VERTEX:
        iMax = nvertex;
        break;
    case MODE_SIDEDEF:
        iMax = nside;
        break;
    }

    return Next();
}

Object& ObjEnum::Next()
{
    if( iNumber == iMax )
    {
        obj.SetInvalid();
    }
    else
    {
        obj.iNumber = iNumber++;
    }
    return obj;
}

void ObjEnum::PauseOne()
{
    --iNumber;
    --iMax;
}

char * obj_toggleflags( Object& objCursor, uchar ch )
/*-------------------------------------------------------------------------*
 * toggles the flags of the selected objects, and returns a string saying
 * what was done.
 *-------------------------------------------------------------------------*/
{
    short   i, unmarkobj = 0;
    LINEDEF *ldp = NULL;
    THING   *thingp = NULL;
    unint   uFlagValue, nSet, nMarked;
    static  char szResult[128];
    bool    _fFullUpdate = FALSE;
    char    *pszFlagStr = NULL, *pszSetStr = NULL;

    if( editmode == MODE_LINE )
    {
        switch( ch )
        {
        case 'I':   // impassable
            uFlagValue = LA_IMPASS;
            pszFlagStr = "Impassable";
            _fFullUpdate = TRUE;
            break;
        case 'M':   // monster block
            uFlagValue = LA_MBLOCKER;
            pszFlagStr = "Monster Block";
            _fFullUpdate = TRUE;
            break;
        case '@':   // 2-sided ("shift 2")
            uFlagValue = LA_TSIDE;
            pszFlagStr = "2-Sided";
            _fFullUpdate = TRUE;
            break;
        case 'U':   // upper unpegged
            uFlagValue = LA_UUNPEGGED;
            pszFlagStr = "Upper Unpegged";
            break;
        case 'L':   // lower unpegged
            uFlagValue = LA_LUNPEGGED;
            pszFlagStr = "Lower Unpegged";
            break;
        case 'S':   // secret
            uFlagValue = LA_SECRET;
            pszFlagStr = "Secret";
            break;
        case 'D':   // sound
            uFlagValue = LA_SBLOCK;
            pszFlagStr = "Sound Block";
            _fFullUpdate = TRUE;
            break;
        case 'N':   // not on map
            uFlagValue = LA_NOTONMAP;
            pszFlagStr = "Not on Map";
            break;
        case 'A':   // auto on map
            uFlagValue = LA_AUTOONMAP;
            pszFlagStr = "Always on Map";
            break;
        case 'R':
            if( fHexen )
            {
                uFlagValue = LA_REPEAT;
                pszFlagStr = "Special Repeatable";
                break;
            }
        default:
            return NULL;
        }
    }
    else if( editmode == MODE_THING )
    {
        switch( ch )
        {
        case '!':   // skill 1-2
        case '@':
            uFlagValue = TA_SKILL12;
            pszFlagStr = "Skill 1-2";
            break;
        case '#':   // skill 3
            uFlagValue = TA_SKILL3;
            pszFlagStr = "Skill 3";
            break;
        case '$':   // skill 4-5
        case '%':
            uFlagValue = TA_SKILL45;
            pszFlagStr = "Skill 4-5";
            break;
        case 'A':   // deaf
            uFlagValue = TA_DEAF;
            pszFlagStr = "Ambush (Deaf)";
            if( fHexen )
            break;
        case 'C':
            if( !fHexen )
                break;
            uFlagValue = TA_CLERIC;
            pszFlagStr = "Cleric";
            break;
        case 'M':
            if( fHexen )
            {
                uFlagValue = TA_MAGE;
                pszFlagStr = "Mage";
            }
            else
            {
                uFlagValue = TA_MPLAYER;
                pszFlagStr = "Multi-Player";
            }
            break;
        case 'F':
            if( !fHexen )
                break;
            uFlagValue = TA_FIGHTER;
            pszFlagStr = "Fighter";
            break;
        case 'D':
            if( !fHexen )
                break;
            uFlagValue = TA_GDEATHMATCH;
            pszFlagStr = "Deathmatch";
            break;
        case 'T':
            if( !fHexen )
                break;
            uFlagValue = TA_DORMANT;
            pszFlagStr = "Dormant";
            break;
        case 'S':
            if( !fHexen )
                break;
            uFlagValue = TA_GSINGLE;
            pszFlagStr = "Single Player";
            break;
        case 'P':
            if( !fHexen )
                break;
            uFlagValue = TA_GCOOP;
            pszFlagStr = "Cooperative";
            break;
        default:
            return NULL;
        }
    }
    else
        return NULL;

    if( pszFlagStr == NULL )
        return NULL;

    if( objCursor.IsValid() && !obj_ismarked( objCursor ) )
        obj_mark( objCursor ), unmarkobj = 1;

    nMarked = obj_nmarked();

    if( !nMarked )
        return "No objects marked.";

    ObjEnum oe;

    if( editmode == MODE_LINE )
    {
        // first check to see if they're >= 90% set
        nSet = 0;
        Object obj = oe.Start( MODE_LINE );
        while( obj.IsValid() )
        {
            if( obj_ismarked( obj ) )
            {
                ldp = (LINEDEF*) obj;
                if( ldp->attribs & uFlagValue )
                    nSet++;
            }

            obj = oe.Next();
        }

        if( nSet > ((nMarked * 85) / 100) )
        {
            Object obj = oe.Start( MODE_LINE );
            while( obj.IsValid() )
            {
                if( obj_ismarked( obj ) )
                    ((LINEDEF*)obj)->attribs &= ~uFlagValue;
                obj = oe.Next();
            }
            pszSetStr = "removed";
        }
        else
        {
            // no, fewer than 90% are set: so set the value.
            Object obj = oe.Start( MODE_LINE );
            while( obj.IsValid() )
            {
                if( obj_ismarked( obj ) )
                    ((LINEDEF*)obj)->attribs |= uFlagValue;
                obj = oe.Next();
            }
            pszSetStr = "set";
        }
    }
    else if( editmode == MODE_THING )
    {
        nSet = 0;
        Object obj = oe.Start( MODE_THING );
        while( obj.IsValid() )
        {
            if( obj_ismarked( obj ) )
            {
                thingp = (THING*) obj;
                if( thingp->attribs & uFlagValue )
                    nSet++;
            }

            obj = oe.Next();
        }

        if( nSet > ((nMarked * 85) / 100) )
        {
            // yes, most or all are set: so unset the value.
            Object obj = oe.Start( MODE_THING );
            while( obj.IsValid() )
            {
                if( obj_ismarked( obj ) )
                    ((THING*)obj)->attribs &= ~uFlagValue;
                obj = oe.Next();
            }
            pszSetStr = "removed";
        }
        else
        {
            // no, fewer than 90% are set: so set the value.
            Object obj = oe.Start( MODE_THING );
            while( obj.IsValid() )
            {
                if( obj_ismarked( obj ) )
                    ((THING*)obj)->attribs |= uFlagValue;
                obj = oe.Next();
            }
            pszSetStr = "set";
        }
    }

    if( unmarkobj )
        obj_unmark( objCursor );

    nMadeChanges ++;
    if( _fFullUpdate )
        post_fullupdate();

    sprintf( szResult, "The %s flag was %s on %d object(s)",
             pszFlagStr, pszSetStr, nMarked );

    return szResult;
}

Object& obj_findtag( Object& objSearch, int iDirection, bool fFirst )
{
    static int iStartAt, iTag, iDone;
    static int searchmode, iLastDirec = -2;
    static int objectmode;
    int i;
    void SetEditMode( short mode );
    static Object obj;

    obj.SetInvalid();

    if( iLastDirec == -2 )
        fFirst = TRUE;

    if( !fFirst && iLastDirec == iDirection && iDone )
        goto NoMoreObjects;

    iLastDirec = iDirection;
    iDone = 0;

    if( fFirst )
    {
        if( !objSearch.IsValid() )
        // ask for a number
        {
            static bool fUseTID = 0;
            Dialog* pDialog = new Dialog;
            pDialog->Create( Point( Centered, Centered ), Size( 120, 40 ),
                    "Enter an ID to search for" );
            new fldString( pDialog, Point( 4, 4 ),
                    25, ftInteger, "ID:", csPromptBeside, 3 )->SetTransferBuf( &iTag );
            if( editmode == MODE_LINE )
            {
                new fldCheckBox( pDialog, Point( 50, 6 ),
                    "Search for TID" )->SetTransferBuf( &fUseTID );
            }
            pDialog->AddDefaultButtons();
            int iRvl = pDialog->Process();
            delete pDialog;

            if( iRvl == daExitNoSave )
                return obj;

            if( editmode == MODE_LINE )
            {
                searchmode = MODE_LINE;
                if( fUseTID )
                {
                    objectmode = MODE_THING;
                }
                else
                {
                    objectmode = MODE_SECTOR;
                }
            }
            else if( editmode == MODE_SECTOR )
            {
                searchmode = MODE_SECTOR;
                objectmode = MODE_LINE;
            }
            else if( editmode == MODE_THING )
            {
                searchmode = MODE_THING;
                objectmode = MODE_LINE;
            }
        }
        else if( objSearch.iType == MODE_LINE )
        {
            if( lineneedstag( linelist[objSearch.iNumber].special ) )
            {
                iTag = lSecTag( &linelist[objSearch.iNumber] );
                searchmode = MODE_SECTOR;
                objectmode = MODE_LINE;
            }
            else if( LineUsesTID( objSearch.iNumber ) )
            {
                iTag = linelist[objSearch.iNumber].arg0;
                searchmode = MODE_THING;
                objectmode = MODE_LINE;
            }
            else
                iTag = 0;
        }
        else if( objSearch.iType == MODE_SECTOR )
        {
            iTag = sectorlist[objSearch.iNumber].sectag;
            searchmode = MODE_LINE;
            objectmode = MODE_SECTOR;
        }
        else if( objSearch.iType == MODE_THING )
        {
            iTag = thinglist[objSearch.iNumber].tid;
            searchmode = MODE_LINE;
            objectmode = MODE_THING;
        }
        else
            return obj;

        if( iTag == 0 )
        {
            message_box( MB_OK, st_dcksays,
                "The object under the cursor does not have a Sector Tag or Item ID." );
            return obj;
        }
    }

    if( searchmode == MODE_LINE )
    {
        LINEDEF *ldp;
        if( fFirst )
            i = iStartAt = (iDirection == 1) ? 0 : nline-1;
        else
            i += iStartAt + iDirection;

        for( ; i >= 0 && i < nline; i += iDirection )
        {
            ldp = &linelist[i];

            if( objectmode == MODE_SECTOR &&
                !(lSecTag( ldp ) == iTag && lineneedstag( ldp->special )) )
                continue;

            if( objectmode == MODE_THING &&
                !(ldp->arg0 == iTag && LineUsesTID( i )) )
                continue;

            // jump to it
            iStartAt = i;
            SetStatusMsg( "View the next object with the '/' key, or the "
                            "previous object with the '\\' key." );

            obj.iType = searchmode;
            obj.iNumber = i;
            return obj;
        }
    }

    if( searchmode == MODE_SECTOR )
    {
        SECTOR *secp;
        if( fFirst )
            i = iStartAt = (iDirection == 1) ? 0 : nsector-1;
        else
            i += iStartAt + iDirection;

        for( ; i >= 0 && i < nsector; i += iDirection )
        {
            secp = &sectorlist[i];
            if( secp->sectag == iTag )
            {
                // jump to it
                iStartAt = i;
                SetStatusMsg( "View the next object with the '/' key, or the "
                              "previous object with the '\\' key." );
                obj.iType = searchmode;
                obj.iNumber = i;
                return obj;
            }
        }
    }

    if( searchmode == MODE_THING )
    {
        THING *thingp;
        if( fFirst )
            i = iStartAt = (iDirection == 1) ? 0 : nthing-1;
        else
            i += iStartAt + iDirection;

        for( ; i >= 0 && i < nthing; i += iDirection )
        {
            thingp = &thinglist[i];
            if( thingp->tid == iTag )
            {
                // jump to it
                iStartAt = i;
                SetStatusMsg( "View the next object with the '/' key, or the "
                              "previous object with the '\\' key." );
                obj.iType = searchmode;
                obj.iNumber = i;
                return obj;
            }
        }
    }


    iDone = 1;

NoMoreObjects:
    message_box( MB_OK, st_dcksays,
        "There are no more matching objects with a tag of '%d'.", iTag );
    if( !fFirst )
    {
        SetStatusMsg( "View the %s object with the %s key.",
            iDirection == 1 ? "previous" : "next", iDirection == 1 ? "\\" : "/" );
    }
    else
    {
        SetStatusMsg( st_blank );
    }
    return obj;
}
