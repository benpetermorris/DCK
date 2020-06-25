#include <graphics.h>
#include "gdef.h"
#include "strlist.hpp"
#include "coords.hpp"
#include "inpdev.h"
#include "clr.h"

typedef struct
{
    char szKeyword[33];
    long lFileOffset;
    int  nLines;

} HelpKey;

class PopHelp
{
public:
    PopHelp();
    ~PopHelp();

    bool Init( const char* pszHelpFile );
    void Display( const char* pszHelpKey, const char* pszMoreHelp = NULL );
    void Draw();
    void Erase();
    void SetActive( bool );

    // simple functions:
    inline bool IsPtInside( Point& ptPoint )
        { return rcScreenRect.IsPtInside( ptPoint ); }
    void AddAvoidRect( Rect& rcRect );
    inline void RemoveAvoidRect()
        { nAvoid--; }
    void ClearAvoidRects()
        { nAvoid = 0; }

private:
    bool LoadText( const char* );

    Rect rcScreenRect;  // screen rectangle of current help box
    char *pImageSave;   // underlying image for rectangle
    char *pText;        // text to go in rectangle
    const char *pszShowHelpKey;
    const char *pszCurHelpKey;    // current help key
    int iTextSize;      // sizen of texten

    Rect rcAvoid[10];   // screen rectangles to avoid cluttering
    int  nAvoid;        // number of above

    bool fMoreHelp;     // display "more help" message?
    bool fVisible;      // box up?
    bool fInit;         // initialized?
    bool fActive;       // active?  (display stuff?)

    // help file indexing:
    int  nHelpKeys;
    HelpKey *pHelpKeys;
    FILE *pFp;          // guess.
};
