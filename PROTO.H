/*-------------------------------------------------------------------------*
 * Miscellaneous prototypes                                                *
 *-------------------------------------------------------------------------*/

/* MISC.C                                                                  */
void Unregistered();
void ResetKeys();
short getduration( short seconds );
void settimerfreq( unsigned short freq );
void resettimer( void );
short isolder( struct ftime *ft1, struct ftime *ft2 );
short islevelresource( char *name );
short game_choose( void );
short strsum( uchar *s );
short wncmp( char *s1, char *s2 );
short wncmpi( char *s1, char *s2 );
short samefile( uchar *fn, uchar *fn2 );
short open_file( uchar *fn, short level );
uchar *WN2Str( uchar *s );
uchar *searchfile( uchar *fn, uchar *path );
void wncpy( char *dest, char *src );
short isnodeexist( void );
short line_len( short x1, short y1, short x2, short y2 );
short usage( void );
void DEBUG_INIT( char *str, ... );
void DEBUG_EXIT( void );
void error( char *str, ... );
void memfree( void *ptr );
void polyMake( short x1, short y1, short x2, short y2,
               short npoints, short start_ang );
void *memgetmax( size_t *length );
void *memget( size_t length );
void *memresize( void *old, size_t length );
void chkrange( char *name, long value, long lowest, long highest );
short getcfgint( FILE *fp );
short getcfgline( char *buf, FILE *fp );
void NukeLinkedList( NODEPTR head_ndp );
NODEPTR NodeFindData( NODEPTR head, void *data );
ulong memavail( void );
short check_save( void );
void setscale( short __xscale );
void rs_coords( short *dx, short *dy, short sx, short sy,
                double angle, double scale );
short file_save( bool fDoDialog, bool fTestMap );
short firstfreetag( void );
short calc_percent( long current, long total, short maximum );
void rt_runthis( char *runwhat );
void rt_setparm( char *parm, char *contents );
void rt_replace( char* szBuffer );
void setfilertparms();
void broadcast( short msg );
void bc_register( BFUNC fp );
short _angle( short x1, short y1, short x2, short y2 );
short obj_lineangle( short object );

/* WAD.C                                                                   */
void wad_reload( WAD* wptr, short mode );
short wad_load( WAD *wptr, short mode );
WAD *wad_open( char *wadfn, short mode );
short wadgethandle( void );
uchar *dirnext( void );
void *dirload( WAD* wptr, ulong* size, uchar* buf );
short dirgimmehandle( WAD* );
ulong dirgimmesize( WAD* );
WADENTRY* wad_find( WAD *wptr, uchar *_name );
WADENTRY* wad_findCurMap( WAD *wptr, uchar *_name );
int WadSave( const char *pszFile, const char *pszMapTag, int iMode );
uchar *wadgetname( void );
void wad_close( WAD* );
WADENTRY *wadnewentry( short handle, char *name, long size );
FILE* wad_e2h( WAD *wptr, WADENTRY *weptr );
WAD *wad_new( char *wadfn );
void closecurwad( short newgame );

/* GFX.C                                                                   */
uchar *ipi2img( uchar *src, size_t buflen, img_hdr *dest );
void loadpalette( char *fn );
void gr_vidmode( short mode );
void gr_line( short x1, short y1, short x2, short y2, short color );
void gr_rect( short x1, short y1, short x2, short y2, short color );
void sc_line( short x1, short y1, short x2, short y2 );
void sc_plotx( short x1, short y1, short color );
void sc_drawthing( short tnumber );
void gr_clearviewport( void );
void gr_setviewport( short left, short top, short right, short bottom );
void gr_closegraph( void );
void gr_setcolor( short color );
void gr_setvisualpage( short );
void gr_setactivepage( short );
void gr_setwritemode( short );
void gr_outtextxy( short x, short y, char *text );
void gr_shadrect( short x1, short y1, short x2, short y2, short color );
void gr_setlinestyle( short style, short thickness );
void gr_box( short x1, short y1, short x2, short y2, short color );
void cursor_setsize( short size );
short gr_textwidth( char *str );
uchar cursor_isvis( void );
void gr_rshadrect( short x1, short y1, short x2, short y2, short color );
void _cursor_put( short mode );
void gr_fillcircle( short _x, short _y, short radius );
void gr_drawcircle( short _x, short _y, short radius );

/* INPUT.C                                                                 */
void setup_go_object( short object );
short le_input( void );

/* MAPDRAW.C                                                               */
void map_update( bool fDraftMode );
void grid_update( void );
void globtomap( short *dx, short *dy, short gx, short gy );
void maptoglob( short *dx, short *dy, short mx, short my );

/* MAPMEM.C                                                                */
short findgamename( uchar *name );
short ismaptag( uchar *name, int iMatchGame = -1 );
short ismaptag_menu( uchar *name, WAD* );
void map_freedata( short data );
void map_test( void );
void map_edit( uchar *load_fn, uchar *load_tag, uchar *load_game );
void map_load( void );
void map_purge( short purgewhat );
void map_new( short newgame );

/* LOADSAVE.C                                                              */
short file_get( short mode, uchar *fn, uchar *ext );
short map_get( void );

/* SBAR.C                                                                  */
void sbs_data( stBarp sbp, char *data );
void sbs_string( stBarp sbp, char *fmt, ... );
void si_str( sbItemp item, char *fmt, ... );
void si_clr( sbItemp item, short c );
void si_onoff( sbItemp item, short onoff );
void sbd_string( stBarp sbp );
void sbd_main( stBarp sbp );
void sbd_standard( stBarp sbp );
void sb_clearitems( stBarp sbp );
sbItemp sbi_add( stBarp sbp, short type, short width, char *stp );
sbItemp sbi_addcombo( stBarp sbp, char *stp );
void sb_free( stBarp sbp );
stBarp sb_init( short height, uchar rerect, VFP upd_func );
void sb_attach( stBarp sbp, short where );
void sb_detach( stBarp sbp );
void sb_recalcpos( void );
void sb_setupdate( stBarp sbp, short mode );
short sb_getupdate( stBarp sbp );
void sb_update( short fFullUpdate );

/* OBJECT.C                                                                */
bool obj_ismarked( Object& obj );
void obj_unmark( Object& obj );
void obj_mark( Object& obj );

inline bool obj_ismarked( int iObject, int iObjectMode = editmode )
    { return obj_ismarked( Object( iObject, iObjectMode ) ); }
inline void obj_mark( int iObject, int iObjectMode = editmode )
    { obj_mark( Object( iObject, iObjectMode ) ); }
inline void obj_unmark( int iObject, int iObjectMode = editmode )
    { obj_unmark( Object( iObject, iObjectMode ) ); }

Object& EnumObjects( int iObjectType = -1, int iStartAt = 0 );

void undo_init( void );
void undo_setvars( void );
void undo_undo( void );
void obj_markxlate( short oldmode, short newmode );
short obj_getphyssector( short line_num, short side );
short linesplit( short lineno, short vertexno );
short vertex_createjoin( short vertex, short *newvertex );
short obj_sectsetmotif( short object );
short obj_linesetmotif( short object );
short obj_linefind( ushort command, ... );
void obj_center( short editmode, short obj_num, short *x, short *y );
void vertex_purge( void );
void mapcoord_upd( short x, short y );
short obj_get( short obj_type, short x1, short y1, short x2, short y2, int *iDistance = NULL );
short obj_highlight( short obj_type, short object, short color );
void obj_drawmaniplist( short coordx, short coordy, double angle, double scale );
void obj_exitmanip( short save );
short obj_initmanip( int obj_type, short coordx, short coordy, Object& objCursor,
                     short copy, short relative );
short obj_manipupd( short x, short y );
short obj_markbox( short x1, short y1, short x2, short y2, short mode );
short obj_nmarked( int iObjectType = -1 );
short obj_delvjoinline( short csr_object );
short obj_delljoinsectors( short csr_object );
short obj_snapmarked( Object& );
short obj_insert( short objtype, short source, short x, short y );
short obj_splitlines( short csr_object );
void obj_clearmarks( short csr_object, short update );
short findsector( short x, short y, short *lineno, short *whichside );
short obj_align( short csr_object, short axis );
void find_slope_point( short *dx, short *dy, short xpos, short ypos, VERTEX *vfrom, VERTEX *vend );
void vertex_replace( short search, short replace );
short obj_delete( short editmode, Object& objDelete, short delmarked );
void _scr2map( short *dx, short *dy, short cx, short cy );
void _mapsnap( short *dx, short *dy, short sx, short sy, bool fSimple = FALSE );
void obj_findcenter( short *x, short *y );
short line_flip( short csr_object, uchar flipvertex );
short obj_linelen( short object );
short obj_tagset( short source, short sourcemode, short destmode );
short obj_sectnormalize( short object );
short obj_linenormalize( short object );
short obj_lfinishcombine( short csr_object );
short obj_lstartcombine( short csr_object );
char * obj_toggleflags( Object&, uchar ch );

/* READ.C                                                                  */
short readtext( char *title, char **text, short numlines, short ydisp );
short countlines( char **text );

/* THINGS.C                                                                */
short thing_color( short type );
void thing_calcradii( void );
short thing_height( short type );
short thing_radius( short type );
short thing_mapradius( short type );
char *thing_visstring( short when ) ;
short thing_edit( short object, short editmarked );
short thing_markpattern( short object );
void thing_autosetangle( short object );
short td_getflags( void );
short td_check( ushort attribs );

/* MENU.C                                                                  */
#define mnu_addhelp(a,b)
#define mnu_run(a,b) 0
#define mnu_clear(a)
#define mnu_undisp(a)
#define mnu_disp(a,b)
#define mnu_coords(a)
#define mnu_init(a,b,c,d,e) NULL
#define mnu_additem(a,b,c,d,e,f,g)
#define mnu_checkmouse(a,b,c) NULL
#define mnu_free(a)

/* CLIP.C                                                                  */
short cb_draw( short x, short y );
short cb_paste( short obj_type, short x, short y );
short cb_clip( short obj_type, short csr_object );
short cb_isempty( void );
void cb_free( short type );

/* LINE.C                                                                  */
bool MatchLineWithTID( int iLine, int iTID );
void setpsector( short num );
short side_replacetex( short csr_object );
short line_edit( short object, short editmarked, short goside );
short line_editside( short csr_object, short whichside );
void lc_abort( void );
void lc_start( short x, short y );
short lc_save( short startnew );
short vmatch( short v1idx, short v2idx );
void sect_fixtex( short obj_num, short remove );
void side_getneedtex( short line_no, short whichside,
                      short *upper, short *full, short *lower );
void _WFfixborderlines( short sector );
void _WFcreatedoor( short sector );
void _WFfixtexnames( short obj_num, short whichside, short remove );
short _WFsplitsector( short splitline );
void _WFcreatelift( short sector );
short _WFcompleteregion( short lstart, short lend );

/* TEXTURE.C                                                               */
int TexBrowser( int *iSelected, bool fFloors );
void mkpicbuf( short width, short height );
void floor_disp( short x, short y, short floorno );
short floor_find( short startpos, short direc, uchar *floorname );
char *texture_tostr( char *str, short texno );
short texture_choose( short curtex, short dofloor );
short texture_find( short startpos, short direc, uchar *texname );
short texfind_dlg( char *str );
short texture_toint( WADNAME name );
short texture_toint_ic( WADNAME texname );
short floor_toint( char *floorname );
void we_disppic( short x, short y, short xw, short yw, WAD* wptr, WADENTRY *weptr );
void checktexupdate( WAD *wptr );
short istexwad( char *fn );
short updatetextures( WAD *wptr, short texnum );
short updatepnames( WAD *wptr, short texnum );
void init_textures( void );
void removetexwad( short texnum );
short addtexwad( WAD* wptr );
void texture_disp( short x, short y, short xw, short yw, short xofs,
        short yofs, TEXTURE *t, short xtile );

/* SECTOR.C                                                                */
short sector_edit( short object, short editmarked );
short sect_changeheight( short csr_object );

/* WASSIST.C                                                               */
void wcfg_save( void );
void wcfg_load( void );
short wa_configure( void );

/* CONFIG.C                                                                */
void main_config( void );

/* MSGBOX.C                                                                */
// ushort message_box( ushort flags, uchar *title, uchar *fmt, ... );

/* MENUBAR.C                                                               */
void mk_menus( void );
short mn_editmap( void );
short new_level( void );
int exit_confirm();
int key_help();
int about();
int MenuPopArray( int iStartItem, const char* pszTitle, const char* pszHelp,
    const char* pszItemArray[] );
int MenuPop( int iStartItem, const char* pszTitle, const char* pszHelp,
    ... );

/* CONSIST.C                                                               */
short cs_run( short force );
bool lineneedstag( int iType );

/* HELP.C                                                                  */
short help_init( char *fn );
short help_disp( char *kwd, short ydisp );

/* ADF.C                                                                   */
short so_pick( void );
short adf_load( void );
void so_set( short object, short so_num );
void so_apply( short object, short so_num );
void motif_set( short mno );
void motif_pick( void );
short sect_getmostobviousparent( short object );

/* STAIRS.C                                                                */
void stair_do( void );

/* Broadcast-receive functions                                             */
void _bf_consist( short msg );
void _bf_map( short code );

/* FPAINT.C                                                                */
ushort fpainter( short objtype, short source_obj );

/* CONFIG.C                                                                */
short load_cfg( uchar *fn );
uchar *getline_cfg( uchar *buf );
void countline_cfg( int *nLines, int *nCols );
void parsesecline_cfg( uchar *buf, uchar *secname, uchar *additional );
void parseline_cfg( uchar *buf, uchar *val1, ... );
void readconfig( void );
void getconfig( void );
void saveconfig( void );

/* GAMEDAT.C                                                               */
void game_switch( int gamenum );

/* STRTABLE.C                                                              */
void strtbl_del( STRTABLE *st, short item );
STRTABLE *strtbl_init( uchar *name, short maxrow, short maxcol );
void strtbl_clear( STRTABLE *st );
void strtbl_rpl( STRTABLE *st, short pos, uchar *s, short ident );
void strtbl_free( STRTABLE *st );
void strtbl_add( STRTABLE *st, uchar *s, short ident );
short strtbl_strtoid( STRTABLE *st, uchar *s );
short strtbl_find( STRTABLE *st, uchar *s );
char *strtbl_idtostr( STRTABLE *st, short id );
void strtbl_kwdset( STRTABLE *st, uchar *kwd );
uchar *strtbl_get( STRTABLE *st, uchar *s, short pos );
short strtbl_len( STRTABLE *st );
short strtbl_findnokey( STRTABLE *st, uchar *s );
void strtbl_sort( STRTABLE *st );
short strtbl_findid( STRTABLE *st, short id );

/* BSP.C                                                                   */
short build_nodes( void );

/* TXTMENU.C                                                               */
short bar_menu( char **sel, char *title, short start, short x, short y );

/* EDITSTR.C                                                               */
int editdir( char *__buf, int x, int y, unsigned char attr, int len );

/* TALIGN.CPP                                                              */
int TexAlign( int iTexture, int *iXpos, int *iYpos, bool fTopOrigin );
void line_align( short object );
void sector_align( short object );

// runacc.cpp:
int DosCommand( const char* pszCommand, ... );
int GetAnswer( const char* pszBuf, const char* pszChoices );

#define dResume()
#define dPaise()
#define dShutdown()
#define dInitPlayback(a)
#define dInitRecord(a)
