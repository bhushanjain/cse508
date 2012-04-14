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

#ifndef __PATH_MAPPING_H__
#define __PATH_MAPPING_H__
#include "PathHashTable.h"
#include <vector>

class PathMapping {
    private:
    PathHashTable ht ;
    bool noGUI;
    long seqnumber ;
    char cachedir[PATH_MAX] ;
    vector<pid_t> parent, children ;

    vector<char *> reffile ;
    vector<time_t> access_time ;
    
    public:
    PathMapping();
    ~PathMapping();

    int addMapping(int filetype, int maptype, const char *oldpath, 
		   const char *newpath) ;
    int findMapping(const char *oldpath, char *newpath, size_t size) ;
    int deleteMapping(const char *oldpath) ;
    
    void setCacheDir(const char *cdir) {
	strncpy(cachedir, cdir, PATH_MAX) ;
    }

    int appendTime(const char *oldpath, pid_t pid, pid_t ppid) ;
    void recordRelation(pid_t pid, pid_t ppid) {
	parent.push_back(ppid) ;
	children.push_back(pid) ;
    }
    
    void record_time(const char *path) ;
    bool hasChanges(const char *path) ;
    void getChanges(const char *path, DirChange *ptr) {
	ht.getChanges(path, ptr) ;
    }
    
    bool is_update(const char *oldpath);
	bool is_delete(const char *oldpath);

    bool sendtoGUI(bool) ;
} ;

#endif
