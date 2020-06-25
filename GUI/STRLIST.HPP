#ifndef _STRLIST_HPP
#define _STRLIST_HPP

#include <string.h>

#ifndef bool
#define bool unsigned char
#endif

typedef struct
{
    char *pszText;      // pointer to text
    int  iIdent;        // id code for string

} Str;

class StrList
{
public:
    StrList( int iMaxItems, int iMaxWidth, const char* pszName = NULL );
    StrList( StrList* pStrList );
    ~StrList();

    const char* GetName()
        { return szName; }

    // modification routines MUST NOT be called when keyword
    // is active ..
    void AddTail( const char* pszText, int iIdent = -1 );
    void SetAt( int iIndex, const char* pszText, int iIdent = -1 );

    void Delete( int iIndex );
    void DeleteAll();

    // the only ones that use idents for searching:
    char *IdToStr( int iIdent, char* pszText = NULL );
    int StrToId( const char* pszText );
    int IdToIndex( int iIdent );

    // get index/strings:
    int StrToIndex( const char* pszText );
    char* GetAt( char* pszText, int iIndex, int* ipID = NULL ); // keyword
    const char* operator[]( int );  // keyword

    // counts:
    inline int GetCount()
        { if( iKwdOfs != -1 )
            return nItemKwd;
          return nItem; }
    inline int GetCountNoKey()
        { return nItem; }

    // searching:
    int Find( const char* pszText, int iSearchLen = -1 );
    int FindNoKey( const char* pszText, bool fPartial = 1 );

    // keyword/sort:
    void SetKey( const char* pszKey );

    void PauseKey()
        { if( Save_iKwdOfs == -1 )
          { Save_iKwdOfs = iKwdOfs;
            Save_nItemKwd = nItemKwd;
            iKwdOfs = -1;
            nItemKwd = -1; }
        }

    void ResumeKey()
        { if( Save_iKwdOfs != -1 )
          { iKwdOfs = Save_iKwdOfs;
            nItemKwd = Save_nItemKwd;
            Save_iKwdOfs = -1; }
        }

    void Sort();

private:
    int  _FirstItem()
        { if( iKwdOfs != -1 )
            return iKwdOfs;
          return 0;
        }   // uses Keywords

    int  _LastItem()        // uses Keywords
        { if( iKwdOfs != -1 )
            return iKwdOfs + nItemKwd;
          return nItem-1;
        }

    char szName[40];        // name of table
    int  nItem;             // number of items
    int  nMaxItem;          // maximum number of items
    int  nMaxCol;           // maximum length of each item
    int  iKwdOfs;           // keyword index offset
    int  nItemKwd;          // number of items in keyword section
    Str  *pStrHead;         // first string
    char *pChunk;           // chunk of strings
    bool fCopy;             // copy of another?

    int  Save_iKwdOfs;      // for PauseKey and ResumeKey
    int  Save_nItemKwd;
};


#endif // _STRLIST_HPP
