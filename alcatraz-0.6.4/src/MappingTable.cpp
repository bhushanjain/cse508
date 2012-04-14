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

#include "CommonDefs.h"
#include "MappingTable.h"

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

MappingTable::MappingTable()
{
    char buf[PATH_MAX] ;
    realpath("/tmp", buf) ;
    snprintf(cachedir, PATH_MAX, "%s/Alcatraz.%d", buf, getpid()) ;
    mkdir(cachedir, 0700) ;
    mapping.setCacheDir(cachedir) ;
}

MappingTable::~MappingTable()
{
}

int MappingTable::isolate(const char *orig, char *newpath, bool trunc) 
{
    int fdnew, fdold ;
    struct stat statbuf ;

    snprintf(newpath, PATH_MAX, "%s/UI.XXXXXX", cachedir) ;
    
    fdnew = mkstemp(newpath) ;
    if (fdnew < 0) 
	return -1 ;

    fdold = open(orig, O_RDONLY) ;
    if (fdold < 0) {
	unlink(newpath) ;
	return -2 ;
    }
    else {
	/* copy the original file to the location 
	   and set owner and permission*/
	if (!trunc) {
	    char buf[2048] ;
	    size_t size ;
	    while(1) {
		size = read(fdold, buf, 2048) ;
		//Bhushan: Encrypt the data while copying.
		if (size == 0) break; 
		write(fdnew, buf, size) ;
	    }
	}
	close(fdold) ;

	if (stat(orig, &statbuf) == 0) {
	    /* set owner and permission info */ 
		printf("\nThe permissions for path %s are owner: %d and mode: %d\n",orig,statbuf.st_uid,statbuf.st_mode);
	    chmod(newpath, statbuf.st_mode) ;
	}
    }
    close(fdnew) ;
    /* insert into the PathMapping object */
    int retval = mapping.addMapping(TYPE_FILE, PATH_MODIFIED, orig, newpath) ; 
    if (retval < 0) {
	unlink(newpath) ;
	return -3 ;
    }

    return 0 ;
}

int MappingTable::newEntry(int filetype, int maptype, const char *orig, 
			   char *newpath) 
{
    snprintf(newpath, PATH_MAX, "%s/UI.XXXXXX", cachedir) ;
    close(mkstemp(newpath)) ;
    unlink(newpath);
    return mapping.addMapping(filetype, maptype, orig, newpath) ;
}

int MappingTable::newDelete(int ft, const char *orig)
{
    int retval ;
    retval = mapping.addMapping(ft, PATH_DELETED, orig, 0) ;
    return retval ;
}

int MappingTable::getStatus(const char *orig, char *newpath, bool followlink) 
{
    char buf[PATH_MAX];

    // canicalize path into buf
    strncpy(buf, orig, PATH_MAX) ;
    
    int type = mapping.findMapping(buf, newpath, PATH_MAX) ;

    if (PATH_NEW == type)
	strncpy(newpath, orig, PATH_MAX) ;

    return type ;
}
