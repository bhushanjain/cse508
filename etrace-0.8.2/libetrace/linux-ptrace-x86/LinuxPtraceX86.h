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

#ifndef __LINUX_PTRACE_X86_H
#define __LINUX_PTRACE_X86_H

#include <ArchDependent.h>
#include <ProcHashTable.h>
#include <ArgumentRep.h>

#include <string>
#if __GNUC__ >= 3 
#include <ext/hash_map>
using __gnu_cxx::hash_map  ;
#else
#include <hash_map>
#endif
#include <fstream>
#include <dlfcn.h>
using namespace std ;

const int GETDATA_NO_BUFFER = 0 ;
const int GETDATA_START_BUFFER = 1 ;
const int GETDATA_USE_BUFFER = 2 ;

class ProcInfo {
    unsigned int exe_beg_, exe_end_, stk_end_, libc_beg_, libc_end_, ld_beg_, ld_end_;
    int follow_depth_;
    unsigned int previousPC_;
    string syscallName_;

    public:
    bool isEntry ;
    bool suspended ;
    bool stopped ;
    bool abortCall ;
    bool firstCall ;
    bool omit ;
    bool waiting ;
    bool exiting ;
    bool delayedDetach ;
    int flags ;
    long storedPC ;
    long storedInst ;
    long storedCall ;
    int newRC ;
    int newErrno ;
    long newStackPtr ;
    long scno ;
    long uscno ;
    long prevCall ;
    ProcInfo *parent ;
    int children ;
    pid_t waitpid ;
    pid_t pid ;
    struct SysCallArgRep arg[6] ;

    //fields for buffered getData
    long start_addr ;
    size_t buffer_size ;
    char *buffer ;

    ProcInfo() {
	isEntry = true ;
	suspended = false ;
	stopped = false ;
	abortCall = false ;
	firstCall = true ;
	omit = false ;
	waiting = false ;
	delayedDetach = false ;
	parent = 0 ;
	children = 0 ;
	exiting = false ;
	newStackPtr = 0 ;
	exe_beg_ = exe_end_ = stk_end_ = 0 ;
	start_addr = 0 ;
	buffer_size = 0 ;
	buffer = 0 ;
	for (int i = 0 ; i < 6 ; i++)
	    arg[i].buf_ = 0 ;
    }
    ~ProcInfo() {
	clearArgRep() ;
	if (buffer != 0)
	    delete [] buffer ;
    }

    void resetCallInfo() {
	newStackPtr = 0 ;
	if (buffer != 0) 
	    delete [] buffer ;
	buffer = 0 ;
	start_addr = 0 ;
	buffer_size = 0 ;
    }

    unsigned int exe_beg() const { return exe_beg_; }
    unsigned int exe_end() const { return exe_end_; }
    unsigned int libc_beg() const { return libc_beg_; }
    unsigned int libc_end() const { return libc_end_; }
    unsigned int ld_beg() const { return ld_beg_; }
    unsigned int ld_end() const { return ld_end_; }
    unsigned int stk_end() const { return stk_end_; }

    void exe_beg(unsigned int addr) { exe_beg_ = addr; }
    void exe_end(unsigned int addr) { exe_end_ = addr; }
    void libc_beg(unsigned int addr) { libc_beg_ = addr; }
    void libc_end(unsigned int addr) { libc_end_ = addr; }
    void ld_beg(unsigned int addr) { ld_beg_ = addr; }
    void ld_end(unsigned int addr) { ld_end_ = addr; }
    void stk_end(unsigned int addr) { stk_end_ = addr; }
    void previousPC(unsigned int PC_) { previousPC_ = PC_; }
    unsigned int previousPC() { return previousPC_; }

    void syscallName(string &name) { syscallName_ = name ; }
    string  syscallName() { return syscallName_ ; }    

    void setArgRep(int number, bool isreg, long addr, pid_t pid) {
	if ((number < 0) || (number >= 6)) return ;
	arg[number].ISreg_ = isreg ;
	arg[number].addr_ = addr ;
	arg[number].pid_ = pid;
    }
    /* 
     * Clear the state flags of syscall args, the flag is used to 
     * indicate whether this is a new alloc, or a cleanup during the
     * entry of a system call. 
     */
    void clearArgRep(){
	for (int i = 0 ; i < 6 ; i++){
	    arg[i].inited_ = false ;
	    if (arg[i].buf_) {
		delete [] arg[i].buf_ ;
	    }
	    arg[i].buf_ = 0 ;
	}
    }

} ;

typedef ProcHashTable<ProcInfo> ProcStates ;

const int MAXSYSCALLNUM = 350 ;

class LinuxPtraceX86 : public ArchDep
{
    ProcStates procTable ;

    pid_t prev_pid ;
    
    SysCallInfo scinfo[MAXSYSCALLNUM] ;
    int stopProcess(pid_t pid, ProcInfo *pInfo) ;
    int resumeProcess(pid_t pid, ProcInfo *pInfo) ;
    
    int internal_detach(pid_t pid, int signal) ;
    int notify_parent(ProcInfo *pInfo) ;
    
    int unifyCall(int scno, int subcall) ;
    int checkcall(unsigned& returnaddr, pid_t pid, const char* scname, 
		  int *indirect_call) ;
    void initScInfo() ;

public:
    inline void *openDynamicLibrary(const char *lib) {
	return dlopen(lib, RTLD_NOW);
    }
    inline void *getDLFunction(void *handle, char *symbol) {
	return dlsym(handle, symbol);
    }
    inline const char *getDLError() {
	return dlerror();
    }
    inline int closeDynamicLibrary(void *handle) {
	return dlclose(handle);
    }

    LinuxPtraceX86() ;
    ~LinuxPtraceX86() ;
    long getReg(pid_t pid, int reg) ;
    int  setReg(pid_t pid, int reg, long val) ;
    
    inline int getData(pid_t pid, long addr, char* buf, size_t len){
	return getData(pid, addr, buf, len, GETDATA_NO_BUFFER, 0);
    }
    int getData(pid_t pid, long addr, char* buf, size_t len, int buffermode, 
		size_t size) ;
    int setData(pid_t pid, long addr, const char *buf, size_t len) ;

    long allocMem(pid_t pid, size_t size) ;

    long getCallNum(pid_t pid) ;
    int  setCallNum(pid_t pid, long scno)  ;
    
    int getReturnVal(pid_t pid, int *errnum) ;
    int setReturnVal(pid_t pid, int rc, int errnum)  ;

    int getWorkingDir(pid_t pid, char *path, size_t size) ;
    int getExecPath(pid_t pid, char *path, size_t size) ;

    int getUID(pid_t pid) ; 
    int getEUID(pid_t pid) ;
    int getGID(pid_t pid)  ;
    int getEGID(pid_t pid)  ;
    pid_t getPPID(pid_t pid) ;
    
    unsigned long getIP(pid_t pid) ;
    unsigned long getIPOffset(pid_t pid) ;
    SysCallInfo *scInfo(int scno) ;
 
    pid_t startTracing(const char *path, char **argv) ; 
    int attachProc(pid_t pid) ;
    int detachProc(pid_t pid) ;
    int terminateProc(pid_t pid) ;
    int waitForCall(pid_t *newpid, SystemCall *call) ;
    
    int abortCall(pid_t pid, int rc, int errnum) ;
} ;

const int NOARG_TYPE = 0 ;
const int CADDR_T_TYPE = 1 ;
const int CAP_USER_DATA_T_TYPE = 2 ;
const int CAP_USER_HEADER_T_TYPE = 3 ;
const int CSTRING_TYPE = 4 ;
const int CHAR_P_P_TYPE = 5 ;
const int DEV_T_TYPE = 6 ;
const int FD_SET_P_TYPE = 7 ;
const int GID_T_TYPE = 8 ;
const int GID_T_P_TYPE = 9 ;
const int GID_T_ARRAY_TYPE = 10 ;
const int INT_TYPE = 11 ;
const int FUNC_P_TYPE = 12 ;
const int INT_P_TYPE = 13 ;
const int INT_ARRAY_TYPE = 14 ;
const int LOFF_T_P_TYPE = 15 ;
const int MODE_T_TYPE = 16 ;
const int OFF_T_TYPE = 17 ;
const int PID_T_TYPE = 18 ;
const int QID_T_TYPE = 19 ;
const int SIGHANDLER_T_TYPE = 20 ;
const int SIGSET_T_P_TYPE = 22 ;
const int SIZE_T_TYPE = 23 ;
const int SIZE_T_P_TYPE = 24 ; 
const int STACK_T_P_TYPE = 25 ;
const int SYSCTL_ARGS_P_TYPE = 26 ;
const int DIRENT_P_TYPE = 27 ;
const int FLOCK_P_TYPE = 28 ;
const int IOVEC_P_TYPE = 29 ;
const int ITIMERVAL_P_TYPE = 30 ;
const int KERNEL_SYM_P_TYPE = 31 ;
const int MODULE_P_TYPE = 32 ;
const int NFSCTL_ARG_P_TYPE = 33 ;
const int POLLFD_P_TYPE = 34 ;
const int RLIMIT_P_TYPE = 35 ;
const int RUSAGE_P_TYPE = 36 ;
const int SCHED_PARAM_P_TYPE = 37 ;
const int SIGACTION_P_TYPE = 38 ;
const int STAT_P_TYPE = 39 ;
const int STATFS_P_TYPE = 40 ;
const int SYSINFO_P_TYPE = 41 ;
const int TIMESPEC_P_TYPE = 42 ;
const int TIMEVAL_P_TYPE = 43 ;
const int TIMEX_P_TYPE = 44 ;
const int TIMEZONE_P_TYPE = 45 ;
const int TMS_P_TYPE = 46 ;
const int USTAT_P_TYPE = 47 ;
const int UTIMBUF_P_TYPE = 48 ;
const int UTSNAME_P_TYPE = 49 ;
const int VM86PLUS_STRUCT_P_TYPE = 50 ;
const int TIME_T_P_TYPE = 51 ;
const int UID_T_TYPE = 52 ;
const int UID_T_P_TYPE = 53 ;
const int NFSCTL_RES_P_TYPE = 54 ; // Union type 
const int UNSIGNED_INT_TYPE = 55 ;
const int LONG_TYPE = 56 ;
const int UNSIGNED_LONG_TYPE = 57 ;
const int VOID_P_TYPE = 58 ;
const int SOCKADDR_P_TYPE = 59 ;
const int SOCKLEN_T_TYPE = 60 ;
const int SOCKLEN_T_P_TYPE = 61 ;
const int MSGHDR_P_TYPE = 62 ;
const int OFF_T_P_TYPE = 63 ;
const int SEMBUF_P_TYPE = 64 ;
const int KEY_T_TYPE = 65 ;
const int MSGBUF_P_TYPE = 66 ;
const int MSQID_DS_P_TYPE = 67 ;
const int SHMID_DS_P_TYPE = 68 ;

const int MAX_TYPES = 69 ;

#endif
 
