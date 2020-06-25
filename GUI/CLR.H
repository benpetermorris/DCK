/*-------------------------------------------------------------------------*
 * COLORS!
 *-------------------------------------------------------------------------*/

#ifndef _CLR_H
#define _CLR_H

#ifndef _MAINMODULE
extern short guito256[][4];
#endif

#define C_BLACK         guito256[0][3]
#define C_BLUE          guito256[1][3]
#define C_GREEN         guito256[2][3]
#define C_CYAN          guito256[3][3]
#define C_RED           guito256[4][3]
#define C_MAGENTA       guito256[5][3]
#define C_BROWN         guito256[6][3]
#define C_GRAY1         guito256[7][3]
#define C_GRAY3         guito256[8][3]
#define C_LIGHTBLUE     guito256[9][3]
#define C_LIGHTGREEN    guito256[10][3]
#define C_LIGHTCYAN     guito256[11][3]
#define C_LIGHTRED      guito256[12][3]
#define C_LIGHTMAGENTA  guito256[13][3]
#define C_YELLOW        guito256[14][3]
#define C_WHITE         guito256[15][3]
#define C_GRAY2         guito256[16][3]
#define C_GRAY4         guito256[17][3]
#define C_GRAY5         guito256[18][3]
#define C_ILINE         guito256[19][3]
#define C_PLINE         guito256[20][3]
#define C_GRID          guito256[21][3]
#define C_WHITE2        guito256[22][3]

#define BLACK           0
#define BLUE            1
#define GREEN           2
#define CYAN            3
#define RED             4
#define MAGENTA         5
#define BROWN           6
#define GRAY1           7
#define GRAY3           8
#define LIGHTBLUE       9
#define LIGHTGREEN      10
#define LIGHTCYAN       11
#define LIGHTRED        12
#define LIGHTMAGENTA    13
#define YELLOW          14
#define WHITE           15
#define GRAY2           16
#define GRAY4           17
#define GRAY5           18
#define ILINE           19
#define PLINE           20
#define GRID            21
#define WHITE2          22
#define USER            23

// for setguicolor and getguicolor
enum
{
    gcMenuBkgrnd,
    gcMenuText,
    gcMenuItemText,
    gcMenuItemBarHigh,
    gcMenuItemTextHigh,
    gcMenuItemSelch,

    gcDlgBkgrnd,
    gcDlgTitleBarActive,
    gcDlgTitleBarInactive,
    gcDlgTitleText,
    gcDlgText,

    gc3DUp,
    gc3DDown,

    gcFldCaret,
    gcFldFocus,
    gcFldBkgrnd,
    gcFldText,

    gcLboxBarHigh,
    gcLboxTextHigh,
    gcLboxFocus,
    gcLboxBackground,

    gcBlack,
    gcBlue,
    gcGreen,
    gcCyan,
    gcRed,
    gcMagenta,
    gcBrown,
    gcLightGrey,
    gcDarkGrey,
    gcLightBlue,
    gcLightGreen,
    gcLightCyan,
    gcLightRed,
    gcLightMagenta,
    gcYellow,
    gcWhite,

    gcUser
};

int GetGuiColor( int iIndex );
void SetGuiColor( int iIndex, int iColor );

#endif // _CLR_H
