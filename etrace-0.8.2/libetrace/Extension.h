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

#ifndef __EXTENSION_H
#define __EXTENSION_H
#include "String.h"
#include "SystemCall.h"
#include "ArgumentType.h"
#include <iostream>
using namespace std;

class MonitoredProc ;
/*****************************************************************************
 * Doubly Linked List class
 ****************************************************************************/

/*
  Conventions: head of a list will have left pointer pointing to tail, and
  the tail will have right link pointing to the head. By keeping the list
  as a circular node, you are guaranteed that all pointers (left and right)
  always have legitimate value.

  Limitation: A list must have at least one element. Attempt to delete the
  last element from the list has no effect.

  This class DOES NOT handle memory management. Thus the destructor
  does nothing.
*/
using namespace std ;
class DNode
{
    private:
    DNode *left;
    DNode *right;

    public:
    DNode() { init(); }
    ~DNode() { finalize(); }

    int init(void) {
	// set node to point to itself 

	left = right = this;
	return 0;
    }
    int finalize(void)  { return 0; }

    void InsertRight(DNode *p) {
	if (!p)
	    return;

	// link p to its successor on the right
	p->right = right;
	right->left = p;
                        
	// link p to the current node on its left
	p->left = this;
	right = p;
    }

    void InsertLeft(DNode *p) {
	if (!p)
	    return;

	// link p to its successor on the left
	p->left = left;
	left->right = p;

	// link p to the current node on its right
	p->right = this;
	left = p;
    }

    DNode *DeleteNode(void) {
	// node on the left is linked to right node
	left->right = right;

	// node on the right is linked to left node
	right->left = left;

	// return the address of the current node
	return this;
    }

    const DNode *NextNodeRight(void) const { return this->right; }
    const DNode *NextNodeLeft(void) const  { return this->left; }
    DNode *NextNodeRight(void) { return this->right; }
    DNode *NextNodeLeft(void)  { return this->left; }
                
};

/* return value of deliverEvent */
const int EXTENSION_NORMAL = 0 ;    /* continue with next extension */
const int EXTENSION_ABORT = 1 ;     /* kill this process */
const int EXTENSION_ACCEPT = 2 ;    /* don't trace this process any more */

const int EVENT_ENTRY = 0 ;
const int EVENT_EXIT = 1 ;

class Extension : public DNode {
    unsigned int priority_ ;
    protected:
    MonitoredProc* mp;
    public:
    Extension() {
	priority_ = 0 ;
    }
    virtual ~Extension() {} ;

    unsigned int priority() { return priority_ ; }
    void priority(unsigned int pr) { priority_ = pr ; }

    virtual int deliverEvent(); 
    virtual Extension* clone() { return new Extension(); };
    virtual void setMonitoredProc(MonitoredProc *MP); 

    void getEvent(int*, int*);
    SysCallArgRep *getArgRep(int i) ;
  
    int abort(int err);
    int fake(int rc); 
    void terminate(); 

    int getCurrentSysCall();
    String programName();
    String getCwd();

 
    int getPID();
    int getPPID();
    int isChild(int);

    int getUID();
    int getEUID();
  
    int getGID();
    int getEGID();
  
    int getREUID();
  
    int realRC();
    void logMessageStr(String, String);
    void logMessageInt(String, int);
  
    String RealPath(String path);
    int checkPath(String); 
    int writeFile(int mode);
    int readFile(int mode);
    int isInFile(String, String);
    int sameFile (String , String);
    int inTree(String dir,String file);
    int isPrefix(String, String);
    int isSuffix(String, String);
    int isSymLink(String);
    void switchExtension(const char *libname, const char *classname) ;
	
    bool procInSession(pid_t pid);
} ;

class KILL : public Extension 
{
    public:
    KILL() : Extension() {} ;
    int deliverEvent() {
	return EXTENSION_ABORT ;
    }
    KILL *clone() { return new KILL(*this) ; }
} ;

class ACCEPT : public Extension
{
    public:
    ACCEPT() : Extension() {} ;
    int deliverEvent() {
	return EXTENSION_ACCEPT ;
    }
    ACCEPT *clone() { return new ACCEPT(*this) ; }
} ;

class NONE: public Extension
{
    public:
    NONE() : Extension() {} ;
    int deliverEvent() {
	return EXTENSION_NORMAL ;
    }
    
    NONE *clone() { return new NONE(*this) ; }
} ;

#endif

