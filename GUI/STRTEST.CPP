#include <std.h>
#include "strlist.hpp"

void main( void )
{
    FILE *fp = fopen( "TEST.DOC", "rb" );
    char buf[128];

    StrList stb( 512, 128 );

    while( 1 )
    {
        if( fgets( buf, 128, fp ) == NULL )
            break;

        stb.AddTail( buf );
    }

    fclose( fp );

    for( int i = 0; i < stb.GetCount(); i++ )
    {
        printf( "%s", stb[i] );
    }
}
