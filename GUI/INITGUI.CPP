#include <std.h>
#include <graphics.h>
#include "gdef.h"
#include "coords.hpp"
#include "inpdev.h"
#include <limits.h>
#include "clr.h"
#include "pophelp.hpp"

/*-------------------------------------------------------------------------*
 * InitGUI:
 * Runs basic initializations for the user interface.
 * Assumes that the graphics are initialized, and that the font has
 * been loaded.
 *-------------------------------------------------------------------------*/

int iScreenWidth, iScreenHeight;

PopHelp _PopHelp;

void InitGUI( void )
{
    iScreenWidth = getmaxx();
    iScreenHeight = getmaxy();

    SetUnits();
}

static int GuiColors[] =
{
    GRAY3,
    WHITE,
    WHITE,
    BLUE,
    WHITE,
    RED,

    GRAY3,
    BLUE,
    GRAY2,
    WHITE,
    WHITE,

    GRAY2,
    GRAY4,

    WHITE,
    BLACK,
    GRAY5,
    WHITE,

    BLUE,
    WHITE,
    RED,
    GRAY5,

    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    GRAY1,
    GRAY3,
    LIGHTBLUE,
    LIGHTGREEN,
    LIGHTCYAN,
    LIGHTRED,
    LIGHTMAGENTA,
    YELLOW,
    WHITE
};

int GetGuiColor( int iIndex )
{
    return guito256[GuiColors[iIndex]][3];
}

void SetGuiColor( int iIndex, int iColor )
{
    GuiColors[iIndex] = iColor;
}

