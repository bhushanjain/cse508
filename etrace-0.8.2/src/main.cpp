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

#include <ManagerClass.h>
#include "LinuxPtraceX86.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>

using namespace std ;

ManagerClass *manager ;
ArchDep *arch ; 

const int SIG_MAX = 64 ;
void cleanup(int sig) 
{
    cout << "Signal " << sig << " received! Quiting... " << endl ;
    if (0 != manager) {
	delete manager ;
	manager = 0;
    }
    if (0 != arch) {
	delete arch ;
	arch = 0;
    }
    exit(1) ;
}

int main(int argc, char **argv)
{
    char *mapfile = 0 ;
    pid_t pid = 0 ;
    bool switch_on_exec = false ;

    if (argc < 4) {
	cerr << "Usage: " << argv[0] ;
	cerr << " -f[s] <mapping file> [ -p <pid> | <command> ]" << endl ;
	exit(1);
    }

    arch = new LinuxPtraceX86() ;
    if (0 == arch) {
	cerr << "Cannot allocate ArchDep Object!" << endl ; 
	exit(1) ;
    }

    if (strncmp(argv[1], "-f", 2) != 0) {
	cerr << "Usage: " << argv[0] ;
	cerr << " -f[s] <mapping file> [ -p <pid> | <command> ]" << endl ;
	exit(1);
    }
    
    if (strncmp(argv[1], "-fs", 4) == 0)
	switch_on_exec = true ;

    mapfile = argv[2] ;

    if (strncmp(argv[3], "-p", 3) == 0) {
	if (argc < 5) {
	    cerr << "Usage: " << argv[0] ;
	    cerr << " -f <mapping file> [ -p <pid> | <command> ]" << endl ;
	    exit(1);
	}
        pid = atoi(argv[4]) ;
	argv += 4 ;
    }
    else {
	argv += 3 ;
    }
    
    if (0 == mapfile) {
	cerr << "No mapping file!" << endl ;
	cerr << "Usage: " << argv[0] ;
	cerr << " -f <mapping file> [ -p <pid> | <command> ]" << endl ;
	exit(1) ;
    }

    manager = new ManagerClass(arch, mapfile);
    if (0 == manager) {
	cerr << "Cannot allocate ManagerClass!" << endl ;
	delete arch ;
	exit(1) ;
    }

    /* set up signal handler */
    for (int i = 0 ; i <= SIG_MAX ; i++) {
	switch (i) {
	case SIGCHLD:
	case SIGPROF:
	case SIGSTOP:
	case SIGTSTP:
	case SIGCONT:
	    break ;
	default:
	    if (signal(i, cleanup) < 0)
		cerr << "Error in setting up signal handler for signal " 
		     << i << "!" << endl ;
	    break ;
	}
    }	
   
    manager->switchOnExec(switch_on_exec) ;

    if (pid == 0)
	manager->startTracing(argv) ;
    else 
	manager->startTracing(pid) ;

    if (0 != manager) {
	delete manager ;
	manager = 0 ;
    }
    
    if (0 != arch) {
	delete arch ;
	arch = 0 ;
    }
    return 0 ;
}
