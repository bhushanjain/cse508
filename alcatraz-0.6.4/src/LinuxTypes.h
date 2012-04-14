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

#ifndef __LINUX_TYPES_H__
#define __LINUX_TYPES_H__
#include <unistd.h>
#include <ArgumentType.h>
#include <sys/types.h>

typedef BasicType<caddr_t> Caddr_t ;

#include <linux/capability.h>
typedef BasicType<cap_user_data_t> Cap_user_data_t ;
typedef BasicType<cap_user_header_t> Cap_user_header_t ;

// char * corresponds to CString, char ** corresponds to ArgV

typedef BasicType<dev_t> Dev_t ;
typedef FixSizedPtr<fd_set> Fd_setP ;
typedef BasicType<gid_t> Gid_t ;
typedef FixSizedPtr<gid_t> Gid_tP ;
typedef Array<gid_t> Gid_tA ;

typedef BasicType<int> Int ;
typedef Int Integer ;
typedef FixSizedPtr<int> IntP ;
typedef Array<int> IntA ;

typedef FixSizedPtr<loff_t> Loff_tP ;
typedef BasicType<mode_t> Mode_t ;
typedef BasicType<off_t> Off_t ;
typedef BasicType<pid_t> Pid_t ;

#include <signal.h>
typedef FixSizedPtr<sigset_t> Sigset_tP ;
typedef BasicType<size_t> Size_t ;
typedef FixSizedPtr<size_t> Size_tP ;

typedef FixSizedPtr<stack_t> Stack_t ;
typedef FixSizedPtr<struct __sysctl_args> Sysctl_argsP_ ;
class Sysctl_argsP : public Sysctl_argsP_ {
    public:
    Sysctl_argsP(SysCallArgRep *repr):Sysctl_argsP_(repr) {}
    Sysctl_argsP(PersistArg *pst): Sysctl_argsP_(pst) {}
    Sysctl_argsP(SysCallArg *pst) : Sysctl_argsP_(pst) {}
};

#include <dirent.h>
typedef FixSizedPtr<struct dirent> DirentP_ ;
class DirentP : public DirentP_ {
    public:
    DirentP(SysCallArgRep *repr):DirentP_(repr) {}
    DirentP(PersistArg *pst): DirentP_(pst) {}
    DirentP(SysCallArg *pst) : DirentP_(pst) {}
    long get_dino() { return get()->d_ino ; }
    char *get_dname() { return get()->d_name ; }
} ;

#include <sys/file.h>
typedef FixSizedPtr<struct flock> FlockP ;
#include <sys/uio.h>
typedef FixSizedPtr<struct iovec> IovecP ;
#include <sys/time.h>
typedef FixSizedPtr<struct itimerval> ItimervalP_ ;
class ItimervalP : public ItimervalP_ {
    public:
    ItimervalP(SysCallArgRep *repr):ItimervalP_(repr) {}
    ItimervalP(PersistArg *pst): ItimervalP_(pst) {}
    ItimervalP(SysCallArg *pst) : ItimervalP_(pst) {}
    long get_it_valsec() { return get()->it_value.tv_sec ;}
    long get_it_valusec() { return get()->it_value.tv_usec ;}
} ;

typedef FixSizedPtr<struct kernel_sym> Kernel_symP ;
typedef FixSizedPtr<struct module> ModuleP ;
#include <sys/poll.h>
typedef FixSizedPtr<struct pollfd> PollfdP ;
#include <sys/resource.h>
typedef FixSizedPtr<struct rlimit> RlimitP_ ;
class RlimitP : public RlimitP_ {
    public:
    RlimitP(SysCallArgRep *repr):RlimitP_(repr) {}
    RlimitP(PersistArg *pst): RlimitP_(pst) {}
    RlimitP(SysCallArg *pst) : RlimitP_(pst) {}
    rlim_t get_rlim_cur() { return get()->rlim_cur ;}
    rlim_t get_rlim_max() { return get()->rlim_max ;}
};

typedef FixSizedPtr<struct rusage> RusageP_ ;
class RusageP : public RusageP_ {
    public:
    RusageP(SysCallArgRep *repr):RusageP_(repr) {}
    RusageP(PersistArg *pst): RusageP_(pst) {}
    RusageP(SysCallArg *pst) : RusageP_(pst) {}
    long get_ru_maxrss() { return get()->ru_maxrss ;}
    long get_ru_ixrss() { return get()->ru_ixrss ;}
    long get_ru_idrss() { return get()->ru_idrss ;}
    long get_ru_isrss() { return get()->ru_isrss ;}
};

#include <sched.h>
typedef FixSizedPtr<struct sched_param> Sched_paramP_ ;
class Sched_paramP : public Sched_paramP_ {
    public:
    Sched_paramP(SysCallArgRep *repr):Sched_paramP_(repr) {}
    Sched_paramP(PersistArg *pst): Sched_paramP_(pst) {}
    Sched_paramP(SysCallArg *pst) : Sched_paramP_(pst) {}
    int get_priority() { return get()->sched_priority ;}
} ;

typedef FixSizedPtr<struct sigaction> SigactionP ;
typedef FixSizedPtr<struct stat> StatP_ ;
class StatP : public StatP_ {
    public:
    StatP(SysCallArgRep *repr):StatP_(repr) {}
    StatP(PersistArg *pst): StatP_(pst) {}
    StatP(SysCallArg *pst) : StatP_(pst) {}
    ino_t get_ino() { return get()->st_ino ; }
    mode_t get_mode() { return get()->st_mode ; }
    uid_t get_uid() { return get()->st_uid ; }
    gid_t get_gid() { return get()->st_gid ; }
    
} ;

#include <sys/vfs.h>
typedef FixSizedPtr<struct statfs> StatfsP_ ;
class StatfsP : public StatfsP_ {
    public:
    StatfsP(SysCallArgRep *repr):StatfsP_(repr) {}
    StatfsP(PersistArg *pst): StatfsP_(pst) {}
    StatfsP(SysCallArg *pst) : StatfsP_(pst) {}
    long get_fs_type() { return get()->f_type ; }
} ;

typedef FixSizedPtr<struct sysinfo> SysinfoP ;
typedef FixSizedPtr<struct timespec> TimespecP_ ;
class TimespecP : public TimespecP_ {
    public:
    TimespecP(SysCallArgRep *repr):TimespecP_(repr) {}
    TimespecP(PersistArg *pst): TimespecP_(pst) {}
    TimespecP(SysCallArg *pst) : TimespecP_(pst) {}
    time_t get_sec() { return get()->tv_sec ; }
    long get_nsec() { return get()->tv_nsec ; }
} ;

typedef FixSizedPtr<struct timeval> TimevalP_ ;
class TimevalP : public TimevalP_ {
    public:
    TimevalP(SysCallArgRep *repr):TimevalP_(repr) {}
    TimevalP(PersistArg *pst): TimevalP_(pst) {}
    TimevalP(SysCallArg *pst) : TimevalP_(pst) {}
    long get_usec() { return get()->tv_usec ;}
    long get_sec() { return get()->tv_sec ;}
};
#include <sys/timex.h>
typedef FixSizedPtr<struct timex> TimexP ;
typedef FixSizedPtr<struct timezone> TimezoneP_ ;
class TimezoneP : public TimezoneP_ {
    public:
    TimezoneP(SysCallArgRep *repr):TimezoneP_(repr) {}
    TimezoneP(PersistArg *pst): TimezoneP_(pst) {}
    TimezoneP(SysCallArg *pst) : TimezoneP_(pst) {}
    int get_tz_dsttime() { return get()->tz_dsttime ; }
} ;

#include <sys/times.h>
typedef FixSizedPtr<struct tms> TmsP_ ;
class TmsP : public TmsP_ {
    public:
    TmsP(SysCallArgRep *repr):TmsP_(repr) {}
    TmsP(PersistArg *pst): TmsP_(pst) {}
    TmsP(SysCallArg *pst) : TmsP_(pst) {}

    int get_tms_stime() { return get()->tms_stime ;}
    int get_tms_utime();
    int get_tms_cstime();
    int get_tms_cutime();
};

typedef FixSizedPtr<struct ustat> UstatP ;
#include <utime.h>
typedef FixSizedPtr<struct utimbuf> UtimbufP_ ;
class UtimbufP : public UtimbufP_ {
    public: 
    UtimbufP(SysCallArgRep *repr):UtimbufP_(repr) {}
    UtimbufP(PersistArg *pst): UtimbufP_(pst) {}
    UtimbufP(SysCallArg *pst) : UtimbufP_(pst) {}

    time_t get_modtime() { return get()->modtime ;}
    time_t get_actime() { return get()->actime ;}
} ;
#include <sys/utsname.h>
typedef FixSizedPtr<struct utsname> UtsnameP ;
#include <sys/vm86.h>
typedef FixSizedPtr<struct vm86plus_struct> Vm86plus_structP ;

typedef FixSizedPtr<time_t> Time_tP ;
typedef BasicType<uid_t> Uid_t ;
typedef FixSizedPtr<uid_t> Uid_tP ;
typedef BasicType<unsigned int> UInt ;
typedef BasicType<unsigned long> ULong ;

typedef Array<char> VoidP ;
typedef VoidP Ptr ;
#include <sys/socket.h> 
typedef Size_t Socklen_t ;
typedef Size_tP Socklen_tP ;
typedef FixSizedPtr<struct msghdr> MsghdrP ;

#include <netinet/in.h>
#include <arpa/inet.h>
typedef FixSizedPtr<struct sockaddr_in> Sockaddr_inP_ ;
class Sockaddr_inP : public Sockaddr_inP_ {
    public:
    Sockaddr_inP(SysCallArgRep *repr):Sockaddr_inP_(repr) {}
    Sockaddr_inP(PersistArg *pst): Sockaddr_inP_(pst) {}
    Sockaddr_inP(SysCallArg *pst) : Sockaddr_inP_(pst) {}
    int family() { 
	struct sockaddr_in *sa = get() ;
	if (0 != sa)
	    return sa->sin_family ; 
	return 0 ;
    }
    unsigned short get_port() {
	struct sockaddr_in *sa = get() ;
	if (0 != sa)
	    return ntohs(sa->sin_port) ;
	return 0 ;
    }
    char *get_ip_address() {
	struct sockaddr_in *sa = get() ;
	if (0 != sa)
	    return inet_ntoa(sa->sin_addr) ;
	return 0 ;
    } 
} ;
typedef Sockaddr_inP SockaddrP ;
#include <sys/un.h>
typedef FixSizedPtr<struct sockaddr_un> Sockaddr_unP_ ;
class Sockaddr_unP : public Sockaddr_unP_ {
    public:
    Sockaddr_unP(SysCallArgRep *repr):Sockaddr_unP_(repr) {}
    Sockaddr_unP(PersistArg *pst): Sockaddr_unP_(pst) {}
    Sockaddr_unP(SysCallArg *pst) : Sockaddr_unP_(pst) {}
    char *get_path() {
	struct sockaddr_un *sa = get() ;
	if (0 == sa)
		return 0 ;
	return sa->sun_path ;
    } 
    void set_path(const char *path) {
	struct sockaddr_un sun ;
	struct sockaddr_un *sa = get() ;
	if (0 == sa)
		return ;
	memcpy(&sun, sa, sizeof(sun)) ;
	strncpy(sun.sun_path, path, PATH_MAX) ;
	set(&sun) ;
    } 
} ;

#include <sys/sem.h>
typedef FixSizedPtr<struct sembuf> SembufP ;
typedef BasicType<key_t> Key_t ;

#include <sys/msg.h>
typedef FixSizedPtr<struct msgbuf> MsgbufP ;
typedef FixSizedPtr<struct msqid_ds> Msqid_dsP ;
#include <sys/shm.h>
typedef FixSizedPtr<struct shmid_ds> Shmid_dsP ;


#endif
