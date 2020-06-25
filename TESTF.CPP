#include <io.h>
#include <dos.h>
#include <fcntl.h>

int num_filehan(void)
/* Counts the number of file handles. */
{
    union REGS regs;
    struct SREGS sregs;
    unsigned short *ptr;
    int n = 0;

    regs.h.ah = 0x52;
    segread( &sregs );
    intdosx( &regs, &regs, &sregs );

    ptr = (unsigned short *) MK_FP( sregs.es, regs.x.bx + 4 );
    ptr = (unsigned short *) MK_FP( ptr[1], ptr[0] );

    while ( FP_OFF(ptr) != 0xFFFF )
    {
        n += *(ptr+2);
        ptr = (unsigned short *) MK_FP( ptr[1], ptr[0] );
    }

    return n;
}

int num_freehan( int req, char *fn )
{
    short handles[255];
    short i, freehan = 0;

    for( i = 0; i < req && i < 255; i++ )
    {
        handles[i] = open( fn, O_RDONLY );
        if( handles[i] == -1 )
            break;
        freehan++;
    }

    for( --i; i >= 0; i-- )
        close( handles[i] );

    return freehan >= req;
}

void main( int argc, char *argv[] )
{
    printf( "free handles >= 10? -- %d\n", num_freehan( 10, argv[0] ) );
    printf( "file handles = %d\n", num_filehan() );
}
