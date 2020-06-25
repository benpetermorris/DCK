/*-------------------------------------------------------------------------*
 * Custom field implementations.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "fields.hpp"

/*-------------------------------------------------------------------------*
 *
 * fldWallTex: Wall texture field.
 *
 *-------------------------------------------------------------------------*/

fldWallTex::fldWallTex() :
    fldRegion(),
    iWallIndex(-1),
    iOldIndex(-1),
    pTex(NULL)
{
}

void fldWallTex::SetWall( const char* pszWallName )
{
    iWallIndex = texture_toint( (char*) pszWallName );
    pTex = &texlist[iWallIndex];
    if( pdlgParent )
        OnDraw();
}

void fldWallTex::SetWall( int iWallIndex )
{
    this->iWallIndex = iWallIndex;
    pTex = &texlist[iWallIndex];
    if( pdlgParent )
        OnDraw();
}

void fldWallTex::Draw()
{
    if( iOldIndex == iWallIndex && !IsFirstDraw() )
        return;

    iOldIndex = iWallIndex;

    // call the region draw function
    fldRegion::Draw();

    if( iWallIndex == -1 )
        return;

    // draw the Wall
    Rect rcRect = GetRectInParent();
    texture_disp( rcRect.left+1, rcRect.top+1, rcRect.Width(),
        rcRect.Height(), 0, 0, pTex, FALSE );
}

void fldWallTex::Transfer( int iDirection )
{
    int *ipXfer = (int*) pvTransferBuf;

    if( iDirection == transFromField )
        ipXfer[0] = GetWall();
    else
        SetWall( ipXfer[0] );
}

/*-------------------------------------------------------------------------*
 *
 * fldFlatTex: Flat texture field
 *
 *-------------------------------------------------------------------------*/

fldFlatTex::fldFlatTex() :
    fldRegion(),
    iFlatIndex(-1)
{
}

void fldFlatTex::SetFlat( const char* pszFlatName )
{
    iFlatIndex = floor_toint( (char*) pszFlatName );
    OnDraw();
}

void fldFlatTex::SetFlat( int iFlatIndex )
{
    this->iFlatIndex = iFlatIndex;
    OnDraw();
}

void fldFlatTex::Draw()
{
    if( iOldIndex == iFlatIndex && !IsFirstDraw() )
        return;

    iOldIndex = iFlatIndex;

    // call the region draw function
    if( IsFirstDraw() )
        fldRegion::Draw();

    // draw the flat
    Rect rcRect = GetRectInParent();

    // don't draw the X
    if( GetParent()->GetMultiEdit() && iFlatIndex == -1 )
        return;

    floor_disp( rcRect.left+1, rcRect.top+1, iFlatIndex );
}

void fldFlatTex::Transfer( int iDirection )
{
    int *ipXfer = (int*) pvTransferBuf;

    if( iDirection == transFromField )
        ipXfer[0] = GetFlat();
    else
        SetFlat( ipXfer[0] );
}
