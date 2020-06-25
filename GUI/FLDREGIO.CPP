#include <std.h>
#include "dialog.hpp"

fldRegion::fldRegion( Dialog* pDialog, Rect& rcRect, int iStyle ) :
/*-------------------------------------------------------------------------*
 * Defines a region field
 *-------------------------------------------------------------------------*/
    Field( pDialog, ftRegion, iStyle )
{
    Create( rcRect.TopLeft(), Size( rcRect.Width(), rcRect.Height() ) );

    // black inside is the default
    clrFill = gcBlack;
    // indented shadow rect is the default
    uRectFlags = srThin | srIndent;
}

void fldRegion::Draw()
/*-------------------------------------------------------------------------*
 * Draws the region
 *-------------------------------------------------------------------------*/
{
    if( uRectFlags != 0xFFFF )
        pdlgParent->ShadowRect( rcDlgPixels, uRectFlags, GetGuiColor( clrFill ) );
    else
    {
        pdlgParent->FrameRect( rcDlgPixels );
        Rect rcDeflated( rcDlgPixels );
        rcDeflated.Inflate( -1, -1 );
        pdlgParent->FillRect( rcDeflated, GetGuiColor( clrFill ) );
    }
}

void fldRegion::Erase()
/*-------------------------------------------------------------------------*
 * Erases the region!
 *-------------------------------------------------------------------------*/
{
    pdlgParent->FillRect( rcDlgPixels );
}

int fldRegion::Event( Event_t& )
/*-------------------------------------------------------------------------*
 * Process events - default is none.
 *-------------------------------------------------------------------------*/
{
    return daContinue;
}

void fldRegion::Replace( fldRegion* pNewField )
/*-------------------------------------------------------------------------*
 * Replaces this field with pNewField - copies all of this field's data
 * into pNewField, deletes itself, and returns.
 *-------------------------------------------------------------------------*/
{
    *pNewField = *this;
    Field::Replace( pNewField );
    delete this;
}
