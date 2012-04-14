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

#include "MonitoredProc.h"
#include "ManagerClass.h"

MonitoredProc::~MonitoredProc(){
    // delete all extensions
    cleanUpExtensions() ;
    // we don't need to kill the process if it terminates normally
    if (!exited_)
	getArch()->terminateProc(pid_);
}

ArchDep *MonitoredProc::getArch() const 
{ 
    return managerObj_->getArch() ; 
} 


void MonitoredProc::insertExtension(Extension *pExtension) 
{ 
    Extension *cur = (Extension*)headExtension.NextNodeLeft() ;
    
    pExtension->setMonitoredProc(this);
    while (true) {
	if (cur == &headExtension)
	    break ;
	if (pExtension->priority() < cur->priority()) {
	    cur = (Extension*)cur->NextNodeLeft() ;
	} else 
	    break ;
    }

    cur->InsertRight(pExtension) ;
}

void MonitoredProc::cleanUpExtensions() 
{
    Extension *pExtension ;
    while ( (pExtension=(Extension*)headExtension.NextNodeRight()) != &headExtension ) {
	pExtension->DeleteNode() ;
	delete pExtension ;
    }
}

void MonitoredProc::replaceExtension(Extension *oldExt, Extension *newExt) 
{
    Extension *left = (Extension*)oldExt->NextNodeLeft() ;
    
    oldExt->DeleteNode() ;
    delete oldExt ;
    left->InsertRight(newExt) ;
}

void MonitoredProc::resetExtensionList() 
{ 
    currentExtension = &headExtension ; 
}

Extension *MonitoredProc::getNextExtension() 
{ 
    Extension *next ;
    
    next = (Extension*)currentExtension->NextNodeRight() ;
    if (next==&headExtension) return 0 ;
    
    currentExtension = next ;
    return next ;
}

Extension *MonitoredProc::getPrevExtension() 
{
    Extension *prev ;
    
    prev = (Extension*)currentExtension->NextNodeLeft() ;
    if (prev==&headExtension) return 0 ;
    
    currentExtension = prev ;
    return prev ;
}

pid_t MonitoredProc::ppid() 
{
    return getArch()->getPPID(pid_) ;
}

int MonitoredProc::abort(long rc)
{
    int ret, err ;
    if (rc < 0) {
	ret = -1 ;
	err = -rc ;
    }
    else {
	ret = rc ;
	err = 0 ;
    }
	
    return getArch()->abortCall(pid_, ret, err) ;
}

long MonitoredProc::currentRC()
{
    int err ;
    int ret = getArch()->getReturnVal(pid_, &err) ;
    if (ret == -1)
	ret = -err ;
    return ret ;
}

bool MonitoredProc::processInSession(pid_t pid)
{
    return managerObj_->processInSession(pid); 
}
