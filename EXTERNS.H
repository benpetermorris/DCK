/*-------------------------------------------------------------------------*
 * EXTERNS.H:                                                              *
 * External definitions to go in DCK.H                                     *
 *-------------------------------------------------------------------------*/

/* File variables                                                          */
extern  uchar       szCurMapTag[9];            /* current level tag */
extern  WAD*        pwdCurWad;             /* current .wad */
extern  WAD*        pwdIWAD;               /* pwdIWAD */
extern  GAMEDATA    *pgdGameArray;      /* game data arrays */
extern  short       iMaxGame;
extern  GAMEDATA    *pgdCurGame;           /* current game data */
extern  short       iCurGame;         /* current game data number */
extern  MAPOPTIONS  MapOptions;
extern  byte        fHexen;         // hexen data structures?
extern  int         nMapEntryCount;     // number of entries in a map for current game

/* Misc. variables                                                         */
extern  short   cursorx, cursory;       /* x and y of cursor location on map */
extern  int     ismouse;                /* Mouse on? */
extern  uchar   *_memstr;               /* What are we doing with memory? */
extern  uchar   fFullUpdate;            /* Do a full update? */
extern  ushort  nMadeChanges;           /* Changes since last save? */
extern  ushort  nMapChanges;            /* map changes? */
extern  short   iLastThingEdited;             /* Last thing edited.. for copying */
extern  short   torigin_x, torigin_y;   /* thing origin */
extern  uchar   restore;                /* restore status */
extern  short   _fontwidth;
extern  short   _fontheight;
extern  short   _vradius;               /* vertex radius.  0 = 3x3 square, 1 = circle r = 5 */
extern  short   fGetConfig;             /* does DCK32.CFG not exist? */
extern  short   fFindGames;             /* try to find games? */
extern  short   iExitLev;
extern  short   get_out;
extern  bool    fNewMap;                // new map?
extern  char    szDSFile[128];          // Dialog Script filename

/* Defaults and stuff                                                      */
extern  short   _def_ftxidx;            /* default full texture index */
extern  short   _def_utxidx;            /* default upper texture index */
extern  short   _def_ltxidx;            /* default lower texture index */
extern  short   _def_stairtxidx;        /* default stair texture index */
extern  short   _def_doorbig;
extern  short   _def_doorsmall;
extern  short   _def_doortrak;
extern  short   _def_light;
extern  WADNAME _def_ceiltex;
extern  WADNAME _def_liftfloor;
extern  WADNAME _def_floortex;
extern  Config  config;                 /* configuration structure */

/* Graphics variables                                                      */
extern  short   lastvidmode;            /* To prevent unnecessary switching */
extern  short   lastcolor;              /* To prevent unnecessary color changes */
extern  short   sxlen, sylen;           /* X and Y size of the viewport */
extern  short   mxplen, myplen;         /* X and Y size of the map area */
extern  short   mxpstart, mypstart;     /* X and Y start of the map area */
extern  short   xstart, ystart;         /* MAP x and y coords that appear at top
                                         * left of viewport. */
extern  short   xscale;                 /* Scale (zoom) factor.  This is used on
                                         * all sc_xxx functions, which take virtual
                                         * coordinates and not screen coordinates. */
extern  short   yscale;                 /* y scale factor (slave to xscale) */
extern  long    sc_x, sc_y;             /* Precalcs (map->screen) */
extern  long    sc_x_r, sc_y_r;         /* Precalcs (screen->map) */
extern  short   sb_y;                   /* Sbar y pos */
extern  short   gridstep;               /* gridstep in map coords */
extern  short   editmode;               /* Current map edit mode [MODE_xxx] */
extern  uchar   marklist[MAXOBJECT];    /* boolean mark array */
extern  uchar   linemarklist[MAXOBJECT];    /* boolean mark array */
extern  short   dckto256[][4];
extern  TFlags  td_flags;               /* thing display flags */
extern  short   file_version;           /* file versions .. woo woo ooo ooo */
extern  uchar   *loadtexwads[16];       /* load these texture wads. */

/* map variables                                                         */
extern  VERTEX  *vlist;                 /* List of vertices in current map */
extern  short   nvertex;                /* Number of vertices in current map */
extern  short   maxvertex;
extern  LINEDEF *linelist;              /* Lines in current map */
extern  short   nline;                  /* Number of lines */
extern  short   maxline;
extern  THING   *thinglist;             /* Things! */
extern  short   nthing;                 /* Number of things */
extern  short   maxthing;
extern  SECTOR  *sectorlist;            /* Sectors in current map */
extern  short   nsector;                /* Number of sectors */
extern  short   maxsector;
extern  SIDEDEFM *sidelist;             /* Sidedefs in current map */
extern  short   nside;                  /* Number of sides */
extern  short   maxside;
extern  TEXTURE *texlist;               /* Texture names used in this map */
extern  short   ntex;                   /* Number of textures */
extern  WADENTRY *floorlist;            /* Floor/ceiling list */
extern  short   nfloor;                 /* Number of floors/ceilings */
extern  SSector *ssectors;
extern  long    num_ssectors;
extern  Pseg    *psegs;
extern  long    num_psegs;
extern  Pnode   *pnodes;
extern  long    num_pnodes;
extern  Block   blockhead;
extern  short   *blockptrs;
extern  short   *blocklists;
extern  long    blockptrs_size;
extern  long    blockmap_size;
extern  char    *reject;
extern  long    reject_size;
extern  char    *pBehavior;
extern  long    lBehaviorSize;

/* misc. toggles                                                           */
extern  uchar   tog_fine;               /* fine mode on? */
extern  uchar   tog_grid;               /* grid mode on? */
extern  uchar   tog_drag;               /* drag mode on? */
extern  uchar   tog_mbox;               /* mark box mode on? */
extern  uchar   tog_snap;               /* snap to grid? */
extern  uchar   tog_key;                /* key status bar on? */
extern  uchar   tog_join;               /* autojoin mode on? */
extern  uchar   tog_csize;              /* Cursor size (0 = small) */
extern  uchar   tog_vradius;            /* use vradius? */
extern  uchar   tog_tgraph;             /* thing icons? */
extern  uchar   tog_poly;
extern  uchar   tog_rect;

/* game pointer "aliases"                                                  */
extern  StrList *stlThingSections;
extern  StrList *stlThingTypes;
extern  StrList *stlThingArgs;
extern  StrList *stlActSections;
extern  StrList *stlActTypes;
extern  StrList *stlActArgs;
extern  StrList *stlSectorTypes;
extern  StrList *stlAngles;
extern  StrList *stlFlats;
