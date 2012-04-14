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

#include "ProcData.h"
#include "PathMapping.h"
#include "PathHashTable.h"
#include "GUIInterface.h"
#include "DirTools.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

static char *typeCode[] = {"", "F", "D"} ;   
static char *chgCode[] = {"", "CR", "MD", "DE"} ;
#ifdef INSTALL_SHIELD
extern IDInfo<char> processCommand ;
extern IDInfo<char> procCmdSHA ;
#endif

PathMapping::PathMapping() { 
    seqnumber = 100000000;
//    noGUI = false;
    // First connect to GUI, if it fails, use terminal to display information
//    if (connectgui(8500) < 0) {
#ifdef INSTALL_SHIELD
//	cerr << "No GUI server is running. Quitting ..." << endl;
//	exit(1);
#endif
	noGUI = true ;
//    }
} 

bool PathMapping::sendtoGUI(bool afterexec)
{
    int count = 0;
    bool commit = false ;
    PathHashMapI ptr ;
    ht.resetPosition() ;
    while ( ht.getNextItem(&ptr) != 0 ) {
	if (noGUI) {
	    switch(ptr->second->filetype) {
	    case TYPE_FILE:
		cout << "File " ;
		break ;
	    case TYPE_DIRECTORY:
		cout << "Directory " ;
		break ;
	    }
	    switch(ptr->second->maptype) {
	    case PATH_CREATED:
		cout << ptr->first << " Created-> " << ptr->second->newpath  ;
		break ;
	    case PATH_MODIFIED:
		cout << ptr->first << " Mapped-> " << ptr->second->newpath ;
		break ;
	    case PATH_DELETED:
		cout << ptr->first << " Deleted"  ;
		break ;
	    }
	    cout << endl ;
#ifdef INSTALL_SHIELD_OLD
	    for (unsigned int i = 0 ; i < ptr->second->timestamp.size() ; i++){
		pid_t pid = ptr->second->pid[i] ;
		cout << " by process " << pid ;
		char *cmd = processCommand.lookUp(pid) ;
		if (0 != cmd) 
		    cout << " " << cmd ;
		cout << " at time " ;
		cout << ptr->second->timestamp[i] ;
		cout << " seq " << ptr->second->sequence[i] << endl ;
	    }
#endif
	}
	else { // send to GUI the mapping database entries. 
#ifdef INSTALL_SHIELD_OLD
	    char buf[1024];
	    snprintf(buf, 1024, "MAPTO: %s %s %s %s\n", ptr->first, 
		     typeCode[ptr->second->filetype],
		     chgCode[ptr->second->maptype], ptr->second->newpath);
	    sendfile(buf, strlen(buf)+1);
#endif 
#ifndef INSTALL_SHIELD
	    char *pvtdata = 0;
	    char timestr[64] ;
	    char na[] = "N/A";
	    
	    strncpy(timestr, "0", 2) ;

	    switch(ptr->second->maptype) {
	    case PATH_CREATED:
		pvtdata = ptr->second->newpath ;
		break ;
	    case PATH_MODIFIED:
		pvtdata = ptr->second->newpath ;
		break ;
	    case PATH_DELETED:
		pvtdata = na ;
		break ;
	    }
	    sendfile(ptr->first, chgCode[ptr->second->maptype],
		     pvtdata, timestr, typeCode[ptr->second->filetype]) ;
#endif

	}
	count++ ;
    }
#ifdef INSTALL_SHIELD_OLD
    if (!noGUI) {
	    sendfile("PID", "", "", "", "", "", "", "", "") ;
	    
	    for (unsigned int i = 0 ; i < parent.size() ; i++) {
		char na[] = "N/A" ;
		char multina[] = "N/A 0 N/A N/A" ;
		char *sha = procCmdSHA.lookUp(children[i]) ;
		if (0 == sha)
		    sha = na ;

		char pidstr[64], ppidstr[64] ;
		/* get the executable file and full command line */
		char *cmd = processCommand.lookUp(children[i]) ;
		if (0 == cmd)
		    cmd = multina ;

		snprintf(ppidstr, 64, "%d", parent[i]) ;
		snprintf(pidstr, 64, "%d", children[i]) ;
		/* send to the UI "pid ppid sha1sum executable command" */
		sendfile(pidstr, ppidstr, sha, cmd, "", "", "", "", "") ;
	    } 
    }
#endif
    if (!noGUI)
        finish_sending() ;

    if (afterexec) {
	if (noGUI) {
	    if (count > 0) { // there are changes made to the file system
		do {
		    char ch ;
		    cout << "Would you like to commit these changes?(y/n) " ;
		    cin >> ch ;
		    if ('y' == ch || 'Y' == ch) {
			commit = true ;
			break ;
		    }
		    if ('n' == ch || 'N' == ch) {
			break ;
		    }
		} while (1) ;
	    }
	}
	else {
	    if (getResponse() == 1)
		commit = true ;
	}
    } 
    /* check committing criteria */
	/*Bhushan: This is how to traverse the changed files. Also see sendfile() function to ask script for indivifual files.*/
    if (commit) {
	for (unsigned int i = 0 ; i < reffile.size() ; i++) {
	    if (commit && (strncmp(reffile[i], "/proc", 5) != 0) && (strncmp(reffile[i], "/dev/tty", 8) != 0)) {
		struct stat statbuf ;
		if (stat(reffile[i], &statbuf) < 0) {
		    cout << "Cannot check the status of " << reffile[i] <<endl;
		    commit = false ;
		}
		else {
		    if (statbuf.st_mtime >= access_time[i]) {
			cout << reffile[i] << " modified!" << endl ;
			commit = false ;
		    }
		}
	    }
	    free(reffile[i]) ;
	}	    
    }
    return commit ;
}

PathMapping::~PathMapping() 
{
    PathHashMapI ptr ;
    char command[300] ;
    bool commit ;
    commit = sendtoGUI(true) ;
   
    if (commit)
	cout << "Committing changes ..." << endl ;
    else 
	cout << "Discarding changes ..." << endl; 

    ht.resetPosition() ;
    while ( ht.getNextItem(&ptr) != 0 ) {
	if (commit) {
	    switch(ptr->second->maptype) {
	    case PATH_CREATED:
	    case PATH_MODIFIED:
			if(is_update(ptr->first))
			{
				snprintf(command, 300,"cp -f %s %s", ptr->second->newpath, 
				ptr->first) ;
				system(command) ;
			}
			snprintf(command, 300,"wipe -c -f -M a -P 5 -r -S r -s %s", ptr->second->newpath) ;
			system(command) ;
			break ;
	    case PATH_DELETED:
			if(is_delete(ptr->first))
			{
				snprintf(command, 300, "rm -rf %s", ptr->first) ;
				system(command) ;
			}
			break ;
	    }
	} else {
	    // delete that entry
//	    char command[300] ;
	    switch(ptr->second->maptype) {
	    case PATH_CREATED:
	    case PATH_MODIFIED:
		snprintf(command, 300,"wipe -c -f -M a -P 5 -r -S r -s %s", ptr->second->newpath) ; 
		system(command) ;
		break ;
	    }
	} 
    }

    if (!noGUI) {
        if (commit)
	    sendOK() ;
        else 
	    sendNOK() ;
    }
    ht.Cleanup() ;
    snprintf(command, 300, "wipe -c -f -M a -P 5 -r -S r -s %s", cachedir) ;
    system(command) ;
}

/*
 * Insert an entry to the mapping, the paths must be absolute path
 */
int PathMapping::addMapping(int filetype, int maptype, const char *oldpath, 
			    const char *newpath) 
{
    map_node *theNode ;
    
    //if (oldpath[0] != '/' || newpath[0] != '/') return -1 ;
    
    theNode = new map_node ;
    //theNode = (map_node*)malloc(sizeof(map_node)) ;
    if (NULL == theNode) return -1 ;

    theNode->filetype = filetype ;
    theNode->maptype = maptype ;

    if (PATH_MODIFIED == maptype || PATH_CREATED == maptype) {
	strncpy(theNode->newpath, newpath, PATH_MAX) ;
    }
    
    size_t len = strlen(oldpath) ;
    char *o_path = new char[len+1] ;
    strncpy(o_path, oldpath, len+1) ;

    if (ht.insert(o_path, theNode) == NULL)
	return -1 ;
    
    return 0 ; 
}

/*
 * first check whether the oldpath is mapped or deleted, 
 * then check whether the path is deleted
 * Assumption: If we mapped a directory to a tempory location, the files and 
 * directories in that directory must be mapped to a new location, too.
 */
int PathMapping::findMapping(const char *oldpath, char *newpath, size_t size) 
{
    map_node *theNode ;

    if (oldpath[0] != '/') return -1 ;
    theNode = ht.lookUp(oldpath) ;
    if (NULL != theNode) {
	switch (theNode->maptype) {
	case PATH_CREATED:
	case PATH_MODIFIED: 
	    strncpy(newpath, theNode->newpath, size) ;
	    break ;
	case PATH_DELETED:
	    break ;
	}
	
	return theNode->maptype ;
    }	

    // now check whether the path is deleted or mapped to a new one 
    char tmppath[PATH_MAX] ;
    strncpy(tmppath, oldpath, PATH_MAX) ;
	
    while(1) {
	int position = strlen(tmppath) - 1 ;
	while (tmppath[position] != '/' && position > 0) position --;
	if (position == 0) break ;
	tmppath[position] = 0 ;
	theNode = ht.lookUp(tmppath) ;
	if (NULL == theNode) continue ;
	switch (theNode->maptype) {
	case PATH_MODIFIED: // should not have mapped, since we don't map dir
	case PATH_CREATED:{
	    int position = strlen(tmppath) ;
	    strncpy(newpath, theNode->newpath, size) ;
	    strncat(newpath, oldpath+position, size-strlen(newpath)) ;
	    return PATH_CREATED ; // if a file is not directly mapped, it's created
	}
	case PATH_DELETED:
	    return PATH_DELETED ;
	
	}
    }
    
    return PATH_NEW ; // the path is not mapped or deleted 
}

int PathMapping::deleteMapping(const char *oldpath) 
{
    return ht.remove(oldpath) ;
}

int PathMapping::appendTime(const char *oldpath, pid_t pid, pid_t ppid) 
{
    map_node *theNode ;

    theNode = ht.lookUp(oldpath) ;
    if (0 != theNode) {
	theNode->pid.push_back(pid) ;
	theNode->ppid.push_back(ppid) ;
	theNode->timestamp.push_back(time(0)) ;
	theNode->sequence.push_back(seqnumber++) ;
    }	
    
    return 0 ;
}

void PathMapping::record_time(const char *path) 
{
    char *tmp = strdup(path) ;
    reffile.push_back(tmp) ;
    access_time.push_back(time(0)) ;
}
    
bool PathMapping::hasChanges(const char *path) 
{
    return ht.hasChanges(path) ;
}
bool PathMapping::is_update(const char *oldpath)
{
	do 
	{
	    char ch ;
	    cout << "Would you like to update/create file "<<oldpath <<"?(y/n) " ;
	    cin >> ch ;
	    if ('y' == ch || 'Y' == ch) {
		return true ;
	    }
	    if ('n' == ch || 'N' == ch) {
		return false ;
	    }
	} while (1) ;
}

bool PathMapping::is_delete(const char *oldpath)
{
	do 
	{
	    char ch ;
	    cout << "Would you like to delete file "<<oldpath <<"?(y/n) " ;
	    cin >> ch ;
	    if ('y' == ch || 'Y' == ch) {
		return true ;
	    }
	    if ('n' == ch || 'N' == ch) {
		return false ;
	    }
	} while (1) ;	
}
