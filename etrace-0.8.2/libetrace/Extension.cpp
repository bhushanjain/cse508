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

#include "ArchDependent.h"
#include "Extension.h"
#include "MonitoredProc.h"

#include <stdio.h>
#include <limits.h>

void Extension::setMonitoredProc(MonitoredProc *MP) 
{ 
    mp = MP ;
    SysCallArg::setArch(MP->getArch());
}

int Extension::abort(int err) {
    ArchDep *arch = mp->getArch() ;
    return arch->abortCall(mp->pid(), -1, err) ;
}

int Extension::fake(int rc) {
    ArchDep *arch = mp->getArch() ;
    return arch->abortCall(mp->pid(), rc, 0) ;
} 

SysCallArgRep *Extension::getArgRep(int i)
{
    SystemCall *call = mp->getCurCall() ;
    return call->getArgRep(i) ;
}

String Extension::programName() {
    char *name = mp->programName() ;
    return String(name); 
}

String Extension::getCwd() {
    char buf[PATH_MAX] ;
    ArchDep *arch = mp->getArch() ;
    
    arch->getWorkingDir(mp->pid(), buf, PATH_MAX) ;
    return String(buf);
}
    
int Extension::getPID() {
    return mp->pid() ;
}

int Extension::getPPID()
{
    ArchDep *arch = mp->getArch() ;
    return arch->getPPID(mp->pid()) ;
}

int Extension::isChild(int pid)
{
    ArchDep *arch = mp->getArch() ;

    pid_t ppid = arch->getPPID(pid) ;
    
    if (mp->pid() == ppid) 
	return 1 ;

    return 0 ;
}

int Extension::getUID()
{
    ArchDep *arch = mp->getArch() ;

    return arch->getUID(mp->pid()) ;
}

int Extension::getEUID()
{
    ArchDep *arch = mp->getArch() ;

    return arch->getEUID(mp->pid()) ;
}

int Extension::getGID() {
  return 0;
}
int Extension::getEGID() {
  return 0;
} 

int Extension::getREUID() {
  return 0;
}
  
int 
Extension::realRC() {
    ArchDep *arch = mp->getArch() ;
    int err ;
    int rc = arch->getReturnVal(mp->pid(), &err) ;
    return rc ;
}

void 
Extension::logMessageStr(String fmt, String s) {
  fprintf(stderr, fmt.c_str(), s.c_str());
} 
 
void Extension::logMessageInt(String fmt, int i) {
  fprintf(stderr, fmt.c_str(), i);
}

String Extension::RealPath(String path) {
  return path;
}

int Extension::checkPath(String s) {
  return 0;
}
int Extension::writeFile(int mode) {
  return (mode & 0200 || mode & 0020  || mode & 0002); 
}

int Extension::readFile(int mode) {
  return (mode & 0400 || mode & 0040  || mode & 0004); 
}

int Extension::isInFile(String, String) {
  return 0;
}
int Extension::sameFile (String , String) {
  return 0;
}
int Extension::inTree(String dir,String file) {
  return 0;
}

int Extension::isPrefix(String, String) {
  return 0;
}

int Extension::isSuffix(String, String) {
  return 0;
}

int Extension::isSymLink(String) {
  return 0;
}

int Extension::getCurrentSysCall() {
  SystemCall *call = mp->getCurCall() ;
  return call->scno() ;
}

void Extension::getEvent(int *scno, int *type) {
    SystemCall *call = mp->getCurCall() ;
    
    *scno = call->scno()  ;
    if (call->isEntry())
	*type = EVENT_ENTRY ;
    else 
	*type = EVENT_EXIT ;
    return;
}

int Extension::deliverEvent() {
    return EXTENSION_NORMAL ;
}

void Extension::switchExtension(const char *libname, const char *classname) 
{
    mp->switchExt(true) ;
    mp->lib(libname) ;
    mp->monClass(classname) ;
}

void Extension::terminate()
{
    raise(SIGINT); 
}

bool Extension::procInSession(pid_t pid)
{
    return mp->processInSession(pid); 
}

