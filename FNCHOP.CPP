#include "zlib.h"

unsigned fnchop( uchar *dest, uchar *src, unsigned sticker )
/*-------------------------------------------------------------------------*
 * Chops up a standard complete DOS filename and sticks it back together   *
 * using the options specified in sticker.                                 *
 * Returns a combination of the options that were specified and that were  *
 * found.                                                                  *
 *-------------------------------------------------------------------------*/
{
    unsigned    rvl = 0;
    size_t      n;
    uchar       *sptr = src, *tptr, doret = 0, tbuf[20];

    /* Set it to blank first */
    *dest = 0;

/* Copy over the DRIVE if there is one                                     */
    /* Check for drive in source */
    if( *(sptr+1) == ':' )
    {
        /* Do we want to copy it, though? */
        if( sticker & FNC_DRIVE )
        {
            strncpy( dest, sptr, 2 );     /* Yep! */
            dest[2] = 0;
            rvl |= FNC_DRIVE;
        }
        sptr += 2;
    }

/* Copy over the DIRECTORY if there is one                                 */
    /* Copy from the beginning of sptr 'till we reach the LAST backspace
     * character. */
    tptr = strrchr( (char*) sptr, '\\' );
    n = (size_t) (tptr-sptr) + 1;

    /* If there's nothing after the last backslash, assume that
     * there's no filename and DON'T copy over the last \.
     * otherwise DO copy it because we're gonna stick a filename
     * at the end. */
    if( tptr && !(*(sptr+n)) )
    {
        if( n > 1 )
            --n;
        doret = 1;
    }
    else
        if( !(sticker & (FNC_FILE | FNC_EXTENSION)) && n > 1 )
            --n;

    /* Copy it over? */
    if( tptr && (sticker & FNC_DIRECTORY) )
    {
        strncat( dest, sptr, n );
        rvl |= FNC_DIRECTORY;
    }

    if( tptr )
        sptr += n;

    if( doret )
        return rvl;

/* Copy over the FILE if there is one                                      */
    /* If *sptr, there's a filename. */
    if( *sptr )
    {
        /* Copy until we reach a '.' */
        doret = strccpy( tbuf, sptr, "." );

        if( sticker & FNC_FILE )
        {
            strcat( dest, tbuf );
            rvl |= FNC_FILE;
        }

        /* If we didn't stop on a period there's no extension to
         * copy so just return. */
        if( doret != '.' )
            return rvl;
    }
    else return rvl;

    sptr += strlen( tbuf ) + 1;

/* Finally copy over the extension                                         */
    if( sticker & FNC_EXTENSION )
    {
        if( rvl & FNC_FILE )
            strcat( dest, "." );
        strcat( dest, sptr );
        rvl |= FNC_EXTENSION;
    }

    return rvl;
}
