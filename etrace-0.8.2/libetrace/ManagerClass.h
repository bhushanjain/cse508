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

#ifndef __MANAGERCLASS_H
#define __MANAGERCLASS_H

#include "ArchDependent.h"
#include "ProcHashTable.h"
#include "MonitoredProc.h"

class ManagerClass
{
 private:
    static bool exist ;
    bool disable ;
    bool switch_on_exec ;
    ArchDep *arch ;
    char mapFileName[PATH_MAX] ;
    ProcHashTable<MonitoredProc> monitoringTable ;

#ifdef MEASURE_TIME
    int user_time[256] ;
    int sys_time[256] ;
    int call_count[256] ;
    int total_time ;
#endif
    int process_count ;  /* number of processes under monitor */

    int mainLoop();
    
 public:
    ManagerClass(ArchDep *pAD, const char *mapfile) ;
    ~ManagerClass() ;
    
    Extension* createExtension(const char* lib, const char* supClass);
    int setupExtensions(const char *mapfile, const char *path, MonitoredProc*);
    int startTracing(char* argv[]);
    int startTracing(pid_t pid) ;
    inline void switchOnExec(bool val) { switch_on_exec = val ; }
    
    inline ArchDep *getArch() { return arch ; }
    inline bool processInSession(pid_t pid) {
	if (0 == monitoringTable.lookUp(pid))
	    return false;
	return true; 
    }
    
} ;

#endif
