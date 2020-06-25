/*-------------------------------------------------------------------------*
 * SECTOR.C:                                                               *
 * Functions for editing sectors, of course.                               *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "fields.hpp"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static Dialog       *pSectorDlg;
static fldString    *pfFloorVal, *pfCeilVal, *pfSectorTag;
static fldListBox   *pfCeilList, *pfFloorList, *pfTypeList;
static fldFlatTex   *pfFloorTex, *pfCeilTex;

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

int dlgfindtag( fldString* pField )
{
    /* Find the first free tag! */
    pField->SetInteger( firstfreetag() );
    return daContinue;
}

/*-------------------------------------------------------------------------*
 * Supports + and - in ceilings / floors.                                  *
 *-------------------------------------------------------------------------*/

static int _fc_exit( fldString* pField )
{
    const char* pszField = pField->GetText();
    short base = ((pField == pfFloorVal) ?
                 atoi( pfCeilVal->GetText() ) : atoi( pfFloorVal->GetText() ));
    short n = atoi( pszField+1 );

    if( *pszField == '-' || *pszField == '+' )
    {
        ++pszField;
        if( *pszField == '+' || *pszField == '-' )  /* second char too */
        {
            /* if the first character of the data buffer is a plus or
             * minus, add the rest of it to the opposite of the field
             * (floor/ceil, ceil/floor) */
            base += n;
            char szBuf[10];
            itoa( base, szBuf, 10 );
            pField->SetText( szBuf );
        }
    }

    return TRUE;
}

static int _fld_showflat( fldListBox* pField )
/*-------------------------------------------------------------------------*
 * Displays a floor texture in a region, depending on what field 'pField'
 * is..
 *-------------------------------------------------------------------------*/
{
    if( pField == pfCeilList )  // display the ceiling
        pfCeilTex->SetFlat( pField->GetText() );
    else if( pField == pfFloorList )
        pfFloorTex->SetFlat( pField->GetText() );

    return daContinue;
}

int TexBrowser( int *iSelected, bool fFloors );

static int _floorbrow( fldListBox* pField )
{
    int iTexture = floor_toint( (char*) pField->GetText() );
    if( TexBrowser( &iTexture, TRUE ) )
    {
        pField->SetCurItem( iTexture );
        _fld_showflat( pField );
    }

    return daContinue;
}

static int _sdbox_up( Dialog* )
{
    _fld_showflat( pfCeilList );
    _fld_showflat( pfFloorList );

    return daContinue;
}

static void MakeFlatField( Field* pField )
{
    pField->SetUserEvent( etKey, KEY_F2, (FUFUNC) _floorbrow );
    pField->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _floorbrow );
}

short sector_edit( short object, short editmarked )
/*-------------------------------------------------------------------------*
 * Edits the sector in 'object' plus all the marked sectors, if any, if    *
 * editmarked is set to 1.                                                 *
 *-------------------------------------------------------------------------*/
{
    static char fszSectorType[80];
    static int  iCeilHeight, iFloorHeight;
    static int  iSectorTag;
    static int  iLightValue;
    static char fszFloorTex[9];
    static char fszCeilTex[9];

    SECTOR *sptr = &sectorlist[object], *secp2;
    Field* pField;
    short  rvl, newtype, i, stype;
    char   szTitle[80];
    uchar  unmarkobj = 0;

    if( object != -1 && !obj_ismarked( object ) )
        obj_mark( object ), unmarkobj = 1;

    if( object != -1 && obj_nmarked() == 1 )
        editmarked = 0;

    if( !editmarked && object == -1 )
        return 0;

    stype = -2;

    for( i = 0, secp2 = sectorlist; i < nsector; i++, secp2++ )
    {
        if( obj_ismarked( i ) )
        {
            if( stype == -2 )
            {
                iCeilHeight = secp2->cheight;
                iFloorHeight = secp2->fheight;
                iSectorTag = secp2->sectag;
                iLightValue = secp2->light;
                memcpy( fszFloorTex, secp2->ftxname, sizeof( WADNAME ) );
                memcpy( fszCeilTex, secp2->ctxname, sizeof( WADNAME ) );
                fszFloorTex[8] = fszCeilTex[8] = 0;
                stype = secp2->type;

                if( !editmarked )
                    break;
            }

            if( secp2->cheight != iCeilHeight )
                iCeilHeight = FI_NODATA;
            if( secp2->fheight != iFloorHeight )
                iFloorHeight = FI_NODATA;
            if( secp2->sectag  != iSectorTag  )
                iSectorTag = FI_NODATA;
            if( secp2->light   != iLightValue   )
                iLightValue = FI_NODATA;
            if( memcmp( fszFloorTex, secp2->ftxname, sizeof( WADNAME ) ) )
                *fszFloorTex = 0;
            if( memcmp( fszCeilTex, secp2->ctxname, sizeof( WADNAME ) ) )
                *fszCeilTex = 0;
            if( secp2->type != stype )
                stype = -1;
        }
    }

    if( stype == -1 )
        *fszSectorType = 0;
    else
        strcpy( fszSectorType, SecTypeToStr( stype ) );

    fldRegion* rgnTemp;

    pSectorDlg = new Dialog;
    // load from file
    pSectorDlg->Create( szDSFile, "SectorDlg" );

    VarTransfer_t vtSectorDlg[] =
    {
        { "Special", (Field**)&pfTypeList, (PTR)&fszSectorType, NULL },
        { "FloorVal", (Field**)&pfFloorVal, (PTR)&iFloorHeight, NULL },
        { "CeilVal", (Field**)&pfCeilVal, (PTR)&iCeilHeight, NULL },
        { "CeilList", (Field**)&pfCeilList, (PTR)&fszCeilTex, NULL },
        { "FloorList", (Field**)&pfFloorList, (PTR)&fszFloorTex, NULL },
        { "Light", NULL, (PTR)&iLightValue, NULL },
        { "Tag", (Field**)&pfSectorTag, (PTR)&iSectorTag, NULL },

        // end-marker
        { NULL, NULL, NULL, NULL }
    };

    pSectorDlg->SetOnEnter( (DFUNC) _sdbox_up );
    pSectorDlg->VarSetup( vtSectorDlg );

    // set fields
    pfFloorTex = new fldFlatTex;
    pfCeilTex = new fldFlatTex;

    pSectorDlg->REPLACEREGION( "FloorTex", pfFloorTex );
    pSectorDlg->REPLACEREGION( "CeilTex", pfCeilTex );

    // set key functions
    pfCeilList->SetOnChange( (FFUNC) _fld_showflat );
    MakeFlatField( pfCeilList );
    pfFloorList->SetOnChange( (FFUNC) _fld_showflat );
    MakeFlatField( pfFloorList );

    pfFloorVal->SetOnExit( (FFUNC) _fc_exit );
    pfCeilVal->SetOnExit( (FFUNC) _fc_exit );

    pfSectorTag->SetUserEvent( etKey, KEY_ALTF, (FUFUNC) dlgfindtag );
    pfSectorTag->SetUserEvent( etKey, KEY_CTRLF, (FUFUNC) dlgfindtag );
    pfSectorTag->SetUserEvent( etKey, 'F', (FUFUNC) dlgfindtag );

    pfTypeList->SetItemList( pgdCurGame->stlSectorTypes );
    pfFloorList->SetItemList( pgdCurGame->stlFlats );
    pfCeilList->SetItemList( pgdCurGame->stlFlats );

    pSectorDlg->SetMultiEdit( editmarked ? TRUE : FALSE );

    /* Process the dialog */
_dodlg:
    rvl = pSectorDlg->Process();

    if( rvl == daExitSave )
    {
        if( (fszCeilTex[0] && floor_toint( fszCeilTex ) == -1) ||
            (fszFloorTex[0] && floor_toint( fszFloorTex ) == -1) )
        {
            message_box( MB_OK, "Error",
                         "One of the ceiling or floor textures is invalid." );
            goto _dodlg;
        }

        newtype = SecStrToType( fszSectorType );

        nMadeChanges++;

        if( !editmarked )
        {
            /* Stick it back in */
            if( newtype == -1 )
                newtype = 0;

            sptr->cheight = iCeilHeight;
            sptr->fheight = iFloorHeight;
            sptr->sectag = iSectorTag;
            sptr->light = iLightValue;
            sptr->type = newtype;
            memcpy( sptr->ctxname, fszCeilTex, 8 );
            memcpy( sptr->ftxname, fszFloorTex, 8 );
            sect_fixtex( object, 0 );
        }
        else
        {
            for( i = 0; i < MAXOBJECT; i++ )
            {
                if( obj_ismarked( i ) )
                {
                    sptr = &sectorlist[i];
                    if( newtype != -1 )
                        sptr->type = newtype;
                    if( iCeilHeight != FI_NODATA )
                        sptr->cheight = iCeilHeight;
                    if( iFloorHeight != FI_NODATA )
                        sptr->fheight = iFloorHeight;
                    if( iSectorTag != FI_NODATA )
                        sptr->sectag = iSectorTag;
                    if( iLightValue != FI_NODATA )
                        sptr->light = iLightValue;
                    if( fszCeilTex[0] )
                        memcpy( sptr->ctxname, fszCeilTex, 8 );
                    if( fszFloorTex[0] )
                        memcpy( sptr->ftxname, fszFloorTex, 8 );
                }
            }

            /* Do it here cuz some of the sectors affected might be being modified too.
             * This way we ensure all the mods are made befroe fixing textures so we
             * definitely don't miss any. */
            for( i = 0; i < MAXOBJECT; i++ )
                if( obj_ismarked( i ) )
                    sect_fixtex( i, 0 );
        }

    }

    if( unmarkobj )
        obj_unmark( object );

    return object;
}

short sect_changeheight( short csr_object )
/*-------------------------------------------------------------------------*
 * ALlows the user to change the heights of the marked sectors.            *
 *-------------------------------------------------------------------------*/
{
    short   i, nchange = 0;
    int     fdelta = 0, cdelta = 0, ldelta = 100;
    SECTOR  *secp;
    uchar   unmarkobj = 0;
    static  uchar   fixtex = 0;

    Dialog* pDialog = new Dialog;
    pDialog->Create( szDSFile, "SectHeightDlg" );

    VarTransfer_t vtSectHeight[] =
    {
        { "FloorDelta", NULL, (PTR) &fdelta, NULL },
        { "CeilDelta", NULL, (PTR) &cdelta, NULL },
        { "LightAmp", NULL, (PTR) &ldelta, NULL },

        // end-marker
        { NULL, NULL, NULL, NULL }
    };

    pDialog->VarSetup( vtSectHeight );

    int iRvl = pDialog->Process();

    delete pDialog;

    if( iRvl == daExitNoSave )
        return 0;

    if( csr_object != -1 && !obj_ismarked( csr_object ) )
        obj_mark( csr_object ), unmarkobj = 1;

    for( i = 0, secp = sectorlist; i < nsector; i++, secp++ )
    {
        if( obj_ismarked( i ) )
        {
            secp->fheight += fdelta;
            secp->cheight += cdelta;

            secp->light = secp->light * ldelta / 100;

            if( secp->light < 0 )
                secp->light = 0;
            else if( secp->light > 255 )
                secp->light = 255;

            ++ nchange;

            sect_fixtex( i, 0 );
        }
    }

    if( nchange )
        nMadeChanges ++;

    if( unmarkobj )
        obj_unmark( csr_object );

    return nchange;
}
