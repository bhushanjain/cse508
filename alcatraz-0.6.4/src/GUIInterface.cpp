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

#include "GUIInterface.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void error(char *msg)
{
    return;
}

static int sockfd;


int connectgui(int portno)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);    

    if (sockfd < 0)
	{
	    error("ERROR opening socket");
	    return -1;
	}
    
    server = gethostbyname("127.0.0.1");
    if (server == NULL)
	{
	    fprintf(stderr,"ERROR, no such host\n");
	    return -1 ;
	}
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serv_addr.sin_addr.s_addr,
	  server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
	{
	    return -1 ;
	}
    return 0 ;

}

#ifdef INSTALL_SHIELD
int sendfile(const char *message, size_t size)
{
    int n;
    char buffer[256];	
	
    n = write(sockfd, message, size);	
    if (n < 0) {
	return -1;
    }
    
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) {
	return -1;
    }
    
    return 0 ;
}
#else
int sendfile(const char *filepath, const char *status, const char *privpath, const char *timestamp, const char *filetype)
{

    int n;
	
    char message[1070];	
	
	
    char buffer[256];	
	
    strncpy(message,"FILE ",6);
    strncat(message,filepath,strlen(filepath));
    strncat(message," ",2);
    strncat(message,status,strlen(status));
    strncat(message," ",2);
    strncat(message,privpath,strlen(privpath));
    strncat(message," ",2);
    strncat(message,timestamp,strlen(timestamp));
    strncat(message," ",2);
    strncat(message, filetype, strlen(filetype)) ;
    strncat(message,"\n",2);		
	
    n = write(sockfd,message,strlen(message));	
    if (n < 0) 
	{
	    error("ERROR writing to socket");
	    return -1;
	}

    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
	{
	    error("ERROR reading from to socket");
	    return -1;
	}
    
    return 0 ;
}
#endif

int finish_sending()
{
    int n;
    char message[5]="EOF\n" ;
    n = write(sockfd,message,strlen(message));
    if (n < 0) 
	{
	    return -1;
	}
    
    printf("Waiting for response from user ...\n");    			
    return 0 ;
}


int sendOK()
{
    int n;
    char message[4]="OK\n" ;
    n = write(sockfd,message,strlen(message));
    if (n < 0) 
	{
	    return -1;
	}    
    return 0 ;
}

int sendNOK()
{
    int n;
    char message[5]="NOK\n" ;
    n = write(sockfd,message,strlen(message));
    if (n < 0) 
	{
	    return -1;
	}    
    return 0 ;
}

// Return code: -1 for error, 0 for successful processing
int getResponse()
{
    int n;
    char buffer[256];
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
	{
	    return -1;
	}
 
 
    n=sendOK();
    if(n==-1)
	return -1;

    if(strncmp(buffer,"Commit",6)==0)
	{
	    do
		{
		    bzero(buffer,256);
		    n = read(sockfd,buffer,255);
		    if (n < 0)
			{
			    return -1;
			}
		    n=sendOK();
		    if(n==-1)
			return -1;
		}		
	    while(strncmp(buffer,"EOF",3)!=0);
	    return 1;
	}
    if(strncmp(buffer,"Cancel",6)==0)    
	return 0;
    
    
    return 0;
}
