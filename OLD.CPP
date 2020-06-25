#if 0
#define NOT 0x8000
#define OP_ANYTHING 0xFFFF

typedef void (*disFP)(unsigned);

struct
{
    unsigned    uKey;           // key
    ushort  sModes;             // do this under what edit modes?
    ushort  sOp;                // do this under what current operation?
    disFP   fp;                 // function to call

} DispatchTable[] =
{

// ANY EDIT MODE:

    // misc commands
    { KEY_ALTBACK, MODE_ALL, OP_NOTHING, (disFP)undo_undo },

    // clipboard
    { KEY_CTRLINS, MODE_ALL, OP_NOTHING, (disFP)dCopy },
    { KEY_CTRLDEL, MODE_ALL, OP_NOTHING, (disFP)dCut },
    { KEY_INS, MODE_ALL, OP_NOTHING, (disFP)dPaste },

    // home/end: doing nothing- change edit mode
    { KEY_HOME, MODE_ALL, OP_NOTHING, (disFP)dTogEditMode },
    { KEY_END, MODE_ALL, OP_NOTHING, (disFP)dTogEditMode },

    // home/end: dragging, thingmode- switch drag mode
    { KEY_HOME, MODE_THING, OP_DRAG, (disFP)dTogDragMode },
    { KEY_END, MODE_THING, OP_DRAG, (disFP)dTogDragMode },

    // 'O': place thing origin
    { 'O', MODE_THING, OP_ANYTHING, (disFP)dPlaceOrigin },
    // 'D': Thing display flags
    { 'D', MODE_THING, OP_ANYTHING, (disFP)dThingFlags },

    // alt+c: anything, any mode- consistency checker
    { KEY_ALTC, MODE_ANY, OP_ANYTHING, (disFP)dConsist },

    // alt+w: anything, any mode- goto object
    { KEY_ALTW, MODE_ANY, OP_ANYTHING, (disFP)dGotoObject },


    // ctrl+g: anything, any mode- center map },
    { KEY_CTRLG, MODE_ANY, OP_ANYTHING, (disFP)CenterMap },

    // ctrl+i:

    // ctrl+r: nothing, line mode- flip sidedefs },
    { KEY_CTRLG, MODE_ANY, OP_ANYTHING, (disFP)dFlipSides },

    // ctrl+v: nothing, any mode- snap marked objects
    { KEY_CTRLV, MODE_ANY, OP_NOTHING, (disFP)dSnapObjects },

    // ctrl+w: nothing, sector/linemode- align textures
    { KEY_CTRLW, MODE_SECTOR | MODE_LINE, OP_NOTHING, (disFP)dAlignTextures },

    // ctrl+z: nothing, linemode- create stairs
    { KEY_CTRLZ, MODE_LINE, OP_NOTHING, (disFP)stair_do },


    // end marker
    { -1, NULL }
};
#endif

void bintostr( ulong value, char *pszDest )
{
    char *pDest = pszDest;
    ulong i = 31;


    do {
        if( value & (1 << i) )
            *pDest++ = '1';
        else
            *pDest++ = '0';
    } while( i-- );

    *pDest = 0;
}

