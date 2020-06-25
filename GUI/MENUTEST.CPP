#define MAINMODULE

#include <std.h>
#include <graphics.h>
#include "menu.hpp"
#include <limits.h>

extern int _FontWidth, _FontHeight;

#if 1
short dckto256[][4] =
{
    { 0, 0, 0, 0 },          //BLACK
    { 0, 0, 170, 1 },        //BLUE
    { 0, 170, 0, 2 },        //GREEN
    { 0, 170, 170, 3 },      //CYAN
    { 170, 0, 0, 4 },        //RED
    { 170, 0, 170, 5 },      //MAGENTA
    { 170, 85, 0, 6 },       //BROWN
    { 220, 220, 220, 7 },    //GRAY1
    { 160, 160, 160, 8 },    //GRAY3
    { 85, 85, 255, 9 },      //LIGHTBLUE
    { 85, 255, 85, 10 },     //LIGHTGREEN
    { 85, 255, 255, 11 },    //LIGHTCYAN
    { 255, 85, 85, 12 },     //LIGHTRED
    { 255, 85, 255, 13 },    //LIGHTMAGENTA
    { 255, 255, 85, 14 },    //YELLOW
    { 255, 255, 255, 15 },   //WHITE
    { 210, 210, 210, 94 },   //GRAY2
    { 115, 115, 115, 106 },  //GRAY4
    { 35, 35, 35, 6 },       //GRAY5
    { 171, 171, 255, 194 },  //ILINE
    { 0, 0, 255, 200 },      //PLINE
    { 30, 30, 100, 240 },    //GRID
    { 255, 255, 255, 168 },  //WHITE2
    { -1, -1, -1, -1 }       //DUH2
};
#else
short dckto256[][4] =
{
    { 0, 0, 0, 0 },          //BLACK
    { 0, 0, 170, 1 },        //BLUE
    { 0, 170, 0, 2 },        //GREEN
    { 0, 170, 170, 3 },      //CYAN
    { 170, 0, 0, 4 },        //RED
    { 170, 0, 170, 5 },      //MAGENTA
    { 170, 85, 0, 6 },       //BROWN
    { 190, 190, 190, 7 },    //GRAY1
    { 100, 100, 100, 8 },    //GRAY3
    { 85, 85, 255, 9 },      //LIGHTBLUE
    { 85, 255, 85, 10 },     //LIGHTGREEN
    { 85, 255, 255, 11 },    //LIGHTCYAN
    { 255, 85, 85, 12 },     //LIGHTRED
    { 255, 85, 255, 13 },    //LIGHTMAGENTA
    { 255, 255, 85, 14 },    //YELLOW
    { 255, 255, 255, 15 },   //WHITE
    { 147, 147, 147, 94 },   //GRAY2
    { 60, 60, 60, 106 },     //GRAY4
    { 35, 35, 35, 6 },       //GRAY5
    { 171, 171, 255, 194 },  //ILINE
    { 0, 0, 255, 200 },      //PLINE
    { 30, 30, 100, 240 },    //GRID
    { 255, 255, 255, 168 },  //WHITE2
    { -1, -1, -1, -1 }       //DUH2
};
#endif

uchar pal256[256][3];

uchar findrgb( short r, short g, short b )
{
    long    curidx = -1, curdist = LONG_MAX, dist;
    short   i, rd, gd, bd;

    for( i = 0; i < 256; i++ )
    {
        rd = r-(long)pal256[i][0];
        gd = g-(long)pal256[i][1];
        bd = b-(long)pal256[i][2];

        dist = ((rd*rd)+(gd*gd)+(bd*bd));

        if( dist < curdist )
        {
            curdist = dist;
            curidx = i;
        }
    }

    return curidx;
}

void convertcolortable( short table[][4] )
{
    short   c;
    long    r, g, b;

    for( c = 0; table[c][0] != -1; c++ )
    {
        r = table[c][0];
        g = table[c][1];
        b = table[c][2];

        table[c][3] = findrgb( r, g, b );
    }
}

void loadpalette( char *fn )
{
    int     i;
    int     r, g, b;
    FILE    *fp;

    fp = fopen( fn, "rb" );
    fread( pal256, 768, 1, fp );

    for( i = 0; i < 256; i++ )
    {
        r = (int) pal256[i][0];
        g = (int) pal256[i][1];
        b = (int) pal256[i][2];
        setrgbpalette( i, r >> 2, g >> 2, b >> 2 );
    }

    convertcolortable( dckto256 );

    fclose( fp );
}

int iScreenWidth, iScreenHeight;

void CommitStack()
// NOTE: You must compile with the -N switch to enable stack overflow
// checking before this function can work (Borland uses the stack
// check prologue to commit locals that exceed 4K)
{
   long stackgrow[8192];  // 32K bytes - commit a big chunk of stack space

   stackgrow[8191] = 0;   // to force the stack to grow and become committed
                          // we must touch a location near the end.
}

void main( int argc, char *argv[] )
{
    int gdriver = ATI256, gmode = RES1024x768;

    if( argc == 1 )
        return;

    CommitStack();

    initgraph( &gdriver, &gmode, NULL );
    int font = installuserfont( argv[1] );
    settextstyle( font, HORIZ_DIR, 1 );

    iScreenWidth = getmaxx();
    iScreenHeight = getmaxy();
    mouse_init();
    mouse_show();
    atexit( mouse_kill );

    loadpalette( "DEFAULT.PAL" );

    SetUnits();

    Menu MySubSubMenu( 10, Point( 0, 0 ), mtVertical, NULL );
    MySubSubMenu.AddItem( "Menu item &1\tAlt+1", (MFUNC) NULL );
    MySubSubMenu.AddItem( "Menu item &2\tAlt+2", (MFUNC) NULL );
    MySubSubMenu.AddSepbar();
    MySubSubMenu.AddItem( "Menu item &3\tAlt+3", (MFUNC) NULL );

    Menu MySubMenu( 10, Point( 0, 0 ), mtVertical, NULL );
    MySubMenu.AddItem( "Menu item &1\tAlt+1", (MFUNC) NULL );
    MySubMenu.AddItem( "&&Menu 2\tAlt+2", (MFUNC) NULL );
    MySubMenu.AddItem( "Menu &3\tAlt+3", (MFUNC) NULL );
    MySubMenu.AddItem( "Menu itemZz &4\tAlt+4", &MySubSubMenu );

    Menu MyMainMenu( 10, Point( 0, 0 ), mtHorizontal, NULL );
    MyMainMenu.AddItem( "Sub &Menu", &MySubMenu );
    MyMainMenu.AddItem( "Nothing!", (MFUNC) NULL );

    MyMainMenu.Run();

    restorecrtmode();
    closegraph();
}
