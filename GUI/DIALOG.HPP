#ifndef _DIALOG_HPP
#define _DIALOG_HPP

#include <std.h>
#include <graphics.h>
#include "gdef.h"
#include "gui.hpp"
#include "shadow.h"
#include "msgbox.h"
#include "keys.h"
#include <assert.h>
#include <limits.h>

#define Centered -1

#define FI_NODATA INT_MIN
#define FL_NOITEM INT_MIN

#ifndef MAINMODULE
extern int     iScreenWidth, iScreenHeight;
#endif

#ifndef _PTR_DEFINED
typedef void* PTR;
#define _PTR_DEFINED
#endif

#define TRUE 1
#define FALSE 0

class Field;
class Label;
class Dialog;

enum
{
    _daStart = 0x100,

    daContinue,
    daNextItem,
    daPrevItem,
    daExitSave,
    daExitNoSave,
    daRedraw,
    daDefaultButton
};

enum
{
    ltText,
    ltBitmap,
    ltBox,
    ltVertical,
    ltHorizontal
};

enum
{
    ftCheckBox,
    ftCheckBox3,
    ftRadio,
    ftRadio2,
    ftInteger,
    ftString,
    ftButton,
    ftListBox,
    ftRegion
};

enum
{
    csSkip          = 0x01,
    csReadOnly      = 0x02,
    csUpper         = 0x04,
    csMouseButton   = 0x08,
    csInitNoDisp    = 0x10,
    csInvisible     = 0x20,
    csDiscardable   = 0x40,
    csUsesCaret     = 0x80,
    csPromptBeside  = 0x100,    // default is above
    csNoIntegralHeight  = 0x200,
    csCreateAbsSize = 0x400, // Field::Create() is passed pixels
    csDefaultButton = 0x800,
    csAutoSize      = 0x1000,
    csAutoPlace     = 0x2000,
    csPopup         = 0x4000,
    csEraseFirst    = 0x8000,
    csMultiSel      = 0x10000
};

enum
{
    dsStatusBar     = 0x01,
    dsCreateAbsSize = 0x02,
    dsSimple        = 0x04
};

enum
{
    transFromField,
    transToField
};

enum
{
    valOkay,
    valNotOkay
};

enum
{
    etKey,
    etMouse
};

enum
{
    htAbove = 0x01,
    htBelow = 0x02,
    htToLeft = 0x04,
    htToRight = 0x08,
    htInside = 0x10
};

typedef struct
{
    Point       ptPoint;
    int         iHitWhere;
    char        type;
    unsigned    iCode;

} Event_t;

typedef struct
{
    char*   pszName;
    Field** ppField;
    PTR     pData;
    char*   pszMessage;

} VarTransfer_t;

typedef int (* FFUNC)(Field*);
typedef int (* FVFUNC)(long,Field*);
typedef int (* DFUNC)(Dialog*);
typedef int (* DVFUNC)(Dialog*);
typedef int (* FUFUNC)(Field*,Event_t&);

class Label
{
friend Dialog;
public:
    Label( Dialog* pdlgParent, int iType, int iStyle );
    virtual ~Label();

    // initialization
    void Create( const Point& ptPoint, const Size& szSize );
    void CreateAbsSize( const Point& ptPoint, const Size& szSize );

    // implementation
    inline int GetType()
        { return iType; }
    inline void SetName( const char* pszName )
        { strcpy( szName, pszName ); }
    inline const char* GetName()
        { return szName; }
    inline void SetStyle( int iStyle, bool fOnOff )
        { if( fOnOff == TRUE )
            this->iStyle |= iStyle;
          else
            this->iStyle &= ~iStyle; }
    inline int GetStyle()
        { return iStyle; }
    inline bool CanDraw();
    inline Rect GetRectInParent()
        { return Rect( rcDlgPixels ); }

    // draw/erase functions
    void OnDraw();
    void OnErase();

    // draw/erase overrides
    virtual void Draw() = 0;
    virtual void Erase() = 0;

    inline int HitTest( Event_t& event )
    {
        if( rcDlgPixels.IsPtInside( event.ptPoint ) )
            return event.iHitWhere = htInside;
        return 0;
    }

protected:
    // coordinates (in dialog units):
    Point   ptDlgUnits;  // top-left (units)
    Size    szDlgUnits;  // size     (units)
    Rect    rcDlgUnits;  // rect!    (units)

    // coordinates (in dialog pixels):
    Rect    rcDlgPixels; // rectangle

    // misc:
    char    szName[32];
    int     iType;      // fldXXX
    int     iStyle;     // options
    Dialog* pdlgParent; // belongs-to

    // transfer pointer:
    void*   pvTransferBuf;

    // next/prev field
    Label* plbNext;
    Label* plbPrev;
};


class Field
{
friend Dialog;
public:
    Field();
    Field( Dialog* pdlgParent, int iType, int iStyle );
    virtual ~Field();

    // initialization
    void Create( const Point& ptPoint, const Size& szSize );
    void CreateAbsSize( const Point& ptPoint, const Size& szSize );

    // set the viewport to the current field.  you got it.
    void SetViewPortToField();

    // implementation
    void Remove();
    inline int GetType()
        { return iType; }
    inline void SetName( const char* pszName )
        { strcpy( szName, pszName ); }
    inline const char* GetName()
        { return szName; }
    inline void SetStyle( int iStyle, bool fOnOff )
        { if( fOnOff == TRUE )
            this->iStyle |= iStyle;
          else
            this->iStyle &= ~iStyle;
          if( plbPrompt ) plbPrompt->SetStyle( iStyle, fOnOff ); }
    inline int GetStyle()
        { return iStyle; }
    inline void SetTransferBuf( PTR pvTransferBuf )
        { this->pvTransferBuf = pvTransferBuf; }
    inline bool IsInFocus();
    inline bool CanDraw();
    inline Rect ScreenRect();
    inline Rect GetRectInParent()
        { return Rect( rcDlgPixels ); }
    inline void SetMessage( const char* pszMessage )
        { pszStatusMsg = pszMessage; }
    inline void SetDrawDown( bool fDrawDown )
        { if( GetDrawDown() != fDrawDown )
          { this->fDrawDown = fDrawDown;
            OnDraw(); }
        }
    inline bool GetDrawDown()
        { return fDrawDown; }
    inline void SetValMsg( const char* pszValMsg )
        { this->pszValMsg = pszValMsg; }
    inline bool IsFirstDraw();
    inline void SetFirstDraw()
        { fFirstDraw = TRUE; }
    inline Label* GetPrompt()
        { return plbPrompt; }

    // get parent
    inline const Dialog* GetParent()
        { return pdlgParent; }

    inline bool IsInsertMode();
    inline void SetInsertMode( bool fValue = TRUE );

    // set/get modified flag
    inline void SetModified( bool fValue )
        { fModified = fValue; }
    inline int GetModified()
        { return fModified; }

    // set/do help
    inline void SetHelp( const char* pszHelpKey )
        { strcpy( szHelpKey, pszHelpKey ); }
    void Help();

    // sets the function pointers
    inline void SetOnEnter( FFUNC fp )
        { fpEnter = fp; }
    inline void SetOnExit( FFUNC fp )
        { fpExit = fp; }
    inline void SetOnChange( FFUNC fp )
        { fpChange = fp; }
    inline void SetOnValidate( FVFUNC fp )
        { fpValidate = fp; }

    // user events:
    void SetUserEvent( int iType, int iCode, FUFUNC fp );
    int RunUserEvent( Event_t& event );

    // hit test
    int HitTest( Event_t& event );

    // calls the function pointers
    virtual int OnEnter();
    virtual int OnExit();
    virtual int OnChange();
    virtual void OnFocusChange();

    // base functions
    virtual int OnEvent( Event_t& event );
    void OnDraw();
    void OnErase();
    int Validate();

    // overrides
    virtual long GetValidateData() = 0;
    virtual void Draw() = 0;
    virtual void Erase() = 0;
    virtual void Transfer( int iDirection = transFromField ) = 0;
    virtual int Event( Event_t& event );
    virtual void OnTimer( long );
    // if ontimer is overridden, it must call the base OnTimer().

    // user data pointer
    PTR pUserData;

    // coordinates (in dialog pixels):
    Rect    rcDlgPixels; // rectangle

    // transfer pointer:
    void*   pvTransferBuf;

protected:
    // coordinates (in dialog units):
    Point   ptDlgUnits;  // top-left (units)
    Size    szDlgUnits;  // size     (units)
    Rect    rcDlgUnits;  // rect!    (units)

    // misc:
    char    szName[32];
    int     iType;      // fldXXX
    int     iStyle;     // options
    Dialog* pdlgParent; // belongs-to

    // next/prev field
    Field*  pfldNext;
    Field*  pfldPrev;

    // draw/hide caret functions
    void SetCaretPos( Point& ptCaret )
        { ShowCaret( FALSE );   // get rid of it
          rcCaret.SetRect( rcDlgPixels.TopLeft() + ptCaret,
                           szCaret );  // new pos
          ShowCaret( fCaretVisible ); } // show it
    void SetCaretSize( Size& szCaret );
    void ShowCaret( bool fShow = TRUE, long lTimerValue = -1 );
    inline void SetCaretColor( int clrCaret )
        { this->clrCaret = clrCaret; }

    // field prompt:
    Label*  plbPrompt;

    // copy:
    void Replace( Field* pNewField );

private:
    // function pointers:
    FFUNC   fpEnter;    // on enter field
    FFUNC   fpExit;     // on exit field
    FFUNC   fpChange;   // on contents changed
    FVFUNC  fpValidate; // on validate

    // flags:
    //   draw down (buttons, etc):
    bool    fDrawDown;
    //   modified:
    bool    fModified;
    //   drawn yet:
    bool    fFirstDraw;

    // user events:
    struct {
        int iType;
        int iCode;
        FUFUNC fp;
    } UserEvent[16];
    int     nUserEvent;

    // caret variables:
    int     clrCaret;       // caret color, of course
    Rect    rcCaret;        // in client area
    Size    szCaret;        // size of the caret
    char*   pUnderCaret;    // buffer for what's under it
    bool    fCaretVisible;  // caret visible?
    long    lLastCaret;     // last time caret shown

    // status message:
    const char* pszStatusMsg;

    // help keyword:
    char szHelpKey[33];

    // vlaidation message:
    const char* pszValMsg;

    // saved image for popup fields
    char*   pImageSave;
};


/*-------------------------------------------------------------------------*
 * stText  - static text class
 *-------------------------------------------------------------------------*/

class stText : public Label
{
public:
    stText( Dialog* pdlgParent, Point& ptPoint, const char* pszText,
            int iStyle = 0 );

    void Draw();
    void Erase();
    void SetText( const char* pszText );
    void SetColor( int iColor );
    void SetBackColor( int iColor );

private:
    char szText[128];
    int iColor;
    int iBackColor;
};


/*-------------------------------------------------------------------------*
 * stBox  - "combo box" class
 *-------------------------------------------------------------------------*/

class stBox : public Label
{
public:
    stBox( Dialog* pdlgParent, Rect& rcRect, const char* pszText = NULL );

    void Draw();
    void Erase();
};


/*-------------------------------------------------------------------------*
 * stLine  - "line"
 *-------------------------------------------------------------------------*/

class stLine : public Label
{
public:
    stLine( Dialog* pdlgParent, Rect& rcRect, int iLineType = ltVertical );

    void Draw();
    void Erase();

private:
    Rect rcLine;
};


/*-------------------------------------------------------------------------*
 * fldCheckBox  - checkbox field class
 *-------------------------------------------------------------------------*/

class fldCheckBox : public Field
{
public:
    fldCheckBox() : Field() {}
    fldCheckBox( Dialog* pDialog, Point& ptPoint, const char* pszPrompt = NULL,
                 int iStyle = 0, int iType = ftCheckBox );
    inline void SetState( int iState )
        { this->iState = iState;
          OnDraw(); }
    inline int GetState()
        { return iState; }

    long GetValidateData()
        { return (long) GetState(); }
    void Toggle();
    inline void Set3State( bool f3State )
        { iType = f3State ? ftCheckBox3 : ftCheckBox; }
    int Event( Event_t& );
    void Draw();
    void Erase();
    void Transfer( int );
    void Replace( fldCheckBox* pNewField )
        {
            *pNewField = *this;
            Field::Replace( pNewField );
            delete this;
        }

private:
    int iState;     // checked or unchecked
};


/*-----------------ý--------------------------------------------------------*
 * fldString  - string edit field class
 *-------------------------------------------------------------------------*/

class fldString : public Field
{
public:
    fldString() : Field() {}
    fldString( Dialog* pDialog, Point& ptPoint, int iWidth, int iType,
               const char* pszPrompt = NULL, int iStyle = 0,
               int iMaxChars = 80 );
    ~fldString();

    // null means "anything printable."
    inline void SetAllow( const char* pszAllow = NULL )
        { this->pszAllow = pszAllow; }

    void Draw();
    void Erase();
    void Transfer( int );
    int OnEnter();
    int OnExit();
    int Event( Event_t& );
    long GetValidateData();
    inline void SetFirstEdit( bool fFirstEdit )
        { this->fFirstEdit = fFirstEdit; }

    inline const char* GetText()
        { return (const char*) pszEditBuf; }
    void SetText( const char* );
    void SetInteger( int iValue );
    int GetInteger()
        { return atoi( pszEditBuf ); }
    void SetChoices( StrList* stlChoices )
        { this->stlChoices = stlChoices; }
    int GetLength()
        { if( pszEditBuf ) return strlen( pszEditBuf );
          else return 0; } // return length of edit buffer
    void PopChoices();
    void UpdateCaretPos();
    void SetCurPos( int iPos, bool fChunkIt = TRUE,
                    bool fForceUpdate = FALSE );
    bool IsValidInput( unsigned );
    void OnFocusChange() {} // cursor is apparent, don't need focus rect
    void Replace( fldString* pNewField )
        {
            *pNewField = *this;
            Field::Replace( pNewField );
            delete this;
        }

private:
    // functions:
    int OnKey( unsigned uKey );    // process edit keys

    // variables:
    int iMaxChars;      // max chars!
    char* pszEditBuf;   // edit buffer
    int iCurPos;        // current insertion point
    const char* pszAllow;  // "allow buffer"
    int iScrollPos;     // first visible char
    int nCharsVis;      // number visible characters
    bool fFirstEdit;    // is this the first edit made?
    StrList* stlChoices;    // list of string choices
};


/*-------------------------------------------------------------------------*
 * fldButton  - button field class
 *-------------------------------------------------------------------------*/

class fldButton : public Field
{
public:
    fldButton() : Field() {}
    fldButton( Dialog* pDialog, Rect& rcRect, const char* pszCaption,
               FFUNC fp = NULL, int iStyle = 0 );

    inline void SetFunction( FFUNC fp )
        { fpButton = fp; }

    int Event( Event_t& );
    void Draw();
    void Erase();
    void OnFocusChange();
    int OnEnter();
    int OnExit();
    void Transfer( int ) { return; }
    long GetValidateData() { return 0; }
    void SetText( const char* pszCaption );
    void Replace( fldButton* pNewField )
        {
            *pNewField = *this;
            Field::Replace( pNewField );
            delete this;
        }

    int DoButton();

private:
    Size szText;
    char szCaption[64];
    FFUNC fpButton;
};


/*-------------------------------------------------------------------------*
 * fldRadio  - radio field class
 *-------------------------------------------------------------------------*/

class fldRadio : public Field
{
friend Field;
friend Dialog;
public:
    fldRadio() : Field() {}
    fldRadio( Dialog* pDialog, Point& ptPoint, int iGroup,
                const char* pszPrompt = NULL, int iStyle = 0,
                int iType = ftRadio );

    inline int GetState()
        { return pGroupFirst->iState; }
    fldRadio* FindNumInGroup( int iNum, int iGroup = -1 );
    void Set();
    void Draw();
    void Erase();
    void Transfer( int );
    int Event( Event_t& );
    long GetValidateData()
        { return (long) GetState(); }
    inline void SetType2( bool fType2 )
        { iType = fType2 ? ftRadio2 : ftRadio; }
    void Replace( fldRadio* pNewField )
        {
            *pNewField = *this;
            Field::Replace( pNewField );
            delete this;
        }

private:
    inline void SetState( int iState )
        { pGroupFirst->iState = iState; }
    fldRadio *pGroupFirst;  // pointer to first in group
    int iState;     // contains radio # in group that is checked
    int iNum;       // number in group
    int iMaxNum;    // total number in this group
    int iGroup;     // group number, of course
};


/*-------------------------------------------------------------------------*
 * fldListBox  - list box field class
 *-------------------------------------------------------------------------*/

class fldListBox : public Field
{
public:
    fldListBox() : Field() {}
    fldListBox( Dialog* pDialog, Rect& rcRect, const char* pszPrompt = NULL,
                int iStyle = 0 );
    //~fldListBox();
    void SetRect( Rect& rcRect );

    inline void SetItemList( StrList* pslItems )
        { this->pslItems = pslItems; }
    inline StrList* GetItemList()
        { return pslItems; }

    int Event( Event_t& );
    void Draw();
    void DrawItem( int );
    void Erase();
    void SetCurItem( int );
    inline int GetCurItem( void )
        { return iCurItem; }
    void Transfer( int );
    void OnFocusChange();
    void UpdateCaretPos();
    int OnKey( unsigned uKey );    // process edit keys
    void OnTimer( long );
    //void SetItemSel( int iItem, bool fSelected );
    //bool GetItemSel( int iItem );
    inline const char* GetText()
        { return iCurItem < 0 ? "" : (*pslItems)[ iCurItem ]; }
    inline long GetValidateData()
        { return iCurItem < 0 ? (long) "" : (long) (*pslItems)[ iCurItem ]; }
    inline void ReInit()
        { iLastTop = iLastCur = -1;
          SetFirstDraw(); }
    void SelectString( const char* pszString );
    void Replace( fldListBox* pNewField )
        {
            *pNewField = *this;
            Field::Replace( pNewField );
            delete this;
        }

private:
    StrList* pslItems;
    bool *pItemSel;
    int iTopItem;
    int iCurItem;
    int nVisItems;
    int iLastTop;
    int iLastCur;
    char szSearch[80];
    int iSearchLen;
    Rect rcCurItemRect;
    Point ptTopItem;
    bool fScrolling;    // mouse scrolling?
    long lLastScroll;   // last time scrolled
};


/*-------------------------------------------------------------------------*
 * fldRegion  - general purpose region
 *-------------------------------------------------------------------------*/

class fldRegion : public Field
{
public:
    fldRegion() : Field() {}
    fldRegion( Dialog* pDialog, Rect& rcRect, int iStyle = 0 );

    void SetFillColor( int clrFill )
        { this->clrFill = clrFill; }
    void SetRectStyle( unsigned uFlags )
        { this->uRectFlags = uFlags; }
    virtual void Draw();
    virtual void Erase();
    virtual void Transfer( int ) {}
    long GetValidateData()
        { return 0; }
    virtual int Event( Event_t& );
    void Replace( fldRegion* );

private:
    int clrFill;
    unsigned uRectFlags;
};


/*-------------------------------------------------------------------------*
 * Dialog class
 *-------------------------------------------------------------------------*/

class Dialog
{
public:
    Dialog();
    ~Dialog();

    void Create( const Point& ptTopLeft, const Size& szDlgUnits,
                 const char* pszTitle, unsigned uFlags = 0 );
    bool Create( const char* pszScript, const char* pszName );

    int Process( int iStartItem = 0 );

    void Draw();
    int Read( int iStartItem = 0 );
    void Erase();

    // first / current fields
    Field*  pfldFirst;
    Field*  pfldCurrent;

    Label*  plbFirst;

    void GoToItem( int iItem );
    void GoToItem( Field* pField );
    Field* GetField( const char* pszName );
    Label* GetLabel( const char* pszName );
    void DrawTitleBar();
    void DrawStatusBar( const char* pszMessage );
    void DrawLabels();
    void DrawFields();
    void DrawTextXY( Point&, const char*, bool fDrawAccel = FALSE,
            int iXOffset = 0, int iYOffset = 0 );
    void ShadowRect( Rect& rcRect, unsigned uFlags, int clrInterior = -2 );
    void FillRect( Rect& rcRect, int clrInterior = -2 );
    void FrameRect( Rect& rcRect, int clrBorder = -2 );
    void FocusRect( Rect& rcRect, int clrColor = -1, bool fExpand = TRUE );
    void EraseTextXY( Point&, const char* );
    void SetActive();
    void SetRadioGroup( int, int );
    void DrawDefaultAs( bool );
    void Transfer( int );
    void AddDefaultButtons();
    bool VarSetup( VarTransfer_t* pVT );
    void MoveTo( Point &ptPoint );
    int Validate();
    inline void SetMouseCapture( Field* pField )
        { pfldMouseCapture = pField; }
    inline Field* GetMouseCapture()
        { return pfldMouseCapture; }
    inline bool IsVisible()
        { return fIsVisible; }
    inline const Rect& ClientRect()
        { return rcClient; }
    inline const Size& ClientSize()
        { return szClientUnits; }
    inline Rect ScreenRect()
        { return Rect( rcScreen ); }
    inline Rect ScreenClientRect()
        { return Rect( rcScreenClient ); }
    inline bool IsFirstDraw()
        { return fFirstDraw; }
    inline void SetTitle( const char* pszTitle )
        { strcpy( szTitle, pszTitle );
          DrawTitleBar(); }
    inline void SetMultiEdit( bool fValue )
        { fMultiEdit = fValue; }
    inline bool GetMultiEdit()
        { return fMultiEdit; }

    // set function pointers
    inline void SetOnEnter( DFUNC fp )
        { fpEnter = fp; }
    inline void SetOnExit( DFUNC fp )
        { fpExit = fp; }
    inline void SetOnSave( DFUNC fp )
        { fpSave = fp; }
    inline void SetOnNoSave( DFUNC fp )
        { fpNoSave = fp; }
    inline void SetOnValidate( DVFUNC fp )
        { fpValidate = fp; }
    inline void SetNextAct( long lAct )
        { lNextAct = lAct; }

    Field* EnumFields( Field* );
    Label* EnumLabels( Label* );

    long lTimerValue;
    bool fInsertMode;

    fldListBox* pChoiceListBox;
    fldButton* pDefaultButton;

    // one-call function to replace a standard region field (pszName)
    // with a pointer to a new region (usually the base of a derived
    // class) - pNewRgn.
    bool REPLACEREGION( const char* pszName, fldRegion* pNewRgn );

    bool SETBUTTONFUNC( const char* pszName, FFUNC fp );

private:
    // private member functions
    int _ReadLoop();    // input loop

private:
    // title:
    char    szTitle[128];

    // functions:
    DFUNC   fpExit;
    DFUNC   fpEnter;
    DFUNC   fpSave;
    DFUNC   fpNoSave;
    DVFUNC  fpValidate;

    // coordinates:
    Rect    rcScreen;
    Rect    rcScreenClient;
    Rect    rcClient;   // client - offset from top, left of rcScreen
    Rect    rcTitle;    // title - offset from top, left of rcScreen
    Rect    rcStatus;   // status messages - offset from etc etc
    Size    szClientUnits;  // size of client area in units

    // save buffer:
    char*   pImageSave;

    // previous active Dialog:
    Dialog* pdlgPrev;

    // misc:
    bool    fMultiEdit; // for editing non-similar data
    bool    fIsVisible;
    const   char* pszLastStatusMsg;   // so we don't do it again..
    Field*  pfldMouseCapture;
    bool    fFirstDraw; // first draw here?
    bool    fStatusBar; // status bar?
    long    lNextAct;   // -1 if nothing
};

/*-------------------------------------------------------------------------*
 * button ok/cancel functions
 *-------------------------------------------------------------------------*/

int pbOkay( Field* );
int pbCancel( Field* );


/*-------------------------------------------------------------------------*
 * Class functions
 *-------------------------------------------------------------------------*/

inline bool Field::IsInFocus()
{
    return (pdlgParent->pfldCurrent) == this;
}

inline bool Field::IsInsertMode()
{
    return pdlgParent->fInsertMode;
}

inline void Field::SetInsertMode( bool fValue )
{
    pdlgParent->fInsertMode = fValue;
}

inline bool Field::CanDraw()
{
    return (pdlgParent->IsVisible() &&
           !(GetStyle() & csInvisible));
}

inline bool Field::IsFirstDraw()
{
    return pdlgParent->IsFirstDraw() || fFirstDraw;
}

inline bool Label::CanDraw()
{
    return (pdlgParent->IsVisible() &&
           !(GetStyle() & csInvisible));
}

inline Rect Field::ScreenRect()
{
    Rect rcRect = rcDlgPixels;
    rcRect.MoveRel( pdlgParent->ScreenClientRect().TopLeft() );
    return rcRect;
}

#endif // _DIALOG_HPP
