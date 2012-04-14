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
#include "CommonDefs.h"
#include "GUIInterface.h"
#include "MappingTable.h"
#include <LinuxTypes.h>
#include <MonitoredProc.h>
#include <ArchDependent.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

extern MappingTable mt ;
extern IDInfo<ProcData> processData ;
int getdents(uint fd, struct dirent * dirp, uint count);
int getdents64(uint fd, struct dirent * dirp, uint count);

#ifdef INSTALL_SHIELD
#include <unistd.h>
#include <openssl/sha.h>
#include <fstream>

IDInfo<char> processCommand ;
IDInfo<char> procCmdSHA ;
long serialnum = 10000000 ;

ofstream logfile("/tmp/modification.log") ;

void mod_log(char *filetype, char *filepath, char *newpath, char *status, pid_t pid, pid_t ppid)
{
    char buf[512] ;
    snprintf(buf, 512, "MODIFICATION: %ld %ld %s %s %s %s %d %d\n", serialnum, time(0), filetype, filepath, status, newpath, pid, ppid) ;
    sendfile(buf, strlen(buf)+1);
    logfile << buf ;
    serialnum ++ ;
}

void newproc_log(pid_t pid, pid_t ppid) 
{
    char buf[512];
    snprintf(buf, 512, "NEWPROCESS: %ld %ld %d %d\n", serialnum, time(0), pid, ppid);
    sendfile(buf, strlen(buf)+1);
    logfile << buf;
    serialnum ++;
}

void exec_log(pid_t pid, const char *shasum, const char *wd,const char *cmd)
{
    char buf[512];
    snprintf(buf, 512, "EXECUTE: %ld %ld %d %s %s %s\n", serialnum, time(0),
	     pid, shasum == 0? "NULL":shasum, wd, cmd);
    sendfile(buf, strlen(buf)+1);
    logfile << buf;
    serialnum ++;
}

const char *sha1sum(const char *file)
{
    SHA_CTX ctx ;
    int fd, count ;
    char buf[1024] ;
    unsigned char shabuf[20] ;
    static char output[41] ;
    char newbuf[PATH_MAX] ;
    const char *tmp ;

    SHA1_Init(&ctx) ;
    int maptype ;
    maptype = mt.mapping.findMapping(file, newbuf, PATH_MAX) ;
    switch (maptype) {
    case PATH_CREATED:
    case PATH_MODIFIED:
	tmp = newbuf ;
	break ;
    default:
	tmp = file ;
	break ;
    }
    fd = open(tmp, 0) ;
    if (fd < 0) 
	return 0 ;

    while ((count = read(fd, buf, 1024)) > 0) {
	SHA1_Update(&ctx, buf, count) ;
    }

    SHA1_Final(shabuf, &ctx) ;

    output[0] = 0 ;

    for (count = 0 ; count < 20 ; count++) {
	snprintf(output+count*2, 2, "%x", shabuf[count]/16) ;
	snprintf(output+count*2+1, 2, "%x", shabuf[count]%16) ;
    }

    close(fd) ;
    return output ;
}

#endif

void Alcatraz::chdir_entry()
{
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    CString arg0(getArgRep(0)) ;
    char normpath[PATH_MAX], buf[PATH_MAX] ;

    normalizePath(arg0.get().c_str(), normpath) ;
    strncpy(tempname, normpath, PATH_MAX) ;
    int maptype = translatePath(normpath, buf, true);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    case PATH_DELETED:{
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    }		    
    case PATH_CREATED:
    case PATH_MODIFIED:{
	arg0.set(buf) ;
	break ;
    }
    default: 
	arg0.set(normpath) ;
    }
}

void Alcatraz::chdir_exit()
{
    if (mp->currentRC() == 0) {
	workingdir = tempname ;
    }
}

void Alcatraz::fchdir_exit()
{
    Integer arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    int fd = arg0.get() ;

    if (mp->currentRC() == 0) {
	ProcData *pData = processData.lookUp(pid) ;
	if (0 == pData) return ; // no data for this process
	
	FDInfo *pFDInfo = pData->getFDInfo(fd) ;
	if (0 == pFDInfo) return  ; // no data for this file
	
	workingdir = pFDInfo->filename ;
    }
}

void Alcatraz::fork_entry()
{
}

void Alcatraz::fork_exit()
{
#ifdef INSTALL_SHIELD
    pid_t pid = mp->pid() ;
    pid_t childpid = mp->currentRC() ;

    if (childpid > 0) {
	newproc_log(childpid, pid);
    }
#endif
}

void Alcatraz::getdents_entry()
{
    pid_t pid = mp->pid() ;
    Integer arg0(getArgRep(0)) ;

    int fd = arg0.get() ;
    ProcData *pData = processData.lookUp(pid) ;
    if (0 == pData) return ; // no data for this process

    FDInfo *pFDInfo = pData->getFDInfo(fd) ;
    if (0 == pFDInfo) return  ; // no data for this file

    if (mt.mapping.hasChanges(pFDInfo->filename)) {
	if (pFDInfo->isFirst) {
	    // this is the first getdents() call to this directory
	    pFDInfo->isFirst = false ;

	    struct stat statbuf ;
	    if (stat(pFDInfo->filename, &statbuf) < 0) return ;
	    char *tmp = (char *)malloc(statbuf.st_size) ;
	    if (0 == tmp) return ;

	    int dirfd = open(pFDInfo->filename, O_RDONLY) ;
	    if (dirfd < 0) {
		free(tmp) ;
		return ;
	    }	

	    // create the filters
	    DirChange dchg ;
	    mt.mapping.getChanges(pFDInfo->filename, &dchg) ;

	    DentReader dr ;
	    DentWriter dw(statbuf.st_size) ;
	    struct dirent *pDir ;

	    while (1) {
		int retval ;
		retval = getdents(dirfd, (struct dirent*)tmp, statbuf.st_size) ;
		if (0 == retval) break ;
		dr.set(tmp, retval) ;
	    
		while ((pDir = dr.getNext()) != 0) { 
		    if (!dchg.isDeleted(pDir->d_name)) 
			dw.put(pDir) ;
		}
	    }
	
	    struct dirent added ;
	    char *newfile ;
	    while ((newfile = dchg.getNextNew()) != 0) {
		added.d_reclen = sizeof(added) ;
		strncpy(added.d_name, newfile, 256) ;
		added.d_ino = 1 ;
		dw.put(&added) ;
	    }
	    pFDInfo->dReader.set(dw.get(), dw.size()) ;
	    dr.set(dw.get(), dw.size()) ;
	
	}
    }
}

void Alcatraz::getdents_exit()
{
    pid_t pid = mp->pid() ;
    Integer arg0(getArgRep(0)) ;
    Integer arg2(getArgRep(2)) ;

    int fd = arg0.get() ;

    ProcData *pData = processData.lookUp(pid) ;
    if (0 == pData) return ; // no data for this process

    FDInfo *pFDInfo = pData->getFDInfo(fd) ;
    if (0 == pFDInfo) return  ; // no data for this file

    if (mt.mapping.hasChanges(pFDInfo->filename)) {
	size_t size = arg2.get() ;
	DentWriter dw(size) ;
	struct dirent *pDir ;
    
	while (1) {
	    pDir = pFDInfo->dReader.getNext() ;
	    if (0 == pDir) break ;
	    if (size - dw.size() >= pDir->d_reclen) 
		dw.put(pDir) ;
	    else {
		pFDInfo->dReader.back() ;
		break ;
	    }
	}

	//copy the writer buffer to the return value and return writer's size
	VoidP arg1(getArgRep(1), dw.size()) ;
 	arg1.set(dw.get()) ;
 	mp->getArch()->setReturnVal(pid, dw.size(), 0) ;
    
    }
}

void Alcatraz::getdents64_entry()
{
    pid_t pid = mp->pid() ;
    Integer arg0(getArgRep(0)) ;

    int fd = arg0.get() ;
    ProcData *pData = processData.lookUp(pid) ;
    if (0 == pData) return ; // no data for this process

    FDInfo *pFDInfo = pData->getFDInfo(fd) ;
    if (0 == pFDInfo) return  ; // no data for this file

    /* if there is no changes on the directory, no need to do the actions */

    if (mt.mapping.hasChanges(pFDInfo->filename)) {
	if (pFDInfo->isFirst) {
	    // this is the first getdents() call to this directory
	    pFDInfo->isFirst = false ;

	    struct stat statbuf ;
	    if (stat(pFDInfo->filename, &statbuf) < 0) return ;
	    char *tmp = (char *)malloc(statbuf.st_size) ;
	    if (0 == tmp) return ;

	    int dirfd = open(pFDInfo->filename, O_RDONLY) ;
	    if (dirfd < 0) {
		free(tmp) ;
		return ;
	    }	

	    // create the filters
	    DirChange dchg ;
	    mt.mapping.getChanges(pFDInfo->filename, &dchg) ;
	
	    DentReader64 dr ;
	    DentWriter64 dw(statbuf.st_size) ;
	    struct dirent64 *pDir ;

	    while (1) {
		int retval ;
		retval = getdents64(dirfd, (struct dirent*)tmp, statbuf.st_size) ;
		if (0 == retval) break ;
		dr.set(tmp, retval) ;
	    
		while ((pDir = dr.getNext()) != 0) { 
		    // we can also do something to hide the modification cache
		    if (!dchg.isDeleted(pDir->d_name)) 
			dw.put(pDir) ;
		}
	    }
	
	    struct dirent64 added ;
	    char *newfile ;
	    while ((newfile = dchg.getNextNew()) != 0) {
		added.d_reclen = sizeof(added) ;
		strncpy(added.d_name, newfile, 256) ;
		added.d_ino = 1 ;
		dw.put(&added) ;
	    }
	    pFDInfo->dReader64.set(dw.get(), dw.size()) ;
	    dr.set(dw.get(), dw.size()) ;
	
	}
    }
}

void Alcatraz::getdents64_exit()
{
    pid_t pid = mp->pid() ;
    Integer arg0(getArgRep(0)) ;
    DirentP arg1(getArgRep(1)) ;
    Integer arg2(getArgRep(2)) ;

    int fd = arg0.get() ;

    ProcData *pData = processData.lookUp(pid) ;
    if (0 == pData) return ; // no data for this process

    FDInfo *pFDInfo = pData->getFDInfo(fd) ;
    if (0 == pFDInfo) return  ; // no data for this file

    if (mt.mapping.hasChanges(pFDInfo->filename)) {
	size_t size = arg2.get() ;

	DentWriter64 dw(size) ;
	struct dirent64 *pDir ;
    
	while (1) {
	    pDir = pFDInfo->dReader64.getNext() ;
	    if (0 == pDir) break ;
	    if (size - dw.size() >= pDir->d_reclen) 
		dw.put(pDir) ;
	    else {
		pFDInfo->dReader64.back() ;
		break ;
	    }
	}

	//copy the writer buffer to the return value and return writer's size
	VoidP arg1(getArgRep(1), dw.size()) ;
 	arg1.set(dw.get()) ;
 	mp->getArch()->setReturnVal(pid, dw.size(), 0) ;
    }
    
}

void Alcatraz::chroot_entry()
{
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    arch->abortCall(pid, -1, EPERM) ;
}

/*
 * The simplest operations do not involve file or directory modification
 * They only read the content or status of the specified file. 
 * For those operations, what we need to do is to check the status of the
 * file and perform the corresponding operations. 
 */

int Alcatraz::simple_mapping(int argnum, bool followlink) 
{
    CString arg(getArgRep(argnum)) ;
    char normpath[PATH_MAX], buf[PATH_MAX] ;
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    /* first get the real path of the file */
    normalizePath(arg.get().c_str(), normpath) ;
    int maptype = translatePath(normpath, buf, followlink);
    switch (maptype) {
    case PATH_NOTALLOWED:
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    case PATH_DELETED:
	arch->abortCall(pid, -1, ENOENT) ;
	break ;
    case PATH_CREATED:
    case PATH_MODIFIED:{
	arg.set(buf) ;
	break ;
    }
    case PATH_NEW: 
	/* if use relative path, ../something, 
	   and if the absolute path is not mapped, 
	   there the relative path will refer to that in 
	   modification cache */
	arg.set(normpath) ; 
	break ;
    }
    return maptype ;
}

void Alcatraz::execve_entry()
{
#ifdef INSTALL_SHIELD
    pid_t pid = mp->pid() ;

    CString arg0(getArgRep(0)) ;
    const int ENTRYSIZE = 2048 ;
    char temp[ENTRYSIZE];
    const char *sha = 0;

    if (0 != temp) {
	snprintf(temp, ENTRYSIZE, "%s", arg0.get().c_str()) ;
	ArgV arg1(getArgRep(1)) ;
	const char **argv = arg1.get() ;
	if (argv[1] != 0) {
	    sha = sha1sum(argv[1]) ;
	}
	int count = 0 ;
	while (argv[count] != 0) {
	    strncat(temp, " ", ENTRYSIZE - strlen(temp) - 1) ;
	    strncat(temp, argv[count], ENTRYSIZE - strlen(temp) - 1) ;
	    count ++ ; 
	}
	
	exec_log(pid, sha, workingdir.c_str(), temp);
    } 
#endif
    simple_mapping(0); 
}

void Alcatraz::execve_exit()
{
#ifdef INSTALL_SHIELD
    pid_t pid = mp->pid() ;
    long retcode = mp->currentRC() ;
    if (retcode == 0) {
	exec_log(pid, "0", "", "");
    }
    else 
	exec_log(pid, "-1", "", "");
#endif
}

void Alcatraz::utime_entry()
{
    simple_mapping(0) ;
}

void Alcatraz::access_entry()
{
    CString arg0(getArgRep(0)) ;

    /* the /.alcatraz is a virtual directory for a readonly access to the
       main version of the files */

    if (strncmp("/.alcatraz", arg0.get().c_str(), 10) == 0) {
	arg0.set(arg0.get().c_str()+10) ;
	return ;
    }

    simple_mapping(0) ;
}


void Alcatraz::readlink_entry()
{
    simple_mapping(0, false) ;
}

void Alcatraz::statfs_entry()
{
    simple_mapping(0) ;
}

void Alcatraz::stat_entry()
{
    CString arg0(getArgRep(0)) ;

    if (strncmp("/.alcatraz", arg0.get().c_str(), 10) == 0) {
	arg0.set(arg0.get().c_str()+10) ;
	return ;
    }

    simple_mapping(0) ;
}

void Alcatraz::lstat_entry()
{
    CString arg0(getArgRep(0)) ;

    if (strncmp("/.alcatraz", arg0.get().c_str(), 10) == 0) {
	arg0.set(arg0.get().c_str()+10) ;
	return ;
    }

    simple_mapping(0, false) ;
}

void Alcatraz::getcwd_entry()
{
    pid_t pid = mp->pid() ;
    ArchDep *arch = mp->getArch() ;

    ULong arg0(getArgRep(0)) ;
    arch->abortCall(pid, arg0.get(), 0) ;
}

void Alcatraz::getcwd_exit()
{
    if (mp->currentRC() != 0) {
    Size_t arg1(getArgRep(1)) ;
    char buf[PATH_MAX] ;
    size_t sz ;
    if (arg1.get() > workingdir.size()+1)
	sz = workingdir.size()+1 ;
    else 
	sz = arg1.get() ;
    VoidP arg0(getArgRep(0), sz) ;
    strncpy(buf, workingdir.c_str(), PATH_MAX) ;
    arg0.set(buf) ;
    }
}

#ifdef INSTALL_SHIELD
void Alcatraz::write_entry()
{
    Integer arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    pid_t ppid = mp->ppid() ;

    int fd = arg0.get() ;
    ProcData *pData = processData.lookUp(pid) ;
    if (0 == pData) return ; // no data for this process

    FDInfo *pFDInfo = pData->getFDInfo(fd) ;
    if (0 == pFDInfo) return  ; // no data for this file
	
//	if(pFDInfo->isSocket)
//		printf("\nSocket writing to %s\n",pFDInfo->filename);
	
//    if (!pFDInfo->isSocket) {
	char buf[PATH_MAX] ;
	int maptype ;
	maptype = mt.getStatus(pFDInfo->filename, buf) ;
	mt.mapping.appendTime(pFDInfo->filename, pid, ppid) ;
	mod_log("F", pFDInfo->filename, buf, "MD", pid, ppid) ;
//    }
}
#endif

void Alcatraz::read_entry()
{
    Integer arg0(getArgRep(0)) ;
    pid_t pid = mp->pid() ;
    int fd = arg0.get() ;

    ProcData *pData = processData.lookUp(pid) ;
    if (0 == pData) return ; // no data for this process

    FDInfo *pFDInfo = pData->getFDInfo(fd) ;
    if (0 == pFDInfo) return  ; // no data for this file
	
//	if(pFDInfo->isSocket)
//		printf("\nSocket reading from %s\n",pFDInfo->filename);
    if (access(pFDInfo->filename, F_OK)==0)// && !pFDInfo->isSocket)
	mt.mapping.record_time(pFDInfo->filename) ;
}
//Bhushan: Update read and write entry to encrypt the data before storing.
//Bhushan: Add hooks for read and write exit to decrypt the data before returning.