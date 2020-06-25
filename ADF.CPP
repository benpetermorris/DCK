/*-------------------------------------------------------------------------*
 * ADF.C:                                                                  *
 * Fucntions for loading the ADF tables and selecting motifs, sector-      *
 * objects, etc.                                                           *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

char        *motif_name = "";   /* current motif name for display on sbar */
extern      short motif_num;    /* dck.c -- used for status save/restore */

AD_SECTION  *ad_list;           /* current game data adf pointers.. to save time */
AD_SECTION  *defmotif_ptr;
short       n_adf;

AD_SECTION  *motif_ptr;         /* current motif ptr */

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

short adf_load( void )
{
    FILE        *fp;
    short       i, i2, i3, m1 = 1;
    char        fn[128];
    AD_SECTION  *secp;
    MO_PARM     *mptr;
    SO_COMMAND  *scptr;
    SO_PARM     *spptr;

    sprintf( fn, "%s.AD", pgdCurGame->cfgname );
    fp = fopen( fn, "rb" );

    if( fp == NULL )
        return 0;

    if( fgetc( fp ) != 'A' || fgetc( fp ) != 'D' || fgetc( fp ) != 'F' )
    {
        fclose( fp );
        return 0;
    }

    fread( &n_adf, sizeof n_adf, 1, fp );

    _memstr = "ADF list";
    ad_list = memget( n_adf * sizeof( AD_SECTION ) );

    /* Do each one */
    for( i = 0, secp = ad_list; i < n_adf; i++, secp++ )
    {
        fread( &(secp->hdr), sizeof( ADF_SECTION ), 1, fp );

        /* Now, read in its commands */
        switch( secp->hdr.type )
        {
        case SEC_MOTIF:

            if( m1 )        /* First motif */
                defmotif_ptr = secp;
            m1 = 0;

            /* Read in a motif.  This is easy. */
            _memstr = "motif command list";
            secp->cmd.mo_cmds = memget( secp->hdr.ncmd * sizeof( MO_PARM ) );
            mptr = secp->cmd.mo_cmds;

            for( i2 = 0; i2 < secp->hdr.ncmd; i2++, mptr++ )
            {
                fread( mptr->name, TOKLEN, 1, fp );
                /* Read in the type of parameter */
                fread( &(mptr->type), sizeof( short ), 1, fp );
                /* Read in the parameter */
                switch( mptr->type )
                {
                case SOP_STRING:
                    fread( mptr->sdata, TOKLEN, 1, fp );
                    break;
                case SOP_INTEGER:
                    fread( &(mptr->idata), sizeof( short ), 1, fp );
                    break;
                }
            }
            break;
        case SEC_SOBJECT:
            /* Read in a sector object. */
            _memstr = "sector object command list";
            secp->cmd.so_cmds = memget( secp->hdr.ncmd * sizeof( SO_COMMAND ) );
            scptr = secp->cmd.so_cmds;

            for( i2 = 0; i2 < secp->hdr.ncmd; i2++, scptr++ )
            {
                spptr = scptr->p;

                /* Read in the command number */
                scptr->cmd = fgetc( fp );

                /* Read in the parameters */
                for( i3 = 0; i3 < SO_MAXPARM; i3++, spptr++ )
                {
                    spptr->type = fgetc( fp );
                    switch( spptr->type )
                    {
                    case SOP_NONE:
                        /* Done reading this command */
                        i3 = SO_MAXPARM;
                        break;
                    case SOP_STRING:
                        fread( spptr->sdata, TOKLEN, 1, fp );
                        break;
                    case SOP_INTEGER:
                    case SOP_ADJTYPE:
                        fread( &(spptr->idata), sizeof( short ), 1, fp );
                        break;
                    }
                }
            }
            break;
        }
    }

    pgdCurGame->ad_list = ad_list;
    pgdCurGame->defmotif_ptr = defmotif_ptr;
    pgdCurGame->n_adf = n_adf;
    motif_ptr = defmotif_ptr;

    return 1;
}

MO_PARM *motif_getparm( uchar *cmdstr )
{
    MO_PARM *pptr = motif_ptr->cmd.mo_cmds;
    short     i;

    for( i = 0; i < motif_ptr->hdr.ncmd; i++, pptr++ )
    {
        if( !strcmp( pptr->name, cmdstr ) )
            return pptr;
    }

    /* Search in the default. */
    pptr = defmotif_ptr->cmd.mo_cmds;

    for( i = 0; i < defmotif_ptr->hdr.ncmd; i++, pptr++ )
    {
        if( !strcmp( pptr->name, cmdstr ) )
            return pptr;
    }

    return NULL;
}

uchar *motif_getstr( uchar *cmdstr )
{
    MO_PARM *pptr = motif_getparm( cmdstr );

    if( pptr )
        return pptr->sdata;

    return NULL;
}

short motif_getint( uchar *cmdstr )
{
    MO_PARM *pptr = motif_getparm( cmdstr );

    if( pptr )
        return pptr->idata;

    return NULL;
}

void _textest( short tex, char *texname, char *motname )
{
    if( tex < 1 )
    {
        message_box( MB_OK, st_dcksays,
          "The texture '%s' is invalid (used in the motif\n"
          "named '%s' in %s.GD.)", texname, motname, pgdCurGame->cfgname );
    }
}

void motif_set( short mno )
{
    short   n = 0, i;
    char    *tp, *cp;

    /* Find the motif */
    for( i = 0; i < n_adf; i++ )
    {
        if( ad_list[i].hdr.type == SEC_MOTIF )
            if( n++ == mno )
                break;
    }

    n = i;
    motif_num = mno;
    motif_ptr = &ad_list[n];

    motif_name = motif_ptr->hdr.name;
    cp = motif_name;

    _def_ftxidx = texture_toint( tp = motif_getstr( "TEXNORMAL" ) );
    _textest( _def_ftxidx, tp, cp );
    _def_utxidx = texture_toint( tp = motif_getstr( "TEXUPPER" ) );
    _textest( _def_utxidx, tp, cp );
    _def_ltxidx = texture_toint( tp = motif_getstr( "TEXLOWER" ) );
    _textest( _def_ltxidx, tp, cp );
    _def_doorbig = texture_toint( tp = motif_getstr( "DOORBIG" ) );
    _textest( _def_doorbig, tp, cp );
    _def_doorsmall = texture_toint( tp = motif_getstr( "DOORSMALL" ) );
    _textest( _def_doorsmall, tp, cp );
    _def_doortrak = texture_toint( tp = motif_getstr( "DOORTRAK" ) );
    _textest( _def_doortrak, tp, cp );
    memcpy( _def_ceiltex, motif_getstr( "CEILTEX" ), sizeof( WADNAME ) );
    memcpy( _def_floortex, motif_getstr( "FLOORTEX" ), sizeof( WADNAME ) );
    _def_light = motif_getint( "LIGHTNORMAL" );
}

void motif_pick( void )
/*-------------------------------------------------------------------------*
 * Allows the user to pick a motif from the list of motifs.  Then sets     *
 * the appropriate global variables so the motif'll be used in defaults.   *
 *-------------------------------------------------------------------------*/
{
    char    *mlist[32];
    short   i, n = 0;

    /* First, create a list of all the motifs. */
    for( i = 0; i < n_adf && n < 32; i++ )
    {
        if( ad_list[i].hdr.type == SEC_MOTIF )
            mlist[n++] = ad_list[i].hdr.name;
    }

    mlist[n] = NULL;        /* mnu_popa needs that. */

    /* Select one! */
    n = MenuPopArray( motif_num, "Choose a motif", NULL, mlist );

    if( n == -1 )
        return;     /* none selected */

    if( n != 0 )
        motif_set( n ); /* Set picked */
}

short so_pick( void )
/*-------------------------------------------------------------------------*
 * Pretty much the same as motif_pick, except it uses sector objects.      *
 *-------------------------------------------------------------------------*/
{
    char    *slist[32];
    static  short last;
    short     i, n = 0;

    /* First, create a list of all the sector objects. */
    for( i = 0; i < n_adf && n < 32; i++ )
    {
        if( ad_list[i].hdr.type == SEC_SOBJECT )
            slist[n++] = ad_list[i].hdr.name;
    }

    slist[n] = NULL;        /* mnu_popa needs that. */

    /* Select one! */
    n = MenuPopArray( 0, "Apply sector object", NULL, slist );

    if( n == -1 )
        return -1;     /* none selected */

    last = n;
    return n;
}

short sect_getmostobviousparent( short object )
/*-------------------------------------------------------------------------*
 * Based on the number of two-sided lines that reference object AND        *
 * another sector, returns the number of the sector that's referenced      *
 * most often.                                                             *
 *-------------------------------------------------------------------------*/
{
    short     *s_count = memget( nsector * sizeof( short ) ), i;
    short     theline, thesect = 0, thecount = 0;
    LINEDEF *ldp;

    memset( s_count, 0, nsector * sizeof( short ) );

    theline = obj_linefind( LF_BEGIN,
                            LF_ATTRIBS, LA_TSIDE,
                            LF_SIDEWITHSEC, object, LF_END );

    while( theline != -1 )
    {
        ldp = &linelist[theline];

        if( ldp->lsdef >= 0 && sidelist[ldp->lsdef].sector == object )
            s_count[sidelist[ldp->rsdef].sector]++;
        else if( ldp->rsdef >= 0 && sidelist[ldp->rsdef].sector == object )
            s_count[sidelist[ldp->lsdef].sector]++;

        theline = obj_linefind( LF_CONTINUE );
    }

    /* Find the one with the most references and return it. */
    for( i = 0; i < nsector; i++ )
    {
        if( s_count[i] > thecount )
        {
            thecount = s_count[i];
            thesect = i;
        }
    }

    memfree( s_count );
    return thesect;
}

short getadjint( SECTOR *secp, short psecval, SO_COMMAND *cmds )
{
    short     adjcmd = cmds->p[0].idata;      /* adj command */
    short     val = cmds->p[1].idata;         /* value to adjust by */

    switch( adjcmd )
    {
    case ADJ_CEILING:
        return secp->cheight + val;
    case ADJ_FLOOR:
        return secp->fheight + val;
    case ADJ_REAL:
        return val;
    case ADJ_ADJACENT:
        return psecval + val;
    }

    return val;
}

uchar *sostrcvt( uchar *origstr, uchar *defstr )
/*-------------------------------------------------------------------------*
 * Cannot be used as a pointer source.  Must be copied.                    *
 *-------------------------------------------------------------------------*/
{
    static  char tmpbuf[128];
    uchar   *sbuf;

    /* If the first character is not '$', just return the original string
     * because no conversion is required. */
    if( origstr[0] != '$' )
        return origstr;

    /* If it's a M_ var.. */
    if( !memcmp( origstr, "$M_", 3 ) )
    {
        sbuf = motif_getstr( origstr+3 );
        if( sbuf == NULL )
        {
            message_box( MB_OK, "Warning",
                         "Variable '%s' was not recognized.\n"
                         "Using default setting: '%s'.", origstr, defstr );
            sbuf = defstr;
        }
        strcpy( tmpbuf, sbuf );
        return tmpbuf;
    }

    return origstr;
}

void so_apply( short object, short so_num )
/*-------------------------------------------------------------------------*
 * Applies sectorobject so_num to 'object'.                                *
 *-------------------------------------------------------------------------*/
{
    SECTOR  *secp, *psecp;
    AD_SECTION *so_ptr;
    SO_COMMAND *so_cptr;
    SO_PARM *spptr;
    SIDEDEFM *sdp;
    short     n = 0, i, tmpint, theline, tmpx, tmpy, texidx, defidx;
    uchar   tempbuf[128];
    LINEDEF *ldp;

    /* Find the sector object */
    for( i = 0; i < n_adf; i++ )
    {
        if( ad_list[i].hdr.type == SEC_SOBJECT )
            if( n++ == so_num )
                break;
    }

    /* Set some VARS man */
    n = i;
    so_ptr = &ad_list[n];
    so_cptr = ad_list[n].cmd.so_cmds;

    /* Set sector and parent sector */
    secp = &sectorlist[object];
    psecp = &sectorlist[sect_getmostobviousparent( object )];

    for( i = 0; i < so_ptr->hdr.ncmd; i++, so_cptr++ )
    {
        spptr = so_cptr->p;

        switch( so_cptr->cmd )
        {
        case SC_FLOORTEX:       /* Set floor texture */
            wncpy( secp->ftxname, sostrcvt( spptr[0].sdata, _def_floortex ) );
            break;
        case SC_FLOOR:          /* Set floor height */
            secp->fheight = getadjint( secp, psecp->fheight, so_cptr );
            break;
        case SC_CEILTEX:        /* Set ceiling texture */
            wncpy( secp->ctxname, sostrcvt( spptr[0].sdata, _def_ceiltex ) );
            break;
        case SC_CEIL:           /* Set ceiling hegiht */
            secp->cheight = getadjint( secp, psecp->cheight, so_cptr );
            break;
        case SC_LIGHT:          /* Light value, simple 'nuf. */
            secp->light = getadjint( secp, psecp->light, so_cptr );
            break;
        case SC_BLTYPE:         /* Borderline type */
            theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, object, LF_END );
            tmpint = spptr[0].idata;
            while( theline != -1 )
            {
                ldp = &linelist[theline];
                ldp->special = tmpint;
                theline = obj_linefind( LF_CONTINUE );
            }
            break;
        case SC_OBJINSERT:      /* Insert an object. */
            obj_center( MODE_SECTOR, object, &tmpx, &tmpy );
            thinglist[obj_insert( MODE_THING, -1, tmpx, tmpy )].type = spptr[0].idata;
            break;
        case SC_SECTYPE:        /* Sector type. */
            secp->type = spptr[0].idata;
            break;
        case SC_SECTAG:         /* Find me a sector tag. */
            secp->sectag = firstfreetag();
            break;
        case SC_BLSETTAG:       /* Borderline type */
            theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, object, LF_END );
            while( theline != -1 )
            {
                ldp = &linelist[theline];
                ldp->sectag = secp->sectag;
                theline = obj_linefind( LF_CONTINUE );
            }
            break;
        case SC_BLLTEX:         /* Set borderline textures */
            defidx = _def_ltxidx;
            goto _dotex;
        case SC_BLUTEX:
            defidx = _def_utxidx;
            goto _dotex;
        case SC_BLFTEX:
            defidx = _def_ftxidx;
_dotex:
            texture_tostr( tempbuf, defidx );   /* For defaults on error. */
            texidx = texture_toint( sostrcvt( spptr[0].sdata, tempbuf ) );

            theline = obj_linefind( LF_BEGIN, LF_SIDEWITHSEC, object, LF_END );
            while( theline != -1 )
            {
                ldp = &linelist[theline];

                if( ldp->lsdef >= 0 )
                {
                    sdp = &sidelist[ldp->lsdef];
                    switch( so_cptr->cmd )
                    {
                    /* Set the textures depending on the commmand */
                    case SC_BLLTEX: sdp->ltxidx = texidx; break;
                    case SC_BLUTEX: sdp->utxidx = texidx; break;
                    case SC_BLFTEX: sdp->ftxidx = texidx; break;
                    }
                }

                if( ldp->rsdef >= 0 )
                {
                    sdp = &sidelist[ldp->rsdef];
                    switch( so_cptr->cmd )
                    {
                    /* Set the textures depending on the command */
                    case SC_BLLTEX: sdp->ltxidx = texidx; break;
                    case SC_BLUTEX: sdp->utxidx = texidx; break;
                    case SC_BLFTEX: sdp->ftxidx = texidx; break;
                    }
                }

                theline = obj_linefind( LF_CONTINUE );
            }
            break;
        }
    }

    nMadeChanges++;
}

void so_set( short object, short so_num )
/*-------------------------------------------------------------------------*
 * Does it on all the marked objects.                                      *
 *-------------------------------------------------------------------------*/
{
    short     i;
    short     unmarkobj = 0;

    if( object != -1 && !obj_ismarked( object ) )
        obj_mark( object ), unmarkobj = 1;

    for( i = 0; i < nsector; i++ )
    {
        if( obj_ismarked( i ) )
        {
            so_apply( object, so_num );
            post_fullupdate();
        }
    }

    if( unmarkobj )
        obj_unmark( object );
}
