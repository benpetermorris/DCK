#include <std.h>

void _ExceptInit( void ) {}

void main( int argc, char *argv[] )
{
    int iMegs = 4, iHandle, i;
    FILE *fp, *fp2;
    char *buf, buf2[128], buf3[16];

    puts( "DCK Swapfile Creation Utility, v1.0" );

    if( access( "DCK.BAT", 0 ) == -1 )
    {
        puts( "You must run this utility in DCK's directory." );
        return;
    }

    if( argc > 1 && !isdigit( argv[1][0] ) )
    {
        puts(
"Usage: MAKESWAP [size-in-megabytes]\n"
"\n"
"The default size of the swapfile is 4 megabytes. If you still have\n"
"\"out of memory\" problems after running MAKESWAP, try using a larger\n"
"number -- such as MAKESWAP 6 or MAKESWAP 8."
);
        return;
    }

    if( argc > 1 )
        iMegs = atoi( argv[1] );

    if( iMegs < 1 || iMegs > 10 )
    {
        puts( "Try entering a more valid number." );
        return;
    }

    strcpy( buf2, "þ Creating a " );
    itoa( iMegs, buf3, 10 );
    strcat( buf2, buf3 );
    strcat( buf2, " megabyte swapfile" );
    puts( buf2 );

    fp = fopen( "DCK.SWP", "wb" );
    buf = malloc( 32768 );
    memset( buf, 0, 32768 );

    while( iMegs-- )
    {
        for( i = 0; i < 32; i++ )
        {
            if( fwrite( buf, 32768, 1, fp ) != 1 )
            {
                puts( "  There's not enough disk space!" );
                fclose( fp );
                remove( "DCK.SWP" );
                return;
            }
        }
    }

    fclose( fp );

    // modify batch file
    puts( "þ Modifying DCK.BAT" );

    fp = fopen( "DCK.BA$", "wt" );

    getcwd( buf2, 128 );
    i = 0;

    fp2 = fopen( "DCK.BAT", "r" );
    while( fgets( buf, 128, fp2 ) != NULL )
    {
        // don't have two
        if( !strncmp( buf, "SET DPMI", 8 ) )
            continue;

        fputs( buf, fp );

        if( i == 0 )
        {
            fputs( "SET DPMIMEM=SWAPFILE ", fp );
            fputs( buf2, fp );
            fputs( "\\DCK.SWP\n", fp );
            i = 1;
        }
    }
    fclose( fp );
    fclose( fp2 );

    remove( "DCK.BAT" );
    rename( "DCK.BA$", "DCK.BAT" );

    puts( "þ Done." );
}
