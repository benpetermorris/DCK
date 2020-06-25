#include <std.h>
#include "dialog.hpp"

#define _INITFIELD()      \
            pvTransferBuf(NULL),  \
            fpEnter(NULL),        \
            fpExit(NULL),         \
            fpChange(NULL),       \
            fpValidate(NULL),     \
            pdlgParent(NULL),     \
            pImageSave(NULL),     \
            pUnderCaret(NULL),    \
            lLastCaret(0L),       \
            fCaretVisible(FALSE), \
            pszStatusMsg(NULL),   \
            fModified(FALSE),     \
            plbPrompt(NULL),      \
            pUserData(NULL),      \
            fFirstDraw(TRUE),     \
            fDrawDown(FALSE),     \
            nUserEvent(0),        \
            clrCaret(gcFldCaret), \
            pszValMsg(NULL)


#define _mtoupper(ch)   ((ch) >= 'a' && (ch) <= 'z' ? ((ch)-32) : (ch))

Field::Field() :
/*-------------------------------------------------------------------------*
 * Just initialize the elements
 *-------------------------------------------------------------------------*/
    _INITFIELD()
{
    szHelpKey[0] = 0;
    SetStyle( csDiscardable, TRUE );
    szName[0] = 0;
}

Field::Field( Dialog* pdlgParent, int iType, int iStyle ) :
/*-------------------------------------------------------------------------*
 * Initializes a field structure.
 *-------------------------------------------------------------------------*/
    _INITFIELD()
{
    szHelpKey[0] = 0;

    // initialize parent
    this->pdlgParent = pdlgParent;

    // initialize previous/next fields
    if( !pdlgParent->pfldFirst )
    {
        pdlgParent->pfldFirst = this;
        pdlgParent->pfldFirst->pfldNext = this;
        pdlgParent->pfldFirst->pfldPrev = this;
    }
    pfldPrev = pdlgParent->pfldFirst->pfldPrev;
    pfldNext = pdlgParent->pfldFirst;
    pdlgParent->pfldFirst->pfldPrev->pfldNext = this;
    pdlgParent->pfldFirst->pfldPrev = this;

    // initialize type
    this->iType = iType;
    this->iStyle = iStyle;  // default styles
    this->pdlgParent = pdlgParent;
    SetStyle( csDiscardable, TRUE );
    szName[0] = 0;
}

Field::~Field()
{
    if( pUnderCaret )
    {
        delete pUnderCaret;
        pUnderCaret = NULL;
    }
    if( pImageSave )
    {
        delete pImageSave;
        pImageSave = NULL;
    }
}

void Field::SetUserEvent( int iType, int iCode, FUFUNC fp )
/*-------------------------------------------------------------------------*
 * Sets a custom event function in the field.
 *-------------------------------------------------------------------------*/
{
    int i;

    for( i = 0; i < nUserEvent; i++ )
    {
        // search for it already there..
        if( (UserEvent[i].iType == iType &&
             UserEvent[i].iCode == iCode) ||
            UserEvent[i].fp == NULL )
            break;
    }

    if( i == nUserEvent )
        ++nUserEvent;

    if( fp )
    {
        if( iType == etKey )
            iCode = _mtoupper( iCode );
        UserEvent[i].iCode = iCode;
        UserEvent[i].iType = iType;
        UserEvent[i].fp = fp;
    }
    else
    {
        // remove it
        UserEvent[i].fp = (FUFUNC) NULL;
    }
}

int Field::RunUserEvent( Event_t& event )
/*-------------------------------------------------------------------------*
 * Runs the function associated with event.iCode and iType. if there is no
 * function, returns -1.  otherwise, returns the return value of the
 * function called.
 *-------------------------------------------------------------------------*/
{
    int i, iCode = event.iCode;
    FUFUNC fp = NULL;

    if( event.type == etKey )
        iCode = _mtoupper( iCode );

    for( i = 0; i < nUserEvent; i++ )
    {
        // search for it already there..
        if( (UserEvent[i].iType == event.type &&
             UserEvent[i].iCode == iCode) )
        {
            fp = UserEvent[i].fp;
            break;
        }
    }

    if( fp )
    {
        int iRvl = (*fp)(this,event);
        pdlgParent->SetActive();
        return iRvl;
    }

    return -1;
}

void Field::Remove()
/*-------------------------------------------------------------------------*
 * Removes itself from the list of fields.
 *-------------------------------------------------------------------------*/
{
    pfldPrev->pfldNext = pfldNext;
    pfldNext->pfldPrev = pfldPrev;
}

void Field::Create( const Point& ptPoint, const Size& szSize )
/*-------------------------------------------------------------------------*
 * Initializes size structures for the field.
 * This version takes dialog units for both parameters.
 *-------------------------------------------------------------------------*/
{
    if( GetStyle() & csCreateAbsSize )
    {
        // create absolute size if that style is set
        CreateAbsSize( ptPoint, szSize );
        return;
    }

    ptDlgUnits = ptPoint;
    szDlgUnits = szSize;

    // Centered?
    if( ptDlgUnits.x == Centered )
        ptDlgUnits.x = pdlgParent->ClientSize().width / 2 -
                       szDlgUnits.width / 2;
    if( ptDlgUnits.y == Centered )
        ptDlgUnits.y = pdlgParent->ClientSize().height / 2 -
                       szDlgUnits.height / 2;

    rcDlgUnits.SetRect( ptDlgUnits, szDlgUnits );
    rcDlgPixels = rcDlgUnits;
    rcDlgPixels.UnitsToPixels();
}

void Field::CreateAbsSize( const Point& ptPoint, const Size& szSize )
/*-------------------------------------------------------------------------*
 * Initializes size structures for the field.
 * This version takes dialog pixels for the size parameter.
 *-------------------------------------------------------------------------*/
{
    ptDlgUnits = ptPoint;
    szDlgUnits = szSize;
    szDlgUnits.PixelsToUnits();     // NOW it's units

    // Centered?
    if( ptDlgUnits.x == Centered )
        ptDlgUnits.x = pdlgParent->ClientSize().width / 2 -
                       szDlgUnits.width / 2;
    if( ptDlgUnits.y == Centered )
        ptDlgUnits.y = pdlgParent->ClientSize().height / 2 -
                       szDlgUnits.height / 2;

    rcDlgUnits.SetRect( ptDlgUnits, szDlgUnits );
    Point ptDlgPixels = rcDlgUnits.TopLeft();
    ptDlgPixels.UnitsToPixels();
    rcDlgPixels.SetRect( ptDlgPixels, szSize );
}

void Field::SetViewPortToField()
/*-------------------------------------------------------------------------*
 * Sets the viewport's bottom right corner to the bottom right corner
 * of the current field.  this is useful for those situations where
 * the extent of the data drawn is not known.
 *-------------------------------------------------------------------------*/
{
    // create screen coords for bottom right.
    // offset by -1, -1 for field border.
    Point ptBottomRight( rcDlgPixels.right-1, rcDlgPixels.bottom-1 );
    ptBottomRight += pdlgParent->ScreenClientRect().TopLeft();

    struct viewporttype vp;
    getviewsettings( &vp );
    SetViewPort( vp.left, vp.top, ptBottomRight.x, ptBottomRight.y );
}

void Field::Help()
/*-------------------------------------------------------------------------*
 * Calls the global help function with the field's help key
 *-------------------------------------------------------------------------*/
{
}

static Field* pCurFields[32];
static int nCurFields;

int Field::OnEnter()
/*-------------------------------------------------------------------------*
 * Calls the field's "on enter" function.
 *-------------------------------------------------------------------------*/
{
    int iRvl = daContinue;

    // draw popup help, if there's any
    pCurFields[nCurFields++] = this;
    _PopHelp.AddAvoidRect( ScreenRect() );
    _PopHelp.Display( szHelpKey );

    OnFocusChange();

    if( fpEnter )
    {
        iRvl = (*fpEnter)(this);
        pdlgParent->SetActive();    // who knows what it did to the poor viewport?
    }

    if( !pszValMsg )
        pdlgParent->DrawStatusBar( pszStatusMsg );
    else
        pdlgParent->DrawStatusBar( pszValMsg );

    pszValMsg = NULL;

    return iRvl;
}

void _FieldRemovePopup()
{
    --nCurFields;

    // erase popup help, if there's any
    _PopHelp.Erase();
    _PopHelp.RemoveAvoidRect();
}

int Field::OnExit()
/*-------------------------------------------------------------------------*
 * Calls the field's "on exit" function.
 * An exit function returns TRUE on "yes, exit." or FALSE otherwise.
 *-------------------------------------------------------------------------*/
{
    int iRvl = TRUE;

    if( fpExit )
    {
        iRvl = (*fpExit)(this);
    }

    pdlgParent->SetActive();    // who knows what it did to the poor viewport?

    if( iRvl == TRUE )
    {
        _FieldRemovePopup();

        OnFocusChange();
        ShowCaret( FALSE );
        if( pUnderCaret )
        {
            delete pUnderCaret;
            pUnderCaret = NULL;
        }
    }

    return iRvl;
}

void Field::OnFocusChange()
/*-------------------------------------------------------------------------*
 * Draw a focus rect.
 *-------------------------------------------------------------------------*/
{
    if( !plbPrompt || (GetType() != ftCheckBox3 && GetType() != ftCheckBox &&
        GetType() != ftRadio && GetType() != ftRadio2) )
    {
        pdlgParent->FocusRect( rcDlgPixels );
    }
    else
    {
        Rect rcRect = plbPrompt->GetRectInParent();
        rcRect.Inflate( 0, -2 );
        pdlgParent->FocusRect( rcRect );
    }
}

int Field::OnChange()
/*-------------------------------------------------------------------------*
 * Calls the field's "on change" function.
 *-------------------------------------------------------------------------*/
{
    int iRvl = daContinue;

    // field has been modified if thsi function is called
    SetModified( TRUE );

    if( fpChange )
    {
        iRvl = (*fpChange)(this);
        // who knows what it did to the poor viewport?
        pdlgParent->SetActive();
    }

    return iRvl;
}

int Field::Validate()
/*-------------------------------------------------------------------------*
 * Calls the field's validation function if it has one, or returns
 * valOkay if it doesn't.
 *-------------------------------------------------------------------------*/
{
    int iRvl = valOkay;
    long lData;

    if( fpValidate )
    {
        lData = GetValidateData();

        iRvl = (*fpValidate)( lData, this );

        if( iRvl == valNotOkay && IsInFocus() && pszValMsg )
        {
            pdlgParent->DrawStatusBar( pszValMsg );
            pszValMsg = NULL;
        }
    }

    return iRvl;
}

void Field::OnDraw()
/*-------------------------------------------------------------------------*
 * Hides the mouse and calls the draw function.
 *-------------------------------------------------------------------------*/
{
    // make sure it's draw-able
    if( !CanDraw() )
        return;

    if( fFirstDraw && (GetStyle() & csInitNoDisp) )
    {
        SetStyle( csInitNoDisp, FALSE );
        return;
    }

    Label* pLabel;
    if( pLabel = GetPrompt() )
    {
        if( pLabel->GetStyle() & (csInvisible | csInitNoDisp) )
        {
            pLabel->SetStyle( 0, FALSE );
            pLabel->SetStyle( GetStyle(), TRUE );
            pLabel->OnDraw();
        }
    }

    pdlgParent->SetActive();

    bool fHidden = mouse_hide( rcDlgPixels );

    if( GetStyle() & csPopup && !pImageSave )
    {
        pImageSave = new char[imagesize( rcDlgPixels.left, rcDlgPixels.top,
                              rcDlgPixels.right, rcDlgPixels.bottom )];
        getimage( rcDlgPixels.left, rcDlgPixels.top, rcDlgPixels.right,
                  rcDlgPixels.bottom, pImageSave );
    }

    Draw();
    fFirstDraw = FALSE;

    mouse_show( fHidden );
}

void Field::OnErase()
/*-------------------------------------------------------------------------*
 * Hides the mouse and calls the draw function.
 *-------------------------------------------------------------------------*/
{
    pdlgParent->SetActive();

    bool fHidden = mouse_hide( rcDlgPixels );

    if( GetStyle() & csPopup )
    {
        putimage( rcDlgPixels.left, rcDlgPixels.top, pImageSave,
                  COPY_PUT );
        delete pImageSave;
        pImageSave = NULL;
    }
    else
        Erase();

    if( GetPrompt() )
        GetPrompt()->OnErase();

    SetFirstDraw();

    mouse_show( fHidden );
}

int Field::Event( Event_t& event )
/*-------------------------------------------------------------------------*
 * Default - does nothing.
 *-------------------------------------------------------------------------*/
{
    return daContinue;
}

int Field::OnEvent( Event_t& event )
/*-------------------------------------------------------------------------*
 * default processing for some events.
 *-------------------------------------------------------------------------*/
{
    if( pdlgParent->pfldCurrent != this &&
        pdlgParent->GetMouseCapture() != this )
    {
        // make sure it's not an end-mouse event
        if( event.type == etMouse )
        {
            switch( event.iCode )
            {
            case MOUSE_LUP:
            case MOUSE_LDRAGEXIT:
                // it is - ignore it
                return daContinue;
            }
        }
    }

    int iRvl;
    fldRadio *pRadio = (fldRadio*) this;
    bool fIsRadio = (GetType() == ftRadio || GetType() == ftRadio2) ? 1 : 0;

    pdlgParent->GoToItem( this );

    switch( event.type )
    {
    case etKey:
        // check for this event:
        iRvl = RunUserEvent( event );
        if( iRvl != -1 )
            return iRvl;

        // system events:
        switch( event.iCode )
        {
        case KEY_ESC:
            return daExitNoSave;
        case KEY_F10:
            return daExitSave;
        case KEY_TAB:
            return daNextItem;
            break;
        case KEY_STAB:
            return daPrevItem;
            break;
        case KEY_ENTER:
            if( GetType() != ftButton )
                return daDefaultButton;
            break;
        }

        // default event:
        return Event( event );
    case etMouse:
        // default mouse events
        switch( event.iCode )
        {
        case MOUSE_LDOWN:
            SetDrawDown( TRUE );
            pdlgParent->SetMouseCapture( this );
            break;
        case MOUSE_LUP:
            SetDrawDown( FALSE );
            pdlgParent->SetMouseCapture( NULL );
            break;
        case MOUSE_MOVE:
            // if it's no longer in there, don't draw down
            if( event.iHitWhere != htInside )
                SetDrawDown( FALSE );
            else
                SetDrawDown( TRUE );
            break;
        }

        // check for this event:
        iRvl = RunUserEvent( event );
        if( iRvl != -1 )
            return iRvl;

        // default event:
        return Event( event );
    }

    // corrupt event structure - oh dear!
    return daContinue;
}

int Field::HitTest( Event_t& event )
/*-------------------------------------------------------------------------*
 * returns the htxxx code corresponding to the relative position of
 * the point in 'event' to the field's rectangle.  also sets event's
 * 'iHitWhere' member to this code.
 * the point must be relative to the top left of the client area.
 *-------------------------------------------------------------------------*/
{
    Point& ptPoint = event.ptPoint;

    if( GetStyle() & csSkip )
        return (event.iHitWhere = 0);

    if( plbPrompt && (GetType() == ftCheckBox3 || GetType() == ftCheckBox ||
        GetType() == ftRadio || GetType() == ftRadio2) )
    {
        if( plbPrompt->HitTest( event ) )
            return event.iHitWhere;
    }

    event.iHitWhere = 0;

    if( rcDlgPixels.IsPtInside( ptPoint ) )
        event.iHitWhere = htInside; // inside.
    else
    {
        if( ptPoint.y < rcDlgPixels.top )
            event.iHitWhere |= htAbove;
        else if( ptPoint.y > rcDlgPixels.bottom )
            event.iHitWhere |= htBelow;
        if( ptPoint.x < rcDlgPixels.left )
            event.iHitWhere |= htToLeft;
        else if( ptPoint.x > rcDlgPixels.right )
            event.iHitWhere |= htToRight;
    }

    return event.iHitWhere;
}

void Field::OnTimer( long lTimerValue )
/*-------------------------------------------------------------------------*
 * Called every tic while the dialog is idle.
 *-------------------------------------------------------------------------*/
{
    if( IsInFocus() &&
        (GetStyle() & csUsesCaret) &&
        (lTimerValue - lLastCaret) > 8 )
    {
        ShowCaret( !fCaretVisible, lTimerValue );
    }
}

void Field::SetCaretSize( Size& szCaret )
/*-------------------------------------------------------------------------*
 * Set caret size, allocate memory for its underlying image
 *-------------------------------------------------------------------------*/
{
    this->szCaret = szCaret;
    if( pUnderCaret )
    {
        delete pUnderCaret;
        pUnderCaret = NULL;
    }
}

void Field::ShowCaret( bool fShow, long lTimerValue )
/*-------------------------------------------------------------------------*
 * shows or hides the caret.
 *-------------------------------------------------------------------------*/
{
    if( !IsInFocus() && fShow == TRUE ) // don't show if not in focus,
        return;                         //   but do delete

    if( !pUnderCaret )
    {
        pUnderCaret = new char[
            imagesize( 0, 0, szCaret.width, szCaret.height )];
    }

    if( fShow == fCaretVisible )
        return; // already shown/hidden

    bool fHidden = mouse_hide( rcCaret );

    // show?
    if( fShow == TRUE )
    {
        // get image
        getimage( rcCaret.left, rcCaret.top, rcCaret.right, rcCaret.bottom,
            pUnderCaret );
        // display caret
        pdlgParent->FillRect( rcCaret, GetGuiColor( clrCaret ) );
    }
    // hide?
    else
    {
        // put image
        putimage( rcCaret.left, rcCaret.top, pUnderCaret, COPY_PUT );
    }

    if( lTimerValue == -1 )
        lLastCaret = pdlgParent->lTimerValue;
    else
        lLastCaret = lTimerValue;

    fCaretVisible = fShow;

    mouse_show( fHidden );
}

void Field::Replace( Field* pNewField )
{
    if( pdlgParent->pfldFirst == this )
        pdlgParent->pfldFirst = pNewField;
    pfldPrev->pfldNext = pNewField;
    pfldNext->pfldPrev = pNewField;
}
