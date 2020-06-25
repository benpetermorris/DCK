#include <std.h>
#include "dialog.hpp"

Label::Label( Dialog* pdlgParent, int iType, int iStyle )
/*-------------------------------------------------------------------------*
 * Initializes a Label class.
 *-------------------------------------------------------------------------*/
{
    // initialize parent
    this->pdlgParent = pdlgParent;

    // initialize previous/next Statics
    if( !pdlgParent->plbFirst )
    {
        pdlgParent->plbFirst = this;
        pdlgParent->plbFirst->plbNext = this;
        pdlgParent->plbFirst->plbPrev = this;
    }
    plbPrev = pdlgParent->plbFirst->plbPrev;
    plbNext = pdlgParent->plbFirst;
    pdlgParent->plbFirst->plbPrev->plbNext = this;
    pdlgParent->plbFirst->plbPrev = this;

    // initialize type
    this->iType = iType;
    this->iStyle = iStyle;  // default styles
    this->pdlgParent = pdlgParent;
    szName[0] = 0;
    SetStyle( csDiscardable, TRUE );
}

Label::~Label()
{
}

void Label::OnDraw()
/*-------------------------------------------------------------------------*
 * Hides the mouse and calls the draw function.
 *-------------------------------------------------------------------------*/
{
    bool fHidden = mouse_hide( rcDlgPixels );
    if( GetStyle() & csEraseFirst )
        Erase();
    Draw();
    mouse_show( fHidden );
}

void Label::OnErase()
/*-------------------------------------------------------------------------*
 * Hides the mouse and calls the draw function.
 *-------------------------------------------------------------------------*/
{
    bool fHidden = mouse_hide( rcDlgPixels );
    Erase();
    mouse_show( fHidden );
}

void Label::Create( const Point& ptPoint, const Size& szSize )
/*-------------------------------------------------------------------------*
 * Initializes size structures for the Label.
 *-------------------------------------------------------------------------*/
{
    if( GetStyle() & csCreateAbsSize )  // create absolute size if that style is set
        CreateAbsSize( ptPoint, szSize );
    else
    {
        Size szMySize = szSize;
        szMySize.UnitsToPixels();
        CreateAbsSize( ptPoint, szMySize );
    }
}

void Label::CreateAbsSize( const Point& ptPoint, const Size& szSize )
/*-------------------------------------------------------------------------*
 * Initializes size structures for the Label.
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
