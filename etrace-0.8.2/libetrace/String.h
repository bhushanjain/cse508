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

#ifndef __STRING__H__
#define __STRING__H__

#include <cstring>

class String {
    public:
    String(): sz_(0) {
	str_ = new char[1];
	str_[0] ='\0';
    };
  
    String(const char* s) {
	if (s != NULL) {
	    sz_ = strlen(s);
	} else 
	    sz_ = 0;
    
	str_ = new char[sz_+1];
	if (s != NULL)
	    strncpy(str_, s, sz_);
	str_[sz_] = '\0';
    }
  
    String(const String& s) {
	sz_ = s.sz_;
	str_ = new char[sz_+1];
	strncpy(str_, s.str_, sz_);
	str_[sz_] = '\0';
    }
  
    const String& operator=(const String& rhs) {
	if (this == &rhs) 
	    return *this;
	else {
	    if (str_ != NULL && sz_ != 0) 
		delete [] str_;
	    sz_ = rhs.sz_;
	    str_ = new char[sz_ + 1];
	    strncpy(str_, rhs.str_, sz_);
	    str_[sz_] = '\0';
	}
	return *this;
    }
  
    bool operator==(const String& rhs) const {
	if (sz_ != rhs.sz_)
	    return false;
	return (strncmp(str_, rhs.str_, sz_) == 0)? true : false;
    }
  
    bool operator !=(const String& rhs) const 
	{ return !operator==(rhs); }
  
    const char* c_str() const
	{ return str_; }
  
    unsigned int length() const 
	{ return sz_; }
  
    ~String() {
	if (str_ != NULL) {
	    delete [] str_;
	}
	sz_ = 0;
    }
  
    private:
    char* str_;
    unsigned int sz_;
};


#endif
