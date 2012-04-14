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

#include <sys/types.h>
#include <string.h>
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#include <dirent.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <DirTools.h>
#include <asm/unistd.h>
#include <unistd.h>

//_syscall3(int, getdents, uint, fd, struct dirent *, dirp, uint, count);
//_syscall3(int, getdents64, uint, fd, struct dirent *, dirp, uint, count);
//_syscall3(int, readdir, uint, fd, struct dirent *, dirp, uint, count);
syscall3(int, getdents, uint, fd, struct dirent *, dirp, uint, count)
syscall3(int, getdents64, uint, fd, struct dirent *, dirp, uint, count)
syscall3(int, readdir, uint, fd, struct dirent *, dirp, uint, count)

//int getdents(uint fd, struct dirent * dirp, uint count)
//{
//	return syscall(__NR_helloworld, i);
//}
//int getdents64(uint fd, struct dirent * dirp, uint count)
//{
//
//}
//int readdir(uint fd, struct dirent * dirp, uint count)
//{
//
//}


void NewPath(char *path, const char *change)
{
    if ('/' == change[0]) 
	strncpy(path, change, PATH_MAX) ;
    else {
	size_t len = strlen(path) ;
	// remove the tailing '/'
	if (len != 1 && path[len-1] == '/')
	    path[len-1] = 0 ;
	    
	const char *tmp, *tail ;
	char item[PATH_MAX] ;
	tmp = change ;
	tail = change ;
	while (true) {
	    // get one directory level
	    int counter = 0 ;
	    while (*tail != '/' && *tail != 0) {
		item[counter] = *tail ;
		counter ++ ;
		tail ++ ;
	    }
	    item[counter] = 0 ;
		
	    // apply the change to original directory
	    if (strcmp(item, ".") != 0) {
		if (strcmp(item, "..") == 0) {
		    // go up one level 
		    char *newtmp = &path[strlen(path)-1] ;
		    while (*newtmp != '/') newtmp-- ;
		    if (newtmp == path) *(newtmp+1) = 0 ;
		    else *newtmp = 0 ;
		} else {
		    // appent the new dir to original one
		    size_t len = strlen(path) ;
		    strncat(path, "/", PATH_MAX-len-1) ;
		    strncat(path, item, PATH_MAX-len-2) ;
		}
	    }

	    // continue to next round
	    if ('/' == *tail) {
		tmp = tail+1 ;
		tail = tmp ;
	    } else break ;
	    if (0 == *tmp) break ;
		
	}
    }
}
