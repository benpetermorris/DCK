/*-------------------------------------------------------------------------*
 * PickDialog definition.
 *-------------------------------------------------------------------------*/

typedef void (*CHFUNC)(const char*,StrList*);

class PickDialog : public Dialog
{
public:
    PickDialog( const char*, StrList*, StrList*, CHFUNC );
    ~PickDialog();
    int Process( const char*, bool fShowEntireFirst = FALSE );
    const char* GetSelText()
        { return pfItems->GetText(); }
    bool FindWithKeys( const char* pszKeyText, bool fCopyAll = FALSE );
    void OnItemChanged();
    bool IsUsingCats() { return fUseCats; }

    fldListBox  *pfCategories, *pfItems;
    fldString   *pfSearchString;

    const char* FindItemCategory( int iPosInList, int *ipCat );
    StrList     *stlCategories, *stlItems;
    StrList     *stlFoundItems;
    StrList     *stlAllItems;
    CHFUNC      fpOnChange;
    bool        fUseCats;
};
