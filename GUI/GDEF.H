/*-------------------------------------------------------------------------*
 * GDEF.H:
 * I hate Borland.
 *-------------------------------------------------------------------------*/

extern char _md_flag;

#define clmou   _md_flag--
#define stmou   _md_flag++

#define arc(a,b,c,d,e)          clmou, arc(a,b,c,d,e), stmou
#define bar(a,b,c,d)            clmou, bar(a,b,c,d), stmou
#define circle(a,b,c)           clmou, circle(a,b,c), stmou
#define clearviewport()         clmou, clearviewport(), stmou
#define fillpoly(a,b)           clmou, fillpoly(a,b), stmou
#define getimage(a,b,c,d,e)     clmou, getimage(a,b,c,d,e), stmou
#define putimage(a,b,c,d)       clmou, putimage(a,b,c,d), stmou
#define getviewsettings(a)      clmou, getviewsettings(a), stmou
#define setviewport(a,b,c,d,e)  clmou, setviewport(a,b,c,d,e), stmou
#define line(a,b,c,d)           clmou, line(a,b,c,d), stmou
#define lineto(a,b)             clmou, lineto(a,b), stmou
#define moveto(a,b)             clmou, moveto(a,b), stmou
#define outtextxy(a,b,c)        clmou, outtextxy(a,b,c), stmou
#define putpixel(a,b,c)         clmou, putpixel(a,b,c), stmou
#define rectangle(a,b,c,d)      clmou, rectangle(a,b,c,d), stmou
#define fillpoly(a,b)           clmou, fillpoly(a,b), stmou
#define fillellipse(a,b,c,d)    clmou, fillellipse(a,b,c,d), stmou
#define setfillstyle(a,b)       clmou, setfillstyle(a,b), stmou
#define setlinestyle(a,b,c)     clmou, setlinestyle(a,b,c), stmou
#define setwritemode(a)         clmou, setwritemode(a), stmou
#define setcolor(a)             clmou, setcolor(a), stmou
