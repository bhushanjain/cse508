/*  Copyright (C) 2002 - 2006 Zhenkai Liang and R. Sekar
    
    This file is part of etrace.

    Etrace is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Etrace is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with etrace; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __ARGUMENT_REP_H
#define __ARGUMENT_REP_H
#include <sys/types.h>

struct SysCallArgRep {
  /*
     The following assumptions are made about system call arguments:
       -- they can be stored in a long; data that wont fit within a long
            is stored in memory, and a pointer to this buffer is the actual
	    system call argument.
       -- they are contained in a register or memory
  */
    pid_t pid_ ; 
    bool ISreg_; /* whether this argument is a register */
    long addr_ ; /* if ISreg_, it is reg number, or it's the addr of data */
    bool inited_; /* whether data_ is assigned to the value in addr_ */
    long data_; /* local cache to store syscall argument that is in
		   monitored process registers or memory space. */
    char *buf_; /* if buf_ is NULL, it's uninitialized. */
    size_t size_ ; /* if buf_ is initialized, it is the size of the buffer */
};


#endif
