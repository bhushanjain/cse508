/*  Copyright (C) 2003 - 2004 Divya Padmanabhan
    
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

import java.net.*;
import java.io.*;

public class CommServer 
{
    
    final int port=8500;
    ServerSocket server_socket;
    BufferedReader input;
    PrintWriter  output;
    Socket socket;
    public CommServer()
    {	
	try {
		    
	    server_socket = new ServerSocket(port);
	    System.out.println("Alcatraz user interface server is ready.");
	    socket = server_socket.accept();
	}		
	catch(BindException be) {
	    System.exit(0);
	}
	catch (IOException e) {
	    System.err.println(e);
	}
    }
	
    public int write(String s)
    {
	try {
	    output= new PrintWriter(socket.getOutputStream(), true);
	    output.println(s);
	}
	catch (IOException e) {
	    // connection closed by client
	    try {
		socket.close();
		return -1;
	    }
	    catch (IOException excp) {
		return -1;
	    }
	}
	return 0;			
    }
	
    public String read()
    {
	try {
	    input = new BufferedReader(new InputStreamReader(socket.getInputStream())); 
	    String message = input.readLine();
	    return message;
	}
	catch (IOException excp) {
	    return "Error";
	}		
    }			
	
	
    int close()
    {	
	try {
	    socket.close();
	    server_socket.close();
			
	}
	catch (IOException excp) {
	    return -1;
	}
		
	return 0;
    }
		
		
}
		
