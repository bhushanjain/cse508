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

#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__

const int PATH_NOTALLOWED = -1;
const int PATH_NEW = 0 ;
const int PATH_CREATED = 1 ;
const int PATH_MODIFIED = 2 ;
const int PATH_DELETED = 3 ;
const int PATH_CHANGED = 4 ;

const int TYPE_FILE = 1 ;
const int TYPE_DIRECTORY = 2 ;

#ifdef INSTALL_SHIELD
void mod_log(char *filetype, char *filepath, char *newpath, char *status, pid_t pid, pid_t ppid) ;
#endif 

#endif
