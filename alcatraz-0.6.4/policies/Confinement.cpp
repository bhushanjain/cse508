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

#include "Confinement.h"
#include <ArchDependent.h>
#include <SysCallNum.h>
#include <MonitoredProc.h>
#include <linux/capability.h>
#include <LinuxTypes.h>
#include <stdio.h>

extern "C" {
    Extension* initConfinement() {
	return new Confinement();
    }
};

void Confinement::priviledged()
{
    abort(EPERM);
}

void Confinement::invalid()
{
    abort(EPERM);
}

void Confinement::ipccalls()
{
    abort(EPERM);
}

/* 
 * Restrict access to devices on the system. The application cannot
 * arbitrarily access any device on the system
 * allowed devices: /dev/null, /dev/tty, /dev/urandom, /dev/dsp, /dev/mixer
 */
void Confinement::checkdevaccess()
{
    CString arg0(getArgRep(0)) ;
    if (strncmp(arg0.get().c_str(), "/dev", 4) == 0) {
	if (strcmp(arg0.get().c_str(), "/dev/null") != 0 &&
	    strcmp(arg0.get().c_str(), "/dev/tty") != 0 &&
	    strcmp(arg0.get().c_str(), "/dev/urandom") != 0 &&
	    strcmp(arg0.get().c_str(), "/dev/dsp") != 0 &&
	    strcmp(arg0.get().c_str(), "/dev/mixer") != 0)

	    abort(EPERM);
    }
}

int Confinement::deliverEvent() 
{
    SystemCall *call = mp->getCurCall() ;
    
    switch (call->uscno()) {
	// priviledged operations
    case SYSCALL_acct:
    case SYSCALL_reboot:
    case SYSCALL_idle:
    case SYSCALL_vm86:
    case SYSCALL_vm86old:
    case SYSCALL_nfsservctl:
    case SYSCALL_pivot_root:
    case SYSCALL_create_module:
    case SYSCALL_delete_module:
    case SYSCALL_init_module:
    case SYSCALL_swapoff:
    case SYSCALL_swapon:
    case SYSCALL_bdflush:
    case SYSCALL_ioperm:
    case SYSCALL_iopl:
    case SYSCALL_mlockall:
    case SYSCALL_munlockall:
    case SYSCALL_mount:
    case SYSCALL_umount:
    case SYSCALL_umount2:
    case SYSCALL_quotactl:
	if (call->isEntry())
	    priviledged(); 
	break; 
	// invalid operations
    case SYSCALL_profil:
    case SYSCALL_afs_syscall:
    case SYSCALL_getpmsg:
    case SYSCALL_putpmsg:
    case SYSCALL_uselib:
    case SYSCALL_readdir:
    case SYSCALL_ulimit:
    case SYSCALL_break:
	if (call->isEntry())
	    invalid();
	break;
	// restrict IPC calls
    case SYSCALL_msgctl:
    case SYSCALL_msgget:
    case SYSCALL_msgrcv:
    case SYSCALL_msgsnd:
    case SYSCALL_semctl:
    case SYSCALL_semget:
    case SYSCALL_semop:
//    case SYSCALL_shmat:
//    case SYSCALL_shmctl:
//    case SYSCALL_shmdt:
//    case SYSCALL_shmget:
	if (call->isEntry())
	    ipccalls(); 
	break; 
	// restrict device access
    case SYSCALL_open:
	if (call->isEntry())
	    checkdevaccess(); 
	break; 
	// process interference
    case SYSCALL_ptrace:
//	if (call->isEntry())
//	    abort(EPERM);
	break; 
    case SYSCALL_kill:
	if (call->isEntry()) {
	    Int arg0(getArgRep(0)); 
	    if (!procInSession(arg0.get()))
		abort(EPERM);
	}
	break; 
	// network operation
    case SYSCALL_socket:
	if (call->isEntry()) {
	    Int arg0(getArgRep(0));
//		printf("\nrequest to create socket of  type %d\n",arg0);
//	    if (arg0.get() != PF_LOCAL)
//		abort(EPERM);
	}
	break; 
    case SYSCALL_bind:
	if (call->isEntry()) {
	    Sockaddr_unP addr(getArgRep(1));
		Int arg0(getArgRep(0));
//		printf("\nrequest to bind fd %d to path %s\n",arg0.get(),addr.get_path());
	}
//	if (call->isEntry())
//	    abort(EPERM); 
	break ;
    case SYSCALL_connect:
	if (call->isEntry()) {
	    Sockaddr_unP addr(getArgRep(1));
		Int arg0(getArgRep(0));
	    // only allow local X connection to display :2
	    // need to change to a dynamically decided display number
//		printf("\nrequest to connect fd %d to path %s\n",arg0.get(),addr.get_path());
//	    if (strcmp(addr.get_path(), "/tmp/.X11-unix/X2") != 0)
//		{
//			printf("\nCannot connect to path %s\n",addr.get_path());
//			abort(EPERM);
//		}
	}
    default:
	break; 
    }

    return 0; 
}

