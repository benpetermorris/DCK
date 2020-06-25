/*-------------------------------------------------------------------------*
 * MISC.C:                                                                 *
 * Miscellaneous functions                                                 *
 *-------------------------------------------------------------------------*/

#include "dck.h"
#pragma hdrstop

#include "gui\dialog.hpp"
#include "gui\menu.hpp"

#include <errno.h>

#include <math.h>

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code Begins Here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

#define RAD_DEG     (180.0 / M_PI)
#define DEG_RAD     (M_PI / 180.0)

void Unregistered()
{
    Dialog* pDialog = new Dialog;
    pDialog->Create( szDSFile, "UnRegDlg" );
    pDialog->Process();
    delete pDialog;
}

short isolder( struct ftime *ft1, struct ftime *ft2 )
/*
 * Returns TRUE if ft2 is older than ft1.
 */
{
    if( ft1->ft_year > ft2->ft_year )
            return TRUE;
    else if( ft1->ft_year == ft2->ft_year &&
        ft1->ft_month > ft2->ft_month )
            return TRUE;
    else if( ft1->ft_year == ft2->ft_year &&
        ft1->ft_month == ft2->ft_month &&
        ft1->ft_day > ft2->ft_day )
            return TRUE;
    else if( ft1->ft_year == ft2->ft_year &&
        ft1->ft_month == ft2->ft_month &&
        ft1->ft_day == ft2->ft_day &&
        ft1->ft_hour > ft2->ft_hour )
            return TRUE;
    else if( ft1->ft_year == ft2->ft_year &&
        ft1->ft_month == ft2->ft_month &&
        ft1->ft_day == ft2->ft_day &&
        ft1->ft_hour == ft2->ft_hour &&
        ft1->ft_min > ft2->ft_min )
            return TRUE;
    else if( ft1->ft_year == ft2->ft_year &&
        ft1->ft_month == ft2->ft_month &&
        ft1->ft_day == ft2->ft_day &&
        ft1->ft_hour == ft2->ft_hour &&
        ft1->ft_min == ft2->ft_min &&
        ft1->ft_tsec > ft2->ft_tsec )
            return TRUE;

    return 0;

}

short game_choose( void )
{
    const char *array[10];
    short   idx;

    for( idx = 0; idx < iMaxGame; idx++ )
        array[idx] = (const char*) pgdGameArray[idx].name;

    array[idx] = NULL;

    if( idx == 1 )
        return 0;

    return MenuPopArray( 0, " Choose a game ", NULL, array );
}

short islevelresource( char *name )
{
    if( !memcmp( name, "LINEDEFS", 8 ) ||
        !memcmp( name, "VERTEXES", 8 ) ||
        !memcmp( name, "THINGS", 6 ) ||
        !memcmp( name, "SIDEDEFS", 8 ) ||
        !memcmp( name, "SEGS", 4 ) ||
        !memcmp( name, "SSECTORS", 8 ) ||
        !memcmp( name, "NODES", 5 ) ||
        !memcmp( name, "SECTORS", 7 ) ||
        !memcmp( name, "REJECT", 6 ) ||
        !memcmp( name, "BEHAVIOR", 8 ) ||
        !memcmp( name, "BLOCKMAP", 8 ) )
        return 1;
    return 0;
}

short strsum( uchar *s )
{
    uchar   *p = s;
    short   idx = 0, sum = 0;

    while( *p )
    {
        sum += (*p++) + idx;
        sum ^= idx++;
    }

    return sum;
}

NODEPTR NodeFindData( NODEPTR head, void *data )
{
    NODEPTR ndp = NodeFirst( head );

    while( ndp != head )
    {
        if( ndp->data == data )
            return ndp;

        ndp = NodeNext( ndp );
    }

    return NULL;
}

short wncmp( char *s1, char *s2 )
/*-------------------------------------------------------------------------*
 * 0 same, 1 different.
 *-------------------------------------------------------------------------*/
{
    short i;

    for( i = 0; i < 8; i++ )
    {
        if( *s1 != *s2 )
            return 1;
        if( *s1 == 0 && *s2 == 0 )
            return 0;
        ++s1; ++s2;
    }

    return 0;
}

short wncmpi( char *s1, char *s2 )
/*-------------------------------------------------------------------------*
 * 0 same, 1 different.
 *-------------------------------------------------------------------------*/
{
    short i, a, b;

    for( i = 0; i < 8; i++ )
    {
        a = *s1++; b = *s2++;

        if( !a && !b )
            return 0;
        if( _mtoupper( a ) != _mtoupper( b ) )
            return 1;
    }

    return 0;
}

void wncpy( char *dest, char *src )
{
    short   i;

    /* Copy */
    for( i = 0; i < 8 && *src; i++ )
        *dest++ = *src++;

    /* Pad */
    while( i++ < 8 )
        *dest++ = 0;
}

short samefile( uchar *fn, uchar *fn2 )
{
    uchar   buf1[128], buf2[128];

    _fullpath( buf1, fn, 128 );
    _fullpath( buf2, fn, 128 );

    return !strcmpi( fn, fn2 );
}

uchar *searchfile( uchar *fn, uchar *path )
{
    uchar   *ptr;
    uchar   buf[128];
    static  uchar checkfn[128];

    strcpy( buf, path );

    if( lastchar( buf ) != ';' )
        strcat( buf, ";" );

    ptr = strtok( buf, ";" );

    while( ptr )
    {
        strcpy( checkfn, ptr );

        if( lastchar( ptr ) != '\\' )
            strcat( checkfn, "\\" );

        strcat( checkfn, fn );

        if( access( checkfn, 0 ) != -1 )
            return checkfn;

        ptr = strtok( NULL, ";" );
    }

    return NULL;
}

short open_file( uchar *fn, short level )
/*-------------------------------------------------------------------------*
 * Looks in some obvious places for a file:                                *
 *  1. current dir                                                         *
 *  2. mainwadloc paths                                                    *
 *  3. the path itself                                                     *
 *-------------------------------------------------------------------------*/
{
    uchar   *ptr;

    if( access( fn, 0 ) != -1 )
        return z_open( fn, level );
    else if( (ptr = searchfile( pgdCurGame->dir, fn )) != NULL )
        return z_open( ptr, level );
    else if( (ptr = searchfile( getenv( "PATH" ), fn )) != NULL )
        return z_open( ptr, level );

    return -1;
}

uchar *WN2Str( uchar *s )
{
    static  char    bufs[5][10];
    static  short   lastbuf = -1;

    ++lastbuf;

    if( lastbuf == 5 )
        lastbuf = 0;

    wncpy( bufs[lastbuf], s );
    bufs[lastbuf][8] = 0;

    return bufs[lastbuf];
}

/*-------------------------------------------------------------------------*
 * Error functions
 *-------------------------------------------------------------------------*/

static char     edebug[1024];
static short    elastpos[16];
static short    ecount;

void DEBUG_EXIT( void )
{
    if( ecount )
        edebug[elastpos[--ecount]] = 0;
}

void DEBUG_INIT( char *str, ... )
{
    char    buf[300];
    va_list vl;

    sprintf( buf, "%d. ", ecount+1 );

    va_start( vl, str );
    vsprintf( buf+strlen(buf), str, vl );
    va_end( vl );

    elastpos[ecount++] = strlen( edebug );

    if( *edebug )
        strcat( edebug, "\n" );
    strcat( edebug, buf );
}

void error( char *str, ... )
/*-------------------------------------------------------------------------*
 * Prints an error message and exits to DOS.                               *
 *-------------------------------------------------------------------------*/
{
    char buf[300];
    va_list vl;
    extern  short its_loaded;
    short   myerrno = errno;

    va_start( vl, str );
    vsprintf( buf, str, vl );
    va_end( vl );

    if( its_loaded )
    {
        remove( "_CRASH.WAD" );
        WadSave( "_CRASH.WAD", szCurMapTag, wsDataOnly );
    }

    /* Restore textmode */
    gr_vidmode( MODE_TEXT );
    gr_closegraph();

    printf( "\nError: %s\n", buf );

    if( *edebug )
        printf( "\nDebug log:\n%s\n", edebug );

    printf( "\n\"I hate it when that happens.\"\n" );

    if( its_loaded )
        printf( "(Saved work to %s in _CRASH.WAD.)\n", szCurMapTag );

    if( myerrno == EMFILE || myerrno == ENFILE )
    {
        printf( "\nThis error occured because you don't have enough\n"
                "File Handles. Edit the FILES= number in CONFIG.SYS\n"
                "and set it to 20 or higher and reboot your system.\n"
                "(Refer to your DOS manual if you're not sure how\n"
                "to edit CONFIG.SYS)\n" );
    }
    else if( strinc( "allocate", str ) )
    {
        char buf[128];

        getcwd( buf, 128 );
        strcat( buf, "> MAKESWAP" );
        strlwr( buf );

        // memory error.
        printf( "\n"
                "This error occured because you don't have enough free\n"
                "system memory. You can fix this problem easily by\n"
                "running the MAKESWAP utility right now, like this:\n"
                "\n"
                "  %s\n"
                "\n"
                "After you run MAKESWAP, run DCK again.\n", buf );
    }
    else
    {
        // other errors
        printf( "\nPlease read TROUBLE.DOC if this is a recurring error.\n" );
    }

    exit( EL_EXIT );
}

void chkrange( char *name, long value, long lowest, long highest )
{
    if( value < lowest || value > highest )
        error( "%s must have a value between %ld and %ld",
               name, lowest, highest );
}

void *memresize( void *old, size_t length )
/*-------------------------------------------------------------------------*
 * Resizes memory.
 *-------------------------------------------------------------------------*/
{
    void *ptr = realloc( old, length );

    if( ptr == NULL && length )
    {
        if( _memstr == NULL )
            error( "memresize: can't reallocate %d bytes", length );
        else
            error( "memresize: can't reallocate %d bytes for %s", length, _memstr );
    }

    return ptr;
}

void *memget( size_t length )
/*-------------------------------------------------------------------------*
 * Allocates memory.                                                       *
 *-------------------------------------------------------------------------*/
{
    void *ptr = malloc( length );

    if( ptr == NULL && length )
    {
        if( _memstr == NULL )
            error( "memget: can't allocate %d bytes", length );
        else
            error( "memget: can't allocate %d bytes for %s", length, _memstr );
    }

    memset( ptr, 0, length );

    _memstr = NULL;
    return ptr;
}

void *memgetmax( size_t *length )
/*-------------------------------------------------------------------------*
 * Attempts to allocate length.  If it can't, decreases it by 2048         *
 * and tries again -> repeat.                                              *
 *-------------------------------------------------------------------------*/
{
    void *ptr;
    ushort orig = *length;

    while( *length > 2048 )
    {
        ptr = malloc( *length );

        if( ptr )
        {
            _memstr = NULL;
            return ptr;
        }

        *length -= 2048;
    }

    if( _memstr == NULL )
        error( "memgetmax: couldn't allocate <= %d bytes", orig );
    else
        error( "memgetmax: couldn't allocate <= %d bytes for %s", orig, _memstr );

    return NULL;    /* never happens anyway */
}

void memfree( void *ptr )
/*-------------------------------------------------------------------------*
 * Frees memory                                                            *
 *-------------------------------------------------------------------------*/
{
    if( ptr == NULL )
        error( "memfree: attempt to free NULL" );
    else
        free( ptr );
}

short getcfgline( char *buf, FILE *fp )
/*-------------------------------------------------------------------------*
 * Gets the next 'valid' line from fp into buf and returns 1, or 0 if no   *
 * more lines.                                                             *
 *-------------------------------------------------------------------------*/
{
    char *ptr;

    while( 1 )
    {
        ptr = buf;

        if( fgets( buf, 128, fp ) == NULL )
            return 0;

        while( isspace( *ptr ) )
            ptr++;

        if( *ptr && *ptr != ';' )
            break;
    }

    strccpy( buf, ptr, "\r\n" );

    return 1;
}

short getcfgint( FILE *fp )
/*-------------------------------------------------------------------------*
 * Gets the enxt integer from fp.                                          *
 *-------------------------------------------------------------------------*/
{
    char buf[128];
    if( getcfgline( buf, fp ) )
        return atoi( buf );
    else
        return -32000;
}

void NukeLinkedList( NODEPTR head_ndp )
{
    NODEPTR ndp = NodeLast( head_ndp ), prev;
    short     breakit = 0;

    if( ndp == head_ndp )
    {
        memfree( head_ndp );
        return;
    }

    for( ;; )
    {
        /* Get previous node */
        prev = NodePrev( ndp );
        /* If the previous is the head, we break after freeing it. */
        if( prev == head_ndp )
            breakit = 1;
        if( ndp->data )
            memfree( ndp->data );
        memfree( ndp );
        if( breakit )
            break;
        ndp = prev;
    }

    memfree( head_ndp );
}

short check_save( void )
/*-------------------------------------------------------------------------*
 * If the level has been modified, pops up a dialog box and asks the user  *
 * if they'd like to save the changes.  If not, returns 1.  If so, saves   *
 * the changes and returns 1.  If Cancel was pushed, returns 0.            *
 *-------------------------------------------------------------------------*/
{
    short     rvl;

    if( !nMadeChanges )
        return 1;

__again:
    rvl = message_box( MB_YESNOCANCEL, "Current file has changed!",
                       "Would you like to save the changes\n"
                       "before proceeding?" );
    if( rvl == MB_YES )       /* "Yes" */
    {
        /* Save changes */
        if( !file_save( FALSE, FALSE ) )
            goto __again;
        return 1;
    }

    if( rvl == MB_CANCEL )      /* "Cancel" */
        return 0;

    return 1;       /* "No" */
}

void setscale( short __xscale )
{
    extern short xend, yend;

    xscale = __xscale;
    yscale = (unsigned)(((long)myplen * 1000L) / (((long)mxplen * 1000L) / (long)xscale));

    if( tog_vradius )
    {
        if( xscale > 2000 )
            _vradius = 0;
        else if( xscale > 1500 )
            _vradius = 1;
        else if( xscale > 1000 )
            _vradius = 2;
        else if( xscale > 500 )
            _vradius = 3;
        else
            _vradius = 4;
    }

    xend = xstart + xscale;
    yend = ystart + yscale;

    sc_x = (((long)mxplen << MSCALE) / (long)xscale);
    sc_y = (((long)myplen << MSCALE) / (long)yscale);
    sc_x_r = (((long)xscale << MSCALE) / (long)mxplen);
    sc_y_r = (((long)yscale << MSCALE) / (long)myplen);
}

short line_len( short x1, short y1, short x2, short y2 )
{
    long    xl, yl;

    xl = (x2-x1);
    xl = abs( xl );
    xl *= xl;
    yl = (y2-y1);
    yl = abs( yl );
    yl *= yl;
    return (short) sqrt( (float)xl + (float)yl );
}

int     pmPoints[pmMaxPoints+1][2];

void polyMake( short x1, short y1, short x2, short y2, short npoints, short start_ang )
{
    short   point;
    double  angle = start_ang, angle_delta = 360.0 / (double) npoints;
    double  xrad = (x2-x1) / 2, yrad = (y2-y1) / 2;

    /* start off as top, left - add radius */
    x1 += xrad;
    y1 += yrad;

    for( point = 0; point < npoints; point++, angle += angle_delta )
    {
        if( angle > 360 )
            angle -= 360;

        pmPoints[point][0] = x1 + (short)(sin( angle * DEG_RAD ) * (float)xrad);
        pmPoints[point][1] = y1 + (short)(cos( angle * DEG_RAD ) * (float)yrad);
    }

    pmPoints[point][0] = pmPoints[0][0];
    pmPoints[point][1] = pmPoints[0][1];
}

void rs_coords( short *dx, short *dy, short sx, short sy,
                double angle, double scale )
{
    double r, theta;

    if( !sx && !sy )
    {
        *dx = 0;
        *dy = 0;
        return;
    }

    angle *= 0.0174533;

    r = hypot( (double) sx, (double) sy );
    theta = atan2( (double) sy, (double) sx );
    *dx = (short) (r * scale * cos( theta + angle ) + 0.5 );
    *dy = (short) (r * scale * sin( theta + angle ) + 0.5 );
}

short _angle( short x1, short y1, short x2, short y2 )
{
    float   x, y;
    short   rvl;

    x = x2 - x1;
    y = y2 - y1;

    if( !x && !y )
        return 0;

    rvl = (short) (atan2( y, x ) * RAD_DEG);

    if( rvl < 0 )
        rvl += 360;

    return rvl;
}

short obj_lineangle( short object )
{
    LINEDEF *ldp = &linelist[object];

    return _angle( vlist[ldp->vfrom].x, vlist[ldp->vfrom].y,
                   vlist[ldp->vend].x, vlist[ldp->vend].y );
}

void _bf_map( short code )
{
    switch( code )
    {
    case BC_CLEANFILE:
        fNewMap = TRUE;
        break;
    }
}

short firstfreetag( void )
/*-------------------------------------------------------------------------*
 * Searches for the first free tag.                                        *
 *-------------------------------------------------------------------------*/
{
    short tag = 0, i, done = 0;

    while( !done )
    {
        ++tag;
        done = 1;

        for( i = 0; i < nline; i++ )
        {
            if( lSecTag( &linelist[i] ) == tag )
            {
                done = 0;
                break;
            }
        }

        for( i = 0; i < nsector; i++ )
        {
            if( sectorlist[i].sectag == tag )
            {
                done = 0;
                break;
            }
        }
    }

    return tag;
}

short firstfreetid()
/*-------------------------------------------------------------------------*
 * Searches for the first free tid.                                        *
 *-------------------------------------------------------------------------*/
{
    short tid = 0, i, done = 0;

    while( !done )
    {
        ++tid;
        done = 1;

        for( i = 0; i < nthing; i++ )
        {
            if( thinglist[i].tid == tid )
            {
                done = 0;
                break;
            }
        }
    }

    return tid;
}

short firstfreepolyid()
/*-------------------------------------------------------------------------*
 * Searches for the first free polyid.
 *-------------------------------------------------------------------------*/
{
    short polyid = 0, i, done = 0;

    while( !done )
    {
        ++polyid;
        done = 1;

        for( i = 0; i < nthing; i++ )
        {
            if( inrange( thinglist[i].type, 3000, 3002 ) )
            {
                if( thinglist[i].angle == polyid )
                {
                    done = 0;
                    break;
                }
            }
        }
    }

    return polyid;
}


short calc_percent( long current, long total, short maximum )
/*-------------------------------------------------------------------------*
 * Scales current:total to maximum
 *-------------------------------------------------------------------------*/
{
    long _total, _current, num;

    _total = total;
    _current = current;

    if( _total <= _current )
        return maximum;

    if( !_total )
        return maximum;

    num = _current*100/_total;
    num *= 100;
    num *= (long)maximum;
    num /= 100;
    num /= 100;

    return (short)num;
}

void ResetKeys()
/*-------------------------------------------------------------------------*
 * Sets all the gamedata string list keywords to NULL.
 *-------------------------------------------------------------------------*/
{
    stlThingTypes->SetKey( NULL );
    stlThingSections->SetKey( NULL );
    stlSectorTypes->SetKey( NULL );
    stlActTypes->SetKey( NULL );
    stlActSections->SetKey( NULL );
    stlAngles->SetKey( NULL );
    stlFlats->SetKey( NULL );
}

long freespace( short drive )
/*-------------------------------------------------------------------------*
 * Guess.
 *-------------------------------------------------------------------------*/
{
    struct  diskfree_t df;
    long    avail;

    _dos_getdiskfree( drive, &df );
    avail = (long) df.avail_clusters * (long) df.sectors_per_cluster * (long) df.bytes_per_sector;

    return avail;
}

short isnodeexist( void )
/*-------------------------------------------------------------------------*
 * If the node structures for the current map are available and up-to-date *
 * returns 1.  Otherwise, returns .. guess!  0.                            *
 *-------------------------------------------------------------------------*/
{
    if( nMapChanges && pnodes != NULL )
    {
        /* free the nodes. */
        map_freedata( F_PRECALC );
    }

    return( !nMapChanges && pnodes != NULL );
}

short usage( void )
/*-------------------------------------------------------------------------*
 * Displays a dialog box with the amount of information used, etc.         *
 *-------------------------------------------------------------------------*/
{
    char    lines[16][80];      /* Look how big you make my.. stack! */
    short   n = 0, i, tmp, width;
    long    k;

    k = (long) (sizeof( LINEDEF ) * nline) +
        (long) ((long) sizeof( SIDEDEF ) * (long) nside) +
        (long) (sizeof( VERTEX ) * nvertex) +
        (long) (sizeof( SECTOR ) * nsector) +
        (long) (sizeof( THING ) * nthing) +
        (long) (sizeof( WADENTRY ) * nMapEntryCount);

    k /= 1024L;

#ifndef HEXEN
    sprintf( lines[n++], "DOOM Construction Kit v%s (Protected Mode)", st_version );
#else
    sprintf( lines[n++], "HEXEN Construction Kit v%s (Protected Mode)", st_version );
#endif
    sprintf( lines[n++], "(c) Copyright 1995 Ben Morris and Zerius Development" );
    sprintf( lines[n++], " Free memory:     %s bytes", strcomma( coreleft() ) );
    sprintf( lines[n++], " Free disk space: %s bytes (drive %c:)",
                          strcomma( freespace( getdisk()+1 ) ), getdisk() + 'A' );
    sprintf( lines[n++], " Video:           %dx%dx%d", sxlen+1, sylen+1, getmaxcolor()+1 );

    sprintf( lines[n++], "" );
    sprintf( lines[n++], "WAD Information (%lu k) [%s]:", k, szCurMapTag );
    sprintf( lines[n++], " From the file:   %s", pwdCurWad->fn );
    sprintf( lines[n++], " Linedefs:        %d (%d free)", nline, maxline-nline );
    sprintf( lines[n++], " Sectors:         %d (%d free)", nsector, maxsector-nsector );
    sprintf( lines[n++], " Sidedefs:        %d (%d free)", nside, maxside-nside );
    sprintf( lines[n++], " Vertices:        %d (%d free)", nvertex, maxvertex-nvertex );
    sprintf( lines[n++], " Things:          %d (%d free)", nthing, maxthing-nthing );
    sprintf( lines[n++], " Precalculations: %s",
             isnodeexist() ? "Are up-to-date" : "Need rebuilding" );

    width = 0;

    for( i = 0; i < n; i++ )
    {
        if( (tmp = strlen( lines[i] )) > width )
            width = tmp;
    }

    width += 4;

    /* Now, create the dialog box */
    Dialog* pDialog = new Dialog;
    pDialog->Create( Point( Centered, Centered ),
        Size( width * XUPC, (n+3) * YUPC ), "Information" );

    for( i = 0; i < n; i++ )
        new stText( pDialog, Point( 4, i * YUPC + 4 ), lines[i] );

    Size szSize = pDialog->ClientSize();

    new fldButton( pDialog, Rect( szSize.width - 34, szSize.height - 18,
        szSize.width - 4, szSize.height - 4 ), "Ok", (FFUNC) pbOkay );

    pDialog->Process();
    delete pDialog;

    return maExit;     /* Used in menus too! */
}

/*-------------------------------------------------------------------------*
 *
 * show a tip function
 *
 *-------------------------------------------------------------------------*/

#include <fstream.h>

void ShowTip()
{
    ifstream inf( "TIPS.DAT" );

    int nTipCount, iTip;

    inf >> nTipCount;

    iTip = rand() % nTipCount;

    char szLine[128], szBuffer[2048];

    for( int i = 0; ; )
    {
        inf.getline( szLine, 128 );
        if( szLine[0] == '-' )
        {
            if( i == iTip )
            {
                break;
            }
            ++i;
        }

        if( inf.eof() )
            return;
    }

    strcpy( szBuffer, szLine+2 ); // nuke the "- "

    // get the rest of the tip

    while( 1 )
    {
        inf.getline( szLine, 128 );

        if( szLine[0] == '-' || inf.eof() )
        {
            // done
            break;
        }

        strcat( szBuffer, "\n" );
        strcat( szBuffer, szLine );
    }

    message_box( MB_OK, "Did you know...", szBuffer );
}

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Run-this functions.                                                     *
 *                                                                         *
 *-------------------------------------------------------------------------*/

uchar   *rt_parms[32];
uchar   *rt_contents[32];
short   rt_nparm;

void rt_setparm( char *parm, char *contents )
{
    short   i;
    uchar   *ptr = NULL;

    for( i = 0; i < rt_nparm; i++ )
    {
        if( !strcmpi( rt_parms[i], parm ) )
        {
            ptr = rt_contents[i];
            break;
        }
    }

    /* Do we need to add it? */
    if( ptr == NULL )
    {
        rt_parms[i] = new uchar[zstrlen( parm ) + 1];
        strcpy( rt_parms[i], parm );
        ptr = rt_contents[rt_nparm] = new uchar[128];
        rt_nparm++;
    }

    /* Copy it in */
    strcpy( ptr, contents );
}

void rt_replace( char* szBuffer )
{
    for( int i = 0; i < rt_nparm; i++ )
        strichangestr( szBuffer, rt_parms[i], rt_contents[i] );
}

void setfilertparms()
{
    char    tmpstr[128];
    extern  short tagnum[];
    char    fn[128];

    strcpy( fn, pwdCurWad->fn );

    if( striinc( pgdCurGame->pwdIWAD, fn ) )
    {
        rt_setparm( "$FULL", "" );
        rt_setparm( "$FILE", "" );
        rt_setparm( "$PATH", "" );
    }
    else
    {
        rt_setparm( "$FULL", fn );
        fnchop( tmpstr, fn, FNC_FILE | FNC_EXTENSION );
        rt_setparm( "$FILE", tmpstr );
        fnchop( tmpstr, fn, FNC_DRIVE | FNC_DIRECTORY );
        rt_setparm( "$PATH", tmpstr );
    }

    ismaptag( szCurMapTag );       /* sets "tagnum" */
    sprintf( tmpstr, "%d", tagnum[0] );
    rt_setparm( "$NUM1", tmpstr );
    sprintf( tmpstr, "%d", tagnum[1] );
    rt_setparm( "$NUM2", tmpstr );

    rt_setparm( "$MAP", szCurMapTag );
}

/*-------------------------------------------------------------------------*
 * Broadcast functions.                                                    *
 *-------------------------------------------------------------------------*/

BFUNC   bf_array[32];
short     bf_num;     /* number registered */

void bc_register( BFUNC fp )
/* Register another broadcast function.                                    */
{
    bf_array[bf_num] = (BFUNC) fp;
    ++bf_num;
}

void broadcast( short msg )
/* Sends msg to all the registered functions.                              */
{
    short     i;

    for( i = 0; i < bf_num; i++ )
        (*(bf_array[i]))(msg);
}
