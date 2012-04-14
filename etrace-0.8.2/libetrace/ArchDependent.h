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

#ifndef __ARCH_DEP_H
#define __ARCH_DEP_H

#include "SystemCall.h"
#include "String.h"
#include <sys/types.h>

const int NEXTCALL_ERROR = -1;
const int NEXTCALL_NORMAL = 0;
const int NEXTCALL_EXITED = 1;
const int NEXTCALL_CREATED = 2;
const int NEXTCALL_EXECED = 3;
/*
 * Information for each system call, specifying the type of each argument
 * and whether it is an argument provided by the caller or it is filled by 
 * the system call
 */
struct SysCallInfo {
    String name;
    int numArgs; // records max arg number we are interested in
    int basePtrOffset;
    int flag; 
    int argType[6] ;
    bool filledByCall[6] ; // filledByCall[i]==1 <==> ith arg is a return arg
} ;

class ArchDep 
{
    public:
    
    virtual ~ArchDep() {} 
    virtual void *openDynamicLibrary(const char *lib)=0;
    virtual void *getDLFunction(void *handle, char *symbol)=0;
    virtual const char *getDLError()=0;
    virtual int closeDynamicLibrary(void *handle)=0;

    /* access the register */
    virtual long getReg(pid_t pid, int reg)=0;
    virtual int  setReg(pid_t pid, int reg, long val)=0;

    /* access the memory */
    virtual int getData(pid_t pid, long addr, char* buf, size_t len)=0;
    virtual int setData(pid_t pid, long addr, const char *buf, size_t len)=0;

    virtual long allocMem(pid_t pid, size_t size)=0;

    /* access the system call number */
    virtual long getCallNum(pid_t pid)=0;
    virtual int  setCallNum(pid_t pid, long scno)=0;

    /* access the return value */
    virtual int getReturnVal(pid_t pid, int *errnum)=0;
    virtual int setReturnVal(pid_t pid, int rc, int errnum)=0;

    /* get the current working directory */
    virtual int getWorkingDir(pid_t pid, char *path, size_t size)=0;
    /* get the executable name */
    virtual int getExecPath(pid_t pid, char *path, size_t size)=0;

    virtual int getUID(pid_t pid)=0; 
    virtual int getEUID(pid_t pid)=0;
    virtual int getGID(pid_t pid)=0;
    virtual int getEGID(pid_t pid)=0;
    virtual pid_t getPPID(pid_t pid)=0;
    
    virtual unsigned long getIP(pid_t pid)=0;
    virtual SysCallInfo *scInfo(int scno)=0;

    virtual pid_t startTracing(const char *path, char **argv)=0; 
    virtual int attachProc(pid_t pid)=0;
    virtual int detachProc(pid_t pid)=0;
    virtual int terminateProc(pid_t pid)=0;
    virtual int waitForCall(pid_t *newpid, SystemCall *call)=0;
    virtual int abortCall(pid_t pid, int rc, int errnum)=0;

} ;

#endif
