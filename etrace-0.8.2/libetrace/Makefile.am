#     Copyright (C) 2002 - 2006 Zhenkai Liang and R. Sekar
    
#     This file is part of etrace.

#     Etrace is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.

#     Etrace is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with etrace; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

lib_LTLIBRARIES = libetrace.la
libetrace_la_SOURCES = ArgumentType.cpp MonitoredProc.cpp Extension.cpp ManagerClass.cpp linux-ptrace-x86/LinuxPtraceX86.cpp ManagerClass.h 
libetrace_la_LDFLAGS = -ldl
include_HEADERS = ArchDependent.h ArgumentType.h  Extension.h  MonitoredProc.h  SystemCall.h ProcHashTable.h ArgumentRep.h String.h linux-ptrace-x86/LinuxTypes.h linux-ptrace-x86/LinuxPtraceX86.h linux-ptrace-x86/SysCallInfo.h linux-ptrace-x86/SysCallNum.h


