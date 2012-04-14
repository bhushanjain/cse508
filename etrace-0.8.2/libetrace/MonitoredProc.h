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

#ifndef __MONITOREDPROC_H
#define __MONITOREDPROC_H
#include <string>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <iostream>

#include "Extension.h" 
#include "SystemCall.h"
class ArchDep ;
class ManagerClass ;
using namespace std ;

class MonitoredProc
{
    protected:
    pid_t pid_ ;
    ManagerClass *managerObj_;
    char program[PATH_MAX] ;
    char programtemp[PATH_MAX] ;
    char mapfile[PATH_MAX] ;

    string lib_, monClass_ ;
    bool switchExt_ ;

    Extension headExtension ;
    Extension *currentExtension ;

    SystemCall *curCall_ ;
    bool exited_;
    public:
    MonitoredProc(ManagerClass *mgr) : headExtension(){ 
	managerObj_ = mgr;
	switchExt_ = false;
	exited_ = false;
    }
	
    ~MonitoredProc();

    void pid( pid_t p ) { pid_ = p ; } 
    pid_t pid() const { return pid_; } 
    pid_t ppid() ;

    void setExitFlag() { exited_ = true; }
    ArchDep *getArch() const;

    void programTemp(const char *prog) {
	strncpy(programtemp, prog, PATH_MAX) ;
    }
    void changeProg() {
	strncpy(program, programtemp, PATH_MAX) ;
    }
    void programName(const char *prog) {
	strncpy(program, prog, PATH_MAX) ;
    }
    char *programName() { return program ; }

    void mapFileName(const char *map) {
	strncpy(mapfile, map, PATH_MAX) ;
    }
    char *mapFileName() { return mapfile ; }

    void switchExt(bool c) { switchExt_ = c ; }
    bool switchExt() { return switchExt_ ; } 

    const char *lib() const { return lib_.c_str() ; } 
    void lib(string p) { lib_ = p ; } 

    const char *monClass() const { return monClass_.c_str() ;}
    void monClass(string p) { monClass_ = p ; } 

    void setValues(MonitoredProc* s) ;

    bool noExtension() { return headExtension.NextNodeRight() == &headExtension; }
    void insertExtension(Extension *pExtension) ;
    void cleanUpExtensions() ;
    void replaceExtension(Extension *oldExt, Extension *newExt) ;
    void resetExtensionList() ; 
    Extension *getNextExtension() ; 
    Extension *getPrevExtension() ;

    void setCurCall(SystemCall *call) { curCall_ = call ; }
    SystemCall * getCurCall() { return curCall_ ; }

    int abort(long rc) ;
    long currentRC() ;

    bool processInSession(pid_t pid); 
} ;

#endif
