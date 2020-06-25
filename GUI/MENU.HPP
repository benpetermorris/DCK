/*-------------------------------------------------------------------------*
 * Menu.hpp:
 * Menu Class definition/implementation header file.
 *-------------------------------------------------------------------------*/

#ifndef _MENU_HPP
#define _MENU_HPP

#include <std.h>
#include <graphics.h>
#include "gdef.h"
#include "gui.hpp"
#include "shadow.h"
#include "msgbox.h"
#include "keys.h"

class StatusBar;
class Menu;

#ifndef _PTR_DEFINED
typedef void* PTR;
#define _PTR_DEFINED
#endif

#ifndef MAINMODULE
extern int     iScreenWidth, iScreenHeight;
#endif

typedef int (*MFUNC)(long);
typedef void (*MSTATFP)(const char*);

enum
{
    mtHorizontal,
    mtVertical
};

enum
{
    _maStart = 0x100,

    maExit,
    maExitLevel,
    maContinue,
    maNextItem,
    maPrevItem,
    maPrevHoriz,
    maNextHoriz
};

enum
{
    mitItem,
    mitSepbar,
    mitText
};

// what to do with data when enter pressed:
enum
{
    mrMenu,
    mrFunction
};

typedef struct
{
    char szItemText[33];// item text
    char szKeyText[13]; // key string text
    int iSelPos;        // position of selection character
    int iSelPosX;       // x position in string of selection char
    MFUNC fp;           // function to run when it is selected
                        //   (iAction=mrFunction)
    Menu* mp;           // menu to run when selected
                        //   (iAction=mrMenu)
    long lParam;        // parameter to pass to "fp"
    const char* pszMessage;   // message for status bar
    int iType;          // type of item
    int iAction;        // what to do when item selected
    Rect rcItem;        // rect, from top-left of menu item area

} MenuItem;

class Menu
{
public:
    Menu( int iMaxItem, Point& ptPoint, int iType, MSTATFP fpStatus = NULL );
    ~Menu();

    void AddItem( const char* pszText, MFUNC fp, long lParam = 0,
                  const char* pszMessage = NULL, int iSelPos = -1 );
    void AddItem( const char* pszText, Menu* mp,
                  const char* pszMessage = NULL, int iSelPos = -1 );
    void AddSepbar();
    void AddText( const char* pszText );
    void Clear();
    void Draw( bool fDrawActive = TRUE );
    void SetRect();
    void DrawItem( int, bool fDrawActive = TRUE );
    void Erase();
    inline void SetPos( Point& ptPoint )
        { ptMenu = ptPoint; }
    int HitTest( Point& ptPoint );
    int Run( int iActivateItem = -1 );
    //int Run( Point& ptPoint );
    inline Rect GetRect()
        { return Rect( rcRect ); }

    bool IsSelected( int iItem )
        { return iCurItem == iItem; }

private:
    MSTATFP fpStatus;
    MenuItem& _AddItem( const char* pszText, const char* pszMessage,
                     int iSelPos );
    void GoToItem( int iItem );
    bool GoToItemWithKey( unsigned uKey );
    int RunItem( int iItem );

    MenuItem* ItemList; // list of items
    Rect rcRect;        // rectangle on screen
    Point ptMenu;       // left, top of the menu
    Size szMenu;        // size of menu in pixels, not including margins
    int iCurItem;       // current item
    int iGoItem;        // go to this item when next active
    int iType;          // type of menu
    int iMaxItem;       // max items
    int nItem;          // number of items
    StatusBar* pStatus; // status bar for messages
    char* pImageSave;   // underlying image
    Menu* pPrevMenu;    // previous running menu
    int iWidthSoFar;    // horizontal menus: width of items so far
};

#endif // _MENU_HPP
