/*-------------------------------------------------------------------------*
 * coords.hpp:
 * Rect, Size and Point class header.
 *-------------------------------------------------------------------------*/

#ifndef _COORDS_HPP
#define _COORDS_HPP

class Rect;
class Size;
class Point;

#define XUPC    4
#define YUPC    8

#define inrange( n, min, max )      ((n) >= (min) && (n) <= (max))

void SetUnits( void );
unint _XUnitsToPixels( unint n );
unint _YUnitsToPixels( unint n );
unint _XPixelsToUnits( unint n );
unint _YPixelsToUnits( unint n );
int GetFontHeight();
int GetFontWidth();

class Size
{
public:
    Size( int width, int height );
    Size( const char* );
    Size() : width(0), height(0) {}

    void PixelsToUnits();
    void UnitsToPixels();

    Size& operator+=( Size& );
    Size& operator-=( Size& );
    Size& operator=( Size& );
    inline unsigned operator==( Size& s )
        { return (s.width == width && s.height == height); }
    inline unsigned operator!=( Size& s )
        { return (s.width != width || s.height != height); }
    inline Size operator+( Size& s )
        { return Size( s.width + width, s.height + height ); }

    int width,
        height;
};

class Point
{
public:
    Point( int x, int y );
    Point() : x(0), y(0) {}

    void PixelsToUnits();
    void UnitsToPixels();

    inline unsigned operator==( Point& p )
        { return (p.x == x && p.y == y); }
    inline unsigned operator!=( Point& p )
        { return (p.x != x || p.y != y); }
    Point& operator=( Point& );
    Point& operator+=( Point& );
    Point& operator-=( Point& );
    Point& operator+=( Size& );
    Point operator+( Point& );
    Point operator-( Point& );
    Point operator-( Size& szSize )
        { return Point( x - szSize.width, y - szSize.height ); }
    Point operator+( Size& );

    int x, y;
};

class Rect
{
public:
    Rect( int left, int top, int right, int bottom );
    Rect( Point&, Size& );
    Rect() : left(0), top(0), right(0), bottom(0) {}

    void SetRect( int left, int top, int right, int bottom );
    void SetRect( Point&, Size& );

    inline int CenterX()
        { return (right+left)/2; }
    inline int CenterY()
        { return (top+bottom)/2; }

    void MoveTo( Point& );
    void MoveRel( Size& );
    inline void MoveRel( Point& p )
        { left += p.x; right += p.x;
          top += p.y; bottom += p.y; }
    void Inflate( int x, int y );
    void Center( Rect& );
    void OffsetRect( Rect& );
    inline Point TopLeft()
        {
            return Point( left, top );
        }
    inline Point BottomRight()
        {
            return Point( right, bottom );
        }

    void PixelsToUnits();
    void UnitsToPixels();

    bool IsPtInside( Point& ptPoint );
    bool Intersect( Rect& rcCheck );
    inline int Height()
        {
            return bottom-top;
        }
    inline int Width()
        {
            return right-left;
        }

    Rect& operator=( Rect& );

    int left,
        top,
        right,
        bottom;
};

unsigned ImageSize( Rect& rcRect );
void PutImage( Rect& rcRect, char* pBuffer );
void GetImage( Rect& rcRect, char* pBuffer );
void Bar( Rect& rcRect );
void Rectangle( Rect& rcRect );

#endif // _COORDS_HPP
