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

#ifndef __SYSTEM_CALL_H
#define __SYSTEM_CALL_H
#include "ArgumentRep.h"

#include <sys/types.h>

class SystemCall {
    private:

    long scno_ ;
    long uscno_ ;
    bool isEntry_ ;
    SysCallArgRep *arg ;

    public:
    SystemCall() {
    }
    ~SystemCall() {
    }
    
    long scno() { return scno_ ; }
    void scno(long sc) { scno_ = sc ; }

    long uscno() { return uscno_ ; }
    void uscno(long sc) { uscno_ = sc ; }

    bool isEntry() { return isEntry_ ; }
    void isEntry(bool v) { isEntry_ = v ; }

    void setArgRep(SysCallArgRep *val) {
	arg = val ;
    }

    SysCallArgRep *getArgRep(int number){ 
	if ((number >= 0) && (number < 6)) return &arg[number] ;
	return 0 ;
    }

   
} ;

const int SYSCALL_FORK = 1 ;
const int SYSCALL_VFORK = 2 ;
const int SYSCALL_CLONE = 3 ;
const int SYSCALL_KILL = 4 ;

#endif
