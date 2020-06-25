#ifndef _SHADOW_H
#define _SHADOW_H

enum
{
    srOutdent   = 0x00,     // defaults
    srThin      = 0x00,     // defaults

    srIndent    = 0x01,
    srThick     = 0x02,

    srLineDent  = 0x04,
    srBoxDent   = 0x08,

    srDarkOnly  = 0x10,
    srLightOnly = 0x20,

    srLighter   = 0x40,
    srDarker    = 0x80
};

void shadowrect( int left, int top, int right, int bottom,
                        unsigned uFlags, int clrInterior = -2 );

inline void ShadowRect( Rect& rcRect, unsigned uFlags, int clrInterior = -2 )
{
    shadowrect( rcRect.left, rcRect.top, rcRect.right, rcRect.bottom,
                uFlags, clrInterior );
}

#endif
