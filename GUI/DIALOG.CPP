#include <std.h>
#include "dialog.hpp"

// border/margin constants
const int dlgMargin = 4;
const int dlgBorderWidth = 2;
const int dlgBordAndMargin = dlgMargin + dlgBorderWidth;

// size constants
const int dlgTitleHeight = 10;

// active dialog box
static Dialog *pdlgActive;

Dialog::Dialog() :
/*-------------------------------------------------------------------------*
 * constructs a dialog object.  initializes variables, etc.
 *-------------------------------------------------------------------------*/
    pfldFirst(NULL),
    pfldCurrent(NULL),
    plbFirst(NULL),
    pImageSave(NULL),
    pdlgPrev(NULL),
    pfldMouseCapture(NULL),
    pszLastStatusMsg(NULL),
    fIsVisible(FALSE),
    fpExit(NULL),
    fpEnter(NULL),
    fpSave(NULL),
    fpNoSave(NULL),
    fpValidate(NULL),
    fInsertMode(TRUE),
    pChoiceListBox(NULL),
    pDefaultButton(NULL),
    fFirstDraw(TRUE),
    lNextAct(-1)
{
}

void Dialog::Create( const Point& ptTopLeft, const Size& szDlgUnits,
                const char* pszTitle, unsigned uFlags )
/*-------------------------------------------------------------------------*
 * Creates the dialog object.
 *-------------------------------------------------------------------------*/
{
    int iTitleHeightPixels = _YUnitsToPixels( dlgTitleHeight );
    int iTitleSeparator = 4, iBorderSize = dlgBordAndMargin;
    int iTitleWidthUnits = pszTitle ?
                           _XPixelsToUnits( textwidth( pszTitle ) ) + 8 :
                           0;

    fStatusBar      = (bool) ((uFlags & dsStatusBar) ? 1 : 0);
    bool fAbsSize   = (bool) ((uFlags & dsCreateAbsSize) ? 1 : 0);
    bool fSimple    = (bool) ((uFlags & dsSimple) ? 1 : 0);

    if( fSimple )
    {
        iTitleHeightPixels = iTitleSeparator = 0;
        szTitle[0] = 0;
        iBorderSize = dlgBorderWidth;
    }
    else
        strcpy( szTitle, pszTitle );

    szClientUnits = szDlgUnits;

    if( fAbsSize )
        szClientUnits.PixelsToUnits();
    else if( szClientUnits.width < iTitleWidthUnits )
        szClientUnits.width = iTitleWidthUnits;

    // set up screen rectangle
    Size szPixels = szClientUnits;
    szPixels.UnitsToPixels();

    rcScreen.SetRect( Point( 0, 0 ), szPixels );

    if( fAbsSize )
    {
        szClientUnits.width -= _XPixelsToUnits(iBorderSize * 2);
        szClientUnits.height -= _YPixelsToUnits(((1 + fStatusBar) * (iTitleHeightPixels + iTitleSeparator)) +
                                (iBorderSize * 2));
    }
    else if( fStatusBar )
        rcScreen.bottom += iTitleSeparator + iTitleHeightPixels;

    //  we want the client area to be szDlgUnits in size
    if( !fAbsSize )
    {
        rcScreen.Inflate( iBorderSize, iBorderSize );
        rcScreen.Inflate( 0, iTitleHeightPixels / 2 );
    }

    // now position it
    Point ptPoint = ptTopLeft;
    if( ptPoint.x == Centered )
        ptPoint.x = iScreenWidth/2 - rcScreen.CenterX();
    if( ptPoint.y == Centered )
        ptPoint.y = (iScreenHeight - rcScreen.Height()) / 3;
    rcScreen.MoveTo( ptPoint );

    // set up client rectangle
    rcClient = rcScreen;
    rcClient.MoveTo( Point( 0, 0 ) );
    rcClient.Inflate( -iBorderSize, -iBorderSize );
    rcClient.top += iTitleHeightPixels + iTitleSeparator;

    rcScreenClient = rcClient;
    rcScreenClient.OffsetRect( rcScreen );

    // set up title rectangle
    rcTitle = rcScreen;
    rcTitle.MoveTo( Point( 0, 0 ) );
    rcTitle.Inflate( -iBorderSize, -iBorderSize );
    rcTitle.bottom = rcTitle.top + iTitleHeightPixels;

    // inflate screen rectangle for status bar on bottom
    rcStatus = rcScreen;
    rcStatus.MoveTo( Point( 0, 0 ) );
    rcStatus.top = rcStatus.bottom - iTitleHeightPixels -
                   iBorderSize;
    rcStatus.bottom = rcStatus.top + iTitleHeightPixels;
    rcStatus.Inflate( -iBorderSize, 0 );

    // set up other variables
}

Dialog::~Dialog()
/*-------------------------------------------------------------------------*
 * deletes the dialog object.
 *-------------------------------------------------------------------------*/
{
    // delete discardable fields and labels.
    Field *pField = pfldFirst;

    if( pField )
    do {
        Field *pFieldNext = pField->pfldNext;
        if( pField->GetStyle() & csDiscardable )
            delete pField;
        pField = pFieldNext;
    } while( pField != pfldFirst );

    Label *pLabel = plbFirst;

    if( pLabel )
    do {
        Label *pLabelNext = pLabel->plbNext;
        if( pLabel->GetStyle() & csDiscardable )
            delete pLabel;
        pLabel = pLabelNext;
    } while( pLabel != plbFirst );
}

void Dialog::MoveTo( Point &ptPoint )
{
    if( ptPoint.x == Centered )
        ptPoint.x = iScreenWidth/2 - rcScreen.CenterX();
    if( ptPoint.y == Centered )
        ptPoint.y = (iScreenHeight - rcScreen.Height()) / 3;
    rcScreen.MoveTo( ptPoint );

    rcScreenClient = rcClient;
    rcScreenClient.OffsetRect( rcScreen );
}

void Dialog::ShadowRect( Rect& rcRect, unsigned uFlags, int clrInterior )
/*-------------------------------------------------------------------------*
 * Draw a shadow rectangle in the dialog box.
 * rcRect is in Pixels.
 *-------------------------------------------------------------------------*/
{
    shadowrect( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom,
                uFlags, clrInterior );
}

void Dialog::FillRect( Rect& rcRect, int clrInterior )
/*-------------------------------------------------------------------------*
 * Draw a rectangle in the dialog box.
 * rcRect is in Pixels.
 *-------------------------------------------------------------------------*/
{
    if( clrInterior == -2 )
    {
        // automatically set interior color
        clrInterior = GetGuiColor( gcDlgBkgrnd );
    }

    bool fHidden = mouse_hide( rcRect );
    setfillstyle( SOLID_FILL, clrInterior );
    bar( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
    mouse_show( fHidden );
}

void Dialog::FrameRect( Rect& rcRect, int clrBorder )
/*-------------------------------------------------------------------------*
 * Draw a rectangle in the dialog box.
 * rcRect is in Pixels.
 *-------------------------------------------------------------------------*/
{
    if( clrBorder == -2 )
    {
        // automatically set border color to dialog color
        clrBorder = GetGuiColor( gcDlgBkgrnd );
    }

    bool fHidden = mouse_hide( rcRect );
    setcolor( clrBorder );
    rectangle( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
    mouse_show( fHidden );
}

void Dialog::FocusRect( Rect& rcRect, int clrColor, bool fExpand )
/*-------------------------------------------------------------------------*
 * Draw a focus rectangle.
 * rcRect is in Pixels, and is expanded by two before drawing.
 *-------------------------------------------------------------------------*/
{
    Rect rcNew = rcRect;

    if( fExpand )
        rcNew.Inflate( 2, 2 );

    bool fHidden = mouse_hide( rcNew );
    setlinestyle( USERBIT_LINE, 0x5555, 1 );
    setwritemode( XOR_PUT );
    if( clrColor == -1 )
        clrColor = GetGuiColor( gcDlgBkgrnd ) ^ GetGuiColor( gcDlgText );
    setcolor( clrColor );
    rectangle( rcNew.left, rcNew.top, rcNew.right, rcNew.bottom );
    setlinestyle( SOLID_LINE, 0, 1 );
    setwritemode( COPY_PUT );
    mouse_show( fHidden );
}

void Dialog::Draw()
/*-------------------------------------------------------------------------*
 * Draws the dialog, no less.
 *-------------------------------------------------------------------------*/
{
    // make this the active dialog box
    pdlgPrev = pdlgActive;
    pdlgActive = this;

    // want complete redraw
    fFirstDraw = TRUE;

    // draw the previously active title bar
    if( pdlgPrev )
        pdlgPrev->DrawTitleBar();

    // perform transfer
    Transfer( transToField );

    SetViewPort( 0, 0, iScreenWidth, iScreenHeight );
    bool fHidden = mouse_hide( rcScreen );

    // first, save what's underneath it.
    unsigned uImageSize = ImageSize( rcScreen );

    if( szTitle[0] )    // sheesh .. change this to dsSimple sometime
    {
        pImageSave = (char*) malloc( uImageSize );
        // erase popup help, if there's any
        _PopHelp.AddAvoidRect( rcScreen );
        _PopHelp.Erase();
    }

    if( pImageSave )
    {
        // this isn't _necessary_, so no big error on failure
        GetImage( rcScreen, pImageSave );
    }

    // now, draw the black bound rectangle
    setcolor( C_BLACK );
    Rectangle( rcScreen );

    // now, draw the shadow rectangle
    Rect rcInside = rcScreen;
    rcInside.Inflate( -1, -1 );
    unsigned uRectStyle = srOutdent;
    if( szTitle[0] )
        uRectStyle |= srThick;
    shadowrect( rcInside.left, rcInside.top, rcInside.right, rcInside.bottom,
                uRectStyle, GetGuiColor( gcDlgBkgrnd ) );

    fIsVisible = TRUE;

    // now, draw the title and title bar
    DrawTitleBar();

    // draw status bar
    DrawStatusBar( NULL );

    // the rest of this is in the client area ..
    SetActive();

    // draw the background labels
    DrawLabels();

    // draw fields
    DrawFields();

    mouse_show( fHidden );

    fFirstDraw = FALSE;
}

void Dialog::DrawStatusBar( const char* pszMessage )
/*-------------------------------------------------------------------------*
 * Draw the status bar, and place a message in there
 *-------------------------------------------------------------------------*/
{
    if( !fStatusBar || !fIsVisible )
        return;

    if( pszMessage == pszLastStatusMsg && !IsFirstDraw() )
        return;

    pszLastStatusMsg = pszMessage;

    // don't make the dialog active, because the status bar is outside of
    // the client area.  instead, make a screen Status rectangle:
    Rect rcScreenStatus = rcStatus;
    rcScreenStatus.OffsetRect( rcScreen );

    SetActive();

    // set the viewport to the screen Status rectangle
    SetViewPort( rcScreenStatus.left, rcScreenStatus.top - 4,
                 rcScreenStatus.right, rcScreenStatus.bottom );

    // draw the Status rectangle
    Rect rcStatusText( 0, 4, rcScreenStatus.Width(),
                        rcScreenStatus.Height() + 4 );

    bool fHidden = mouse_hide( rcStatusText );

    ShadowRect( rcStatusText, srIndent, GetGuiColor( gcDlgBkgrnd ) );

    // draw the text
    if( pszMessage )
    {
        Point ptStatusOfs( 2, 1 );
        ptStatusOfs.UnitsToPixels();
        setcolor( GetGuiColor( gcDlgTitleText ) );
        outtextxy( ptStatusOfs.x, ptStatusOfs.y + 4, pszMessage );
    }

    // draw a mini shadow rect
    shadowrect( 0, rcStatusText.top-3, rcStatusText.Width(),
            rcStatusText.top-2, srLineDent, -1 );

    // visible? set to client area
    if( IsVisible() )
        SetActive();

    mouse_show( fHidden );
}

void Dialog::DrawLabels()
/*-------------------------------------------------------------------------*
 * Draws all the background objects in the dialog box.
 *-------------------------------------------------------------------------*/
{
    Label*  pLabel = plbFirst;

    if( !pLabel )   // nothing to draw
        return;

    do {
        pLabel->OnDraw();
        pLabel = pLabel->plbNext;
    } while( pLabel != plbFirst );
}

void Dialog::DrawFields()
/*-------------------------------------------------------------------------*
 * Draws all the fields in the dialog box.
 *-------------------------------------------------------------------------*/
{
    Field*  pField = pfldFirst;

    if( !pField )   // nothing to draw
        return;

    do {
        pField->SetFirstDraw();
        pField->OnDraw();
        pField = pField->pfldNext;
    } while( pField != pfldFirst );
}

void Dialog::SetActive()
/*-------------------------------------------------------------------------*
 * Sets the dialog in focus, that is: sets the viewport to the dialog's
 * client area.
 *-------------------------------------------------------------------------*/
{
    SetViewPort( rcScreenClient.left, rcScreenClient.top,
                 iScreenWidth, iScreenHeight );
}

void Dialog::DrawTitleBar()
/*-------------------------------------------------------------------------*
 * draw the dialog title.  duh.
 *-------------------------------------------------------------------------*/
{
    if( !fIsVisible || !szTitle[0] )
        return;

    // don't make the dialog active, because the title is outside of
    // the client area.  instead, make a screen title rectangle:
    Rect rcScreenTitle = rcTitle;
    rcScreenTitle.OffsetRect( rcScreen );

    SetActive();
    bool fHidden = mouse_hide( rcScreenTitle );

    // set the viewport to the screen title rectangle
    SetViewPort( rcScreenTitle.left, rcScreenTitle.top,
                 rcScreenTitle.right, rcScreenTitle.bottom + 4 );

    // fill the entire rectangle
    setfillstyle( SOLID_FILL, GetGuiColor( gcDlgBkgrnd ) );
    bar( 0, 0, rcScreenTitle.right, rcScreenTitle.bottom );

    // draw the title rectangle
    if( this == pdlgActive )
        setfillstyle( SOLID_FILL, GetGuiColor( gcDlgTitleBarActive ) );
    else
        setfillstyle( SOLID_FILL, GetGuiColor( gcDlgTitleBarInactive ) );

    Rect rcTitleText( 0, 0, rcScreenTitle.Width(), rcScreenTitle.Height() );

    bar( 0, 0, rcTitleText.right, rcTitleText.bottom );

    // draw the close button - if current.
    if( 0 ) // this == pdlgActive )
    {
        Rect rcClose( 0, 0, rcTitleText.Height(), rcTitleText.Height() );
        rcClose.Inflate( -4, -4 );
        ShadowRect( rcClose, srOutdent | srThin, GetGuiColor( gcDlgBkgrnd ) );
        setcolor( C_BLACK );
        rcClose.Inflate( -2, -2 );
        line( rcClose.left, rcClose.top, rcClose.right, rcClose.bottom );
        line( rcClose.right, rcClose.top, rcClose.left, rcClose.bottom );
    }

    // draw the text
    Point ptTitleOfs( 4, 1 );
    ptTitleOfs.UnitsToPixels();
    setcolor( GetGuiColor( gcDlgTitleText ) );
    outtextxy( ptTitleOfs.x, ptTitleOfs.y, szTitle );

    // draw a mini shadow rect
    shadowrect( 0, rcTitleText.bottom+2, rcTitleText.Width(),
                rcTitleText.bottom+3, srLineDent, -1 );

    // visible? set to client area
    if( IsVisible() )
        SetActive();

    mouse_show( fHidden );
}

void Dialog::DrawTextXY( Point& ptPoint, const char* pszText,
                    bool fDrawAccel, int iXOffset, int iYOffset )
/*-------------------------------------------------------------------------*
 * Draws text in the dialog - ptPoint is in dialog units.
 *-------------------------------------------------------------------------*/
{
    char *pAmpersand = NULL;
    int _FontAscent = 10;

    if( fDrawAccel )
        pAmpersand = (char*) strchr( pszText, '&' );

    Point ptPixels = ptPoint;
    ptPixels.UnitsToPixels();
    ptPixels += Point( iXOffset, iYOffset );

    if( pAmpersand )
    {
        int     iAccelWidth;
        int     iFirstWidth;
        char    szFirstPart[128];

        // make the part of the string before the ampersand
        iFirstWidth = (pAmpersand-pszText)+2;
        strncpy( szFirstPart, pszText, iFirstWidth );
        szFirstPart[iFirstWidth] = 0;

        iAccelWidth = textwidth( szFirstPart + iFirstWidth - 1 );
        szFirstPart[iFirstWidth-2] = 0;

        iFirstWidth = textwidth( szFirstPart );

        // draw it all now.. phew!
        outtextxy( ptPixels.x, ptPixels.y, szFirstPart );
        outtextxy( ptPixels.x + iFirstWidth, ptPixels.y, pAmpersand+1 );
        line( ptPixels.x+iFirstWidth,
              ptPixels.y+_FontAscent,
              ptPixels.x+iFirstWidth+iAccelWidth,
              ptPixels.y+_FontAscent );
    }
    else
        outtextxy( ptPixels.x, ptPixels.y, pszText );
}

void Dialog::EraseTextXY( Point& ptPoint, const char* pszText )
/*-------------------------------------------------------------------------*
 * Erases text in the dialog - ptPoint is in dialog units.
 *-------------------------------------------------------------------------*/
{
    Point ptPixels = ptPoint;
    ptPixels.UnitsToPixels();
    Rect rcText( ptPixels,
                 Size( textwidth( pszText ), textheight( pszText ) ) );
    bar( rcText.left, rcText.top, rcText.right, rcText.bottom );
}

void Dialog::Erase()
/*-------------------------------------------------------------------------*
 * Restores the underlying image to the screen, if there was one saved.
 *-------------------------------------------------------------------------*/
{
    if( pImageSave )
    {
        // erase popup help, if there's any
        _PopHelp.RemoveAvoidRect();
        _PopHelp.Erase();

        SetViewPort( 0, 0, iScreenWidth, iScreenHeight );
        bool fHidden = mouse_hide( rcScreen );
        putimage( rcScreen.left, rcScreen.top, pImageSave, COPY_PUT );
        free( pImageSave );
        pImageSave = NULL;
        mouse_show( fHidden );
    }

    // make the previously active dialog box active
    pdlgActive = pdlgPrev;
    pdlgPrev = NULL;

    // draw the re-activated dialog's title bar
    if( pdlgActive )
        pdlgActive->DrawTitleBar();

    fIsVisible = FALSE;
}

int Dialog::Process( int iStartItem )
/*-------------------------------------------------------------------------*
 * Calls Draw(), Read(), and Erase().
 *-------------------------------------------------------------------------*/
{
    struct viewporttype vp;
    getviewsettings( &vp );

    Draw();
    int iRvl = Read( iStartItem );
    Erase();

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom );

    return iRvl;
}

Field* Dialog::EnumFields( Field* pField )
/*-------------------------------------------------------------------------*
 * Enumerates the fields in the dialog box.  Pass it NULL to start with,
 * and the return value in subsequent calls.  Returns NULL when passed
 * the last field.
 *-------------------------------------------------------------------------*/
{
    if( pField == NULL )
        pField = pfldFirst;
    else
    {
        pField = pField->pfldNext;
        if( pField == pfldFirst )
            return NULL;
    }

    return pField;
}

Label* Dialog::EnumLabels( Label* pLabel )
/*-------------------------------------------------------------------------*
 * Enumerates the Labels in the dialog box.  Pass it NULL to start with,
 * and the return value in subsequent calls.  Returns NULL when passed
 * the last Label.
 *-------------------------------------------------------------------------*/
{
    if( pLabel == NULL )
        pLabel = plbFirst;
    else
    {
        pLabel = pLabel->plbNext;
        if( pLabel == plbFirst )
            return NULL;
    }

    return pLabel;
}

void Dialog::GoToItem( Field* pField )
/*-------------------------------------------------------------------------*
 * Goes to pField.
 *-------------------------------------------------------------------------*/
{
    // already there
    if( pfldCurrent == pField )
        return;

    if( pField->GetStyle() & csMouseButton )
        return;

    if( pField->GetStyle() & csSkip )
        return;

    if( pfldCurrent )
    {
        if( pfldCurrent->OnExit() != TRUE )
            return;
    }

    pfldCurrent = pField;

    pfldCurrent->OnEnter();
}

void Dialog::GoToItem( int iItem )
/*-------------------------------------------------------------------------*
 * Goes to an item.
 *
 * iItem is either an item number, or a daNextItem or daPrevItem.
 *-------------------------------------------------------------------------*/
{
    Field *pfldNewItem = pfldCurrent;
    int iDirection = 1, iCount; // forward
    int iRadGroup = -1;

    if( !pfldNewItem )
        pfldNewItem = pfldFirst;

    if( iItem < _daStart )  // not a daXXX action - is an item number
        pfldNewItem = pfldFirst;
    else if( iItem == daPrevItem )
        iDirection = -1;    // backwards

    if( !pfldNewItem )
        return;

    if( pfldCurrent )
    {
        if( pfldCurrent->GetType() == ftRadio || pfldCurrent->GetType() ==
            ftRadio2 )
        {
            iRadGroup = ((fldRadio*) pfldCurrent)->iGroup;
        }
    }

    // find that item number
    for( iCount = 0; ; iCount++ )
    {
        // going to a specific item number:
        if( iItem < _daStart )
        {
            if( iCount == iItem ) // got it
                break;
            else if( pfldNewItem == pfldFirst ) // back at the start
                return;
        }

        // otherwise, going to the next/prev item:
        if( iDirection == -1 )
            pfldNewItem = pfldNewItem->pfldPrev;
        else
            pfldNewItem = pfldNewItem->pfldNext;

        // make sure it's not the same rad group
        if( pfldNewItem->GetType() == ftRadio || pfldNewItem->GetType() ==
            ftRadio2 )
        {
            if( ((fldRadio*) pfldNewItem)->iGroup == iRadGroup )
                continue;
        }

        // if it's go-able, break
        if( !(pfldNewItem->GetStyle() & csSkip) &&
            !(pfldNewItem->GetStyle() & csMouseButton) )
            break;
    }

    // we now have a new item in pfldNewItem .. so, exit the previous
    // field first.
    if( pfldCurrent )
    {
        if( pfldCurrent->OnExit() != TRUE )
            return;
    }

    if( pfldNewItem->GetType() == ftRadio || pfldNewItem->GetType() == ftRadio2 )
    {
        fldRadio *pRadio = (fldRadio*) pfldNewItem;
        if( pRadio->GetState() != pRadio->iNum )
            pfldNewItem = (Field*) pRadio->FindNumInGroup( pRadio->GetState() );
    }

    // set the current field to the new pointer
    pfldCurrent = pfldNewItem;

    // now, go to the new item
    pfldCurrent->OnEnter();
}

void Dialog::Transfer( int iDirection )
/*-------------------------------------------------------------------------*
 * Transfers data
 *-------------------------------------------------------------------------*/
{
    Field*  pField = EnumFields( NULL );

    while( pField )
    {
        if( pField->pvTransferBuf )
            pField->Transfer( iDirection );
        pField = EnumFields( pField );
    }
}

int Dialog::Validate()
/*-------------------------------------------------------------------------*
 * Validates the fields, and if that's okay, calls the dialog's validation
 * function.
 *-------------------------------------------------------------------------*/
{
    Field*  pField = EnumFields( NULL );

    while( pField )
    {
        if( pField->Validate() == valNotOkay )
        {
            GoToItem( pField );
            return valNotOkay;
        }
        pField = EnumFields( pField );
    }

    // dialog validation ..
    if( fpValidate )
    {
        if( (*fpValidate)(this) == valNotOkay )
            return valNotOkay;
    }

    return valOkay;
}

int Dialog::Read( int iStartItem )
/*-------------------------------------------------------------------------*
 * Reads dialog input.
 *-------------------------------------------------------------------------*/
{
    int iRvl;
    bool fFirst = TRUE;

    SetCursorType( ctArrow );

    // draw the active title bar
    DrawTitleBar();

    // set active
    SetActive();

    // go to start item
    GoToItem( iStartItem );

    // enter function
    if( fpEnter )
        (*fpEnter)(this);

    do  {

        if( !fFirst )
        {
            pfldCurrent->OnEnter();
        }

        fFirst = FALSE;

        // read loop
        iRvl = _ReadLoop();

        if( fpSave && iRvl == daExitSave )
        {
            if( !(*fpSave)(this) )
                continue;
        }
        else if( fpNoSave && iRvl == daExitNoSave )
        {
            if( !(*fpNoSave)(this) )
                continue;
        }
        else if( fpExit )
        {
            if( !(*fpExit)(this) )
                continue;
        }

        // validate
        if( iRvl == daExitSave && Validate() == valOkay )
            break;  // all done!
        else if( iRvl == daExitNoSave )
            break;

    } while( 1 );

    // transfer
    if( iRvl == daExitSave )
        Transfer( transFromField );

    return iRvl;
}

int Dialog::_ReadLoop()
/*-------------------------------------------------------------------------*
 * Input loop:
 * On a key, calls the current field's OnKey function.
 * On a mouse click, finds the first field that responds to a HitTest(),
 *    changes to that field (*), and calls its Hit() function.
 *
 * (*) - only if the field is not a "mousable button."
 *-------------------------------------------------------------------------*/
{
    int iRvl = daContinue;
    int iMouseEvent;
    short suMouseX, suMouseY;
    Event_t event;
    unsigned uKey;

    do {

        long lTime = biostime( 0, 0L );

        switch( iRvl )
        {
        case daContinue:
            break;  // do nothing
        case daNextItem:
            GoToItem( daNextItem );
            break;  // next item
        case daPrevItem:
            GoToItem( daPrevItem );
            break;  // prev item
        case daRedraw:
            Draw();
            break;
        case daDefaultButton:
            if( pDefaultButton )
            {
                iRvl = pDefaultButton->DoButton();
                goto ReLoop;
            }
            break;
        }

        if( lNextAct == -1 )
            iRvl = daContinue;
        else
        {
            iRvl = lNextAct;
            lNextAct = -1;
            goto ReLoop;
        }

        if( keycheck() )
        {
            uKey = keyget();

            event.type = etKey;
            event.iCode = uKey;

            if( !GetMouseCapture() )
                iRvl = pfldCurrent->OnEvent( event );

            SetActive();
        }
        else if( (iMouseEvent = mouse_event( &suMouseX, &suMouseY ))
                    != MOUSE_NOTHING )
        {
            Field* pMouseField;

            if( iMouseEvent == MOUSE_RUP )
            {
                iRvl = daExitNoSave;
                goto ReLoop;
            }

            // set up the event structure
            event.type = etMouse;
            event.iCode = iMouseEvent;
            event.ptPoint = Point( suMouseX, suMouseY ) -
                            rcScreenClient.TopLeft();

            // is the mouse captured?
            if( pMouseField = GetMouseCapture() )
            {
                event.iHitWhere = pMouseField->HitTest( event );
                event.ptPoint -= pMouseField->rcDlgPixels.TopLeft();
                iRvl = pMouseField->OnEvent( event );
                goto ReLoop;
            }
            else if( event.iCode == MOUSE_MOVE )
            {
                // not interested in mouse movements if no capture is on
                goto ReLoop;
            }

            // otherwise, find a field
            pMouseField = EnumFields( NULL );

            // figure out which field it's in
            while( pMouseField )
            {
                if( pMouseField->HitTest( event ) == htInside &&
                   !(pMouseField->GetStyle() & csSkip) )
                {
                    event.ptPoint -= pMouseField->rcDlgPixels.TopLeft();
                    iRvl = pMouseField->OnEvent( event );
                    break;
                }
                pMouseField = EnumFields( pMouseField );
            }

            SetActive();
        }
        else
        {
            // call all fields' idle function
            if( lTime != lTimerValue )
            {
                lTimerValue = lTime;
                Field* pField = pfldFirst;

                do {
                    pField->OnTimer( lTime );
                    pField = pField->pfldNext;
                } while( pField != pfldFirst );
            }
        }

ReLoop:

        if( pfldCurrent )
        {
            if( iRvl == daExitSave )
                pfldCurrent->OnExit();
            else if( iRvl == daExitNoSave )
            {
                void _FieldRemovePopup();
                _FieldRemovePopup();
            }
        }

    } while( iRvl != daExitSave && iRvl != daExitNoSave );

    return iRvl;
}

void Dialog::AddDefaultButtons()
/*-------------------------------------------------------------------------*
 * Add Okay/cancel buttons
 *-------------------------------------------------------------------------*/
{
    Field* pButton;

    pButton = new fldButton( this,
        Rect( szClientUnits.width - 68, szClientUnits.height - 20,
              szClientUnits.width - 38, szClientUnits.height - 8 ), "Ok",
                   (FFUNC) pbOkay, csDefaultButton );
    pButton->SetMessage( "Click to save and exit the dialog box." );

    pButton = new fldButton( this,
        Rect( szClientUnits.width - 34, szClientUnits.height - 20,
              szClientUnits.width - 4, szClientUnits.height - 8 ), "Cancel",
                   (FFUNC) pbCancel );
    pButton->SetMessage( "Click to exit and cancel the dialog box." );
}

Field* Dialog::GetField( const char* pszName )
/*-------------------------------------------------------------------------*
 * Returns the field named.. 'name!'!
 *-------------------------------------------------------------------------*/
{
    Field* pField = EnumFields( NULL );

    while( pField )
    {
        if( pField->GetName() && !strcmpi( pszName, pField->GetName() ) )
            return pField;

        pField = EnumFields( pField );
    }

    return (Field*) NULL;
}

Label* Dialog::GetLabel( const char* pszName )
/*-------------------------------------------------------------------------*
 * Returns the label named.. 'name!'!
 *-------------------------------------------------------------------------*/
{
    Label* pLabel = EnumLabels( NULL );

    while( pLabel )
    {
        if( pLabel->GetName() && !strcmpi( pszName, pLabel->GetName() ) )
            return pLabel;

        pLabel = EnumLabels( pLabel );
    }

    return (Label*) NULL;
}

bool Dialog::VarSetup( VarTransfer_t* pVT )
/*-------------------------------------------------------------------------*
 * sets up variables in a variable array.
 *-------------------------------------------------------------------------*/
{
    int i;
    bool fRvl = TRUE;
    Field* pField;

    for( i = 0; pVT[i].pszName; i++ )
    {
        pField = GetField( pVT[i].pszName );
        if( pField )
        {
            if( pVT[i].ppField )    // store field
                pVT[i].ppField[0] = pField;
            if( pVT[i].pData )      // do stuff with data
            {
                if( pField->GetType() == ftButton )
                    ((fldButton*)pField)->SetFunction( (FFUNC) pVT[i].pData );
                else
                    pField->SetTransferBuf( pVT[i].pData );
            }
            if( pVT[i].pszMessage )
                pField->SetMessage( pVT[i].pszMessage );
        }
        else
        {
            if( pVT[i].ppField )
                pVT[i].ppField[0] = NULL;
            fRvl = FALSE;
        }
    }

    return fRvl;
}

void Dialog::DrawDefaultAs( bool fDefault )
/*-------------------------------------------------------------------------*
 * Draws the default button as the default, or as not the default,
 * as it weren't.
 *-------------------------------------------------------------------------*/
{
    if( !pDefaultButton )
        return;

    pDefaultButton->SetStyle( csDefaultButton, fDefault );
    pDefaultButton->OnDraw();
}

bool Dialog::SETBUTTONFUNC( const char* pszName, FFUNC fp )
{
    fldButton* pButton = (fldButton*) GetField( pszName );
    if( pButton == NULL )
        return FALSE;
    pButton->SetFunction( fp );
    return TRUE;
}

bool Dialog::REPLACEREGION( const char* pszName, fldRegion* pNewRgn )
{
    fldRegion* pOldRgn = (fldRegion*) GetField( pszName );
    if( pOldRgn == NULL )
        return FALSE;
    pOldRgn->Replace( pNewRgn );
    return TRUE;
}
