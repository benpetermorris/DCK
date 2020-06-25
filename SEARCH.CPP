/*-------------------------------------------------------------------------*
 * SEARCH.C                                                                *
 * Functions for starting, continuing and exiting searches and replaces.   *
 *-------------------------------------------------------------------------*/

#if 0

#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

enum
{
    SF_REPLACE = 0x01,              /* Doing a replace? */
    SF_ALWAYS = 0x02,               /* ALWAYS replace ? */
    SF_MARKED = 0x04,               /* Search marked only? */
    SF_MARKFOUND = 0x08,            /* Mark the found objects? */
    SF_DISPONLY = 0x10              /* Only display found objects? */
};

#define nMaxFind    2048

typedef struct
{
    short   *objects;         /* the found objects */
    short   numfound;
    void    *srchstruct;      /* To pass to the search function */
    void    *repstruct;       /* To pass the replace function */
    short   curidx;           /* index to objects[] array */
    short   replace_percent;  /* Replace this many of occurances (auto-search) */
    STRTABLE *obj_stbl;       /* synchronous with objects[] */
    DIALOG  *srchdlg;         /* search, replace data dialogs */
    DIALOG  *repdlg;
    short   new;              /* re-init vars on startup? */
    short   mode;             /* mode (search, replace) */

    /* options */
    uchar   always, marked, markfound, disponly, stopfirst;

} SearchBlk;

SearchBlk   lineblk, thingblk, secblk;
SearchBlk   *sblk;      /* pointer to currently active block */
short       stblidx;    /* search table index (currently active) */

#define nMaxDataDesc    4
#define nMaxDescLine    55

static  FIELD   *srchdesc[nMaxDataDesc],
                *repdesc[nMaxDataDesc];
static  char    srchbufs[nMaxDataDesc][nMaxDescLine];
static  char    repbufs[nMaxDataDesc][nMaxDescLine];
static  DIALOG  *sdlgp;

struct
{
    short       editmode;
    SearchBlk   *searchp;
    size_t      strucsize;

} search_tbl[] =
{
    { MODE_LINE,    &lineblk,   sizeof( LDIALOG ) },
    { MODE_THING,   &thingblk,  0 },
    { MODE_SECTOR,  &secblk,    0 },
    { -1,           NULL }
};

/* Local prototypes */
static void _ld_search( void );
static void _ld_replace( void );

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

void srch_initstruc( void *struc )
{
    LDIALOG *ldata;

    switch( editmode )
    {
    case MODE_LINE:
        ldata = struc;
        ldata->a_impass = ldata->a_mblocker = ldata->a_tside = ldata->a_uunpegged = 2;
        ldata->a_notonmap = ldata->a_sblock = ldata->a_secret = ldata->a_lunpegged = 2;
        ldata->a_autoonmap = 2;
        *(ldata->typestr) = 0;
        ldata->type = -1;
        ldata->sectag = -1;
        ldata->sd1_utxidx = -2;
        ldata->sd1_utx[0] = 0;
        ldata->sd1_ftx[0] = 0;
        ldata->sd1_ltx[0] = 0;
        ldata->sd2_utx[0] = 0;
        ldata->sd2_ftx[0] = 0;
        ldata->sd2_ltx[0] = 0;
        ldata->sd1_sector = -1;
        ldata->sd1_xofs = -1;
        ldata->sd1_yofs = -1;
        ldata->sd2_sector = -1;
        ldata->sd2_xofs = -1;
        ldata->sd2_yofs = -1;
        ldata->sd2_valid = 0;
        break;
    }
}

#define _dconcat( buf )     { if( buf[0] )  strcat( buf, "; " ); }

static short addattrib( uchar *buf, uchar attrib, uchar *s, short addcomma )
{
    if( attrib == 2 )
        return 0;

    if( addcomma )
        strcat( buf, ", " );

    sprintf( buf + strlen( buf ), "%s%s", attrib == 0 ? "not " : "", s );

    return 1;
}

void getdatadesc( uchar *buf, void* data )
/*-------------------------------------------------------------------------*
 * Gets description for the data in "data", such as :
 *
 * line type #02, attributes of [etc etc].
 *-------------------------------------------------------------------------*/
{
    LDIALOG *ldata = data;
    short   a_got = 0;  /* done any attributes yet? */

    *buf = 0;

    switch( editmode )
    {
    case MODE_LINE:
/*-------------------------------------------------------------------------*
 * template:
 * [line type n]
 * [[;]sector tag n]
 * [[;]attributes: s, s, ..]
 * [[;]side n [upper 's'] [full 's'] [lower 's'] [sector] [xofs] [yofs]]
 *-------------------------------------------------------------------------*/
        if( ldata->type != -1 )
            sprintf( buf, "type: %s", SpcTypeToStr( ldata->type ) );
        if( ldata->sectag != -1 )
        {
            _dconcat( buf );
            sprintf( buf+strlen( buf ), "sector tag #%d", ldata->sectag );
        }
        if( ldata->a_impass != 2 || ldata->a_mblocker != 2 || ldata->a_tside != 2 ||
            ldata->a_uunpegged != 2 || ldata->a_notonmap != 2 || ldata->a_sblock != 2 ||
            ldata->a_secret != 2 || ldata->a_lunpegged != 2 || ldata->a_autoonmap != 2 )
        {
            _dconcat( buf );
            strcat( buf, "attributes: " );

            a_got = addattrib( buf, ldata->a_impass, "impassable", a_got );
            a_got = addattrib( buf, ldata->a_mblocker, "monster blocker", a_got );
            a_got = addattrib( buf, ldata->a_tside, "2-sided", a_got );
            a_got = addattrib( buf, ldata->a_secret, "secret", a_got );
            a_got = addattrib( buf, ldata->a_sblock, "sound blocker", a_got );
            a_got = addattrib( buf, ldata->a_uunpegged, "upper unpeg", a_got );
            a_got = addattrib( buf, ldata->a_lunpegged, "lower unpeg", a_got );
            a_got = addattrib( buf, ldata->a_notonmap, "never on map", a_got );
            a_got = addattrib( buf, ldata->a_autoonmap, "always on map", a_got );
        }
        if( ldata->sd1_utxidx != -1 || ldata->sd1_ltxidx != -1 || ldata->sd1_ftxidx != -1 ||
            ldata->sd1_sector != -1 || ldata->sd1_xofs != -1 || ldata->sd1_yofs != -1 )
        {
            a_got = 0;

            _dconcat( buf );
            strcat( buf, "side 1: " );

            if( ldata->sd1_utxidx != -1 )
                sprintf( buf+strlen( buf ), "upper '%s'", WN2Str( texlist[ldata->sd1_utxidx].name ) ), a_got = 1;
            if( ldata->sd1_ftxidx != -1 )
                sprintf( buf+strlen( buf ), "%sfull '%s'", a_got?", ":"", WN2Str( texlist[ldata->sd1_ftxidx].name ) ), a_got = 1;
            if( ldata->sd1_ltxidx != -1 )
                sprintf( buf+strlen( buf ), "%slower '%s'", a_got?", ":"", WN2Str( texlist[ldata->sd1_ltxidx].name ) ), a_got = 1;
            if( ldata->sd1_xofs != -1 )
                sprintf( buf+strlen( buf ), "%sxoffs %d", a_got?", ":"", ldata->sd1_xofs ), a_got = 1;
            if( ldata->sd1_yofs != -1 )
                sprintf( buf+strlen( buf ), "%syoffs %d", a_got?", ":"", ldata->sd1_yofs ), a_got = 1;
            if( ldata->sd1_sector != -1 )
                sprintf( buf+strlen( buf ), "%ssector %d", a_got?", ":"", ldata->sd1_sector ), a_got = 1;
        }
        if( ldata->sd2_utxidx != -1 || ldata->sd2_ltxidx != -1 || ldata->sd2_ftxidx != -1 ||
            ldata->sd2_sector != -1 || ldata->sd2_xofs != -1 || ldata->sd2_yofs != -1 )
        {
            a_got = 0;

            _dconcat( buf );
            strcat( buf, "side 2: " );

            if( ldata->sd2_utxidx != -1 )
                sprintf( buf+strlen( buf ), "upper '%s'", WN2Str( texlist[ldata->sd2_utxidx].name ) ), a_got = 1;
            if( ldata->sd2_ftxidx != -1 )
                sprintf( buf+strlen( buf ), "%sfull '%s'", a_got?", ":"", WN2Str( texlist[ldata->sd2_ftxidx].name ) ), a_got = 1;
            if( ldata->sd2_ltxidx != -1 )
                sprintf( buf+strlen( buf ), "%slower '%s'", a_got?", ":"", WN2Str( texlist[ldata->sd2_ltxidx].name ) ), a_got = 1;
            if( ldata->sd2_xofs != -1 )
                sprintf( buf+strlen( buf ), "%sxoffs %d", a_got?", ":"", ldata->sd2_xofs ), a_got = 1;
            if( ldata->sd2_yofs != -1 )
                sprintf( buf+strlen( buf ), "%syoffs %d", a_got?", ":"", ldata->sd2_yofs ), a_got = 1;
            if( ldata->sd2_sector != -1 )
                sprintf( buf+strlen( buf ), "%ssector %d", a_got?", ":"", ldata->sd2_sector ), a_got = 1;
        }

        break;
    }

    buf[0] = toupper( buf[0] );

}

void initSearchBlk( SearchBlk *sblk, short mode )
/*-------------------------------------------------------------------------*
 * If a search block hasn't been initialized, this does it.  Allocates
 * objects memory, srchstruc and repstruc memory, etc.
 *-------------------------------------------------------------------------*/
{
    char    *st_tsrch = "Define Search Data";
    char    *st_trep = "Define Replace Data";

    if( !(sblk->objects) )
    {
        /* needs initializing */
        sblk->objects = memget( sizeof( short ) * nMaxFind );
        sblk->srchstruct = memget( search_tbl[stblidx].strucsize );
        sblk->repstruct = memget( search_tbl[stblidx].strucsize );

        srch_initstruc( sblk->srchstruct );
        srch_initstruc( sblk->repstruct );

        switch( editmode )
        {
        case MODE_LINE:
            sblk->srchdlg = line_makedialog( sblk->srchstruct, st_tsrch );
            sblk->repdlg = line_makedialog( sblk->repstruct, st_trep );
            break;
        }
    }

    sblk->replace_percent = 100;

    /* set options based on mode */
    if( sblk->mode != mode )
    {
        sblk->always = 0;
        sblk->marked = (mode && obj_nmarked()) ? 1 : 0;
        sblk->markfound = 0;
        sblk->disponly = 0;
        sblk->stopfirst = mode ? 1 : 0;
    }

    sblk->mode = mode;
}

static void _updbufs( FIELD *fldlist[], void *data )
/*-------------------------------------------------------------------------*
 * Updates the "search/replace" description on the dialog box:
 * calls getdatadesc() to get the buffer description of the data
 * parameters, then word-wraps it into 4 lines or so.
 *-------------------------------------------------------------------------*/
{
    FIELD   *fldp = NULL;
    short   curlen = 0, curfld = -1;
    char    *ptr, descbuf[1024];

    getdatadesc( descbuf, data );
    ptr = strtok( descbuf, " " );

    while( ptr )
    {
        if( !fldp || (strlen( ptr ) + curlen >= nMaxDescLine) )
        {
            if( fldp )
                fld_disp( fldp );

            ++curfld;
            curlen = 0;
            fldp = fldlist[curfld];
            /* clear the previous contents */
            memset( fldp->data, 32, strlen( fldp->data ) );
            fld_disp( fldp );
            ((char*)fldp->data)[0] = 0;
        }

        /* concatenate it */
        strcat( fldp->data, ptr );
        strcat( fldp->data, " " );
        curlen += strlen( ptr )+1;

        /* next word */
        ptr = strtok( NULL, " " );
    }

    fld_disp( fldp );
}

static short pb_setsearch( FIELD *fldp )
{
    extern short _sdeditnum, _lnedit;

    void ld_s2idx( LDIALOG *ldata );

    if( editmode == MODE_LINE )
    {
        /* as if we're editing multiple lines */
        _lnedit = 2;
        _sdeditnum = 0;
    }

    dlg_proc( 0, sblk->srchdlg );

    /* line mode only */
    if( editmode == MODE_LINE )     /* needs to be done */
        ld_s2idx( sblk->srchstruct );

    _updbufs( srchdesc, sblk->srchstruct );

    return DF_CONTINUE;
}

void srch_start( short mode )
/*-------------------------------------------------------------------------*
 * Searches for objects and pops up a dialog box of all the found items.
 * Mode : 0 = search
 *        1 = replace
 *-------------------------------------------------------------------------*/
{
    FIELD       *f1, *f2, *fldp;
    short       y, i;
    uchar       rcheck = mode;

    stblidx = -1;

    for( i = 0; search_tbl[i].editmode != -1; i++ )
    {
        if( editmode == search_tbl[i].editmode )
        {
            stblidx = i;
            break;
        }
    }

    sblk = search_tbl[stblidx].searchp;

    if( !sblk || stblidx == -1 || search_tbl[stblidx].strucsize == 0 )
    {
        message_box( MB_OK, "Under Construction",
            "This edit mode is not yet supported!" );
        return;
    }

    /* initialize the search block if needed */
    initSearchBlk( sblk, mode );

    memset( srchbufs, 0, sizeof srchbufs );
    memset( repbufs, 0, sizeof repbufs );

    if( !sdlgp )
    {
        sdlgp = dlg_init( Centered, Centered, 60, 20, "Search / Replace Objects" );

        y = 3;

        fldp = dlg_addbox( sdlgp, 0, y, 0, nMaxDataDesc+1 );
        fldp->y1 -= 2;
        dlg_addpush( sdlgp, 54, y-1, " Set ", pb_setsearch );
        dlg_addtext( sdlgp, 1, y-1, " Find: " );

        /* create text fields */
        for( i = 0; i < nMaxDataDesc; i++ )
            srchdesc[i] = dlg_addtext( sdlgp, 1, y+i, srchbufs[i] );

        y += 6;

        fldp = dlg_addbox( sdlgp, 0, y, 0, 5 );
        fldp->y1 -= 2;
        dlg_addpush( sdlgp, 54, y-1, " Set ", (__DFUNC) pb_ok ); // pb_setreplace );
        dlg_addtext( sdlgp, 1, y-1, " Replace with: " );

        /* create text fields */
        for( i = 0; i < nMaxDataDesc; i++ )
            repdesc[i] = dlg_addtext( sdlgp, 1, y+i, repbufs[i] );

        y += 7;

        /* "marked objects only" */
        f1 = dlg_addcheck( sdlgp, 1, y, &sblk->marked );
        dlg_addtext( sdlgp, 3, y, "Search marked objects only" );

        dlg_addcheck( sdlgp, 30, y, &sblk->markfound );
        dlg_addtext( sdlgp, 32, y, "Mark found objects" );

        /* this field controls what is done after the search: if 1, then
        * the first found object is "go-to'd" without the results dialog
        * box popping up - if not, the results dialog is popped up after
        * the search. */

        y++;

        fldp = dlg_addcheck( sdlgp, 1, y, &sblk->stopfirst );
        fld_setadj( fldp, 0, 2 );
        fldp = dlg_addtext( sdlgp, 3, y, "Jump to first object" );
        fld_setadj( fldp, 0, 2 );

        fldp = dlg_addcheck( sdlgp, 30, y, &sblk->disponly );
        fld_setadj( fldp, 0, 2 );
        f2 = fldp = dlg_addtext( sdlgp, 32, y, "Only display found objects" );
        fld_setadj( fldp, 0, 2 );

        dlg_addcbox( sdlgp, f1, f2 );
    }

    dlg_proc( 0, sdlgp );
}

/*
short srch_next( SearchBlk *srchp, short direc )
/*-------------------------------------------------------------------------*
 * Finds the prev / next object in the search.                             *
 *-------------------------------------------------------------------------*/
{
    SFUNC   *s_fp;
    RFUNC   *r_fp;
    short   rvl, rquery;

    /* Set up these to avoid indirection */
    s_fp = srch_tables[srchp->srch_code].search_fp;
    r_fp = srch_tables[srchp->srch_code].replace_fp;

    /* Run the search function */
    rvl = (*s_fp)( srchp, direc );
    srchp->curobject = rvl;       /* Whatever this was set to. */

    if( rvl != -1 )
    {
        /* Mark found objects? */
        obj_mark( srchp->curobject );

        /* Jump to the object. */
        setup_go_object( srchp->curobject )
        map_update();
        go_obj = srchp->curobject;

        if( srchp->flags & SF_REPLACE )
        {
            /* Ask if they want to.. */
            if( !(srchp->flags & SF_ALWAYS) )
            {
                rquery = message_box( MB_YESNOALWAYS | MB_CANCEL,
                                      "Replace Query",
                                      "Replace this object?" );
            }
            else
                rquery = MB_YES;

            /* rquery = always?  Then set the always flag. */
            switch( rquery )
            {
            case MB_ALWAYS:
                srchp->flags |= SF_ALWAYS;
                /* Follow through to below.. */
            case MB_YES:
                (*r_fp)( srchp );
                break;
            case MB_NO:     /* no, let's not replace this one. */
                break;
            case MB_CANCEL:     /* woah horsie */
                return S_CANCEL;
            }
        }
    }

    return rvl;
}
*/

/*
static void _ld_search( void )
/*-------------------------------------------------------------------------*
 * Searches for the prev / next matching linedef from the information      *
 * in srchp.                                                               *
 *-------------------------------------------------------------------------*/
{
    short       i;
    LINEDEF     *ldp;
    SIDEDEF     *sd1, *sd2;
    LDIALOG     *ldsp = (LDIALOG *) srchp->srchstruct;
    short       attribs;

    for( i = srchp->curobject + direc, ldp = linelist+i;
         (direc == S_FORWARD ? (i < nline) : (i >= 0));
         i += direc )
    {
        if( !(srchp->flags & SF_MARKED) || obj_ismarked( i ) )
        {
            sd1 = ldp->rsdef == -1 ? NULL : &sidelist[ldp->rsdef];
            sd2 = ldp->lsdef == -1 ? NULL : &sidelist[ldp->lsdef];

            attribs = ldp->attribs;

            /* Do this one. */
            if( ldsp->type != -1 && ldsp->type != ldp->type )
                continue;
            if( ldsp->sectag != -1 && ldsp->sectag != ldp->sectag )
                continue;
            if( !bittest( attribs, ldsp->a_impass, LA_IMPASS ) )
                continue;
            if( !bittest( attribs, ldsp->a_mblocker, LA_MBLOCKER ) )
                continue;
            if( !bittest( attribs, ldsp->a_tside, LA_TSIDE ) )
                continue;
            if( !bittest( attribs, ldsp->a_uunpegged, LA_UUNPEGGED ) )
                continue;
            if( !bittest( attribs, ldsp->a_lunpegged, LA_LUNPEGGED ) )
                continue;
            if( !bittest( attribs, ldsp->a_secret, LA_SECRET ) )
                continue;
            if( !bittest( attribs, ldsp->a_sblock, LA_SBLOCK ) )
                continue;
            if( !bittest( attribs, ldsp->a_notonmap, LA_NOTONMAP ) )
                continue;
            if( !bittest( attribs, ldsp->a_autoonmap, LA_AUTOONMAP ) )
                continue;

            /* check sidedefs : if any of the search data is set and does not match
             * OR the linedef's corresponding sidedef does not exist, continue. */
            if( ldata->sd1_utxidx != -1 && (sd1 == NULL || sd1->utxidx != ldata->sd1_utxidx) )
                continue;
            if( ldata->sd1_ftxidx != -1 && (sd1 == NULL || sd1->ftxidx != ldata->sd1_ftxidx) )
                continue;
            if( ldata->sd1_ltxidx != -1 && (sd1 == NULL || sd1->ltxidx != ldata->sd1_ltxidx) )
                continue;
            if( ldata->sd1_sector != -1 && (sd1 == NULL || sd1->sector != ldata->sd1_sector) )
                continue;
            if( ldata->sd1_xofs != -1 && (sd1 == NULL || sd1->wxofs != ldata->sd1_xofs) )
                continue;
            if( ldata->sd1_yofs != -1 && (sd1 == NULL || sd1->wyofs != ldata->sd1_yofs) )
                continue;

            if( ldata->sd2_utxidx != -1 && (sd2 == NULL || sd2->utxidx != ldata->sd2_utxidx) )
                continue;
            if( ldata->sd2_ftxidx != -1 && (sd2 == NULL || sd2->ftxidx != ldata->sd2_ftxidx) )
                continue;
            if( ldata->sd2_ltxidx != -1 && (sd2 == NULL || sd2->ltxidx != ldata->sd2_ltxidx) )
                continue;
            if( ldata->sd2_sector != -1 && (sd2 == NULL || sd2->sector != ldata->sd2_sector) )
                continue;
            if( ldata->sd2_xofs != -1 && (sd2 == NULL || sd2->wxofs != ldata->sd2_xofs) )
                continue;
            if( ldata->sd2_yofs != -1 && (sd2 == NULL || sd2->wyofs != ldata->sd2_yofs) )
                continue;

            // foundobject( i );
        }
    }
}
*/

static void _ld_replace( void )
{
    line_dlg2object( sblk->repstruct, sblk->objects[sblk->curidx] );
}

#endif
