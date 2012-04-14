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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.border.*;
import javax.swing.table.*;
import java.util.*;
import javax.swing.text.*;
import java.io.*;

public class CommHandler 
{
    FilesWindow fw;
    Vector data;
    public CommHandler()
    {
	CommServer server = new CommServer();		
		
	String fileInfo = "BOF";  

	Vector data = new Vector();
	int cnt = 0;
	while(true) {
	    Vector v=new Vector();
	    fileInfo=server.read();

	    if(fileInfo.equals("EOF"))   //end condition
		break;       

	    String splitstr[];				
	    splitstr = stringsplit(fileInfo," ");      //extract file info
	    data.add(splitstr);	     //add array of string to Vector
	    if(splitstr[5].equals("D") && splitstr[2].equals("CR"))
		{
		    File f = new File(splitstr[3]);
		    Vector childdata = getChildren(f,splitstr[1],splitstr[3],
						   splitstr[4]);
		    for(int j=0;j<childdata.size();j++)
			data.add((String [])childdata.elementAt(j));
				
		}
	    server.write("OK");
	    cnt++;
	}
	fw=new FilesWindow(data,server);
	fw.addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent e) {
		    System.exit(0);
		}
								
	    });
    }
	
    Vector getChildren(File f,String path,String privpath,String timestamp)
    {
	
	String[] files = f.list();
	Vector filedata = new Vector();
	for(int k = 0; k < files.length; k++)
	    {
		String[] newfile = new String[6];
		newfile[0] = "FILE";
		newfile[1] = path+"/"+files[k];
		newfile[2] = "CR";
		newfile[3] = privpath+"/"+files[k];
		newfile[4] = timestamp;
		File newf = new File(newfile[3]);
		if(newf.isDirectory())
		    {
			newfile[5] = "D";
			filedata.add(newfile);
			Vector childdata = getChildren(newf,newfile[1],newfile[3],newfile[4]);
			for(int j=0;j<childdata.size();j++)
			    filedata.add((String [])childdata.elementAt(j));
		    }
		else
		    {
			newfile[5] = "F";				
			filedata.add(newfile);
		    }
			
			
	    }
	return filedata;
	
    }
	
	
    // funtion to split a string based on adelimiter	
    static String [] stringsplit(String str,String delim)
    {
	int cnt = 0;
	for(int i = 0; i <str.length(); i++)
	    {
        	if(str.charAt(i) == ' ')
		    cnt++;
	    }
	String arr[] = new String[cnt+1];
	StringTokenizer st= new StringTokenizer(str,delim);
	cnt = 0;
	while(st.hasMoreTokens())
	    {
		String token=st.nextToken();
		arr[cnt]=token;
		cnt++;
	    }
	return(arr);
    }
}
