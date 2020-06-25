/*-------------------------------------------------------------------------*
 * WINDOW.H:                                                               *
 * Functions for windows =)                                                *
 *-------------------------------------------------------------------------*/

typedef struct tagGWINDOW
{
    short   x1, y1, x2, y2;     /* Coordinates */
    short   tmarg, bmarg;       /* margins.. */
    short   lmarg, rmarg;       /* margins.. */
    short   flags;              /* Window flags */
    uchar   *savebuf;           /* Save buffer, of course */
    unsigned sblen;             /* Save buffer length */
    short   color;              /* Window "face" color */
    short   tcolor;             /* Window title color */
    short   txtcolor;           /* Text color */
    short   bcolor;             /* Border color for non-3d windows */
    uchar   *title;             /* Here's a title for ya */
    short   xcenter, ycenter;   /* x/y centers, d00d */
    short   xlen, ylen;         /* x/y lenghts in fonts */

} GWINDOW;

/* Defines                                                                 */
#define WF_POPUP        0x01
#define WF_ONSCREEN     0x02
#define WF_3D           0x04
#define WF_NOTEXTBG     0x08

#define WNB_3D          0x01
#define WNB_FILL        0x02
#define WNB_EMPTY       0x04

#define wn_isup(p)      ((p)->flags & WF_ONSCREEN)
#define MAKESTATUSWINDOW(t,n) wn_def( Centered, Centered, 40, 5+n, C_GRAY3, C_GRAY1, WF_POPUP | WF_3D, t )

/* Prototypes                                                              */
GWINDOW *wn_def( short x, short y, short xlen, short ylen, uchar color,
                 uchar bcolor, short style, uchar *title );
void wn_disp( GWINDOW *wnp );
void wn_active( GWINDOW *wnp );
void wn_clear( GWINDOW *wnp );
void wn_undisp( GWINDOW *wnp );
void wn_xytext( GWINDOW *wnp, short x, short y, char *fmt, ... );
void wn_xyctext( GWINDOW *wnp, short x, short y, uchar color, char *fmt, ... );
void wn_box( GWINDOW *wnp, short x1, short y1, short xw, short yw,
             short style, uchar color );
void wn_line( GWINDOW *wnp, short x1, short y1, short x2, short y2,
              uchar color );
void wn_free( GWINDOW *wnp );
void wn_stat( GWINDOW *wnp, short n, short nmax );
void wn_statstr( GWINDOW *wnp, char *text );
void _textrect( short x, short y, char *buf, short color );

#define sw_opts(wnp, opts, stat)        ((wnp)->flags = (stat == ON) ? ((wnp)->flags | (opts)) : ((wnp)->flags & ~(opts)))

