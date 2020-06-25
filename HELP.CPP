/*-------------------------------------------------------------------------*
 * HELP.C:                                                                 *
 * Functions for loading and reading a help file.                          *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

typedef struct
{
    char    kwd[33];
    ulong   ofs;
    short     nline;

} TOPIC;

#define MAXTOPIC                64

TOPIC   *index;                 /* Help index. */
short     ntopic;                 /* number of topics */
FILE    *helpfp;

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

short help_init( char *fn )
/*-------------------------------------------------------------------------*
 * Initializes the help system by opening filename, parsing it and getting *
 * all the keywords into index, closing the file and returning.            *
 *-------------------------------------------------------------------------*/
{
    char    buf[128];
    short     nline = 0;

    helpfp = fopen( fn, "rb" );

    if( helpfp == NULL )
        return 0;       /* error! */

    /* ALlocate index first */
    _memstr = "help index";
    index = (TOPIC*) memget( sizeof( TOPIC ) * MAXTOPIC );

    ntopic = 0;

    /* read in all the keywords */
    while( 1 )
    {
        if( fgets( buf, 128, helpfp ) == NULL )
        {
            index[ntopic-1].nline = nline;
            break;
        }

        /* Check buf */
        if( *buf == '*' )       /* Keyword! */
        {
            if( ntopic )
                index[ntopic-1].nline = nline;      /* Number of lines in previous topic */

            strccpy( index[ntopic].kwd, buf+1, "\r\n" );
            index[ntopic].ofs = ftell( helpfp );

            nline = 0;
            ++ntopic;
        }
        else
            ++nline;
    }

    /* Resize the index */
    index = (TOPIC*) realloc( index, ntopic * sizeof( TOPIC ) );

    return 1;
}

short help_disp( char *kwd, short ydisp )
/*-------------------------------------------------------------------------*
 * Displays some help.                                                     *
 *-------------------------------------------------------------------------*/
{
    short     i, rvl;
    char    **htext, buf[128];
    TOPIC   *t;

    /* Search for the topic */
    for( i = 0; i < ntopic; i++ )
    {
        if( !strcmpi( kwd, index[i].kwd ) )
            break;
    }

    if( i == ntopic )
        return 0;

    t = &index[i];

    /* Now read in the help */
    _memstr = "help string array";
    htext = (char**) memget( t->nline * sizeof( char * ) );
    fseek( helpfp, t->ofs, SEEK_SET );

    for( i = 0; i < t->nline; i++ )
    {
        fgets( buf, 128, helpfp );
        _memstr = "help string";
        htext[i] = (char*) memget( zstrlen( buf ) + 1 );
        strccpy( htext[i], buf, "\r\n" );
    }

    /* Now read it */
    rvl = readtext( htext[0], htext+2, t->nline-2, ydisp );

    for( i = t->nline-1; i >= 0; i-- )
        memfree( htext[i] );   /* In reverse! */

    memfree( htext );

    return rvl;
}
