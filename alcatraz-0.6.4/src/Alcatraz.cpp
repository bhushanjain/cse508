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

#include <stdio.h>
#include "Alcatraz.h"
#include "DirTools.h"
#include "PathHashTable.h"
#include "MappingTable.h"
#include <MonitoredProc.h>
#include <SystemCall.h>
#include <LinuxTypes.h>
#include <LinuxPtraceX86.h>
#include <SysCallNum.h>

#include <iostream>
#include <unistd.h>
#include <asm/unistd.h>
#include <dirent.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <DirTools.h>
#include <asm/unistd.h>
#include <unistd.h>

using namespace std ;

bool Alcatraz::initialized = false ;

MappingTable mt ;

#ifdef DEBUG_MODE
ofstream logfile("/tmp/Alcatraz.debug") ;
#endif

extern "C" {
    Extension* initAlcatraz() {
	return new Alcatraz();
    }
};

// initialize workingdir and rootdir
void Alcatraz::init()
{
    char buf[PATH_MAX] ;
    getcwd(buf, PATH_MAX) ;
    workingdir = buf ;
    rootdir = "/" ;
}

// Given a path of the isolated process, this function convert it into
// a carnonical and unchrooted path. 
void Alcatraz::normalizePath(const char *orig, char *newpath)
{
    char buf[PATH_MAX] ;

    // convert the original path into absolute path
    if (orig[0] != '/')
	snprintf(buf, PATH_MAX, "%s/%s", workingdir.c_str(), orig) ;
    else 
	strncpy(buf, orig, PATH_MAX);

    strncpy(newpath, "/", PATH_MAX) ;
    char *tok = strtok(buf, "/") ;
    do {
	if (0 == tok) break ;
	if (strcmp(tok, ".")==0)
	    continue ;
	if (strcmp(tok, "..")==0) {
	    int loc = strlen(newpath) ;
	    while (newpath[loc] != '/' && loc >= 0)
		loc -- ;
	    if (0 == loc) 
		newpath[1] = 0 ;
	    else 
		newpath[loc] = 0 ;
	    continue ;
	}
	if (strcmp(newpath, "/")==0)
	    strncat(newpath, tok, PATH_MAX-strlen(newpath)) ;
	else {
	    strncat(newpath, "/", PATH_MAX-strlen(newpath)) ;
	    strncat(newpath, tok, PATH_MAX-strlen(newpath)) ;
	}
    } while ((tok = strtok(0, "/")) != 0) ;

    // if the process had a chroot operation, the root dir will be non-empty
    // we need to prepend the rootdir to the path
    if (strcmp(rootdir.c_str(), "/") != 0) {
	snprintf(buf, PATH_MAX, "%s/%s", rootdir.c_str(), newpath);
	strncpy(newpath, buf, PATH_MAX);
    }
}

// replace the above two functions with this one
bool Alcatraz::inCache(const char *path)
{
    bool retval = false; 
    if (!strncmp(path, mt.cacheloc(), strlen(mt.cacheloc())))
	retval = true;
    return retval; 
}

#define DBG(a) a
int Alcatraz::translatePath(const char *path, char *newpath, bool followlink,
			    char *lastpath)
{
    int retval; 
    char prefix[PATH_MAX], buf[PATH_MAX];
    strncpy(buf, path, PATH_MAX);
    while (true) {
	retval = PATH_NEW;
	DBG(logfile << "PATH: " << buf << endl;)
	if (inCache(buf)) {
	    retval = PATH_NOTALLOWED;
	    break; 
	} else {
	    // translate path using MappingTable
	    if (0 != lastpath)
		strncpy(lastpath, buf, PATH_MAX);
	    retval = mt.getStatus(buf, newpath);
	    DBG(logfile << buf << " -> " << newpath << endl;)

	    if (newpath[strlen(newpath)-1] == '/')
		newpath[strlen(newpath)-1] = 0;

	    // check whether the translated path contains symbolic link
	    // resolve the symbolic link and do this process again
	    // As in Linux, the symbolic link is resolved from left to right
	    bool noSymlink = true;
	    if (PATH_NEW == retval || PATH_DELETED == retval) {
		if (PATH_DELETED == retval)
		    strncpy(newpath, buf, PATH_MAX);
		// resolve symbolic link using normalizePath()
		char *ptr = newpath; 
		int counter = 0;
		while (true) {
		    // extract a prefix from the newpath
		    if (0 == *ptr) break;
		    do {
			if (counter < PATH_MAX)
			    prefix[counter] = *ptr; 
			ptr++;
			counter++;
		    } while (*ptr != 0 && *ptr != '/');
		    prefix[counter] = 0;

		    DBG(logfile << "Checking " << prefix << endl;)
		    // if the operation doen't follow symbolic link
		    // we don't need to check the last element
		    if (followlink || *ptr != 0) {
			// check whether this prefix is a symbolic link
			int linksize = readlink(prefix, buf, PATH_MAX);
			if (linksize > 0) {
			    buf[linksize] = 0; // add the terminating 0 

			    // it is a symbolic link
			    int cnt = counter;
			    while (prefix[cnt] != '/' && cnt >= 0)
				cnt --;
			    prefix[cnt+1] = 0;
			
			    if (buf[0] != '/')
				strncat(prefix, buf, PATH_MAX-strlen(prefix));
			    else 
				strncpy(prefix, buf, PATH_MAX);

			    normalizePath(prefix, buf);
			    DBG(logfile << "Result " << buf << endl;)
			
			    strncat(buf, ptr, PATH_MAX-strlen(buf));
			    DBG(logfile << "Appended rest " << buf << endl;)
			    noSymlink = false;
			    break; 
			}
		    }
		}
		
	    } else if (PATH_CREATED == retval || PATH_MODIFIED == retval) {
		// resolve symbolic link with respect to its original path 
		char *ptr = newpath; 
		int counter = 0;
		while (true) {
		    // extract a prefix from the newpath
		    if (0 == *ptr) break;
		    do {
			if (counter < PATH_MAX)
			    prefix[counter] = *ptr; 
			ptr++;
			counter++;
		    } while (*ptr != 0 && *ptr != '/');
		    prefix[counter] = 0;

		    // if the operation doen't follow symbolic link
		    // we don't need to check the last element
		    if (followlink || *ptr != 0) {
			// check whether this prefix is a symbolic link
			int linksize = readlink(prefix, buf, PATH_MAX);
			if (linksize > 0) {
			    buf[linksize] = 0; // add the terminating 0 
			
			    // the prefix is a symbolic link
			    // If it is a relative symbolic link, 
			    // it should be resolved with respect to its
			    // original path
			    char origpath[PATH_MAX];
			    strncpy(origpath, path, PATH_MAX);
			    
			    // because the rest part of the origpath and 
			    // newpath are same, so we have
			    int cnt = counter+strlen(origpath)-strlen(newpath);
			    origpath[cnt] = 0;
			    while (origpath[cnt] != '/' && cnt >= 0)
				cnt --;
			    origpath[cnt+1] = 0;
			
			    if (buf[0] != '/')
				strncat(origpath, buf, PATH_MAX-strlen(origpath));
			    else
				strncpy(origpath, buf, PATH_MAX);

			    normalizePath(origpath, buf);
			    
			    strncat(buf, ptr, PATH_MAX-strlen(buf));
			    noSymlink = false;
			    break; 
			}
		    }
		}
	    }
	    // if symbolic link resolved, repeat the above process
	    if (noSymlink) 
		break; 
	}
    }
    return retval; 
}

int Alcatraz::deliverEvent() {
    SystemCall *call = mp->getCurCall() ;
    
    switch (call->uscno()) {
    case SYSCALL_ipc:
	// restrict ipc calls
//	mp->getArch()->abortCall(mp->pid(), -1, EPERM);
	break; 
    case SYSCALL_chdir:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "chdir(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    chdir_entry() ;
	}
	else { 
	    chdir_exit() ;
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_fork:
    case SYSCALL_vfork:
    case SYSCALL_clone:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "fork()" ;
#endif
	    fork_entry() ;
	}
	else {
	    fork_exit() ;
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_fchdir:
	if (!call->isEntry()) {
	    fchdir_exit() ;
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "fchdir() = " ;
	    logfile << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_getdents:
	if (call->isEntry())
	    getdents_entry() ;
	else 
	    getdents_exit() ;
	break ;
    case SYSCALL_getdents64:
	if (call->isEntry())
	    getdents64_entry() ;
	else 
	    getdents64_exit() ;
	break ; 
    case SYSCALL_getcwd:
	if (!call->isEntry())
	    getcwd_exit() ;
	break ;

    case SYSCALL_chroot:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "chroot(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    chroot_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;

    case SYSCALL_truncate:
    case SYSCALL_truncate64:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "truncate(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    truncate_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_creat:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "creat(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    creat_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_mknod:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "mknod(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    mknod_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_mkdir:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "mkdir(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    mkdir_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_rmdir:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "rmdir(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    rmdir_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_unlink:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "unlink(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    unlink_entry() ;
	}
	else { 
	    unlink_exit() ;
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_link:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "link(" ;
	    CString arg0(getArgRep(0)) ;
	    CString arg1(getArgRep(1)) ;
	    logfile << arg0.get().c_str() << ", " ;
	    logfile << arg1.get().c_str() << ")" ;
#endif
	    link_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_rename:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "rename(" ;
	    CString arg0(getArgRep(0)) ;
	    CString arg1(getArgRep(1)) ;
	    logfile << arg0.get().c_str() << ", " ;
	    logfile << arg1.get().c_str() << ")" ;
#endif
	    rename_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_symlink:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "symlink(" ;
	    CString arg0(getArgRep(0)) ;
	    CString arg1(getArgRep(1)) ;
	    logfile << arg0.get().c_str() << ", " ;
	    logfile << arg1.get().c_str() << ")" ;
#endif
	    symlink_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_open:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "open(" ;
	    CString arg0(getArgRep(0)) ;
	    Integer arg1(getArgRep(1)) ;
	    logfile << arg0.get().c_str() << ", " << arg1.get() << ")" ;
#endif
	    open_entry() ;
	}
	else {
	    open_exit() ;
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
#ifdef INSTALL_SHIELD
    case SYSCALL_write:
	/*Bhushan: encrypt and then write*/
    case SYSCALL_pwrite:
	if (call->isEntry())
	    write_entry() ;
	break ;
#endif
    case SYSCALL_read:
	/*Bhushan: read and then decrypt*/
    case SYSCALL_pread:
	if (call->isEntry())
	    read_entry() ;
	break ;

// simple operation
    case SYSCALL_execve:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "execve(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    execve_entry() ;
	}
	else {
	    execve_exit() ;
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_access:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "access(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    access_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_utime:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "utime(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    utime_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_readlink:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "readlink(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    readlink_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_statfs:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "statfs(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    statfs_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_stat:
    case SYSCALL_oldstat:
    case SYSCALL_stat64:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "stat(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    stat_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_lstat:
    case SYSCALL_oldlstat:
    case SYSCALL_lstat64:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "lstat(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    lstat_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;

    case SYSCALL_chmod:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "chmod(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    chmod_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_lchown:
    case SYSCALL_lchown32:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "lchown(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    lchown_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;
    case SYSCALL_chown:
    case SYSCALL_chown32:
	if (call->isEntry()) {
#ifdef DEBUG_MODE
	    logfile << mp->pid() << " " << "chown(" ;
	    CString arg0(getArgRep(0)) ;
	    logfile << arg0.get().c_str() << ")" ;
#endif
	    chown_entry() ;
	}
	else {
#ifdef DEBUG_MODE
	    logfile << " = " << mp->currentRC() << endl;
#endif
	}
	break ;

    case SYSCALL_socket:
	if (call->isEntry()) 
	    socket_entry() ;
	else 
	    socket_exit() ;
	break ;
    default:
	break ;
    }

    return 0 ;
}



