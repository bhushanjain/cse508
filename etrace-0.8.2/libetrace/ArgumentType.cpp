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

#include <malloc.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#include <netinet/in.h>

#include "ArgumentType.h"
#include "MonitoredProc.h"
#include "ManagerClass.h" 
#include "ArchDependent.h"

ArchDep *SysCallArg::arch_;

long SysCallArg::getData() 
{
  if (!repr_->inited_) {
      pid_t pid = repr_->pid_ ;

      if (repr_->ISreg_) 
	  repr_->data_ = arch_->getReg(pid, repr_->addr_) ;
      else
	  arch_->getData(pid, repr_->addr_, (char*)&repr_->data_, 4) ;

      repr_->inited_ = true ;
  }
  return repr_->data_;
}

int SysCallArg::setData(long dat)
{
    pid_t pid = repr_->pid_ ;
    
    if (repr_->ISreg_) 
	arch_->setReg(pid, repr_->addr_, dat) ;
    else
	arch_->setData(pid, repr_->addr_, (char*)&dat, 4) ;
    
    repr_->data_ = dat ;
    repr_->inited_ = true ;
    
    return 0 ;
}

void *SysCallArg::getBuf(size_t size)
{
    /* Note: We are caching the buffer pointed to by a system call argument
       in buf_. This way, we dont need to repeatedly copy it from monitored
       process memory. If multiple Extensions are involved, the caching requires
       an the assumption: that all of these Extension will interpret a particular 
       system call argument as a pointer pointing to the same type of data. */

    if (repr_->buf_ == NULL) { // not cached, read from monitored proc mem 
	pid_t pid = repr_->pid_ ;

	getData() ;
	if (0 == repr_->data_) {
	    return 0 ;
	}
	repr_->buf_ = new char[size] ;
	repr_->size_ = size ;
	if ( repr_->buf_ != NULL ) {
	    int retval ;
	    retval = arch_->getData(pid, repr_->data_, repr_->buf_, size) ;

	    // check for error, set buf_ to NULL and delete buf_
	    if (retval < 0) {
		delete [] repr_->buf_;
		repr_->buf_ = NULL ;
	    }
	}
	else
	    cerr << "new error!" << endl ;
    }
    return repr_->buf_ ;
}

void *SysCallArg::getDelimBuf(size_t size, unsigned char delim)
{
    if ( repr_->buf_ == NULL ) {

	getData() ;
	pid_t pid = repr_->pid_ ;

	const size_t BUFSIZE = 512 ;
	size_t BufUnit = BUFSIZE < size? BUFSIZE : size ;
	size_t bufsize = BufUnit ;
	repr_->buf_ =  new char[bufsize] ;
	if ( repr_->buf_ != NULL ) {

	    while (1) {
		int retval ;
		bool found = false ;
		retval = arch_->getData(pid, repr_->data_+bufsize-BufUnit, 
				    repr_->buf_+bufsize-BufUnit, BufUnit) ;
		if (retval < 0) {
		    delete [] repr_->buf_;
		    repr_->buf_ = NULL ;
		    break ;
		}
		for ( unsigned int i = bufsize-BufUnit; i < bufsize ; i++ )
		    if ( repr_->buf_[i] == delim ) {
			repr_->size_ = i+1 ; /* including the delimeter */
			found = true ;
			break ;
		    }
		if (found) break ;
		bufsize += BufUnit ;
		repr_->buf_ = (char*) realloc(repr_->buf_, bufsize) ;
		if (repr_->buf_ == NULL) break ;
	    }
	} else 
	    cerr << "new error!" << endl ;
    }
    return repr_->buf_ ;
}

bool SysCallArg::setBuf(const char *buf, size_t size)
{
    int retval ;
    getData() ;

    pid_t pid = repr_->pid_ ;

    retval = arch_->setData(pid, repr_->data_, buf, size) ;
    if ( retval == 0 ) {
	if ( repr_->buf_ ) delete [] repr_->buf_ ;
	char *tmp =  new char[size] ;
	if (0 != tmp) {
	    memcpy(tmp, buf, size) ;
	}
	repr_->buf_ = tmp ;
	return true ;
    }
    return false ;
    
}

bool SysCallArg::setNewBuf(const char *buf, size_t size)
{
    int retval ;
    getData() ;

    pid_t pid = repr_->pid_ ;
    
    long esp = arch_->allocMem(pid, size) ; 
    if (0 == esp) {
	cerr << "SysCallArg: alloc new buffer error" << endl ;
	return false ;
    }

    repr_->data_ = esp ;

    if (repr_->ISreg_) 
	arch_->setReg(pid, repr_->addr_, esp) ;
    else
	arch_->setData(pid, repr_->addr_, (char*)&repr_->data_, 4) ;

    retval = setBuf(buf, size) ;

    return retval ;
}

const char** SysCallArg::getArgv()
{
    char *tmp ;
    long backup ;
    int count, totallen ;
    char *args[256] ;
    char **result ;

    if (repr_->buf_) return (const char**)repr_->buf_ ;

    getData() ;

    pid_t pid = repr_->pid_ ;

    count = 0 ;
    totallen = 0 ;
    while(true) {
	arch_->getData(pid, repr_->data_+count*sizeof(tmp), (char*)&tmp, sizeof(tmp)) ;
	if (tmp == NULL) {
	    args[count] = NULL ;
	    break;
	}

	backup = repr_->data_ ;
	repr_->data_ = (long)tmp ;
	args[count] =  getStr(STR_MAX) ;
	totallen += (strlen(args[count])+1) ;
	repr_->data_ = backup ;
	repr_->buf_ = NULL ;
	count ++ ;
    }
    
    repr_->size_ = (count+1)*sizeof(char*)+totallen ;
    result = new char *[repr_->size_] ;
    
    int i ;
    totallen = (count+1)*sizeof(char*) ;
    for (i=0; i<count; i++) {
	result[i] = (char*)result + totallen ;
	strcpy(result[i], args[i]) ;
	totallen += (strlen(args[i])+1) ;
    }
    result[count] = NULL ;
 
    /* clean up */
    repr_->buf_ = (char*)result ;
    for (i=0; i<count; i++) 
	delete [] args[i] ;
    return (const char**)result ;
}

void PersistArg::serialize(ostream &out)
{
    uint32_t temp ;
    temp = htonl(data_) ;
    out.write((char*)&temp, sizeof(temp)) ;
    temp = htonl(size_) ;
    out.write((char*)&temp, sizeof(temp)) ;
    if (0 != size_) {
	out.write(buf_, size_) ;
    }
} 

void PersistArg::unserialize(istream &in)
{
    uint32_t temp ;
    in.read((char*)&temp, sizeof(temp)) ;
    if (in.eof())
	return ;
    data_ = (long)ntohl(temp) ;
    in.read((char*)&temp, sizeof(temp)) ;
    if (in.eof())
	return ;
    size_ = (size_t)ntohl(temp) ;
    if (0 != size_) {
	buf_ = new char[size_] ;
	if (buf_ != 0) in.read(buf_, size_) ;
    }
    else 
	buf_ = 0 ;
}

