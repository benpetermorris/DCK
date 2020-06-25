/*-------------------------------------------------------------------------*
 * MOTIF.C:
 * Motifs!
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "fields.hpp"

/*-------------------------------------------------------------------------*
 *
 * Data begins here
 *
 *-------------------------------------------------------------------------*/

#define dMaxStr     32
#define dNDataType  2

static  short   nMotifCmd = 11;
extern  short   motif_num;      /* dck.c -- used for status save/restore */

char    *motif_name = "";       /* current motif name for display on sbar */

void save_motifs( void );
void load_motifs_fn( uchar *fn );
void load_motifs( void );

enum    /* data type identfiers */
{
    DT_Str,
    DT_Num
};

enum    /* display type identifiers */
{
    DD_None,        /* no display past data information */
    DD_Wall,        /* display wall texture */
    DD_Flat         /* display flat texture */
};

typedef struct
{
    uchar   *dName;         /* description of item */
    short   dType;          /* from DT_xxx */
    short   dDisplay;       /* from DD_xxx */

} mDataType;

enum
{
    MD_TexUpper,
    MD_TexNormal,
    MD_TexLower,
    MD_TexStair,
    MD_DoorBig,
    MD_DoorSmall,
    MD_DoorTrak,
    MD_LiftFloor,
    MD_FloorTex,
    MD_CeilTex,
    MD_LightNormal
};

mDataType mDataTable[] =
{
    { "Upper texture", DT_Str, DD_Wall },
    { "Normal texture", DT_Str, DD_Wall },
    { "Lower texture", DT_Str, DD_Wall },
    { "Stair texture", DT_Str, DD_Wall },
    { "Door lines longer than 64", DT_Str, DD_Wall },
    { "Door lines shorter than 65", DT_Str, DD_Wall },
    { "Door internal lines", DT_Str, DD_Wall },
    { "Lift floor", DT_Str, DD_Flat },
    { "Normal floor", DT_Str, DD_Flat },
    { "Normal ceiling", DT_Str, DD_Flat },
    { "Light normal", DT_Num, DD_None },

    { NULL, -1, -1 }
};

typedef struct
{
    uchar   dStr[dMaxStr];  /* string data */
    short   dNum;           /* number data */

} mDataRecord;

typedef struct
{
    uchar       name[64];
    mDataRecord *data;

} Motif;

/*-------------------------------------------------------------------------*
 *
 * Code begins here
 *
 *-------------------------------------------------------------------------*/

Motif *mtInit( void )
{
    Motif   *mptr = (Motif*) memget( sizeof( Motif ) );
    mptr->data = (mDataRecord*) memget( sizeof( mDataRecord ) * nMotifCmd );

    return mptr;
}

void mtFree( Motif *mptr )
{
    memfree( mptr->data );
    memfree( mptr );
}

void mtCopy( Motif *dest, Motif *src )
{
    if( !src || !dest )
        return;

    strcpy( dest->name, src->name );
    memcpy( dest->data, src->data, sizeof( mDataRecord ) * nMotifCmd );
}

void* mtGetData( Motif *m, short dataid, void *buf )
/*-------------------------------------------------------------------------*
 * gets data from a motif into a buffer!
 *-------------------------------------------------------------------------*/
{
    static char staticbuf[128];

    if( buf == NULL )
        buf = staticbuf;

    switch( mDataTable[dataid].dType )
    {
    case DT_Str:
        strcpy( (uchar*) buf, m->data[dataid].dStr );
        break;
    case DT_Num:
        ((short*)buf)[0] = m->data[dataid].dNum;
        break;
    }

    return buf;
}

/*-------------------------------------------------------------------------*
 *
 * Dialog functions
 *
 *-------------------------------------------------------------------------*/

static  Dialog*     pMeditDlg;          /* motif dialog */
static  mDataType*  mdp;            /* data type information pointer */
static  short       md_curnum;      /* current motif data type */
static  Motif*      meditp;         /* edit this motif */
static  fldWallTex* pfWall;
static  fldFlatTex* pfFlat;
static  fldString*  pfData;       /* data field pointer for current data */
static  fldListBox* pfFieldList;       /* list of motif data */
static  fldString*  editflds[5];    /* different edit fields for different edit types */

static int _mdDataUp();
static int _mdDataDown();

int _ffShowChangedTex( fldString* pString )
{
    if( pString->pUserData )
    {
        fldWallTex* pfTexture = (fldWallTex*) (pString->pUserData);
        pfTexture->SetWall( pString->GetText() );
    }
    return daContinue;
}

int _ffTexBrowser( fldString* pString )
{
    int iSel = texture_toint( (char*) pString->GetText() );
    if( iSel == -1 )
        iSel = 0;
    if( TexBrowser( &iSel, FALSE ) )
    {
        char szBuf[128];
        texture_tostr( szBuf, iSel );
        pString->SetText( szBuf );
        _ffShowChangedTex( pString );
    }
    return daContinue;
}

void MakeWallField( Field* pField )
{
    pField->SetUserEvent( etKey, KEY_F2, (FUFUNC) _ffTexBrowser );
    pField->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _ffTexBrowser );
    pField->SetOnChange( (FFUNC) _ffShowChangedTex );
}

static int _ffShowChangedFlat( fldString* pString )
{
    if( pString->pUserData )
    {
        fldFlatTex* pfFlat = (fldFlatTex*) (pString->pUserData);
        pfFlat->SetFlat( pString->GetText() );
    }
    return daContinue;
}

static int _ffFlatBrowser( fldString* pString )
{
    int iSel = floor_toint( (char*) pString->GetText() );
    if( iSel == -1 )
        iSel = 0;
    if( TexBrowser( &iSel, TRUE ) )
    {
        pString->SetText( WN2Str( floorlist[iSel].name ) );
        _ffShowChangedFlat( pString );
    }
    return daContinue;
}

void MakeFlatField( Field* pField )
{
    pField->SetUserEvent( etKey, KEY_F2, (FUFUNC) _ffFlatBrowser );
    pField->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) _ffFlatBrowser );
    pField->SetOnChange( (FFUNC) _ffShowChangedFlat );
}

static int _mdDataOut()
/*-------------------------------------------------------------------------*
 * Called when the user exits the data field - updates the display.
 *-------------------------------------------------------------------------*/
{
    switch( md_curnum )
    {
    case MD_TexNormal:
    case MD_TexUpper:
    case MD_TexLower:
    case MD_TexStair:
    case MD_DoorBig:
    case MD_DoorSmall:
    case MD_DoorTrak:
        pfWall->SetWall( pfData->GetText() );
        break;
    case MD_FloorTex:
    case MD_CeilTex:
    case MD_LiftFloor:
        pfFlat->SetFlat( pfData->GetText() );
        break;
    case MD_LightNormal:
        break;
    }

    return TRUE;
}

static void _mdSaveData( void )
/*-------------------------------------------------------------------------*
 * Saves the data entered into the field into the current motif
 *-------------------------------------------------------------------------*/
{
    if( mdp == NULL )
        return;

    switch( mdp->dType )
    {
    case DT_Str:
        strcpy( meditp->data[md_curnum].dStr, pfData->GetText() );
        break;
    case DT_Num:
        meditp->data[md_curnum].dNum = pfData->GetInteger();
        break;
    }
}

static void _mdGetData( void )
/*-------------------------------------------------------------------------*
 * Gets data
 *-------------------------------------------------------------------------*/
{
    if( mdp == NULL )
        return;

    switch( mdp->dType )
    {
    case DT_Str:
        pfData->SetText( meditp->data[md_curnum].dStr );
        break;
    case DT_Num:
        pfData->SetInteger( meditp->data[md_curnum].dNum );
        break;
    }
}

static int _mdChangeField()
/*-------------------------------------------------------------------------*
 * This is called as the user scrolls around the "item to edit" list.
 * It changes the active data field (if necessary) by hiding the old one
 * and unhiding the new one.
 * It modifies the active data field to respond to certain keypresses
 * (depending on the type): for example, F2 to browse textures from
 * one of the texture fields.
 *-------------------------------------------------------------------------*/
{
    short       newitem = pfFieldList->GetCurItem(); /* get new item type number */
    short       need_erase;
    mDataType   *newmdp = &mDataTable[newitem];
    fldString   *newfldp = editflds[newmdp->dType];

    _mdSaveData();  /* save current data */

    if( mdp != NULL )
    {
        need_erase = (mdp->dDisplay != newmdp->dDisplay);

        /* check the current mdp against the new one: if they're different
         * input types (string integer etc), hide the old and unhide the new */
        if( mdp->dType != newmdp->dType )
        {
            pfData->OnErase();
            pfData->SetStyle( csInvisible | csSkip, ON );
        }

        if( pfData != newfldp && pMeditDlg->pfldCurrent != pfFieldList )
        {
            /* need to set this to the go item */
            pMeditDlg->GoToItem( newfldp );
        }
    }
    else
        need_erase = 1;

    pfData = newfldp;
    mdp = newmdp;
    md_curnum = newitem;

    _mdGetData();

    pfData->SetStyle( csInvisible | csSkip, OFF );
    pfData->SetOnExit( (FFUNC) _mdDataOut );
    pfData->SetFirstEdit( TRUE );
    pfData->SetCurPos( strlen( pfData->GetText() ), TRUE, TRUE );
    pfData->SetUserEvent( etKey, KEY_UP, (FUFUNC) _mdDataUp );
    pfData->SetUserEvent( etKey, KEY_DOWN, (FUFUNC) _mdDataDown );
    pfData->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) _mdChangeField );

    switch( mdp->dDisplay )
    {
    case DD_Wall:
        MakeWallField( pfData );
        pfData->pUserData = (PTR) pfWall;
        if( !(pfFlat->GetStyle() & csInvisible) )
        {
            pfFlat->OnErase();
            pfFlat->SetStyle( csInvisible | csSkip, ON );
        }
        pfWall->SetStyle( csInvisible, OFF );
        pfWall->OnDraw();
        break;
    case DD_Flat:
        MakeFlatField( pfData );
        pfData->pUserData = (PTR) pfFlat;
        if( !(pfWall->GetStyle() & csInvisible) )
        {
            pfWall->OnErase();
            pfWall->SetStyle( csInvisible | csSkip, ON );
        }
        pfFlat->SetStyle( csInvisible, OFF );
        pfFlat->OnDraw();
        break;
    case DD_None:
        pfFlat->OnErase();
        pfWall->OnErase();
        pfWall->SetStyle( csInvisible | csSkip, ON );
        pfFlat->SetStyle( csInvisible | csSkip, ON );
        pfData->pUserData = (PTR) NULL;
        break;
    }

    _mdDataOut();

    return daContinue;
}

static int _mdDataUp()
{
    pfFieldList->SetCurItem( pfFieldList->GetCurItem() - 1 );
    _mdChangeField();
    return daContinue;
}

static int _mdDataDown()
{
    pfFieldList->SetCurItem( pfFieldList->GetCurItem() + 1 );
    _mdChangeField();
    return daContinue;
}

static int _mdUp()
{
    _mdChangeField();
    return daContinue;
}

short motif_edit( short x, short y, Motif *mptr )
/*-------------------------------------------------------------------------*
 * Edits one motif.
 *-------------------------------------------------------------------------*/
{
    StrList     *stlFieldNames = new StrList( nMotifCmd, 30 );
    short       i;
    fldString   *pfName;

    if( !meditp )   /* dummy data */
        meditp = mtInit();
    mtCopy( meditp, mptr );

    /* create a description list for the dialog box */
    for( i = 0; i < nMotifCmd; i++ )
        stlFieldNames->AddTail( mDataTable[i].dName );

    /* initialize these to NULL */
    mdp = NULL;
    pfData = NULL;

    VarTransfer_t vtMotifEdit[] =
    {
        { "Name", (Field**) &pfName, NULL, NULL },
        { "FieldList", (Field**) &pfFieldList, NULL, NULL },
        { "ItemString", (Field**) &editflds[DT_Str], NULL, NULL },
        { "ItemInteger", (Field**) &editflds[DT_Num], NULL, NULL },

        // end marker
        { NULL, NULL, NULL, NULL }
    };

    pMeditDlg = new Dialog;
    pMeditDlg->Create( szDSFile, "MotifEditDlg" );
    pMeditDlg->SetOnEnter( (DFUNC) _mdUp );
    pMeditDlg->VarSetup( vtMotifEdit );

    Rect rcRect = pMeditDlg->ScreenRect();
    pMeditDlg->MoveTo( rcRect.TopLeft() + Point( 50, 80 ) );

    pfWall = new fldWallTex;
    pfFlat = new fldFlatTex;
    pMeditDlg->REPLACEREGION( "RgnWall", pfWall );
    pMeditDlg->REPLACEREGION( "RgnFlat", pfFlat );

    pfFieldList->SetItemList( stlFieldNames );
    pfFieldList->SetOnChange( (FFUNC) _mdChangeField );
    pfName->SetTransferBuf( (PTR) meditp->name );

    int iRvl = pMeditDlg->Process();

    _mdSaveData();

    delete pMeditDlg;

    if( iRvl == daExitSave )
        mtCopy( mptr, meditp );

    delete stlFieldNames;

    return (iRvl == daExitSave);
}

/*-------------------------------------------------------------------------*
 *
 * Motif list editor
 *
 *-------------------------------------------------------------------------*/

NODEPTR motif_list;

static fldListBox* pfMotifList;
static StrList *stlMotifs;
static Dialog* pMotifDlg;
static short motif_changes, newmotif_num;

static int pbMotifNew()
/*-------------------------------------------------------------------------*
 * Add a new motif!
 *-------------------------------------------------------------------------*/
{
    Motif *mdp = mtInit();

    /* copy current motif, if any */
    if( pfMotifList->GetCurItem() > -1 )
        mtCopy( mdp, (Motif*) NodeNumtoPtr( pfMotifList->GetCurItem(), motif_list )->data );
    strcpy( mdp->name, "New Motif" );

    /* init a new motif, edit it, add it. */
    while( 1 )
    {
        Point ptPoint = pMotifDlg->ScreenRect().TopLeft();

        if( !motif_edit( ptPoint.x+20, ptPoint.y+20, mdp ) )
        {
            mtFree( mdp );  /* aborted */
            return daContinue;
        }
        else if( !mdp->name[0] )
        {
            message_box( MB_OK, st_dcksays,
             "I think you should name this motif!" );
        }
        else
            break;
    }

    motif_changes = 1;

    /* add it to the list now */
    NodeInsert( NodeLast( motif_list ), 0 )->data = mdp;
    stlMotifs->AddTail( mdp->name );
    pfMotifList->ReInit();
    pfMotifList->SetCurItem( stlMotifs->GetCount() - 1 );

    return daContinue;
}

static int pbMotifEdit()
/*-------------------------------------------------------------------------*
 * Edit an existing motif.
 *-------------------------------------------------------------------------*/
{
    NODEPTR ndp = NodeNumtoPtr( pfMotifList->GetCurItem(), motif_list );
    Motif* mdp;

    mdp = (Motif*) ndp->data;

    if( pfMotifList->GetCurItem() < 0 || !mdp || !ndp )
        return pbMotifNew();

    /* init a new motif, edit it, add it. */
    while( 1 )
    {
        Point ptPoint = pMotifDlg->ScreenRect().TopLeft();

        if( !motif_edit( ptPoint.x+20, ptPoint.y+20, mdp ) )
        {
            /* aborted */
            return daContinue;
        }
        else if( !mdp->name[0] )
        {
            message_box( MB_OK, st_dcksays,
             "I think you should name this motif!" );
        }
        else
            break;
    }

    motif_changes = 1;

    /* add it to the list now */
    pfMotifList->ReInit();
    stlMotifs->SetAt( pfMotifList->GetCurItem(), mdp->name );
    pfMotifList->OnDraw();

    return daContinue;
}

static int pbMotifImport()
/*-------------------------------------------------------------------------*
 * Imports a list of motifs from another MTF file.
 *-------------------------------------------------------------------------*/
{
    uchar   fn[128];
    Motif*  mdp;
    NODEPTR ndp = NodeLast( motif_list );

    *fn = 0;

    if( !file_get( FG_LOAD | FG_NOMAPS, fn, "MTF" ) )
        return daContinue;

    load_motifs_fn( fn );   /* adds them to the list */

    /* loop through from the new motifs and add 'em to the dbox list */
    ndp = NodeNext( ndp );
    int nNewMotifs = 0;
    while( ndp != motif_list )
    {
        mdp = (Motif*) ndp->data;
        stlMotifs->AddTail( mdp->name );
        ndp = NodeNext( ndp );
        ++nNewMotifs;
    }

    message_box( MB_OK, st_dcksays, "%d motifs were imported.", nNewMotifs );

    motif_changes = 1;

    pfMotifList->ReInit();
    pfMotifList->OnDraw();

    return daContinue;
}

static int pbMotifRemove()
/*-------------------------------------------------------------------------*
 * Take a guess!
 *-------------------------------------------------------------------------*/
{
    short   item = pfMotifList->GetCurItem();
    NODEPTR ndp = NodeNumtoPtr( item, motif_list );

    if( !item && stlMotifs->GetCount() == 1 )
    {
        message_box( MB_OK, st_dcksays,
          "You can't delete the only motif in the list." );
        return daContinue;
    }

    /* nothing to delete */
    if( !ndp->data )
        return daContinue;

    if( message_box( MB_YESNO, st_dcksays,
        "Are you sure you want to delete this motif?" ) == MB_NO )
        return daContinue;

    motif_changes = 1;

    /* delete it from the list and the stringtable */
    NodeDelete( ndp );
    stlMotifs->Delete( item );
    /* redisplay the field */
    pfMotifList->ReInit();
    pfMotifList->SetCurItem( item );

    /* little bit of processing.. change newmotif_num based on the motif
     * deleted.  if the deleted is greater than newmotif_num, leave it.
     * if the deleted is less than newmotif_num, decrement newmotif_num.
     * if the deleted IS newmotif_num, reset it to the default (0.) */
    if( newmotif_num > item )
        --newmotif_num;    /* but no need to redisplay it */
    if( newmotif_num == item )
    {
        newmotif_num = 0;
        pfMotifList->SetCurItem( 0 );
        pfMotifList->SetCurItem( item );
    }

    return daContinue;
}

void _textest( short tex, char *texname, char *motname )
{
    if( tex < 1 )
    {
        message_box( MB_OK, st_dcksays,
          "The texture '%s' is invalid (used in the motif\n"
          "'%s'.)", texname, motname );
    }
}

void motif_set( short mno )
{
    Motif*  mptr;
    char    *tp;

    if( mno < 0 )
        mno = 0;

    mptr = (Motif*) (NodeNumtoPtr( mno, motif_list )->data);

    if( !mptr )
    {
        mptr = (Motif*) (NodeNumtoPtr( 0, motif_list )->data);
        if( !mptr )
        {
            if( message_box( MB_YESNO, st_dcksays,
              "There are no motifs defined!  You have to create one\n"
              "before you can use "_ABBREV_".  Do you want to now?" ) == MB_YES )
                motif_pick();
            else
                error( "fine, be that way." );
            return;
        }
    }

    motif_num = mno;
    motif_name = mptr->name;

    _def_ftxidx = texture_toint( tp = (char*) mtGetData( mptr, MD_TexNormal, NULL ) );
    _textest( _def_ftxidx, tp, motif_name );
    _def_utxidx = texture_toint( tp = (char*) mtGetData( mptr, MD_TexUpper, NULL ) );
    _textest( _def_utxidx, tp, motif_name );
    _def_ltxidx = texture_toint( tp = (char*) mtGetData( mptr, MD_TexLower, NULL ) );
    _textest( _def_ltxidx, tp, motif_name );
    _def_stairtxidx = texture_toint( tp = (char*) mtGetData( mptr, MD_TexStair, NULL ) );
    _textest( _def_stairtxidx, tp, motif_name );
    _def_doorbig = texture_toint( tp = (char*) mtGetData( mptr, MD_DoorBig, NULL ) );
    _textest( _def_doorbig, tp, motif_name );
    _def_doorsmall = texture_toint( tp = (char*) mtGetData( mptr, MD_DoorSmall, NULL ) );
    _textest( _def_doorsmall, tp, motif_name );
    _def_doortrak = texture_toint( tp = (char*) mtGetData( mptr, MD_DoorTrak, NULL ) );
    _textest( _def_doortrak, tp, motif_name );
    wncpy( _def_ceiltex, (char*) mtGetData( mptr, MD_CeilTex, NULL ) );
    wncpy( _def_floortex, (char*) mtGetData( mptr, MD_FloorTex, NULL ) );
    wncpy( _def_liftfloor, (char*) mtGetData( mptr, MD_LiftFloor, NULL ) );
    mtGetData( mptr, MD_LightNormal, &_def_light );
}

static int dfMotifListUp()
{
    pfMotifList->SetCurItem( newmotif_num );
    return daContinue;
}

void motif_pick( void )
/*-------------------------------------------------------------------------*
 * Allows the user to pick from / edit the list of motifs.
 * Returns the motif the user was on when F10 was pressed, or -1 for no
 * motif.
 *-------------------------------------------------------------------------*/
{
    short       i, rvl;
    static      char cmtext[128];
    NODEPTR     ndp;
    Motif       *mdp;

    cmtext[0] = 0;

    motif_changes = 0;
    newmotif_num = motif_num;

    VarTransfer_t vtMotifList[] =
    {
        { "MotifList", (Field**) &pfMotifList, NULL, NULL },
        { "BtnCreate", NULL, (PTR) pbMotifNew, NULL },
        { "BtnRemove", NULL, (PTR) pbMotifRemove, NULL },
        { "BtnEdit", NULL, (PTR) pbMotifEdit, NULL },
        { "BtnImport", NULL, (PTR) pbMotifImport, NULL },

        // end marker
        { NULL, NULL, NULL, NULL }
    };

    /* init dialog box */
    pMotifDlg = new Dialog;
    pMotifDlg->Create( szDSFile, "MotifListDlg" );
    pMotifDlg->VarSetup( vtMotifList );
    pMotifDlg->SetOnEnter( (DFUNC) dfMotifListUp );

    stlMotifs = new StrList( 64, 60 );

    /* add motifs into list */
    ndp = NodeFirst( motif_list );

    while( ndp != motif_list )
    {
        mdp = (Motif*) ndp->data;
        stlMotifs->AddTail( mdp->name );
        ndp = NodeNext( ndp );
    }

    // add user functions for motif list
    pfMotifList->SetUserEvent( etMouse, MOUSE_LDBLCLICK, (FUFUNC) pbOkay );
    // pfMotifList->SetUserEvent( etKey, KEY_ENTER, (FUFUNC) pbOkay );
    pfMotifList->SetUserEvent( etKey, 'E', (FUFUNC) pbMotifEdit );
    pfMotifList->SetUserEvent( etKey, KEY_INS, (FUFUNC) pbMotifNew );
    pfMotifList->SetUserEvent( etKey, KEY_DEL, (FUFUNC) pbMotifRemove );
    pfMotifList->SetItemList( stlMotifs );

    int iRvl = pMotifDlg->Process();
    delete pMotifDlg;

    if( motif_changes && message_box( MB_YESNO, st_dcksays,
        "Do you want to save the changes you made?" ) == MB_YES )
        save_motifs();
    else if( motif_changes )
    {
        /* load the old motif list */
        load_motifs();
        motif_set( motif_num );
        iRvl = daExitNoSave;        /* no set new motif.. */
    }

    if( iRvl == daExitSave )
    {
        /* set the motif */
        motif_set( pfMotifList->GetCurItem() );
    }

    delete stlMotifs;
}

void save_motifs( void )
/*-------------------------------------------------------------------------*
 * Guess what this does, honeybunch.
 *
 * The file structure is simple:
 *  50      header
 *  word    number of entries per motif
 *  Data!
 *-------------------------------------------------------------------------*/
{
    NODEPTR ndp;
    Motif*  mptr;
    FILE    *fp;
    uchar   fn[128];
    short   nmotif = 0;

    sprintf( fn, "%s.MTF", pgdCurGame->cfgname );
    fp = fopen( fn, "wb" );

    ndp = NodeFirst( motif_list );

    fwrite( _ABBREV_" Motif Data File\r\n\x1a", 50, 1, fp );
    fwrite( &nmotif, sizeof nmotif, 1, fp );
    fwrite( &nMotifCmd, sizeof nMotifCmd, 1, fp );

    while( ndp != motif_list )
    {
        mptr = (Motif*) ndp->data;

        fwrite( mptr->name, sizeof mptr->name, 1, fp );
        fwrite( mptr->data, sizeof( mDataRecord ), nMotifCmd, fp );
        ++nmotif;

        ndp = NodeNext( ndp );
    }

    fseek( fp, 50L, SEEK_SET );
    fwrite( &nmotif, sizeof nmotif, 1, fp );

    fclose( fp );
}

void load_motifs_fn( uchar *fn )
/*-------------------------------------------------------------------------*
 * Loads motifs from a file into the current game's motif list.
 *-------------------------------------------------------------------------*/
{
    NODEPTR ndp;
    Motif*  mptr;
    short   nLoadCmd, nmotif;
    FILE    *fp;

    fp = fopen( fn, "rb" );

    if( pgdCurGame->motif_list == NULL )
        pgdCurGame->motif_list = InitList();

    motif_list = pgdCurGame->motif_list;

    if( !fp )   /* no motifs */
        return;

    /* seek past header */
    fseek( fp, 50L, SEEK_SET );
    fread( &nmotif, sizeof nmotif, 1, fp );
    fread( &nLoadCmd, sizeof nLoadCmd, 1, fp );

    while( nmotif-- )
    {
        /* allocate motif and data for motif */
        mptr = (Motif*) (NodeInsert( NodeLast( motif_list ), sizeof( Motif ) )->data);
        mptr->data = (mDataRecord*) memget( nMotifCmd * sizeof( mDataRecord ) );

        /* read in the name */
        fread( mptr->name, sizeof mptr->name, 1, fp );
        /* read in the data */
        fread( mptr->data, sizeof( mDataRecord ), nLoadCmd, fp );
    }

    fclose( fp );
}

void load_motifs( void )
/*-------------------------------------------------------------------------*
 * I bet you don't know.
 *-------------------------------------------------------------------------*/
{
    uchar   fn[128];
    sprintf( fn, "%s.MTF", pgdCurGame->cfgname );

    if( pgdCurGame->motif_list )
    {
        /* delete the list.. this resets the list and reloads it */
        NukeLinkedList( pgdCurGame->motif_list );
        pgdCurGame->motif_list = NULL;
    }

    load_motifs_fn( fn );
}
