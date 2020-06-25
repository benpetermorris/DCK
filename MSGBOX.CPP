/*-------------------------------------------------------------------------*
 * MSGBOX.C:                                                               *
 *                                                                         *
 * Contains single function, MessageBox().  Simply displays a messagebox   *
 * with a user-defined message and title, and allows the user to make      *
 * a selection from a combination of buttons.                              *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

/* Disable a few annoying warnings                                         */
#pragma warn -par
#pragma warn -sus

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals                                                                 *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static short mb_rvl;              /* Set by the pb action functions */
static short pbOkFunc( FIELD * );
static short pbCancelFunc( FIELD * );
static short pbYesFunc( FIELD * );
static short pbNoFunc( FIELD * );
static short pbAlwaysFunc( FIELD * );

/*-------------------------------------------------------------------------*
 *                                                                         *
 * File entry point is here                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static void mkchrfld( FIELD *fldp, uchar *chrbuf )
{
    while( *chrbuf )
    {
        switch( *chrbuf )
        {
        case 'Y':
            fld_setkey( fldp, 'Y', (__DFUNC) pbYesFunc );
            break;
        case 'N':
            fld_setkey( fldp, 'N', (__DFUNC) pbNoFunc );
            break;
        case 'A':
            fld_setkey( fldp, 'A', (__DFUNC) pbAlwaysFunc );
            break;
        case 'O':
            fld_setkey( fldp, 'O', (__DFUNC) pbOkFunc );
            break;
        case 'C':
            fld_setkey( fldp, 'C', (__DFUNC) pbCancelFunc );
            break;
        }

        ++chrbuf;
    }
}

static uchar *mystrtok( uchar *s )
{
    static uchar *p = NULL;
    uchar *rvl;

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

ushort message_box( ushort flags, uchar *title, uchar *fmt, ... )
{
    DIALOG      *dlgp;
    short       fm_rvl, pbxloc;
    short       stline = title ? 2 : 1;
    short       ln_q = strcountchar( fmt, '\n' )+(stline-1), col_q, nbutton = 0,
                tline = 1;
    uchar       *msg, *mptr, chrbuf[10], *chrp = chrbuf, *msglines[25];
    FIELD       *fldp;
    va_list     vl;

    _memstr = "messagebox() buffer";
    msg = memget( 1024 );          /* Get memory for message */

    va_start( vl, fmt );            /* Format string print */
    vsprintf( msg, fmt, vl );
    va_end( vl );

/* Get number of columns                                                   */
    col_q = (ushort)strchr( msg, '\n' ) != NULL ?
            (ushort)(strchr( msg, '\n' )-msg) : strlen( msg );

    mptr = mystrtok( msg );
    tline = 0;

    /* Define a bunch of textobjects! */
    while( mptr )
    {
        if( strlen( mptr ) > col_q )
            col_q = strlen( mptr );
        msglines[tline++] = mptr;
        mptr = mystrtok( NULL );
    }

    msglines[tline] = 0;

    if( col_q < strlen( title )+2 )    /* Make room for title */
        col_q = strlen( title )+2;

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
    if( col_q < (nbutton * 10) + 2 )
        col_q = (nbutton * 10) + 2;

/*-------------------------------------------------------------------------*
 * Define the form                                                         *
 *-------------------------------------------------------------------------*/
    dlgp = dlg_init( Centered, Centered, col_q + 2, ln_q + 5, title );

/*-------------------------------------------------------------------------*
 * Define the buttons                                                      *
 *-------------------------------------------------------------------------*/
    /* Find starting x location for buttons */
    pbxloc = col_q - (nbutton * 10) + 2;

    if( flags & MB_YES )
    {
/* Yes button                                                              */
        fldp = dlg_addpush( dlgp, pbxloc, ln_q+3, "  Yes   ", pbYesFunc );
        mkchrfld( fldp, chrbuf );
        pbxloc += 10;
    }
    if( flags & MB_NO )
    {
/* No Button                                                               */
        fldp = dlg_addpush( dlgp, pbxloc, ln_q+3, "   No   ", pbNoFunc );
        mkchrfld( fldp, chrbuf );
        pbxloc += 10;
    }
    if( flags & MB_ALWAYS )
    {
/* Always function                                                         */
        fldp = dlg_addpush( dlgp, pbxloc, ln_q+3, " Always ", pbAlwaysFunc );
        mkchrfld( fldp, chrbuf );
        pbxloc += 10;
    }
    if( flags & MB_OK )
    {
/* OK button                                                        */
        fldp = dlg_addpush( dlgp, pbxloc, ln_q+3, st_ok, pbOkFunc );
        mkchrfld( fldp, chrbuf );
        pbxloc += 10;
    }
    if( flags & MB_CANCEL )
    {
/* Cancel button                                                    */
        fldp = dlg_addpush( dlgp, pbxloc, ln_q+3, st_cancel, pbCancelFunc );
        mkchrfld( fldp, chrbuf );
        pbxloc += 10;
    }

/*-------------------------------------------------------------------------*
 * Define background text items                                            *
 *-------------------------------------------------------------------------*/

/* Only one line?  Get it differently and center the text.                 */
    if( ln_q == 1 )
        dlg_addtext( dlgp, Centered, stline, msglines[0] );
    else
    {
        tline = stline;

        while( msglines[tline-stline] )
        {
            dlg_addtext( dlgp, 1, tline, msglines[tline-stline] );
            ++tline;
        }
    }

/*-------------------------------------------------------------------------*
 * Stick up the form                                                       *
 *-------------------------------------------------------------------------*/
    fm_rvl = dlg_proc( 0, dlgp );

    dlg_free( dlgp );
    memfree( msg );

    if( fm_rvl == DF_NOSAVE )
        return (flags & MB_CANCEL) ? MB_CANCEL : MB_NO;

    return mb_rvl;
}

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Interface-called code begins here                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static short pbOkFunc( FIELD *notused )
/* Returns MB_OK                                                           */
{
    mb_rvl = MB_OK;
    return DF_SAVE;
}

static short pbCancelFunc( FIELD *notused )
/* Returns MB_CANCEL                                                       */
{
    mb_rvl = MB_CANCEL;
    return DF_SAVE;
}

static short pbYesFunc( FIELD *notused )
/* Returns MB_YES                                                          */
{
    mb_rvl = MB_YES;
    return DF_SAVE;
}

static short pbNoFunc( FIELD *notused )
/* Returns MB_NO                                                           */
{
    mb_rvl = MB_NO;
    return DF_SAVE;
}

static short pbAlwaysFunc( FIELD *notused )
/* Returns MB_ALWAYS */
{
    mb_rvl = MB_ALWAYS;
    return DF_SAVE;
}
