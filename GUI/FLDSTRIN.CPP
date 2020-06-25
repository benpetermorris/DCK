#include <std.h>
#include "dialog.hpp"

fldString::fldString( Dialog* pdlgParent, Point& ptPoint, int iWidth,
            int iType, const char *pszPrompt, int iStyle, int iMaxChars ) :
/*-------------------------------------------------------------------------*
 * defines a string field.  party.
 *-------------------------------------------------------------------------*/
    Field( pdlgParent, iType, iStyle | csUsesCaret )
{
    if( pszPrompt )
    {
        Point ptPrompt = ptPoint;

        Size szString( pszPrompt );
        szString.PixelsToUnits();
        // szString.height++;
        szString.width += 4;
        if( GetStyle() & csPromptBeside )
        {
            szString.height = 0;
            ptPrompt += Point( 0, 2 );
            ptPoint += szString;
        }
        else
        {
            szString.width = 0;
            ptPrompt += Point( 0, -1 );
            ptPoint += szString;
        }

        plbPrompt = (Label*) new stText( pdlgParent, ptPrompt, pszPrompt, iStyle );
    }

    // create coordinates
    Create( ptPoint, Size( iWidth, 10 ) );
    ptDlgUnits += Point( 2, 1 );
    rcDlgPixels.Inflate( 1, 1 );

    // set up other vars
    this->iMaxChars = iMaxChars;

    pszEditBuf = new char[iMaxChars+1];
    pszEditBuf[0] = 0;
    iCurPos = 0;
    iScrollPos = 0;
    pszAllow = NULL;
    nCharsVis = 0;
    SetFirstEdit( TRUE );  // first edit
}

fldString::~fldString()
/*-------------------------------------------------------------------------*
 * Destructor
 *-------------------------------------------------------------------------*/
{
    // delete the edit buffer
    delete pszEditBuf;
}

int fldString::Event( Event_t& event )
/*-------------------------------------------------------------------------*
 * Process an event.
 *-------------------------------------------------------------------------*/
{
    switch( event.type )
    {
    case etKey:
        return OnKey( event.iCode );
    case etMouse:
        // position cursor
        if( event.iCode == MOUSE_LDOWN )
        {
            int iTotalLen = 0, iLastLen = 0;
            char szCharBuf[2], *pEdit = pszEditBuf + iScrollPos;

            szCharBuf[1] = 0;

            // find the width of each char, and stop at the one
            // the mouse is on.
            event.ptPoint -= Point( _XUnitsToPixels( 2 ), 0 );

            while( *pEdit )
            {
                szCharBuf[0] = *pEdit;
                iTotalLen += textwidth( szCharBuf );
                if( inrange( event.ptPoint.x, iLastLen, iTotalLen ) )
                    break;
                ++pEdit;
            }

            SetCurPos( (int) (pEdit - pszEditBuf) );
            UpdateCaretPos();
        }
        break;
    }

    return daContinue;
}

void fldString::Draw()
/*-------------------------------------------------------------------------*
 * Draws the string.  Wow.
 *-------------------------------------------------------------------------*/
{
    if( !CanDraw() )
        return;

    int iColor;

    if( GetStyle() & csSkip )
        iColor = GetGuiColor( gcDlgBkgrnd );
    else
        iColor = GetGuiColor( gcFldBkgrnd );

    // draw rectangle
    pdlgParent->ShadowRect( rcDlgPixels, srIndent, -1 );
    Rect rcRect = rcDlgPixels;
    rcRect.Inflate( -1, -1 );
    pdlgParent->ShadowRect( rcRect, srIndent | srDarker, iColor );

    // display text
    SetViewPortToField();
    setcolor( GetGuiColor( gcFldText ) );
    setwritemode( COPY_PUT );
    pdlgParent->DrawTextXY( ptDlgUnits, pszEditBuf + iScrollPos );
    pdlgParent->SetActive();
}

void fldString::Erase()
/*-------------------------------------------------------------------------*
 * Erases the string.  Excitement.
 *-------------------------------------------------------------------------*/
{
    if( !CanDraw() )
        return;

    pdlgParent->FillRect( rcDlgPixels );
}

long fldString::GetValidateData()
/*-------------------------------------------------------------------------*
 * Return a value to be passed to the field's validation function.
 *-------------------------------------------------------------------------*/
{
    // want to validate the data
    if( GetType() == ftString )
        return (long) GetText();
    else
        return atol( GetText() );
}

void fldString::Transfer( int iDirection )
/*-------------------------------------------------------------------------*
 * transfers the string to or from the storage buffer.
 *-------------------------------------------------------------------------*/
{
    if( !pvTransferBuf )    // nothing to transfer to/from
        return;

    if( GetType() == ftString )
    {
        // string
        if( iDirection == transFromField )
            strcpy( (char*) pvTransferBuf, pszEditBuf );
        else
            strcpy( pszEditBuf, (char*) pvTransferBuf );
    }
    else
    {
        // integer
        int *ipXfer = (int*) pvTransferBuf;

        if( iDirection == transFromField )
        {
            if( pszEditBuf[0] || !pdlgParent->GetMultiEdit() )
                ipXfer[0] = atoi( pszEditBuf );
            else
                ipXfer[0] = FI_NODATA;
        }
        else
        {
            if( ipXfer[0] != FI_NODATA )
                itoa( ipXfer[0], pszEditBuf, 10 );
            else
                pszEditBuf[0] = 0;
        }
    }

    if( iDirection == transToField )
    {
        // put the cursor at the end
        SetCurPos( strlen( pszEditBuf ) );
    }
}

void fldString::UpdateCaretPos()
/*-------------------------------------------------------------------------*
 * Sets the position of the caret based on the current position in
 * the edit string.
 *-------------------------------------------------------------------------*/
{
    char szBuf[128];

    strncpy( szBuf, pszEditBuf + iScrollPos, iCurPos - iScrollPos );
    szBuf[iCurPos - iScrollPos] = 0;
    SetCaretPos( Point( textwidth( szBuf ) + _XUnitsToPixels( 2 ), 3 ) );

    ShowCaret();
}

int fldString::OnEnter()
/*-------------------------------------------------------------------------*
 * set up caret size, position, etc
 *-------------------------------------------------------------------------*/
{
    // set up size
    SetCaretSize( Size( 2, rcDlgPixels.Height() - 6 ) );
    UpdateCaretPos();

    return Field::OnEnter();
}

int fldString::OnExit()
/*-------------------------------------------------------------------------*
 * nuke the caret
 *-------------------------------------------------------------------------*/
{
    return Field::OnExit();
}

static void inschinstr( char *s, uchar ch, int pos )
/* Insters character 'ch' into 's' at 'pos' */
{
    register int i, ilen = strlen(s);

    for( i = ilen+1; i > pos; i-- )
        *(s+i) = *(s+i-1);

    *(s+pos) = ch;
}

bool fldString::IsValidInput( unsigned uCh )
/*-------------------------------------------------------------------------*
 * checks the allowed characters, and returns 1 if uch qualifies.
 *-------------------------------------------------------------------------*/
{
    if( !inrange( uCh, 32, 126 ) )
        return FALSE;

    if( GetType() == ftInteger && (!isdigit( uCh ) && uCh != '-') )
        return FALSE;

    if( !pszAllow )
        return TRUE;    // no more checking to do

    // check allow string
    // ...

    return TRUE;
}

void fldString::SetText( const char* pszString )
/*-------------------------------------------------------------------------*
 * Set text, yes.
 *-------------------------------------------------------------------------*/
{
    strcpy( pszEditBuf, pszString );
    SetCurPos( strlen( pszString ), TRUE, TRUE );
}

void fldString::SetInteger( int iValue )
{
    if( GetType() != ftInteger )
        return;

    char szText[128];

    if( iValue == FI_NODATA )
        szText[0] = 0;
    else
        itoa( iValue, szText, 10 );

    SetText( szText );
}

void fldString::SetCurPos( int iPos, bool fChunkIt, bool fForceUpdate )
/*-------------------------------------------------------------------------*
 * Sets the caret position in the field.
 *-------------------------------------------------------------------------*/
{
    char szVisBuf[128];
    int iOldScrollPos = iScrollPos;

    iCurPos = iPos;

    if( iCurPos < iScrollPos )
    {
        while( iCurPos < iScrollPos )
            iScrollPos -= 5;
        if( iScrollPos < 0 )
            iScrollPos = 0;
    }
    else
    {
        // figure out if we're past the visible end..
        strcpy( szVisBuf, pszEditBuf + iScrollPos );
        szVisBuf[iCurPos - iScrollPos] = 0;

        // if so, scroll a bit.
        while( (textwidth( szVisBuf )) >
            (rcDlgPixels.Width() - _XUnitsToPixels( 2 )) )
        {
            iScrollPos += 5;
            strcpy( szVisBuf, szVisBuf+5 );
        }
    }

    if( CanDraw() )
    {
        ShowCaret( FALSE );

        if( iScrollPos != iOldScrollPos || fForceUpdate )
            OnDraw();

        UpdateCaretPos();
    }
}

static int SetFieldToChoice( Field* pChoiceField )
{
    fldString* pStringField = (fldString*) pChoiceField->pUserData;
    pStringField->SetText( (const char*)
            pChoiceField->GetValidateData() );

    return daContinue;
}

static int KillField( Field* pChoiceField )
{
    Field* pStringField = (Field*) pChoiceField->pUserData;

    pChoiceField->OnErase();
    pChoiceField->Remove();
    // delete pChoiceField;

    // go to the user data pointer
    Dialog* pDialog = (Dialog*) pStringField->GetParent();

    pDialog->pfldCurrent = pStringField;

    return daContinue;
}

void fldString::PopChoices()
/*-------------------------------------------------------------------------*
 * If there is a choice list, show it
 *-------------------------------------------------------------------------*/
{
    if( !stlChoices )
        return;

    fldListBox* pChoiceField;

    // create choicelist
    Rect rcChoices = rcDlgUnits;
    rcChoices.MoveRel( Size( 0, rcDlgUnits.Height() + 3 ) );
    rcChoices.bottom = rcChoices.top + 8 * 6;
    if( !pdlgParent->pChoiceListBox )
    {
        pChoiceField = new fldListBox( pdlgParent, rcChoices, NULL, csPopup );
        pdlgParent->pChoiceListBox = pChoiceField;
    }
    else
    {
        pChoiceField = pdlgParent->pChoiceListBox;
        pChoiceField->SetRect( rcChoices );
    }
    //pChoiceField->rcDlgPixels.left = rcDlgPixels.left;
    //pChoiceField->rcDlgPixels.right = rcDlgPixels.right;
    pChoiceField->SetItemList( stlChoices );
    pChoiceField->pUserData = (PTR) this;
    pChoiceField->SetOnChange( (FFUNC) SetFieldToChoice );
    pChoiceField->SetOnExit( (FFUNC) KillField );
    pChoiceField->ReInit();
    pChoiceField->OnDraw();
    pdlgParent->GoToItem( pChoiceField );
}

int fldString::OnKey( unsigned uKey )
/*-------------------------------------------------------------------------*
 * Processes keystrokes for the field, yes.
 *-------------------------------------------------------------------------*/
{
    switch( uKey )
    {
    // movement keys:
    case KEY_LEFT:
        if( iCurPos )
            SetCurPos( iCurPos-1, FALSE );
        break;
    case KEY_RIGHT:
        if( iCurPos < GetLength() )
            SetCurPos( iCurPos+1, FALSE );
        break;
    case KEY_HOME:
        if( iCurPos )
            SetCurPos( 0 );
        break;
    case KEY_END:
        if( iCurPos != GetLength() )
            SetCurPos( GetLength() );
        break;
    // choices:
    case KEY_F2:
        // PopChoices();
        return daContinue;
    // toggle insert mode
    case KEY_INS:
        SetInsertMode( !IsInsertMode() );
        break;
    // backspace
    case KEY_BACK:
        if( (GetStyle() & csReadOnly) )
            break;

        if( iCurPos )
        {
            strcpy( pszEditBuf+iCurPos-1, pszEditBuf+iCurPos );
            if( iScrollPos && (iCurPos - iScrollPos) == 5 )
                --iScrollPos;
            SetCurPos( iCurPos-1, TRUE, TRUE );
            OnChange();
        }
        break;
    // delete
    case KEY_DEL:
        if( (GetStyle() & csReadOnly) )
            break;

        if( iCurPos < GetLength() )
        {
            strcpy( pszEditBuf+iCurPos, pszEditBuf+iCurPos+1 );
            SetCurPos( iCurPos, TRUE, TRUE );
            OnChange();
        }
        break;
    // typing keys
    default:
        if( (GetStyle() & csReadOnly) )
            break;

        if( IsValidInput( uKey ) &&     // check for valid input
            !(iCurPos == iMaxChars && !fFirstEdit) )
        {
            if( fFirstEdit )
            {
                // kill edit string
                pszEditBuf[0] = 0;
                SetCurPos( 0, TRUE, TRUE );
            }

            if( GetStyle() & csUpper )  // uppercase it?
                uKey = toupper( uKey );

            // now, insert or overstrike the current key
            if( IsInsertMode() && !(iCurPos == GetLength()) )
            {
                inschinstr( pszEditBuf, uKey, iCurPos );
                SetCurPos( iCurPos+1, TRUE, TRUE );
            }
            else
            {
overchar:
                // check length before adding to it
                if( iCurPos == GetLength() )
                    pszEditBuf[iCurPos+1] = 0;  // zero term
                // overwrite current character
                pszEditBuf[iCurPos] = uKey;
                SetCurPos( iCurPos+1, TRUE, TRUE );
            }

            OnChange();
        }
        break;
    }

    // entered a char now
    SetFirstEdit( FALSE );

    return daContinue;
}

