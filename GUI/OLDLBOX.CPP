#include <std.h>
#include "dialog.hpp"

const int lboxMarginY = 0;
const int lboxMarginX = 2;

fldListBox::fldListBox( Dialog *pDialog, Rect& rcRect, const char* pszPrompt,
                    int iStyle ) :
/*-------------------------------------------------------------------------*
 * Create a listbox
 *-------------------------------------------------------------------------*/
        Field( pDialog, ftListBox, iStyle ),
        pslItems(NULL),
        iTopItem(0),
        iCurItem(0),
        iLastTop(-1),
        iLastCur(-1),
        iSearchLen(0),
        fScrolling(FALSE)
{
    Rect rcMyRect = rcRect;

    // create text prompt
    if( pszPrompt )
    {
        plbPrompt = (Label*) new stText( pdlgParent, rcMyRect.TopLeft() +
                Point( 0, -1 ), pszPrompt );
        plbPrompt->SetStyle( csDiscardable, TRUE );
        rcMyRect.top += YUPC;
    }

    SetRect( rcMyRect );

    // set caret size (search strings)
    SetCaretSize( Size( 2, _YUnitsToPixels( 8 ) ) );
    SetCaretColor( gcLboxTextHigh );
}

void fldListBox::SetRect( Rect& rcRect )
/*-------------------------------------------------------------------------*
 * Sets the size of the listbox.
 *-------------------------------------------------------------------------*/
{
    Size szSize( rcRect.Width(), rcRect.Height() );

    // resize for string heights
    if( !(GetStyle() & csNoIntegralHeight) )
        szSize -= Size( 0, (szSize.height - lboxMarginY * 2) % 8 );

    Create( rcRect.TopLeft(), szSize );
    rcDlgPixels.Inflate( 1, 1 );

    nVisItems = (szSize.height - lboxMarginY * 2) / 8;

    // create top left item point
    ptTopItem = rcDlgUnits.TopLeft();
    ptTopItem += Size( lboxMarginX, lboxMarginY );
}

void fldListBox::DrawItem( int iItem )
/*-------------------------------------------------------------------------*
 * Draw an item
 *-------------------------------------------------------------------------*/
{
    if( iItem == FL_NOITEM )
    {
        if( iItem == iCurItem )
        {
            Rect rcHigh( ptTopItem, Size( rcDlgUnits.Width(), YUPC ) );
            rcHigh.left -= lboxMarginX;
            rcHigh.right -= lboxMarginX;
            rcHigh.UnitsToPixels();
            rcCurItemRect = rcHigh;
        }

        return;
    }

    Point ptText = ptTopItem + Size( 0, (iItem - iTopItem) * YUPC );

    if( iItem == iCurItem || iItem == iLastCur )
    {
        // draw current item background
        Rect rcHigh( ptText, Size( rcDlgUnits.Width(), YUPC ) );
        rcHigh.left -= lboxMarginX;
        rcHigh.right -= lboxMarginX;
        rcHigh.UnitsToPixels();
        if( iItem == iCurItem )
        {
            // draw current item with highlighted background
            pdlgParent->FillRect( rcHigh, GetGuiColor( gcLboxBarHigh ) );
            setcolor( GetGuiColor( gcLboxTextHigh ) );
            rcCurItemRect = rcHigh;
        }
        else
        {
            // draw last current item with unhighlighted background
            pdlgParent->FillRect( rcHigh, GetGuiColor( gcFldBkgrnd ) );
            setcolor( GetGuiColor( gcFldText ) );
        }
    }
    else
        setcolor( GetGuiColor( gcFldText ) );

    if( iItem < pslItems->GetCount() )
        pdlgParent->DrawTextXY( ptText, (*pslItems)[iItem] );
}

void fldListBox::Draw()
/*-------------------------------------------------------------------------*
 * Draw the listbox, yes.
 *-------------------------------------------------------------------------*/
{
    if( !CanDraw() )
        return;

    if( IsFirstDraw() )
        iLastTop = iLastCur = -1;

    // draw rect
    if( iTopItem != iLastTop )
    {
        if( abs( iTopItem - iLastTop ) == 1 &&
            abs( iCurItem - iLastCur ) == 1 && !IsFirstDraw() )
        {
            int iPixTextHeight = GetFontHeight()/10;
            Rect rcScroll = rcDlgPixels;
            rcScroll.Inflate( -1, -1 );
            char *pImageBuf = new char[ImageSize( rcScroll )];

            // just scroll the list.. we hope!
            if( iTopItem < iLastTop )   // scroll down
            {
                rcScroll.bottom -= iPixTextHeight;
                GetImage( rcScroll, pImageBuf );
                putimage( rcScroll.left, rcScroll.top + iPixTextHeight,
                    pImageBuf, COPY_PUT );
            }
            else
            {
                rcScroll.top += iPixTextHeight;
                GetImage( rcScroll, pImageBuf );
                putimage( rcScroll.left, rcScroll.top - iPixTextHeight,
                    pImageBuf, COPY_PUT );
            }

            DrawItem( iLastCur );   // remove highlight
            DrawItem( iCurItem );   // draw current item

            delete pImageBuf;
        }
        else
        {
            pdlgParent->ShadowRect( rcDlgPixels, srIndent,
                    GetGuiColor( gcFldBkgrnd ) );

            int nItem = pslItems->GetCount();

            for( int i = iTopItem; i < iTopItem + nVisItems &&
                i < nItem; i++ )
            {
                if( i != iCurItem )
                    DrawItem( i );
            }
            DrawItem( iCurItem );
        }
    }
    else
    {
        // just draw the two changed items
        DrawItem( iLastCur );
        DrawItem( iCurItem );
    }

    // set last markers
    iLastCur = iCurItem;
    iLastTop = iTopItem;

    // redraw focus rect
    if( IsInFocus() )
        OnFocusChange();
}

void fldListBox::Erase()
/*-------------------------------------------------------------------------*
 * Erase it, yes.
 *-------------------------------------------------------------------------*/
{
    pdlgParent->FillRect( rcDlgPixels );
}

void fldListBox::Transfer( int iDirection )
/*-------------------------------------------------------------------------*
 * Transfer!
 *-------------------------------------------------------------------------*/
{
    if( !pvTransferBuf )    // nothing to transfer to/from
        return;

    char* pszTransfer = (char*) pvTransferBuf;

    if( iDirection == transFromField )
    {
        if( GetCurItem() != FL_NOITEM )
            strcpy( pszTransfer, (*pslItems)[iCurItem] );
        else
            pszTransfer[0] = 0;
    }
    else
    {
        if( pszTransfer[0] || !pdlgParent->GetMultiEdit() )
            SelectString( pszTransfer );
        else
            SetCurItem( FL_NOITEM );
    }
}

void fldListBox::SetCurItem( int iItem )
/*-------------------------------------------------------------------------*
 * Sets the current item, of course.
 *-------------------------------------------------------------------------*/
{
    int iLastCur = iCurItem;

    if( iItem == FL_NOITEM )
    {
        iTopItem = 0;
        iCurItem = iItem;
    }
    else
    {
        iCurItem = iItem;

        if( iCurItem >= pslItems->GetCount() )
            iCurItem = pslItems->GetCount() - 1;

        if( iCurItem < 0 )
            iCurItem = 0;

        if( iCurItem < iTopItem )
            iTopItem = iCurItem;
        else if( iCurItem >= iTopItem + nVisItems )
            iTopItem += (iCurItem - (iTopItem + nVisItems)) + 1;

        if( iTopItem + nVisItems > pslItems->GetCount() )
            iTopItem = pslItems->GetCount() - nVisItems;

        if( iTopItem < 0 )
            iTopItem = 0;
    }

    if( iCurItem != iLastCur || iTopItem != iLastTop )
    {
        // turn off caret ..
        ShowCaret( FALSE );
        SetStyle( csUsesCaret, FALSE );
        OnDraw();
    }

    if( iCurItem != iLastCur )
        OnChange();
}

void fldListBox::UpdateCaretPos()
/*-------------------------------------------------------------------------*
 * Update the caret in the listbox for search strings
 *-------------------------------------------------------------------------*/
{
    szSearch[iSearchLen] = 0;
    SetCaretColor( gcLboxTextHigh );
    SetCaretPos( Point( 1 + textwidth( szSearch ) +
            _XUnitsToPixels( lboxMarginX ),
            1 + _YUnitsToPixels( (iCurItem - iTopItem) * 8 ) ) );
    SetStyle( csUsesCaret, TRUE );
    ShowCaret();
}

int fldListBox::OnKey( unsigned uKey )
/*-------------------------------------------------------------------------*
 * Process keys
 *-------------------------------------------------------------------------*/
{
    if( GetCurItem() == FL_NOITEM )
        iCurItem = -1;

    // process keys
    switch( uKey )
    {
    case KEY_UP:
        SetCurItem( iCurItem-1 );
        break;
    case KEY_DOWN:
        SetCurItem( iCurItem+1 );
        break;
    case KEY_PGDN:
        SetCurItem( iCurItem + nVisItems );
        break;
    case KEY_PGUP:
        SetCurItem( iCurItem - nVisItems );
        break;
    case KEY_HOME:
        SetCurItem( 0 );
        break;
    case KEY_END:
        SetCurItem( pslItems->GetCount() - 1 );
        break;
    default:
        if( !(GetStyle() & csUsesCaret) )
            // no caret on, so set the search string to 0
            iSearchLen = 0;

        // search for string ..
        if( uKey == KEY_BACK && iSearchLen )
        {
            // decrement length of search string
            --iSearchLen;
            if( !iSearchLen )
            {
                // turn off the caret
                SetCurItem( 0 );
            }
            else
            {
                int iItem = pslItems->Find( szSearch, iSearchLen );
                if( iItem != -1 )
                {
                    SetCurItem( iItem );
                    UpdateCaretPos();
                }
            }
        }
        else if( uKey == KEY_BACK && pdlgParent->GetMultiEdit() )
        {
            iTopItem = 0;
            SetCurItem( FL_NOITEM );
            break;
        }

        if( inrange( uKey, 32, 127 ) )
        {
            szSearch[iSearchLen++] = uKey;
            int iItem = pslItems->Find( szSearch, iSearchLen );
            if( iItem != -1 )
            {
                SetCurItem( iItem );
                UpdateCaretPos();
            }
            else    // get rid of bogus char
                --iSearchLen;
        }
        break;
    }

    return daContinue;
}

void fldListBox::OnTimer( long lTimerValue )
/*-------------------------------------------------------------------------*
 * Called by the dialog box when nothing's happening .. if we're scrolling
 * the listbox, scroll to the next item if the specified time has elapsed.
 *-------------------------------------------------------------------------*/
{
    // call base function
    Field::OnTimer( lTimerValue );

    if( fScrolling && (lTimerValue - lLastScroll) > 0 )
    {
        Event_t Event;

        // get mouse pos, make it relative to the topleft of the client
        // area, and do a hit test..
        mouse_coords( Event.ptPoint );
        Event.ptPoint -= pdlgParent->ScreenClientRect().TopLeft();
        switch( HitTest( Event ) )
        {
        case htAbove:
            SetCurItem( iTopItem - 1 );
            break;
        case htBelow:
            SetCurItem( iTopItem + nVisItems );
            break;
        }

        lLastScroll = lTimerValue;
    }
}

int fldListBox::Event( Event_t& event )
/*-------------------------------------------------------------------------*
 * Process listbox events
 *-------------------------------------------------------------------------*/
{
    switch( event.type )
    {
    case etKey:
        return OnKey( event.iCode );
    case etMouse:
        switch( event.iCode )
        {
        case MOUSE_LUP:
            if( event.iHitWhere == htInside )
            {
                // mouse was upped inside the box, so select the item
                // it's on..
                SetCurItem( iTopItem + event.ptPoint.y /
                            _YUnitsToPixels( 8 ) );
            }
            else
            {
                // it was upped outside the box, so .. uhh .. don't
                // do anything
            }
            fScrolling = FALSE; // no more scrolling, anyway
            break;
        case MOUSE_LDOWN:
            // ldown events always occur inside the box, so select
            // the item..
            SetCurItem( iTopItem + event.ptPoint.y / _YUnitsToPixels( 8 ) );
            break;
        case MOUSE_MOVE:
            // if we get these, the mouse button must still be down..
            // so, drag it around.
            switch( event.iHitWhere )
            {
            case htInside:
                SetCurItem( iTopItem + event.ptPoint.y /
                            _YUnitsToPixels( 8 ) );
                break;
            case htAbove:
            case htBelow:
                // set scrolling in motion
                fScrolling = TRUE;
                lLastScroll = pdlgParent->lTimerValue;
                break;
            }
            break;
        }
    }

    return daContinue;
}

void fldListBox::SelectString( const char* pszString )
/*-------------------------------------------------------------------------*
 * Finds all/part of stirng in the string list, and sets the current
 * selection to that item
 *-------------------------------------------------------------------------*/
{
    SetCurItem( pslItems->Find( pszString ) );
}

void fldListBox::OnFocusChange()
/*-------------------------------------------------------------------------*
 * Draw/Undraw focus rect
 *-------------------------------------------------------------------------*/
{
    pdlgParent->FocusRect( rcCurItemRect,
              GetGuiColor( gcLboxBarHigh ) ^ GetGuiColor( gcLboxFocus ), FALSE );
}

