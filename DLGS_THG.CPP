/*-------------------------------------------------------------------------*
 * DLGS_THG.CPP:
 * Dialogs for editing things, etc.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "fields.hpp"

/*-------------------------------------------------------------------------*
 *
 * Globals
 *
 *-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*
 *
 * Thing edit dialog box
 *
 *-------------------------------------------------------------------------*/

static int ffThingStringChanged( fldString* pString );

short firstfreetid();
short firstfreepolyid();

#include "pickdlg.hpp"

static bool fEditMarked, fDoPolyID;
static fldString* pfThingString;
static fldString* pfSpecial, *pfAngle;
static stText* plbAnglePrompt;

// dlgs_line.cpp:
int dfSetSpecials( Dialog* pDialog );
void UpdateSpecialFields( fldString* pField );
void MakeSpecialFields( Field* pfSpecial, Field** pfArgs );
int ffClearArgs( Field* pField );
extern int iThingArgs;

static int ffFindPolyID( fldString* pField )
{
    if( fDoPolyID )
    {
        // find a new polyid
        pField->SetInteger( firstfreepolyid() );
    }

    return daContinue;
}

int ffFindTID( fldString* pField )
{
    /* Find the first free tag! */
    pField->SetInteger( firstfreetid() );
    return daContinue;
}

int fvThingString( const char* pszText )
{
    if( (!fEditMarked || pszText[0]) &&
        ThgStrToType( (char*) pszText ) == -1 )
    {
        message_box( MB_OK, "Hey!",
            "'%s' isn't a valid Item type.  Press F2 to\n"
            "get a list of Items.", pszText );
        return valNotOkay;
    }

    return valOkay;
}

static fldRegion* pfThingPicture;

void pdchShowThing( const char* pszName )
{
    if( !pfThingPicture->GetParent()->IsVisible() )
        return;

    int iPosInList = stlThingTypes->FindNoKey( pszName, FALSE );
    WADENTRY* we;

    if( iPosInList == -1 )
        we = NULL;
    else
        we = wad_find( pwdIWAD, pgdCurGame->thinginfo[iPosInList].sprite );

    Rect rcRect = pfThingPicture->GetRectInParent();

    pfThingPicture->OnDraw();

    if( we != NULL )    /* now display the picture, eh. */
        we_disppic( rcRect.left+1, rcRect.top+1, 199, 127, pwdIWAD, we );
    else
    {
        gr_setcolor( C_WHITE );
        outtextxy( rcRect.left+2, rcRect.top+2, "No picture available." );
    }
}

int ffPickThings( fldString* pField )
{
    PickDialog PDlgThings( "PickItem", stlThingSections, stlThingTypes,
        (CHFUNC) pdchShowThing );

    PDlgThings.SetTitle( "Pick an Item" );

    pfThingPicture = (fldRegion*) PDlgThings.GetField( "Picture" );

    int iRvl = PDlgThings.Process( pField->GetText() );
    if( iRvl == daExitSave )
    {
        pField->SetText( PDlgThings.GetSelText() );
        ffThingStringChanged( pField );
    }

    return daContinue;
}

static int ffThingStringEnterKey( fldString* pString )
{
    const char* pszText = pString->GetText();

    if( pszText[0] && ThgStrToType( (char*) pszText ) == -1 )
        ffPickThings( pString );
    else
        return -1;

    return daContinue;
}

static int ffPickAngles( fldString* pField )
{
    PickDialog PDlgThings( "PickGeneric", NULL, stlAngles, NULL );

    PDlgThings.SetTitle( "Pick Angle" );

    const char* pszAngle = AngleTypeToStr( pField->GetInteger() );
    int iRvl = PDlgThings.Process( pszAngle, TRUE );
    if( iRvl == daExitSave )
        pField->SetInteger( AngleStrToType( (char*) PDlgThings.GetSelText() ) );

    return daContinue;
}

static int ffThingStringChanged( fldString* pString )
{
    const char* pszText = pString->GetText();
    static bool fThingArgsDisplayed = FALSE;

    pfThingPicture = (fldRegion*) pString->GetParent()->GetField( "Picture" );

    // use strlen()+1 because we want to match the trailing 0 -- this is the
    // only way to ensure we match the entire string.
    int iPosInList = stlThingTypes->Find( pszText, strlen( pszText )+1 );

    if( iPosInList != -1 )
    {
        // copy in final
        char szBuffer[128];
        pString->SetText( stlThingTypes->GetAt( szBuffer, iPosInList ) );
    }

    pdchShowThing( pszText );

    if( !fHexen )
        return daContinue;

    int iId;
    if( (iId = ThgStrToType( (char*) pString->GetText() )) != -1 &&
        pgdCurGame->stlThingArgs->IdToStr( iId ) != NULL )
    {
        // use the thing type's args

        iThingArgs = 2;
        UpdateSpecialFields( pString );
        pfSpecial->SetStyle( csSkip, ON );
        pfSpecial->OnDraw();
        fThingArgsDisplayed = TRUE;
        iThingArgs = 1;
    }
    else if( fThingArgsDisplayed )
    {
        UpdateSpecialFields( pfSpecial );
        pfSpecial->SetStyle( csSkip, OFF );
        pfSpecial->OnDraw();
        fThingArgsDisplayed = FALSE;
    }

    if( inrange( iId, 3000, 3002 ) )
    {
        fDoPolyID = TRUE;
        plbAnglePrompt->SetText( "PolyID:" );
        pfAngle->SetMessage( "Enter the ID of the PolyObject that this is associated with." );
        pfAngle->SetHelp( "Thg_PolyID" );
    }
    else
    {
        fDoPolyID = FALSE;
        plbAnglePrompt->SetText( "Angle:" );
        pfAngle->SetMessage( "Enter the angle that the Item is facing, or press F2 for a list." );
        pfAngle->SetHelp( "Thg_Angle" );
    }

    return daContinue;
}

static int dfThingDlgUp( Dialog* pDialog )
{
    iThingArgs = 1;
    ffThingStringChanged( pfThingString );

    if( fHexen )
    {
        if( !(pfSpecial->GetStyle() & csSkip) )
            UpdateSpecialFields( pfSpecial );
    }

    return daContinue;
}

short thing_edit( short object, short editmarked )
/*-------------------------------------------------------------------------*
 * Edit a thing object.
 *-------------------------------------------------------------------------*/
{
    Dialog*     pThingDlg;
    char        fszThingType[60];
    char        fszSpecial[60];
    bool        fEasy;
    bool        fNormal;
    bool        fHard;
    bool        fFighter;
    bool        fCleric;
    bool        fMage;
    bool        fSingle;
    bool        fCoop;
    bool        fDeathmatch;
    bool        fAmbush;
    bool        fDormant;
    int         iArg0, iArg1, iArg2, iArg3, iArg4;
    int         iTID;
    fldString   *pfArgs[5], *pfTID;
    fldCheckBox *pfEasy, *pfNormal, *pfHard, *pfFighter, *pfCleric,
                *pfMage, *pfSingle, *pfCoop, *pfDeathmatch, *pfAmbush,
                *pfDormant;
    int         iType, iAngle, iSpecial, iAltitude;

    VarTransfer_t vtThings[] =
    {
        { "ThingString",(Field**)&pfThingString,   (PTR) &fszThingType,
            "Enter the Item type, or press F2 for a list." },
        { "TID",        (Field**)&pfTID,           (PTR) &iTID,
            "Enter the Item's ID." },
        { "Angle",      (Field**)&pfAngle,       (PTR) &iAngle,
            NULL },
        { "Altitude",   NULL,                    (PTR) &iAltitude,
            NULL },
        #include "vt_spec.h"
        { "flEasy",     (Field**)&pfEasy,        (PTR) &fEasy,
            NULL },
        { "flNormal",   (Field**)&pfNormal,      (PTR) &fNormal,
            NULL },
        { "flHard",     (Field**)&pfHard,        (PTR) &fHard,
            NULL },
        { "flFighter",  (Field**)&pfFighter,     (PTR) &fFighter,
            NULL },
        { "flCleric",   (Field**)&pfCleric,      (PTR) &fCleric,
            NULL },
        { "flMage",     (Field**)&pfMage,        (PTR) &fMage,
            NULL },
        { "flSingle",   (Field**)&pfSingle,      (PTR) &fSingle,
            NULL },
        { "flCoop",     (Field**)&pfCoop,        (PTR) &fCoop,
            NULL },
        { "flDeathmatch", (Field**)&pfDeathmatch, (PTR) &fDeathmatch,
            NULL },
        { "flAmbush",   (Field**)&pfAmbush,      (PTR) &fAmbush,
            NULL },
        { "flDormant",  (Field**)&pfDormant,     (PTR) &fDormant,
            NULL },

        { NULL, NULL, NULL }
    };

    THING   *t;
    int     otype, nmarked, i, oangle;
    short   rvl = 1;
    short   orx, ory;
    short   xpos, ypos;
    uchar   unmarkobj = 0;

    if( object != -1 && !obj_ismarked( object ) )
        obj_mark( object ), unmarkobj = 1;

    if( editmarked )    nmarked = obj_nmarked();
    else                nmarked = 1;

    if( !nmarked )
        return 0;     /* Nothing to edit anyway */

    /* To make things a little friendlier */
    if( editmarked && nmarked == 1 )
    {
        for( i = 0; i < MAXOBJECT; i++ )
        {
            if( obj_ismarked( i ) )
            {
                object = i;
                break;
            }
        }
        editmarked = 0;
    }

    fEditMarked = editmarked;

    iType = -2;

    for( i = 0, t = thinglist; i < nthing; i++, t++ )
    {
        if( obj_ismarked( i ) )
        {
            if( iType == -2 )
            {
                iType = t->type;
                sprintf( fszThingType, "%s", ThgTypeToStr( t->type ) );

                if( fHexen )
                {
                    iSpecial = t->special;
                    strcpy( fszSpecial, SpcTypeToStr( t->special ) );
                    iArg0 = t->arg0;
                    iArg1 = t->arg1;
                    iArg2 = t->arg2;
                    iArg3 = t->arg3;
                    iArg4 = t->arg4;
                    iAltitude = t->height;
                    iTID = t->tid;
                }

                iAngle = t->angle;

                fAmbush = bittobool( t->attribs, TA_DEAF );     /* set the attributes */
                fEasy = bittobool( t->attribs, TA_SKILL12 );     /* set the attributes */
                fNormal = bittobool( t->attribs, TA_SKILL3 );     /* set the attributes */
                fHard = bittobool( t->attribs, TA_SKILL45 );     /* set the attributes */
                fDeathmatch = bittobool( t->attribs, TA_GDEATHMATCH );     /* set the attributes */
                fSingle = bittobool( t->attribs, TA_GSINGLE );
                fCoop = bittobool( t->attribs, TA_GCOOP );
                fDormant = bittobool( t->attribs, TA_DORMANT );
                fFighter = bittobool( t->attribs, TA_FIGHTER );
                fCleric = bittobool( t->attribs, TA_CLERIC );
                fMage = bittobool( t->attribs, TA_MAGE );
                continue;
            }

            if( iType != t->type )
                fszThingType[0] = 0;
            if( iAngle != t->angle )
                iAngle = FI_NODATA;

            if( fHexen )
            {
                if( iAltitude != t->height )
                    iAltitude = FI_NODATA;
                if( iTID != t->tid )
                    iTID = FI_NODATA;
                if( iSpecial != t->special )
                    fszSpecial[0] = 0;
                if( iArg0 != t->arg0 )
                    iArg0 = FI_NODATA;
                if( iArg1 != t->arg1 )
                    iArg1 = FI_NODATA;
                if( iArg2 != t->arg2 )
                    iArg2 = FI_NODATA;
                if( iArg3 != t->arg3 )
                    iArg3 = FI_NODATA;
                if( iArg4 != t->arg4 )
                    iArg4 = FI_NODATA;
            }

            if( fAmbush != 2 && fAmbush != bittobool( t->attribs, TA_DEAF ) )
                fAmbush = 2;
            if( fEasy != 2 && fEasy != bittobool( t->attribs, TA_SKILL12 ) )
                fEasy = 2;
            if( fNormal != 2 && fNormal != bittobool( t->attribs, TA_SKILL3 ) )
                fNormal = 2;
            if( fHard != 2 && fHard != bittobool( t->attribs, TA_SKILL45 ) )
                fHard = 2;
            if( fDeathmatch != 2 && fDeathmatch != bittobool( t->attribs, TA_GDEATHMATCH ) )
                fDeathmatch = 2;
            if( fDormant != 2 && fDormant != bittobool( t->attribs, TA_DORMANT ) )
                fDormant = 2;
            if( fFighter != 2 && fFighter != bittobool( t->attribs, TA_FIGHTER ) )
                fFighter = 2;
            if( fCleric != 2 && fCleric != bittobool( t->attribs, TA_CLERIC ) )
                fCleric = 2;
            if( fMage != 2 && fMage != bittobool( t->attribs, TA_MAGE ) )
                fMage = 2;
            if( fSingle != 2 && fSingle != bittobool( t->attribs, TA_GSINGLE ) )
                fSingle = 2;
            if( fCoop != 2 && fCoop != bittobool( t->attribs, TA_GCOOP ) )
                fCoop = 2;
        }
    }

    // create dialog box
    pThingDlg = new Dialog;
    pThingDlg->Create( szDSFile, fHexen ? "HexenThingDlg" : "DoomThingDlg" );   // load it up
    pThingDlg->VarSetup( vtThings );

    // set 3-way checkboxes
    if( fEditMarked )
    {
        if( fHexen )
        {
            pfMage->Set3State( TRUE );
            pfCleric->Set3State( TRUE );
            pfFighter->Set3State( TRUE );
            pfDeathmatch->Set3State( TRUE );
            pfCoop->Set3State( TRUE );
            pfSingle->Set3State( TRUE );
        }

        pfDormant->Set3State( TRUE );
        pfAmbush->Set3State( TRUE );
        pfNormal->Set3State( TRUE );
        pfEasy->Set3State( TRUE );
        pfHard->Set3State( TRUE );
    }

    pThingDlg->SetMultiEdit( fEditMarked );

    // set some functions
    if( fHexen )
    {
        MakeSpecialFields( pfSpecial, (Field**) pfArgs );
    }

    pfThingString->SetUserEvent( etKey, KEY_F2, (FUFUNC) ffPickThings );
    pfThingString->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) ffPickThings );
    pfThingString->SetOnValidate( (FVFUNC) fvThingString );
    pfThingString->SetOnChange( (FFUNC) ffThingStringChanged );
    pfThingString->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) ffThingStringEnterKey );
    pfThingString->pUserData = (PTR) pfArgs;

    if( fHexen )
    {
        pfTID->SetUserEvent( etKey, 'F', (FUFUNC) ffFindTID );
        pfTID->SetUserEvent( etKey, KEY_ALTF, (FUFUNC) ffFindTID );
        pfAngle->SetUserEvent( etKey, 'F', (FUFUNC) ffFindPolyID );
        pfAngle->SetUserEvent( etKey, KEY_ALTF, (FUFUNC) ffFindPolyID );
    }

    pfAngle->SetUserEvent( etKey, KEY_F2, (FUFUNC) ffPickAngles );
    pfAngle->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) ffPickAngles );
    plbAnglePrompt = (stText*) pfAngle->GetPrompt();

    pThingDlg->SetOnEnter( (DFUNC) dfThingDlgUp );

    // process the dialog box
    if( pThingDlg->Process() == daExitNoSave )
    {
        rvl = 0;
        goto _returnit;
    }

    nMadeChanges++;

    iType = fszThingType[0] ? ThgStrToType( fszThingType ) : -1;
    iSpecial = fszSpecial[0] ? SpcStrToType( fszSpecial ) : -1;

    /* Stick the stuff back in */
    /* Loop thru all the marked items and figure out what we
     * want to do with their data. */
    for( i = 0; i < MAXOBJECT; i++ )
    {
        if( obj_ismarked( i ) )
        {
            t = &thinglist[i];
            /* Let's make the right choice here.. */
            bitset( t->attribs, fAmbush, TA_DEAF );
            bitset( t->attribs, fEasy, TA_SKILL12 );
            bitset( t->attribs, fNormal, TA_SKILL3 );
            bitset( t->attribs, fHard, TA_SKILL45 );
            bitset( t->attribs, fDormant, TA_DORMANT );
            bitset( t->attribs, fFighter, TA_FIGHTER );
            bitset( t->attribs, fCleric, TA_CLERIC );
            bitset( t->attribs, fMage, TA_MAGE );
            bitset( t->attribs, fSingle, TA_GSINGLE );
            bitset( t->attribs, fDeathmatch, TA_GDEATHMATCH );
            bitset( t->attribs, fCoop, TA_GCOOP );
            if( iAngle != FI_NODATA )   t->angle = iAngle;
            if( iType != -1 )           t->type = iType;

            if( fHexen )
            {
                if( iSpecial != -1 )        t->special = iSpecial;
                if( iAltitude != FI_NODATA )   t->height = iAltitude;
                if( iTID != FI_NODATA )     t->tid = iTID;
                if( iArg0 != FI_NODATA )    t->arg0 = iArg0;
                if( iArg1 != FI_NODATA )    t->arg1 = iArg1;
                if( iArg2 != FI_NODATA )    t->arg2 = iArg2;
                if( iArg3 != FI_NODATA )    t->arg3 = iArg3;
                if( iArg4 != FI_NODATA )    t->arg4 = iArg4;
            }
        }
    }

    if( otype != iType || oangle != iAngle )
        post_fullupdate();

_returnit:
    ResetKeys();

    delete pThingDlg;

    if( unmarkobj )
        obj_unmark( object );

    return rvl;
}


