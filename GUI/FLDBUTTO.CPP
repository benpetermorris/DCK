#include <std.h>
#include "dialog.hpp"

const int butMarginX = 6;
const int butMarginY = 2;

fldButton::fldButton( Dialog* pdlgParent, Rect& rcRect, const char* pszCaption,
            FFUNC fpButton, int iStyle ) :
/*-------------------------------------------------------------------------*
 * Create a button field.
 *-------------------------------------------------------------------------*/
    Field( pdlgParent, ftButton, iStyle )
{
    Size szButton( pszCaption );
    Point ptButton = rcRect.TopLeft();

    if( GetStyle() & csAutoSize )
    {
        // based on top left
        szButton.PixelsToUnits();
        szButton += Size( butMarginX * 2, butMarginY * 2 );
    }
    else if( GetStyle() & csAutoPlace )
    {
        // based on bottom right
        szButton.PixelsToUnits();
        szButton += Size( butMarginX * 2, butMarginY * 2 );

        ptButton.x = rcRect.right - szButton.width;
        ptButton.y = rcRect.bottom - szButton.height;
    }
    else
    {
        // use rectangle size
        szButton.width = rcRect.Width();
        szButton.height = rcRect.Height();
    }

    SetFunction( fpButton );

    Create( ptButton, szButton );

    if( GetStyle() & csDefaultButton )
        pdlgParent->pDefaultButton = this;

    // rcDlgPixels.Inflate( 1, 1 );    // thicker frame

    strcpy( szCaption, pszCaption );
    szText.width = textwidth( szCaption );
    szText.height = textheight( szCaption );
    this->fpButton = fpButton;
}

int fldButton::Event( Event_t& event )
/*-------------------------------------------------------------------------*
 * Process events, as usual
 *-------------------------------------------------------------------------*/
{
    switch( event.type )
    {
    case etKey:
        if( event.iCode == KEY_SPACE ||
            event.iCode == KEY_ENTER )
            return DoButton();
        break;
    case etMouse:
        if( event.iCode == MOUSE_LUP &&
            event.iHitWhere == htInside )
            return DoButton();
            // that's the only mouse event processed here
        break;
    }

    return daContinue;
}

void fldButton::Erase()
/*-------------------------------------------------------------------------*
 * Erase the button.
 *-------------------------------------------------------------------------*/
{
    if( !CanDraw() )
        return;

    pdlgParent->FillRect( rcDlgPixels );
}

void fldButton::Draw()
/*-------------------------------------------------------------------------*
 * Draw the button, yes.
 *-------------------------------------------------------------------------*/
{
    if( !CanDraw() )
        return;

    // create rect for inside black frame
    Rect rcInside = rcDlgPixels;
    //rcInside.Inflate( -1, -1 );

    // frame button
    //pdlgParent->FrameRect( rcDlgPixels, C_BLACK );

    if( GetStyle() & csDefaultButton )
    {
        // thicker frame for default buttons
        pdlgParent->FrameRect( rcInside, C_BLACK );
        rcInside.Inflate( -1, -1 );
    }

    if( !GetDrawDown() )
    {
        // draw as normal
        if( !(GetStyle() & csSkip) )
        {
            pdlgParent->ShadowRect( rcInside, srOutdent | srDarker );
            rcInside.right--;
            rcInside.bottom--;
            pdlgParent->ShadowRect( rcInside, srOutdent );
        }
        else
        {
            pdlgParent->FillRect( rcInside );
            setcolor( GetGuiColor( gc3DDown ) );
            pdlgParent->FrameRect( rcInside );
        }
        setcolor( GetGuiColor( gcDlgText ) );

        outtextxy( rcDlgPixels.CenterX() - szText.width/2,
                   rcDlgPixels.CenterY() - szText.height/2,
                   szCaption );
    }
    else
    {
        // draw as button held down
        pdlgParent->ShadowRect( rcInside, srIndent,
            GetGuiColor( gcDlgBkgrnd ) );
        setcolor( GetGuiColor( gcDlgText ) );

        outtextxy( rcDlgPixels.CenterX() - szText.width/2 + 1,
                   rcDlgPixels.CenterY() - szText.height/2 + 1,
                   szCaption );
    }

    if( IsInFocus() )
        OnFocusChange();
}

void fldButton::SetText( const char* pszCaption )
/*-------------------------------------------------------------------------*
 * Sets button text.
 *-------------------------------------------------------------------------*/
{
    strcpy( szCaption, pszCaption );
    szText.width = textwidth( szCaption );
    szText.height = textheight( szCaption );
    OnDraw();
}

void fldButton::OnFocusChange()
/*-------------------------------------------------------------------------*
 * Change focus rect
 *-------------------------------------------------------------------------*/
{
    Rect rcFocus( rcDlgPixels );
    rcFocus.Inflate( -5, -5 );
    pdlgParent->FocusRect( rcFocus );
}

int fldButton::DoButton()
/*-------------------------------------------------------------------------*
 * do button.
 *-------------------------------------------------------------------------*/
{
    if( fpButton )
        return (*fpButton)(pdlgParent->pfldCurrent);
    else
        return daContinue;
}

int fldButton::OnEnter()
/*-------------------------------------------------------------------------*
 * Draw the default button as a regular button.
 *-------------------------------------------------------------------------*/
{
    int iRvl = Field::OnEnter();

    if( GetStyle() & csDefaultButton )
        return iRvl;
    pdlgParent->DrawDefaultAs( FALSE );

    return iRvl;
}

int fldButton::OnExit()
/*-------------------------------------------------------------------------*
 * Draw the old default button
 *-------------------------------------------------------------------------*/
{
    int iRvl = Field::OnExit();

    if( pdlgParent->pDefaultButton == this )
        return iRvl;

    pdlgParent->DrawDefaultAs( TRUE );

    return iRvl;
}

/*-------------------------------------------------------------------------*
 * Default pushbutton functions
 *-------------------------------------------------------------------------*/

int pbOkay( Field* )
{
    return daExitSave;
}

int pbCancel( Field* )
{
    return daExitNoSave;
}
