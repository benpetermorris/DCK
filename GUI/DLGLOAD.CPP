/*-------------------------------------------------------------------------*
 * dlgload.cpp:
 * Takes a dialog script file, and loads it up.
 *-------------------------------------------------------------------------*/

#include <std.h>
#include "dialog.hpp"
#include <fstream.h>
#include "msgbox.h"

static char* ReadStr( ifstream& inf, char* pszBuf = NULL )
{
    static char szStore[3][128];
    static int iStore = 0;

    if( pszBuf == NULL )
        pszBuf = szStore[iStore++];

    if( iStore == 3 )
        iStore = 0;

    int ch;

    while( 1 )
    {
        ch = inf.get();
        if( ch == '\"' )
            break;
        if( ch == '\n' )
            return NULL;
    }

    inf.getline( pszBuf, 128, '\"' );

    if( !pszBuf[0] )
        return NULL;

    return pszBuf;
}

static bool ReadTrueFalse( ifstream& inf )
{
    char szWord[128];
    inf >> szWord;
    return !strcmpi( szWord, "TRUE" );
}

static Rect ReadRect( ifstream& inf )
{
    int l, t, r, b;
    inf >> l;
    inf >> t;
    inf >> r;
    inf >> b;
    return Rect( l, t, r, b );
}

static Point ReadPoint( ifstream& inf )
{
    int x, y;
    inf >> x;
    inf >> y;
    return Point( x, y );
}

static Size ReadSize( ifstream& inf )
{
    int width, height;
    inf >> width;
    inf >> height;
    return Size( width, height );
}

static int ReadStyle( ifstream& inf )
{
    char szWord[128];
    int iStyle = 0, iNum;

    struct
    {
        char* pszName;
        int iValue;
    } Styles[] =
    {
        {"CSSKIP",          0x01},
        {"CSREADONLY",      0x02},
        {"CSUPPER",         0x04},
        {"CSMOUSEBUTTON",   0x08},
        {"CSINITNODISP",    0x10},
        {"CSINVISIBLE",     0x20},
        {"CSDISCARDABLE",   0x40},
        {"CSUSESCARET",     0x80},
        {"CSPROMPTBESIDE",  0x100},
        {"CSNOINTEGRALHEIGHT",  0x200},
        {"CSCREATEABSSIZE", 0x400},
        {"CSDEFAULTBUTTON", 0x800},
        {"CSAUTOSIZE",      0x1000},
        {"CSAUTOPLACE",     0x2000},
        {"CSPOPUP",         0x4000},

        // dialog styles
        {"DSSTATUSBAR",     0x01},
        {"DSCREATEABSSIZE", 0x02},
        {"DSSIMPLE",        0x04},

        {NULL,0}
    };

    // reads a style: accepts integers or csXXX defines, and stops
    // at the end-line..

    while( 1 )
    {
        int nextch = 0;

        while( !inf.eof() )
        {
            nextch = inf.peek();
            if( nextch == '\r' || nextch == '\n' || nextch == '\"' )
                return iStyle;
            if( nextch != 32 )
                break;
            inf.get();  // nuke that char
        }

        // get a word
        inf >> szWord;

        if( szWord[0] == '|' )
            continue;
        if( !isalpha( szWord[0] ) )
            break;

        iNum = 0;

        // is it a digit?
        if( isdigit( szWord[0] ) )
            iNum = atoi( szWord );  // yes
        else
        {
            // no, so parse it as a style
            strupr( szWord );
            for( int i = 0; Styles[i].pszName != NULL; i++ )
            {
                if( !strcmp( Styles[i].pszName, szWord ) )
                {
                    iNum = Styles[i].iValue;
                    break;
                }
            }

            if( Styles[i].pszName == NULL )
            {
                message_box( MB_OK, "Duhhh! DUHH!",
                    "What is a '%s'?!", szWord );
            }
        }

        iStyle |= iNum;
    }

    return iStyle;
}

bool Dialog::Create( const char* pszScript, const char* pszName )
/*-------------------------------------------------------------------------*
 * Creates a dialog from a dialog script.
 *-------------------------------------------------------------------------*/
{
    ifstream inf( pszScript );

    if( inf.fail() )
        return FALSE;

    char szWord[128];

    // find the dialog identifier
    while( !inf.eof() )
    {
        inf >> szWord;
        if( !strcmpi( szWord, "Dialog" ) )
        {
            inf >> szWord;
            if( !strcmpi( szWord, pszName ) )
                break;
        }
    }

    if( inf.eof() )
        return FALSE;   // not found.

    // now get information about the dialog
    Point ptPoint;
    Size szSize;

    // read point
    ptPoint = ReadPoint( inf );

    if( ptPoint.x == -1 )
        ptPoint.x = Centered;
    if( ptPoint.y == -1 )
        ptPoint.y = Centered;

    // read size
    szSize = ReadSize( inf );

    // read title
    char szTitle[128];
    ReadStr( inf, szTitle );

    // read "default buttons"?
    bool fDefaultButtons;
    fDefaultButtons = ReadTrueFalse( inf );

    // read flags
    unsigned uFlags;
    uFlags = ReadStyle( inf );

    // create dialog box
    Create( ptPoint, szSize, szTitle, uFlags );

    static StrList *stlTemp;

    if( !stlTemp )
    {
        stlTemp = new StrList( 1, 20 );
        stlTemp->AddTail( "Temporary." );
    }

    // read fields
    while( !inf.eof() )
    {
        Field* pField = NULL;
        Label* pLabel = NULL;
        char* pszHelp = NULL;

        // field name
        char szFieldName[32];
        inf >> szFieldName;

        if( szFieldName[0] == ';' )
        {
            inf.ignore( 255, '\n' );    // next line
            continue;
        }

        if( !strcmpi( szFieldName, "End" ) )
        {
            // all done
            break;
        }

        // is there a colon at the end?  this means "set field message"
        if( szFieldName[strlen( szFieldName ) - 1] == ':' )
        {
            szFieldName[strlen( szFieldName ) - 1] = 0;
            pField = GetField( szFieldName );
            char* pszText = ReadStr( inf );
            if( pField )
                pField->SetMessage( pszText );
            continue;
        }

        // field type
        char szFieldType[16];
        inf >> szFieldType;

        // check field type
        strupr( szFieldType );
        if( !strncmp( szFieldType, "CHECKBOX", 8 ) )
        {
            // create checkbox
            ptPoint = ReadPoint( inf );
            char* pszPrompt = ReadStr( inf );
            int iStyle = ReadStyle( inf );
            pszHelp = ReadStr( inf );
            pField = (Field*) new fldCheckBox( this, ptPoint, pszPrompt,
                iStyle, szFieldType[8] == '3' ? ftCheckBox3 : ftCheckBox );
        }
        else if( !strcmp( szFieldType, "STRING" ) )
        {
            ptPoint = ReadPoint( inf );
            int iSize;
            inf >> iSize;
            char* pszPrompt = ReadStr( inf );
            int iMaxChars;
            inf >> iMaxChars;
            int iStyle = ReadStyle( inf );
            pszHelp = ReadStr( inf );
            // create string field
            pField = (Field*) new fldString( this, ptPoint, iSize,
                ftString, pszPrompt, iStyle, iMaxChars );
        }
        else if( !strcmp( szFieldType, "INTEGER" ) )
        {
            ptPoint = ReadPoint( inf );
            int iSize;
            inf >> iSize;
            char* pszPrompt = ReadStr( inf );
            int iMaxChars;
            inf >> iMaxChars;
            int iStyle = ReadStyle( inf );
            pszHelp = ReadStr( inf );
            // create integer field
            pField = (Field*) new fldString( this, ptPoint, iSize,
                ftInteger, pszPrompt, iStyle, iMaxChars );
        }
        else if( !strncmp( szFieldType, "RADIO", 5 ) )
        {
            ptPoint = ReadPoint( inf );
            int iGroup;
            inf >> iGroup;
            char* pszPrompt = ReadStr( inf );
            int iStyle = ReadStyle( inf );
            pszHelp = ReadStr( inf );
            // create radio button
            pField = (Field*) new fldRadio( this, ptPoint, iGroup,
                pszPrompt, iStyle,
                szFieldType[5] == '2' ? ftRadio2 : ftRadio );
        }
        else if( !strcmp( szFieldType, "LISTBOX" ) )
        {
            // create listbox
            Rect rcRect = ReadRect( inf );
            char* pszPrompt = ReadStr( inf );
            int iStyle = ReadStyle( inf );
            pszHelp = ReadStr( inf );
            pField = (Field*) new fldListBox( this, rcRect, pszPrompt,
                iStyle );
            ((fldListBox*)(pField))->SetItemList( stlTemp );
        }
        else if( !strcmp( szFieldType, "BUTTON" ) )
        {
            // create button
            Rect rcRect = ReadRect( inf );
            char* pszPrompt = ReadStr( inf );
            int iStyle = ReadStyle( inf );
            FFUNC bFp = (FFUNC) NULL;
            pszHelp = ReadStr( inf );
            if( !strcmp( szFieldName, "OK" ) )
                bFp = (FFUNC) pbOkay;
            else if( !strcmp( szFieldName, "Cancel" ) )
                bFp = (FFUNC) pbCancel;
            pField = (Field*) new fldButton( this, rcRect, pszPrompt,
                (FFUNC) bFp, iStyle );
        }
        else if( !strcmp( szFieldType, "REGION" ) )
        {
            // create region
            Rect rcRect = ReadRect( inf );
            int iStyle = ReadStyle( inf );
            pszHelp = ReadStr( inf );
            pField = (Field*) new fldRegion( this, rcRect, iStyle );
        }

        // check for labels:
        else if( !strcmp( szFieldType, "TEXT" ) )
        {
            // create text
            ptPoint = ReadPoint( inf );
            char* pszPrompt = ReadStr( inf );
 //           int iStyle = ReadStyle( inf );
            pLabel = (Label*) new stText( this, ptPoint, pszPrompt );
        }
        else if( !strcmp( szFieldType, "BOX" ) )
        {
            // create bocks
            Rect rcRect = ReadRect( inf );
            char *pszText = ReadStr( inf );
            pLabel = (Label*) new stBox( this, rcRect, pszText );
        }
        else if( !strcmp( szFieldType, "VLINE" ) )
        {
            // create vline
            pLabel = (Label*) new stLine( this, ReadRect( inf ), ltVertical );
        }
        else if( !strcmp( szFieldType, "HLINE" ) )
        {
            // create hline
            pLabel = (Label*) new stLine( this, ReadRect( inf ), ltHorizontal );
        }

        else
        {
            // error!
            message_box( MB_OK, "I don't get it.",
                    "What kind of type is '%s'?!", szFieldType );
            inf.ignore( 255, '\n' );    // next line
        }

        if( pszHelp )
        {
            pField->SetHelp( pszHelp );
        }

        if( pField )
        {
            // set a field name
            pField->SetName( szFieldName );
        }
        else if( pLabel )
        {
            // set label name
            pLabel->SetName( szFieldName );
        }
    }

    if( fDefaultButtons )
        AddDefaultButtons();

    return TRUE;    // anyway, it's done
}
