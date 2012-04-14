#     Copyright (C) 2002 - 2006 Zhenkai Liang
    
#     This file is part of alcatraz.

#     Alcatraz is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.

#     Alcatraz is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with alcatraz; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

lib_LTLIBRARIES = libalcatraz.la
libalcatraz_la_SOURCES = Alcatraz.cpp Alcatraz.h CommonDefs.h ComplexCall.cpp DirTools.cpp DirTools.h GUIInterface.cpp GUIInterface.h Isolation.cpp MappingTable.cpp MappingTable.h PathHashTable.h PathMapping.cpp PathMapping.h ProcData.h ViewMaintain.cpp
libalcatraz_la_LDFLAGS = -L$(ETRACEDIR)/lib -letrace -lssl
libalcatraz_la_CXXFLAGS = -I$(ETRACEDIR)/include
