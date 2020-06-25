/*-------------------------------------------------------------------------*
 * PMATRIX.C:                                                              *
 * Functions for creating a polygonal area matrix, and for checking to     *
 * see if a line is completely enclosed in that matrix.                    *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "pmatrix.h"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Module begins                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#define MaxLines        128
#define Swap( a, b )    { short tmp = a; a = b; b = tmp; }
#define byte            uchar
#define until(x)        while( !(x) )

typedef short           shRange[2];

short   x1[MaxLines], x2[MaxLines];
short   y1[MaxLines], y2[MaxLines];
shRange *mtxRange;
byte    Use[MaxLines];
short   NumLines;
short   mtxDepth;
short   mtxMinY, mtxMaxY;

void mtxSetupfromLinedefs( short *llist, short listlen, short sector )
/*-------------------------------------------------------------------------*
 * Sets up the coordinate variables from a series of linedefs .. but only  *
 * on those that use sector.                                               *
 *-------------------------------------------------------------------------*/
{
    short     i, i2 = 0, *lp = llist;
    short   clist[128][2];
    LINEDEF *ldp;

    for( i = 0; i < listlen; i++, lp++ )
    {
        ldp = &linelist[*lp];

        if( ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == sector ||
            ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == sector )
        {
            x1[i2] = vlist[ldp->vfrom].x;
            x2[i2] = vlist[ldp->vend].x;
            y1[i2] = vlist[ldp->vfrom].y;
            y2[i2] = vlist[ldp->vend].y;
            i2 ++;
        }
    }

    x2[i2] = x1[0];
    y2[i2] = y1[0];

    NumLines = listlen;
}

short mtxCheckLinedef( short ldnum )
{
    VERTEX  *v1 = &vlist[linelist[ldnum].vfrom];
    VERTEX  *v2 = &vlist[linelist[ldnum].vend];

    return mtxCheck( v1->x, v1->y, v2->x, v2->y );
}

short mtxCheck( short x1, short y1, short x2, short y2 )
/*-------------------------------------------------------------------------*
 * Checks to see if the line passed is completely within the range matrix. *
 *-------------------------------------------------------------------------*/
{
    y1 -= mtxMinY; y2 -= mtxMinY;

    if( mtxRange == NULL )      /* Oops ! */
        return 0;

    if( y1 < 0 || y1 > mtxDepth || y2 < 0 || y2 > mtxDepth )
        return 0;

    if( x1 >= mtxRange[y1][0] && x1 <= mtxRange[y1][1] &&
        x2 >= mtxRange[y2][0] && x2 <= mtxRange[y2][1] )
        return 1;

    return 0;
}

void mtxDelete( void )
/*-------------------------------------------------------------------------*
 * Frees the matrix memory, if there is any allocated.                     *
 *-------------------------------------------------------------------------*/
{
    if( mtxRange != NULL )
        free( mtxRange );
}

short mtxBuild( void )
/*-------------------------------------------------------------------------*
 * Builds a range matrix of minimum and maximum X values down an array     *
 * of integer pairs from the line information in x1/y1 and x2/y2.          *
 *-------------------------------------------------------------------------*/
{
    short i, j;
    short LNum, RNum;
    short OldY;
    short Min, LMin, RMin;
    byte Done;
    short LgDeltaL, ShDeltaL;
    short CycleL, LgStepL;
    short LgDeltaR, ShDeltaR;
    short CycleR, LgStepR;
    short LineModeL, LineModeR;
    short NeedNewLineL = 0, NeedNewLineR = 0;
    short CurXL, CurYL;
    short CurXR, CurYR;
    short workint, workint2;
    short workint3, workint4;
    char *workptr;

    mtxMinY = 32000;
    mtxMaxY = -32000;

    /* Figure out y-depth of matrix */
    for( i = 0; i <= NumLines; i++ )
    {
        if( y1[i] < mtxMinY )
            mtxMinY = y1[i];
        if( y1[i] > mtxMaxY )
            mtxMaxY = y1[i];

        if( y2[i] < mtxMinY )
            mtxMinY = y2[i];
        if( y2[i] > mtxMaxY )
            mtxMaxY = y2[i];
    }

    mtxDepth = abs( mtxMaxY - mtxMinY );
    /* Try to allocate the matrix */
    mtxRange = malloc( sizeof( shRange ) * (mtxDepth + 2) );

    if( mtxRange == NULL )
    {
        message_box( MB_OK, "Matrix Error",
                     "Not enough memory to create sector reference transformation\n"
                     "matrix (This is not fatal; please see the documentation.)" );
        return 0;
    }

    /* Run through y values and make them relative to the matrix depth */
    for( i = 0; i <= NumLines; i++ )
    {
        y1[i] -= mtxMinY;
        y2[i] -= mtxMinY;
    }

    /* Initialize range matrix */
    for( i = 0; i < mtxDepth; i++ )
    {
        mtxRange[i][0] = 32000;
        mtxRange[i][1] = -32000;
    }

    for( i = 0; i < NumLines; i++ )
    {
        if( y1[i] == y2[i] )
            Use[i] = 0;
        else
            Use[i] = 1;

        if( y2[i] < y1[i] )
        {
            Swap( y1[i], y2[i] );
            Swap( x1[i], x2[i] );
        }

    }

    Min = y1[0];
    LNum = 0;

    for( i = 0; i < NumLines; i++ )
        if( Use[i] )
            if( y1[i] <= Min )
            {
                LNum = i;
                Min = y1[i];
            }

    Min = y1[0];
    RNum = 0;

    for( i = 0; i < NumLines; i++ )
        if( Use[i] )
            if( y1[i] <= Min )
                if( i != LNum )
                {
                    RNum = i;
                    Min = y1[i];
                }

    if( x1[LNum] > x1[RNum] || x2[LNum] > x2[RNum] )
        Swap( LNum, RNum );

    CurXL = x1[LNum];
    CurYL = y1[LNum];
    CurXR = x1[RNum];
    CurYR = y1[RNum];

    LgDeltaL = x2[LNum] - CurXL;
    ShDeltaL = y2[LNum] - CurYL;
    LgDeltaR = x2[RNum] - CurXR;
    ShDeltaR = y2[RNum] - CurYR;

    if( LgDeltaL < 0 )
    {
        LgDeltaL = -LgDeltaL;
        LgStepL = -1;
    }
    else
        LgStepL = 1;

    if( LgDeltaR < 0 )
    {
        LgDeltaR = -LgDeltaR;
        LgStepR = -1;
    }
    else
        LgStepR = 1;

    if( ShDeltaL < LgDeltaL )
    {
        CycleL = LgDeltaL >> 1;
        LineModeL = 0;
    }
    else
    {
        CycleL = ShDeltaL >> 1;
        Swap( LgDeltaL, ShDeltaL );
        LgStepL = 1;
        LineModeL = 1;
    }

    if( ShDeltaR < LgDeltaR )
    {
        CycleR = LgDeltaR >> 1;
        LineModeR = 0;
    }
    else
    {
        CycleR = ShDeltaR >> 1;
        Swap( LgDeltaR, ShDeltaR );
        LgStepR = 1;
        LineModeR = 1;
    }

    Done = 0;

    do  {

        OldY = CurYL;

        if( LineModeL == 0 )
        {
            workint =  y2[LNum];
            workint2 = x2[LNum];

            do
            {
                CycleL += ShDeltaL;

                if( CycleL > LgDeltaL )
                {
                    CycleL -= LgDeltaL;
                    CurYL ++;
                }

                CurXL += LgStepL;


                if( CurXL == workint2 )
                {
                    NeedNewLineL = 1;
                    CurYL = workint;
                    OldY = CurYL + 1;
                }

            } until( OldY != CurYL );
        }
        else
        {
            workint =  y2[LNum];
            workint2 = x2[LNum];

            do
            {
                CycleL += ShDeltaL;

                if( CycleL > LgDeltaL )
                {
                    CycleL -= LgDeltaL;
                    CurXL ++;
                }

                CurYL += LgStepL;


                if( CurYL == workint )
                {
                    NeedNewLineL = 1;
                    CurXL = workint2;
                    OldY = CurYL + 1;
                }
            } until( OldY != CurYL );
        }


        if( NeedNewLineL == 1 )
        {
            NeedNewLineL = 0;
            Use[LNum] = 0;
            Done = 1;

            for( i = 0; i < NumLines; i++ )
                if( Use[i] )
                    if( (x1[i] == CurXL && y1[i] == CurYL) || (x2[i] == CurXL && y2[i] == CurYL) )
                    {


                        LNum = i;
                        Done = 0;

                        CurXL = x1[LNum];
                        CurYL = y1[LNum];

                        LgDeltaL = x2[LNum]-CurXL;
                        ShDeltaL = y2[LNum]-CurYL;

                        if( LgDeltaL < 0 )
                        {
                            LgDeltaL = -LgDeltaL;
                            LgStepL = -1;
                        }
                        else
                            LgStepL = 1;

                        if( ShDeltaL < LgDeltaL )
                        {
                            CycleL = LgDeltaL >> 1;
                            LineModeL = 0;
                        }
                        else
                        {
                            CycleL = ShDeltaL >> 1;
                            Swap( LgDeltaL, ShDeltaL );
                            LgStepL = 1;
                            LineModeL = 1;
                        }
                    }
        }


        OldY = CurYR;

        if( LineModeR == 0 )
        {
            do  {
                CycleR += ShDeltaR;

                if( CycleR > LgDeltaR )
                {
                    CycleR -= LgDeltaR;
                    CurYR ++;
                }

                CurXR += LgStepR;


                if( CurXR == x2[RNum] )
                {
                    NeedNewLineR = 1;
                    CurYR = y2[RNum];
                    OldY = CurYR+1;
                }
            } until( OldY != CurYR );
        }
        else
        {
            do  {
                CycleR += ShDeltaR;

                if( CycleR > LgDeltaR )
                {
                    CycleR -= LgDeltaR;
                    CurXR ++;
                }

                CurYR += LgStepR;


                if( CurYR == y2[RNum] )
                {
                    NeedNewLineR = 1;
                    CurXR = x2[RNum];
                    OldY = CurYR + 1;
                }
            } until( OldY != CurYR );
        }


        if( NeedNewLineR  )
        {
            NeedNewLineR = 0;
            Use[RNum] = 0;
            Done = 1;

            for( i = 0; i < NumLines; i++ )
                if( Use[i] )
                    if( (x1[i] == CurXR && y1[i] == CurYR) || (x2[i] == CurXR && y2[i] == CurYR) )
                    {


                        RNum = i;
                        Done = 0;

                        CurXR = x1[RNum];
                        CurYR = y1[RNum];

                        LgDeltaR = x2[RNum] - CurXR;
                        ShDeltaR = y2[RNum] - CurYR;

                        if( LgDeltaR < 0 )
                        {
                            LgDeltaR = -LgDeltaR;
                            LgStepR = -1;
                        }
                        else
                            LgStepR = 1;

                        if( ShDeltaR < LgDeltaR )
                        {
                            CycleR = LgDeltaR >> 1;
                            LineModeR = 0;
                        }
                        else
                        {
                            CycleR = ShDeltaR >> 1;
                            Swap( LgDeltaR, ShDeltaR );
                            LgStepR = 1;
                            LineModeR = 1;
                        }
                    }
        }

        if( !Done )
        {
            /* Store it in the matrix */
            if( CurXL < mtxRange[CurYL][0] )
                mtxRange[CurYL][0] = CurXL;
            if( CurXL > mtxRange[CurYL][1] )
                mtxRange[CurYL][1] = CurXL;

            if( CurXR < mtxRange[CurYL][0] )
                mtxRange[CurYL][0] = CurXR;
            if( CurXR > mtxRange[CurYL][1] )
                mtxRange[CurYL][1] = CurXR;
        }

    } until( Done );

    return 1;
}

