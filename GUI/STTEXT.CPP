#include <std.h>
#include "dialog.hpp"

stText::stText( Dialog* pdlgParent, Point& ptPoint, const char* pszText,
                int iStyle ) :
/*-------------------------------------------------------------------------*
 * Creates a stText object.  Is this obvious?
 *-------------------------------------------------------------------------*/
    Label( pdlgParent, ltText, iStyle )
{
    Size szUnits( pszText );
    szUnits.PixelsToUnits();

    Create( ptPoint, szUnits );
    strcpy( szText, pszText );

    iColor = gcDlgText;
    iBackColor = gcDlgBkgrnd;

    rcDlgPixels.top++;
}

void stText::SetColor( int iColor )
{
    this->iColor = iColor;
}

void stText::SetBackColor( int iColor )
{
    this->iBackColor = iColor;
}

void stText::SetText( const char* pszText )
{
    if( !strcmp( pszText, szText ) )
        return;

    OnErase();

    strcpy( szText, pszText );
    Size szPixels( szText );
    rcDlgPixels.SetRect( rcDlgPixels.TopLeft(), szPixels );

    OnDraw();
}

void stText::Draw()
{
    if( !CanDraw() )
        return;

    setcolor( GetGuiColor( iColor ) );
    pdlgParent->DrawTextXY( ptDlgUnits, szText );
}

void stText::Erase()
{
    if( !CanDraw() )
        return;

    setfillstyle( SOLID_FILL, GetGuiColor( iBackColor ) );
    Bar( rcDlgPixels );
    // pdlgParent->EraseTextXY( ptDlgUnits, szText );
}
