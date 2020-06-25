/*-------------------------------------------------------------------------*
 * DBOX.H:                                                                 *
 * Functions for dialog boxes                                              *
 *-------------------------------------------------------------------------*/

#define CENTER              -1
#define RIGHT               -2
#define BOTTOM              -3

#define FLD_CHECK           0
#define FLD_RADIO           1
#define FLD_INTEGER         2
#define FLD_STRING          3
#define FLD_PUSH            4
#define FLD_TEXT            5
#define FLD_BOX             6
#define FLD_CHECK3          7
#define FLD_BITMAP          8
#define FLD_LINE            9
#define FLD_LBOX            10
#define FLD_RADIO2          11      /* "3-way" radio buttons .. heh */

#define _DF_ACTSTART        256
#define DF_CONTINUE         _DF_ACTSTART+0
#define DF_NEXTITEM         _DF_ACTSTART+1
#define DF_PREVITEM         _DF_ACTSTART+2
#define DF_SAVE             _DF_ACTSTART+3
#define DF_NOSAVE           _DF_ACTSTART+4
#define DF_REDISPLAY        _DF_ACTSTART+5

/* Dialog function .. returns a DF_xxx integer, takes the pointer of the
 * current field. */
typedef short (* __DFUNC)(void *);

typedef struct
{
    char    *name;          /* name or null */

    short   x, y;           /* Absolute screen coords */
    short   xlen, ylen;     /* Lengths in fontsize */
    SRECT    trect;
    short   x1, y1, x2, y2; /* Absolute screen coords of dialog */
    short   tx, ty;         /* top left pixel of top left character */
    short   xmarg, ymarg;   /* X and Y margins in pixels */
    NODEPTR fld_list;       /* List of field items on the dbox */
    NODEPTR bg_list;        /* List of background items on the dbox */
    void    *curfldp;       /* Current item pointer */
    short   curitem;        /* Current item */
    short   insmode;        /* Insert mode on? */
    char    *title;         /* Dialog title */
    uchar   *savebuf;       /* Picture underneath the dialog */
    short   orgx, orgy;     /* Origin lines */
    short   clr_org;        /* Origin color */
    short   go_item;        /* go to this item? */
    __DFUNC   upafterfp;      /* called once after dialog upped */
    __DFUNC   downafterfp;
    __DFUNC   valfp;          /* before exiting. */
    __DFUNC   quitfp;         /* before exiting. */
    __DFUNC   exitfp;         /* before exiting. */
    uchar   *help;          /* help keyword */

} DIALOG;

typedef struct
{
    short   type;           /* field type */
    char    *name;          /* name or null */

    char    *data;          /* Data pointer for temp. storage depending on type */
    void    *store;         /* Storage pointer used when init/exit dialog */
    short   rgroup;         /* Radio group if it's a radio button */
    short   drwstat;        /* draw status */
    char    *picture;       /* Picture entry if it's a data string */
    ushort  flags;          /* From FLD_xxx */
    uchar   x, y;           /* X and Y location on the dialog box (font coords) */
    size_t  datalen;        /* Data length */
    __DFUNC   enter_fp;       /* Field entry function pointer */
    __DFUNC   exit_fp;        /* Field exit function pointer */
    __DFUNC   push_fp;        /* Pushbutton fucntion pointer */
    __DFUNC   mod_fp;         /* when the filed is modified */
    DIALOG  *dlgp;          /* Dialog it belongs to */
    short   clr_selected;   /* If it's selected */
    short   clr_normal;     /* If it's not selected */
    short   clr_skip;       /* If it's not available */
    uchar   disp;           /* Has it been displayed yet? */
    uchar   radnum;         /* radio button number in group */
    short   dinit_sum;      /* strsum() of storage on init .. for string fields */
    uchar   dchg;           /* only valid after dialog box exited.. data changed? */
    short   radx, rady;     /* radio button center x and y */
    short   piclen;         /* Picture length */
    short   curpos;         /* String position for data entry */
    short   x1, y1, x2, y2; /* For quick mouse-selection */
    uchar   _firstchar;     /* Had a char yet? */
    short   xadj, yadj;     /* X/Y adjustments */
    uchar   *help;          /* help keyword */

    /* The following is used for listboxes attached to fields */
    uchar   **larray;       /* List box array or NULL if none (default of course) */
    __DFUNC   lbupd_fp;       /* List box update function (or NULL if none) */
    short   lnitem;         /* List box number of items */
    short   lcuritem;       /* List box item */
    short   lb_discard;     /* List box array (not contents of) discardable? */
    uchar   *lsavebuf;      /* List box underlying image (not used for) */
    short   lx1, ly1,       /* List box coordinates */
            lx2, ly2;

    /* The following is used for listbox fields */
    STRTABLE *lf_stbl;
    __DFUNC   lf_updfp;
    SRECT    lf_srup, lf_srdn, lf_srect;
    short   lf_curitem;
    short   lf_topitem;
    short   lf_lastcur;     /* last top item, so we don't have to redisplay all of it */
    short   lf_lasttop;     /* last top item, so we don't have to redisplay all of it */
    short   lf_ndisp;       /* number to display (height) */
    short   lf_width;       /* not used much */
    uchar   *lf_srchbuf;

    int     keys[12];       /* User-installed key functions */
    __DFUNC   keyfuncs[12];   /* User-installed key functions */
    short   keycount;       /* Count for above */

    void    *sparedata;     /* Extra (not used by dialogs) data */

} FIELD;

/*-------------------------------------------------------------------------*
 * Field options                                                           *
 *-------------------------------------------------------------------------*/

#define FLD_SKIP        0x0001
#define FLD_NOEDIT      0x0002
#define FLD_UPCASE      0x0004
#define FLD_BMOUSE      0x0008      /* for buttons: mouse only control */
#define FLD_INITNODISP  0x0010      /* no display field on init */
#define FLD_NODISP      0x0020      /* never display field ever ever nope */

/*-------------------------------------------------------------------------*
 * Function prototypes                                                     *
 *-------------------------------------------------------------------------*/

short pb_cancel( FIELD *fldp );       /* Predefined cancel/ok functions */
short pb_ok( FIELD *fldp );

#define dlg_proc(a,b) 0
#define dlg_addtext(a,b,c,d) ((FIELD*)NULL)
#define dlg_addbox(a,b,c,d,e) ((FIELD*)NULL)
#define dlg_init(a,b,c,d,e) ((DIALOG*)NULL)
#define dlg_setorglines(a,b,c,d)
#define dlg_addfld(a,b,c,d,e,f,g,h,i) ((FIELD*)NULL)
#define fld_addlbox(a,b,c)
#define dlg_ptrnum(a,b) 0
#define dlg_numptr(a,b) ((FIELD*)NULL)
#define dlg_namptr(a,b) ((FIELD*)NULL)
#define dlg_free(a)
#define dlg_addbit(a,b,c,d) ((FIELD*)NULL)
#define dlg_delbg(a,b)
#define fld_addlboxSTRTABLE(a,b)
#define dlg_settitle(a,b)
#define dlg_setxy(a,b,c)
#define dlg_disp(a,b)
#define fld_setkey(a,b,c)
#define fld_coords(a)
#define fld_setadj(a,b,c)
#define dlg_addline(a,b,c,d,e) ((FIELD*)NULL)
#define fld_disp(a)
#define fld_setarray(a,b,c)
#define dlg_drawbg(a)
#define _fld_setpos(a,b)
#define fldismod(a) 0
#define dlg_addcbox(a,b,c) ((FIELD*)NULL)
#define dlg_addlboxfld(a,b,c,d,e,f,g) ((FIELD*)NULL)
#define fld_undisp(a)

#define fld_setoptions(fldp,options,state)

#define fld_opt(fldp,opt)   0

/* Easier macro for adding radio buttons */
#define dlg_addradio(dlgp,x,y,group,store)      \
        dlg_addfld(dlgp,FLD_RADIO,x,y,group,NULL,store,0,NULL)

/* Easier macro for adding string input */
#define dlg_addstring(dlgp,type,x,y,pic,store)  \
        dlg_addfld(dlgp,type,x,y,0,pic,store,0,NULL)

/* Easier macro for adding pushbuttons */
#define dlg_addpush(dlgp,x,y,text,fp)           \
        dlg_addfld(dlgp,FLD_PUSH,x,y,0,text,NULL,0,fp)

/* Easier macro for adding checkboxes */
#define dlg_addcheck(dlgp,x,y,store)            \
        dlg_addfld(dlgp,FLD_CHECK,x,y,0,NULL,store,0,NULL)

#define setname(fldp,nam)

/* Set help */
#define sethelp(p,h)

#define lbfld_setdata(f,lb) 

