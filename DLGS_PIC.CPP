/*-------------------------------------------------------------------------*
 * DLGS_PICK.CPP:
 * Dialogs for picking from a two-tiered list; ie: categories and
 * items in the category.
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "pickdlg.hpp"

static bool fUpdatingList;

static int ffChangeItem( fldListBox* pListBox );

const char* PickDialog::FindItemCategory( int iPosInList, int *ipCat )
{
    short   i, iCount = stlCategories->GetCount();

    for( i = 0; i < iCount; i++ )
    {
        /* ident of the thingsec list is its position start .. eh. */
        int iID;

        stlCategories->GetAt( NULL, i, &iID );

        if( iID > iPosInList )
        {
            if( ipCat )
                ipCat[0] = i-1;
            return (*stlCategories)[i-1];
            /* so if it's greater, it's from the last section. */
        }
    }

    /* last section.. it must be. */
    if( ipCat )
        ipCat[0] = i-1;
    return (*stlCategories)[i-1];
}


static int ffChangeCategory( fldListBox* pfCategories )
// when the category changes, update the item list by setting the
// item list's keyword to the category name and updates the item list.
{
    char szKeyBuf[128];

    sprintf( szKeyBuf, "*%s", pfCategories->GetText() );

    PickDialog* pDialog;

    // redisplay items list
    fldListBox* pfItems = (fldListBox*)
        (pDialog = (PickDialog*) pfCategories->GetParent())->GetField( "Items" );

    if( !pfCategories->GetCurItem() )
        pfItems->SetItemList( pDialog->stlAllItems );
    else
        pfItems->SetItemList( pDialog->stlItems );

    pfItems->GetItemList()->SetKey( szKeyBuf );
    pfItems->ReInit();
    pfItems->SetCurItem( 0 );

    ffChangeItem( pfItems );

    return daContinue;
}


static int ffUpDownList_Keys( fldString* pField, Event_t& event )
{
    PickDialog* pDialog = (PickDialog*) pField->GetParent();
    fldListBox* pfItems = pDialog->pfItems;

    // this is for up/down key events
    pfItems->Event( event );

    return daContinue;
}


static int ffUpdateList_Keys( fldString* pField )
/*-------------------------------------------------------------------------*
 * when the user hits a key, this function calls the PickDialog's
 * "update list" function with the new text.
 *-------------------------------------------------------------------------*/
{
    PickDialog* pDialog = (PickDialog*) pField->GetParent();

    pDialog->pfItems->SetItemList( pDialog->stlFoundItems );
    pDialog->pfCategories->SetCurItem( 0 ); // "all" category

    pDialog->FindWithKeys( pField->GetText() );
    pDialog->OnItemChanged();

    return daContinue;
}


static int ffChangeItem( fldListBox* pListBox )
// when the user picks a new item, this updates the search string
// with that item's text.
{
    if( fUpdatingList )
        return daContinue;

    PickDialog* pDialog = (PickDialog*) pListBox->GetParent();

    fldString* pfSearchString = pDialog->pfSearchString;
    pfSearchString->SetText( pListBox->GetText() );
    pfSearchString->SetFirstEdit( TRUE );

    pDialog->OnItemChanged();

    return daContinue;
}


static int fvString( const char* pszText, fldString* pField )
{
    StrList* stlItems = ((PickDialog*) pField->GetParent())->pfItems->GetItemList();

    if( stlItems->GetCount() != 1 &&
        stlItems->StrToIndex( pszText ) == -1 )
    {
        pField->SetValMsg( "You must select one of the list choices." );
        return valNotOkay;
    }

    return valOkay;
}


static int ffStringOnEnterKey( fldString* pField )
{
    PickDialog* pDialog = (PickDialog*) pField->GetParent();

    // save the current selection in the string
    pField->SetText( pDialog->pfItems->GetText() );

    return -1;  // for normal processing
}


void PickDialog::OnItemChanged()
{
    if( fpOnChange )
        (*fpOnChange)(pfItems->GetText(), pfItems->GetItemList());
}

PickDialog::~PickDialog()
{
    delete stlFoundItems;

    if( !fUseCats )
    {
        delete stlCategories;   // temporary with "(All)"
    }

    delete stlAllItems;
}

static int ffItemExit()
{
    return daExitSave;
}

static int dfPickDlgUp( PickDialog *pDialog )
{
    pDialog->OnItemChanged();
    return daContinue;
}


PickDialog::PickDialog( const char* pszDlgTemplate, StrList* stlCategories,
        StrList* stlItems, CHFUNC fpOnItemChange ) :
    Dialog()
{
    // create the template
    if( Create( szDSFile, pszDlgTemplate ) == FALSE )
        return;

    Point ptScreen( 0, Centered );
    ptScreen.x = (iScreenWidth - ScreenRect().Width());
    ptScreen.x = (ptScreen.x / 3) * 2;

    MoveTo( ptScreen );

    // get some the fields this dialog needs
    pfCategories = (fldListBox*) GetField( "Categories" );
    pfSearchString = (fldString*) GetField( "SearchString" );
    pfItems = (fldListBox*) GetField( "Items" );

    // this creates a non-keyworded item list
    stlAllItems = new StrList( stlItems );

    if( pfCategories == NULL || stlCategories == NULL )
        fUseCats = FALSE;
    else
        fUseCats = TRUE;

    if( !fUseCats )
    {
        stlCategories = new StrList( 1, 10 );
        stlCategories->AddTail( "(All)" );
    }

    pfCategories->SetItemList( stlCategories );
    pfCategories->SetOnChange( (FFUNC) ffChangeCategory );
    pfItems->SetItemList( stlItems );

    // set string lists
    // create list to store found items in
    stlFoundItems = new StrList( 256, 60, "Found items" );
    pfItems->SetItemList( stlFoundItems );
    pfItems->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC )ffItemExit );

    pfSearchString->SetOnChange( (FFUNC) ffUpdateList_Keys );
    pfSearchString->SetUserEvent( etKey, KEY_UP,
        (FUFUNC) ffUpDownList_Keys );
    pfSearchString->SetUserEvent( etKey, KEY_DOWN,
        (FUFUNC) ffUpDownList_Keys );
    pfSearchString->SetUserEvent( etKey, KEY_PGUP,
        (FUFUNC) ffUpDownList_Keys );
    pfSearchString->SetUserEvent( etKey, KEY_PGDN,
        (FUFUNC) ffUpDownList_Keys );
    pfSearchString->SetMessage(
        "Type in words to search for, and use the up/down keys." );

    pfSearchString->SetOnValidate( (FVFUNC) fvString );

    SetOnEnter( (DFUNC) dfPickDlgUp );

    fpOnChange = fpOnItemChange;
    pfItems->SetOnChange( (FFUNC) ffChangeItem );

    this->stlCategories = stlCategories;
    this->stlItems = stlItems;
}


bool PickDialog::FindWithKeys( const char* pszKeyText, bool fCopyAll )
// searches pfItems for items that have the words in "pszKeyText", and
// adds them to pfFoundItems.  then updates the item list.
// returns TRUE if the list was totally processed, or FALSE if the user
// interrupted with a key.
{
    char szKeyText[128];
    char *pWord, *pWordList[16];
    int i, nWords;

    fUpdatingList = TRUE;

    strcpy( szKeyText, pszKeyText );
    strcat( szKeyText, " " );

    for( i = 0; i < 16; i++ )
    {
        pWord = strtok( i ? NULL : szKeyText, " " );
        if( pWord == NULL )
            break;
        pWordList[i] = pWord;
    }

    if( !i ) // copy all thar strings in
        fCopyAll = TRUE;

    nWords = i;

    stlItems->SetKey( NULL );

    int iCount = stlItems->GetCount();
    stlFoundItems->DeleteAll();  // clear found item list

    for( i = 0; i < iCount; i++ )
    {
        const char* pszString = (*stlItems)[i];
        if( pszString[0] == '*' )   // keyword?  don't want it
            continue;
        // search for words
        if( !fCopyAll )
        {
            for( int iWord = 0; iWord < nWords; iWord++ )
            {
                if( striinc( pWordList[iWord], (char*) pszString ) == NULL )
                    goto _NextString;   // nope!
            }
        }
        // add this string
        stlFoundItems->AddTail( pszString );

        if( !fCopyAll && keycheck() )
        {
            fUpdatingList = FALSE;
            return FALSE;
        }
_NextString:
    }

    if( IsVisible() )
    {
        // thar she updates
        pfItems->ReInit();
        pfItems->SetCurItem( 0 );
    }

    fUpdatingList = FALSE;

    return TRUE;
}


int PickDialog::Process( const char* pszSelText, bool fShowEntireFirst )
/*-------------------------------------------------------------------------*
 * Processes the dialog box, using the words in pszSelText as search
 * strings - or not, if it's a dual-list dialog box.
 *-------------------------------------------------------------------------*/
{
    if( pszSelText == NULL )
        pszSelText = st_blank;

    stlItems->SetKey( NULL );
    if( stlItems->Find( pszSelText, strlen( pszSelText )+1 ) != -1 )
        fShowEntireFirst = TRUE;

    FindWithKeys( pszSelText, fShowEntireFirst );
    // now select the string if we're showing the entire list
 //   if( fShowEntireFirst )
    pfItems->SelectString( pszSelText );

   // if( pfItems->GetItemList()->GetCount() && !fUseCats )
    if( pfItems->GetText() != NULL )
        pfSearchString->SetText( pfItems->GetText() );

    // now process the dialog
    int iRvl = Dialog::Process();

    if( iRvl == daExitSave && pfItems->GetItemList()->GetCount() == 1 &&
        fUseCats )
    {
        // copy that one item in
        pfSearchString->SetText( (*stlItems)[0] );
    }

    return iRvl;
}
