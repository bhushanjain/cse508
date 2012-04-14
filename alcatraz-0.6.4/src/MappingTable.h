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

#ifndef __MAPPING_TABLE_H__
#define __MAPPING_TABLE_H__
#include "PathMapping.h"

class MappingTable {
    char cachedir[PATH_MAX] ; 
    public:
    PathMapping mapping ;
    MappingTable() ;
    ~MappingTable() ;
    
    int addMapping(int filetype, int maptype, const char *orig, 
		   const char *newpath) {
	return mapping.addMapping(filetype, maptype, orig, newpath) ;
    }

    int getMapping(const char *orig, char *newpath, size_t size) {
	return mapping.findMapping(orig, newpath, size) ;
    }

    int delMapping(const char *orig) {
	return mapping.deleteMapping(orig) ;
    }

    int isolate(const char *orig, char *newpath, bool trunc) ;
    int newEntry(int ft, int mt, const char *orig, char *newpath) ;
    int newDelete(int ft, const char *orig) ;

    /// getStatus returns the current status of the normalized path
    int getStatus(const char *orig, char *newpath, bool followlink=false) ;
    
    const char *cacheloc() { return cachedir ; } 
};

#endif
