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

#ifndef __DIR_TOOLS_H__
#define __DIR_TOOLS_H__
#include "ProcData.h"
#include <errno.h>

/* 
   redefine _syscall3 to remove the following problem on Red Hat 7.3
   Can't find a register in class `BREG' while reloading `asm'.
*/

/*#undef _syscall3*/

#define syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
long __res; \
__asm__ volatile ("movl %%ecx, %%ebx" : : "c"((long)(arg1))); \
__asm__ volatile ("int $0x80" \
        : "=a" (__res) \
        : "0" (__NR_##name),"c" ((long)(arg2)), \
                  "d" ((long)(arg3))); \
do { \
if ((unsigned long)(__res) >= (unsigned long)(-(128 + 1))) {\
errno = -(__res); \
__res = -1; \
} \
return (type) (__res); \
} while (0); \
}


void NewPath(char *path, const char *change) ;
int getdents(uint fd, struct dirent * dirp, uint count);
int getdents64(uint fd, struct dirent * dirp, uint count);
int readdir(uint fd, struct dirent * dirp, uint count);

#endif
