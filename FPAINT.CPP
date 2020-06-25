/*-------------------------------------------------------------------------*
 * FPAINT.C:                                                               *
 * Functions for the format painter.  Woowowowow.                          *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

/* Paint function: takes dest, source. */
typedef void (* PFUNC)( PTR, PTR, short );

/* Function prototypes */
void _p_linedef( LINEDEF *, LINEDEF *, short );
void _p_thing( THING *, THING *, short );
void _p_sector( SECTOR *, SECTOR *, short );

#pragma warn -sus

#define L_ATTRONLY      0
#define L_TEXTURES      1
#define L_TAGANDTYPE    2
#define L_TAG           3

#define L_SPECIAL       2

static uchar *hex_lcombos[] =
{
    "Attributes only",
    "Textures only",
    "Special and Arguments",
    NULL
};

static uchar *doom_lcombos[] =
{
    "Attributes only",
    "Textures only",
    "Sector Tag and Action Type",
    "Sector Tag only",
    NULL
};

#define S_APPONLY       0
#define S_FHEIGHT       1
#define S_CHEIGHT       2
#define S_BOTHHEIGHT    3

static uchar *scombos[] =
{
    "Appearance only",
    "Floor height",
    "Ceiling height",
    "Both heights",
    NULL
};

enum
{
    T_ALL,
    T_ATTRONLY,
    T_ANGLEONLY,
    T_BOTH
};

static uchar *tcombos[] =
{
    " Everything           ",
    " Attributes only",
    " Angle only",
    " Attributes and Angle ",
    NULL
};

struct
{
    short     objtype;
    void    * addrArray;
    short   * addrCount;
    void    * addrComboList;
    size_t  elemsize;
    PFUNC   fp;
    char    *mnhelp;

} ptable[] =
{
    { MODE_LINE,      (PTR) &linelist,      &nline,     NULL,       sizeof( LINEDEF ),  (PFUNC) _p_linedef, "MN_FPAINT_LINE" },
    { MODE_THING,     (PTR) &thinglist,     &nthing,    &tcombos,   sizeof( THING ),    (PFUNC) _p_thing, "MN_FPAINT_THING" },
    { MODE_SECTOR,    (PTR) &sectorlist,    &nsector,   &scombos,   sizeof( SECTOR ),   (PFUNC) _p_sector, "MN_FPAINT_SECTOR" },
    { -1,             NULL,                 NULL,       NULL,       0,                  NULL }
};

static short fixtex;      /* for sectors where the heights are changed */
static short _fp_curobj;  /* current object */

#pragma warn +sus

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

ushort fpainter( short objtype, short source_obj )
/*-------------------------------------------------------------------------*
 * Given source_obj, calls the appropriate paint function for each marked  *
 * object with source_obj's data and the marked object's data.             *
 *-------------------------------------------------------------------------*/
{
    byte    *aptr;      /* data pointer to the right object list */
    byte    *sptr;      /* Source data pointer */
    const char **carray;   /* combo array */
    char    *help;
    PFUNC   fp;         /* function pointer to the paint function */
    size_t  esize;      /* the element size for each member of the array */
    short     i, copywhat;
    short   n, npaint = 0;

    for( i = 0; ptable[i] .objtype != -1 ; i++ )
    {
        /* Search for the right data in the table */
        if( ptable[i] .objtype == objtype )
        {
            /* This is it. */
            aptr = ((byte **) (ptable[i] .addrArray))[0];
            n = ptable[i] .addrCount[0];
            esize = ptable[i] .elemsize;
            help = ptable[i] .mnhelp;
            carray = (const char **) (ptable[i] .addrComboList);
            fp = ptable[i] .fp;

            /* Find the source pointer */
            sptr = aptr + (esize * source_obj);
            break;
        }
    }

    if( carray == NULL && objtype == MODE_LINE )
    {
        carray = (const char**) (fHexen ? hex_lcombos : doom_lcombos);
    }

    if( ptable[i] .objtype == -1 )      /* can't do dat */
        return 0;

    copywhat = MenuPopArray( 0, " Copy what? ", help, carray );

    if( copywhat == -1 )
        return 0;

    fixtex = -1;    /* this is its initial value. */

    /* Out.  Now loop through the mark list and get me some data. */
    for( i = 0; i < n; i++, aptr += esize )
    {
        if( obj_ismarked( i ) )
        {
            _fp_curobj = i;
            ++npaint;       /* one more painted */
            (*fp)( aptr, sptr, copywhat );        /* call the function */
        }
    }

    if( npaint )
        nMadeChanges ++;        /* Made changes ! */

    return npaint;
}

#define LNOCOPY (LA_IMPASS | LA_TSIDE)

static void _p_linedef( LINEDEF *dest, LINEDEF *src, short copywhat )
/*-------------------------------------------------------------------------*
 * Pastes a linedef's attributes.                                          *
 *-------------------------------------------------------------------------*/
{
    SIDEDEFM    *sdp1, *sdp2;

    /* Remove the important ones */
    if( copywhat == L_ATTRONLY )
        dest->attribs |= (src->attribs & ~LNOCOPY);

    if( !fHexen )
    {
        if( copywhat == L_TAG || copywhat == L_TAGANDTYPE )
            dest->sectag = src->sectag;

        if( copywhat == L_TAGANDTYPE )
            dest->special = src->special;
    }
    else if( copywhat == L_SPECIAL )
    {
        dest->special = src->special;
        dest->arg0 = src->arg0;
        dest->arg1 = src->arg1;
        dest->arg2 = src->arg2;
        dest->arg3 = src->arg3;
        dest->arg4 = src->arg4;
    }

    if( copywhat == L_TEXTURES )
    {
        /* This is mostly to do with sidedefs. */
        if( dest->rsdef >= 0 && src->rsdef >= 0 )
        {
            sdp1 = &sidelist[dest->rsdef];
            sdp2 = &sidelist[src->rsdef];

            if( sdp1->ftxidx && sdp2->ftxidx )   sdp1->ftxidx = sdp2->ftxidx;
            if( sdp1->utxidx && sdp2->utxidx )   sdp1->utxidx = sdp2->utxidx;
            if( sdp1->ltxidx && sdp2->ltxidx )   sdp1->ltxidx = sdp2->ltxidx;
        }

        if( dest->lsdef >= 0 && src->lsdef >= 0 )
        {
            sdp1 = &sidelist[dest->lsdef];
            sdp2 = &sidelist[src->lsdef];

            if( sdp1->ftxidx && sdp2->ftxidx )   sdp1->ftxidx = sdp2->ftxidx;
            if( sdp1->utxidx && sdp2->utxidx )   sdp1->utxidx = sdp2->utxidx;
            if( sdp1->ltxidx && sdp2->ltxidx )   sdp1->ltxidx = sdp2->ltxidx;
        }
    }
}

static void _p_thing( THING *dest, THING *src, short copywhat )
/*-------------------------------------------------------------------------*
 * Pastes a thing's things.                                                *
 *-------------------------------------------------------------------------*/
{
    if( copywhat == T_ALL )
    {
        /* NOT THE X AND Y COORDS YOU GIT !! */
        dest->angle = src->angle;       /* oh boy. */
        dest->attribs = src->attribs;       /* oh boy. */
        dest->type = src->type;
    }
    if( copywhat == T_ATTRONLY || copywhat == T_BOTH )
        dest->attribs = src->attribs;       /* oh boy. */
    if( copywhat == T_ANGLEONLY || copywhat == T_BOTH )
        dest->angle = src->angle;       /* oh boy. */
}

static void _p_sector( SECTOR *dest, SECTOR *src, short copywhat )
/*-------------------------------------------------------------------------*
 * Pastes a sector's attributes.                                           *
 *-------------------------------------------------------------------------*/
{
    if( copywhat == S_APPONLY )
    {
        wncpy( dest->ftxname, src->ftxname );
        wncpy( dest->ctxname, src->ctxname );
        dest->light = src->light;
        dest->type = src->type;
        return;
    }

    if( copywhat == S_FHEIGHT || copywhat == S_BOTHHEIGHT )
        dest->fheight = src->fheight;
    if( copywhat == S_CHEIGHT || copywhat == S_BOTHHEIGHT )
        dest->cheight = src->cheight;

    if( fixtex == -1 )
    {
        fixtex = (message_box( MB_YESNO, st_dcksays,
                   "Should I automatically fix textures\n"
                   "where required?" ) == MB_YES) ? 1 : 0;
    }

    if( fixtex == 1 )  /* aha ! */
        sect_fixtex( _fp_curobj, 0 );
}
