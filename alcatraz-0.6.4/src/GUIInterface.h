/*  Copyright (C) 2003 - 2006 Zhenkai Liang
    
    This file is part of alcatraz.

    Alcatraz is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Alcatraz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with alcatraz; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __GUI_INTERFACE_H__
#define __GUI_INTERFACE_H__
#include <sys/types.h>

// Return Code: negative for failure and 0 for success. 
int connectgui(int) ;

// Return Code: negative for failure and 0 for success.
#ifdef INSTALL_SHIELD
int sendfile(const char *message, size_t size);
#else
int sendfile(const char *filepath, const char *status, const char *privatedat, const char *timestamp, const char *filetype);
#endif

// Return Code: negative for failure and 0 for success.
int finish_sending() ;

// Return code: -1 for error, 0 for cancel and 1 for commit
int getResponse() ;

int sendOK() ;
int sendNOK() ;

#endif

