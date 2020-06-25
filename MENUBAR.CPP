/*-------------------------------------------------------------------------*
 * MENUBAR.C:                                                              *
 * manages the menu bar/creating/etc.                                      *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\menu.hpp"
#include "statbars.h"
#include "cmddefs.h"

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

Menu    *mnu_main,
        *mnu_file,
        *mnu_edit,
        *mnu_help,
        *mnu_o_line,
        *mnu_o_thing,
        *mnu_o_vertex,
        *mnu_o_sector,
        *mnu_misc;

Menu    *mnu_rc_line,
        *mnu_rc_thing,
        *mnu_rc_sect,
        *mnu_rc_vtx;

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

int EditScript();

void run_rc_menu( void )
{
    short   x = scursorx,
            y = scursory;
    Menu    *pMenu;

    /* depending on the edit mode, runs the right menu */
    switch( editmode )
    {
    case MODE_LINE:
        pMenu = mnu_rc_line;
        break;
    case MODE_THING:
        pMenu = mnu_rc_thing;
        break;
    case MODE_SECTOR:
        pMenu = mnu_rc_sect;
        break;
    case MODE_VERTEX:
        pMenu = mnu_rc_vtx;
        break;
    }

    pMenu->SetPos( Point( x, y ) );
    pMenu->Run();
}

void ShowMenuMsg( char* msg )
{
    struct viewporttype vp;
    getviewsettings( &vp );
    SetStatusMsg( msg );
    UpdateMessageBar();
    SetViewPort( vp.left, vp.top, vp.right, vp.bottom );
}

int EditTools();

int about()
/*-------------------------------------------------------------------------*
 * About dialog box.  Simple 'nuff.                                        *
 *-------------------------------------------------------------------------*/
{
    Dialog *pDialog = new Dialog;
    pDialog->Create( szDSFile, "AboutDlg" );
    pDialog->Process();
    delete pDialog;

    return maContinue; /* This is also called from menus */
}

int key_help()
/*-------------------------------------------------------------------------*
 * Key help: Figures out what the help should be about and displays it.    *
 *-------------------------------------------------------------------------*/
{
    /* Nothing going on: read normal text */
    if( help_disp( "KEYHELP", 20 ) )
        post_fullupdate();

    return maExit; /* Called from menus as well as other things */
}

int exit_confirm()
/*-------------------------------------------------------------------------*
 * Confirms exit: If check_save() returns 1, then exit.  Otherwise,        *
 * continue.                                                               *
 *-------------------------------------------------------------------------*/
{
    extern short current_op;

    if( check_save() )
    {
        current_op = OP_EXIT;
        return maExit;     /* Called from menus too */
    }
    return maContinue;
}

int _mn_newmap( long lGame )
/*-------------------------------------------------------------------------*
 * New map: if check_save() returns 1, then wipe the current map.          *
 * Otherwise, continue.                                                    *
 *-------------------------------------------------------------------------*/
{
    if( check_save() )
    {
        map_new( lGame );
        post_fullupdate();
        get_out = 2;
    }
    return maExit;
}

int _mn_texfuncs( void )
{
    void texfile_mgmt( void );
    texfile_mgmt();

    return maExit;
}

static short _mn_shell( short notused )
{
    //static char pbuf[128];

    //sprintf( pbuf, "PROMPT=[DCK] %s", getenv( "PROMPT" ) );
    //putenv( pbuf );

    gr_vidmode( MODE_TEXT );
    printf( "Type EXIT to return to DCK ..." );
    system( getenv( "COMSPEC" ) );

    fFullUpdate = 2;
    gr_vidmode( MODE_GFX );

    return maExit;
}

static int _mn_postch( long lKey )
{
    /* Just posts the character passed and exits the menu */
    keystuff( (unsigned) lKey );
    return maExit;
}

static void _mk_stdobj( Menu *pMenu )
/* Makes a standard object menu's first items and sticks a sepbar at the   *
 * end. */
{
    pMenu->AddItem( "&Properties...\tEnter", (MFUNC) _mn_postch,
        KEY_ENTER, "Edit marked and selected objects" );
    pMenu->AddItem( "&Rotate\tCtrl+O", (MFUNC) _mn_postch,
        KEY_CTRLO, "Rotate marked objects" );
    pMenu->AddItem( "&Scale\tCtrl+A", (MFUNC) _mn_postch,
        KEY_CTRLA, "Scale marked objects" );
    pMenu->AddItem( "Sna&p selected to grid\tCtrl+V", (MFUNC) _mn_postch,
        KEY_CTRLV, "Snap the marked objects to grid" );
    pMenu->AddItem( "Mirror horizontally", (MFUNC) _mn_postch,
        CMD_FLIPHORIZ, "Mirror (flip) the marked objects horizontally" );
    pMenu->AddItem( "Mirror vertically", (MFUNC) _mn_postch,
        CMD_FLIPVERT, "Mirror (flip) the marked objects vertically" );
    pMenu->AddSepbar();
    pMenu->AddItem( "&Go to object (#)\tAlt+G", (MFUNC) _mn_postch,
        KEY_ALTG, "Jumps to and zooms in on a specific object number" );
    pMenu->AddSepbar();
}

static void _mk_stdrc( Menu* pMenu, Menu* pSubMenu )
/* all right-click menus share some objects - delete, etc */
{
    pMenu->AddItem( "&Edit objects\tEnter", (MFUNC) _mn_postch,
                 KEY_ENTER, "Edit marked and selected objects" );
    if( pMenu != mnu_rc_vtx )
        pMenu->AddItem( "&Format painter\tCtrl+P", (MFUNC) _mn_postch,
            KEY_CTRLP, "Use the format painter to change the marked objects" );
    pMenu->AddSepbar();
    pMenu->AddItem( "&Line mode\tL", (MFUNC) _mn_postch, 'L', "Enter line edit mode" );
    pMenu->AddItem( "&Sector mode\tS", (MFUNC) _mn_postch, 'S', "Enter sector edit mode" );
    pMenu->AddItem( "&Item mode\tI", (MFUNC) _mn_postch, 'T', "Enter Item edit mode" );
    //pMenu->AddItem( "&Vertex mode\tV", (MFUNC) _mn_postch, 'V', "Enter vertex edit mode" );
    pMenu->AddSepbar();
    if( pSubMenu )
        pMenu->AddItem( "&Other\t\x1a", pSubMenu, "Other features" );
}

static short  _mn_paste( void )
/*-------------------------------------------------------------------------*
 * Paste from the menu
 *-------------------------------------------------------------------------*/
{
    extern short current_op;

    if( current_op != OP_NOTHING )
    {
        message_box( MB_OK, st_dcksays,
          "Finish what you're doing first!" );
        return MA_EXIT;
    }

    cb_paste( editmode, xstart + xscale / 2, ystart + yscale / 2 );

    return MA_EXIT;
}

static const char *helpitems[] =
{
    " Key Commands            ",
    " Designing tips          ",
    " Format Painter          ",
    " Using the Grid          ",
    " Rectangles and Polygons ",
    " Motifs                  ",
    NULL
};

static uchar *helparray[] =
{
    "KEYHELP",
    "DESIGNING",
    "FPAINTER",
    "GRID",
    "RECTPOLY",
    "MOTIFEDIT"
};

void run_help_menu( void )
{
    short rvl = 0;

    while( rvl != -1 )
    {
        rvl = MenuPopArray( rvl, "Help!", NULL, helpitems );

        if( rvl != -1 )
            help_disp( helparray[rvl], 20 );
    }
}

void DecompileScript( char* pBuffer, char* pszFilename,
    char* pszMapTag );

static int _mn_decompile()
{
    DecompileScript( pBehavior, pwdCurWad->fn, szCurMapTag );
    return maExit;
}

int EditMapOptions();

extern Menu* mnu_tools;
void CreateToolsMenu();

void mk_menus( void )
/*-------------------------------------------------------------------------*
 * Creates the menu bar.                                                   *
 * On the first call, creates a menu for the menu bar and all "constant"   *
 * menus (such as Map, Help, etc), PLUS a menu pointer for the object menu *
 * in each edit mode (there's a different object menu for each mode.)      *
 *-------------------------------------------------------------------------*/
{
    if( mnu_main == NULL )
    {
/*-------------------------------------------------------------------------*
 * new map menu
 *-------------------------------------------------------------------------*/
        Menu* mnuNewMap = new Menu( 8, Point( 0, 0 ), mtVertical,
            (MSTATFP) ShowMenuMsg );

        for( int i = 0; i < iMaxGame; i++ )
        {
            mnuNewMap->AddItem( (char*) pgdGameArray[i].name, (MFUNC) _mn_newmap, i,
                "Create a new map for this game" );
        }

/*-------------------------------------------------------------------------*
 * Define file menu                                                        *
 *-------------------------------------------------------------------------*/
        mnu_file = new Menu( 12, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        mnu_file->AddItem( "&New...", mnuNewMap,
         "Create a new map from scratch (wipe the current map)" );
        mnu_file->AddItem( "&Open...\tAlt+E", (MFUNC) _mn_postch,
         KEY_ALTE, "Edit an existing map (new episode/mission); load another PWAD" );
        mnu_file->AddItem( "&Save...\tAlt+S", (MFUNC) _mn_postch,
         KEY_ALTS, "Save the current map" );
        mnu_file->AddItem( "Save &As...\tAlt+A", (MFUNC) _mn_postch,
         KEY_ALTA, "Save the current map under a new file or map name" );
        mnu_file->AddSepbar();
        mnu_file->AddItem( "&Run map\tF9", (MFUNC) _mn_postch,
         KEY_F9, "Play-test the current map" );
        mnu_file->AddItem( "&WAD utilities\tAlt+W", (MFUNC) _mn_postch,
         0x1100, "WAD management features: Build PWADs" );
        mnu_file->AddItem( "&Decompile Behavior...", (MFUNC) _mn_decompile,
         0, "Decompile the Behavior Lump from the current map (Hexen)" );
        mnu_file->AddSepbar();
        mnu_file->AddItem( "&Configure "_ABBREV_"...\tAlt+N", (MFUNC) _mn_postch,
         KEY_ALTN, "Configuration options for "_ABBREV_ );
        mnu_file->AddSepbar();
      //mnu_file->AddItem( "&DOS Shell\t", (MFUNC) _mn_shell,
      //0, "Shell to DOS temporarily" );
        mnu_file->AddItem( "E&xit\tAlt+X", (MFUNC) exit_confirm,
         0, "Exit "_ABBREV_ );

/*-------------------------------------------------------------------------*
 * Define help menu                                                        *
 *-------------------------------------------------------------------------*/
        mnu_help = new Menu( 10, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        mnu_help->AddItem( "&About", (MFUNC) about, 0,
         "About the DOOM Construction Kit" );
        mnu_help->AddItem( "&Information", (MFUNC) usage, 0,
         "Information about the current map, etc" );
        mnu_help->AddSepbar();
        mnu_help->AddItem( "&Help\tF1", (MFUNC) key_help, 0,
         "Help with Keys" );

/*-------------------------------------------------------------------------*
 * Define edit menu                                                        *
 *-------------------------------------------------------------------------*/
        mnu_edit = new Menu( 16, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        mnu_edit->AddItem( "&Undo\tAlt+BkSp", (MFUNC) _mn_postch,
                     KEY_ALTBACK, "Undo the last action" );
        mnu_edit->AddSepbar();
        mnu_edit->AddItem( "C&opy\tCtrl+Ins", (MFUNC) _mn_postch, KEY_CTRLINS,
                     "Copy marked objects to the clipboard" );
        mnu_edit->AddItem( "C&ut\tCtrl+Del", (MFUNC) _mn_postch, KEY_CTRLDEL,
                     "Cut marked objects to the clipboard" );
        mnu_edit->AddItem( "&Paste\tIns", (MFUNC) _mn_paste, 0,
                     "Paste clipboard objects to the map" );
        mnu_edit->AddItem( "Clear\tDel", (MFUNC) _mn_postch, KEY_DEL,
                     "Clear (delete) marked objects" );
        mnu_edit->AddSepbar();
        mnu_edit->AddItem( "&Line editor / tool\tL", (MFUNC) _mn_postch, 'L',
                     "Enter Line editor mode" );
        mnu_edit->AddItem( "&Rectangle tool\tR", (MFUNC) _mn_postch, 'R',
                    "Switch to the rectangle tool" );
        mnu_edit->AddItem( "&Polygon tool\tN", (MFUNC) _mn_postch, 'N',
                    "Switch to the polygon tool" );
        mnu_edit->AddItem( "&Sector editor\tS", (MFUNC) _mn_postch, 'S',
                    "Enter Sector editor mode" );
        mnu_edit->AddItem( "&Item editor\tI", (MFUNC) _mn_postch, 'T',
                    "Enter Item editor mode" );
        //mnu_edit->AddItem( "&Vertex mode\tV", (MFUNC) _mn_postch, 'V',
        //            "Enter vertex edit mode" );
        mnu_edit->AddSepbar();
        mnu_edit->AddItem( "&Motif selection\tAlt+M", (MFUNC) _mn_postch, 0x3200,
                     "Select a new motif (texture defaults, etc)" );
        mnu_edit->AddItem( "E&dit script...", (MFUNC) EditScript, 0,
                     "Edits the current map's script" );

/*-------------------------------------------------------------------------*
 * Define misc menu
 *-------------------------------------------------------------------------*/
        mnu_misc = new Menu( 12, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        mnu_misc->AddItem( "Toggle &grid mode\tG", (MFUNC) _mn_postch,
                    'G', "Toggle grid on / off" );
        mnu_misc->AddItem( "Toggle sna&p to grid\tP", (MFUNC) _mn_postch,
                    'P', "Change snap to grid mode" );
        mnu_misc->AddItem( "Toggle cursor cross&hairs\t*", (MFUNC) _mn_postch,
                    '*', "Toggle crosshairs on / off" );
        mnu_misc->AddItem( "Toggle &full-screen mode\tF", (MFUNC) _mn_postch,
                    'F', "Toggle full-screen mode (no menu bar or status bars.)" );
        mnu_misc->AddSepbar();
        mnu_misc->AddItem( "&Clear all marks\tC", (MFUNC) _mn_postch,
                    'C', "Clear all object marks" );
        mnu_misc->AddItem( "&Format painter\tCtrl+P", (MFUNC) _mn_postch,
                    KEY_CTRLP, "Copies object attributes to other objects" );
        mnu_misc->AddSepbar();
        mnu_misc->AddItem( "&Check map\tAlt+C", (MFUNC) _mn_postch,
         KEY_ALTC, "Run the consistency checker (map diagnostics, debugger)" );

/*-------------------------------------------------------------------------*
 * Vertex menu                                                             *
 *-------------------------------------------------------------------------*/
        mnu_o_vertex = new Menu( 10, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        _mk_stdobj( mnu_o_vertex );
        mnu_o_vertex->AddItem( "Delete vertex/&join lines\tCtrl+J", (MFUNC) _mn_postch,
         KEY_CTRLJ, "Deletes each marked/selected vertex and joins the lines that it intercepts" );

/*-------------------------------------------------------------------------*
 * Line menu                                                               *
 *-------------------------------------------------------------------------*/
        mnu_o_line = new Menu( 23, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        _mk_stdobj( mnu_o_line );
        mnu_o_line->AddItem( "Delete line/&join sectors\tCtrl+J", (MFUNC) _mn_postch,
         KEY_CTRLJ,
         "Deletes each marked/selected line and joins the sectors that it separates" );
        mnu_o_line->AddItem( "Spli&t at Center\tCtrl+C", (MFUNC) _mn_postch,
         KEY_CTRLC, "Adds a vertex at the center of all Selected Lines" );
        mnu_o_line->AddItem( "&Flip\tCtrl+F", (MFUNC) _mn_postch,
         KEY_CTRLF, "Flips the direction and sides of each Selected Line" );
        mnu_o_line->AddItem( "S&wap sides\tCtrl+R", (MFUNC) _mn_postch, KEY_CTRLR,
         "Swaps the sides on each Selected Line" );
        mnu_o_line->AddItem( "Create &stairs\tCtrl+Z", (MFUNC) _mn_postch, KEY_CTRLZ,
         "Creates stairs along two Marked Lines" );
        Menu* pLineTexMenu = new Menu( 10, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        mnu_o_line->AddItem( "Textures\t>", pLineTexMenu );
        pLineTexMenu->AddItem( "Align &textures\tCtrl+W", (MFUNC) _mn_postch, KEY_CTRLW,
         "Automatically aligns textures on all Selected Lines" );
        pLineTexMenu->AddItem( "Rep&lace textures\tAlt+R", (MFUNC) _mn_postch, KEY_ALTR,
         "Searches for textures that you specify and replaces them with other textures" );
        pLineTexMenu->AddItem( "Add re&quired textures\tCtrl+I", (MFUNC) _mn_postch, KEY_CTRLI,
         "Adds missing textures by using the textures from surrounding Lines" );
        pLineTexMenu->AddItem( "Remove &unrequired textures\tCtrl+U", (MFUNC) _mn_postch, KEY_CTRLU,
         "Removes textures from lines that have them where they're not required" );
        pLineTexMenu->AddItem( "Appl&y the current motif\tCtrl+E", (MFUNC) _mn_postch, KEY_CTRLE,
         "Applies the current motif to all Selected Lines" );
        mnu_o_line->AddSepbar();
        mnu_o_line->AddItem( "Ta&g to a Sector...\tCtrl+T", (MFUNC) _mn_postch,
         KEY_CTRLT, "Tags the marked Lines to a Sector" );
        mnu_o_line->AddItem( "Tag to an Item...", (MFUNC) _mn_postch,
         CMD_TAGTOTHING, "Tags the marked Lines to an Item (Monster, etc.)" );
        mnu_o_line->AddItem( "Normali&ze\tCtrl+N", (MFUNC) _mn_postch, KEY_CTRLN,
         "Removes sector tags and sets the Selected Lines' type to normal (normalizes!)" );

/*-------------------------------------------------------------------------*
 * Sector menu                                                             *
 *-------------------------------------------------------------------------*/
        mnu_o_sector = new Menu( 20, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        _mk_stdobj( mnu_o_sector );
        mnu_o_sector->AddItem( "Create &Door...\tCtrl+D", (MFUNC) _mn_postch, KEY_CTRLD,
         "Creates a door from the marked sectors" );
        mnu_o_sector->AddItem( "Create &Lift...\tCtrl+L", (MFUNC) _mn_postch, KEY_CTRLL,
         "Creates a border-activated lift from the marked sectors" );
        mnu_o_sector->AddItem( "Change &heights or light levels\tCtrl+H", (MFUNC) _mn_postch, KEY_CTRLH,
         "Raises or lowers all marked sectors by a specified amount (light levels too)" );
        mnu_o_sector->AddItem( "Appl&y the current motif\tCtrl+E", (MFUNC) _mn_postch, KEY_CTRLE,
         "Applies the current motif to all marked sectors" );
        mnu_o_sector->AddSepbar();
/*        mnu_o_sector->AddItem( "Apply a sector object\tAlt+O", (MFUNC) _mn_postch, KEY_ALTO,
         "Applies a sector object to all marked sectors (acid pools, teleporters, etc.)" );
        mnu_addsepbar( mnu_o_sector );*/
        mnu_o_sector->AddItem( "Ta&g to a Line...\tCtrl+T", (MFUNC) _mn_postch,
         KEY_CTRLT, "Tags the marked Sectors to a Line" );
        mnu_o_sector->AddItem( "Normali&ze\tCtrl+N", (MFUNC) _mn_postch, KEY_CTRLN,
         "Removes sector tags from the marked sectors" );

/*-------------------------------------------------------------------------*
 * Thing menu                                                              *
 *-------------------------------------------------------------------------*/
        mnu_o_thing = new Menu( 10, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
        _mk_stdobj( mnu_o_thing );
        mnu_o_sector->AddItem( "Ta&g to a Line\tCtrl+T", (MFUNC) _mn_postch,
         KEY_CTRLT, "Tags the marked Items to a Line" );
        mnu_o_thing->AddItem( "&Display Filter\tD", (MFUNC) _mn_postch, 'D',
         "Set the criteria by which Items are displayed" );
        //mnu_o_thing->AddItem( "&Find things", 1, MITEM_TEXT, _mn_postch, KEY_ALTF,
        // "Searches for certain things and marks them" );

        CreateToolsMenu();
    }

    if( mnu_rc_line == NULL )
/*-------------------------------------------------------------------------*
 * Right-click menu
 *-------------------------------------------------------------------------*/
    {
        mnu_rc_line = new Menu( 10, Point( 10, 10 ), mtVertical, (MSTATFP) ShowMenuMsg );
        _mk_stdrc( mnu_rc_line, mnu_o_line );
        mnu_rc_vtx = new Menu( 10, Point( 10, 10 ), mtVertical, (MSTATFP) ShowMenuMsg );
        _mk_stdrc( mnu_rc_vtx, NULL );
        mnu_rc_vtx->AddItem( "&Join", (MFUNC) _mn_postch, KEY_CTRLJ,
         "Delete the marked vertices and merge the lines they separate" );
        mnu_rc_thing = new Menu( 10, Point( 10, 10 ), mtVertical, (MSTATFP) ShowMenuMsg );
        _mk_stdrc( mnu_rc_thing, mnu_o_thing );
        mnu_rc_sect = new Menu( 10, Point( 10, 10 ), mtVertical, (MSTATFP) ShowMenuMsg );
        _mk_stdrc( mnu_rc_sect, mnu_o_sector );
    }

    if( mnu_main != NULL )
        mnu_main->Clear();
    else
        mnu_main = new Menu( 10, Point( 0, 0 ), mtHorizontal, (MSTATFP) ShowMenuMsg );

    /* Add some items */
    mnu_main->AddItem( "&File", mnu_file, "New/Load/Save Map, Exit "_ABBREV_ );
    mnu_main->AddItem( "E&dit", mnu_edit, "Edit operations (cut/copy/paste), change edit mode" );
    /* Depends on the edit mode */
    if( editmode == MODE_VERTEX )
    {
        mnu_main->AddItem( "&Object (Vertex)", mnu_o_vertex,
            "Vertex operations" );
    }
    else if( editmode == MODE_LINE )
    {
        if( !tog_poly && !tog_rect )
            mnu_main->AddItem( "&Object (Line)", mnu_o_line,
                "Line operations" );
    }
    else if( editmode == MODE_SECTOR )
    {
        mnu_main->AddItem( "&Object (Sector)", mnu_o_sector,
            "Sector operations" );
    }
    else if( editmode == MODE_THING )
    {
        mnu_main->AddItem( "&Object (Item)", mnu_o_thing,
            "Item operations" );
    }
    mnu_main->AddItem( "M&iscellaneous", mnu_misc, "Miscellaneous features" );
    mnu_main->AddItem( "&Textures", (MFUNC) _mn_texfuncs, 0,
        "Textures: Files, browser, editor" );
    mnu_main->AddItem( "Too&ls", mnu_tools,
        "User-customizable tools menu" );
    mnu_main->AddItem( "&Help", mnu_help, "Help!" );
}

/*-------------------------------------------------------------------------*
 * popup menu functions
 *-------------------------------------------------------------------------*/

static int   iPopValue;

static int _ChoosePopItem( long lItem )
{
    iPopValue = lItem;
    return maExit;
}

static void CheckMenuPosition( Menu* pMenu )
{
    pMenu->SetRect();

    Rect r( pMenu->GetRect() );

    if( r.bottom > iScreenHeight )
        r.top -= (r.bottom - iScreenHeight) + 2;
    if( r.top < 0 )
        r.top = 2;
    if( r.right > iScreenWidth )
        r.right -= (r.right - iScreenWidth) + 2;
    if( r.left < 0 )
        r.left = 2;

    pMenu->SetPos( r.TopLeft() );
}

int MenuPopArray( int iStartItem, const char* pszTitle, const char* pszHelp,
    const char* pszItemArray[] )
{
    int nItem = 0;
    iPopValue = -1;

    for( nItem = 0; pszItemArray[nItem]; nItem++ );

    Menu *pPopMenu = new Menu( nItem + (pszTitle ? 2 : 0),
        Point( scursorx, scursory ), mtVertical, NULL );

    if( pszTitle )
    {
        pPopMenu->AddText( pszTitle );
        pPopMenu->AddSepbar();
        iStartItem += 2;
    }

    for( int i = 0; i < nItem; i++ )
    {
        if( !strcmp( pszItemArray[i], "-" ) )
        {
            pPopMenu->AddSepbar();
        }
        else
        {
            pPopMenu->AddItem( pszItemArray[i], (MFUNC) _ChoosePopItem, i );
        }
    }

    // check for screen fuckup
    CheckMenuPosition( pPopMenu );

    pPopMenu->Run( iStartItem );
    delete pPopMenu;

    return iPopValue;
}

int MenuPop( int iStartItem, const char* pszTitle, const char* pszHelp,
    ... )
{
    int nItem = 0;
    const char *pszItemArray[64];

    va_list vl;
    va_start( vl, pszHelp );
    for( nItem = 0; ; nItem++ )
    {
        pszItemArray[nItem*2] = va_arg( vl, char* );
        if( pszItemArray[nItem*2] == NULL )
            break;
        pszItemArray[nItem*2+1] = va_arg( vl, char* );
    }
    va_end( vl );

    Menu *pPopMenu = new Menu( nItem + (pszTitle ? 2 : 0),
        Point( scursorx, scursory ), mtVertical, NULL );

    if( pszTitle )
    {
        pPopMenu->AddText( pszTitle );
        pPopMenu->AddSepbar();
        iStartItem += 2;
    }

    for( int i = 0; i < nItem; i++ )
    {
        if( !strcmp( pszItemArray[i*2], "-" ) )
        {
            pPopMenu->AddSepbar();
        }
        else
        {
            pPopMenu->AddItem( pszItemArray[i*2], (MFUNC) _ChoosePopItem, i,
                pszItemArray[i*2+1] );
        }
    }

    // check for screen fuckup
    CheckMenuPosition( pPopMenu );

    iPopValue = -1;
    pPopMenu->Run( iStartItem );
    delete pPopMenu;

    return iPopValue;
}
