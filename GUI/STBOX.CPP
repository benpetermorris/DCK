#include <std.h>
#include "dialog.hpp"

stBox::stBox( Dialog* pdlgParent, Rect& rcRect, const char* pszText ) :
    Label( pdlgParent, ltBox, 0 )
{
    Create( rcRect.TopLeft(), Size( rcRect.Width(), rcRect.Height() ) );

    // create some text
    if( pszText )
    {
        Point ptText = rcRect.TopLeft() + Point( 4, -4 );
        char szMyText[128];
        sprintf( szMyText, " %s ", pszText );
        stText *pText = new stText( pdlgParent, ptText, szMyText, csEraseFirst );
        //pText->SetBackColor( gcDlgTitleBarActive );
        pText->SetColor( gcDlgTitleText );
    }
}

void stBox::Draw()
{
    if( !CanDraw() )
        return;

    pdlgParent->ShadowRect( rcDlgPixels, srBoxDent );
}

void stBox::Erase()
{
}


stLine::stLine( Dialog* pdlgParent, Rect& rcRect, int iLineType ) :
    Label( pdlgParent, iLineType, 0 )
{
    Create( rcRect.TopLeft(), Size( rcRect.Width(), rcRect.Height() ) );

    rcLine = rcDlgPixels;

    if( GetType() == ltVertical )
        rcLine.right = rcLine.left + 1;
    else
        rcLine.bottom = rcLine.top + 1;
}

void stLine::Draw()
{
    if( !CanDraw() )
        return;

    pdlgParent->ShadowRect( rcLine, srLineDent );
}

void stLine::Erase()
{
}
