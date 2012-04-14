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

#ifndef __ARGUMENT_TYPE_H
#define __ARGUMENT_TYPE_H
#include "SystemCall.h"
#include "String.h"

#include <sys/stat.h>
#if __GNUC__ < 3
#include <istream.h>
#include <ostream.h>
#else
#include <istream>
#include <ostream>
#endif
using namespace std ;

const int STR_MAX = 2048 ;
class MonitoredProc;
class ArchDep; 
class ArgBase {
    public:
    virtual ~ArgBase() {} ;
    virtual long getData() = 0 ;
    virtual int  setData(long dat) = 0 ;
    virtual void* getBuf(size_t size) = 0 ;
    virtual void* getDelimBuf(size_t size, unsigned char delim) = 0 ;
    virtual char* getStr(size_t size) = 0 ;
    virtual bool setBuf(const char *buf, size_t size) = 0 ;
    virtual bool setNewBuf(const char *buf, size_t size) = 0 ;
    virtual const char **getArgv() = 0 ;
} ;

class PersistArg : public ArgBase {
    private: 
    long data_; /* local cache to store syscall argument that is in
		   monitored process registers or memory space. */
    char *buf_; /* if buf_ is NULL, it's uninitialized. */
    size_t size_ ; /* if buf_ is initialized, it is the size of the buffer */
    public:

    PersistArg(PersistArg* arg) {
      if (!arg) {
	data_ = 0;
	buf_ = NULL;
	size_ = 0;
	return;
      }
      
      data_ = arg->getData();
      buf_ = NULL;
      size_ = 0;
      setBuf((const char *)arg->getBuf(0), arg->getSize());
    }
	    

    PersistArg(long data, char *buf, size_t size) {
	data_ = data ;
	/* if data is 0(null pointer), there will be no data */
	if (data != 0 && buf != 0) {
	    buf_ = new char[size] ;
	    if (buf_ != 0) {
		memcpy(buf_, buf, size) ;
		size_ = size ;
	    }
	} else {
	    buf_ = 0 ;
	    size_ = 0 ;
	}
    }

    PersistArg(istream &in) {
	unserialize(in) ;
    }

    ~PersistArg() { if (size_ != 0) delete [] buf_ ; } 
    
    inline size_t getSize() const { return size_; }
    long getData() { return data_ ; }
    int  setData(long dat) { data_ = dat ; return 0 ; }
    void *getBuf(size_t size) { return (void*)buf_ ; }
    void *getDelimBuf(size_t size, unsigned char delim) { return (void*)buf_ ;}
    char *getStr(size_t size) { return buf_ ; }
    bool setBuf(const char *buf, size_t size) {
	if ((buf != 0) && size) {
	    if (buf_ != 0) delete [] buf_ ;
	    buf_ = new char[size] ;
	    if (buf_ != 0) {
		memcpy(buf_, buf, size) ;
		size_ = size ;
	    }
	} else {
	    buf_ = 0 ;
	    size_= 0 ;
	}

	return true ;
    }	
    bool setNewBuf(const char *buf, size_t size) {
	return setBuf(buf, size) ;
    }
    const char **getArgv() { return (const char**)buf_ ; }
    void serialize(ostream &out) ;
    void unserialize(istream &in) ;
} ;

class SysCallArg : public ArgBase {
    /*
      This class does the "real work" for accessing system call
      arguments. Other classes are primarily wrapper classes, with
      member functions accessing the data from this class, and casting
      it into appropriate data type.
    */
    protected:
    static ArchDep *arch_;
    SysCallArgRep *repr_;

    public:
    SysCallArg(SysCallArgRep *repr) { 
	repr_ = repr ; 
    } 
    virtual ~SysCallArg() {}

    static void setArch(ArchDep *ar) {
	arch_ = ar ; 
    }

    // Operations to access parameters passed by value
    long getData() ; // copies syscall arg into data_ and return it
    int setData(long dat) ;

    void* getBuf(size_t size);
    // copy until size or delim, whichever occurs first
    void* getDelimBuf(size_t size, unsigned char delim);
    char* getStr(size_t size) { return (char*)getDelimBuf(size, 0);};
    
    bool setBuf(const char *buf, size_t size) ;
    bool setNewBuf(const char *buf, size_t size);

    const char **getArgv() ;

    PersistArg *exportArg() {
	PersistArg *tmp ;
	if (repr_->inited_) {
	    tmp = new PersistArg(repr_->data_, repr_->buf_, repr_->size_) ;
	} else 
	    tmp = 0 ;
	return tmp ;
    }
	
};

class ArgType {
    protected:
    bool needclean ;
    ArgBase *pArg ;
    public:
    ArgType(SysCallArgRep *repr) {
	pArg = new SysCallArg(repr) ;
	needclean = true ;
    }
    ArgType(PersistArg *pst) {
	pArg = pst ;
	needclean = false ;
    }
    ArgType(SysCallArg *pst) {
	pArg = pst ;
	needclean = false ;
    }
    ArgType(const ArgType& arg) {
	needclean = arg.needclean ;
	if (needclean)
	    pArg = new SysCallArg(*(SysCallArg*)arg.pArg) ;
	else 
	    pArg = arg.pArg ;
    }		   

    ArgType& operator=(const ArgType& arg) {
	if (needclean)
	     delete pArg ;
	needclean = arg.needclean ;
	if (needclean)
		pArg = new SysCallArg(*(SysCallArg*)arg.pArg) ;
	else
		pArg = arg.pArg ;
    	return *this;	 
    }		   

    virtual ~ArgType() {
	if (needclean)
	    delete pArg ;
    }
} ;
    
template <class T> 
    class BasicType : public ArgType {
	public:
    BasicType(SysCallArgRep *repr):ArgType(repr) {}
    BasicType(PersistArg *pst): ArgType(pst) {}
    BasicType(SysCallArg *pst) : ArgType(pst) {}
    T get() {
	return (T)pArg->getData() ; 
    }
  
    void set(T val) {
	pArg->setData((long)val) ;
    }
}; 

template <class T> 
    class FixSizedPtr : ArgType {
	public:
    FixSizedPtr(SysCallArgRep *repr):ArgType(repr) {}
    FixSizedPtr(PersistArg *pst): ArgType(pst) {}
    FixSizedPtr(SysCallArg *pst) : ArgType(pst) {}
  
    T *get() {
	size_t size = sizeof(T) ;
    
	T *ptr = (T*)pArg->getBuf(size) ;
	return ptr ;
    }
  
    void set(T *val) {
	size_t size = sizeof(T) ;
    
	pArg->setBuf((const char*)val, size) ;
    }
  
} ;

template <class T> 
    class Array : ArgType {
	protected:
    size_t size_ ;
	public:
    Array(SysCallArgRep *repr, size_t sz=1):ArgType(repr){
	size_ = sz ;
    }
    Array(PersistArg *pst, size_t sz):ArgType(pst) { size_ = sz ; }
    Array(SysCallArg *pst, size_t sz):ArgType(pst) { size_ = sz ; }

    T *get() {
	T *ptr = (T*)pArg->getBuf(sizeof(T)*size_) ;
	return ptr ;
    }

    void set(T *val) {
	pArg->setBuf((const char*)val, sizeof(T)*size_) ;
    }
} ;

class CString: public ArgType {
    private: int maxSize_;
    public:
    CString(SysCallArgRep *repr, int maxSize=STR_MAX):ArgType(repr) {
	maxSize_ = maxSize ;
    }
    CString(PersistArg *pst): ArgType(pst) { maxSize_ = STR_MAX ;}
    CString(SysCallArg *pst): ArgType(pst) { maxSize_ = STR_MAX ; }
 
    String get() {return pArg->getStr(maxSize_);}
    bool set(const char *s) { return pArg->setNewBuf(s, strlen(s)+1) ;}
};

class ArgV: public ArgType {
    public:
    ArgV(SysCallArgRep *repr):ArgType(repr){}
    ArgV(PersistArg *pst): ArgType(pst) {} 
    ArgV(SysCallArg *pst): ArgType(pst) {}

    const char** get() { return pArg->getArgv(); }
    int get_number_of_args() {
	const char **argv = get() ;
	int count = 0 ;
	while (argv[count] != 0)
	    count++ ;
	return count ;
    }
    const char *get_arg(int i) {
	const char **argv = get() ;
	if (i < 0 || i >= get_number_of_args())
	    return 0 ;

	return argv[i] ; 
    }
} ;
#endif
