#include "std.h"
               
uchar strccpy( uchar *dest, uchar *src, uchar *ch1 )
/*-------------------------------------------------------------------------*
 * Copies 'src' into 'dest' until it reaches a char in 'ch1'.  Returns the *
 * character from ch1 it stopped copying at.                               *
 *-------------------------------------------------------------------------*/
{
    register int i, ilen = strlen( src ), i2, ilen2 = strlen( ch1 );

    for( i = 0; i < ilen; i++ )
    {
        for( i2 = 0; i2 < ilen2; i2++ )
        {
            if( *(src+i) == *(ch1+i2) )
            {
                *(dest+i) = 0;
                return *(ch1+i2);
            }
        }
        *(dest+i) = *(src+i);
    }

    *(dest+i) = 0;
    return 0;
}

int strcountchar(char *s, int c)
{
    int n = 0;
    char *p;

    for(p = s; *p; p++)
        if(c == *p)
            n++;

    return n;
}
