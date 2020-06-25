/*-------------------------------------------------------------------------*
 * coords.c:
 * functions for manipulating Rect, Size and Point structures.
 *-------------------------------------------------------------------------*/

#define _COORDS_CPP

#include <std.h>
#include "coords.hpp"
#include <graphics.h>
#include "gdef.h"

/*-------------------------------------------------------------------------*
 * units to pixels conversions
 *-------------------------------------------------------------------------*/

static int _FontWidth, _FontHeight, _FontAscent;

char *textcalc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"
                 "abcdefghijklmnopqrstuvwxyz";

int GetFontHeight()
{
    return _FontHeight;
}

int GetFontWidth()
{
    return _FontWidth;
}

void SetUnits( void )
{
    _FontWidth = (textwidth( textcalc ) * 10) / 62.0;
    _FontHeight = textheight( "s" ) * 10;

    _FontAscent = 12;
}

unint _XUnitsToPixels( unint n )
{
    unint uRvl = (n * _FontWidth / 4);
    if( uRvl % 10 >= 5 )
        uRvl += 10;
    return uRvl / 10;
}

unint _YUnitsToPixels( unint n )
{
    unint uRvl = (n * _FontHeight / 8);
    if( uRvl % 10 >= 5 )
        uRvl += 10;
    return uRvl / 10;
}

unint _XPixelsToUnits( unint n )
{
    return (n * 10) / (_FontWidth / 4);
}

unint _YPixelsToUnits( unint n )
{
    return (n * 10) / (_FontHeight / 8);
}

/*-------------------------------------------------------------------------*
 * Rect class
 *-------------------------------------------------------------------------*/

Rect::Rect( int left, int top, int right, int bottom )
{
    SetRect( left, top, right, bottom );
}

Rect::Rect( Point& ptPoint, Size& szSize )
{
    SetRect( ptPoint, szSize );
}

void Rect::SetRect( Point& ptPoint, Size& szSize )
{
    this->left = ptPoint.x;
    this->top = ptPoint.y;
    this->right = this->left + szSize.width;
    this->bottom = this->top + szSize.height;
}

void Rect::SetRect( int left, int top, int right, int bottom )
{
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
}

void Rect::UnitsToPixels()
{
    left = _XUnitsToPixels( left );
    top = _YUnitsToPixels( top );
    right = _XUnitsToPixels( right );
    bottom = _YUnitsToPixels( bottom );
}

void Rect::PixelsToUnits()
{
    left = _XPixelsToUnits( left );
    top = _YPixelsToUnits( top );
    right = _XPixelsToUnits( right );
    bottom = _YPixelsToUnits( bottom );
}

Rect& Rect::operator=( Rect& r )
{
    left = r.left;
    right = r.right;
    top = r.top;
    bottom = r.bottom;

    return *this;
}

void Rect::Inflate( int x, int y )
{
    left -= x;
    right += x;
    top -= y;
    bottom += y;
}

void Rect::MoveTo( Point& p )
{
    int iWidth = Width();
    int iHeight = Height();

    left = p.x;
    top = p.y;
    right = p.x + iWidth;
    bottom = p.y + iHeight;
}

void Rect::MoveRel( Size& s )
{
    top += s.height;
    left += s.width;
    bottom += s.height;
    right += s.width;
}

void Rect::Center( Rect& r )
{
    int iWidth = Width();
    int iHeight = Height();

    left    = (r.left + (r.Width() / 2)) - iWidth / 2;
    right   = left + iWidth;
    top     = (r.top + (r.Height() / 2)) - iHeight / 2;
    bottom  = top + iHeight;
}

void Rect::OffsetRect( Rect& r )
{
    left    += r.left;
    top     += r.top;
    right   += r.left;
    bottom  += r.top;
}

bool Rect::IsPtInside( Point& p )
{
    return (bool) (inrange( p.x, left, right ) &&
                   inrange( p.y, top, bottom ));
}

bool Rect::Intersect( Rect& r )
{
    // check for this points inside
    if( inrange( left, r.left, r.right ) ||
        inrange( right, r.left, r.right ) )
    {
        if( inrange( top, r.top, r.bottom ) )
            return TRUE;
        if( inrange( bottom, r.top, r.bottom ) )
            return TRUE;
    }

    // check for that points inside
    if( inrange( r.left, left, right ) ||
        inrange( r.right, left, right ) )
    {
        if( inrange( r.top, top, bottom ) )
            return TRUE;
        if( inrange( r.bottom, top, bottom ) )
            return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------*
 * Point class
 *-------------------------------------------------------------------------*/

Point::Point( int x, int y )
{
    this->x = x;
    this->y = y;
}

Point& Point::operator=( Point& p )
{
    x = p.x;
    y = p.y;

    return *this;
}

Point Point::operator+( Point& p )
{
    return Point( x+p.x, y+p.y );
}

Point Point::operator-( Point& p )
{
    return Point( x-p.x, y-p.y );
}

Point& Point::operator+=( Size& s )
{
    x += s.width;
    y += s.height;
    return *this;
}

Point& Point::operator+=( Point& p )
{
    x += p.x;
    y += p.y;
    return *this;
}

Point& Point::operator-=( Point& p )
{
    x -= p.x;
    y -= p.y;
    return *this;
}

Point Point::operator+( Size& s )
{
    return Point( x+s.width, y+s.height );
}

void Point::UnitsToPixels()
{
    x = _XUnitsToPixels( x );
    y = _YUnitsToPixels( y );
}

void Point::PixelsToUnits()
{
    x = _XPixelsToUnits( x );
    y = _YPixelsToUnits( y );
}

/*-------------------------------------------------------------------------*
 * Size class
 *-------------------------------------------------------------------------*/

Size::Size( int width, int height )
{
    this->width = width;
    this->height = height;
}

Size::Size( const char* pszText )
{
    width = textwidth( pszText );
    height = _FontHeight / 10;
}

Size& Size::operator=( Size& s )
{
    width = s.width;
    height = s.height;

    return *this;
}

Size& Size::operator+=( Size& s )
{
    width += s.width;
    height += s.height;

    return *this;
}

Size& Size::operator-=( Size& s )
{
    width -= s.width;
    height -= s.height;

    return *this;
}

void Size::UnitsToPixels()
{
    width = _XUnitsToPixels( width );
    height = _YUnitsToPixels( height );
}

void Size::PixelsToUnits()
{
    width = _XPixelsToUnits( width );
    height = _YPixelsToUnits( height );
}


void Rectangle( Rect& rcRect )
{
    rectangle( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
}

void Bar( Rect& rcRect )
{
    bar( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
}

void GetImage( Rect& rcRect, char* pBuffer )
{
    getimage( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom,
              pBuffer );
}

void PutImage( Rect& rcRect, char* pBuffer )
{
    putimage( rcRect.left, rcRect.top, pBuffer, COPY_PUT );
}

unsigned ImageSize( Rect& rcRect )
{
    return imagesize( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom );
}

