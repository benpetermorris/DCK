#define _MAINMODULE

#include <std.h>
#include <graphics.h>
#include "gui.hpp"
#include "dialog.hpp"
#include <limits.h>

void SaveShot() {}

short guito256[][4] =
{
    { 0, 0, 0, 0 },          //BLACK
    { 0, 0, 100, 1 },        //BLUE
    { 0, 170, 0, 2 },        //GREEN
    { 0, 170, 170, 3 },      //CYAN
    { 170, 0, 0, 4 },        //RED
    { 170, 0, 170, 5 },      //MAGENTA
    { 170, 85, 0, 6 },       //BROWN
    { 190, 190, 190, 7 },    //GRAY1
    { 130, 130, 130, 8 },    //GRAY3
    { 85, 85, 255, 9 },      //LIGHTBLUE
    { 85, 255, 85, 10 },     //LIGHTGREEN
    { 85, 255, 255, 11 },    //LIGHTCYAN
    { 255, 85, 85, 12 },     //LIGHTRED
    { 255, 85, 255, 13 },    //LIGHTMAGENTA
    { 255, 255, 85, 14 },    //YELLOW
    { 255, 255, 255, 15 },   //WHITE
    { 160, 160, 160, 94 },   //GRAY2
    { 100, 100, 100, 106 },     //GRAY4
    { 50, 50, 50, 6 },       //GRAY5
    { 171, 171, 255, 194 },  //ILINE
    { 0, 0, 255, 200 },      //PLINE
    { 30, 30, 100, 240 },    //GRID
    { 255, 255, 255, 168 },  //WHITE2
    { -1, -1, -1, -1 }       //DUH2
};

void error(char *fmt, ...)
{
    vprintf(fmt, &fmt+1);
}

void CommitStack()
// NOTE: You must compile with the -N switch to enable stack overflow
// checking before this function can work (Borland uses the stack
// check prologue to commit locals that exceed 4K)
{
   long stackgrow[8192];  // 32K bytes - commit a big chunk of stack space

   stackgrow[8191] = 0;   // to force the stack to grow and become committed
                          // we must touch a location near the end.
}

int valstr( char* pszString, Field* pField )
{
    if( strlen( pszString ) > 5 )
    {
        pField->SetValMsg( "This field must be shorter than 6 characters." );
        return valNotOkay;
    }
    return valOkay;
}

int evtest( Field* pField )
{
    message_box( MB_OKCANCEL, "This is a title.", "Hi there." );
    message_box( MB_YESNOALWAYS, "Title!",
                    "Hi there.\nHiiii there.\nYeaaaaah!" );
    message_box( MB_YESNOALWAYS, "Hi.", "This!" );

    return daContinue;
}

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

    convertcolortable( guito256 );

    fclose( fp );
}




void main( int argc, char *argv[] )
{
    int gdriver = VESA256, gmode = RES640x480;
    char *fn = argv[1];

    if( argc == 1 )
        fn = "chicam15.fnt";

    CommitStack();

    initgraph( &gdriver, &gmode, NULL );
    int font = installuserfont( fn );
    settextstyle( font, HORIZ_DIR, 1 );

    InitGUI();
    _PopHelp.Init( "MYHELP.HLP" );
    _PopHelp.SetActive( TRUE );
    loadpalette( "DEFAULT.PAL" );
    mouse_init();
    mouse_show();
    atexit( mouse_kill );

        SetGuiColor( gcDlgText, BLACK );
        SetGuiColor( gcMenuText, BLACK );
        SetGuiColor( gcMenuItemText, BLACK );
        SetGuiColor( gcFldFocus, BLACK );

    unsigned char *pCheckPat = "\x55\xAA\x55\xAA\x55\xAA\x55\xAA";
    setfillpattern( pCheckPat, C_GRAY4 );
    //setbkcolor( C_GRAY4 );
    Bar( Rect( 0, 0, iScreenWidth, iScreenHeight ) );

/*
    Menu MySubSubMenu( 10, Point( 0, 0 ), mtVertical, NULL );
    MySubSubMenu.AddItem( "Menu item &1\tAlt+1", (MFUNC) NULL );
    MySubSubMenu.AddItem( "Menu item &2\tAlt+2", (MFUNC) NULL );
    MySubSubMenu.AddSepbar();
    MySubSubMenu.AddItem( "Menu item &3\tAlt+3", (MFUNC) NULL );

    Menu MySubMenu( 10, Point( 0, 0 ), mtVertical, NULL );
    MySubMenu.AddItem( "Menu item &1\tAlt+1", (MFUNC) NULL );
    MySubMenu.AddItem( "A &Dialog\tAlt+D", (MFUNC) mnTestDialog );
    MySubMenu.AddItem( "Menu &3\tAlt+3", (MFUNC) NULL );
    MySubMenu.AddItem( "Menu itemZz &4\tAlt+4", &MySubSubMenu );

    Menu MyMainMenu( 10, Point( 0, 0 ), mtHorizontal, NULL );
    MyMainMenu.AddItem( "Sub &Menu", &MySubMenu );
    MyMainMenu.AddItem( "Nothing!", (MFUNC) NULL );

    MyMainMenu.Run();
 */

    Dialog testdlg;
    testdlg.Create( "TEST.DS", "TestDlg" );
    testdlg.Process();

    restorecrtmode();
    closegraph();
}
