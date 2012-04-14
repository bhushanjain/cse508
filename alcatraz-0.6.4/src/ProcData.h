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

#ifndef __PROC_DATA_H__
#define __PROC_DATA_H__
#define OPEN_MAX 256
#include <iostream>
#include <sys/types.h>
#include <malloc.h>
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#include <dirent.h>
#include <string.h>
using namespace std ;
#include <vector>

class DentReader 
{
    char *data ;
    char *ptr, *oldptr ;
    size_t size ;

    public :
    DentReader() { data = 0 ; }
    void set(char *dt, size_t sz) { data = dt ; ptr = dt ; size = sz ; oldptr = ptr ;}
    struct dirent *getNext() {
	struct dirent *retval ;
	if (0 == data) return 0 ;
	if (ptr >= data + size) return 0 ;

	retval = (struct dirent*)ptr ;
	oldptr = ptr ;
	ptr += retval->d_reclen ;
	return retval ;
    }
    
    void back() { ptr = oldptr ; }
} ;

class DentWriter
{
    char *data ;
    size_t offset ;
    size_t bufsize ;

    public:
    DentWriter(size_t sz) { 
	data = (char *)malloc(sz) ;
	offset = 0 ;
	if (0 == data) bufsize = 0 ;
	else bufsize = sz ;
    }

    int put(struct dirent *pDir) {
	if (offset + pDir->d_reclen >= bufsize) {
	    bufsize += 1000 ;
	    data = (char *)realloc(data, bufsize) ;
	}
	if (0 == data) return -1 ;
	memcpy((data+offset), (char*)pDir, pDir->d_reclen) ;
	offset += pDir->d_reclen ;
	return 0 ;
    }
    
    char *get() { return data ; } 
    size_t size() { return offset ; } 
} ;

class DentReader64 
{
    char *data ;
    char *ptr, *oldptr ;
    size_t size ;

    public :
    DentReader64() { data = 0 ; }
    void set(char *dt, size_t sz) { data = dt ; ptr = dt ; size = sz ; oldptr = ptr ;}
    struct dirent64 *getNext() {
	struct dirent64 *retval ;
	if (0 == data) return 0 ;
	if (ptr >= data + size) return 0 ;

	retval = (struct dirent64*)ptr ;
	oldptr = ptr ;
	ptr += retval->d_reclen ;
	return retval ;
    }
    
    void back() { ptr = oldptr ; }
} ;

class DentWriter64
{
    char *data ;
    size_t offset ;
    size_t bufsize ;

    public:
    DentWriter64(size_t sz) { 
	data = (char *)malloc(sz) ;
	offset = 0 ;
	if (0 == data) bufsize = 0 ;
	else bufsize = sz ;
    }

    int put(struct dirent64 *pDir) {
	if (offset + pDir->d_reclen >= bufsize) {
	    bufsize += 1000 ;
	    data = (char *)realloc(data, bufsize) ;
	}
	if (0 == data) return -1 ;
	memcpy((data+offset), (char*)pDir, pDir->d_reclen) ;
	offset += pDir->d_reclen ;
	return 0 ;
    }
    
    char *get() { return data ; } 
    size_t size() { return offset ; } 
} ;

class FDInfo {
    public:
    DentReader dReader ;
    DentReader64 dReader64 ;
    char *filename ;
    bool isFirst ;
    bool isSocket ;
    int sockFamily ;
    
    FDInfo() { filename = 0 ; isFirst = true ; isSocket = false; }
    ~FDInfo() {
	if (0 != filename) delete filename ;
    }
} ;

class ProcData {
    FDInfo *fdArray[OPEN_MAX] ;
    char *pendingData ;
    public:
    ProcData() {
	for (int i = 0 ; i< OPEN_MAX ; i++)
	    fdArray[i] = 0 ;
	pendingData = 0 ;
    } 
    ~ProcData() {
	for (int i = 0 ; i< OPEN_MAX ; i++)
	    if (fdArray[i] != 0) delete fdArray[i] ;
	delete pendingData ;
    }
    
    void storeOpenName(char *data) { pendingData = data ; }
    void insertFDName(int fd) {
	// It's a new fd, should delete old data
	if (fdArray[fd] != 0) {
	    delete fdArray[fd] ;
	    fdArray[fd] = 0 ;
	}

	if (0 == pendingData) return ;
	FDInfo *fiPtr = new FDInfo() ;
	if (0 == fiPtr) return ;

	fiPtr->filename = pendingData ;
	pendingData = 0 ;
	fdArray[fd] = fiPtr ;
    }
    FDInfo *allocFDInfo(int fd) {
	// It's a new fd, should delete old data
	if (fdArray[fd] != 0) {
	    delete fdArray[fd] ;
	    fdArray[fd] = 0 ;
	}

	FDInfo *fiPtr = new FDInfo() ;
	fdArray[fd] = fiPtr ;
	return fiPtr ;
    }
    
    FDInfo *getFDInfo(int fd) {
	if (fd < 0 || fd >= OPEN_MAX) return 0 ;
	return fdArray[fd] ;
    }
    
} ;

struct Action{
    char realpath1[PATH_MAX] ;
    char realpath2[PATH_MAX] ;
    char buf1[PATH_MAX] ;
    char buf2[PATH_MAX] ;
    int action_code ;
} ;



#if __GNUC__ >= 3 
#include <ext/hash_map>
using __gnu_cxx::hash_map  ;
#else
#include <hash_map>
#endif

// This template implements a hash table which hashes the long id to 
// a pointer pointing to the required data
template<class T>
class IDInfo {
    hash_map<long, T*>  Data ;

    public:
    IDInfo() {} ;
    ~IDInfo() { Cleanup() ;}
    T *lookUp(long id) {
        typename hash_map<long, T*>::iterator di = Data.find(id);
	if (di != Data.end())
	    return di->second ;
	else
	    return 0;
    };

    T *insert(long id, T *path) { 
	if (Data.count(id) != 0)
	    return 0;
	else {
	    Data[id] = path ;
	    return path;
	};
    };

    void remove(long id) {
	if (Data.count(id) != 0) {
	    T *rv = Data[id];
	    Data.erase(id);
	    delete[] rv;
	}
    };

    void display() {
	cout << "display" << endl ;
	typename hash_map<long, T*>::iterator ptr ;
	int i, size = Data.size() ;
	for (ptr=Data.begin(), i=0; i < size; i++, ptr++ )
	    cout << ptr->first << endl ;

    }
    void Cleanup() {
	typename hash_map<long, T*>::iterator ptr ;
	int i, size = Data.size() ;
	for (ptr=Data.begin(), i=0; i < size; i++, ptr++ )
	    delete ptr->second ;
    }
    
} ;


#endif
