#ifndef _INPDEV_H
#define _INPDEV_H

#include <graphics.h>
#include "gdef.h"

const int iCursorHeight = 20;

/* Mouse events */
typedef struct TAGmevent
{
    short   x, y, what;
    ulong   when;
    struct  TAGmevent *next;

} MEVENT;

enum
{
    MOUSE_LDOWN = -1,
    MOUSE_LUP = -2,
    MOUSE_RDOWN = -3,
    MOUSE_RUP = -4,
    MOUSE_MDOWN = -5,
    MOUSE_MUP = -6,

    MOUSE_NOTHING = 0,
    MOUSE_MOVE = 1,

    MOUSE_LCLICK = 2,
    MOUSE_LDBLCLICK = 3,
    MOUSE_LDRAGINIT = 4,
    MOUSE_LDRAGEXIT = 5,
    MOUSE_LDBLDRAGINIT = 6,

    MOUSE_RCLICK = 12,
    MOUSE_RDBLCLICK = 13,
    MOUSE_RDRAGINIT = 14,
    MOUSE_RDRAGEXIT = 15,
    MOUSE_RDBLDRAGINIT = 16,

    MOUSE_MCLICK = 22,
    MOUSE_MDBLCLICK = 23,
    MOUSE_MDRAGINIT = 24,
    MOUSE_MDRAGEXIT = 25,
    MOUSE_MDBLDRAGINIT = 26
};

enum
{
    ctArrow,
    ctCross
};

void SetCursorType( int iType );
void mouse_lastcoords( short *x, short *y );
unsigned mouse_status( void );
short mouse_event( short *x, short *y );
void mouse_setcoords( short x, short y );
void mouse_setlimit( short x1, short y1, short x2, short y2 );
void mouse_setpage( short page );
void mouse_hide( void );
void mouse_show( void );
void mouse_getcoords( short *x, short *y );
void mouse_kill( void );
void mouse_setlock( uchar xlock, uchar ylock );
void mouse_init( void );
void movecur( short x, short y );
void killcur( void );
void drawcur( short x, short y );

unsigned short keystat( void );
unsigned keycheck( void );
unsigned keylast( void );
unsigned keyget( void );
void keystuff( unsigned ch );

void mouse_coords( Point& ptPoint );
void mouse_show( bool fShow );
bool mouse_hide( Rect& rcRect );
void SetViewPort( int left, int top, int right, int bottom, bool fClip = 1 );

#ifndef _MOUSE_CPP
extern int scursorx, scursory;
#endif

#endif // _INPDEV_H
