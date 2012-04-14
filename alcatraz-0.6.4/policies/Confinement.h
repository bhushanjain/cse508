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

#ifndef __CONFINEMENT_H__
#define __CONFINEMENT_H__

#include <Extension.h>

class Confinement: public Extension
{
 private:
    void priviledged(); 
    void invalid(); 
    void ipccalls();
    void checkdevaccess(); 

 public:
    int deliverEvent() ;

    Confinement *clone() {
	Confinement *tmp = new Confinement(*this) ;
	return tmp ;
    }
    
} ;


#endif
