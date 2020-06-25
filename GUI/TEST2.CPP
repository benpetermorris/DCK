#include <iostream.h>
#include <fstream.h>

void main( void )
{
    char str[128];

    ifstream inf( "TEST.DOC" );

    while( !inf.eof() )
    {
        inf >> str;
        cout << str << endl;
    }
}
