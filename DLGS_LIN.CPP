/*-------------------------------------------------------------------------*
 * DLGS_LINE.CPP:
 * Dialog boxes for editing lines, etc.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "pickdlg.hpp"
#include "fields.hpp"

/*-------------------------------------------------------------------------*
 *
 * Globals
 *
 *-------------------------------------------------------------------------*/

#define LA_SETSPAC(attribs,ac)  (attribs &= 0x3ff, attribs |= ((ac)<<10))

short firstfreepolyid();

// data & field variables
class fldTexName;

static bool fGotSide1, fGotSide2, fAddSide2, fEditMarked, fDelSide2;
static int  iS1XOfs, iS1YOfs, iS1Sector, iS1Tex0, iS1Tex1, iS1Tex2;
static int  iS2XOfs, iS2YOfs, iS2Sector, iS2Tex0, iS2Tex1, iS2Tex2;
static int  iCurSide, iSecTag;

// for validation:
static char szS1Tex0Name[9], szS1Tex1Name[9], szS1Tex2Name[9];
static char szS2Tex0Name[9], szS2Tex1Name[9], szS2Tex2Name[9];

static fldString    *pfSpecial, *pfArgs[5];
static fldTexName   *pfSideSector, *pfSideXOffset, *pfSideYOffset,
                    *pfTex0Name, *pfTex1Name, *pfTex2Name;
static fldWallTex   *pfTexture0, *pfTexture1, *pfTexture2;
static fldButton    *pfSideSwitch, *pfSideDelete;
static fldCheckBox  *pfImpassable, *pfTwoSided, *pfBlockMonster, *pfUnpegTop,
                    *pfUnpegBottom, *pfSecret, *pfBlockSound, *pfNeverMap,
                    *pfAlwaysMap, *pfRepeatable;

/*-------------------------------------------------------------------------*
 *
 * Code begins
 *
 *-------------------------------------------------------------------------*/

class fldTexName : public fldString
{
public:
    fldTexName() : fldString()
    {
        szSearch[0] = 0;
    }

    int OnEvent( Event_t& event )
    {
        int iTexture;
        int iXofs, iYofs;
        bool fTopOrigin;     // current texture - top or bottom origin?
        fldWallTex* pfWallTex;

        if( event.type == etKey || (event.type == etMouse &&
            event.iCode == MOUSE_LDBLCLICK) )
        {
            // set texture integer & top origin flag
            if( this == pfTex0Name )
            {
                pfWallTex = pfTexture0;
                fTopOrigin = pfUnpegTop->GetState() ? TRUE : FALSE;
            }
            else if( this == pfTex1Name )
            {
                pfWallTex = pfTexture1;
                fTopOrigin = pfUnpegBottom->GetState() ? FALSE : TRUE;
            }
            else if( this == pfTex2Name )
            {
                pfWallTex = pfTexture2;
                fTopOrigin = pfUnpegBottom->GetState() ? TRUE : FALSE;
            }

            iTexture = texture_toint( (char*) GetText() );

            if( iTexture == -1 )  // that's not valid, so get walltex's field
                iTexture = pfWallTex->GetWall();

            switch( event.iCode )
            {
            case KEY_F2:        // pick a texture
            case MOUSE_LDBLCLICK:
                if( TexBrowser( &iTexture, FALSE ) )
                {
                    pfWallTex->SetWall( iTexture );
                    SetText( WN2Str( texlist[iTexture].name ) );
                }
                break;
            case KEY_F3:        // align texture
                if( iCurSide == 1 )
                {
                    iXofs = atoi( pfSideXOffset->GetText() );
                    iYofs = atoi( pfSideYOffset->GetText() );
                }
                else
                {
                    iXofs = atoi( pfSideXOffset->GetText() );
                    iYofs = atoi( pfSideYOffset->GetText() );
                }
                if( TexAlign( iTexture, &iXofs, &iYofs, fTopOrigin ) )
                {
                    pfSideXOffset->SetInteger( iXofs );
                    pfSideYOffset->SetInteger( iYofs );
                }
                break;
            default:
                // call default handler
                return fldString::OnEvent( event );
            }

            return daContinue;
        }

        // all other events, call default handler
        return fldString::OnEvent( event );
    }

    int OnExit()
    {
        const char* pszString = GetText();
        int iTexture;

        iTexture = texture_toint( (char*) pszString );

        // set the new texture
        if( this == pfTex0Name )
            pfTexture0->SetWall( iTexture );
        else if( this == pfTex1Name )
        {
            pfTexture1->SetWall( iTexture );
            if( iTexture != -1 && pfTwoSided->GetState() == 1 &&
                texlist[iTexture].ndescrip > 1 )
            {
                message_box( MB_OK, "Warning!",
                    "The full texture you have entered here is not\n"
                    "\"transparent-able\", which means it will not\n"
                    "be displayed properly as the full texture of\n"
                    "a two-sided line." );
            }
        }
        else if( this == pfTex2Name )
            pfTexture2->SetWall( iTexture );

        return fldString::OnExit();
    }

private:
    char szSearch[10];
};


int fvSpecialString( const char* pszText )
{
    if( (!fEditMarked || pszText[0]) &&
        SpcStrToType( (char*) pszText ) == -1 )
    {
        message_box( MB_OK, "Hey!",
            "'%s' isn't a valid special type.  Press F2 to\n"
            "get a list of specials.", pszText );
        return valNotOkay;
    }

    return valOkay;
}


static int fvTextureName( const char* pszText )
{
    if( (!fEditMarked || pszText[0]) &&
        texture_toint( (char*) pszText ) == -1 )
    {
        message_box( MB_OK, "Hey!",
            "'%s' isn't a valid texture.  Press F2 to access the\n"
            "texture browser.", pszText );
        return valNotOkay;
    }

    return valOkay;
}

static char szArgNames[5][20];
static char szArgDescs[5][40];

int iThingArgs;

void UpdateSpecialFields( fldString* pField )
{
    if( !fHexen )
        return;

    // set args texts
    char *pszArg;
    char szArgString[64];
    Field **pfArgs = (Field**) pField->pUserData;
    stText *plbPrompt;

    memset( szArgString, 0, sizeof szArgString );

    if( iThingArgs == 2 )
    {
        pgdCurGame->stlThingArgs->IdToStr(
            ThgStrToType( (char*) pField->GetText() ), szArgString );
    }
    else
    {
        stlActArgs->IdToStr( stlActTypes->StrToId( pField->GetText() ),
            szArgString );
    }
    pszArg = szArgString;

    for( int i = 0; i < 5; i++ )
    {
        char *p = pszArg;
        while( *p && *p != ';' && *p != ' ' && *p != '(' ) p++;
        *p = 0;
        plbPrompt = (stText*) pfArgs[i]->GetPrompt();

        strcpy( szArgNames[i], pszArg );

        if( pszArg[0] == '$' )
            ++pszArg;

        plbPrompt->SetText( pszArg );

        bool fSkip = (pfArgs[i]->GetStyle() & csSkip) ? 1 : 0;
        if( !pszArg[0] && iThingArgs != 1 )
        {
            pfArgs[i]->SetStyle( csSkip, ON );
            if( !fSkip )
                pfArgs[i]->OnDraw();
        }
        else if( pszArg[0] || iThingArgs == 1 )
        {
            pfArgs[i]->SetStyle( csSkip, OFF );
            if( fSkip )
                pfArgs[i]->OnDraw();
        }

        ++p;
        while( *p && *p == 32 && *p != '(' ) p++;

        szArgDescs[i][0] = 0;

        if( p[0] == '(' )   // get arg desc
        {
            strccpy( szArgDescs[i], p+1, ")" );
            while( *p && *p != ')' ) p++;
            if( p[0] == ')' )
                ++p;
        }
     //   else if( p[0] == ';' )
     //       --p;

        pszArg = p;
    }
}

int ffClearArgs( Field* pField )
{
    fldString* pfSpecial = (fldString*) pField->GetParent()->GetField( "SpecialString" );
    fldString **pfArgs = (fldString**) pfSpecial->pUserData;

    for( int i = 0; i < 5; i++ )
    {
        if( pfArgs[i]->GetStyle() & csSkip )
        {
            pfArgs[i]->SetInteger( 0 );
        }
    }

    return daContinue;
}

StrList *stlPopup;

int ffArgEnter( fldString* pField )
{
    // SpecialArgN
    const char* pszName = pField->GetName();
    int argno = pszName[10] - '0';
    stText *plbArgText = (stText*) pField->GetParent()->GetLabel( "ArgText" );

    if( szArgNames[argno][0] == '$' )
    {
        // this has a popup list - set the arg description text
        // to the description of the item from the popup.
        stlPopup = GetPopupList( szArgNames[argno]+1 );
        const char* pszStr = stlPopup->IdToStr( pField->GetInteger() );
        if( pszStr )
            plbArgText->SetText( pszStr );
        else
            plbArgText->SetText( "Press F2 to make a selection." );
    }
    else
    {
        plbArgText->SetText( szArgDescs[argno] );
        stlPopup = NULL;
    }

    return daContinue;
}

int ffArgExit( fldString* pField )
{
    stText *plbArgText = (stText*) pField->GetParent()->GetLabel( "ArgText" );
    plbArgText->SetText( "" );
    stlPopup = NULL;

    return TRUE;
}

int ffArgPickPopup( fldString* pField )
{
    if( !stlPopup )
    {
        message_box( MB_OK, st_dcksays,
            "There are no choices available for this argument." );
        return daContinue;
    }

    PickDialog PopupDlg( "PickGeneric", NULL, stlPopup, NULL );
    PopupDlg.SetTitle( "Pick an item" );

    if( PopupDlg.Process( stlPopup->IdToStr( pField->GetInteger() ), TRUE ) ==
                daExitSave )
    {
        pField->SetInteger( stlPopup->StrToId( PopupDlg.GetSelText() ) );
        ffArgEnter( pField );
    }

    return daContinue;
}

static int ffFindID( fldString* pField )
{
    // 'specialargn'
    const char *p = pField->GetName();

    if( !strcmpi( szArgNames[p[10]-'0'], "PolyID" ) )
    {
        // find a new polyid if this is a polyid field
        pField->SetInteger( firstfreepolyid() );
    }

    if( !strcmpi( szArgNames[p[10]-'0'], "LineID" ) )
    {
        // find a new line ID
        short tag = 0, i, done = 0;

        while( !done )
        {
            ++tag;
            done = 1;

            for( i = 0; i < nline; i++ )
            {
                if( linelist[i].special == 121 && linelist[i].arg0 == tag )
                {
                    done = 0;
                    break;
                }
            }
        }

        pField->SetInteger( tag );
    }

    return daContinue;
}

void SetArgFunctions( Field** pfArgs )
{
    for( int i = 0; i < 5; i++ )
    {
        pfArgs[i]->SetOnEnter( (FFUNC) ffArgEnter );
        pfArgs[i]->SetOnChange( (FFUNC) ffArgEnter );
        pfArgs[i]->SetOnExit( (FFUNC) ffArgExit );
        pfArgs[i]->SetUserEvent( etKey, KEY_F2, (FUFUNC) ffArgPickPopup );
        pfArgs[i]->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) ffArgPickPopup );
        pfArgs[i]->SetMessage( "Type in the value or press F2 to pick from a list (if available.)" );
        pfArgs[i]->SetUserEvent( etKey, 'F', (FUFUNC) ffFindID );
        pfArgs[i]->SetUserEvent( etKey, KEY_ALTF, (FUFUNC) ffFindID );
    }
}

int ffPickSpecials( fldString* pField )
{
    PickDialog PDlgSpecials( "PickGeneric", stlActSections, stlActTypes,
        NULL );

    PDlgSpecials.SetTitle( "Pick Specials" );

    int iRvl = PDlgSpecials.Process( pField->GetText() );
    if( iRvl == daExitSave )
    {
        // set text
        const char* pszSelText = PDlgSpecials.GetSelText();
        pField->SetText( pszSelText );
        UpdateSpecialFields( pField );
        pField->SetFirstEdit( TRUE );
    }

    return daContinue;
}

int ffSpecialStringExit( fldString *pField )
{
    const char* pszText = pField->GetText();

    if( isdigit( pszText[0] ) )
    {
        // convert to string
        char *pszStr = stlActTypes->IdToStr( atoi( pszText ) );

        if( pszStr == NULL )
        {
            message_box( MB_OK, st_dcksays,
                "That's not a valid special type. Try again." );
            pField->SetFirstEdit( TRUE );
            return FALSE;
        }

        pField->SetText( pszStr );
        UpdateSpecialFields( pField );
        pField->SetFirstEdit( TRUE );
    }

    if( pszText[0] && SpcStrToType( (char*) pszText ) == -1 )
        ffPickSpecials( pField );

    return TRUE;
}

int ffSpecialStringEnterKey( fldString *pField )
{
    const char* pszText = pField->GetText();

    if( isdigit( pszText[0] ) )
    {
        // convert to string
        char *pszStr = stlActTypes->IdToStr( atoi( pszText ) );

        if( pszStr == NULL )
        {
            message_box( MB_OK, st_dcksays,
                "That's not a valid special type. Try again." );
            pField->SetFirstEdit( TRUE );
            return daContinue;
        }

        pField->SetText( pszStr );
        UpdateSpecialFields( pField );
        pField->SetFirstEdit( TRUE );
    }

    if( pszText[0] && SpcStrToType( (char*) pszText ) == -1 )
        ffPickSpecials( pField );
    else
        return -1;

    return daContinue;
}

int ffSpecialStringChanged( fldString *pField )
{
    UpdateSpecialFields( pField );
    return daContinue;
}

int dfSetSpecials( Dialog* pDialog )
{
    fldString* pfSpecial = (fldString*) pDialog->GetField( "SpecialString" );
    UpdateSpecialFields( pfSpecial );
    return daContinue;
}

static void SetSideTransfer( int iSide )
{
    // set transfer buffers for iSide
    if( iSide == 1 )
    {
        pfTexture0->SetTransferBuf( (PTR) &iS1Tex0 );
        pfTexture1->SetTransferBuf( (PTR) &iS1Tex1 );
        pfTexture2->SetTransferBuf( (PTR) &iS1Tex2 );
        pfTex0Name->SetTransferBuf( (PTR) szS1Tex0Name );
        pfTex1Name->SetTransferBuf( (PTR) szS1Tex1Name );
        pfTex2Name->SetTransferBuf( (PTR) szS1Tex2Name );
        pfSideSector->SetTransferBuf( (PTR) &iS1Sector );
        pfSideXOffset->SetTransferBuf( (PTR) &iS1XOfs );
        pfSideYOffset->SetTransferBuf( (PTR) &iS1YOfs );
    }
    else if( iSide == 2 )
    {
        pfTexture0->SetTransferBuf( (PTR) &iS2Tex0 );
        pfTexture1->SetTransferBuf( (PTR) &iS2Tex1 );
        pfTexture2->SetTransferBuf( (PTR) &iS2Tex2 );
        pfTex0Name->SetTransferBuf( (PTR) szS2Tex0Name );
        pfTex1Name->SetTransferBuf( (PTR) szS2Tex1Name );
        pfTex2Name->SetTransferBuf( (PTR) szS2Tex2Name );
        pfSideSector->SetTransferBuf( (PTR) &iS2Sector );
        pfSideXOffset->SetTransferBuf( (PTR) &iS2XOfs );
        pfSideYOffset->SetTransferBuf( (PTR) &iS2YOfs );
    }
}

static void EnableSide( int iSide, bool fFirst = FALSE )
/*-------------------------------------------------------------------------*
 * Enables a particular sidedef.
 * 1 = right, 2 = left.
 *-------------------------------------------------------------------------*/
{
    iCurSide = iSide;

    // store current data from the side fields in the side fields.
    pfTexture0->GetParent()->Transfer( transFromField );

    // set new transfer buffers
    SetSideTransfer( iSide );

    // store variables in fields
    pfTexture0->GetParent()->Transfer( transToField );

    // update fields
    if( !fFirst )
    {
        pfTexture0->OnDraw();
        pfTexture1->OnDraw();
        pfTexture2->OnDraw();
        pfSideSector->OnDraw();
        pfSideXOffset->OnDraw();
        pfSideYOffset->OnDraw();
        pfTex0Name->OnDraw();
        pfTex1Name->OnDraw();
        pfTex2Name->OnDraw();
    }

    char szBuf[128];

    // change side button text
    sprintf( szBuf, "Side %d", iCurSide );
    pfSideSwitch->SetText( szBuf );

    // enable/disable the side delete button
    if( iCurSide == 1 )
        pfSideDelete->SetStyle( csSkip, TRUE );
    else
        pfSideDelete->SetStyle( csSkip, FALSE );
    pfSideDelete->OnDraw();
}

static int pbSideDelete()
/*-------------------------------------------------------------------------*
 * Delete the current side - valid for side 2 only.
 *-------------------------------------------------------------------------*/
{
    fDelSide2 = TRUE;
    fGotSide2 = FALSE;

    // new values for these..
    pfImpassable->SetState( 1 );
    pfTwoSided->SetState( 0 );

    // go back to side 1
    EnableSide( 1 );

    return daContinue;
}

static int pbSideToggle()
/*-------------------------------------------------------------------------*
 * Toggle the current side.
 *-------------------------------------------------------------------------*/
{
    if( iCurSide == 1 )
    {
        if( fDelSide2 )
        {
            // side 2 was deleted before - do they just want to restore it?
            if( message_box( MB_YESNO, "Wait a minute!",
                " The second side was deleted before - do you want to\n"
                "restore it?" ) == MB_YES )
            {
                fDelSide2 = FALSE;
                fGotSide2 = TRUE;
                pfImpassable->SetState( 0 );
                pfTwoSided->SetState( 1 );
            }
        }

        if( !fGotSide2 )
        {
            if( message_box( MB_YESNO, "Wait a minute!",
                " There is no second side defined for the line(s)\n"
                "you're editing.  Do you want to add one?" ) == MB_NO )
                return daContinue;
            fGotSide2 = fAddSide2 = TRUE;   // side 2 data is valid
            // set blank side2 data
            iS2XOfs = iS2YOfs = iS2Sector = iS2Tex0 = iS2Tex1 = iS2Tex2 = 0;
            // set new values for impassable/2s
            pfImpassable->SetState( 0 );
            pfTwoSided->SetState( 1 );
        }
        iCurSide = 2;
    }
    else
        iCurSide = 1;
    EnableSide( iCurSide );
    return daContinue;
}

static int dfLineDlgUp( Dialog* pDialog )
{
    EnableSide( iCurSide, TRUE );
    dfSetSpecials( pDialog );
    return daContinue;
}

static int checksector( short thesec, short oppsec )
/*-------------------------------------------------------------------------*
 * Returns sector number if everything's coo, otherwise -1.
 *-------------------------------------------------------------------------*/
{
    short   rvl;

    if( thesec < 0 || thesec >= nsector )
    {
        if( message_box( MB_YESNO, st_dcksays,
            "The sector reference you've entered (%d) is out of range.\n"
            "Would you like to create a new sector (Yes) or re-enter\n"
            "the number (No)?", thesec ) == MB_NO )
            return -1;

        rvl = message_box( MB_YESNOCANCEL, st_dcksays,
            "The new sector is number %d.\n"
            "Note: Because you've used this method to create a new sector,\n"
            _ABBREV_" could not set any real data in it.  Do you want to edit\n"
            "the new sector now?", nsector );

        if( rvl == MB_CANCEL )
            return -1;

        thesec = obj_insert( MODE_SECTOR, oppsec, 0, 0 );

        if( rvl == MB_YES )
            sector_edit( thesec, 0 );
    }

    return thesec;
}

static bool _valtex( char *pszTexture )
/*-------------------------------------------------------------------------*
 * Validates a texture field.
 *-------------------------------------------------------------------------*/
{
    /* oopsies! */
    if( !(!pszTexture[0] && fEditMarked) && texture_toint( pszTexture ) == -1 )
    {
        message_box( MB_OK, st_dcksays,
                    "The texture '%s' is not valid!  I can't save this\n"
                    "information until you re-enter it.", pszTexture );
        return FALSE;
    }

    return TRUE;
}

static int dvLineDlgValidate( Dialog* pDialog )
{
    int idx, oppsec = FI_NODATA, thesec = FI_NODATA, newsec;

    pDialog->Transfer( transFromField );

    /* if there's a side 2, get its sector so we can copy that
     * sector's attributes in checksector() */
    if( fGotSide2 )
        oppsec = iS2Sector;
    thesec = iS1Sector;

    if( thesec != FI_NODATA )
    {
        /* first side */
        if( (newsec = checksector( thesec, oppsec )) == -1 )
            return valNotOkay;
        iS1Sector = newsec;
        if( iCurSide == 1 )
            pfSideSector->SetInteger( newsec );
    }

    if( oppsec != FI_NODATA )
    {
        /* do the second side .. just reverse the parameters */
        if( (newsec = checksector( oppsec, thesec )) == -1 )
            return valNotOkay;
        iS2Sector = newsec;
        if( iCurSide == 2 )
            pfSideSector->SetInteger( newsec );
    }

    // validate textures
    Field* pfGoto = NULL;

    if( fGotSide1 )
    {
        if( !_valtex( szS1Tex0Name ) )
            pfGoto = pfTex0Name;
        if( !_valtex( szS1Tex1Name ) )
            pfGoto = pfTex1Name;
        if( !_valtex( szS1Tex2Name ) )
            pfGoto = pfTex2Name;

        if( pfGoto )
        {
            if( iCurSide != 1 )
                EnableSide( 1 );
            pDialog->GoToItem( pfGoto );
            return valNotOkay;
        }
    }

    if( fGotSide2 )
    {
        if( !_valtex( szS2Tex0Name ) )
            pfGoto = pfTex0Name;
        if( !_valtex( szS2Tex1Name ) )
            pfGoto = pfTex1Name;
        if( !_valtex( szS2Tex2Name ) )
            pfGoto = pfTex2Name;

        if( pfGoto )
        {
            if( iCurSide != 2 )
                EnableSide( 2 );
            pDialog->GoToItem( pfGoto );
            return valNotOkay;
        }
    }

    return valOkay;  /* everything's spunky */
}

void MakeSpecialFields( Field* pfSpecial, Field** pfArgs )
{
    pfSpecial->pUserData = (PTR) pfArgs;
    pfSpecial->SetUserEvent( etKey, KEY_F2, (FUFUNC) ffPickSpecials );
    pfSpecial->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) ffSpecialStringEnterKey );
    pfSpecial->SetOnValidate( (FVFUNC) fvSpecialString );
    pfSpecial->SetOnChange( (FFUNC) ffSpecialStringChanged );
    pfSpecial->SetOnExit( (FFUNC) ffSpecialStringExit );
    pfSpecial->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) ffPickSpecials );

    if( pfArgs[0] )
        SetArgFunctions( pfArgs );
}

void LinedefEdit( int iObject, int iJumpSide )
/*-------------------------------------------------------------------------*
 * Edits a linedef, or a series of if fEditMarked == TRUE.
 * iJumpSide == the side to jump to.
 *-------------------------------------------------------------------------*/
{
    static  int iLastSide = 1;
    bool    fUnmarkObject = FALSE;
    Dialog* pDialog = new Dialog;

    pDialog->Create( szDSFile, fHexen ? "HexenLineDlg" : "DoomLineDlg" );  // load up the resource

    // dialog field variables & transfer variables
    char        fszSpecial[60];
    int         iArg0, iArg1, iArg2, iArg3, iArg4;
    int         iActivation = -2, iSpecial;
    bool        fImpassable, fTwoSided, fBlockMonster, fUnpegTop, fRepeatable,
                fUnpegBottom, fSecret, fBlockSound, fNeverMap, fAlwaysMap;

    fldRadio    *pfActivation;

    iThingArgs = 0;

    #include "vt_line.h"

    pDialog->VarSetup( vtLineTransfer );

    fAddSide2 = FALSE;
    fDelSide2 = FALSE;
    fGotSide1 = FALSE;
    fGotSide2 = FALSE;

    if( iJumpSide == -1 )
        iJumpSide = iLastSide;

    if( iObject != -1 && !obj_ismarked( iObject ) )
        fUnmarkObject = TRUE, obj_mark( iObject );

    int nObjMarked = obj_nmarked();

    if( nObjMarked == 1 )
    {
        fEditMarked = FALSE;
        for( int i = 0; i < MAXOBJECT; i++ )
            if( obj_ismarked( i ) )
            {
                iObject = i;
                break;
            }
    }
    else if( !nObjMarked )
        return;
    else
        fEditMarked = TRUE;

    // fill up them vars with some info
    for( int i = 0; i < MAXOBJECT; i++ )
    {
        LINEDEF* pLine = &linelist[i];

        if( obj_ismarked( i ) )
        {
            // get some data
            if( iActivation == -2 )
            {
                // initial values
                sprintf( fszSpecial, "%s", SpcTypeToStr( pLine->special ) );
                iSpecial = pLine->special;

                if( fHexen )
                {
                    iActivation = LA_GETSPAC( pLine->attribs );
                    iArg0 = pLine->arg0;
                    iArg1 = pLine->arg1;
                    iArg2 = pLine->arg2;
                    iArg3 = pLine->arg3;
                    iArg4 = pLine->arg4;
                    fRepeatable = bittobool( pLine->attribs, LA_REPEAT );
                }
                else
                {
                    iSecTag = pLine->sectag;
                    iActivation = 0;
                }

                fImpassable = bittobool( pLine->attribs, LA_IMPASS );
                fTwoSided = bittobool( pLine->attribs, LA_TSIDE );
                fBlockMonster = bittobool( pLine->attribs, LA_MBLOCKER );
                fUnpegTop = bittobool( pLine->attribs, LA_UUNPEGGED );
                fUnpegBottom = bittobool( pLine->attribs, LA_LUNPEGGED );
                fSecret = bittobool( pLine->attribs, LA_SECRET );
                fBlockSound = bittobool( pLine->attribs, LA_SBLOCK );
                fNeverMap = bittobool( pLine->attribs, LA_NOTONMAP );
                fAlwaysMap = bittobool( pLine->attribs, LA_AUTOONMAP );
                fNeverMap = bittobool( pLine->attribs, LA_NOTONMAP );
            }

            if( !fGotSide1 && pLine->rsdef != -1 )
            {
                SIDEDEFM* pSide = &sidelist[pLine->rsdef];
                iS1XOfs = pSide->wxofs;
                iS1YOfs = pSide->wyofs;
                iS1Sector = pSide->sector;
                iS1Tex0 = pSide->utxidx;
                iS1Tex1 = pSide->ftxidx;
                iS1Tex2 = pSide->ltxidx;
                fGotSide1 = TRUE;
            }
            if( !fGotSide2 && pLine->lsdef != -1 )
            {
                SIDEDEFM* pSide = &sidelist[pLine->lsdef];
                iS2XOfs = pSide->wxofs;
                iS2YOfs = pSide->wyofs;
                iS2Sector = pSide->sector;
                iS2Tex0 = pSide->utxidx;
                iS2Tex1 = pSide->ftxidx;
                iS2Tex2 = pSide->ltxidx;
                fGotSide2 = TRUE;
            }

            // get regular data
            if( iSpecial != pLine->special )
                fszSpecial[0] = 0;

            if( fHexen )
            {
                if( iActivation != LA_GETSPAC( pLine->attribs ) )
                    iActivation = -1;
                if( iArg0 != pLine->arg0 )
                    iArg0 = FI_NODATA;
                if( iArg1 != pLine->arg1 )
                    iArg1 = FI_NODATA;
                if( iArg2 != pLine->arg2 )
                    iArg2 = FI_NODATA;
                if( iArg3 != pLine->arg3 )
                    iArg3 = FI_NODATA;
                if( iArg4 != pLine->arg4 )
                    iArg4 = FI_NODATA;
                if( fRepeatable != 2 && fRepeatable != bittobool( pLine->attribs, LA_REPEAT ) )
                    fRepeatable = 2;
            }
            else
            {
                if( iSecTag != pLine->sectag )
                    iSecTag = FI_NODATA;
            }

            if( fImpassable != 2 && fImpassable != bittobool( pLine->attribs, LA_IMPASS ) )
                fImpassable = 2;
            if( fTwoSided != 2 && fTwoSided != bittobool( pLine->attribs, LA_TSIDE ) )
                fTwoSided = 2;
            if( fBlockMonster != 2 && fBlockMonster != bittobool( pLine->attribs, LA_MBLOCKER ) )
                fBlockMonster = 2;
            if( fUnpegTop != 2 && fUnpegTop != bittobool( pLine->attribs, LA_UUNPEGGED ) )
                fUnpegTop = 2;
            if( fUnpegBottom != 2 && fUnpegBottom != bittobool( pLine->attribs, LA_LUNPEGGED ) )
                fUnpegBottom = 2;
            if( fSecret != 2 && fSecret != bittobool( pLine->attribs, LA_SECRET ) )
                fSecret = 2;
            if( fBlockSound != 2 && fBlockSound != bittobool( pLine->attribs, LA_SBLOCK ) )
                fBlockSound = 2;
            if( fNeverMap != 2 && fNeverMap != bittobool( pLine->attribs, LA_NOTONMAP ) )
                fNeverMap = 2;
            if( fAlwaysMap != 2 && fAlwaysMap != bittobool( pLine->attribs, LA_AUTOONMAP ) )
                fAlwaysMap = 2;
            if( fNeverMap != 2 && fNeverMap != bittobool( pLine->attribs, LA_NOTONMAP ) )
                fNeverMap = 2;

            if( pLine->rsdef != -1 )
            {
                SIDEDEFM* pSide = &sidelist[pLine->rsdef];
                if( iS1XOfs != pSide->wxofs )
                    iS1XOfs = FI_NODATA;
                if( iS1YOfs != pSide->wyofs )
                    iS1YOfs = FI_NODATA;
                if( iS1Sector != pSide->sector )
                    iS1Sector = FI_NODATA;
                if( iS1Tex0 != pSide->utxidx )
                    iS1Tex0 = -1;
                if( iS1Tex1 != pSide->ftxidx )
                    iS1Tex1 = -1;
                if( iS1Tex2 != pSide->ltxidx )
                    iS1Tex2 = -1;
            }
            if( pLine->lsdef != -1 )
            {
                SIDEDEFM* pSide = &sidelist[pLine->lsdef];
                if( iS2XOfs != pSide->wxofs )
                    iS2XOfs = FI_NODATA;
                if( iS2YOfs != pSide->wyofs )
                    iS2YOfs = FI_NODATA;
                if( iS2Sector != pSide->sector )
                    iS2Sector = FI_NODATA;
                if( iS2Tex0 != pSide->utxidx )
                    iS2Tex0 = -1;
                if( iS2Tex1 != pSide->ftxidx )
                    iS2Tex1 = -1;
                if( iS2Tex2 != pSide->ltxidx )
                    iS2Tex2 = -1;
            }

            if( !fEditMarked )
                break;
        }
    }

    if( fGotSide1 )
    {
        if( iS1Tex0 >= 0 )  texture_tostr( szS1Tex0Name, iS1Tex0 );
        else                szS1Tex0Name[0] = 0;
        if( iS1Tex1 >= 0 )  texture_tostr( szS1Tex1Name, iS1Tex1 );
        else                szS1Tex1Name[0] = 0;
        if( iS1Tex2 >= 0 )  texture_tostr( szS1Tex2Name, iS1Tex2 );
        else                szS1Tex2Name[0] = 0;
    }
    if( fGotSide2 )
    {
        if( iS2Tex0 >= 0 )  texture_tostr( szS2Tex0Name, iS2Tex0 );
        else                szS2Tex0Name[0] = 0;
        if( iS2Tex1 >= 0 )  texture_tostr( szS2Tex1Name, iS2Tex1 );
        else                szS2Tex1Name[0] = 0;
        if( iS2Tex2 >= 0 )  texture_tostr( szS2Tex2Name, iS2Tex2 );
        else                szS2Tex2Name[0] = 0;
    }

    if( iJumpSide == 2 && !fGotSide2 )
        iJumpSide = 1;

    iCurSide = iJumpSide;
    pDialog->SetOnEnter( (DFUNC) dfLineDlgUp );
    pDialog->SetOnValidate( (DVFUNC) dvLineDlgValidate );

    pfTexture0 = new fldWallTex;
    pfTexture1 = new fldWallTex;
    pfTexture2 = new fldWallTex;
    pDialog->REPLACEREGION( "Texture0", pfTexture0 );
    pDialog->REPLACEREGION( "Texture1", pfTexture1 );
    pDialog->REPLACEREGION( "Texture2", pfTexture2 );

    pfTex0Name = new fldTexName;
    pfTex1Name = new fldTexName;
    pfTex2Name = new fldTexName;
    fldString* pfTmpString;
    pfTmpString = (fldString*) pDialog->GetField( "Tex0Name" );
    pfTmpString->Replace( pfTex0Name );
    pfTmpString = (fldString*) pDialog->GetField( "Tex1Name" );
    pfTmpString->Replace( pfTex1Name );
    pfTmpString = (fldString*) pDialog->GetField( "Tex2Name" );
    pfTmpString->Replace( pfTex2Name );
    pfTex0Name->SetOnValidate( (FVFUNC) fvTextureName );
    pfTex1Name->SetOnValidate( (FVFUNC) fvTextureName );
    pfTex2Name->SetOnValidate( (FVFUNC) fvTextureName );

    MakeSpecialFields( pfSpecial, (Field**) pfArgs );

    SetSideTransfer( iJumpSide );

    if( fEditMarked )
    {
        // set radio buttons & checkboxes to 3-states
        pfImpassable->Set3State( TRUE );
        pfTwoSided->Set3State( TRUE );
        pfBlockMonster->Set3State( TRUE );
        pfUnpegTop->Set3State( TRUE );
        pfUnpegBottom->Set3State( TRUE );
        pfSecret->Set3State( TRUE );
        pfBlockSound->Set3State( TRUE );
        pfNeverMap->Set3State( TRUE );
        pfAlwaysMap->Set3State( TRUE );

        if( fHexen )
        {
            pfActivation->SetType2( TRUE );
            pfRepeatable->Set3State( TRUE );
        }
    }

    pDialog->SetMultiEdit( fEditMarked );

    int iRvl = pDialog->Process();

    iLastSide = iCurSide;

    delete pDialog;

    if( iRvl == daExitSave )
    {
        // convert to integers temporary stuff
        iSpecial = fszSpecial[0] ? SpcStrToType( fszSpecial ) : -1;

        // convert data
        int iAddSide2 = -1; // ask them

        for( i = 0; i < MAXOBJECT; i++ )
        {
            LINEDEF* pLine = &linelist[i];

            if( obj_ismarked( i ) )
            {
                if( iSpecial != -1 )
                {
                    if( pLine->special != iSpecial )
                        post_fullupdate();
                    pLine->special = iSpecial;
                }

                if( fHexen )
                {
                    if( iArg0 != FI_NODATA )
                        pLine->arg0 = iArg0;
                    if( iArg1 != FI_NODATA )
                        pLine->arg1 = iArg1;
                    if( iArg2 != FI_NODATA )
                        pLine->arg2 = iArg2;
                    if( iArg3 != FI_NODATA )
                        pLine->arg3 = iArg3;
                    if( iArg4 != FI_NODATA )
                        pLine->arg4 = iArg4;
                    if( iActivation != -1 )
                        LA_SETSPAC( pLine->attribs, iActivation );
                }
                else
                {
                    if( iSecTag != FI_NODATA )
                        pLine->sectag = iSecTag;
                }

                bitset( pLine->attribs, fRepeatable, LA_REPEAT );
                bitset( pLine->attribs, fImpassable, LA_IMPASS );
                bitset( pLine->attribs, fTwoSided, LA_TSIDE );
                bitset( pLine->attribs, fBlockMonster, LA_MBLOCKER );
                bitset( pLine->attribs, fUnpegTop, LA_UUNPEGGED );
                bitset( pLine->attribs, fUnpegBottom, LA_LUNPEGGED );
                bitset( pLine->attribs, fSecret, LA_SECRET );
                bitset( pLine->attribs, fBlockSound, LA_SBLOCK );
                bitset( pLine->attribs, fNeverMap, LA_NOTONMAP );
                bitset( pLine->attribs, fAlwaysMap, LA_AUTOONMAP );

                if( fGotSide1 )
                {
                    SIDEDEFM* pSide = &sidelist[pLine->rsdef];
                    if( iS1Tex0 != -1 )
                        pSide->utxidx = iS1Tex0;
                    if( iS1Tex1 != -1 )
                        pSide->ftxidx = iS1Tex1;
                    if( iS1Tex2 != -1 )
                        pSide->ltxidx = iS1Tex2;
                    if( iS1XOfs != FI_NODATA )
                        pSide->wxofs = iS1XOfs;
                    if( iS1YOfs != FI_NODATA )
                        pSide->wyofs = iS1YOfs;
                    if( iS1Sector != FI_NODATA )
                        pSide->sector = iS1Sector;
                }

                if( fGotSide2 )
                {
                    if( pLine->lsdef == -1 )
                    {
                        if( fAddSide2 )
                            iAddSide2 = TRUE;

                        if( fEditMarked && iAddSide2 == -1 )
                        {
                            // check to see if we're supposed to add sides
                            // to those lines that don't have second sides.
                            if( MB_YES == message_box( MB_YESNO, st_dcksays,
                                "Some of the lines you're editing don't have second sides.  Do\n"
                                "you want me to add second sides to these lines (pick Yes), or\n"
                                "to leave them one-sided (pick No)?" ) )
                            {
                                iAddSide2 = TRUE;
                            }
                            else
                                iAddSide2 = FALSE;
                        }

                        if( iAddSide2 == FALSE )    // no add side 2
                        {
                            // reset impass/2sided bits to one-sided line
                            pLine->attribs |= LA_IMPASS;
                            pLine->attribs &= ~LA_TSIDE;
                            continue;
                        }

                        // add a side
                        pLine->lsdef = obj_insert( MODE_SIDEDEF, -1, 0, 0 );
                        post_fullupdate();  // different line colors, etc.
                    }

                    SIDEDEFM* pSide = &sidelist[pLine->lsdef];
                    if( iS2Tex0 != -1 )
                        pSide->utxidx = iS2Tex0;
                    if( iS2Tex1 != -1 )
                        pSide->ftxidx = iS2Tex1;
                    if( iS2Tex2 != -1 )
                        pSide->ltxidx = iS2Tex2;
                    if( iS2XOfs != FI_NODATA )
                        pSide->wxofs = iS2XOfs;
                    if( iS2YOfs != FI_NODATA )
                        pSide->wyofs = iS2YOfs;
                    if( iS2Sector != FI_NODATA )
                        pSide->sector = iS2Sector;

                    nMadeChanges++;
                }
                else if( fDelSide2 && pLine->lsdef != -1 )
                {
                    // delete the side and fix the textures
                    pLine->lsdef = -1;
                    _WFfixtexnames( i, 0, 0 );
                    post_fullupdate();  // different line colors, etc.
                }

                if( !fEditMarked )
                    break;
            }
        }
    }

    ResetKeys();

    if( fUnmarkObject )
        obj_unmark( iObject );
}
