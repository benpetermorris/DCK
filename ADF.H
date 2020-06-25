/*-------------------------------------------------------------------------*
 * Definitions for automation definiiton files                             *
 *-------------------------------------------------------------------------*/

#define SO_MAXPARM      2
#define SOP_NONE        0
#define SOP_INTEGER     1
#define SOP_STRING      2
#define SOP_ADJTYPE     3
#define SOP_BSTRING     4
#define TOKLEN          16

#define SEC_SOBJECT     1
#define SEC_MOTIF       2

#define ADJ_CEILING     0
#define ADJ_FLOOR       1
#define ADJ_REAL        2
#define ADJ_ADJACENT    3

typedef struct
{
    uchar   name[TOKLEN];
    short   type;
    uchar   sdata[TOKLEN];
    short   idata;

} MO_PARM;

typedef struct
{
    uchar   *name;          /* Name of the parm, ie: "Floor." */
    uchar   p1type;
    uchar   p2type;
    uchar   cmdno;

} SO_LINETABLE;

enum    sc_commands
{
    SC_FLOOR,
    SC_FLOORTEX,
    SC_LIGHT,
    SC_BLTYPE,
    SC_OBJINSERT,
    SC_CEIL,
    SC_CEILTEX,
    SC_SECTYPE,
    SC_BLLTEX,
    SC_BLUTEX,
    SC_BLFTEX,
    SC_SECTAG,
    SC_BLSETTAG
};

typedef struct
{
    short   type;           /* Section type; motif, sectobject, etc */
    char    name[TOKLEN];   /* Name of the section */
    short   ncmd;           /* Number of commands */

} ADF_SECTION;

/*-------------------------------------------------------------------------*
 * This goes in the compiled file:                                         *
 *  (For each object): Number of commands    (WORD)                        *
 *                     { Command List }                                    *
 *-------------------------------------------------------------------------*/

typedef struct
{
    uchar   type;               /* Type. */
    uchar   sdata[TOKLEN];      /* parameter data */
    short   idata;

} SO_PARM;

typedef struct
{
    uchar   cmd;            /* command number (from so_table) */
    SO_PARM p[SO_MAXPARM];  /* parameters */

} SO_COMMAND;
