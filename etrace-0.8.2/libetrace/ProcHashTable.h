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

#ifndef __PROC_HASH_TABLE_H
#define __PROC_HASH_TABLE_H

#if __GNUC__ >= 3 
#include <ext/hash_map>
using __gnu_cxx::hash_map  ;
#else
#include <hash_map>
#endif

template <class T>
class ProcHashTable
{
    private:
    hash_map<pid_t, T*> tbl_;

    public:
    ProcHashTable() {};
    ProcHashTable(const ProcHashTable& ht): tbl_(ht.tbl_) {};

    const T* lookUp(pid_t id) const { 
	typename hash_map<pid_t, T*>::const_iterator dci = tbl_.find(id);
	if (dci != tbl_.end())
	    return dci->second ;
	else
	    return NULL;
    };

    T* lookUp(pid_t id) { 
	return (T*)(((const ProcHashTable*)this)->lookUp(id));
    };

    T* insert(pid_t id, T* data) { 
	if (tbl_.count(id) != 0)
	    return NULL;
	else {
	    tbl_[id] = data;
	    return data;
	};
    };

    bool remove(pid_t id) {
	if (tbl_.count(id) != 0) {
	    T *rv = tbl_[id];
	    tbl_.erase(id);
	    delete rv ; 
	    return true ;
	}
	return false ;
    };

    void Cleanup() {
	typename hash_map<pid_t, T*>::iterator ptr ;
	int i, size = tbl_.size() ;
	for (ptr=tbl_.begin(), i=0; i < size; i++, ptr++ ) {
	    if (ptr->second != 0) {
		delete ptr->second;
		ptr->second = 0;
	    }
	}
    }
};
#endif
