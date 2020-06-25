/*- Doom Structures .. Colin Reed 1994 -------------------------------------*/

typedef struct
{
	short minx;
	short miny;
	short xblocks;
	short yblocks;
} Block;

/*--------------------------------------------------------------------------*
  These are the structure used for creating the NODE bsp tree.
 *--------------------------------------------------------------------------*/

typedef struct tagSeg
{
   short int start;     /* from this vertex ... */
   short int end;       /* ... to this vertex */
   unsigned short angle;/* angle (0 = east, 16384 = north, ...) */
   short int linedef;   /* linedef that this seg goes along*/
   short int flip;      /* true if not the same direction as linedef */
   unsigned short dist; /* distance from starting point */
   struct tagSeg *next;
} Seg;

typedef struct
{
   short int start;     /* from this vertex ... */
   short int end;       /* ... to this vertex */
   unsigned short angle;/* angle (0 = east, 16384 = north, ...) */
   short int linedef;   /* linedef that this seg goes along*/
   short int flip;      /* true if not the same direction as linedef */
   unsigned short dist; /* distance from starting point */
} Pseg;

typedef struct tagNode
{
   short int x, y;									/* starting point*/
   short int dx, dy;									/* offset to ending point*/
   short int maxy1, miny1, minx1, maxx1;		/* bounding rectangle 1*/
   short int maxy2, miny2, minx2, maxx2;		/* bounding rectangle 2*/
   short int chright, chleft;						/* Node or SSector (if high bit is set)*/
   struct tagNode *nextr,*nextl;
   short int node_num;                             /* starting at 0 (but reversed when done)*/
} Node;

typedef struct
{
   short int x, y;									/* starting point*/
   short int dx, dy;									/* offset to ending point*/
   short int maxy1, miny1, minx1, maxx1;		/* bounding rectangle 1*/
   short int maxy2, miny2, minx2, maxx2;		/* bounding rectangle 2*/
   short int chright, chleft;						/* Node or SSector (if high bit is set)*/
} Pnode;

typedef struct
{
   short int num;       /* number of Segs in this Sub-Sector */
   short int first;     /* first Seg */
} SSector;

typedef struct
{
	short int halfx;
	short int halfy;
	short int halfsx;
	short int halfsy;
} splitter;

/*--------------------------------------------------------------------------*/
