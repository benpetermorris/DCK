#include "menu.hpp"

// current menu that is running -
static Menu* pCurrentMenu = NULL;
// menu to jump to -
static Menu* pJumpMenu = NULL;

static int iMenuLevel = 0;

const int iMenuXMargin = 3;
const int iMenuYMargin = 3;
const int iMenuPaddingUnits = 4;
const int iUnitsPerBar = 10;

Menu::Menu( int iMaxItem, Point& ptPoint, int iType, MSTATFP fpStatus ) :
/*-------------------------------------------------------------------------*
 * Initializes a menu class.
 *-------------------------------------------------------------------------*/
    nItem(0),
    pImageSave(NULL),
    iGoItem(-1),
    iWidthSoFar(0),
    iCurItem(0)
{
    if( iType == mtVertical )
        szMenu = Size( 0, 0 );
    else
        ; // szmenu isn't used for horizontal menus

    ptMenu = ptPoint;

    // create item list
    ItemList = new MenuItem[iMaxItem];

    this->iMaxItem = iMaxItem;
    this->iType = iType;
    this->fpStatus = fpStatus;
}

Menu::~Menu()
{
    // delete menu
    delete ItemList;
    if( pImageSave )
        free( pImageSave );
}

void Menu::Clear()
/*-------------------------------------------------------------------------*
 * Clears the contents of the menu.
 *-------------------------------------------------------------------------*/
{
    nItem = 0;
    szMenu = Size( 0, 0 );
    iWidthSoFar = 0;
    iCurItem = 0;
}

void Menu::AddSepbar()
/*-------------------------------------------------------------------------*
 * adds a separation bar to the menu
 *-------------------------------------------------------------------------*/
{
    // no sep bars on horizontals
    if( iType == mtHorizontal )
        return;

    MenuItem& Item = ItemList[nItem++];

    Item.iType = mitSepbar;
    Point ptItem = Point( iMenuXMargin, iMenuYMargin + szMenu.height );
    Item.rcItem.SetRect( ptItem,
        Size( rcRect.Width() - iMenuXMargin * 2, _YUnitsToPixels( iUnitsPerBar/2 ) ) );
    Item.iSelPos = Item.iSelPosX = -1;

    szMenu.height += _YUnitsToPixels( iUnitsPerBar/2 );
}

MenuItem& Menu::_AddItem( const char* pszText, const char* pszMessage,
                     int iSelPos )
/*-------------------------------------------------------------------------*
 * Does the actual "adding of item" to the menu.
 *-------------------------------------------------------------------------*/
{
    MenuItem& Item = ItemList[nItem++];

    Item.iType = mitItem;
    Item.pszMessage = pszMessage;

    // set menu item variables for text/key strings
    char *pszTab;
    if( (pszTab = (char*) strchr( pszText, '\t' )) )
    {
        strncpy( Item.szItemText, pszText, (pszTab - pszText) );
        Item.szItemText[pszTab - pszText] = 0;
        strcpy( Item.szKeyText, pszTab+1 );
    }
    else
    {
        strcpy( Item.szItemText, pszText );
        Item.szKeyText[0] = 0;
    }

    // find out selection character position
    if( iSelPos != -1 )
        Item.iSelPos = iSelPos;
    else
    {
        char* pszAmpersand = strchr( Item.szItemText, '&' );
        if( pszAmpersand && pszAmpersand[1] != '&' )
        {
            // found an ampersand - get sel pos, and delete it from
            // the text
            Item.iSelPos = (int) (pszAmpersand - (char*) Item.szItemText);
            strcpy( pszAmpersand, pszAmpersand+1 );
        }
        else
            Item.iSelPos = -1;
    }

    if( Item.iSelPos != -1 )
    {
        char szTemp[128];
        strcpy( szTemp, Item.szItemText );
        szTemp[Item.iSelPos] = 0;
        Item.iSelPosX = textwidth( szTemp );
    }

    int iWidth = textwidth( Item.szItemText ) + textwidth( Item.szKeyText );
    Point ptItem;

    // calculate width of item
    if( iType == mtVertical )
    {
        // * 3 - left, right, and in between key and item text
        iWidth += _XUnitsToPixels( iMenuPaddingUnits ) * 4;

        // see if we need to expand the bawx
        if( szMenu.width < iWidth )
            szMenu.width = iWidth;

        // make the box deeper
        ptItem = Point( iMenuXMargin, iMenuYMargin + szMenu.height );
        szMenu.height += _YUnitsToPixels( iUnitsPerBar );
    }
    // horizontal menu
    else
    {
        // * 2 - left and right
        iWidth += _XUnitsToPixels( iMenuPaddingUnits ) * 2;
        ptItem = Point( iWidthSoFar + iMenuXMargin, iMenuYMargin );
    }

    Item.rcItem = Rect( ptItem, Size( iWidth, _YUnitsToPixels( iUnitsPerBar ) ) );

    // increment so far width
    iWidthSoFar += iWidth;

    return Item;
}

void Menu::AddItem( const char* pszText, MFUNC fp, long lParam,
                    const char* pszMessage, int iSelPos )
/*-------------------------------------------------------------------------*
 * Adds an item that calls a function.
 *-------------------------------------------------------------------------*/
{
    MenuItem& Item = _AddItem( pszText, pszMessage, iSelPos );
    Item.fp = fp;
    Item.lParam = lParam;
    Item.iAction = mrFunction;
}

void Menu::AddItem( const char* pszText, Menu* mp, const char* pszMessage,
                int iSelPos )
/*-------------------------------------------------------------------------*
 * Adds an item that calls a submenu.
 *-------------------------------------------------------------------------*/
{
    MenuItem& Item = _AddItem( pszText, pszMessage, iSelPos );
    Item.mp = mp;
    Item.iAction = mrMenu;
}

void Menu::AddText( const char* pszText )
/*-------------------------------------------------------------------------*
 * Adds a text item to the menu.
 *-------------------------------------------------------------------------*/
{
    MenuItem& Item = _AddItem( pszText, NULL, -1 );

    strcpy( Item.szItemText, pszText );
    Item.szKeyText[0] = 0;
    Item.iSelPos = Item.iSelPosX = -1;
    Item.pszMessage = NULL;
    Item.iType = mitText;
}

void Menu::SetRect()
/*-------------------------------------------------------------------------*
 * Sets the screen rectangle for the menu
 *-------------------------------------------------------------------------*/
{
    Size szPixels;
    if( iType == mtVertical )
        szPixels = Size( iMenuXMargin*2, iMenuYMargin*2 );
    else
        szPixels = Size( iScreenWidth, iMenuYMargin*2 +
                         _YUnitsToPixels( iUnitsPerBar ) );
    rcRect.SetRect( ptMenu, szPixels + szMenu );
}

void Menu::DrawItem( int iItem, bool fShowActive )
/*-------------------------------------------------------------------------*
 * Draws an item
 *-------------------------------------------------------------------------*/
{
    if( !inrange( iItem, 0, nItem-1 ) )
        return;

    MenuItem& Item = ItemList[iItem];
    int iMenuXMarginPixels;

    if( iType == mtVertical )
    {
        // make the rectangle the width of the menu, minus margins
        Item.rcItem.right = Item.rcItem.left + rcRect.Width() -
                            iMenuXMargin * 2;
    }

    // create temp rect for drawing
    Rect rcItem( Item.rcItem );
    // offset our temp rect from the top left of the menu
    rcItem.MoveRel( ptMenu );

    // create point for text
    Point ptText = rcItem.TopLeft();
    ptText.x += (iMenuXMarginPixels = _XUnitsToPixels( iMenuPaddingUnits ));

    mouse_hide();

    // draw background rectangle
    if( IsSelected( iItem ) && fShowActive )
    {
        if( iType == mtHorizontal )
        {
            setfillstyle( SOLID_FILL, GetGuiColor( gcMenuItemBarHigh ) );
            Bar( rcItem );
        }
        else
            ShadowRect( rcItem, srThin | srIndent, GetGuiColor( gcMenuItemBarHigh ) );
        setcolor( GetGuiColor( gcMenuItemTextHigh ) );
    }
    else
    {
        setfillstyle( SOLID_FILL, GetGuiColor( gcMenuBkgrnd ) );
        Bar( rcItem );
        setcolor( GetGuiColor( gcMenuItemText ) );
    }

    int iPadding = _YUnitsToPixels( (iUnitsPerBar-YUPC)/2 );

    // draw item
    switch( Item.iType )
    {
    case mitText:
        setcolor( GetGuiColor( gcMenuText ) );
        outtextxy( ptText.x, ptText.y+iPadding, Item.szItemText );
        break;
    case mitItem:
        outtextxy( ptText.x, ptText.y+iPadding, Item.szItemText );
        outtextxy( rcItem.right - iMenuXMarginPixels -
                   textwidth( Item.szKeyText ), ptText.y+iPadding, Item.szKeyText );

        // selection char
        if( (!IsSelected( iItem ) || !fShowActive) && Item.iSelPos != -1 )
        {
            char szSel[] = "\0\0";
            setcolor( GetGuiColor( gcMenuItemSelch ) );
            szSel[0] = Item.szItemText[Item.iSelPos];
            outtextxy( ptText.x + Item.iSelPosX, ptText.y+iPadding, szSel );
        }
        break;
    case mitSepbar:
        {
        int y = rcItem.CenterY();
        setcolor( GetGuiColor( gc3DDown ) );
        line( rcItem.left, y, rcItem.right, y );
        setcolor( GetGuiColor( gc3DUp ) );
        line( rcItem.left, y+1, rcItem.right, y+1 );
        }
        break;
    }

    mouse_show();
}

void Menu::Draw( bool fShowActive )
/*-------------------------------------------------------------------------*
 * Draws the menu.
 *-------------------------------------------------------------------------*/
{
    struct viewporttype vp;

    getviewsettings( &vp );
    SetViewPort( 0, 0, iScreenWidth, iScreenHeight );

    // create rectangle
    SetRect();

    Rect rcInside = rcRect;

    bool fHidden = mouse_hide( rcRect );

    // get underlying image
    if( !pImageSave && iType == mtVertical )
        pImageSave = (char*) malloc( ImageSize( rcRect ) );
    if( pImageSave )
        GetImage( rcRect, pImageSave );

    // display rectangle
    //setcolor( C_BLACK );
    //Rectangle( rcInside );
    //rcInside.Inflate( -1, -1 );
    ShadowRect( rcInside, srThin, GetGuiColor( gcMenuBkgrnd ) );

    // display items
    for( int iItem = 0; iItem < nItem; iItem++ )
        DrawItem( iItem, fShowActive );

    mouse_show( fHidden );

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom );
}

void Menu::Erase()
/*-------------------------------------------------------------------------*
 * Erase the menu from the screen - restore the contents of the screen
 * under the menu
 *-------------------------------------------------------------------------*/
{
    SetViewPort( 0, 0, iScreenWidth, iScreenHeight );
    bool fHidden = mouse_hide( rcRect );
    if( pImageSave )
    {
        PutImage( rcRect, pImageSave );
        free( pImageSave );
    }
    pImageSave = NULL;
    mouse_show( fHidden );
}

int Menu::RunItem( int iItem )
/*-------------------------------------------------------------------------*
 * Performs whatever action "iItem" contains.
 *-------------------------------------------------------------------------*/
{
    MenuItem& Item = ItemList[iItem];

    switch( Item.iAction )
    {
    case mrFunction:
        if( Item.fp )
            return (*(Item.fp))(Item.lParam);
        break;
    case mrMenu:
        if( Item.mp )
        {
            // create submenu coordinates and go ..
            if( iType == mtHorizontal )
                Item.mp->ptMenu = ptMenu + Item.rcItem.TopLeft() +
                                Size( -iMenuXMargin, _YUnitsToPixels( iUnitsPerBar ) +
                                iMenuYMargin + 1 );
            else
                Item.mp->ptMenu = ptMenu + Item.rcItem.TopLeft() +
                                Size( Item.rcItem.Width(), -iMenuYMargin );
            return Item.mp->Run();
        }
        break;
    }

    // corrupted menu structure
    return maContinue;
}

void Menu::GoToItem( int iItem )
/*-------------------------------------------------------------------------*
 * Changes the current item to 'iItem'.
 *-------------------------------------------------------------------------*/
{
    int iOldItem = iCurItem;

    if( iItem > _maStart )
    {
        // action - next, prev item etc.
        switch( iItem )
        {
        case maNextItem:
            while( 1 )
            {
                iCurItem++;
                if( iCurItem == nItem )
                    iCurItem = 0;
                if( ItemList[iCurItem].iType == mitItem )
                    break;
            }
            break;
        case maPrevItem:
            while( 1 )
            {
                iCurItem--;
                if( iCurItem == -1 )
                    iCurItem = nItem-1;
                if( ItemList[iCurItem].iType == mitItem )
                    break;
            }
            break;
        case maPrevHoriz:
        case maNextHoriz:
            // first previous horizontal menu
            Menu* pMenu = this;

            while( pMenu )
            {
                if( pMenu->iType == mtHorizontal )
                {
                    // aha!
                    pJumpMenu = pMenu;
                    break;
                }

                pMenu = pMenu->pPrevMenu;
            }

            if( !pMenu )
                break;

            // next / prev item
            if( iItem == maPrevHoriz )
                pMenu->GoToItem( maPrevItem );
            else
                pMenu->GoToItem( maNextItem );

            pMenu->iGoItem = pMenu->iCurItem;

            break;
        }
    }
    else
    {
        if( ItemList[iItem].iType == mitItem )
            iCurItem = iItem;
    }

    if( iOldItem != iCurItem )
    {
        // the item has changed, so draw both..
        DrawItem( iOldItem );
        DrawItem( iCurItem );
        if( fpStatus )
            (*fpStatus)( ItemList[iCurItem].pszMessage );
    }
}

int Menu::HitTest( Point& ptScreen )
/*-------------------------------------------------------------------------*
 * As is implied, ptScreen is a screen point.  this function tests
 * to see if it's inside the menu, and returns the item number it's on
 * or -1 if it's not.
 *-------------------------------------------------------------------------*/
{
    Point ptInMenu = ptScreen;

    ptInMenu -= ptMenu;

    // search through items
    for( int iItem = 0; iItem < nItem; iItem++ )
    {
        if( ItemList[iItem].iType == mitItem &&
            ItemList[iItem].rcItem.IsPtInside( ptInMenu ) )
            return iItem;
    }

    return -1;
}

bool Menu::GoToItemWithKey( unsigned uKey )
/*-------------------------------------------------------------------------*
 * Searches the current menu for an item whose selection character matches
 * "uKey", and goes to it.
 *-------------------------------------------------------------------------*/
{
    int iSelPos;

    // search through items
    for( int iItem = 0; iItem < nItem; iItem++ )
    {
        if( (iSelPos = ItemList[iItem].iSelPos) != -1 )
        {
            if( toupper( ItemList[iItem].szItemText[iSelPos] ) ==
                toupper( uKey ) )
            {
                // found one
                GoToItem( iItem );
                return TRUE;
            }
        }
    }

    return FALSE;
}

int Menu::Run( int iActivateItem )
/*-------------------------------------------------------------------------*
 * Runs the menu.
 *
 * Returns the current item if the user hits ENTER, or -1 if the user
 * hits ESC.
 *-------------------------------------------------------------------------*/
{
    unsigned uKey;
    int iRvl = maContinue;
    int iMouseEvent;
    short suMouseX, suMouseY;
    struct viewporttype vp;

    SetCursorType( ctArrow );

    getviewsettings( &vp );
    SetViewPort( 0, 0, iScreenWidth, iScreenHeight );

    ++iMenuLevel;

    // set previous menu to current running menu:
    pPrevMenu = pCurrentMenu;
    // set current running menu to this:
    pCurrentMenu = this;

    if( iActivateItem != -1 )
        iGoItem = iCurItem = iActivateItem;
    else if( iMenuLevel == 1 && iType == mtHorizontal )
        iGoItem = iCurItem;

    Draw();

    if( fpStatus && iCurItem >= 0 )
        (*fpStatus)( ItemList[iCurItem].pszMessage );

    do {

        // menu to jump to?
        if( pJumpMenu )
        {
            if( pJumpMenu == this )
                pJumpMenu = NULL;   // stay here - it's where we want to be
            else
            {
                iRvl = maExitLevel;
                break;      // exit this menu - pJumpMenu is before it
            }
        }

        // item to jump to?
        if( iGoItem != -1 )
        {
            GoToItem( iGoItem );
            iGoItem = -1;
            if( ItemList[iCurItem].iAction == mrMenu )
                iRvl = RunItem( iCurItem );
        }

        if( iRvl == maExit || iRvl == maExitLevel )
            break;

        // key pressed?
        if( keycheck() )
        {
            uKey = keyget();

            switch( uKey )
            {
            case KEY_ENTER:
                iRvl = RunItem( iCurItem );
                break;
            case KEY_ESC:
                if( iMenuLevel > 2 )
                    iRvl = maExitLevel;
                else
                    iRvl = maExit;
                break;
            case KEY_LEFT:
                // if horizontal, previous item.  or:
                // find previous horizontal menu, go to it
                if( iType == mtVertical )
                {
                    // if there's a previous vertical menu,
                    // just go back to that
                    if( pPrevMenu && pPrevMenu->iType == mtVertical )
                        iRvl = maExitLevel;
                    else
                        GoToItem( maPrevHoriz );
                }
                else
                {
                    GoToItem( maPrevItem );
                    if( ItemList[iCurItem].iAction == mrMenu )
                        iRvl = RunItem( iCurItem );
                }
                break;
            case KEY_RIGHT:
                // if horizontal, next item.  or:
                // find next horizontal menu, go to it
                if( iType == mtVertical )
                    GoToItem( maNextHoriz );
                else
                {
                    GoToItem( maNextItem );
                    if( ItemList[iCurItem].iAction == mrMenu )
                        iRvl = RunItem( iCurItem );
                }
                break;
            case KEY_DOWN:
                // next item
                GoToItem( maNextItem );
                break;
            case KEY_UP:
                // prev item
                GoToItem( maPrevItem );
                break;
            case KEY_HOME:
                // first item
                GoToItem( 0 );
                break;
            case KEY_END:
                // last item
                GoToItem( nItem-1 );
                break;
            default:
                // search for key item ..
                if( inrange( uKey, 0, 127 ) )
                {
                    if( GoToItemWithKey( uKey ) )
                        keystuff( KEY_ENTER );
                }
                break;
            }
        }
        else if( iMouseEvent = mouse_event( &suMouseX, &suMouseY ) )
        {
            Point ptMouse( suMouseX, suMouseY );
            Menu* pMenu = this;

            // first, figure out where this event occured:
            while( pMenu )
            {
                int iItem;

                if( (iItem = pMenu->HitTest( ptMouse )) != -1 )
                {
                    // aha!
                    if( pMenu->iCurItem != iItem )
                    {
                        pMenu->iGoItem = iItem;
                        pJumpMenu = pMenu;
                    }
                    break;
                }

                pMenu = pMenu->pPrevMenu;
            }

            switch( iMouseEvent )
            {
            case MOUSE_LUP:
                int iItem;
                // left button up - run the current item
                if( (iItem = HitTest( ptMouse )) != -1 )
                    iRvl = RunItem( iItem );
                break;
            case MOUSE_LDOWN:
                // left button down - we don't care
                break;
            case MOUSE_RUP:
                // exit menu
                iRvl = maExit;
            }
        }

    } while( iRvl != maExit && iRvl != maExitLevel );

    // if it's not escaped, change the return value to the current
    // item number.
    if( iRvl == maExitLevel )
        iRvl = maContinue;
    else if( iRvl != maExit )
        iRvl = iCurItem;

    // set current menu to previously running menu:
    pCurrentMenu = pPrevMenu;

    Erase();

    --iMenuLevel;

    if( iMenuLevel == 0 )
        Draw(FALSE);

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom );

    return iRvl;
}
