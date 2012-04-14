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

extern MappingTable mt ;

// cannot handle the rename of dir
int Alcatraz::rename_failure(const char *file0, const char *file1)
{
    return 0 ;
}

int Alcatraz::rename_failure0(const char *file) 
{
    if (!parent_writable(file))
	return EPERM ;

    if (access(file, F_OK) != 0)
	return ENOENT ;

    struct stat statbuf; 
    lstat(file, &statbuf) ;
    if (S_ISDIR(statbuf.st_mode))
	return EISDIR ;

    return 0 ;
}

int Alcatraz::rename_failure1(const char *file)
{
    if (!parent_writable(file))
	return EPERM ;
    return 0 ;
}

void Alcatraz::rename_entry() 
{
    CString arg0(getArgRep(0)) ;
    CString arg1(getArgRep(1)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    char normpath0[PATH_MAX], buf0[PATH_MAX] ;
    char normpath1[PATH_MAX], buf1[PATH_MAX] ;

    // rename is delete the original file and create or overwrite the 
    // new one

    normalizePath(arg0.get().c_str(), normpath0) ;
    normalizePath(arg1.get().c_str(), normpath1) ;

    int maptype ;
    maptype = translatePath(normpath0, buf0, false, tempname);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break; 
    case PATH_DELETED:{
	arch->abortCall(pid, -1, ENOENT);
	return;
    }		 
    case PATH_CREATED: {
	mt.delMapping(tempname) ;
	arg0.set(buf0) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath0, "N/A", "DE", pid, ppid) ;
#endif
	break ;
    }
    case PATH_MODIFIED:{
	mt.delMapping(tempname) ;
	mt.newDelete(TYPE_FILE, tempname) ;
	arg0.set(buf0) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath0, "N/A", "DE", pid, ppid) ;
#endif
	break ;
    }
    case PATH_NEW:{
	int retval ;
	retval = rename_failure0(buf0) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	    return ;
	} 
	else {
	    char tempfile[PATH_MAX] ;
	    mt.isolate(buf0, tempfile, false) ;
	    mt.delMapping(buf0) ;
	    mt.newDelete(TYPE_FILE, buf0) ;
	    arg0.set(tempfile) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("F", normpath0, "N/A", "DE", pid, ppid) ;
#endif
	}
	break ;
    }
    default:
	break; 
    }

    maptype = translatePath(normpath1, buf1, false, tempname);
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
	mod_log("F", normpath1, tempfile, "CR", pid, ppid) ;
#endif
	arg1.set(tempfile) ;
	break ;
    }		    
    case PATH_CREATED:
    case PATH_MODIFIED:{
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	if (access(buf1, F_OK) == 0)
	    mod_log("F", normpath1, buf1, "MD", pid, ppid) ;
	else
	    mod_log("F", normpath1, buf1, "CR", pid, ppid) ;
#endif
	arg1.set(buf1) ;
	break ;
    }
    case PATH_NEW:{
	int retval = rename_failure1(buf1) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	    return ;
	} else {
	    int mtype ;
	    if (access(buf1, F_OK) == 0) {
		mtype = PATH_MODIFIED ;
	    } else {
		mtype = PATH_CREATED;
	    }
	    char tempfile[PATH_MAX] ;
	    mt.newEntry(TYPE_FILE, mtype, buf1, tempfile) ;
	    arg1.set(tempfile) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    if (PATH_MODIFIED == mtype)
		mod_log("F", normpath1, tempfile, "MD", pid, ppid) ;
	    else 
		mod_log("F", normpath1, tempfile, "CR", pid, ppid) ;
#endif
	}
    }
    }

}


int Alcatraz::symlink_failure(const char *file)
{
    if (!parent_writable(file))
	return EPERM ;
    return 0 ;
}


void Alcatraz::symlink_entry()
{
    //CString arg0(getArgRep(0)) ;
    CString arg1(getArgRep(1)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;
    
    
    char normpath[PATH_MAX], buf[PATH_MAX] ;
    
    normalizePath(arg1.get().c_str(), normpath) ;
    int maptype = translatePath(normpath, buf, false, tempname);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break;
    case PATH_DELETED:{
	mt.delMapping(tempname) ;
	char tempfile[PATH_MAX] ;
	mt.newEntry(TYPE_FILE, PATH_MODIFIED, tempname, tempfile) ;
	arg1.set(tempfile) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath, tempfile, "CR", pid, ppid) ;
#endif
	break ;
    }		    
    case PATH_CREATED:
    case PATH_MODIFIED:{
	arg1.set(buf) ;
	break ;
    }
    case PATH_NEW:{
	int retval = symlink_failure(buf) ;
	if (retval !=0) {
	    arch->abortCall(pid, -1, retval) ;
	}
	else {
	    char tempfile[PATH_MAX] ;
	    mt.newEntry(TYPE_FILE, PATH_CREATED, buf, tempfile) ;
	    arg1.set(tempfile) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("F", normpath, tempfile, "CR", pid, ppid) ;
#endif
	}
	break ;
    }
    }
    
}

int Alcatraz::link_failure(const char *file0, const char *file1) 
{
    return 0 ;
}

int Alcatraz::link_failure0(const char *file) 
{
    struct stat statbuf; 
    lstat(file, &statbuf) ;
    if (S_ISDIR(statbuf.st_mode))
	return EPERM ;
    
    return 0 ;
}

int Alcatraz::link_failure1(const char *file)
{
    if (!parent_writable(file))
	return EACCES ;
    if (access(file, F_OK) == 0)
	return EEXIST ;

    return 0 ;
}

void Alcatraz::link_entry()
{
    CString arg0(getArgRep(0)) ;
    CString arg1(getArgRep(1)) ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    char normpath0[PATH_MAX], buf0[PATH_MAX];
    char normpath1[PATH_MAX], buf1[PATH_MAX] ;

    normalizePath(arg0.get().c_str(), normpath0) ;
    normalizePath(arg1.get().c_str(), normpath1) ;

    int retval = link_failure(normpath0, normpath1) ;
    if ( retval != 0) 
	arch->abortCall(pid, -1, retval) ;

    // no need to check the first argument

    int maptype ;
    maptype = translatePath(normpath0, buf0, false);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT) ;
	return ;
    case PATH_DELETED:
	arch->abortCall(pid, -1, ENOENT) ;
	return ;
    case PATH_CREATED: 
    case PATH_MODIFIED:{
	break ;
    }
    case PATH_NEW:{
	int retval ;
	retval = link_failure0(normpath0) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	    return ;
	} 
	else {
	    strncpy(buf0, normpath0, PATH_MAX) ;
	}
	break ;
    }
    default:
	break; 
    }

    maptype = translatePath(normpath1, buf1, false, tempname);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT);
	break;
    case PATH_DELETED:{
	mt.delMapping(tempname) ;
	mt.newEntry(TYPE_FILE, PATH_MODIFIED, tempname, buf1) ;
#ifdef INSTALL_SHIELD
	pid_t ppid = mp->ppid() ;
	mod_log("F", normpath1, buf1, "CR", pid, ppid) ;
#endif
	    
	break ;
    }		    
    case PATH_CREATED:
    case PATH_MODIFIED:{
	arch->abortCall(pid, -1, EEXIST) ;
	break ;
    }
    case PATH_NEW:{
	int retval = link_failure1(buf1) ;
	if (retval != 0) {
	    arch->abortCall(pid, -1, retval) ;
	    return ;
	} else {
	    char tempfile[PATH_MAX] ;
	    mt.newEntry(TYPE_FILE, PATH_CREATED, buf1, tempfile) ;
	    strncpy(buf1, tempfile, PATH_MAX) ;
#ifdef INSTALL_SHIELD
	    pid_t ppid = mp->ppid() ;
	    mod_log("F", normpath1, tempfile, "CR", pid, ppid) ;
#endif
	}
    }
    }

    // copy buf0 to buf1 

    int fd0, fd1 ;
    fd0 = open(buf0, O_RDONLY) ;
    fd1 = open(buf1, O_CREAT|O_WRONLY|O_TRUNC, 0777) ;
    if (fd0 < 0 || fd1 < 0) {
	arch->abortCall(pid, -1, ENOENT) ;
	return ;
    }
    struct stat statbuf ;
    if (fstat(fd0, &statbuf) == 0)
	fchmod(fd1, statbuf.st_mode) ;
    
    char tempbuf[512] ;
    while (true) {
	int count = read(fd0, tempbuf, 512) ;
	if (count > 0)
	    write(fd1, tempbuf, count) ;
	else 
	    break ;
    }
    close(fd0) ;
    close(fd1) ;
    
    

    arch->abortCall(pid, 0, 0) ;
}
