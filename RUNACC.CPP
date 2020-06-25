/*-------------------------------------------------------------------------*
 * RUNACC.CPP:
 * functions for running the hexen script compiler.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "acc.h"
#include <process.h>
#include <errno.h>

void _MouseSetHandler();
void _MouseKillHandler();
void DrawScreen();

int GetAnswer( const char* pszBuf, const char* pszChoices )
{
    printf( pszBuf );

    while( 1 )
    {
        int ch = toupper( keyget() );

        if( ch == KEY_ENTER )
            return pszChoices[0];

        if( strchr( pszChoices, ch ) )
            return ch;
    }
}

int DosCommand( const char* pszCommand, ... )
{
    char szRunBuf[256];
    va_list vl;

    va_start( vl, pszCommand );
    vsprintf( szRunBuf, pszCommand, vl );
    va_end( vl );

    printf( "\n\n[Command-line: %s]\n", szRunBuf );
    return system( szRunBuf );
}

const char* GetScriptName()
{
    static char szScriptName[128];
    strccpy( szScriptName, pwdCurWad->fn, "." );
    char *ptr = strrchr( szScriptName, '\\' ) + 1;
    ptr[6] = 0;
    strcat( szScriptName, szCurMapTag+3 );
    strcat( szScriptName, ".acs" );

    return szScriptName;
}

int EditScript()
{
    gr_vidmode( MODE_TEXT );
    DosCommand( "%s %s", config.szEditor, GetScriptName() );
    gr_vidmode( MODE_GFX );
    mouse_init();
    DrawScreen();
    mouse_show();

    return maExit;
}

bool UpdateScriptData( bool fRestoreGfx )
{
    char szRunBuf[128];
    char szObjectName[128];
    char szLogBuf[128];
    const char* pszScriptName;
    FILE *pFp;
    bool fInTextMode = FALSE;
    int iScriptOk;
    GWINDOW *gwnp;

#if 0
    strcpy( szScriptName, MapOptions.szScriptName );

    if( szScriptName[0] == 0 )
        return TRUE;

    if( strchr( szScriptName, '.' ) == NULL )
        strcat( szScriptName, ".acs" );
#else
    pszScriptName = GetScriptName();
#endif

    if( access( pszScriptName, 0 ) == -1 )
    {
        return TRUE;
    }

    strccpy( szObjectName, (uchar*) pszScriptName, "." );
    strcat( szObjectName, ".o" );

    // first, check to see if we need to do any of this ..
    if( !ACC_IsObjectOlder( pszScriptName, szObjectName ) )
        goto _LoadAndReturn;

_DoCompile:
#if 0
    if( access( pszScriptName, 0 ) == -1 )
    {
        //message_box( MB_OK, st_dcksays,
        //    "The script file '%s' does not exist.", pszScriptName );
        return FALSE;
    }
#endif

    if( !fInTextMode )
    {
        gwnp = wn_def( Centered, Centered, 27, 3, C_GRAY3, C_GRAY1,
                        WF_POPUP | WF_3D, st_dcksays );
        wn_disp( gwnp );
        wn_xytext( gwnp, Centered, 1, "Compiling Script..." );
    }
    else
    {
        clrscr();
        printf( "\nCompiling Script...\n" );
    }

    // compile!
    iScriptOk = ACC_CompileScript( (char*) pszScriptName, szObjectName );

    ACC_FreeCodeBuffer();

    if( !fInTextMode )
    {
        wn_undisp( gwnp );
        wn_free( gwnp );
    }

    if( !iScriptOk )
    {
        bool fEdit = FALSE;

        ACCErrorInfo_t ei;
        char szBuf[512];

        ACC_GetErrorInfo( &ei );

        if( ei.iLine == -1 )
        {
            sprintf( szBuf,
            "An error occured while compiling the script file:\n\n"
            "  Error #%d: %s\n"
            "  %s\n\n"
            "Do you want to edit it now?",
                ei.iError, ei.szErrorText, ei.szInfoText );
        }
        else
        {
            sprintf( szBuf,
            "An error occured while compiling the script file:\n\n"
            "  Line %d in file '%s'\n"
            "  Error #%d: %s\n"
            "  %s\n\n"
            "Do you want to edit it now?",
                ei.iLine, ei.szSourceName,
                ei.iError, ei.szErrorText, ei.szInfoText );
        }

        if( !fInTextMode )
        {
            if( message_box( MB_YESNO, st_dcksays, szBuf ) == MB_YES )
                fEdit = TRUE;
        }
        else
        {
            strcat( szBuf, " [Y/n] " );
            printf( "\n\n" );
            if( GetAnswer( szBuf, "YN" ) == 'Y' )
                fEdit = TRUE;
        }

        if( fEdit )
        {
            // edit it here..
            if( !fInTextMode )
            {
                gr_vidmode( MODE_TEXT );
                fInTextMode = TRUE;
            }

            if( ei.iLine == -1 )
                DosCommand( "%s %s", config.szEditor, pszScriptName );
            else
                DosCommand( "%s %s", config.szEditor, ei.szSourceName );

            goto _DoCompile;
        }
        else
        {
            if( fInTextMode && fRestoreGfx )
            {
                gr_vidmode( MODE_GFX );
                mouse_init();
                DrawScreen();
                mouse_show();
            }

            return FALSE;   // no script data
        }
    }

    // load it into behavior data
_LoadAndReturn:
    if( fRestoreGfx && fInTextMode )
    {
        gr_vidmode( MODE_GFX );
        mouse_init();
        DrawScreen();
        mouse_show();
    }

    lBehaviorSize = 0L;
    if( pBehavior )
    {
        memfree( pBehavior );
        pBehavior = NULL;
    }

    pFp = fopen( szObjectName, "rb" );
    lBehaviorSize = filelength( fileno( pFp ) );
    pBehavior = (char*) memget( lBehaviorSize );
    fread( pBehavior, lBehaviorSize, 1, pFp );
    fclose( pFp );

    return TRUE;
}
