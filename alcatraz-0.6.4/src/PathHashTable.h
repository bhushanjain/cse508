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

#ifndef __PATH_HASH_TABLE_H__
#define __PATH_HASH_TABLE_H__
#include "CommonDefs.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <linux/limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#if __GNUC__ >= 3 
#include <ext/hash_map>

using __gnu_cxx::hash_map  ;
using __gnu_cxx::hash ;
using namespace std ;
#else
#include <hash_map>
#endif


typedef struct {
    int filetype ;
    int maptype ;
    char newpath[PATH_MAX] ;
    vector<pid_t> pid ;
    vector<pid_t> ppid ;
    vector<time_t> timestamp ;
    vector<long> sequence ;
} map_node ;

class DirChange {
    vector<char*> add, del ;
    size_t addPosition ;
    public: 
    DirChange() { addPosition = 0 ; }
    ~DirChange() {
	size_t i ;
	for (i = 0 ; i < add.size() ; i++)
	    delete[] add[i] ;
	for (i = 0 ; i < del.size() ; i++)
	    delete[] del[i] ;
    }
    void created(const char *file) {
	size_t len = strlen(file) + 1 ;
	char *tmp = new char[len] ;
	if (0 == tmp) return ;
	strncpy(tmp, file, len) ;
	add.push_back(tmp) ;
    }
    void deleted(const char *file) {
	size_t len = strlen(file) + 1 ;
	char *tmp = new char[len] ;
	if (0 == tmp) return ;
	strncpy(tmp, file, len) ;
	del.push_back(tmp) ;
    }
    bool isDeleted(const char *file) {
	size_t i ;
	for (i=0 ; i < del.size() ; i++)
	    if (!strcmp(file, del[i])) return true ;
	return false ;
    }
    char *getNextNew() {
	if (addPosition >= add.size()) 
	    return 0 ;
	addPosition ++ ;
	return add[addPosition - 1] ;
    } 
} ;

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};
    
typedef hash_map<const char*, map_node*, hash<const char*>, eqstr> PathHashMap;
typedef hash_map<const char*, map_node*, hash<const char*>, eqstr>::iterator PathHashMapI;
typedef hash_map<const char*, map_node*, hash<const char*>, eqstr>::const_iterator PathHashMapCI;

class PathHashTable
{
    private:
    PathHashMap tbl_;
    PathHashMapI nextItem ; 

    public:
    PathHashTable() {};
    PathHashTable(const PathHashTable& ht): tbl_(ht.tbl_) {};

    const map_node* lookUp(const char* path) const { 
	PathHashMapCI dci = tbl_.find(path);
	if (dci != tbl_.end()) {
	    //cout << "found" << endl ;
	    return dci->second ;
	}
	else
	    return NULL;
    };

    map_node* lookUp(const char* path) { 
	return (map_node*)(((const PathHashTable*)this)->lookUp(path));
    };

    map_node* insert(const char* oldpath, map_node* node) { 
      if (tbl_.count(oldpath) != 0) {
	  //cout << "duplicate entry" << endl ;
	    return NULL;
      }
	else {
	    tbl_[oldpath] = node;
	    return node;
	};
    };

    int remove(const char* path) {
	if (tbl_.count(path) != 0) {
	    map_node *rv = tbl_[path];
	    tbl_.erase(path);
	    //free(rv) ;
	    delete rv ;
	    return 0;
	}
	else return -1;
    };

    void Cleanup() {
	PathHashMapI ptr ;
	int i, size = tbl_.size() ;
	for (ptr=tbl_.begin(), i=0; i < size; i++, ptr++ ) {
	    
	    delete ptr->second ;
	    //char *path = (char*)ptr->first ;
	    //tbl_.erase(path) ;
	    //delete[] path ;
	}
    };

    bool hasChanges(const char *path) {
	PathHashMapI ptr ;
	char buf[PATH_MAX] ;
	for (ptr = tbl_.begin();  ptr != tbl_.end() ; ptr++){
	    strncpy(buf, ptr->first, PATH_MAX) ;
	    if (strcmp(path, dirname(buf)) == 0) 
		return true ;
	}
	return false ;
    }
     
    void getChanges(const char *path, DirChange *pchg) {
	PathHashMapI ptr ;
	char buf[PATH_MAX] ;
	for (ptr = tbl_.begin();  ptr != tbl_.end() ; ptr++){
	    strncpy(buf, ptr->first, PATH_MAX) ;
	    if (strcmp(path, dirname(buf)) == 0) {
		strncpy(buf, ptr->first, PATH_MAX) ;
		switch (ptr->second->maptype) {
		case PATH_DELETED:
		    pchg->deleted(basename(buf)) ;
		    break ;
		case PATH_CREATED:
		    pchg->created(basename(buf)) ;
		    break ;
		}
	    }
	}
	
    }

    void resetPosition() {
	nextItem = tbl_.begin() ;
    }
    
    int getNextItem(PathHashMapI *ptr) {
	if (tbl_.end() == nextItem)
	    return 0 ;
	*ptr = nextItem ;
	nextItem ++ ;
	return 1 ;
    }
	
};
#endif
