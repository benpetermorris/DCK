/*-------------------------------------------------------------------------*
 * DEFS.H:                                                                 *
 * Structure typedefs for WADs and other things.                           *
 *-------------------------------------------------------------------------*/

// #define HEXEN

#ifdef HEXEN
#define _ABBREV_    "HCK"
#define _FULL_      "Hexen Construction Kit"
#else
#define _ABBREV_    "DCK"
#define _FULL_      "DOOM Construction Kit"
#endif

/* Typedefs                                                                */
#undef ushort
#undef ulong
#undef uchar
#undef byte
#define ulong           unsigned long
#define uchar           unsigned char
#define ushort          unsigned short
#define byte            unsigned char

/* pointer */
typedef void * PTR;

enum
{
    wsDataOnly,
    wsComplete,
    wsTestMap
};

/* definitions for adf files */
//#include "adf.h"

/* exit levels */
#define EL_EXIT         200
#define EL_PLAY         100     /* play doom or whatever */

/* char restore in dck.c */
#define R_MAP       0x01
#define R_TOGGLES   0x02
#define R_MOTIF     0x04

#include "keys.h"

/* rectangle */
typedef struct
{
    short left, top, right, bottom;

} SRECT;

// map options
typedef struct
{
    unsigned short uSig;            // 0x4d42
    unsigned short uSizeThis;
    char szScriptName[128];

} MAPOPTIONS;

#define MO_SIG      0x4d42

/* thing display flag structure */
typedef struct
{
    uchar   skill1;
    uchar   skill2;
    uchar   skill3;
    uchar   ambush;
    union
    {
    uchar   dormant;
    uchar   multiplayer;
    };
    uchar   fighter;
    uchar   cleric;
    uchar   mage;
    uchar   gdeath;
    uchar   gcoop;
    uchar   gsingle;
    uchar   use;            /* use mask? */

} TFlags;

/* configuration structure */
typedef struct
{
    short   driver;     /* index into d_ints */
    short   mode;       /* index into r_ints */
    short   xscale;
    int     _zoom_high;
    int     _zoom_low;
    uchar   tog_redocc;
    uchar   tog_welcome;
    uchar   tog_shiftadd;
    uchar   tog_combineexact;
    uchar   tog_grid;
    uchar   tog_join;
    uchar   tog_markxlat;
    uchar   tog_snap;
    uchar   tog_unpeg;
    uchar   tog_scrolly;
    uchar   tog_entersave;
    uchar   tog_savesrch;
    uchar   tog_tiletex;
    int     tog_backup;
    uchar   tog_runparm;
    uchar   tog_donodes;
    uchar   tog_pophelp;
    uchar   tog_showtips;
    uchar   tog_usecats;        // picklist categories?

    // thing sprites:
    uchar   tgr_enemies;
    int     tgr_size1;
    int     tgr_size2;
    int     tgr_default;

    // texture browser:
    uchar   fTexShowNames;      // display texture names
    int     iTexFloorSize;      // floor display size
    int     iTexWallSize;       // wall display size

    char    szEditor[128];      // text editor

} Config;

enum
{
    TGR_NEVER = 0,
    TGR_PRIMARY = 1,
    TGR_SECONDARY = 2
};

enum
{
    BKUP_NEVER = 0,
    BKUP_FIRST = 1,
    BKUP_ALWAYS = 2
};

/* At the beginning of every doom picture */
typedef struct
{
    short width;
    short height;
    short notused1, notused2;

} PIC_HDR;

/* linedef dialog boxes */
typedef struct
{
    uchar   typestr[80];
    short   type;
    short   sectag;
    uchar   a_impass;
    uchar   a_mblocker;
    uchar   a_tside;
    uchar   a_uunpegged;
    uchar   a_lunpegged;
    uchar   a_secret;
    uchar   a_sblock;
    uchar   a_notonmap;
    uchar   a_autoonmap;

    uchar   sd1_utx[9], sd1_ftx[9], sd1_ltx[9];
    short   sd1_utxidx, sd1_ftxidx, sd1_ltxidx;
    short   sd1_sector;
    short   sd1_xofs, sd1_yofs;

    uchar   sd2_utx[9], sd2_ftx[9], sd2_ltx[9];
    short   sd2_utxidx, sd2_ftxidx, sd2_ltxidx;
    short   sd2_sector;
    short   sd2_xofs, sd2_yofs;

    short   sd2_valid;      /* is sidedef 2 valid ? */

} LDIALOG;

/* for wad_load */
enum
{
    WL_MAPONLY = 0x01,      /* read maps only */
    WL_COMPLETE = 0x02,     /* read complete wad */
    WL_MDIR = 0x04          /* update the master dir */
};

/* for map_freedata */
enum
{
    F_ALL,
    F_PRECALC
};

/* A string table entry                                                    */
typedef struct
{
    char   *str;
    short  ident;

} STR;

typedef struct
{
    uchar   name[40];
    short   nitem;
    short   maxitem;
    short   maxcol;
    short   kwdofs;     /* if a keyworded section is active */
    short   kwdlen;     /* length of keyworded section */
    STR     *stp;       /* array of strings */
    uchar   *strbuf;    /* buffer of entire strings */

} STRTABLE;

/*-------------------------------------------------------------------------*
 * WADfile structures
 *-------------------------------------------------------------------------*/

/* wad entry name */
typedef char            WADNAME[8];

/* Header at the beginning of a WAD file                                   */
typedef struct
{
    uchar   sig[4];     /* Signature of wad [IWAD/PWAD] */
    ulong   nentry;     /* Number of entries in this WAD's directory */
    ulong   dirstart;   /* The offset where the directory begins */

} WADHEADER;

/* Each entry in the WAD's directory is as follows...                      */
typedef struct
{
    ulong   offset;     /* Offset of the data */
    ulong   length;     /* Data length */
    WADNAME name;       /* Name of the data */

} WADENTRY;

/* A wad!                                                                  */
typedef struct
{
    uchar   fn[128];
    short   mode;       /* mode opened with */
    uchar   typ;        /* 1 = pwad, 0 = pwdIWAD */
    NODEPTR dir_list;
    NODEPTR curdir;
    WADHEADER hdr;

} WAD;

/* Each THING on a map is ...                                              */
#define TA_SKILL12      0x0001
#define TA_SKILL3       0x0002
#define TA_SKILL45      0x0004
#define TA_DEAF         0x0008
#define TA_AMBUSH       0x0008
#define TA_DORMANT      0x0010
#define TA_FIGHTER      0x0020
#define TA_CLERIC       0x0040
#define TA_MAGE         0x0080
#define TA_GSINGLE      0x0100
#define TA_GCOOP        0x0200
#define TA_GDEATHMATCH  0x0400
#define TA_MPLAYER      0x0010

typedef struct
{
    short   x;          /* X location of thing */
    short   y;          /* Y location of thing */
    short   angle;      /* Angle that the front of the thing is facing */
    short   type;       /* TYPE of thing [database] */
    short   attribs;    /* The thing's attributes [TA_xxx | ...] */

} DoomThing;

typedef struct
{
    short   tid;
    short   x;          /* X location of thing */
    short   y;          /* Y location of thing */
    short   height;
    short   angle;      /* Angle that the front of the thing is facing */
    short   type;       /* TYPE of thing [database] */
    short   attribs;    /* The thing's attributes [TA_xxx | ...] */
    byte    special;
    byte    arg0;
    byte    arg1;
    byte    arg2;
    byte    arg3;
    byte    arg4;

} HexenThing;

#define THING   HexenThing

/* Each VERTEX on a map is ...                                             */
typedef struct
{
    short   x;
    short   y;

} VERTEX;

/* Each LINEDEF on a map is ...                                            */
#define LA_IMPASS       0x0001
#define LA_MBLOCKER     0x0002
#define LA_TSIDE        0x0004
#define LA_UUNPEGGED    0x0008
#define LA_LUNPEGGED    0x0010
#define LA_SECRET       0x0020
#define LA_SBLOCK       0x0040
#define LA_NOTONMAP     0x0080
#define LA_AUTOONMAP    0x0100
#define LA_REPEAT       0x0200
#define LA_GETSPAC(attribs)     ((attribs&0x1c00)>>10)

typedef struct
{
    short   vfrom;      /* Vertex to start at */
    short   vend;       /* Vertex to end at */
    short   attribs;    /* Attributes of the line [LA_xxx | ...] */
    short   special;    /* Line type */
    short   sectag;     /* Affects sectors with this TAG when crossed */
    short   rsdef;      /* Right SIDEDEF */
    short   lsdef;      /* Left SIDEDEF */

} DoomLine;

typedef struct
{
    short   vfrom;      /* Vertex to start at */
    short   vend;       /* Vertex to end at */
    short   attribs;    /* Attributes of the line [LA_xxx | ...] */
    byte    special;

    union
    {
    byte    arg0;
    byte    sectag;
    };

    byte    arg1;
    byte    arg2;
    byte    arg3;
    byte    arg4;
    short   rsdef;      /* Right SIDEDEF */
    short   lsdef;      /* Left SIDEDEF */

} HexenLine;

#define LINEDEF     HexenLine

#define lSecTag(ldp)    ((ldp)->sectag)

#define ldp_isimpass(ldp)       ((ldp)->attribs & LA_IMPASS || (ldp)->lsdef == -1)

/* Each SIDEDEF on a map is ...                                            */
typedef struct
{
    short   wxofs;      /* Wall X offset */
    short   wyofs;      /* Wall Y offset */
    WADNAME utxname;    /* Upper texture name */
    WADNAME ltxname;    /* Lower texture name */
    WADNAME ftxname;    /* Side texture name */
    short   sector;     /* Sector of this sidedef */

} SIDEDEF;

typedef struct
{
    short wxofs;      /* Wall X offset */
    short wyofs;      /* Wall Y offset */
    short utxidx;     /* Upper texture index */
    short ltxidx;     /* Lower texture index */
    short ftxidx;     /* Side texture index */
    short sector;     /* Sector of this sidedef */

} SIDEDEFM;

/* Each SECTOR on a map is ...                                             */
typedef struct
{
    short   fheight;    /* Floor height */
    short   cheight;    /* Ceiling height */
    WADNAME ftxname;    /* Floor texture name */
    WADNAME ctxname;    /* Ceiling texture name */
    short   light;      /* Light value */
    short   type;       /* Sector type */
    short   sectag;     /* Sector action tag [see LINEDEF.sectag] */

} SECTOR;


/* Definitions for textures */
#include "texdefs.h"

/*-------------------------------------------------------------------------*
 * Keyboard status
 *-------------------------------------------------------------------------*/

#define kbRS    0x01
#define kbLS    0x02
#define kbCTRL  0x04
#define kbALT   0x08

#define isshift()   (keystat() & (kbRS | kbLS))
#define isctrl()    (keystat() & (kbCTRL))
#define isalt()     (keystat() & (kbALT))

#define SWM_COPY    COPY_PUT
#define SWM_XOR     XOR_PUT
#define SLS_DOTTED  DOTTED_LINE
#define SLS_SOLID   SOLID_LINE

/* Cursor step (number of steps across an axis)                            */
#define CSTEPTOTAL      50
#define MSTEP           10          /* Move 1/10th of the map on shifts */

/* Edit modes                                                              */
#define MODE_VERTEX     0x01        /* object modes */
#define MODE_LINE       0x02
#define MODE_THING      0x04
#define MODE_SECTOR     0x08

#define MODE_OBJLAST    0x08

#define MODE_SIDEDEF    0x10        /* Not actually used for editing */
#define MODE_POLYGON    0x20
#define MODE_RECTANGLE  0x40

#define MODE_ALL        (MODE_VERTEX | MODE_LINE | MODE_THING | MODE_SECTOR | MODE_SIDEDEF)
#define MODE_ALLMAP     (MODE_ALL & ~MODE_THING)
#define MODE_ANY        0xFF

/* Miscellaneous defines                                                   */
/* booleans */
#define ON                          1
#define OFF                         0
#define YES                         1
#define NO                          0
#define TRUE                        1
#define FALSE                       0

/* max number of points for polyMake */
#define pmMaxPoints  64

/* For gr_vidmode */
#define MODE_GFX                    -1
#define MODE_TEXT                   0x03
/* The higher, the better.. but slower */
#define MSCALE                      13L
/* Colors */
#define COLOR_MARKED                C_LIGHTRED
#define COLOR_CSRSEL                C_YELLOW
#define Centered                    -1
/* Absolute max. number of each type of object */
#define MAXOBJECT                   8192
#define _mapx2scr(x)                (short)((long)((sc_x * (long)(x))) >> MSCALE)
#define _mapy2scr(y)                (short)((long)(myplen-((long)((sc_y * (y))) >> MSCALE)))
#define _absmapy2scr(y)             (short)((long)((sc_y * (long)(y)) >> MSCALE))
#define _scrx2map(x)                (short)((long)((sc_x_r * (long)(x))) >> MSCALE)
#define _scry2map(y)                (short)((long)((sc_y_r * (long)(myplen-(y)))) >> MSCALE)
#define _absscry2map(y)             (short)((long)((sc_y_r * (long)(y))) >> MSCALE)
#define inrange( n, min, max )      ((n) >= (min) && (n) <= (max))
#define post_fullupdate()           { fFullUpdate = 1; }
#define swap2b(a,b)                 { short tmp = a; a = b; b = tmp; }
#define swap4b(a,b)                 { long tmp = a; a = b; b = tmp; }
#define cursor_put                  _cursor_put
#define st_onoff(n)                 ((n) ? "on" : "off")
#define _mtoupper(ch)               ((ch) >= 'a' && (ch) <= 'z' ? ((ch)-32) : (ch))

/* Given a 16-bit value, a bit value, and a command, sets or unsets a bit,
 * or leaves it alone.  0 = unset, 1 = set, 2 = leave */
#define bitset(dest,cmd,bitval)                         \
        { if( cmd == 0 )        (dest) &= ~(bitval);    \
          else if( cmd == 1 )   (dest) |= (bitval); }

#define bittest(testword, cmd, bitidx)                           \
        (((cmd) == 2) ? 1 : (((testword) & (bitidx)) ? 1 : 0) == (cmd))     \

/* in mbox_mode when mbox-marking */
#define MBOX_MARK                   0
#define MBOX_UNMARK                 1

/* cursor-size */
#define CSR_NORMAL                  0
#define CSR_BIG                     1

/* passed to file_get */
enum
{
    FG_LOAD = 0x01,
    FG_SAVE = 0x02,
    FG_NOMAPS = 0x04
};

/* Things passed to cursor_put                                             */
#define CSR_PUT             0
#define CSR_UPDATE          1
#define CSR_ERASE           2

/* Passed to obj_insert when using vertex mode                             */
#define VTX_CREATE                  0
#define VTX_CREATEJOIN              1

/*-------------------------------------------------------------------------*
 * Status bar defs
 *-------------------------------------------------------------------------*/

enum
{
    SBI_FIRST,
    SBI_TEXT,       /* plaintext */
    SBI_TOGGLE,     /* on off */
    SBI_STRING,     /* string */
    SBI_CIRCLE,     /* circle (filled) */
    SBI_RECT,       /* rectangle (filled) */
    SBI_PAD,        /* spacing */
    SBI_BOX,        /* shadow-box */
    SBI_NEXTLINE    /* just a marker.. */
};

enum
{
    SB_TOP = -2,
    SB_BOTTOM = -1
};

enum
{
    SIF_NOCLEAR = 0x01      /* no clear on sb_clearitems */
};

typedef void (* VFP)( PTR );

typedef struct
{
    short   type;       /* SBI_xxx */
    uchar   update;     /* update this item fer shurely */
    ushort  flags;      /* flags for the item, some of them item-specific */

    /* these are used when adding and drawing items */
    short   x, y;       /* x and y position (in pixels) */
    short   xwidth;     /* width of item, including padding, in pixels */
    short   color;      /* the color of the item.  if > 255, includes background */

    short   ib_x2, ib_y2;   /* box item bottom, right corner. */
    short   it_onoff;   /* for toggle items, on or off */
    char *  is_stp;     /* for string items, the string (does not affect width) */

} sbItem, * sbItemp;

typedef struct
{
    short   fheight;            /* Height in font sizes */
    short   height;             /* in pixels */
    uchar   update;             /* force update at next sb_update() ? */
    uchar   rerect;             /* Redraw rect at update? */
    short   x1, y1, x2, y2;     /* Full rectangle including padding */
    NODEPTR itemlist;           /* status bar items, or null if none */

    VFP     upd_func;           /* draw function */
    uchar   *data;              /* Data pointer for user data */

} stBar, * stBarp;

#define si_getflag(i,f)     ((i)->flags & (f))
#define si_setflag(i,f,s)           \
            ((i)->flags = (s) ?     \
             (i)->flags | f :       \
             (i)->flags & ~(f))

#define sbi_nextline(sbp)   sbi_add( sbp, SBI_NEXTLINE, 0, NULL )
#define sbi_pad(sbp,n)      sbi_add( sbp, SBI_PAD, n, NULL )

/*-------------------------------------------------------------------------*
 * MENU definitions (some are shared with BBAR)                            *
 *-------------------------------------------------------------------------*/

#define MITEM_FUNC          0
#define MITEM_SUBMENU       1
#define MITEM_NOTHING       2
#define MITEM_SEPBAR        3
#define MITEM_TEXT          4

#define MENU_VERT           0
#define MENU_HORIZ          1

typedef short (* __MFUNC)( short );

typedef struct
{
    short     x, y;           /* x/y position of menu */
    uchar   *text;          /* text for the item */
    short     selpos;         /* selection underline position */
    uchar   type;           /* MITEM_xxx type */
    __MFUNC   func;           /* if we're calling a function... */
    PTR     submenu;        /* if we're calling a submenu... */
    uchar   *desc;          /* Description text */
    short     fparam;         /* Additional data passed to its function */

} MITEM;

typedef struct TAGmenu
{
    short     x, y;           /* X/y pos of menu */
    uchar   type;           /* from MENU_xxx types */
    short     nitem;          /* number of items in the menu */
    short     maxitem;        /* max number of items in the menu */
    short     curitem;        /* current item */
    MITEM   *itemlist;      /* list of items (array of MITEM * maxitem size) */
    short     x1, y1, x2, y2; /* coords of menu */
    uchar   *savebuf;       /* underlying image */
    unsigned  sbsize;         /* buffer size */
    stBarp  sbp;            /* bar hadnle */
    uchar   *help;          /* help keyword, or NULL */

} MENU;

/* Returned from a menu function */
#define MA_NEXTITEM         0       /* Next item */
#define MA_PREVITEM         1       /* Prev item */
#define MA_BACK             2       /* Back one menu */
#define MA_EXIT             3       /* Exit the entire menu */
#define MA_CONTINUE         4       /* Just continue processing */

#define mnu_addsepbar(mptr) mnu_additem( mptr, NULL, 1, MITEM_SEPBAR, NULL, 0, NULL )

/* obj_linefind                                                            */
#define LF_BEGIN        0
#define LF_CONTINUE     1
#define LF_SETPARM      2
#define LF_VFROM        0
#define LF_VEND         1
#define LF_VANY         2
#define LF_ATTRIBS      3
#define LF_SECTAG       4
#define LF_SIDEWITHSEC  5
#define LF_TYPE         6
#define LF_END          7

/* MessageBox Defines                                                      */
#define MB_YES          0x01
#define MB_NO           0x02
#define MB_OK           0x04
#define MB_CANCEL       0x08
#define MB_ALWAYS       0x10
#define MB_OKCANCEL     (MB_OK | MB_CANCEL)
#define MB_YESNO        (MB_YES | MB_NO)
#define MB_YESNOALWAYS  (MB_YESNO | MB_ALWAYS)
#define MB_YESNOCANCEL  (MB_YESNO | MB_CANCEL)
#define MB_ALL          (MB_YES | MB_NO | MB_OK | MB_CANCEL | MB_ALWAYS)

/* Current operation */
#define OP_NOTHING          0
#define OP_PASTENEXTCLICK   1
#define OP_DRAG             2
#define OP_CREATELINE       3
#define OP_SCALE            4
#define OP_ROTATE           5
#define OP_EXIT             6
#define OP_MARKBOX          7
#define OP_SOMETHING        8       /* Just an indicator */
#define OP_COMBLINE         9
#define OP_FPAINTER         10
#define OP_TAGSET           11
#define OP_POLYGON          12
#define OP_RECTANGLE        13

/* Snap-to status */
#define S_NONE              0
#define S_NORMAL            1
#define S_VERTEX            2

/* Broadcasts */
typedef void (*BFUNC) ( short );      /* Broadcast function */

#define BC_NEWFILE          0
#define BC_CLEANFILE        1

#if 0
typedef struct
{
    ADF_SECTION     hdr;

    union
    {
        SO_COMMAND  *so_cmds;
        MO_PARM     *mo_cmds;

    } cmd;

} AD_SECTION;
#endif

typedef struct
{
    short type;
    short height, color, radius;
    char  sprite[9];
    char  dangle;   /* draw angle? */
    char  clash;    // "stickable"?
    short imgnum;   /* image number for drawing sprites */

} TInfo;

#define MaxPNAME    2048
#define MaxTEXTURE  768
#define MaxFLOOR    256

typedef struct
{
    WADNAME name;           /* name of the patch */
    short   wadnumber;      /* which texture .wad is it in? */
    long    offset;         /* offset in the .wad */

} PNRef;

const int HighestItemID = 11000;
const int nMaxItems = 384;

/* Game data structure                                                     */
typedef struct
{
    uchar   cfgname[9];        /* name of the data file (.GD) - without .GD */

    uchar   name[40];
    uchar   dir[80];
    uchar   pwdIWAD[13];
    uchar   tag[13];
    uchar   newtag[13];
    uchar   szNodes[80];
    uchar   *texwads[16];
    uchar   run[80];        /* what to run when f9 is pressed */
    uchar   palette[32];
    short   lines_notag[128];
    short   ActionsWithTID[32];

    /* each of the "sections" string tables has the name of the section.
     * the id code is the first number in the "..type" list that belongs
     * to that section.  GET IT? */

    TInfo    thinginfo[nMaxItems];
    short    ttype2idx[HighestItemID];
    short    tnum2img[HighestItemID];

    // these are duplicated as globals, so we don't have to reference
    // the current game pointer all the time..
    StrList  *stlThingSections;
    StrList  *stlThingTypes;
    StrList  *stlThingArgs;
    StrList  *stlActSections;
    StrList  *stlActTypes;
    StrList  *stlActArgs;
    StrList  *stlPopups[10];
    StrList  *stlSectorTypes;
    StrList  *stlAngles;
    StrList  *stlFlats;

    PNRef    *pnames;
    TEXTURE  *texlist;
    WADENTRY *floorlist;
    short    npname;
    short    ntex;
    short    nfloor;
    int      nPopups;

    bool     fLargeData;

    NODEPTR  motif_list;

} GAMEDATA;

typedef struct
{
    short width;
    short height;
    short w2;
    short h2;
    short notused;
    short widthpadded;
    short depth;
    short planes;

} img_hdr;

/* for protected mode */
#define zstrlen( s )        (((s) == NULL) ? 0 : strlen( s ))
#define lastchar(s)         (*(char *)((s)+strlen(s)-1))
#define offsetof(typ,id)    (size_t)&(((typ*)0)->id)       /* Love this macro. */
#define mkcolor(fg,bg)      (short)((fg) + ((bg) << 8))
#define mkegacolor(fg,bg)   (short)((fg) + ((bg) << 4))
#define bittobool(val,bitval)   (((val) & (bitval)) ? 1 : 0)

#include "bspstruc.h"
