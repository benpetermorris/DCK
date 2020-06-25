#include "dck.h"
#pragma hdrstop


/*-------------------------------------------------------------------------*
 *
 * tools
 *
 *-------------------------------------------------------------------------*/

const int nMaxTools = 16;

struct
{
    char szDescription[61];
    char szCommand[81];

} Tools[nMaxTools], TempTools[nMaxTools];

static int nTools, nTempTools;

void ReadTools()
{
    // reads the tools from the handle it's passed
    int iHandle = z_open( "TOOLS.CFG", O_BINARY | O_RDONLY );

    if( iHandle == -1 )
        return;

    lseek( iHandle, 30L, SEEK_SET );
    z_read( iHandle, &nTools, sizeof nTools );
    z_read( iHandle, &Tools, sizeof Tools );
    z_close( iHandle );
}

void WriteTools()
{
    // write tools
    int iHandle = z_creat( "TOOLS.CFG", 0 );
    z_write( iHandle, "Tool Configuration\r\n\x1a", 30 );
    z_write( iHandle, &nTools, sizeof nTools );
    z_write( iHandle, &Tools, sizeof Tools );
    z_close( iHandle );
}

int RunTool( int iTool )
{
    int DosCommand( const char* pszCommand, ... );
    void DrawScreen();
    char szRunBuf[128];

    // text mode
    gr_vidmode( MODE_TEXT );

    // get cur dir
    char curdir[128];
    getcwd( curdir, 128 );

    // run it
    setfilertparms();
    strcpy( szRunBuf, Tools[iTool].szCommand );
    rt_replace( szRunBuf );
    DosCommand( szRunBuf );

    // set dir back
    chdir( curdir );
    setdisk( curdir[0] - 'A' );

    printf( "\n[Press a key to continue...]" );
    keyget();

    // gfx mode
    gr_vidmode( MODE_GFX );
    mouse_init();
    DrawScreen();
    mouse_show();

    return maExit;
}

/*-------------------------------------------------------------------------*
 *
 * edit-tools dialog box
 *
 *-------------------------------------------------------------------------*/

static fldListBox   *pfTools;
static fldString    *pfDescription, *pfCommand;
static StrList      *stlTools;

static int ffOnToolListChanged();

static int ffAdd()
{
    ++nTempTools;

    stlTools->AddTail( "New Tool" );
    pfTools->ReInit();
    pfTools->SetCurItem( nTempTools-1 );
    ffOnToolListChanged();

    pfTools->GetParent()->GoToItem( pfDescription );

    return daContinue;
}

static int ffRemove()
{
    int iMax = stlTools->GetCount() - 1,
        iNum = pfTools->GetCurItem();

    if( iNum <= 0 && iMax <= 0 )
        return daContinue;

    stlTools->Delete( iNum );
    pfTools->ReInit();
    pfTools->SetCurItem( iNum == iMax ? iMax-1 : iNum );

    --nTempTools;
    memset( &TempTools[nTempTools], 0, sizeof TempTools / nMaxTools );

    return daContinue;
}

static int ffOnDescriptionChanged()
{
    stlTools->SetAt( pfTools->GetCurItem(), pfDescription->GetText() );
    pfTools->ReInit();
    pfTools->OnDraw();

    strcpy( TempTools[pfTools->GetCurItem()].szDescription, pfDescription->GetText() );

    if( !nTempTools )
        nTempTools++;

    return daContinue;
}

static int ffOnCommandChanged()
{
    strcpy( TempTools[pfTools->GetCurItem()].szCommand, pfCommand->GetText() );
    return daContinue;
}

static int ffOnToolListChanged()
{
    pfCommand->SetText( TempTools[pfTools->GetCurItem()].szCommand );
    pfDescription->SetText( TempTools[pfTools->GetCurItem()].szDescription );

    return daContinue;
}

int EditTools()
{
    Dialog *pDialog = new Dialog;
    pDialog->Create( szDSFile, "ToolsDlg" );

    VarTransfer_t vtTools[] =
    {
        { "Tools", (Field**) &pfTools, NULL, NULL },
        { "Description", (Field**) &pfDescription, NULL, NULL },
        { "Command", (Field**) &pfCommand, NULL, NULL },
        { "BtnAdd", NULL, (PTR) ffAdd, NULL },
        { "BtnRemove", NULL, (PTR) ffRemove, NULL },

        // end-marker
        { NULL, NULL, NULL, NULL }
    };

    pDialog->VarSetup( vtTools );

    // create string list for tools
    stlTools = new StrList( nMaxTools, 80 );
    for( int i = 0; i < nTools; i++ )
        stlTools->AddTail( Tools[i].szDescription );

    memcpy( &TempTools, &Tools, sizeof Tools );
    nTempTools = nTools;

    if( !nTools )
    {
        stlTools->AddTail( "No Tools Defined" );
    }

    pDialog->SetOnEnter( (DFUNC) ffOnToolListChanged );

    pfTools->SetItemList( stlTools );
    pfTools->SetOnChange( (FFUNC) ffOnToolListChanged );
    pfDescription->SetOnChange( (FFUNC) ffOnDescriptionChanged );
    pfCommand->SetOnChange( (FFUNC) ffOnCommandChanged );

    int iRvl = pDialog->Process();

    delete pDialog;

    if( iRvl == daExitNoSave )  // no save
        return maExit;

    if( !nTools )
        ++nTempTools;

    memcpy( &Tools, &TempTools, sizeof Tools );
    nTools = nTempTools;

    // save file
    WriteTools();

    void CreateToolsMenu();
    CreateToolsMenu();

    return maExit;
}

/*-------------------------------------------------------------------------*
 *
 * create tool-menu function
 *
 *-------------------------------------------------------------------------*/

Menu *mnu_tools;

void CreateToolsMenu()
{
    void ShowMenuMsg( char* msg );

    if( !mnu_tools )
        mnu_tools = new Menu( 18, Point( 0, 0 ), mtVertical, (MSTATFP) ShowMenuMsg );
    mnu_tools->Clear();

    mnu_tools->AddItem( "&Customize...", (MFUNC) EditTools, 0,
        "Customize the tools that appear on this menu." );
    mnu_tools->AddSepbar();

    for( int i = 0; i < nTools; i++ )
        mnu_tools->AddItem( Tools[i].szDescription, (MFUNC) RunTool, i );
}

