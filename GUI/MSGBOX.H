#ifndef _MSGBOX_H
#define _MSGBOX_H

#define MB_YES          0x01
#define MB_NO           0x02
#define MB_OK           0x04
#define MB_CANCEL       0x08
#define MB_ALWAYS       0x10
#define MB_OKCANCEL     (MB_OK | MB_CANCEL)
#define MB_YESNO        (MB_YES | MB_NO)
#define MB_YESNOALWAYS  (MB_YESNO | MB_ALWAYS)
#define MB_YESNOCANCEL  (MB_YESNO | MB_CANCEL)
#define MB_ALL          (MB_YES | MB_NO | MB_OK | MB_CANCEL | MB_ALWAYS)

int message_box( int flags, char *title, char *fmt, ... );

#endif // _MSGBOX_H
