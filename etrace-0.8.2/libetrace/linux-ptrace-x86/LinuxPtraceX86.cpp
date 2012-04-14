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

#include "LinuxPtraceX86.h"
#include "SysCallNum.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <asm/unistd.h>
#include <linux/limits.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>

using namespace std ;

#ifdef UI_DEBUG_MODE
ofstream logfile ;
#endif

void err_out(const char *id, const char *msg) 
{
    cerr << id << msg << endl ;
#ifdef UI_DEBUG_MODE
    logfile << id << msg << endl ;
#endif 
}

LinuxPtraceX86::LinuxPtraceX86()
{
    initScInfo() ;
#ifdef UI_DEBUG_MODE
    logfile.open("/tmp/etrace.log", ios::trunc) ;
    logfile.close() ;
    logfile.open("/tmp/etrace.log", ios::app) ;
#endif 
}

LinuxPtraceX86::~LinuxPtraceX86()
{

}

long LinuxPtraceX86::getReg(pid_t pid, int reg)
{
    errno = 0;
    long x = ptrace(PTRACE_PEEKUSER, pid, 4*reg, 0);
    if (errno != 0) {
	err_out("LinuxPtraceX86::getReg ", strerror(errno)) ;
    }

    return x;
}

int LinuxPtraceX86::setReg(pid_t pid, int reg, long val)
{
    return ptrace(PTRACE_POKEUSER, pid, 4*reg, val);
}

/*
 * Get the Data of size 'len' bytes from 'addr' into 'buf'.
 * the first attempt is to use the /proc file system.
 * But the strange thing is that for high memary address, 
 * it will fail on RedHat 7.3( but it is fine for RH 6.2 ).
 * In such case, I use the ptrace instead, four bytes at a time.
 */
 
int LinuxPtraceX86::getData(pid_t pid, long addr, char* buf, size_t len,
			    int buffermode, size_t size)
{
    char memfilename[512];
    int ret, memfd;

    if (0 == len)
	return 0;

    ProcInfo *pInfo = procTable.lookUp(pid) ;
    if (0 == pInfo) {
	err_out("LinuxPtraceX86::getData ", "No such process") ;
	return -1 ;
    }

    switch (buffermode) {
    case GETDATA_NO_BUFFER:
	break ;
    case GETDATA_START_BUFFER:
	pInfo->start_addr = 0 ;
	pInfo->buffer_size = 0 ;
	if (pInfo->buffer != 0) {
	    delete pInfo->buffer ;
	    pInfo->buffer = 0 ;
	}
	break ;
    case GETDATA_USE_BUFFER:
	if ((addr >= pInfo->start_addr) && (addr + len <= pInfo->start_addr + pInfo->buffer_size) && pInfo->buffer != 0) {
	    memcpy(buf, pInfo->buffer+addr-pInfo->start_addr, len) ;
	    return len ;
	}
    default:
	break ;
    }
 
    snprintf(memfilename, 512, "/proc/%d/mem", pid);
    memfd = open(memfilename, O_RDONLY);
    if (memfd < 0) {
	goto ptrace_get ;
    }

    char *tmp ;
    size_t readsize ;
    readsize = len ;
    tmp = buf ;
    if (GETDATA_START_BUFFER == buffermode) {
	if (size > len) {
	    pInfo->buffer = new char[size] ;
	    if (0 != pInfo->buffer) {
		tmp = pInfo->buffer ;
		readsize = size ;
	    }
	}
    }

    if (lseek(memfd, addr, SEEK_SET) < 0 || (ret = read(memfd, tmp, readsize)) < 0) {
	if (GETDATA_START_BUFFER == buffermode && pInfo->buffer != 0) {
	    delete pInfo->buffer ;
	    pInfo->buffer = 0 ;
	}
	close(memfd) ;
	goto ptrace_get ;
    }
    if (GETDATA_START_BUFFER == buffermode && tmp != buf) {
	pInfo->start_addr = addr ;
	pInfo->buffer_size = ret ;
	memcpy(buf, pInfo->buffer, len) ;
    }
    close(memfd);
    return ret;
    
ptrace_get: 
    size_t count = 0 ;
    int i, j ;

    long retval ;
    char *temp ;

    for (i = addr ; ; i += 4){
	retval = ptrace(PTRACE_PEEKDATA, pid, i, NULL) ;
	if ((errno != 0)){
	    if (count == 0) {
		return -1 ;
	    }
	    return count ;
	}
	for (j = 0 ; j < 4 ; j++){
	    temp = (char*)&retval ;
	    buf[count] = *(temp+j) ;
	    count ++ ;
	    if (count == len) 
		return count ;
	}
    }
}

int LinuxPtraceX86::setData(pid_t pid, long addr, const char *c, size_t len)
{
    int i, retval, count ;
    long *data ;

    count = (len)/4;
    long origaddr;

    origaddr = addr;
	
//	printf("\nSetData received %d, %08x, %s, %d\n",pid,addr,c,len);
	char ch;
    for (i = 0; i <= count ; i++) {
	data = (long *)(c+i*4);
	retval = ptrace(PTRACE_POKEDATA, pid, (unsigned long)addr, *data);
	if (retval < 0) {
	    err_out("[A]LinuxPtraceX86::setData ", strerror(errno)) ;
		cin >> ch;
	    return -1 ;
	}
	addr = addr + sizeof(long);
    }

//	int j;
//	for(j=4*i;j<len;j++)
//	{
//		char *a = (char *)&tmp ;
//		
//	}
//    if (len % 4)	{
//	long tmp = ptrace(PTRACE_PEEKDATA, pid, (char *)c+i*4, 0) ;
//	if (errno != 0) {
//	    err_out("[B]LinuxPtraceX86::setData ", strerror(errno)) ;
//		cin >> ch;
//	    return -1 ;
//	}
//
//	char *a = (char *)&tmp ;
//
//	memcpy(a, c+i*4, len%4) ;
//	data = &tmp;
//	
//	retval = ptrace(PTRACE_POKEDATA, pid, (unsigned long)addr, *data) ;
//	if (retval < 0) {
//	    err_out("[C]LinuxPtraceX86::setData ", strerror(errno)) ;
//		cin >> ch;
//	    return -1 ;
//	}
//    }
    return 0 ;
}

int LinuxPtraceX86::getUID(pid_t pid)
{
    char memfilename[512], line[512], word[512] ;
    int uid, euid ;

    sprintf(memfilename, "/proc/%d/status", pid) ;
    FILE* memfd = fopen(memfilename, "r") ;
    if (memfd == NULL)	{
	return -1 ;
    }

    while(fgets(line, 512, memfd) != NULL){
	sscanf(line, "%s %d %d", word, &uid, &euid) ;
	if(strcmp(word, "Uid:") == 0) {
	    fclose(memfd) ;
	    return uid ;
	}
	
    }
    return -1 ;
}

int LinuxPtraceX86::getEUID(pid_t pid)
{
    char memfilename[512], line[512], word[512] ;
    int uid, euid ;

    sprintf(memfilename, "/proc/%d/status", pid) ;
    FILE* memfd = fopen(memfilename, "r") ;
    if (memfd == NULL)	{
	return -1 ;
    }

    while(fgets(line,512,memfd)!=NULL){
	sscanf(line,"%s %d %d",word,&uid,&euid);
	if(strcmp(word,"Uid:")==0) {
	    fclose(memfd);
	    return euid;
	}
	
    }
    return -1;
}

int LinuxPtraceX86::getGID(pid_t pid)
{
    char memfilename[512],line[512],word[512];
    int gid,egid;

    sprintf(memfilename, "/proc/%d/status", pid);
    FILE* memfd = fopen(memfilename,"r");
    if (memfd == NULL)	{
	return -1;
    }

    while(fgets(line,512,memfd)!=NULL){
	sscanf(line,"%s %d %d",word,&gid,&egid);
	if(strcmp(word,"Gid:")==0) {
	    fclose(memfd);
	    return gid;
	}
	
    }
    return -1;
}

int LinuxPtraceX86::getEGID(pid_t pid)
{
    char memfilename[512],line[512],word[512];
    int gid,egid;

    sprintf(memfilename, "/proc/%d/status", pid);
    FILE* memfd = fopen(memfilename,"r");
    if (memfd == NULL)	{
	return -1;
    }

    while(fgets(line,512,memfd)!=NULL){
	sscanf(line,"%s %d %d",word,&gid,&egid);
	if(strcmp(word,"Gid:")==0) {
	    fclose(memfd);
	    return egid;
	}
	
    }
    return -1;
}

pid_t LinuxPtraceX86::getPPID(pid_t pid) 
{
    char memfilename[512],line[512],word[512];
    int ppid;

    sprintf(memfilename, "/proc/%d/status", pid);
    FILE* memfd = fopen(memfilename, "r");
    if (memfd == 0)	{
	return -1;
    }

    while (fgets(line, 512, memfd) != 0){
	sscanf(line, "%s %d", word, &ppid) ;
	if (strcmp(word, "PPid:") == 0) {
	    fclose(memfd) ;
	    return ppid ;
	}
    }
    return -1;

}

long LinuxPtraceX86::allocMem(pid_t pid, size_t size) {
    ProcInfo *pInfo ;
    pInfo = procTable.lookUp(pid) ;
    if (0 == pInfo) 
	return 0 ;

    const int MAX_SIZE = 1000000 ; 

    /* get the current "stack pointer" */
    long addr ;
    if (0 == pInfo->newStackPtr) 
	addr = getReg(pid, UESP) ;
    else 
	addr = pInfo->newStackPtr ;

    addr = addr - (random()/RAND_MAX)*MAX_SIZE - size - 4 ;

    // test whether the new address is accessable
    pInfo->newStackPtr = addr ;
    
#ifdef UI_DEBUG_MODE
    logfile << hex << addr << dec << endl ;
#endif
    return addr ;
}

long LinuxPtraceX86::getCallNum(pid_t pid)
{
    return getReg(pid, ORIG_EAX) ;
}

int LinuxPtraceX86::setCallNum(pid_t pid, long scno)
{
    return setReg(pid, ORIG_EAX, scno) ;
}

int LinuxPtraceX86::getReturnVal(pid_t pid, int *errnum)
{
    int retval, errvalue;
    long rawrc = getReg(pid, EAX) ;
    
    if (rawrc < 0) {
	errvalue = -rawrc ;
	retval = -1 ;
    } else {
	errvalue = 0 ;
	retval = rawrc ;
    }

    if (0 != errnum)
	*errnum = errvalue; 

    return retval ;
}

int LinuxPtraceX86::setReturnVal(pid_t pid, int rc, int errnum)
{
    long rawrc ;
    if (rc == -1) {
	if (0 == errnum)
	    rawrc = -1 ;
	else
	    rawrc = -errnum ;
    }
    else 
	rawrc = rc ;

    return setReg(pid, EAX, rawrc) ;
}

pid_t LinuxPtraceX86::startTracing(const char *path, char **argv) 
{
    /*
     * Start the monitoring and monitored processes
     */
    pid_t pid ;
    switch (pid = fork()) {
    case -1:
	err_out("LinuxPtraceX86::startTracing", "Cannot create process!") ;
	return -1;
    case 0: {
	/*
	 * This is the program to trace
	 */
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
	    err_out("LinuxPtraceX86::startTracing", "Cannot setup tracing!") ;
	    return -1 ;
	}
	
	execv(path, argv) ;
	err_out("LinuxPtraceX86::startTracing Program execution error: ", 
		strerror(errno)) ;
	exit(1) ;
    }
    default: {
	/* insert the first process into procTable */
	ProcInfo *ptr = new ProcInfo() ;
	if (0 == ptr) {
	    kill(pid, SIGKILL) ;
	    return -1 ;
	}
	procTable.insert(pid, ptr) ;
	ptr->pid = pid ;
	break ;
    }
    }
    return pid ;
}    

int LinuxPtraceX86::attachProc(pid_t pid)
{
    int retval = ptrace(PTRACE_ATTACH, pid, (char *)1, 0) ;
    if (0 == retval) {
	ProcInfo *pInfo = new ProcInfo ;
	if (0 == pInfo) {
	    kill(pid, SIGKILL) ;
	    err_out("LinuxPtraceX86::attachProc", 
		    "Cannot alloc control record!") ;
	    retval = -1 ;
	} else {
	    pInfo->pid = pid ;
	    procTable.insert(pid, pInfo) ;
	}
    }
    return retval ;
}

int LinuxPtraceX86::internal_detach(pid_t pid, int signal)
{
    return 0 ;
}

int LinuxPtraceX86::detachProc(pid_t pid) 
{
    ProcInfo *pInfo = procTable.lookUp(pid) ;
    if (pInfo != 0) {
	if (pInfo->omit) {
	    // it is the exit of successful execve system call, another 
	    // SIGTRAP notification will be delivered to the process
	    // If we free it right now, it will be killed by that SIGTRAP
	    // The free operation is defered to code dealing with the 
	    // SIGTRAP events. 
	    pInfo->delayedDetach = true ;
	    return 0 ;
	}
	if (ptrace(PTRACE_DETACH, pid, 0, 0) < 0) {
	    err_out("LinuxPtraceX86::detachProc ", strerror(errno)) ;
	    return -1 ; 
	}
	
	notify_parent(pInfo) ;
	procTable.remove(pid) ;
    }
    return 0 ;
}

int LinuxPtraceX86::terminateProc(pid_t pid) 
{
    if (ptrace(PTRACE_KILL, pid, 0, 0) < 0) {
	err_out("LinuxPtraceX86:killProc ", strerror(errno)) ;
	return -1 ; 
    }
    
    ProcInfo *pInfo = procTable.lookUp(pid) ;
    if (pInfo != 0) {
	notify_parent(pInfo) ;
	procTable.remove(pid) ;
    }
    return 0 ;
}

int LinuxPtraceX86::notify_parent(ProcInfo *pInfo) 
{
    int retval = 0 ;
    if (pInfo->parent) {
	pInfo->parent->children-- ;
	if (pInfo->parent->waiting) {
	    pid_t wpid = pInfo->parent->waitpid ;
	    if (wpid < 0 || wpid == pInfo->pid) {
		pInfo->parent->waiting = false ;
		if (ptrace(PTRACE_SYSCALL, pInfo->parent->pid, 0, 0) < 0) {
		    err_out("LinuxPtraceX86: Error in notifying parent!", "") ;
		    retval = -1 ;
		}
	    }
	}
    }
    return retval ;
}
/*
 * In this function, the program waits for the next system call
 * and finds the corresponding control block. 
 */
int LinuxPtraceX86::waitForCall(pid_t *newpid, SystemCall *call)
{
    int status ;
    int retval = NEXTCALL_NORMAL ;

    ProcInfo *pInfo = procTable.lookUp(*newpid) ;
    if (0 != pInfo) {
	/* need to handle something left in the previous event */
	if (pInfo->exiting) {
	    detachProc(pInfo->pid) ;
	    prev_pid = 0 ;
	    return NEXTCALL_EXITED ;
	} 
	if (pInfo->waiting) {
	    goto wait_begin ;
	}
	if (ptrace(PTRACE_SYSCALL, pInfo->pid, 0, 0) < 0) {
	    err_out("LinuxPtraceX86::waitForCall", 
		    "Cannot setup tracing for next event!") ;
	    return NEXTCALL_ERROR ;
	}
    }

    wait_begin:
    pid_t pid = wait4(-1, &status, __WALL, 0) ;
    if (pid == -1) {
	switch (errno) {
	case EINTR:
	    goto wait_begin ;
	default:
	    err_out("LinuxPtraceX86::waitForCall", strerror(errno)) ;
	    return NEXTCALL_ERROR ;
	}
    }
    
    *newpid = pid ;

    /* First look the process information in procTable */
    pInfo = procTable.lookUp(pid) ;
    if (0 == pInfo) {
	/* after a process is killed by ptrace, the notification of 
	   exec may still come */
	if (WIFEXITED(status) || WIFSIGNALED(status) || getCallNum(pid) == 11)
	    goto wait_begin ;
	err_out("LinuxPtraceX86::waitForCall ", "Unknown process") ;
	return NEXTCALL_ERROR ;
    }

#ifdef UI_DEBUG_MODE
    logfile << "PID: " << pid ;
    logfile << " Signal: " << WSTOPSIG(status) ;
    logfile << " System Call: " << getCallNum(pid) ;
    logfile << endl ;
#endif
    
    /* this process is waiting for its children, we don't need this
       notification */
    if (pInfo->waiting) {
	goto wait_begin ;
    }


    /* We need to filter out the system call events and notify the caller */
    /* The traced process received a signal and terminated */
    if (WIFSIGNALED(status)) {
	notify_parent(pInfo) ;
	procTable.remove(pid) ;
	prev_pid = 0 ;
	return NEXTCALL_EXITED ;
    }

    /* The traced process exited */
    if (WIFEXITED(status)) {
	if (pInfo->parent) {
	    pInfo->parent->children-- ;
	}
	procTable.remove(pid) ;
	prev_pid = 0 ;
	return NEXTCALL_EXITED ;
    }

    /* If this is the first system call of a process, we need to check 
       whether it is stopped by the loop instruction. If so, it must be
       resumed. */

    /* 
       After the first process called PTRACE_TRACEME, the exec system call will
       be notified to the main program by SIGTRAP, all other newly created 
       processes will be notified as SIGSTOP event.
    */

    if (pInfo->firstCall && (WSTOPSIG(status) == SIGTRAP ||
			     WSTOPSIG(status) == SIGSTOP)) {
	pInfo->firstCall = false ;
	if (pInfo->stopped) {
	    pInfo->stopped = false ;
	    if (resumeProcess(pid, pInfo) < 0) {
		err_out("LinuxPtraceX86::waitForCall ", 
			"Cannot resume stopped process!") ;
		return NEXTCALL_ERROR ;
	    }
	}
	
	/* This is either the exit of a fork or the notification from
	   execve. The manager need not to be notified. */ 
	if (ptrace(PTRACE_SYSCALL, pid, 0, 0) < 0) {
	    err_out("LinuxPtraceX86::waitForCall ",
		    "Cannot continue after the first notification!") ;
	    return NEXTCALL_ERROR ;
	}
	goto wait_begin ;
    }
    

    /* This is a normal signal, deliver it to the process */

    if (WSTOPSIG(status) != SIGTRAP) {
	/* deliver this signal back to the process */
	int signal = WSTOPSIG(status) ;

	if (ptrace(PTRACE_SYSCALL, pid, 0, signal) < 0) {
	    err_out("LinuxPtraceX86::waitForCall ", 
		    "ptrace error in passing signal!") ;
	    return NEXTCALL_ERROR ;
	}
	pInfo->waiting = false ;
	goto wait_begin ;
    }
    
    /* This is a system call event */


    if (pInfo->isEntry) {
	pInfo->scno = getCallNum(pid) ;
	if (errno != 0) {
	    err_out("LinuxPtraceX86::waitForCall ", 
		    "Error in Getting Call Number") ;
	    return NEXTCALL_ERROR ;
	}
    }
    int scno = pInfo->scno ;
    
    if (pInfo->omit) {
	pInfo->omit = false ;
	if (pInfo->delayedDetach) 
	    detachProc(pid) ;
	else if (ptrace(PTRACE_SYSCALL, pid, 0, 0) < 0) {
	    err_out("LinuxPtraceX86::waitForCall", "ptrace error!") ;
	    return NEXTCALL_ERROR ;
	}
	goto wait_begin ;
    }


    if (pInfo->abortCall) {
	pInfo->abortCall = false ;
	int retval = setCallNum(pid, pInfo->storedCall) ;
	if (retval < 0) {
	    err_out("LinuxPtraceX86::waitForCall ", 
		    "Error in restore call number!") ;
	    return NEXTCALL_ERROR ;
	}
	retval = setReturnVal(pid, pInfo->newRC, pInfo->newErrno) ;
	if (retval < 0) {
	    err_out("LinuxPtraceX86::waitForCall ",
		    "Error in setting new return value!") ;
	    return NEXTCALL_ERROR ;
	}
    }

    pInfo->resetCallInfo() ;

    /* The entry point of a system call */
    if (pInfo->isEntry) {
	pInfo->isEntry = false ;
	call->isEntry(true) ;
	switch (scno) {
	case __NR_kill:{
	    pid_t arg0 = getReg(pid, 0) ;
	    ProcInfo *killpInfo ;
	    if (arg0 == getpid())  // prevent sending signal to monitor
		setReg(pid, 1, 0) ; // set the signal to 0
	    else if ((killpInfo = procTable.lookUp(pid)) != 0) {
		if (killpInfo->stopped)
		    setReg(pid, 1, 0) ; // prevent sending signal to new proc
	    }
	    break ;
	}
	case __NR_ptrace:{
	    pid_t arg1 = getReg(pid, 1) ;
	    if (arg1 == getpid())
		setReg(pid, 1, 1)  ; // init may not be traced, 
	                             // caller will get error
	    break ;
	}
	case __NR_vfork: {
	    //replace vfork to fork
	    long newcall = __NR_fork ;
	    int retval = setCallNum(pid, newcall) ;
	    if (retval < 0) {
		err_out("LinuxPtraceX86::waitForCall ", 
			"Error in replace vfork!") ;
		return NEXTCALL_ERROR ;
	    }
	} 
	case __NR_fork:
	case __NR_clone: 
	    if (stopProcess(pid, pInfo) < 0) {
		err_out("LinuxPtraceX86::waitForCall ", 
			"Cannot stop process!") ;
		return NEXTCALL_ERROR ;
	    }
	    pInfo->stopped = true ;
	    break;
	case __NR_wait4:
	case __NR_waitpid:{
	    pid_t arg0 = getReg(pid, 0) ;
	    int arg2 = getReg(pid, 2) ;
	    if (!(arg2 & WNOHANG) && pInfo->children > 0) {
		/* There are traced children */
		pInfo->waiting = true ;
		pInfo->waitpid = arg0 ;
	    }
	    break;
	}
	case __NR_exit:
#ifdef __NR_exit_group
	case __NR_exit_group:
#endif
	    pInfo->exiting = true ;
	    break;
	case __NR_execve:
	    retval = NEXTCALL_EXECED;
	    break;
	default: 
	    break ;
	}

	/* fill in the system call information */
	/* initialize the unified system call numbers */
	pInfo->uscno = unifyCall(scno, 0) ;
	call->uscno(pInfo->uscno) ;
	call->scno(pInfo->scno) ;
	call->setArgRep(pInfo->arg) ;
	pInfo->clearArgRep() ;
	switch (scno) {
	case __NR_mmap:
	case __NR_select: {
	    long ptr ;
	    ptr = getReg(pid, 0) ;
	    for (int i = 0 ; i < 6 ; i++) 
		pInfo->setArgRep(i, false, ptr+i*4, pid) ;
	    break ;
	}
	case __NR_ipc: {
	    int subcall ;
	    subcall = getReg(pid, 0) ;
	    pInfo->uscno = unifyCall(scno, subcall) ;
	    call->uscno(pInfo->uscno) ;
	    for (int i = 0 ; i < 5 ; i++)
		pInfo->setArgRep(i, true, i+1, pid) ;
	    break ;
	}
	case __NR_socketcall: {
	    int subcall ;
	    subcall= getReg(pid, 0) ;
	    pInfo->uscno = unifyCall(scno, subcall) ;
	    call->uscno(pInfo->uscno) ;
	    long ptr = getReg(pid, 1) ;
	    for (int i = 0 ; i < 6 ; i++) 
		pInfo->setArgRep(i, false, ptr+i*4, pid) ;
	    break ;
	}
	default:
	    for (int i = 0 ; i < 6 ; i++)
		pInfo->setArgRep(i, true, i, pid) ;
	    break ;
	}
    }
    /* The exit point of a system call */
    else {
	pInfo->isEntry = true ;
	call->uscno(pInfo->uscno) ;
	call->scno(pInfo->scno) ;
	call->setArgRep(pInfo->arg) ;
	call->isEntry(false) ;
	switch (scno){
	case __NR_fork:
	case __NR_vfork:
	case __NR_clone: {
	    /* resume the parent process */
	    bool stopped = pInfo->stopped ;
	    if (stopped) {
		pInfo->stopped = false ;
		if (resumeProcess(pid, pInfo) < 0) {
		    err_out("LinuxPtraceX86::waitForCall ", 
			    "Cannot resume process!") ;
		    return NEXTCALL_ERROR ;
		}
	    }

	    pid_t childpid = getReg(pid, EAX);
	    if (childpid < 0) 
		break ;

	    /* Attach to the new child */
	    if (attachProc(childpid) < 0) {
		char msg[1024];
		snprintf(msg, 1024, "Error in attaching to child %d, %s", childpid, strerror(errno));
		err_out("LinuxPtraceX86::waitForCall ", 
			msg) ;
		kill(childpid, SIGKILL) ;
		return NEXTCALL_ERROR ;
	    }
	    
	    ProcInfo *childInfo = procTable.lookUp(childpid) ;
	    if (0 == childInfo) {
		kill(childpid, SIGKILL) ;
		err_out("LinuxPtraceX86::waitForCall ", 
			"Cannot find control record!") ;
		return NEXTCALL_ERROR ;
	    }
	    if (stopped) {
		childInfo->stopped = true ;
		childInfo->storedPC = pInfo->storedPC ;
		childInfo->storedInst = pInfo->storedInst ;
	    }

	    childInfo->parent = pInfo ;
	    pInfo->children++ ;

	    retval = NEXTCALL_CREATED ;
	    break;
	}
	case __NR_execve:
	    /* 
	     * If the execve call succeeds, we will receive an additional
	     * entry of execve, which is the exit point of execve call in
	     * the new process. So we need to mark it as a new one, so that
	     * we can omit this entry.	   
	     */
	    int errnum ;
	    if (getReturnVal(pid, &errnum) == 0)
		pInfo->omit = true ;
	    retval = NEXTCALL_EXECED;
	    break; 
	}
    }
    
    pInfo->prevCall = scno ;
    prev_pid = pid ;
    return retval ;
}

int LinuxPtraceX86::abortCall(pid_t pid, int rc, int errnum) 
{
    struct ProcInfo *pInfo ;
    pInfo = procTable.lookUp(pid) ;
    if (0 == pInfo) {
	return -1 ;
    }
    
    /* since this flag is toggled at the begin of entry or exit section
       if its entry, the value should be false */
    if (pInfo->isEntry) return -1 ;

    long scno = getCallNum(pid) ;

    if (scno == __NR_exit)
	return -1 ;

#ifdef __NR_exit_group
    if (scno == __NR_exit_group)
	return -1 ;
#endif
    int retval = setCallNum(pid, 0) ;
    if (retval < 0) 
	return -1 ;

    // can only abort call once
    if (!pInfo->abortCall) {
	pInfo->abortCall = true ;
	pInfo->storedCall = scno ;
	pInfo->newRC = rc ;
	pInfo->newErrno = errnum ;
    }
    return 0 ;

}

int LinuxPtraceX86::stopProcess(pid_t pid, ProcInfo *pInfo) 
{
    
    long pc = getReg(pid, EIP) ;
    if (errno != 0) {
	err_out("LinuxPtraceX86::stopProcess ", "Cannot get Program Counter!");
	return -1 ;
    }
    
    pInfo->storedInst = ptrace(PTRACE_PEEKTEXT, pid, pc, 0) ;
    if (errno != 0) {
	err_out("LinuxPtraceX86:stopProcess ", 
		"Cannot read current instruction!") ;
	return -1 ;
    }

    long loop = 0x0000feeb ;
    int retval ;
    retval = ptrace(PTRACE_POKETEXT, pid, pc, loop) ;
    if (retval < 0) {
	err_out("LinuxPtraceX86::stopProcess ", 
		"Cannot insert loop instruction!") ;
	return -1 ;
    }

    pInfo->storedPC = pc ;
    
    return 0 ;
}

int LinuxPtraceX86::resumeProcess(pid_t pid, ProcInfo *pInfo)
{
    int retval ;
    retval = ptrace(PTRACE_POKETEXT, pid, pInfo->storedPC, pInfo->storedInst) ;
    if (retval < 0) {
	err_out("LinuxPtraceX86::resumeProcess ",
		"Cannot restore instruction!") ;
	return -1 ;
    }
    return retval ;
}

int LinuxPtraceX86::getExecPath(pid_t pid, char *path, size_t size) 
{
    char infofile[PATH_MAX] ;
    snprintf(infofile, PATH_MAX, "/proc/%d/exe", pid) ;

    int retval = readlink(infofile, path, size) ;
    if (retval < 0)
	path[0] = 0 ;
    else
	path[retval] = 0 ;

    return retval ;
}

int LinuxPtraceX86::getWorkingDir(pid_t pid, char *path, size_t size)
{
    char infofile[PATH_MAX] ;
    snprintf(infofile, PATH_MAX, "/proc/%d/cwd", pid) ;

    int retval = readlink(infofile, path, size) ;
    if (retval < 0) 
	path[0] = 0 ;
    else 
	path[retval] = 0 ;

    return retval ;
} 

int LinuxPtraceX86::unifyCall(int call, int subcall) 
{
    if (__NR_socketcall == call)
	return 260 + subcall ;
    if (__NR_ipc == call)
	return 300 + subcall ;
    return call ;
}

unsigned long LinuxPtraceX86::getIP(pid_t pid) 
{
    return getReg(pid, EIP);
}

SysCallInfo *LinuxPtraceX86::scInfo(int scno)
{
    if (scno < MAXSYSCALLNUM)
	return &scinfo[scno] ;
    else 
	return 0 ;
} 

void LinuxPtraceX86::initScInfo()
{
    for (int i = 0; i < MAXSYSCALLNUM; i++) {
	char temp[20];
	snprintf(temp, 20, "SysCall #%d", i);
	scinfo[i].name = temp;
	scinfo[i].basePtrOffset = -1;
	for (int j = 0; j < 6; j++) {
	    scinfo[i].argType[j] = 0 ;
	    scinfo[i].filledByCall[j] = false;
	}
    }

#include "SysCallInfo.h"

}
