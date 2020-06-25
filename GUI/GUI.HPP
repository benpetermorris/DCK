/*-------------------------------------------------------------------------*
 * functions for initializing the gui.
 *-------------------------------------------------------------------------*/

#ifndef _GUI_HPP
#define _GUI_HPP

#include "strlist.hpp"
#include "coords.hpp"
#include "inpdev.h"
#include "clr.h"
#include "pophelp.hpp"

void InitGUI( void );
void LoadPalette( const char* fnPalette );

extern int iScreenWidth, iScreenHeight;
extern PopHelp _PopHelp;

#endif // _GUI_HPP
