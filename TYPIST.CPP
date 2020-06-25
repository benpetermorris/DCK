#include <std.h>
#include <iostream.h>

const char *pszKeyMatrix[4][2] =
{
    { "`1234567890-=",      "~!@#$%^&*()_+" },
    { "qwertyuiop[]\\",     "QWERTYUIOP{}|" },
    { "asdfghjkl;'\0\0",    "ASDFGHJKL:\"\0\0" },
    { "zxcvbnm,./\0\0\0",   "ZXCVBNM<>?\0\0\0" },
};

enum
{
    errLeft = 0x01,
    errRight = 0x02,
    errTop = 0x04,
    errBottom = 0x08
};

void typeist( const char* pszString )
{
    const char* pStr = pszString;
    int   iErrorBack, iMode = 0, iFindError;
    bool  fGetOut = FALSE;
    bool  fMadeError = FALSE;

    randomize();

    while( 1 )
    {
        if( fGetOut )
        {
            if( fMadeError )
                iMode = 1;
            else
                break;
        }

        switch( iMode )
        {
        case 0:     // typing

            if( pStr[0] == 0 )
            {
                fGetOut = TRUE;
                break;
            }

            if( pStr[0] == '\a' )
            {
                if( fMadeError )
                    iMode = 1;
                else
                    delay( 1000 );
                ++pStr;
                break;
            }

            if( pStr[0] == '\n' )
            {
                if( fMadeError )
                    iMode = 1;
                else
                    printf( "\n" );
                ++pStr;
                break;
            }

            delay( 80 + rand() % 70 );
            // make an error?
            if( !fMadeError && (rand() % 12) == 3 )
            {
                char ch = pStr[0];
                const char* pMatrix;
                bool fShifted = FALSE;
                int x, y, xdelta, ydelta;
                unsigned uErrorFlags;
                for( int i = 0; i < 4; i++ )
                {
                    if( pMatrix = strchr( pszKeyMatrix[i][0], ch ) )
                    {
                        fShifted = FALSE;
                        y = i;
                        x = (int) (pMatrix - pszKeyMatrix[y][0]);
                        break;
                    }
                    if( pMatrix = strchr( pszKeyMatrix[i][1], ch ) )
                    {
                        fShifted = TRUE;
                        y = i;
                        x = (int) (pMatrix - pszKeyMatrix[y][1]);
                        break;
                    }
                }

                if( i == 4 )    // no error, couldn't find it
                {
                    putch( ch );
                }
                else
                {
                    uErrorFlags = rand() % 4;

                    // found it, so let's make an error!
                    if( !x )    // right only
                        xdelta = (uErrorFlags & errLeft) ? 1 : 0;
                    else
                        xdelta = (uErrorFlags & errLeft) ? 1 :
                                    ((uErrorFlags & errRight) ? -1 : 0);

                    if( !y )    // right only
                        ydelta = (uErrorFlags & errTop) ? 1 : 0;
                    else
                        ydelta = (uErrorFlags & errTop) ? 1 :
                                    ((uErrorFlags & errBottom) ? -1 : 0);

                    ch = pszKeyMatrix[y + ydelta][fShifted][x + xdelta];

                    if( !isprint( ch ) )
                    {
                        putch( ch = pStr[0] );
                    }
                    else
                    {
                        putch( ch );
                    }

                    if( ch != pStr[0] )
                    {
                        fMadeError = TRUE;
                        iErrorBack = 0;
                        iFindError = rand() % 10;
                    }
                }
            }
            else
            {
                putch( pStr[0] );
            }

            ++pStr;

            ++iErrorBack;

            if( fMadeError )
            {
                if( iFindError == iErrorBack )
                    iMode = 1;  // find that error
            }

            break;

        case 1:
            if( !fMadeError )
                break;

            delay( 200 );

            while( iErrorBack-- )
            {
                putch( '\b' );
                putch( 32 );
                putch( '\b' );

                delay( 50 );

                pStr--;
            }

            fMadeError = FALSE;
            iErrorBack = 0;
            iMode = 0;

            break;
        }
    }
}

void main()
{
    typeist( "Hey, you! \a Yeah, you! \a Don't forget to register HCK - it's cheap,\n"
             "and you'll get a bunch more cool features and get rid of this snide message.\n"
             "Wouldn't that be a treat? See the file ORDER.FRM for more information.\n" );
}
