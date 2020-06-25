; *****************************************************************************
;
; Replacements for Borland's icky getch and kbhit.
; These're better because if ctrl-brk is pressed, they ignore it.
;
; Just recompile it with the right memory model and tlib it into your libraries.
; (c) Copyright 1993 by Ben Morris
;
; *****************************************************************************

.model large,c
.data

ext_scan        dw      00
kret            dw      00
ug_flag         db      00
ug_char         dw      00

.code

public  kbhit, getch, ungetch

; *****************************************************************************

ungetch proc    __ch: word

        cmp     [ug_flag], 0
        jne     ret_eof

        mov     ax, __ch
        mov     ug_char, ax
        mov     ug_flag, 1
        ret

ret_eof:
        mov     ax, 0ffffh
        ret

ungetch endp

; *****************************************************************************

kbhit   proc

        cmp     ug_flag, 0
        jne     key_true

        cmp     ext_scan, 0
        jne     key_true

        mov     ah, 1
        int     16h

        jz      key_false

key_true:
        mov     ax, 1
        ret

key_false:
        mov     ax, 0
        ret

kbhit   endp

; *****************************************************************************

getch   proc

        cmp     ug_flag, 0
        je      loc1

        mov     ug_flag, 0
        mov     ax, ug_char
        ret

loc1:
        cmp     ext_scan, 0
        je      wait_key
        mov     ax, ext_scan
        mov     ext_scan, 0
        ret

wait_key:
        mov     ah, 0
        int     16h

        cmp     ax, 0000
        je      wait_key                        ; If the whole thing is zero, it's ctrl-brk.  Repeat.

        mov     kret, 0
        mov     byte ptr [kret], al
        cmp     al, 0
        jne     ret_key
        mov     byte ptr [ext_scan], ah

ret_key:
        mov     ax, kret
        ret

getch   endp

end

