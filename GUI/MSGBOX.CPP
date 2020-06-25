/*-------------------------------------------------------------------------*
 * MSGBOX.C:                                                               *
 *                                                                         *
 * Contains single function, MessageBox().  Simply displays a messagebox   *
 * with a user-defined message and title, and allows the user to make      *
 * a selection from a combination of buttons.                              *
 *-------------------------------------------------------------------------*/

#include <std.h>
#include "dialog.hpp"
#include "misc.h"
#include "msgbox.h"

/* Disable a few annoying warnings                                         */
#pragma warn -par
#pragma warn -sus

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals                                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static int mb_rvl;              /* Set by the mb action functions */
static int mbYesFunc( Field* );
static int mbNoFunc( Field* );
static int mbAlwaysFunc( Field* );
static int mbOkFunc( Field* );
static int mbCancelFunc( Field* );

/*-------------------------------------------------------------------------*
 *                                                                         *
 * File entry point is here                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static void mkchrfld( Field* pField, char *chrbuf )
{
    while( *chrbuf )
    {
        switch( *chrbuf )
        {
        case 'Y':
            pField->SetUserEvent( etKey, 'Y', (FUFUNC) mbYesFunc );
            break;
        case 'N':
            pField->SetUserEvent( etKey, 'N', (FUFUNC) mbNoFunc );
            break;
        case 'A':
            pField->SetUserEvent( etKey, 'A', (FUFUNC) mbAlwaysFunc );
            break;
        case 'O':
            pField->SetUserEvent( etKey, 'O', (FUFUNC) mbOkFunc );
            break;
        case 'C':
            pField->SetUserEvent( etKey, 'C', (FUFUNC) mbCancelFunc );
            break;
        }

        ++chrbuf;
    }
}

static char *mystrtok( char *s )
{
    static char *p = NULL;
    char *rvl;

    if( s )
        p = s;

    if( !p )
        return NULL;

    if( !*p )
        return NULL;

    rvl = p;

    while( *p != '\n' && *p )
        ++p;

    if( *p == '\n' )
    {
        *p = 0;
        ++p;
    }

    return rvl;
}

int message_box( int flags, char *title, char *fmt, ... )
{
    int       fm_rvl, pbxloc;
    int       ln_q = strcountchar( fmt, '\n' )+1, col_q, nbutton = 0;
    char      *msg, *mptr, chrbuf[10], *chrp = chrbuf, *msglines[25];
    va_list   vl;

    msg = new char[1024];          /* Get memory for message */

    va_start( vl, fmt );            /* Format string print */
    vsprintf( msg, fmt, vl );
    va_end( vl );

    col_q = 0;

    mptr = mystrtok( msg );
    int tline = 0;

    while( mptr )
    {
        if( textwidth( mptr ) > col_q )
            col_q = textwidth( mptr );
        msglines[tline++] = mptr;
        mptr = mystrtok( NULL );
    }

    msglines[tline] = 0;

    if( col_q < textwidth( title ) )    /* Make room for title */
        col_q = textwidth( title );

    col_q = _XPixelsToUnits( col_q );
    col_q += 12;

/*-------------------------------------------------------------------------*
 * Now .. add some buttons depending on what the options specified were.   *
 *-------------------------------------------------------------------------*/
    if( flags & MB_YES )
    {
        *chrp++ = 'Y';
        ++nbutton;
    }
    if( flags & MB_NO )
    {
        *chrp++ = 'N';
        ++nbutton;
    }
    if( flags & MB_OK )
    {
        *chrp++ = 'O';
        ++nbutton;
    }
    if( flags & MB_CANCEL )
    {
        *chrp++ = 'C';
        ++nbutton;
    }
    if( flags & MB_ALWAYS )
    {
        *chrp++ = 'A';
        ++nbutton;
    }

    *chrp = 0;

    /* Make sure there's enough room .. */
    if( col_q < nbutton * 34 )
        col_q = nbutton * 34 + 24;

/*-------------------------------------------------------------------------*
 * Define the form                                                         *
 *-------------------------------------------------------------------------*/
    // dlgp = dlg_init( CENTER, CENTER, col_q + 2, ln_q + 5, title );
    Dialog *pDialog = new Dialog;
    pDialog->Create( Point( Centered, Centered ),
                     Size( col_q, (ln_q + 4) * YUPC ), title );

/*-------------------------------------------------------------------------*
 * Define the buttons                                                      *
 *-------------------------------------------------------------------------*/
    Point ptButton( pDialog->ClientSize().width - (nbutton+1) * 34,
                    pDialog->ClientSize().height - 20 );
    Rect rcButton( ptButton, Size( 30, 12 ) );

    if( flags & MB_YES )
    {
/* Yes button                                                              */
        rcButton.MoveRel( Point( 34, 0 ) );
        Field* pField = new fldButton( pDialog, rcButton, "Yes",
                        (FFUNC) mbYesFunc );
        mkchrfld( pField, chrbuf );
    }
    if( flags & MB_NO )
    {
/* No Button                                                               */
        rcButton.MoveRel( Point( 34, 0 ) );
        Field* pField = new fldButton( pDialog, rcButton, "No",
                        (FFUNC) mbNoFunc );
        mkchrfld( pField, chrbuf );
    }
    if( flags & MB_ALWAYS )
    {
/* Always function                                                         */
        rcButton.MoveRel( Point( 34, 0 ) );
        Field* pField = new fldButton( pDialog, rcButton, "Always",
                        (FFUNC) mbAlwaysFunc );
        mkchrfld( pField, chrbuf );
    }
    if( flags & MB_OK )
    {
/* OK button                                                        */
        rcButton.MoveRel( Point( 34, 0 ) );
        Field* pField = new fldButton( pDialog, rcButton, "Ok",
                        (FFUNC) mbOkFunc );
        mkchrfld( pField, chrbuf );
    }
    if( flags & MB_CANCEL )
    {
/* Cancel button                                                    */
        rcButton.MoveRel( Point( 34, 0 ) );
        Field* pField = new fldButton( pDialog, rcButton, "Cancel",
                        (FFUNC) mbCancelFunc );
        mkchrfld( pField, chrbuf );
    }

/*-------------------------------------------------------------------------*
 * Define background text items                                            *
 *-------------------------------------------------------------------------*/

/* Only one line?  Get it differently and Centered the text.                 */
    if( ln_q == 1 )
        new stText( pDialog, Point( Centered, YUPC - 2 ), msglines[0] );
    else
    {
        int iLine = 0;

        while( msglines[iLine] )
        {
            new stText( pDialog, Point( 4, YUPC * (iLine+1) - 2 ),
                        msglines[iLine] );
            ++iLine;
        }
    }

/*-------------------------------------------------------------------------*
 * Stick up the form                                                       *
 *-------------------------------------------------------------------------*/
    int iRvl = pDialog->Process();

    delete pDialog;
    delete msg;

    if( iRvl == daExitNoSave )
        return (flags & MB_CANCEL) ? MB_CANCEL : MB_NO;

    return mb_rvl;
}

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Interface-called code begins here                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static int mbOkFunc( Field* )
/* Returns MB_OK                                                           */
{
    mb_rvl = MB_OK;
    return daExitSave;
}

static int mbCancelFunc( Field* )
/* Returns MB_CANCEL                                                       */
{
    mb_rvl = MB_CANCEL;
    return daExitSave;
}

static int mbYesFunc( Field* )
/* Returns MB_YES                                                          */
{
    mb_rvl = MB_YES;
    return daExitSave;
}

static int mbNoFunc( Field* )
/* Returns MB_NO                                                           */
{
    mb_rvl = MB_NO;
    return daExitSave;
}

static int mbAlwaysFunc( Field* )
/* Returns MB_ALWAYS */
{
    mb_rvl = MB_ALWAYS;
    return daExitSave;
}
