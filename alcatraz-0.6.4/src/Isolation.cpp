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

#include "Alcatraz.h"
#include "ProcData.h"
#include "CommonDefs.h"
#include "MappingTable.h"
#include <LinuxTypes.h>
#include <MonitoredProc.h>
#include <ArchDependent.h>

#include <limits.h>
#include <libgen.h>
#include <unistd.h>

int readdir(uint fd, struct dirent * dirp, uint count);

IDInfo<ProcData> processData ;
extern MappingTable mt ;

bool Alcatraz::parent_writable(const char *file)
{
    char dir[PATH_MAX], temp[PATH_MAX] ;
    strncpy(temp, file, PATH_MAX) ;
    strncpy(dir, dirname(temp), PATH_MAX) ;
    if (access(dir, W_OK)==0) 
	return true ;

    return false ;
}



void Alcatraz::open_entry()
{
    CString arg0(getArgRep(0)) ;
    Integer arg1(getArgRep(1)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    char normpath[PATH_MAX], buf[PATH_MAX] ;
    int maptype ;

    if (strncmp("/.a_to_GUI", arg0.get().c_str(), 10) == 0) {
	mt.mapping.sendtoGUI(false) ;
	arch->abortCall(pid, -1, EPERM) ;
	return ;
    }

    if (strncmp("/.alcatraz", arg0.get().c_str(), 10) == 0) {
	arg0.set(arg0.get().c_str()+10) ;
	arg1.set(0) ;
	return ;
    }

    // check open modes
    int flag = arg1.get() ;
    bool readonly = true ;
    if (flag&O_WRONLY || flag&O_RDWR || flag&O_CREAT)
	readonly = false ;
    bool trunc = false ;
    if (!readonly && flag&O_TRUNC)
	trunc = true ;

    normalizePath(arg0.get().c_str(), normpath) ;
    maptype = translatePath(normpath, buf, true, tempname);
    // Remember the association between file descriptors and path names
    // It remembers the canonized name
    char *tmp ;
    if (PATH_NEW == maptype)
	tmp = buf ;
    else tmp = normpath ;
    size_t strsize = strlen(tmp) + 1 ;
    char *duppath = new char[strsize] ;
    if (0 != duppath) {
	memcpy(duppath, tmp, strsize) ;
    }
    ProcData *pData = processData.lookUp(pid) ;
    if (0 == pData) {
	pData = new ProcData() ;
	processData.insert(pid, pData) ;
    }
    pData->storeOpenName(duppath) ;

    switch(maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break; 
    case PATH_CREATED:
    case PATH_MODIFIED:
#ifdef INSTALL_SHIELD
	if (!readonly) {
	    pid_t ppid = mp->ppid() ;
	    mt.mapping.appendTime(normpath, pid, ppid) ;
	    if (access(buf, F_OK) == 0)
		mod_log("F", normpath, buf, "MD", pid, ppid) ;
	    else 
		mod_log("F", normpath, buf, "CR", pid, ppid) ;
	}
#endif
	arg0.set(buf) ;
	break ;
    case PATH_DELETED: {
	if (readonly)
	    arch->abortCall(pid, -1, ENOENT) ;
	else {
	    /* create a new file, delete previous entry and
	       generate a new temp file */
	    char tempfile[PATH_MAX] ;
	    mt.delMapping(tempname) ;
	    mt.newEntry(TYPE_FILE, PATH_MODIFIED, tempname, tempfile) ;
	    arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("F", normpath, tempfile, "CR", pid, ppid) ;
#endif
	}
	break ;
    }
    case PATH_NEW: {
	if (!readonly){
	    if (access(buf, F_OK)==0) { /* File exists */
		struct stat statbuf ;
		if (stat(buf, &statbuf) == 0){/*success*/
		    if (!S_ISREG(statbuf.st_mode)) break ;
		}

		if (access(buf, W_OK) == 0) {
		    /* can write, isolate file */
		    char tempfile[PATH_MAX] ;
		    mt.isolate(buf, tempfile, trunc) ;
		    arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	            pid_t ppid = mp->ppid() ;
		    mod_log("F", normpath, tempfile, "MD", pid, ppid) ;
#endif

		}
	    }
	    else { /* File Not Exists */
		if (parent_writable(buf)) {
		    char tempfile[PATH_MAX] ;
		    mt.newEntry(TYPE_FILE, PATH_CREATED, buf, tempfile) ;
		    //		    mt.getStatus(normpath, buf) ;
		    arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	            pid_t ppid = mp->ppid() ;
		    mod_log("F", normpath, tempfile, "CR", pid, ppid) ;
#endif
		}
	    }
	}
	else {
	    arg0.set(buf) ;
	}
	break ;
    }
    default:
	break ;
    }
    
}

void Alcatraz::open_exit()
{
    pid_t pid = mp->pid() ;
    int fd = mp->currentRC() ;
    if (fd < 0)
	return ;

    ProcData *pData = processData.lookUp(pid) ;
    if (0 != pData) 
	pData->insertFDName(fd) ;
}

void Alcatraz::chmod_entry() 
{
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;
    int maptype ;
    maptype = simple_mapping(0) ;
    switch (maptype) {
    case PATH_NEW:
	arch->abortCall(pid, 0, 0);
	break ;
    case -LOC_INTEMP:
	break ;
    case -LOC_INCACHE:
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    default:
	break ;
    }
}

void Alcatraz::lchown_entry()
{
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;
    int maptype ;
    maptype = simple_mapping(0) ;
    switch (maptype) {
    case PATH_NEW:
	arch->abortCall(pid, -1, EPERM) ;
	break ;
    case -LOC_INTEMP:
	break ;
    case -LOC_INCACHE:
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    default:
	break ;
    }
}

void Alcatraz::chown_entry()
{
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;
    int maptype ;
    maptype = simple_mapping(0) ;
    switch (maptype) {
    case PATH_NEW:
	arch->abortCall(pid, -1, EPERM) ;
	break ;
    case -LOC_INTEMP:
	break ;
    case -LOC_INCACHE:
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    default:
	break ;
    }
}

int Alcatraz::truncate_failure(const char *file) 
{
    /* file doesn't exists */
    if (access(file, F_OK) != 0)
	return ENOENT ;

    /* file is not writable by process */
    if (access(file, W_OK) != 0)
	return EPERM ;
    struct stat statbuf ;
    if (stat(file, &statbuf) != 0)
	return EPERM ;
    if (S_ISDIR(statbuf.st_mode))
	return EISDIR ;

    return 0 ;
}

void Alcatraz::truncate_entry()
{
    CString arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    char normpath[PATH_MAX], buf[PATH_MAX] ;
    /* first get the real path of the file */

    normalizePath(arg0.get().c_str(), normpath) ;
    int maptype = translatePath(normpath, buf, true);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    case PATH_DELETED:
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    case PATH_CREATED:
    case PATH_MODIFIED:{
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath, buf, "MD", pid, ppid) ;
#endif
	arg0.set(buf) ;
	break ;
    }
    case PATH_NEW: {
	int retval = truncate_failure(buf) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	}
	else {
	    char tempfile[PATH_MAX] ;
	    mt.isolate(buf, tempfile, false) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("F", normpath, tempfile, "MD", pid, ppid) ;
#endif
	    arg0.set(tempfile) ;
	}
	break ;
    }
    }
}

int Alcatraz::creat_failure(const char *file)
{
    if (!parent_writable(file))
	return EPERM ;
    return 0 ;
}

void Alcatraz::creat_entry()
{
    CString arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    char normpath[PATH_MAX], buf[PATH_MAX] ;
	    
    normalizePath(arg0.get().c_str(), normpath) ;
    int maptype = translatePath(normpath, buf, false, tempname);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break;
    case PATH_DELETED:{
	mt.delMapping(tempname) ;
	char tempfile[PATH_MAX] ;
	mt.newEntry(TYPE_FILE, PATH_MODIFIED, tempname, tempfile) ;
	arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath, tempfile, "CR", pid, ppid) ;
#endif
	break ;
    }		    
    case PATH_CREATED:
    case PATH_MODIFIED:
	arg0.set(buf) ;
	break ;
    case PATH_NEW :{
	int retval = creat_failure(buf) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	} else {
	    char tempfile[PATH_MAX] ;
	    mt.newEntry(TYPE_FILE, PATH_CREATED, buf, tempfile) ;
	    arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("F", normpath, tempfile, "CR", pid, ppid) ;
#endif
	}
    }
    }
}

int Alcatraz::mknod_failure(const char *file)
{
    if (!parent_writable(file))
	return EPERM ;
    return 0 ;
}


void Alcatraz::mknod_entry()
{
    CString arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    char normpath[PATH_MAX], buf[PATH_MAX] ;
	    
    normalizePath(arg0.get().c_str(), normpath) ;
    int maptype = translatePath(normpath, buf, false, tempname);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break; 
    case PATH_DELETED:{
	mt.delMapping(tempname) ;
	char tempfile[PATH_MAX] ;
	mt.newEntry(TYPE_FILE, PATH_MODIFIED, tempname, tempfile) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath, tempfile, "CR", pid, ppid) ;
#endif
	arg0.set(tempfile) ;
	break ;
    }		    
    case PATH_CREATED:
    case PATH_MODIFIED:
	arg0.set(buf) ;
	break ;
    case PATH_NEW :{
	int retval = mknod_failure(buf) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	} else {
	    char tempfile[PATH_MAX] ;
	    mt.newEntry(TYPE_FILE, PATH_CREATED, buf, tempfile) ;
	    arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("F", normpath, tempfile, "CR", pid, ppid) ;
#endif
	}
    }
    }
}

int Alcatraz::mkdir_failure(const char *file)
{
    if (access(file, F_OK) == 0)  /* path exists */
	return EEXIST ;
    if (!parent_writable(file))
	return EPERM ;

    return 0 ;
}

void Alcatraz::mkdir_entry() 
{
    CString arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;
	    
    char normpath[PATH_MAX], buf[PATH_MAX] ;

    normalizePath(arg0.get().c_str(), normpath) ;
    int maptype = translatePath(normpath, buf, false, tempname);
    switch(maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break;
    case PATH_CREATED:
    case PATH_MODIFIED:
	arg0.set(buf) ;
	break ;
    case PATH_DELETED: {
	mt.delMapping(tempname) ;
	char tempfile[PATH_MAX] ;
	mt.newEntry(TYPE_DIRECTORY, PATH_MODIFIED, tempname, tempfile) ;
	arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("D", normpath, tempfile, "CR", pid, ppid) ;
#endif
	break; 
    }
    case PATH_NEW: {
	int retval = mkdir_failure(buf) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	} else {
	    char tempfile[PATH_MAX] ;
	    mt.newEntry(TYPE_DIRECTORY, PATH_CREATED, buf, tempfile) ;
	    arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("D", buf, tempfile, "CR", pid, ppid) ;
#endif
	}
	break;
    } 
    default:
	break; 
    }
}

int Alcatraz::rmdir_failure(const char *file)
{
    if (!parent_writable(file)) 
	return EPERM ;

    struct stat statbuf ;
    if (stat(file, &statbuf) != 0)
	return EPERM ;
    
    if (!S_ISDIR(statbuf.st_mode))
	return ENOTDIR ;

    // directory not empty
    int count = 0 ;
    struct dirent dent ;
    int fd = open(file, 0) ;
    if (fd >= 0) {
	while (readdir(fd, &dent, 0) != 0) {
	    count ++ ;
	    if (count > 2) break ;
	}
	close(fd) ;
	if (count > 2) return ENOTEMPTY ;
    }
    return 0 ;
}

void Alcatraz::rmdir_entry()
{
    CString arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    char normpath[PATH_MAX], buf[PATH_MAX] ;

    normalizePath(arg0.get().c_str(), normpath) ;
    int maptype = translatePath(normpath, buf, false, tempname);
    switch(maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break ;
    case PATH_DELETED:{
	arch->abortCall(pid, -1, ENOENT);
	break ;
    }		   
    case PATH_CREATED: 
    case PATH_MODIFIED:{
	int retval = rmdir(buf) ;
	if (0 == retval) {
	    mt.delMapping(tempname) ;
	    arch->abortCall(pid, 0, 0) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("D", normpath, "N/A", "DE", pid, ppid) ;
#endif
	} else {
	    arch->abortCall(pid, -1, errno) ;
	}
	break ;
    }
    case PATH_NEW:{
	int retval = rmdir_failure(buf) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	} else {
	    mt.newDelete(TYPE_DIRECTORY, buf) ;
	    arch->abortCall(pid, 0, 0) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("D", buf, "N/A", "DE", pid, ppid) ;
#endif
	}
	break ;
    }
    default:
	break; 
    }

}

int Alcatraz::unlink_failure(const char *file)
{
    if (!parent_writable(file))
	return EACCES ;

    if (access(file, F_OK) != 0)
	return ENOENT ;

    struct stat statbuf; 
    lstat(file, &statbuf) ;
    if (S_ISDIR(statbuf.st_mode))
	return EISDIR ;

    return 0 ;
}

void Alcatraz::unlink_entry()
{
    CString arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    state = -1 ;

    char normpath[PATH_MAX], buf[PATH_MAX];

    if (strlen(arg0.get().c_str()) == 0)
	arch->abortCall(pid, 0, 0) ;

    normalizePath(arg0.get().c_str(), normpath) ;
    int maptype = translatePath(normpath, buf, false, tempname);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break;
    case PATH_DELETED:{
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    }		 
    case PATH_CREATED: {
	state = 0 ;
	arg0.set(buf) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath, "N/A", "DE", pid, ppid) ;
#endif
	break ;
    }
    case PATH_MODIFIED:{
	state = 1 ;
	arg0.set(buf) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath, "N/A", "DE", pid, ppid) ;
#endif
	break ;
    }
    case PATH_NEW:{
	int retval ;
	retval = unlink_failure(buf) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	} 
	else {
	    mt.newDelete(TYPE_FILE, buf) ;
	    arch->abortCall(pid, 0, 0) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("F", buf, "N/A", "DE", pid, ppid) ;
#endif
	}
	break ;
    }
    default:
	break; 
    }
}

void Alcatraz::unlink_exit()
{
    if (state != -1 && mp->currentRC() == 0) {
	switch (state) {
	case 0:
	    mt.delMapping(tempname) ;
	    break ;
	case 1:
	    mt.delMapping(tempname) ;
	    mt.newDelete(TYPE_FILE, tempname) ;
	    break ;
	default:
	    break ;
	}
    }
}

void Alcatraz::socket_entry()
{
}

void Alcatraz::socket_exit()
{
    pid_t pid = mp->pid() ;
    int fd = mp->currentRC() ;
	printf("\nSocket returning fd as %d\n",fd);
    if (fd >= 0) {
	ProcData *pData = processData.lookUp(pid) ;
	if (0 == pData) {
	    pData = new ProcData() ;
	    processData.insert(pid, pData) ;
	}
	
	FDInfo *pFDInfo = pData->allocFDInfo(fd) ;
	if (0 != pFDInfo) {
	    pFDInfo->isSocket = true ;
	    pFDInfo->sockFamily = state ;
	}
    }
}

