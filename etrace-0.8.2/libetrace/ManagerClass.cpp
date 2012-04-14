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

#include "ManagerClass.h"
#include "ArgumentType.h"

#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/times.h>
#include <sys/wait.h>

using namespace std ;
/*
 * Entry function of this class, it will start the monitor process and
 * the first monitored process. Then enter the main loop.
 */
bool ManagerClass::exist = false ;

ManagerClass::ManagerClass(ArchDep *pAD, const char *mapfile) 
{
    /* only allow one instance of manager class */
    if (exist) {
	disable = true ;
	cerr << "More than one instance of ManagerClass is created!" << endl ;
    } else {
	arch = pAD ;
	if (mapfile != 0)
	    strncpy(mapFileName, mapfile, PATH_MAX) ;
	else 
	    mapFileName[0] = 0 ; 
	exist = true ;
	disable = false ;
	switch_on_exec = false ;
	process_count = 0 ;
    }
#ifdef MEASURE_TIME
    for (int i = 0 ; i < 256 ; i++) {
	user_time[i] = 0 ;
	sys_time[i] = 0 ;
	call_count[i] = 0 ;
    }
#endif
}

ManagerClass::~ManagerClass() 
{
    monitoringTable.Cleanup() ;
#ifdef MEASURE_TIME
    int count = 0 ;
    for (int i = 0 ; i < 256 ; i++) {
	count += call_count[i] ;
    }
    if (total_time !=0) {
	cerr << "Calls made: " << count << endl ;
	cerr << "Calls per second: " << sysconf(_SC_CLK_TCK) * count / total_time << endl ;
    }
#endif
}

Extension* ManagerClass::createExtension(const char* lib, const char* supClass)
{
    char fnname[512];
    void *handle;
    Extension *obj1=0 ;

    if (!strncmp(lib, "PREDEFINED", 11)) {
	if (!strncmp(supClass, "KILL", 5)) 
	    obj1 = new KILL();
	else if (!strncmp(supClass, "NONE", 5)) 
	    obj1 = new NONE();
	else if (!strncmp(supClass, "ACCEPT", 5)) 
	    obj1 = new ACCEPT();
    } else {
	Extension* (*initptr)();
	/* Open the library */
	handle = arch->openDynamicLibrary(lib);
	if (!handle) {
	    cerr << "ManagerClass: Open library error : " << arch->getDLError() << endl;
	    return 0 ;
	}
    
	/*
	 * find the target class creating fuction by its name
	 */
	snprintf(fnname, 512, "init%s", supClass);

	initptr =(Extension* (*)()) arch->getDLFunction(handle, fnname);
	if (initptr == 0) {
	    cerr << "ManagerClass: No init function: " << arch->getDLError() << endl ;
	    return 0 ;
	}
	
	/*
	 * Create the class
	 */
	obj1 = (*initptr)();
	if (obj1 == 0) {
	    cerr << "ManagerClass: Error in allocating new extension!" << endl;
	    return 0 ;
	}

    }
    return obj1;
}

int ManagerClass::setupExtensions(const char *mapfile, const char *path, 
				  MonitoredProc *supObj)
{
    bool found = false ;
    FILE *fp = fopen(mapfile, "r") ;
    if (0 == fp) {
	Extension *pExtension = new KILL() ;
	if (pExtension != 0) 
	    supObj->insertExtension(pExtension) ;
	cerr << "Cannot open mapping file, default to kill process!" << endl ;
	return 0 ;
    }
   
    /* remove all the existing extensions */
    supObj->cleanUpExtensions() ;

    while (!feof(fp)) {
	char buf[1024], cmd[128], lib[128], cname[128] ;
	if (0 == fgets(buf, 1024, fp))
	    break ;
	if (buf[0] == '#')  // comments begin with #
	    continue ;
	if (sscanf(buf, "%128s %128s %128s", cmd, lib, cname) != 3)
	    continue ;
	if (!strncmp(cmd, path, PATH_MAX)){
	    if (!strncmp(lib, "MAPPINGFILE", sizeof("MAPPINGFILE"))) {
		supObj->mapFileName(cname) ;
	    } else {
		found = true ;
		Extension *pExtension = createExtension(lib, cname) ;
		if (pExtension != 0) 
		    supObj->insertExtension(pExtension) ;
		else {
		    cerr << "ManagerClass: Cannot create extension: " ;
		    cerr << lib << "->" << cname << endl ;
		}
	    }
	}
    }
    
    if (!found) {
	fseek(fp, 0, SEEK_SET) ;
	while (!feof(fp)) {
	    char buf[1024], cmd[128], lib[128], cname[128] ;
	    if (0 == fgets(buf, 1024, fp))
		break ;
	    if (buf[0] == '#')  // comments
		continue ;
	    if (sscanf(buf, "%128s %128s %128s", cmd, lib, cname) != 3)
		continue ;
	    if (!strncmp(cmd, "default", PATH_MAX)){
		found = true ;
		Extension *pExtension = createExtension(lib, cname) ;
		if (pExtension != 0) 
		    supObj->insertExtension(pExtension) ;
		else {
		    cerr << "ManagerClass: Cannot create extension: " ;
		    cerr << lib << "->" << cname << endl ;
		}
	    }
	}
    }

    if (!found || supObj->noExtension()) {
	Extension *pExtension = new KILL() ;
	if (pExtension != 0) 
	    supObj->insertExtension(pExtension) ;
	cerr << "No extensions found, default to kill process!" << endl ;
    }
	
    fclose(fp) ;

    return 0 ;
}

int ManagerClass::startTracing(char* argv[])
{
    if (disable) 
	return -1 ;
    pid_t pid ;
    char *filename ;
    char pathname[PATH_MAX], tmppath[PATH_MAX];

    /* 
     * The next few steps are to deal with the path names.
     * They change a given pathname to its realpath, which is
     * the absolute path. If the target is a symbolic link, the result
     * will be the real file, not the link itself.
     */ 

    filename = argv[0];
    if (strchr(filename, '/')) 
	strncpy(tmppath, filename, PATH_MAX);
    else{
	char *path;
	int m, n, len;
	
	for (path = getenv("PATH"); path && *path; path += m) {
	    if (strchr(path, ':')) {
		n = strchr(path, ':') - path;
		m = n + 1;
	    }
	    else
		m = n = strlen(path);
	    if (n == 0) {
		getcwd(tmppath, PATH_MAX);
		len = strlen(tmppath);
	    }
	    else {
		strncpy(tmppath, path, n);
		len = n;
	    }
	    if (len && tmppath[len - 1] != '/')
		tmppath[len++] = '/';
	    strncpy(tmppath + len, filename, PATH_MAX - len);
	    if (access(tmppath, F_OK) == 0)
		break;
	}
    }
    if (access(tmppath, F_OK) < 0) {
	cerr << "ManagerClass: " << "Command not found!" << filename << endl ;
	return -1 ;
    }

    if (realpath(tmppath, pathname) == NULL) {
	cerr << "ManagerClass: " << strerror(errno) << endl ;
	return -1 ;
    }

    pid = arch->startTracing(pathname, argv) ;
    if (pid < 0) {
	cerr << "ManagerClass: Tracing setup error!" << endl ;
	return -1 ;
    }

    /* create the first MonitoredProc object */
    process_count = 1 ;
    
    MonitoredProc *supObj = new MonitoredProc(this) ;
    if (0 == supObj) {
	cerr << "ManagerClass: Cannot alloc MonitoredProc!" << endl ;
	arch->terminateProc(pid) ;
	return -1 ;
    }

    supObj->pid(pid) ;
    supObj->programName(pathname) ;
    supObj->mapFileName(mapFileName) ;
    /* setup the detection engine */
    if (setupExtensions(mapFileName, pathname, supObj) < 0) {
	cerr << "ManagerClass: Extension setup error!" << endl ;
	return -1 ;
    }

    monitoringTable.insert(pid, supObj) ;
	    
#ifdef MEASURE_TIME
    struct tms start_tms ; 
    clock_t start = times(&start_tms) ;
#endif

    if (mainLoop() < 0)
	return -1 ;

    waitpid(pid, 0, 0) ;

#ifdef MEASURE_TIME
    struct tms end_tms ;
    clock_t end = times(&end_tms) ;
    total_time = end - start ;

    //discard the wall time, use u and s time
    total_time = (end_tms.tms_utime+end_tms.tms_stime+end_tms.tms_cutime+
		  end_tms.tms_cstime) - 
	(start_tms.tms_utime+start_tms.tms_stime+start_tms.tms_cutime+
	 start_tms.tms_cstime) ;
	 
#endif

    return 0 ;
}

int ManagerClass::startTracing(pid_t pid)
{
    if (disable) 
	return -1 ;
    
    int retval = arch->attachProc(pid) ;
    if (retval < 0) {
	cerr << "ManagerClass: Cannot attach to process " << pid << strerror(errno)<< endl ;
	return -1 ;
    }

    /* create the first MonitoredProc object */
    process_count = 1 ;
    
    MonitoredProc *supObj = new MonitoredProc(this) ;
    if (0 == supObj) {
	cerr << "ManagerClass: Cannot alloc MonitoredProc!" << endl ;
	arch->terminateProc(pid) ;
	return -1 ;
    }

    supObj->pid(pid) ;

    /* setup the detection engine */

    char pathname[PATH_MAX] ;
    arch->getExecPath(pid, pathname, PATH_MAX) ;
    supObj->programName(pathname) ; 
    supObj->mapFileName(mapFileName) ;
    if (setupExtensions(mapFileName, pathname, supObj) < 0) {
	cerr << "ManagerClass: Extension setup error!" << endl ;
	return -1 ;
    }

    monitoringTable.insert(pid, supObj) ;
	    
    if (mainLoop() < 0)
	return -1 ;

    return 0 ;
}

int ManagerClass::mainLoop()
{
    int retval ;
    pid_t pid = 0 ;
    bool execed = false;
    /*
     * Loop until there's no monitored process left. 
     */
    while_begin:
    while (process_count > 0)	{
	/* wait for next call */
	SystemCall theCall ;
#ifdef MEASURE_TIME
	struct tms start_tms ;
	times(&start_tms) ;
#endif
	retval = arch->waitForCall(&pid, &theCall);
	
#ifdef MEASURE_TIME
	struct tms end_tms ;
	times(&end_tms) ;
#endif

	if (NEXTCALL_ERROR == retval) 
	    continue ;

	MonitoredProc *supObj = monitoringTable.lookUp(pid) ;
	if (0 == supObj) {
	    cerr << "ManagerClass: No record of process!" << endl ;
	    continue ;
	}
	
	execed = false;
	switch (retval) {
	case NEXTCALL_EXITED:
	    process_count -- ;
	    // set the exit flag, so that destructor of MP won't kill it again
	    supObj->setExitFlag();
	    monitoringTable.remove(pid) ;
	    goto while_begin ;
	    break ;
	case NEXTCALL_CREATED:
	    /* alloc a new monitoring object for new process */
	    /* ASSUMPTION: new pid is returned by return value */
	    if (!theCall.isEntry()) {
		int errnum ;
		long childpid = arch->getReturnVal(pid, &errnum) ;
		MonitoredProc *childObj = new MonitoredProc(this) ;
		if (0 == childObj) {
		    cerr <<"ManagerClass: Cannot alloc MonitoredProc!" << endl;
		    arch->terminateProc(childpid) ;
		    return -1 ;
		}
		childObj->pid(childpid) ;
		childObj->programName(supObj->programName()) ;
		childObj->mapFileName(supObj->mapFileName()) ;
		supObj->resetExtensionList() ;
		Extension *pExtension ;
		while ((pExtension=supObj->getNextExtension()) != 0) {
		    Extension *ext = pExtension->clone() ;
		    childObj->insertExtension(ext) ;
		}
	
		monitoringTable.insert(childpid, childObj) ;
		process_count ++ ;
	    }
	    break ;
	case NEXTCALL_EXECED:
	    execed = true;
	    break ;
	default: 
	    break ;
	}

#ifdef MEASURE_TIME
	long scno = theCall.scno() ;
	user_time[scno] += end_tms.tms_utime - start_tms.tms_utime ;
	sys_time[scno] += end_tms.tms_stime - start_tms.tms_stime ;
	call_count[scno] ++ ;
#endif
	/* deliver this event to the Extensions */
	/* The Extensions are arranged as layers, i.e., the first Extension at the 
	   entry point is the last Extension at the exit point */
	

	if (switch_on_exec) {
	    if (execed)
		if (theCall.isEntry()) {
		    CString exec(theCall.getArgRep(0)) ;
		    supObj->programTemp(exec.get().c_str()) ;
		} 
	}
	    
	supObj->setCurCall(&theCall) ;
	Extension *pExtension ;
	supObj->resetExtensionList() ;
	while (true) {
	    if (theCall.isEntry())
		pExtension = supObj->getNextExtension() ;
	    else 
		pExtension = supObj->getPrevExtension() ;
	    if (0 == pExtension) 
		break ;
	    
	    bool nextWait = false ;
	    int retval = pExtension->deliverEvent() ;
	    switch (retval) {
	    case EXTENSION_ABORT:
		/* kill this process and wait for the next event */
		nextWait = true ;
		arch->terminateProc(pid) ;
		process_count -- ;
		break ;
	    case EXTENSION_ACCEPT:
		/* detach this process and wait for the next event */
		nextWait = true ;
		arch->detachProc(pid) ;
		process_count -- ;
		break ;
	    default:
		break ;
	    }
	    if (nextWait)
		break ;

	    if (supObj->switchExt()) {
		Extension *newExt=createExtension(supObj->lib(), 
						  supObj->monClass());
		if (0 != newExt)
		    supObj->replaceExtension(pExtension, newExt) ; 
		supObj->switchExt(false) ;
	    }
	}

	if (switch_on_exec) {
	    if (execed)
		if (!theCall.isEntry()) {
		    int err ; 
		    if (arch->getReturnVal(pid, &err)==0) {
			supObj->changeProg() ;
			if (setupExtensions(supObj->mapFileName(), 
					    supObj->programName(), supObj)< 0){
			    cerr << "ManagerClass: Extension setup error!" << endl ;
			    return -1 ;
			}
		    }
		
		}
	}

    }

    return 0 ;
}
