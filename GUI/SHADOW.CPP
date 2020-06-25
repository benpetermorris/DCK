#include <std.h>
#include <graphics.h>
#include "gdef.h"
#include "coords.hpp"
#include "inpdev.h"
#include "clr.h"
#include "shadow.h"

void shadowrect( int left, int top, int right, int bottom,
                        unsigned uFlags, int clrInterior )
{
    int     clr3DUp = GetGuiColor( gc3DUp ),
            clr3DDown = GetGuiColor( gc3DDown );
    Rect    srRect( left, top, right, bottom );

    if( uFlags & srDarker )
    {
        clr3DUp = GetGuiColor( gcDlgBkgrnd );
        clr3DDown = GetGuiColor( gcBlack );
    }

    int     clrTopLeft      = clr3DUp,
            clrBottomRight  = clr3DDown;

    bool fHidden = mouse_hide( srRect );

    // line-box-dent?
    if( uFlags & srBoxDent )
    {
        setcolor( clr3DUp );
        rectangle( srRect.left+1, srRect.top+1, srRect.right, srRect.bottom );
        setcolor( clr3DDown );
        rectangle( srRect.left, srRect.top, srRect.right-1, srRect.bottom-1 );
        mouse_show( fHidden );
        return;
    }

    if( clrInterior == -2 )
    {
        // automatically set interior color
        if( uFlags & srIndent )
            clrInterior = C_BLACK;
        else
            clrInterior = C_GRAY3;
    }

    if( uFlags & srDarkOnly )
        clrTopLeft = clrInterior;
    if( uFlags & srLightOnly )
        clrBottomRight = clrInterior;

    if( uFlags & srIndent )
    {
        // reverse them if it's indented.
        int clrTemp = clrTopLeft;
        clrTopLeft = clrBottomRight;
        clrBottomRight = clrTemp;
    }

    if( uFlags & srLineDent )
    {
        // little pretty line indents
        clrTopLeft      = clr3DDown;
        clrBottomRight  = clr3DUp;
        clrInterior = -1;
    }

_ReDrawRect:
    setcolor( clrTopLeft );
    moveto( srRect.left, srRect.bottom );
    lineto( srRect.left, srRect.top );
    lineto( srRect.right, srRect.top );
    setcolor( clrBottomRight );
    lineto( srRect.right, srRect.bottom );
    lineto( srRect.left+1, srRect.bottom );

    if( uFlags & srThick )
    {
        uFlags &= ~srThick; // so we don't do it again
        srRect.Inflate( -1, -1 );   // make smaller rect
        goto _ReDrawRect;
    }

    // draw interior
    if( clrInterior != -1 )
    {
        srRect.Inflate( -1, -1 );
        setfillstyle( SOLID_FILL, clrInterior );
        bar( srRect.left, srRect.top, srRect.right, srRect.bottom );
    }

    mouse_show( fHidden );
}

