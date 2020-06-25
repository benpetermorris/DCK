/*-------------------------------------------------------------------------
 * INPDEV.C:
 * Mouse interface routines.
 * Keyboard interface routines.
 *-------------------------------------------------------------------------*/

#pragma -N-

#define _MOUSE_CPP

#include <std.h>
#include <graphics.h>
#include "gui.hpp"

int ismouse;
char _md_flag = 1;
Point ptView;

/* mouse interrupt number */
#define MOUSEINT 0x33

#define MYMOUSECUR

#ifdef __FLAT__
#define int86           int386
#endif

static  MEVENT  *_etfirst;
MEVENT  *_etcur;
MEVENT  *_etplace;

short   _bios_sel;
short   key_tics;
extern int     iScreenWidth, iScreenHeight;
int scursorx, scursory;

/* for demoRecord.event */
#define drKeyboard  0x80
#define drTextBox   0xFF

#define WORD        unsigned short
#define DWORD       unsigned long

#define LOWORD(l)            ((WORD)(l))
#define HIWORD(l)            ((WORD)((DWORD)(l) >> 16))
#define MAKEDWORD(low, high) ((DWORD)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

typedef struct
{
    signed     mx : 10;
    signed     my : 10;
    unsigned   ks : 4;      /* key status (only one key stored) */

} evMouse;

typedef struct
{
    unsigned  key : 16;     /* the keypress */
    unsigned   ks : 4;      /* key status */

} evKeyboard;

typedef struct
{
    unsigned idx  : 8;      /* index into message buffers array */

} evTextBox;

typedef struct
{
    uchar event;

    union
    {
        evMouse     m;
        evKeyboard  k;
        evTextBox   t;

    } e;

    /* delay since last event */
    ushort tdelay;

} demoRecord;

static bool fNoHandler;

static  demoRecord *dQueue, *dqp;
short   demo_record, demo_play, demo_paused;
static  char    *demo_fn;
static  short   ndRecord;
static  ushort  drKeyStat;
static  ulong   last_dtime;
static  short   nmsg = 0;

static  char    _mou_lockx, _mou_locky, _mou_moved;
static  short   mready;
static  long    ldownwhen, rdownwhen, mdownwhen;
static  short   ldownx, ldowny;
static  short   rdownx, rdowny;
static  short   mdownx, mdowny;
static  short   drag_tics, dclick_tics;
static  short   m_minx, m_maxx, m_miny, m_maxy;
#ifdef MYMOUSECUR
static  short   clevel = -1;
static  short   lastx = -1, lasty = -1, con = 0;
static  uchar   *munderbuf;
#endif
#ifdef MDEBUG
static  FILE    *logfp;
#endif


short   mou_ldrag, mou_rdrag, mou_mdrag;
short   mou_ldown, mou_rdown, mou_mdown;
short   mou_deltax, mou_deltay;
short   mou_trapdrag = 1;

#define _MOVE       0x01
#define _LPRESS     0x02
#define _LRELEASE   0x04
#define _RPRESS     0x08
#define _RRELEASE   0x10
#define _MPRESS     0x20
#define _MRELEASE   0x40

// extern ulong _tics;
#define _tics  (biostime( 0,0L))

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

void _mouse_setevent( void );
static void _me_init( void );
void setdemotimer( void );
short dGetMsgBox( void );
void dShowMsgBox( void );

static unsigned long _oldesp, _myesp;

static void drawcur( short x, short y )
{
    if( !con && clevel >= 0 )
    {
        short oc = getcolor();

#if 1   /* poly cur */
        int cpoints[] =
        {
            0, 0,
            16, 16,
            8, 16,
            0, 20,
            0, 0
        };

        lastx = x;
        lasty = y;
        getimage( x, y, x+iCursorHeight, y+iCursorHeight, munderbuf );
        setviewport( x, y, x+iCursorHeight, y+iCursorHeight, 0 );
        setwritemode( COPY_PUT );
        setlinestyle( SOLID_LINE, 0, 1 );
        setfillstyle( SOLID_FILL, C_WHITE );
        setcolor( C_BLACK );
        _oldesp = _ESP;
        _ESP = _myesp;
        fillpoly( 4, cpoints );
        drawpoly( 5, cpoints );
        _ESP = _oldesp;
#else
        lastx = x;
        lasty = y;
        getimage( x, y, x+iCursorHeight, y+iCursorHeight, munderbuf );
        setcolor( C_LIGHTGREEN );
        setwritemode( COPY_PUT );
        setlinestyle( SOLID_LINE, 0, 1 );
        moveto( x+0, y+0 );
        lineto( x+12, y+12 );
        lineto( x+5, y+12 );
        lineto( x+0, y+16 );
        lineto( x+0, y+0 );
        moveto( x+1, y+2 );
        lineto( x+11, y+12 );
        lineto( x+4, y+12 );
        lineto( x+1, y+15 );
        lineto( x+1, y+1 );
#endif
        setcolor( oc );

        con = 1;
    }
}

static void killcur( void )
{
    if( con )
    {
        putimage( lastx, lasty, munderbuf, COPY_PUT );
        con = 0;
    }
}

static short m_lastmx, m_lastmy, m_lastx, m_lasty;

short me_proc;
long me_total;
ulong me_nothing_e, me_nothing_d;

static void mousehandler( long timenow, short mx, short my, ushort code )
/*-------------------------------------------------------------------------*
 * Guess what this does
 *-------------------------------------------------------------------------*/
{
    short   x, y;
    static  short putx;
    struct  viewporttype vp;

#ifdef DEBUG
    ++me_proc;
    me_total += (mx + my + code);
#endif

    mou_deltax = mx;
    mou_deltay = my;

    if( !_mou_lockx )
        x = m_lastx + mou_deltax;
    else x = m_lastx;

    if( !_mou_locky )
        y = m_lasty + mou_deltay;
    else y = m_lasty;

    if( x > m_maxx )
        x = m_maxx;
    else if( x < m_minx )
        x = m_minx;

    if( y > m_maxy )
        y = m_maxy;
    else if( y < m_miny )
        y = m_miny;

    m_lastx = x;
    m_lasty = y;

    if( code & _MOVE )
    {
        scursorx = x;
        scursory = y;

        /* moved */
        _mou_moved = 1;

        if( _md_flag >= 1 && clevel >= 0 )
        {
            getviewsettings( &vp );
            setviewport( 0, 0, iScreenWidth, iScreenHeight, 1 );

            killcur();
            /* DRAWCUR CHANGES VIEWPORT !! !!! */
            drawcur( scursorx, scursory );

            setviewport( vp.left, vp.top, vp.right, vp.bottom, 1 );
        }
    }

    code &= ~_MOVE;

/*
    if( code & _LPRESS )
        code = _LPRESS;
    else if( code & _LRELEASE )
        code = _LRELEASE;
    else if( code & _RPRESS )
        code = _RPRESS;
    else if( code & _RRELEASE )
        code = _RRELEASE;
    else if( code & _MPRESS )
        code = _MPRESS;
    else if( code & _MRELEASE )
        code = _MRELEASE;
*/

    if( code != 0 )
    {
        _etplace->x = x;
        _etplace->y = y;
        _etplace->what = code;
        _etplace->when = timenow;
        _etplace = _etplace->next;
    }
}

void _mcheckevent( void )
{
}

extern "C" void _m_handler( void )
{
    short   mx = _SI, my = _DI, tmp;
    ushort  code = _AX;
    ulong   timenow = _tics;

    if( fNoHandler )
        return;

    if( demo_play )
        return;

    tmp = mx;
    mx -= m_lastmx;
    m_lastmx = tmp;

    tmp = my;
    my -= m_lastmy;
    m_lastmy = tmp;

    code &= 0x7F;

    asm push ds
    mousehandler( timenow, mx, my, code );
    asm pop ds
}

void _MouseSetHandler( void )
{
    asm {
        push es
        mov  ax,cs
        mov  es,ax
        mov  edx,OFFSET _m_handler
        mov  ax,0x000c
        mov  cx,0x007f
        int  0x33
        pop  es
    }
}

void _MouseKillHandler( void )
{
    asm {
        push es
        mov  ax,cs
        mov  es,ax
        mov  edx,0
        mov  ax,0x000c
        mov  cx,0x0000
        int  0x33
        pop  es
    }
}

void mouse_init( void )
{
    union REGS regs;

    _myesp = _ESP - 12000;

    regs.x.ax = 0x0000;
    int86(MOUSEINT, &regs, &regs);
    if (regs.x.ax == 0xffff)
        ismouse = 1; /* mouse */
    else
        ismouse = 0; /* no mouse */

    if( ismouse )
    {
        _me_init();
        clevel = -1;
        con = 0;
        mready = 1;
        regs.x.ax = 2;
        regs.x.bx = 0x40;
        int386( 0x31, &regs, &regs );
        _bios_sel = regs.x.ax;

        _MouseSetHandler();
        mouse_setlimit( 0, 0, iScreenWidth, iScreenHeight );

        drag_tics = 6; //getduration( 30 );
        dclick_tics = 6; //getduration( 30 );

        //if( demo_play || demo_record )
            //setdemotimer();
    }
}

void mouse_setlock( uchar xlock, uchar ylock )
{
    _mou_lockx = xlock;
    _mou_locky = ylock;
}

void mouse_kill( void )
{
    union REGS regs;

    if( !ismouse || !mready )
        return;

    _MouseKillHandler();

    mready = 0;
    regs.x.ax = 0x21;   /* software reset */
    int86(MOUSEINT, &regs, &regs);
}

void mouse_getcoords( short *x, short *y )
{
    *x = scursorx;
    *y = scursory;
}

void mouse_show( void )
{
    struct  viewporttype vp;

    if( !ismouse )
        return;

    ++clevel;

    if( clevel == 0 )
    {
        //_killhandler();
        fNoHandler = 1;
        getviewsettings( &vp );
        setviewport( 0, 0, iScreenWidth, iScreenHeight, 1 );
        drawcur( scursorx, scursory );
        setviewport( vp.left, vp.top, vp.right, vp.bottom, 1 );
        //_sethandler();
        fNoHandler = 0;
    }
}

void mouse_hide( void )
{
    struct  viewporttype vp;

    if( !ismouse )
        return;

    --clevel;

    if( clevel == -1 )
    {
        //_killhandler();
        fNoHandler = 1;
        getviewsettings( &vp );
        setviewport( 0, 0, iScreenWidth, iScreenHeight, 1 );
        killcur();
        setviewport( vp.left, vp.top, vp.right, vp.bottom, 1 );
        //_sethandler();
        fNoHandler = 0;
    }
}

static void _me_init( void )
{
    MEVENT *m;
    short i;

    if( _etfirst )
        return;     /* Already done */

    munderbuf = (uchar*) malloc( imagesize( 0, 0, iCursorHeight+2,
            iCursorHeight+2 ) );

    m = _etfirst = (MEVENT*) malloc( sizeof( MEVENT ) );

    for( i = 0; i < 32; i++ )
    {
        m->next = (MEVENT*) malloc( sizeof( MEVENT ) );
        m = m->next;
    }

    m->next = _etfirst;
    _etplace = _etcur = _etfirst;
}

static short _me_retrieve( MEVENT *me )
{
    long timenow = _tics;

    if( demo_play )
    {
        //if( dqp->event == drTextBox )
        //    dShowMsgBox();

        if( demo_play )
        {
            if( dqp->event != drKeyboard && (timenow-last_dtime) >= dqp->tdelay )
            {
                mousehandler( timenow, dqp->e.m.mx, dqp->e.m.my, dqp->event );
                drKeyStat = dqp->e.m.ks;
                dqp++;
                last_dtime = timenow;

                /* is this the last one? */
                if( --ndRecord == 0 )
                    demo_play = 0;
            }
            else
            {
                me_nothing_d++;
                return 0;
            }
        }
    }

    if( _etcur == _etplace )
    {
        fNoHandler = 1;
        if( _mou_moved )
        {
            _mou_moved = 0;
            me->x = scursorx;
            me->y = scursory;
            me->what = _MOVE;
            fNoHandler = 0;
            return 1;
        }
        fNoHandler = 0;
#ifdef DEBUG
        me_nothing_e++;
#endif
        return 0;
    }

    me->x = _etcur->x;
    me->y = _etcur->y;
    me->what = _etcur->what;
    me->when = _etcur->when;
    _etcur = _etcur->next;

    return 1;
}

#define goret(r)        {rvl = r; goto _goret;}

static short me_lastx, me_lasty;

void mouse_lastcoords( short *x, short *y )
/*-------------------------------------------------------------------------*
 * Returns the coordinates of the last mouse_event() (not including
 * mouse_move events.)
 *-------------------------------------------------------------------------*/
{
    *x = me_lastx;
    *y = me_lasty;
}

unsigned mouse_status( void )
{
    unsigned val = 0;

    if( mou_ldrag ) val |= 0x01;
    if( mou_rdrag ) val |= 0x02;
    if( mou_mdrag ) val |= 0x04;

    return val;
}

short mouse_event( short *x, short *y )
/*-------------------------------------------------------------------------*
 * Returns a "real" mouse event.                                           *
 *-------------------------------------------------------------------------*/
{
    MEVENT  nevent;
    long    timenow = _tics;
    short   rvl;

    if( mou_ldrag < 0 )
    {
        *x = me_lastx;
        *y = me_lasty;
        mou_ldrag = 0;
        goret( MOUSE_LDRAGEXIT )
    }
    if( mou_ldown < 0 )
    {
        *x = ldownx;
        *y = ldowny;
        mou_ldown = 0;
        goret( MOUSE_LDBLCLICK )
    }

    if( _me_retrieve( &nevent ) )
    {
        *x = nevent.x;
        *y = nevent.y;

        switch( nevent.what )
        {
        case _LPRESS:
            /* If the button went down:
             * If ldownwhen was less than the double click time margin
             * ago, increment mou_ldown to 2 because it's gonna be 1. */
            if( !mou_ldown )
            {
                ldownx = nevent.x;
                ldowny = nevent.y;
                ldownwhen = nevent.when;
            }
            ++mou_ldown;
            goret( MOUSE_LDOWN )
            break;
        case _LRELEASE:
            /* If the button went up:
             * If mou_ldown is 2, this is a double click.  Return it.
             */
            if( mou_ldrag > 0 )
            {
                mou_ldrag = -1;
                mou_ldown = 0;
                goret( MOUSE_LUP )
            }
            if( mou_ldown == 3 )
            {
                mou_ldown = -1;
                goret( MOUSE_LUP )
            }
            ++mou_ldown;
            goret( MOUSE_LUP )
            break;
        case _MPRESS:
            if( !mou_mdown )
            {
                mdownx = nevent.x;
                mdowny = nevent.y;
                mdownwhen = nevent.when;
            }
            ++mou_mdown;
            goret( MOUSE_MDOWN )
            break;
        case _MRELEASE:
            if( mou_mdrag )
            {
                mou_mdrag = 0;
                mou_mdown = 0;
                goret( MOUSE_MDRAGEXIT )
            }
            if( mou_mdown == 3 )
            {
                mou_mdown = 0;
                *x = mdownx;
                *y = mdowny;
                goret( MOUSE_MDBLCLICK )
            }
            ++mou_mdown;
            goret( MOUSE_MUP )
            break;
        case _RPRESS:
            if( !mou_rdown )
            {
                rdownx = nevent.x;
                rdowny = nevent.y;
                rdownwhen = nevent.when;
            }
            ++mou_rdown;
            goret( MOUSE_RDOWN )
            break;
        case _RRELEASE:
            if( mou_rdrag )
            {
                mou_rdrag = 0;
                mou_rdown = 0;
                goret( MOUSE_RDRAGEXIT )
            }
            if( mou_rdown == 3 )
            {
                *x = rdownx;
                *y = rdowny;
                mou_rdown = 0;
                goret( MOUSE_RDBLCLICK )
            }
            ++mou_rdown;
            goret( MOUSE_RUP )
            break;
        case _MOVE:
            /* If the mouse has moved:
             * If down is 1, return draginit.
             * If down is not 1, return mousemove.
             */
            if( mou_ldown && !mou_ldrag &&
                ((timenow-ldownwhen > dclick_tics) ||
                    (abs( ldownx - nevent.x ) > drag_tics ||
                     abs( ldowny - nevent.y ) > drag_tics)) )
            {
                *x = ldownx;
                *y = ldowny;
                if( mou_ldown == 1 || mou_ldown == 3 )
                {
                    mou_ldrag = 1;
                    if( mou_ldown == 1 )
                        goret( MOUSE_LDRAGINIT )
                    else
                        goret( MOUSE_LDBLDRAGINIT )
                }
                else
                {
                    mou_ldown = 0;
                }
            }
            if( mou_mdown && !mou_mdrag &&
                ((timenow-mdownwhen > dclick_tics) ||
                    (abs( mdownx - nevent.x ) > drag_tics ||
                     abs( mdowny - nevent.y ) > drag_tics)) )
            {
                *x = mdownx;
                *y = mdowny;
                if( mou_mdown == 1 || mou_mdown == 3 )
                {
                    mou_mdrag = 1;
                    if( mou_mdown == 1 )
                        goret( MOUSE_MDRAGINIT )
                    else
                        goret( MOUSE_MDBLDRAGINIT )
                }
                else
                {
                    mou_mdown = 0;
                }
            }
            if( mou_rdown && !mou_rdrag &&
                ((timenow-rdownwhen > dclick_tics) ||
                    (abs( rdownx - nevent.x ) > drag_tics ||
                     abs( rdowny - nevent.y ) > drag_tics)) )
            {
                *x = rdownx;
                *y = rdowny;
                if( mou_rdown == 1 )
                {
                    mou_rdrag = 1;
                    goret( MOUSE_RDRAGINIT )
                }
                else
                {
                    mou_rdown = 0;
                }
            }
            goret( MOUSE_MOVE )
        }
    }
    else
    {
        if( mou_ldown == 2 && timenow-ldownwhen > dclick_tics )
        {
            mou_ldown = 0;
        }
        if( mou_rdown == 2 && (timenow-rdownwhen > dclick_tics) )
        {
            mou_rdown = 0;
        }
        if( mou_mdown == 2 && (timenow-mdownwhen > dclick_tics) )
        {
            mou_mdown = 0;
        }
    }

    *x = scursorx;
    *y = scursory;
    mou_deltax = mou_deltay = 0;
    return MOUSE_NOTHING;

_goret:
    if( rvl != MOUSE_MOVE )
    {
        me_lastx = *x;
        me_lasty = *y;
    }

    return rvl;
}

void mouse_setcoords( short x, short y )
{
    if( !ismouse )
        return;

    mouse_hide();

    fNoHandler = 1;
    /* move it the difference between the desired position and the current */
    m_lastx = x;
    m_lasty = y;
    mousehandler( _tics, 0, 0, _MOVE );
    fNoHandler = 0;

    mouse_show();
}

void mouse_setlimit( short x1, short y1, short x2, short y2 )
{
    if( !ismouse )
        return;

    m_minx = x1;
    m_maxx = x2;
    m_miny = y1;
    m_maxy = y2;
}

void mouse_setpage( short page )
{
}

unsigned short keystat( void )
/*-------------------------------------------------------------------------*
 * Gets the current keyboard status
 *-------------------------------------------------------------------------*/
{
    if( demo_play )
        return drKeyStat;   /* set when demo events are processed */

_asm {
    mov     si, _bios_sel
    mov     ax, ds
    mov     ds, si
    xor     bx, bx
    mov     bl, ds:[17h]
    mov     ds, ax
}

    return  _BX;
}

static long uWaitKeys[16];
static int nWaitKeys = 0;
static int nGotKeys = 0;
static unsigned uLastKey;

void SaveShot( void );

unsigned keycheck( void )
/*-------------------------------------------------------------------------*
 * Checks for a key press
 *-------------------------------------------------------------------------*/
{
    return ((nWaitKeys != nGotKeys) || kbhit());
}

unsigned keylast( void )
/*-------------------------------------------------------------------------*
 * Returns the last key got.
 *-------------------------------------------------------------------------*/
{
    return uLastKey;
}

unsigned keyget( void )
/*-------------------------------------------------------------------------*
 * Gets a key and returns it.
 *-------------------------------------------------------------------------*/
{
    unsigned uRvl;

    if( nGotKeys != nWaitKeys )
    {
        uRvl = uWaitKeys[nGotKeys++];
        if( nGotKeys == 16 )
        {
            nGotKeys = 0;
        }
    }
    else
    {
GetAgain:
        uRvl = getch();
        if( !uRvl )
        {
            uRvl = ((unsigned) getch()) << 8;
        }
        if( uRvl == 21760 ) // shift-f2
        {
            SaveShot();
            goto GetAgain;
        }
        if( uRvl == 21504 ) // shift-f1
        {
            _PopHelp.SetActive( 2 );
        }
    }

    return uLastKey = uRvl;
}

void keystuff( unsigned ch )
/*-------------------------------------------------------------------------*
 * Stuffs a key into the pending buffer
 *-------------------------------------------------------------------------*/
{
    uWaitKeys[nWaitKeys++] = ch;
    if( nWaitKeys == 16 )
        nWaitKeys = 0;
}

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#include "gdef.h"

extern Point ptView;

void SetViewPort( int left, int top, int right, int bottom, bool fClip )
{
    ptView.x = left;
    ptView.y = top;
    setviewport( left, top, right, bottom, fClip );
}

bool mouse_hide( Rect& rcRect )
{
    int scx = scursorx - ptView.x, scy = scursory - ptView.y;
    if( !clevel && rcRect.Intersect( Rect( scx, scy, scx+iCursorHeight,
                    scy+iCursorHeight ) ) )
    {
        mouse_hide();
        return TRUE;
    }
    return FALSE;
}

void mouse_show( bool fShow )
// complement to the above function
{
    if( fShow )
        mouse_show();
}

void mouse_coords( Point& ptPoint )
{
    extern int scursorx, scursory;
    ptPoint = Point( scursorx, scursory );
}

