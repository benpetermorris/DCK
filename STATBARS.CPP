/*-------------------------------------------------------------------------*
 * STATBARS.CPP:
 * Create/Modify/Update status bars.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "fields.hpp"

extern uchar *op_strings[];
extern short current_op;
extern byte fFullScreen;

class fldFlag : public fldRegion
{
public:
    fldFlag( const char* pszCaption, int iColour = gcLightGreen,
        bool fCenter = TRUE ) : fldRegion()
        {
            this->iColour = iColour;
            SetCaption( pszCaption );
            fState = 0xfe;
            this->fCenter = fCenter;
            iXPos = -1;
            fUpdate = TRUE;
        }

    void SetColour( int iColour )
        {
            if( iColour == this->iColour && !fUpdate )
                return;

            fState = (iColour == gcBlack) ? 0 : 1;
            this->iColour = iColour;
            OnDraw();
        }

    void SetState( int fState )
        {
            fState = fState ? 1 : 0;

            if( this->fState != fState || fUpdate )
            {
                this->fState = fState;
                OnDraw();
            }
        }

    void Draw()
        {
            Rect rcRect = GetRectInParent();

            if( fCenter == FALSE )
            {
                SetRectStyle( 0xFFFF );
                iXPos = rcRect.left + 2;
            }
            else if( iXPos == -1 )
            {
                SetRectStyle( 0xFFFF );
                iXPos = rcRect.CenterX() - textwidth( szCaption ) / 2;
            }

            fldRegion::SetFillColor( fState ? iColour : gcBlack );
            fldRegion::Draw();
            setcolor( (fState && iColour > gcBlack + 8) ?
                GetGuiColor( gcBlack ) : GetGuiColor( gcWhite ) );
            outtextxy( iXPos, rcRect.top, szCaption );

            fUpdate = FALSE;
        }

    void Erase()
        {
            fldRegion::Erase();
            fState = 0xfe;
            fUpdate = TRUE;
        }

    void SetCaption( const char* pszCaption, bool fRedraw = FALSE )
        {
            if( !strcmp( pszCaption, szCaption ) )
                return;

            strcpy( szCaption, pszCaption );
            iXPos = -1;

            if( fRedraw )
                OnDraw(), fUpdate = FALSE;
            else
                fUpdate = TRUE;
        }

private:
    bool fUpdate;
    bool fState;
    bool fCenter;
    int iColour;
    int iXPos;
    char szCaption[128];
};



// status bars (dialogs!):
static Dialog *pSectorInfoBar;
static Dialog *pLineInfoBar;
static Dialog *pThingInfoBar;
static Dialog *pVertexInfoBar;
static Dialog *pMainInfoBar;

// message bar variables:
static stText   *plbStatusMsg;
static bool     fMsgUpdate;
static char     szStatusMsg[1024];

// vertex info:
static stText   *plbVIDesc;

// thing info bar fields:
static fldFlag  *pfTIEasy, *pfTIHard, *pfTINormal, *pfTIAmbush, *pfTIDormant,
                *pfTIFighter, *pfTICleric, *pfTIMage, *pfTISingle, *pfTICoop,
                *pfTIDeathmatch;
static stText   *plbTIType, *plbTISpecial, *plbTIAngle, *plbTIAnglePrompt, *plbTIAltitude, *plbTITID;

// sector info bar fields:
static fldFlatTex  *pfSIFloorTex, *pfSICeilTex;
static stText   *plbSIFloorTexName, *plbSICeilTexName;
static stText   *plbSIFloorHeight, *plbSICeilHeight;
static stText   *plbSIType, *plbSILight, *plbSITag, *plbSINumber;

// line info bar fields:
static stText   *plbLIType, *plbLITag, *plbLILength, *plbLIActivation;
static fldFlag  *pfLISide1Upper, *pfLISide1Full, *pfLISide1Lower;
static stText   *plbLISide1Sector;
static fldFlag  *pfLISide2Upper, *pfLISide2Full, *pfLISide2Lower;
static stText   *plbLISide2Sector;
static fldFlag  *pfLIImpassable, *pfLITwoSided, *pfLIRepeat, *pfLISecret, *pfLIBlockSound,
                *pfLIBlockMonster, *pfLIUpUnpeg, *pfLILowUnpeg, *pfLINeverMap, *pfLIAlwaysMap;

// main info bar fields:
static fldFlag  *pfMISnap, *pfMIFine, *pfMIJoin;
static stText   *plbMIXscale, *plbMIGrid, *plbMICurOp, *plbMIFile, *plbMIMotif;

//
//
// Code begins here
//
//

void SetStatusMsg( char *fmt, ... )
{
    if( !fmt )
        fmt = "";

    char    szBuf[1024];
    va_list vl;

    va_start( vl, fmt );
    vsprintf( szBuf, fmt, vl );
    va_end( vl );

    if( !strcmp( szBuf, szStatusMsg ) )
        return;

    fMsgUpdate = TRUE;
    strcpy( szStatusMsg, szBuf );
}

void UpdateMessageBar()
{
    if( !fMsgUpdate || fFullScreen )
        return;

    pMainInfoBar->SetActive();
    plbStatusMsg->SetText( szStatusMsg );

    fMsgUpdate = FALSE;
}

void UpdateMainBar()
/*-------------------------------------------------------------------------
 * Updates the fields of the main status bar, then calls sbd_standard
 * to display it.
 *-------------------------------------------------------------------------*/
{
    if( fFullScreen )
        return;

    extern  int iCreateObject;
    extern  char *motif_name;
    char    *editstr, *wadnameptr = pwdCurWad->fn;
    uchar   *snapstr[] = { "grid", "snap", "svtx" };
    char    buf[128];
    static  uchar oldsnap = 4;

    enum
    {
        coPolygon,
        coRectangle
    };

    pMainInfoBar->SetActive();

    if( tog_snap != oldsnap )
    {
        pfMISnap->SetCaption( snapstr[tog_snap] );
        pfMISnap->SetState( tog_snap );
        pfMISnap->OnDraw();
    }
    oldsnap = tog_snap;
    plbMIGrid->SetText( ultoa( gridstep, buf, 10 ) );

    /* Fine mode */
    pfMIFine->SetState( tog_fine );
    /* Join mode */
    pfMIJoin->SetState( tog_join );
    /* X scale */
    plbMIXscale->SetText( ultoa( xscale, buf, 10 ) );
    /* Motif */
    plbMIMotif->SetText( motif_name );
    /* Current operation */
    //plbMICurOp->SetText( op_strings[current_op] );

    /* Map/WAD file */
    switch( editmode )
    {
    case MODE_THING: editstr = "Editing items"; break;
    case MODE_VERTEX: editstr = "Editing vertices"; break;
    case MODE_LINE:
        if( iCreateObject == coPolygon )
            editstr = "Creating polygons";
        else if( iCreateObject == coRectangle )
            editstr = "Creating rectangles";
        else
            editstr = "Editing lines";
        break;
    case MODE_SECTOR: editstr = "Editing sectors"; break;
    }

    if( fNewMap )
        wadnameptr = "UNTITLED.WAD";

    if( wadnameptr )
        sprintf( buf, "%s [%s]", szCurMapTag, wadnameptr );
    else
        sprintf( buf, "No map loaded." );

    plbMIFile->SetText( buf );
}

static void CheckLastHexen( unsigned mode )
{
    static  bool   fLastHexen = 2;

    if( fLastHexen == fHexen && fFullUpdate != 2 )
        return;

    if( mode & MODE_THING )
    {
        if( fHexen == FALSE )
        {
            pfTIFighter->OnErase();
            pfTIFighter->SetStyle( csInvisible, ON );
            pfTICleric->OnErase();
            pfTICleric->SetStyle( csInvisible, ON );
            pfTIMage->OnErase();
            pfTIMage->SetStyle( csInvisible, ON );
            pfTISingle->OnErase();
            pfTISingle->SetStyle( csInvisible, ON );
            pfTICoop->OnErase();
            pfTICoop->SetStyle( csInvisible, ON );
            pfTIDeathmatch->OnErase();
            pfTIDeathmatch->SetStyle( csInvisible, ON );
            pfTIDormant->SetCaption( "multi" );
            pfTIDormant->OnDraw();
        }
        else
        {
            pfTIFighter->SetStyle( csInvisible, OFF );
            pfTIFighter->OnDraw();
            pfTICleric->SetStyle( csInvisible, OFF );
            pfTICleric->OnDraw();
            pfTIMage->SetStyle( csInvisible, OFF );
            pfTIMage->OnDraw();
            pfTISingle->SetStyle( csInvisible, OFF );
            pfTISingle->OnDraw();
            pfTICoop->SetStyle( csInvisible, OFF );
            pfTICoop->OnDraw();
            pfTIDeathmatch->SetStyle( csInvisible, OFF );
            pfTIDeathmatch->OnDraw();
            pfTIDormant->SetCaption( "dormnt" );
            pfTIDormant->OnDraw();
        }
    }
    if( mode & MODE_LINE )
    {
        if( fHexen == FALSE )
        {
            pfLIRepeat->OnErase();
            pfLIRepeat->SetStyle( csInvisible, ON );
        }
        else
        {
            pfLIRepeat->SetStyle( csInvisible, OFF );
            pfLIRepeat->OnDraw();
        }
    }

    fLastHexen = fHexen;
}

void UpdateObjectBar( Object& obj, bool fForce )
{
    if( !obj.IsValid() )
        obj.iNumber = -1;

    int object = obj.iNumber;

    if( fFullScreen )
        return;

    THING   *t;
    VERTEX  *v;
    LINEDEF *l;
    SECTOR  *s;
    SIDEDEFM *sd;
    short   len;
    ushort  attribs;
    short   cftx, cutx, cltx;
    char    buf1[9], buf[128];

    static  short  lastmode = -1, lastobj = -1;
    static  ushort lmchanges = -1;

    struct viewporttype vp;
    getviewsettings( &vp );

    static SECTOR BlankSector =
    {
        0, 0, "", "", 0, 0, 0
    };
    static LINEDEF BlankLine =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1
    };
    static THING BlankThing =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    char* ActTypes[] =
    {
        "Player Cross",
        "Player Use",
        "Monster Cross",
        "Projectile Impact (Wall)",
        "Player Push (Wall)",
        "Projectile Cross"
    };

    if( !fForce && lmchanges == nMadeChanges && lastmode == editmode &&
        lastobj == object )
        return;

    lastmode = obj.iType;
    lastobj = object;
    lmchanges = nMadeChanges;
    const char *pszAngle;

    switch( editmode )
    {
    case MODE_THING:
        if( !pThingInfoBar )
            break;

        t = &thinglist[object];
        if( object == -1 )
            t = &BlankThing;

        pThingInfoBar->SetActive();

        CheckLastHexen( MODE_THING );
        attribs = t->attribs;

        pszAngle = AngleTypeToStr( t->angle );

        if( fHexen && inrange( t->type, 3000, 3002 ) )
            plbTIAnglePrompt->SetText( "PolyID:" );
        else
            plbTIAnglePrompt->SetText( "Angle:" );

        // create angle string - include name of angle if it's a cardinal direction
        if( object != -1 )
        {
            if( pszAngle )
                sprintf( buf, "%d (%s)", t->angle, pszAngle );
            else
                itoa( t->angle, buf, 10 );
            plbTIAngle->SetText( buf );
        }
        else
            plbTIAngle->SetText( st_blank );

        // create type string
        if( object != -1 )
            sprintf( buf, "#%-04d: %s", object, ThgTypeToStr( t->type ) );
        else
            strcpy( buf, "No item selected." );
        plbTIType->SetText( buf );

        // set tid
        if( fHexen && object != -1 )
        {
            itoa( t->tid, buf, 10 );
            plbTITID->SetText( buf );
        }
        else
            plbTITID->SetText( st_blank );

        // set altitude
        if( fHexen && object != -1 )
        {
            itoa( t->height, buf, 10 );
            plbTIAltitude->SetText( buf );
        }
        else
            plbTIAltitude->SetText( st_blank );

        // set special
        if( fHexen && object != -1 )
            plbTISpecial->SetText( SpcTypeToStr( t->special ) );
        else
            plbTISpecial->SetText( st_blank );

        // set flags
        pfTIEasy->SetState( attribs & TA_SKILL12 );
        pfTINormal->SetState( attribs & TA_SKILL3 );
        pfTIHard->SetState( attribs & TA_SKILL45 );
        pfTIAmbush->SetState( attribs & TA_AMBUSH );
        pfTIDormant->SetState( attribs & TA_DORMANT );

        if( fHexen )
        {
            pfTIFighter->SetState( attribs & TA_FIGHTER );
            pfTICleric->SetState( attribs & TA_CLERIC );
            pfTIMage->SetState( attribs & TA_MAGE );
            pfTISingle->SetState( attribs & TA_GSINGLE );
            pfTICoop->SetState( attribs & TA_GCOOP );
            pfTIDeathmatch->SetState( attribs & TA_GDEATHMATCH );
        }

        break;
    case MODE_LINE:
        if( !pLineInfoBar )
            break;

        pLineInfoBar->SetActive();

        if( obj.iType == MODE_VERTEX )
            object = -1;

        l = &linelist[object];
        if( object == -1 )
            l = &BlankLine;

        attribs = l->attribs;

        if( object != -1 )
            len = obj_linelen( object );
        else
            len = 0;

        plbLILength->SetText( itoa( len, buf1, 10 ) );

        if( object != -1 )
        {
            if( !lSecTag( l ) && lineneedstag( l->special ) )
                plbLITag->SetColor( gcRed );   /* red if needs tag and no tag */
            else
                plbLITag->SetColor( gcDlgText );
            plbLITag->SetText( itoa( lSecTag( l ), buf, 10 ) );
        }
        else
            plbLITag->SetText( "" );

        if( fHexen && object != -1 && l->special )
        {
            int iAct = LA_GETSPAC( l->attribs );
            char *pszAct;
            if( iAct < 0 || iAct > 5 )
            {
                pszAct = "(invalid!)";
                plbLIActivation->SetColor( gcRed );
            }
            else
            {
                pszAct = ActTypes[iAct];
                plbLIActivation->SetColor( gcDlgText );
            }
            plbLIActivation->SetText( pszAct );
        }
        else
            plbLIActivation->SetText( "" );

        if( obj.iType != MODE_VERTEX )
        {
            const char *p = NULL;

            if( object != -1 )
                p = SpcTypeToStr( l->special );

            if( p == NULL && object != -1 )
            {
                static char szBuf[128];
                sprintf( szBuf, "Unknown special type %d", l->special );
                p = szBuf;
                plbLIType->SetColor( gcRed );
            }
            else
            {
                plbLIType->SetColor( gcDlgText );
            }

            if( object != -1 )
                sprintf( buf, "#%-04d: %s", object, p );
            else
                strcpy( buf, "No line selected." );

            plbLIType->SetText( buf );
        }
        else
        {
            v = &vlist[obj.iNumber];
            sprintf( buf, "Vertex #%-04d: at %5d, %-5d", obj.iNumber, v->x, v->y );
            plbLIType->SetText( buf );
        }

        pfLIImpassable->SetState( attribs & LA_IMPASS );

        if( fHexen )
        {
            pfLIRepeat->SetState( attribs & LA_REPEAT );
        }

        pfLIBlockSound->SetState( attribs & LA_SBLOCK );
        pfLIBlockMonster->SetState( attribs & LA_MBLOCKER );
        pfLIUpUnpeg->SetState( attribs & LA_UUNPEGGED );
        pfLILowUnpeg->SetState( attribs & LA_LUNPEGGED );
        pfLITwoSided->SetState( attribs & LA_TSIDE );
        pfLISecret->SetState( attribs & LA_SECRET );
        pfLINeverMap->SetState( attribs & LA_NOTONMAP );
        pfLIAlwaysMap->SetState( attribs & LA_AUTOONMAP );

        if( l->rsdef != -1 )
        {
            sd = &sidelist[l->rsdef];
            side_getneedtex( object, 0, &cutx, &cftx, &cltx );
            if( cutx )   cutx = (sd->utxidx ? gcBlue : gcRed);
            else         cutx = gcBlack;
            if( cftx )   cftx = (sd->ftxidx ? gcBlue : gcRed);
            else         cftx = gcBlack;
            if( cltx )   cltx = (sd->ltxidx ? gcBlue : gcRed);
            else         cltx = gcBlack;

            pfLISide1Upper->SetCaption( texture_tostr( buf1, sd->utxidx ) );
            pfLISide1Full->SetCaption( texture_tostr( buf1, sd->ftxidx ) );
            pfLISide1Lower->SetCaption( texture_tostr( buf1, sd->ltxidx ) );
            plbLISide1Sector->SetText( itoa( sd->sector, buf1, 10 ) );

            pfLISide1Upper->SetColour( cutx );
            pfLISide1Full->SetColour( cftx );
            pfLISide1Lower->SetColour( cltx );
        }
        else
        {
            pfLISide1Upper->SetCaption( st_blank );
            pfLISide1Full->SetCaption( st_blank );
            pfLISide1Lower->SetCaption( st_blank );
            pfLISide1Upper->SetColour( gcBlack );
            pfLISide1Full->SetColour( gcBlack );
            pfLISide1Lower->SetColour( gcBlack );
            plbLISide1Sector->SetText( st_blank );
        }

        if( l->lsdef != -1 )
        {
            sd = &sidelist[l->lsdef];
            side_getneedtex( object, 1, &cutx, &cftx, &cltx );
            if( cutx )   cutx = (sd->utxidx ? gcBlue : gcRed);
            else         cutx = gcDlgText;
            if( cftx )   cftx = (sd->ftxidx ? gcBlue : gcRed);
            else         cftx = gcDlgText;
            if( cltx )   cltx = (sd->ltxidx ? gcBlue : gcRed);
            else         cltx = gcDlgText;

            pfLISide2Upper->SetCaption( texture_tostr( buf1, sd->utxidx ) );
            pfLISide2Full->SetCaption( texture_tostr( buf1, sd->ftxidx ) );
            pfLISide2Lower->SetCaption( texture_tostr( buf1, sd->ltxidx ) );
            plbLISide2Sector->SetText( itoa( sd->sector, buf1, 10 ) );

            pfLISide2Upper->SetColour( cutx );
            pfLISide2Full->SetColour( cftx );
            pfLISide2Lower->SetColour( cltx );
        }
        else
        {
            pfLISide2Upper->SetCaption( st_blank );
            pfLISide2Full->SetCaption( st_blank );
            pfLISide2Lower->SetCaption( st_blank );
            pfLISide2Upper->SetColour( gcBlack );
            pfLISide2Full->SetColour( gcBlack );
            pfLISide2Lower->SetColour( gcBlack );
            plbLISide2Sector->SetText( st_blank );
        }
        break;
    case MODE_SECTOR:
        if( !pSectorInfoBar )
            break;

        s = &sectorlist[object];
        if( object == -1 )
            s = &BlankSector;

        char *p1, *p2;
        pfSIFloorTex->SetFlat( p1 = WN2Str( s->ftxname ) );
        pfSICeilTex->SetFlat( p2 = WN2Str( s->ctxname ) );
        plbSIFloorTexName->SetText( p1 );
        plbSICeilTexName->SetText( p2 );

        plbSICeilHeight->SetText( itoa( s->cheight, buf1, 10 ) );
        plbSIFloorHeight->SetText( itoa( s->fheight, buf1, 10 ) );
        plbSIType->SetText( SecTypeToStr( s->type ) );
        plbSILight->SetText( itoa( s->light, buf1, 10 ) );
        plbSITag->SetText( itoa( s->sectag, buf1, 10 ) );
        plbSINumber->SetText( itoa( object == -1 ? 0 : object, buf1, 10 ) );
        break;
    }

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom, 1 );
}

void CreateStatusBars()
{
    int i;

    // main status bar:
    pMainInfoBar = new Dialog;
    pMainInfoBar->Create( szDSFile, "MainInfoBar" );
    pMainInfoBar->Create( Point( 0, iScreenHeight-83-39 ),
            Size( iScreenWidth-7, 38 ), NULL, dsCreateAbsSize | dsSimple );
    pfMISnap = new fldFlag( "snap" );
    pfMIFine = new fldFlag( "fine" );
    pfMIJoin = new fldFlag( "join" );
    pMainInfoBar->REPLACEREGION( "Snap", pfMISnap );
    pMainInfoBar->REPLACEREGION( "Fine", pfMIFine );
    pMainInfoBar->REPLACEREGION( "Join", pfMIJoin );
    plbMIGrid = (stText*) pMainInfoBar->GetLabel( "Grid" );
    plbMIXscale = (stText*) pMainInfoBar->GetLabel( "Scale" );
    plbMIMotif = (stText*) pMainInfoBar->GetLabel( "Motif" );
    //plbMICurOp = (stText*) pMainInfoBar->GetLabel( "CurOp" );
    plbMIFile = (stText*) pMainInfoBar->GetLabel( "File" );
    plbStatusMsg = (stText*) pMainInfoBar->GetLabel( "Info" );

    // vertex status bar:
    pVertexInfoBar = new Dialog;
    pVertexInfoBar->Create( szDSFile, "VertexInfoBar" );
    pVertexInfoBar->Create( Point( 0, iScreenHeight-83 ),
            Size( iScreenWidth-7, 82 ), NULL, dsCreateAbsSize | dsSimple );
    plbVIDesc = (stText*) pVertexInfoBar->GetLabel( "Desc" );

    // thing status bar:
    pThingInfoBar = new Dialog;
    pThingInfoBar->Create( szDSFile, "ThingInfoBar" );
    pThingInfoBar->Create( Point( 0, iScreenHeight-83 ),
            Size( iScreenWidth-7, 82 ), NULL, dsCreateAbsSize | dsSimple );
    pfTIEasy = new fldFlag( "easy" );
    pfTINormal = new fldFlag( "normal" );
    pfTIHard = new fldFlag( "hard" );
    pfTIAmbush = new fldFlag( "ambush" );
    pfTIDormant = new fldFlag( "dormnt" );
    pfTIFighter = new fldFlag( "fightr" );
    pfTICleric = new fldFlag( "cleric" );
    pfTIMage = new fldFlag( "mage" );
    pfTISingle = new fldFlag( "single" );
    pfTICoop = new fldFlag( "coop" );
    pfTIDeathmatch = new fldFlag( "dmatch" );
    pThingInfoBar->REPLACEREGION( "Easy", pfTIEasy );
    pThingInfoBar->REPLACEREGION( "Normal", pfTINormal );
    pThingInfoBar->REPLACEREGION( "Hard", pfTIHard );
    pThingInfoBar->REPLACEREGION( "Ambush", pfTIAmbush );
    pThingInfoBar->REPLACEREGION( "Dormant", pfTIDormant );
    pThingInfoBar->REPLACEREGION( "Fighter", pfTIFighter );
    pThingInfoBar->REPLACEREGION( "Cleric", pfTICleric );
    pThingInfoBar->REPLACEREGION( "Mage", pfTIMage );
    pThingInfoBar->REPLACEREGION( "Single", pfTISingle );
    pThingInfoBar->REPLACEREGION( "Coop", pfTICoop );
    pThingInfoBar->REPLACEREGION( "Deathmatch", pfTIDeathmatch );
    plbTIType = (stText*) pThingInfoBar->GetLabel( "Type" );
    plbTISpecial = (stText*) pThingInfoBar->GetLabel( "Special" );
    plbTIAngle = (stText*) pThingInfoBar->GetLabel( "Angle" );
    plbTIAnglePrompt = (stText*) pThingInfoBar->GetLabel( "AnglePrompt" );
    plbTIAltitude = (stText*) pThingInfoBar->GetLabel( "Altitude" );
    plbTITID = (stText*) pThingInfoBar->GetLabel( "TID" );

    // sector status bar:
    pSectorInfoBar = new Dialog;
    pSectorInfoBar->Create( szDSFile, "SectorInfoBar" );
    pSectorInfoBar->Create( Point( 0, iScreenHeight-83 ),
            Size( iScreenWidth-7, 82 ), NULL, dsCreateAbsSize | dsSimple );
    pfSIFloorTex = new fldFlatTex;
    pfSICeilTex = new fldFlatTex;
    pSectorInfoBar->REPLACEREGION( "FloorTex", pfSIFloorTex );
    pSectorInfoBar->REPLACEREGION( "CeilTex", pfSICeilTex );
    plbSIFloorTexName = (stText*) pSectorInfoBar->GetLabel( "FloorTexName" );
    plbSICeilTexName = (stText*) pSectorInfoBar->GetLabel( "CeilTexName" );
    plbSIFloorHeight = (stText*) pSectorInfoBar->GetLabel( "FloorHeight" );
    plbSICeilHeight = (stText*) pSectorInfoBar->GetLabel( "CeilHeight" );
    plbSIType = (stText*) pSectorInfoBar->GetLabel( "Type" );
    plbSILight = (stText*) pSectorInfoBar->GetLabel( "Light" );
    plbSITag = (stText*) pSectorInfoBar->GetLabel( "Tag" );
    plbSINumber = (stText*) pSectorInfoBar->GetLabel( "Number" );

    // line status bar:
    pLineInfoBar = new Dialog;
    pLineInfoBar->Create( szDSFile, "LineInfoBar" );
    pLineInfoBar->Create( Point( 0, iScreenHeight-83 ),
            Size( iScreenWidth-7, 82 ), NULL, dsCreateAbsSize | dsSimple );
    pfLIImpassable = new fldFlag( "impass" );
    pLineInfoBar->REPLACEREGION( "Impassable", pfLIImpassable );
    pfLITwoSided = new fldFlag( "2-side" );
    pLineInfoBar->REPLACEREGION( "TwoSided", pfLITwoSided );
    pfLIRepeat = new fldFlag( "repeat" );
    pLineInfoBar->REPLACEREGION( "Repeat", pfLIRepeat );
    pfLISecret = new fldFlag( "secret" );
    pLineInfoBar->REPLACEREGION( "Secret", pfLISecret );
    pfLIBlockSound = new fldFlag( "bsound" );
    pLineInfoBar->REPLACEREGION( "BlockSound", pfLIBlockSound );
    pfLIBlockMonster = new fldFlag( "benemy" );
    pLineInfoBar->REPLACEREGION( "BlockMonster", pfLIBlockMonster );
    pfLIUpUnpeg = new fldFlag( "uunpeg" );
    pLineInfoBar->REPLACEREGION( "UpUnpeg", pfLIUpUnpeg );
    pfLILowUnpeg = new fldFlag( "lunpeg" );
    pLineInfoBar->REPLACEREGION( "LowUnpeg", pfLILowUnpeg );
    pfLINeverMap = new fldFlag( "no map" );
    pLineInfoBar->REPLACEREGION( "NeverMap", pfLINeverMap );
    pfLIAlwaysMap = new fldFlag( "1stmap" );
    pLineInfoBar->REPLACEREGION( "AlwaysMap", pfLIAlwaysMap );

    plbLIType = (stText*) pLineInfoBar->GetLabel( "Type" );
    plbLITag = (stText*) pLineInfoBar->GetLabel( "Tag" );
    plbLILength = (stText*) pLineInfoBar->GetLabel( "Length" );
    plbLIActivation = (stText*) pLineInfoBar->GetLabel( "Activate" );

    pfLISide1Upper = new fldFlag( "", gcBlack, FALSE );
    pfLISide1Full = new fldFlag( "", gcBlack, FALSE );
    pfLISide1Lower = new fldFlag( "", gcBlack, FALSE );

    pLineInfoBar->REPLACEREGION( "Side1Upper", pfLISide1Upper );
    pLineInfoBar->REPLACEREGION( "Side1Full", pfLISide1Full );
    pLineInfoBar->REPLACEREGION( "Side1Lower", pfLISide1Lower );

    plbLISide1Sector = (stText*) pLineInfoBar->GetLabel( "Side1Sector" );

    pfLISide2Upper = new fldFlag( "", gcBlack, FALSE );
    pfLISide2Full = new fldFlag( "", gcBlack, FALSE );
    pfLISide2Lower = new fldFlag( "", gcBlack, FALSE );

    pLineInfoBar->REPLACEREGION( "Side2Upper", pfLISide2Upper );
    pLineInfoBar->REPLACEREGION( "Side2Full", pfLISide2Full );
    pLineInfoBar->REPLACEREGION( "Side2Lower", pfLISide2Lower );

    plbLISide2Sector = (stText*) pLineInfoBar->GetLabel( "Side2Sector" );
}

void DrawStatusBars()
// draws all status bars (full redraw.)
{
    if( fFullScreen )
        return;

    CheckLastHexen( MODE_ALL );

    if( editmode == MODE_SECTOR )
        pSectorInfoBar->Draw();
    if( editmode == MODE_LINE )
        pLineInfoBar->Draw();
    if( editmode == MODE_THING )
        pThingInfoBar->Draw();
    if( editmode == MODE_VERTEX )
        pVertexInfoBar->Draw();
    pMainInfoBar->Draw();
}
