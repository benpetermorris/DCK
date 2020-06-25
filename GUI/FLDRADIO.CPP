#include <std.h>
#include "dialog.hpp"

fldRadio::fldRadio( Dialog* pDialog, Point& ptPoint, int iGroup,
                const char* pszPrompt, int iStyle, int iType ) :
/*-------------------------------------------------------------------------*
 * Create a radio button
 *-------------------------------------------------------------------------*/
    Field( pDialog, iType, iStyle ),
    iState(0)
{
    int iSize = (GetFontHeight()*2)/30;

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

    // what's the number of this one in its group?
    Field *pField = pdlgParent->EnumFields( NULL );
    int iMaxNum = -1;

    while( pField )
    {
        if( pField != this &&
            pField->GetType() == ftRadio ||
            pField->GetType() == ftRadio2 )
        {
            fldRadio* pRadio = (fldRadio*) pField;
            if( pRadio->iGroup == iGroup )
            {
                if( pRadio->iNum > iMaxNum )
                    iMaxNum = pRadio->iNum;
                if( pRadio->iNum == 0 )
                    pGroupFirst = pRadio;
            }
        }
        pField = pdlgParent->EnumFields( pField );
    }

    iNum = iMaxNum + 1;

    if( !iNum )
        pGroupFirst = this;

    pGroupFirst->iMaxNum = iNum;

    this->iGroup = iGroup;
}

fldRadio* fldRadio::FindNumInGroup( int iNum, int iGroup )
/*-------------------------------------------------------------------------*
 * Finds the iNum'th button belonging to iGroup.
 *-------------------------------------------------------------------------*/
{
    Field *pField = pdlgParent->EnumFields( NULL );

    if( iNum == -1 )    // none 'o that!
        return NULL;

    if( iGroup == -1 )
        iGroup = this->iGroup;

    while( pField )
    {
        if( pField->GetType() == ftRadio ||
            pField->GetType() == ftRadio2 )
        {
            fldRadio* pRadio = (fldRadio*) pField;

            if( pRadio->iGroup == iGroup &&
                pRadio->iNum == iNum )
                return pRadio;
        }
        pField = pdlgParent->EnumFields( pField );
    }

    return NULL;
}

int fldRadio::Event( Event_t& event )
/*-------------------------------------------------------------------------*
 * Process events.
 *-------------------------------------------------------------------------*/
{
    fldRadio *pRadio;

    switch( event.type )
    {
    case etKey:
        switch( event.iCode )
        {
        case KEY_SPACE:
            Set();
            break;
        case KEY_LEFT:
        case KEY_UP:
            pRadio = FindNumInGroup( iNum-1 );
            if( pRadio == NULL )
                pRadio = FindNumInGroup( pGroupFirst->iMaxNum );
            if( pRadio == NULL )
                break;
            GetParent()->GoToItem( pRadio );
            pRadio->Set();
            break;
        case KEY_RIGHT:
        case KEY_DOWN:
            pRadio = FindNumInGroup( iNum+1 );
            if( pRadio == NULL )
                pRadio = FindNumInGroup( 0 );
            if( pRadio == NULL )
                break;
            GetParent()->GoToItem( pRadio );
            pRadio->Set();
            break;
        }
        break;
    case etMouse:
        if( event.iCode == MOUSE_LUP &&
            event.iHitWhere == htInside )
            Set();
        // that's the only mouse event processed here
        break;
    }

    return daContinue;
}

void fldRadio::Set()
/*-------------------------------------------------------------------------*
 * Sets the radio button.
 * Finds the last one that was set, and unsets it.  If the last one
 * that was set is THIS one, and the type is a Radio2 type, it unsets
 * it and returns.  Otherwise it redraws the old one and this one.
 *-------------------------------------------------------------------------*/
{
    if( (pGroupFirst->GetStyle() & csReadOnly) )
        return;

    fldRadio *pRadio = FindNumInGroup( GetState() );

    // previously selected
    if( pRadio == this && pGroupFirst->GetType() == ftRadio2 )
    {
        SetState( -1 );
        OnDraw();
        OnChange();
    }
    else if( pRadio != this )
    {
        SetState( iNum );
        if( pRadio )
            pRadio->OnDraw();
        OnDraw();
        OnChange();
    }
}

void fldRadio::Transfer( int iDirection )
/*-------------------------------------------------------------------------*
 * Transfers to/from the field.
 *-------------------------------------------------------------------------*/
{
    if( !pvTransferBuf )
        return;

    if( iNum )  // only the 0th in a radio group does the transfer
        return;

    if( iDirection == transFromField )
        ((int*) pvTransferBuf)[0] = GetState();
    else
        SetState( ((int*) pvTransferBuf)[0] );

    return;
}

void fldRadio::Draw()
/*-------------------------------------------------------------------------*
 * Draw the radio button.
 *-------------------------------------------------------------------------*/
{
    if( !CanDraw() )
        return;

    int iXpos = rcDlgPixels.CenterX(),
        iYpos = rcDlgPixels.CenterY(),
        iColor,
        iRadius = rcDlgPixels.Height()/2;

    int clrIndex = (GetStyle() & csSkip) ? gcDlgBkgrnd : gcFldBkgrnd;

    if( !GetDrawDown() )
        setfillstyle( SOLID_FILL, iColor = GetGuiColor( clrIndex ) );
    else
        setfillstyle( SOLID_FILL, iColor = GetGuiColor( gcDlgBkgrnd ) );

    setcolor( iColor );
    fillellipse( iXpos, iYpos, iRadius-2, iRadius-2 );

    if( IsFirstDraw() )
    {
        setcolor( GetGuiColor( gc3DDown ) );
        arc( iXpos, iYpos, 45, 235, iRadius );
        setcolor( GetGuiColor( gc3DUp ) );
        arc( iXpos, iYpos, 235, 405, iRadius );
    }

    if( GetState() == iNum )
    {
        setfillstyle( SOLID_FILL, GetGuiColor( gcFldText ) );
        setcolor( iColor );
        fillellipse( iXpos, iYpos, iRadius-2, iRadius-2 );
    }
    else
    {
        //setfillstyle( SOLID_FILL, GetGuiColor( clrIndex ) );
        //setcolor( iColor );
        //fillellipse( iXpos, iYpos, iRadius-3, iRadius-3 );
    }
}

void fldRadio::Erase()
/*-------------------------------------------------------------------------*
 * Erase the radio button.
 *-------------------------------------------------------------------------*/
{
    if( !CanDraw() )
        return;

    int iXpos = rcDlgPixels.CenterX(),
        iYpos = rcDlgPixels.CenterY(),
        iRadius = rcDlgPixels.Height()/2;

    setfillstyle( SOLID_FILL, GetGuiColor( gcDlgBkgrnd ) );
    fillellipse( iXpos, iYpos, iRadius, iRadius );
}
