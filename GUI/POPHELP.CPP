#include <std.h>
#include <graphics.h>
#include "gui.hpp"
#include "shadow.h"
#include <math.h>
#include "msgbox.h"
#include "misc.h"

const int iHelpMargin = 5;

PopHelp::PopHelp() :
    pImageSave(NULL),
    pText(NULL),
    nAvoid(0),
    fMoreHelp(FALSE),
    fVisible(FALSE),
    fInit(FALSE),
    fActive(FALSE),

    nHelpKeys(0),
    pHelpKeys(NULL),
    pFp(NULL)
{
    pText = (char*) malloc( 1024 );
}

PopHelp::~PopHelp()
{
    if( pHelpKeys )
        free( pHelpKeys );
    if( pFp )
        fclose( pFp );
    if( pText )
        free( pText );
}

void PopHelp::AddAvoidRect( Rect& rcRect )
{
    if( nAvoid < 10 )
        rcAvoid[nAvoid++] = rcRect;
    if( nAvoid == 10 )
        message_box( MB_OK, "Error!", "More than 10 PopHelp Avoid-Rects" );
}

bool PopHelp::Init( const char* pszHelpFile )
{
    if( fInit )
        return FALSE;

    pFp = fopen( pszHelpFile, "rb" );

    if( pFp == NULL )
        return FALSE;

    // this is resized later
    pHelpKeys = (HelpKey*) malloc( sizeof( HelpKey ) * 512 );

    if( pHelpKeys == NULL )
    {
        fclose( pFp );
        return FALSE;
    }

    // first, find the keywords
    char szLineBuff[128];
    int iCurLine = 0, iLastKwdLine = 0;

    while( fgets( szLineBuff, 128, pFp ) != NULL )
    {
        iCurLine++;

        if( szLineBuff[0] == '[' )
        {
            if( nHelpKeys )
                pHelpKeys[nHelpKeys-1].nLines = (iCurLine - iLastKwdLine - 1);

            iLastKwdLine = iCurLine;

            // a keyword
            strccpy( pHelpKeys[nHelpKeys].szKeyword, szLineBuff+1, "]" );
            pHelpKeys[nHelpKeys].lFileOffset = ftell( pFp );
            ++nHelpKeys;
        }
    }

    if( nHelpKeys )
        pHelpKeys[nHelpKeys-1].nLines = (iCurLine - iLastKwdLine - 1);

    pHelpKeys = (HelpKey*) realloc( (void*) pHelpKeys,
            sizeof( HelpKey ) * nHelpKeys );

    fInit = TRUE;
    return TRUE;
}

bool PopHelp::LoadText( const char* pszKeyword )
{
    if( !fInit )
        return FALSE;

    int iLen = strlen( pszKeyword );

    if( pszKeyword == pszCurHelpKey )
        return TRUE;

    for( int i = 0; i < nHelpKeys; i++ )
    {
        if( !strncmpi( pszKeyword, pHelpKeys[i].szKeyword, iLen ) )
            break;
    }

    if( i == nHelpKeys )
        return FALSE;

    HelpKey& TheKey = pHelpKeys[i];
    char szLineBuff[128], *ptr;
    int iTextLen = 0, iSpaceWidth = textwidth( " " );

    iTextSize = 0;

    fseek( pFp, TheKey.lFileOffset, SEEK_SET );

    for( i = 0; i < TheKey.nLines; i++ )
    {
        fgets( szLineBuff, 128, pFp );
        ((char*) (ptr = strchr( szLineBuff, '\r' )))[0] = 0;
        strcpy( pText + iTextLen, szLineBuff );
        iTextLen += ((int) (ptr - szLineBuff)) + 1;
        strcpy( pText + iTextLen - 1, " " );
        iTextSize += textwidth( szLineBuff ) + iSpaceWidth;
    }

    return TRUE;
}

void PopHelp::Erase()
{
    if( !fInit )
        return;

    if( !fVisible )
        return;

    struct viewporttype vp;
    getviewsettings( &vp );

    SetViewPort( 0, 0, iScreenWidth, iScreenHeight );

    bool fHidden = mouse_hide( rcScreenRect );

    PutImage( rcScreenRect, pImageSave );
    free( pImageSave );
    pImageSave = NULL;

    mouse_show( fHidden );

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom );

    fVisible = FALSE;
}

void PopHelp::Display( const char* pszHelpKey, const char* pszMoreHelp )
{
    pszShowHelpKey = pszHelpKey;

    fMoreHelp = pszMoreHelp ? 1 : 0;

    Draw();
}

void PopHelp::Draw()
//
// displays the popup help window on the screen.
// first, determines if the window needs to be drawn:
//   - is the window not up!?  i'd say it needs drawing.
//   - the window is up.  does it intersect any of the
//     avoid rectangles?
//       - if so, recalculate its position and redraw.
//       - if not, redraw in current position (to erase text)
//
{
    if( !fInit || !fActive )
        return;

    Point ptScreen;
    Size sizeText, sizeRect;
    int i;
    bool fReposition = TRUE;

    if( fVisible && pszShowHelpKey == pszCurHelpKey )
    {
        // check intersection
        for( i = 0; i < nAvoid; i++ )
        {
            if( rcAvoid[i].Intersect( rcScreenRect ) )
                goto _ReDraw;
        }

        // just redraw
        fReposition = FALSE;
    }

_ReDraw:
    if( !pszShowHelpKey || !pszShowHelpKey[0] ||
        !LoadText( pszShowHelpKey ) )
    {
        Erase();
        return;
    }

    if( fReposition )
    {
        Erase();    // only if visible

        if( nAvoid < 2 )
        {
            message_box( MB_OK, "Error!", "nAvoid < 2" );
            return;
        }

        Rect rcAvoid1 = rcAvoid[nAvoid-1], rcAvoid0 = rcAvoid[nAvoid-2];
        char* pHorizOrder = "01", *pVertOrder = "23";
        int iDistance = 32000, iSide;

        if( rcAvoid1.left < iScreenWidth/2 )
        {
            // left side
            iSide = 0;
            iDistance = abs( rcAvoid1.left - rcAvoid0.left );
        }
        else
        {
            // right side
            iSide = 2;
            iDistance = abs( rcAvoid0.right - rcAvoid1.right );
        }

        if( rcAvoid1.top < iScreenHeight/2 )
        {
            // top
            int iTemp = abs( rcAvoid1.top - rcAvoid0.top );
            if( iTemp < iDistance )
            {
                iDistance = iTemp;
                iSide = 1;
            }
        }
        else
        {
            // bottom
            int iTemp = abs( rcAvoid0.bottom - rcAvoid1.bottom );
            if( iTemp < iDistance )
            {
                iDistance = iTemp;
                iSide = 3;
            }
        }

        char szOrder[5];
        strcpy( szOrder, pHorizOrder );
        strcat( szOrder, pVertOrder );

        // calculate square size
        //int iSqrt = (int) sqrt( iTextSize );
        int iNumLines;
        int iTryCount = 0;

_CheckAgain:
        if( iTryCount > 4 )
        {
            return; // just give the fuck up
        }

        switch( iSide )
        {
        case 0:
            // left
            sizeText.width = rcAvoid0.left - (iHelpMargin * 2) - 4;

            if( sizeText.width > iScreenWidth / 4 )
                sizeText.width = iScreenWidth / 4;

            sizeText.height = (iTextSize / sizeText.width + 2) * GetFontHeight() / 10;
            ptScreen.x = rcAvoid0.left - (sizeText.width + (iHelpMargin * 2)) - 2;
            ptScreen.y = rcAvoid1.top;

            if( sizeText.width < 50 )
            {
                ++iTryCount;
                ++iSide;
                goto _CheckAgain;
            }

            break;
        case 1:
            // top
            sizeText.width = rcAvoid0.left - (iHelpMargin * 2) - 4;

            if( sizeText.width > iScreenWidth / 4 )
                sizeText.width = iScreenWidth / 4;

            sizeText.height = (iTextSize / sizeText.width + 2) * GetFontHeight() / 10;
            ptScreen.x = rcAvoid1.left;
            ptScreen.y = rcAvoid0.top - (sizeText.height + (iHelpMargin * 2)) - 2;

            if( sizeText.width < 50 )
            {
                ++iTryCount;
                iSide = 0;
                goto _CheckAgain;
            }

            break;
        case 2:
            // right
            sizeText.width = (iScreenWidth - rcAvoid0.right) - (iHelpMargin * 2) - 4;

            if( sizeText.width > iScreenWidth / 4 )
                sizeText.width = iScreenWidth / 4;

            sizeText.height = (iTextSize / sizeText.width + 2) * GetFontHeight() / 10;
            ptScreen.x = rcAvoid0.right + 2;
            ptScreen.y = rcAvoid1.top;

            if( sizeText.width < 50 )
            {
                ++iTryCount;
                ++iSide;
                goto _CheckAgain;
            }

            break;
        case 3:
            // bottom:
            sizeText.height = iScreenHeight - rcAvoid0.bottom - (iHelpMargin * 2) - 4;

            if( sizeText.height > iScreenHeight / 4 )
                sizeText.height = iScreenHeight / 4;

            sizeText.width = (iTextSize / sizeText.height + 2);
            ptScreen.x = rcAvoid1.left;
            ptScreen.y = rcAvoid0.bottom + 2;

            if( sizeText.width < 50 )
            {
                ++iTryCount;
                iSide = 0;
                goto _CheckAgain;
            }

            break;
        }

        sizeRect.width = sizeText.width + iHelpMargin * 2;
        sizeRect.height = sizeText.height + iHelpMargin * 2;
        rcScreenRect.SetRect( ptScreen, sizeRect );
    }

_DrawWindow:
    static char *pFBlock;
    char *pLines[128], *pThisLine, *pTLP;
    int iNumLines = 0;

    if( !pFBlock )
        pFBlock = (char*) malloc( 1024 );

    char *p = pText, *pWord, *pStartWord;
    char szLine[128], szWord[128];
    int iLineWidth = 0, iWordWidth, iLineChars = 0, iWordChars;
    int iSpaceWidth = textwidth( " " );
    bool fGetNewWord = TRUE;

    pThisLine = pFBlock;

    for( ; *p || szWord[0]; )
    {
        // get word
        pWord = szWord;
        if( fGetNewWord )
        {
            iWordChars = 0;
            pStartWord = p;
            while( p[0] != 32 && p[0] )
            {
                *pWord++ = *p++;
                ++iWordChars;
            }
            *pWord = 0;
            while( p[0] == 32 )
                ++p;
            iWordWidth = textwidth( szWord );
        }

        if( iWordWidth > sizeText.width )
        {
            if( !fGetNewWord )
            {
                // this is an old word, so we must save it
                fGetNewWord = TRUE;
                goto SaveLine;
            }

            pWord = szWord;
            char *pszTmp = "\0\0";
            int iWidth = 0, iTmp, i = 0;
            while( *pWord )
            {
                pszTmp[0] = pWord[0];

                iTmp = textwidth( pszTmp );
                if( iWidth + iTmp > sizeText.width )
                {
                    if( !i )    // next word
                        goto ReLoop;

                    pWord[-1] = 0;
                    p = pStartWord + i;

                    // use this partial word next time
                    fGetNewWord = FALSE;

                    break;
                }

                iWidth += iTmp;
                ++i;
                ++pWord;
            }
        }

        if( iLineWidth + iWordWidth > sizeText.width )
        {
            fGetNewWord = FALSE;

SaveLine:   // jumped to by big-word processing

            // next line: save this one
            pLines[iNumLines++] = pThisLine;
            pThisLine += iLineChars + 1;
            iLineChars = iLineWidth = 0;
            szLine[0] = 0;
        }
        else
        {
            // add this word to the current line
            iLineWidth += iWordWidth + iSpaceWidth;
            memcpy( pThisLine + iLineChars, szWord, iWordChars );
            iLineChars += iWordChars + 1;
            memcpy( pThisLine + iLineChars - 1, " ", 2 );

            // get new word
            fGetNewWord = TRUE;
        }
ReLoop:
    }

    pLines[iNumLines++] = pThisLine;

    int iFontHeight = GetFontHeight() / 10;
    sizeText.height = iNumLines * iFontHeight;

    sizeRect.width = sizeText.width + iHelpMargin * 2;
    sizeRect.height = sizeText.height + iHelpMargin * 2;
    rcScreenRect.SetRect( ptScreen, sizeRect );

    if( rcScreenRect.left < 0 )    // extends too far to the left
        rcScreenRect.MoveTo( Point( 2, rcScreenRect.top ) );
    if( rcScreenRect.right >= iScreenWidth )    // to the right
        rcScreenRect.MoveRel( Size( iScreenWidth-rcScreenRect.right - 2, 0 ) );
    if( rcScreenRect.top < 0 )
        rcScreenRect.MoveTo( Point( rcScreenRect.left, 2 ) );
    if( rcScreenRect.bottom >= iScreenHeight )   // to the bottom
        rcScreenRect.MoveRel( Size( 0, iScreenHeight-rcScreenRect.bottom - 2 ) );

    if( rcAvoid[nAvoid-2].Intersect( rcScreenRect ) )
    {
        return;
    }

    unsigned char *pCheckPat = "\x55\xAA\x55\xAA\x55\xAA\x55\xAA";

    struct viewporttype vp;
    getviewsettings( &vp );

    SetViewPort( 0, 0, iScreenWidth, iScreenHeight );
    bool fHidden = mouse_hide( rcScreenRect );

    pImageSave = (char*) malloc( ImageSize( rcScreenRect ) );
    GetImage( rcScreenRect, pImageSave );

    setfillpattern( pCheckPat, C_YELLOW );
    setbkcolor( C_WHITE );
    Bar( rcScreenRect );
    setbkcolor( C_BLACK );
    setcolor( C_GRAY3 );
    Rectangle( rcScreenRect );
    Rect rcRect( rcScreenRect );
    rcRect.Inflate( -1, -1 );
    ShadowRect( rcRect, srIndent, -1 );

    setcolor( C_BLACK );

    int x = rcScreenRect.left + iHelpMargin,
        y = rcScreenRect.top + iHelpMargin;

    for( i = 0; i < iNumLines; i++, y += iFontHeight )
        outtextxy( x, y, pLines[i] );

    fVisible = TRUE;
    pszCurHelpKey = pszShowHelpKey;

    mouse_show( fHidden );

    SetViewPort( vp.left, vp.top, vp.right, vp.bottom );
}

void PopHelp::SetActive( bool _fActive )
{
    if( _fActive == 2 )
        _fActive = !fActive;

    if( _fActive && !fVisible )
    {
        fActive = _fActive;
        Draw();
    }
    else if( !_fActive && fVisible )
    {
        fActive = _fActive;
        Erase();
    }
}
