
#include "dck.h"
#pragma hdrstop

static int ispathname( char ch )
/* Returns 1 if 'ch' is a valid pathname character */
{
    switch( ch )
    {
    case ' ':                   /* Check for illegal characters  */
    case '^':
    case '+':
    case '=':
    case '/':
    case '[':
    case ']':
    case '\"':
    case ';':
    case ',':
    case '?':
    case '*':
    case '<':
    case '>':
    case '|':
        return 0;
    default:
        return 1;
    }
}

static void inschinstr( char *s, uchar ch, uchar pos )
/* Insters character 'ch' into 's' at 'pos' */
{
    register int i, ilen = (uchar) strlen(s);

    for( i = ilen+1; i > pos; i-- )
        *(s+i) = *(s+i-1);

    *(s+pos) = ch;
}

int editdir( char *__buf, int x, int y, unsigned char attr, int len )
// returns 0 or 1  .. ESC or ENTER.
{
    int ch, ins_mode = 1, firstchar = 1, buflen = strlen( __buf ), i;
    int xpos = 0, rvl = -1;
    char buf[128];

    strcpy( buf, __buf );

    // init ..
    textattr( attr );
    gotoxy( x, y );
    for( i = 0; i < len+2; i++ )
        putch( 32 );
    ++x;
    gotoxy( x, y );

    // loop!
    while( rvl == -1 )
    {
        ch = keyget();

        switch( ch )
        {
        case KEY_ESC:       // dein aborten!
            rvl = 0;
            break;
        case KEY_ENTER:     // dein accepten!
            strcpy( __buf, buf );
            rvl = 1;
            break;
        case KEY_LEFT:
            if( xpos )
                xpos--;
            gotoxy( x+xpos, y );
            break;
        case KEY_RIGHT:
            if( xpos < buflen )
                xpos++;
            gotoxy( x+xpos, y );
            break;
        case KEY_HOME:
            xpos = 0;
            gotoxy( x+xpos, y );
            break;
        case KEY_END:
            xpos = buflen;
            gotoxy( x+xpos, y );
            break;
        case KEY_INS:
            ins_mode = !ins_mode;
            break;
        case KEY_DEL:
            if( xpos != buflen )
            {
                for( i = xpos+1; buf[i]; i++ )
                    buf[i-1] = buf[i];
                buf[i-1] = 0;
                gotoxy( x+xpos, y );
                cputs( buf+xpos );
                putch( ' ' );
                gotoxy( x+xpos, y );
                --buflen;
            }
            break;
        case KEY_BACK:
            if( xpos != 0 )
            {
                --xpos;
                for( i = xpos+1; buf[i]; i++ )
                    buf[i-1] = buf[i];
                buf[i-1] = 0;
                gotoxy( x+xpos, y );
                cputs( buf+xpos );
                putch( ' ' );
                gotoxy( x+xpos, y );
                --buflen;
            }
            break;
        default:
            ch = toupper( ch );
            if( ispathname( ch ) )
            {
                if( ins_mode && xpos != buflen )
                {
                    inschinstr( buf, ch, xpos );
                    cputs( buf+xpos );
                    ++buflen;
                }
                else
                {
                    buf[xpos] = ch;
                    if( xpos == buflen )
                        buf[xpos+1] = 0;
                    putch( ch );
                    ++buflen;
                }
                ++xpos;
                gotoxy( x+xpos, y );
            }
        }
    }

    // clear it off
    textattr( 0 );
    printf( "\n" );

    return rvl;
}
