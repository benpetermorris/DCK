/*-------------------------------------------------------------------------*
 * In the original wad files                                               *
 *-------------------------------------------------------------------------*/

typedef struct
{
    WADNAME name;
    short   notused1, notused2;
    short   width, height;
    short   notused3, notused4;
    short   ndescrip;

} TEXMETA;

typedef struct
{
    short   xofs, yofs;
    short   pentry;
    short   notused1, notused2;

} TEXDESCRIP;

/*-------------------------------------------------------------------------*
 * In memory                                                               *
 *-------------------------------------------------------------------------*/

typedef struct
{
    WADNAME     name;
    short       width, height;
    uchar       ndescrip;
    short       wadnumber;      /* where was this texture defined? */
    TEXDESCRIP  *dlist;

} TEXTURE;
