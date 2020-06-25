#include <std.h>

ushort dataseg;
ulong _OLDint1c;

void setvect( uchar vector, ulong ofs ) {
  asm {
    push ds
    push edx
    push ax
    mov ax, 0205h
    mov bl, vector
    mov edx, ofs
    int 0x31
    pop  ax
    pop  edx
    pop  ds
  }
}

void getvect( uchar vector, ulong *ofs ) {
  ulong  tempofs;
  asm {
    mov ax, 0204h
    mov bl,vector
    int 0x31
    mov tempofs,edx
  }
  *ofs = tempofs;
}

ulong _tics;

void restoreint1c( void )
{
    setvect( 0x1c, _OLDint1c );
}

extern "C" void _DCKint1c( void )
/*-------------------------------------------------------------------------*
 * Dispatches mouse events if a demo is playing and the next mouse event's
 * time is up.
 *-------------------------------------------------------------------------*/
{
    /* save registers and load ds with dck's data segment */
    asm push ds eax
    asm mov ds, cs:[dataseg]

    // _mcheckevent();

    /* increment timer */
    ++_tics;

    /* restore ds and eax */
    asm pop eax ds

    /* do the old handler */
    asm jmp  cs: dword ptr [_OLDint1c]
}

void main( void )
{
    dataseg = _DS;
    getvect( 0x1c, &_OLDint1c );
    setvect( 0x1c, (ulong) _DCKint1c );
    atexit( restoreint1c );

    while( !kbhit() )
    {
        printf( "%d\n", _tics );
    }
}
