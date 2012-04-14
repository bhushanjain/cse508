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

#ifndef __ALCATRAZ_H__
#define __ALCATRAZ_H__
#ifndef DEBUG_MODE
#define DEBUG_MODE
#endif
#include "ProcData.h"
#include <Extension.h>
#include <string>
const int LOC_NORMAL = 0 ;
const int LOC_INCACHE = 1 ;
const int LOC_INTEMP = 2 ;

class Alcatraz: public Extension
{
    static bool initialized ;
    char tempname[PATH_MAX] ;
    int state ;

    void init() ;
    
    // Since the changes made in isolation cannot be seen by the system,
    // we cannot rely on the system to provide the current working directory. 
    // We have to track it by ourselves. 
    // If Alcatrazed processes chroot, its view of the file system will
    // be different from the monitor's. We use rootdir to keep track of
    // its new root, so that the normalized path is meaningful to the monitor
    // These two fields need to passed down to children

    string workingdir ;
    string rootdir ;

    void normalizePath(const char *orig, char *newpath) ;
    bool inCache(const char *path);
    int translatePath(const char *path, char *newpath, bool followlink,
		      char *lastpath=0);

    int simple_mapping(int arg, bool followlink=true) ;
    bool parent_writable(const char *file) ;

    void chdir_entry() ;
    void chdir_exit() ;
    void fchdir_exit() ;
    void fork_entry() ;
    void fork_exit() ;
    void getdents_entry() ;
    void getdents_exit() ;
    void getdents64_entry() ;
    void getdents64_exit() ;
    void getcwd_entry() ;
    void getcwd_exit() ;

    void chroot_entry() ;

    void open_entry() ;
    void open_exit() ;
    
    void execve_entry() ;
    void execve_exit() ;
    void utime_entry() ;
    void access_entry() ;
    void readlink_entry() ;
    void statfs_entry() ;
    void stat_entry() ;
    void lstat_entry() ;

    void chmod_entry() ;
    void lchown_entry() ;
    void chown_entry() ;

    int  truncate_failure(const char *file) ;
    void truncate_entry() ;
    
    int  creat_failure(const char *file) ;
    void creat_entry() ;
    int  mknod_failure(const char *file) ;
    void mknod_entry() ;
    int  mkdir_failure(const char *file) ;
    void mkdir_entry() ;
    int  rmdir_failure(const char *file) ;
    void rmdir_entry() ;
    int  unlink_failure(const char *file) ;
    void unlink_entry() ;
    void unlink_exit() ;
    int  rename_failure(const char *file0, const char *file1) ;
    int  rename_failure0(const char *file) ;
    int  rename_failure1(const char *file) ;
    void rename_entry() ;
    int  symlink_failure(const char *file) ;
    void symlink_entry() ;
    int  link_failure(const char *file0, const char *file1) ;
    int  link_failure0(const char *file) ;
    int  link_failure1(const char *file) ;
    void link_entry() ;

    void socket_entry() ;
    void socket_exit() ;

    void read_entry() ;

#ifdef INSTALL_SHIELD
    void write_entry() ;
#endif

    public:
    Alcatraz():Extension() {
	if (!initialized) {
	    initialized = true ;
	    init() ;
	}
    }

    Alcatraz(const Alcatraz& al):Extension() {
	workingdir = al.workingdir ;
	rootdir = al.rootdir ;
    }
    
    ~Alcatraz() {}

    int deliverEvent() ;

    Alcatraz *clone() {
	Alcatraz *tmp = new Alcatraz(*this) ;
	return tmp ;
    }
    
} ;

#endif
