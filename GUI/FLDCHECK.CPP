#include <std.h>
#include "dialog.hpp"

fldCheckBox::fldCheckBox( Dialog* pdlgParent, Point& ptPoint,
                          const char* pszPrompt, int iStyle, int iType ) :
/*-------------------------------------------------------------------------*
 * Initialize a checkbox field.
 *-------------------------------------------------------------------------*/
    Field( pdlgParent, iType, iStyle )
{
    int iSize = (GetFontHeight()*2)/25;

    Size szSize( iSize, iSize );
    CreateAbsSize( ptPoint, szSize );

    // create text prompt
    if( pszPrompt )
    {
        szSize.PixelsToUnits();
        stText* textp = new stText( pdlgParent,
            ptPoint + Point( szSize.width + 4, -1 ), pszPrompt );
        plbPrompt = (Label*) textp;
        textp->SetStyle( csDiscardable, TRUE );
    }
}

int fldCheckBox::Event( Event_t& event )
/*-------------------------------------------------------------------------*
 * Process that event, yes.
 *-------------------------------------------------------------------------*/
{
    switch( event.type )
    {
    case etKey:
        if( event.iCode == KEY_SPACE )
            Toggle();
        return daContinue;
    case etMouse:
        if( event.iCode == MOUSE_LUP &&
            event.iHitWhere == htInside )
            Toggle();
        // that's the only mouse event processed here
        break;
    }

    return daContinue;
}

void fldCheckBox::Toggle()
/*-------------------------------------------------------------------------*
 * toggles the checkbox.
 *-------------------------------------------------------------------------*/
{
    if( (GetStyle() & csReadOnly) )
        return;

    if( GetType() == ftCheckBox )
        SetState( !iState );
    else
    {
        // 3-state checkbox
        int iNewState = GetState();
        iNewState++;
        if( iNewState == 3 )
            iNewState = 0;
        SetState( iNewState );
    }

    OnChange();
}

void fldCheckBox::Draw()
/*-------------------------------------------------------------------------*
 * Draws the checkbox (fascinating commentary.)
 *-------------------------------------------------------------------------*/
{
    unsigned char *pCheckPat = "\x55\xAA\x55\xAA\x55\xAA\x55\xAA";

    if( !CanDraw() )
        return;

    int clrIndex = (GetStyle() & csSkip) ? gcDlgBkgrnd : gcFldBkgrnd;

    // draw the rectangle
    pdlgParent->ShadowRect( rcDlgPixels, srIndent, -1 );
    Rect rcCheck = rcDlgPixels;
    rcCheck.Inflate( -1, -1 );
    pdlgParent->ShadowRect( rcCheck, srIndent | srDarker,
        GetGuiColor( clrIndex ) );
    rcCheck.Inflate( -2, -2 );
    // is it checked?
    if( iState == 1 )
    {
        setlinestyle( SOLID_LINE, 0, 1 );
        setcolor( GetGuiColor( gcFldText ) );
        line( rcCheck.left, rcCheck.top,
              rcCheck.right, rcCheck.bottom );
        line( rcCheck.left, rcCheck.bottom,
              rcCheck.right, rcCheck.top );
    }
    else if( iState == 2 )
    {
        // setcolor( GetGuiColor( gcFldText ) );
        setfillpattern( pCheckPat, GetGuiColor( gcFldText ) );
        bar( rcCheck.left, rcCheck.top, rcCheck.right, rcCheck.bottom );
    }

    if( GetDrawDown() )
    {
        rcCheck.Inflate( 1, 1 );
        pdlgParent->FrameRect( rcCheck, GetGuiColor( gcFldText ) );
    }
}

void fldCheckBox::Erase()
/*-------------------------------------------------------------------------*
 * Erases the checkbox.
 *-------------------------------------------------------------------------*/
{
    if( !CanDraw() )
        return;

    pdlgParent->FillRect( rcDlgPixels );
}

void fldCheckBox::Transfer( int iDirection )
/*-------------------------------------------------------------------------*
 * Transfer stuff.
 *-------------------------------------------------------------------------*/
{
    if( !pvTransferBuf )    // nothing to transfer to/from
        return;

    if( iDirection == transFromField )
        ((char*) pvTransferBuf)[0] = iState;
    else
        iState = ((char*) pvTransferBuf)[0];

    return;
}
