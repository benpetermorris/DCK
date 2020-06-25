//  32 bit mouse handler example
//
//  This example shows how to create a user-defined mouse
//  handler, using the mouse interrupt ( short 0x33, function 0x0b )

//
//  NOTE: This example requires Turbo Assembler
#include <stdio.h>
#include <conio.h>
#include <dos.h>
//  we need to define a global variable that we store our programs
//  data segment in, so that it can be loaded inside our mouse
//  handler
short datasegment;
// These global variables are modified by the mouse handler
short event,buttonState,x,y,vticks,hticks;
// the handler itself is defined in assembler, in the file mhandler.asm

extern void mouseHandler( void )
{
// The compiler will generate the following entry code to this
// function:
//
//    push  ebp
//    mov   ebp,esp
//    push  ebx
//    push  esi
//    push  edi
  asm {

    push ds
    mov  ds,cs:[datasegment]
    mov  event,ax
    mov  buttonState,bx
    mov  x,cx
    mov  y,dx
    mov  vticks,si
    mov  hticks,di
    pop  ds

// since we need to to a far return, we need to clean up the stack
// ourselves:

    pop  edi
    pop  esi
    pop  ebx
    //pop  ebp
    retf
  }
// compiler generates the following exit code, but we never get
// here because of the retf above:
//

//  pop edi
//  pop esi
//  pop ebx
//  pop ebx
//  ret
}

// calling the mouse interrupt in 32 bit is almost the same
// as calling from 16 bit, just use EDX instead of DX for
// the offset to the mouse handler
void installMouseHandler()
{
  asm {
    push es
    mov  ax,cs
    mov  es,ax
    mov  edx,OFFSET mouseHandler
    mov  ax,0x000c
    mov  cx,0x007f
    int  0x33
    pop  es
  }
}
// remove the mouse handler

void removeMouseHandler() {
  asm {
    mov ax,0x21
    int 0x33
  }
}

short main(void) {
// initialize the global variable to our DS
  datasegment = _DS;
  asm {
    mov ax,0x0
    int 0x33
  }
  installMouseHandler();  /* install our handler */
// print out the global variables until a key is pressed
  while (!kbhit()) {
    printf("%04X %04X (%6d,%6d)  (%6d,%6d)\n",event,buttonState,x,y,vticks,hticks);
  }
  removeMouseHandler();  /* remove mouse handler */
  return 0;

}

