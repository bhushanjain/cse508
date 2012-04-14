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

class FilesDiffWindow extends JFrame implements ActionListener
{
	
    public FilesDiffWindow(String title,String file1,String file2)
    {
	super(title);
		
	JPanel root=(JPanel)getContentPane();
	root.setLayout(new BorderLayout());
	Runtime run = Runtime.getRuntime();			
	String[] cmd = {"/bin/sh", "-c", "diff "+file1+" "+file2};

	JTextArea diff=new JTextArea("Difference between files:"+file1+" and "+file2+"\n\n");
	try
	    {
		Process p=run.exec(cmd);
		InputStream i;
		i=p.getInputStream();
		int ch;
		while( ( ch = i.read() ) != -1 )
		    {
			diff.append(String.valueOf((char)ch));
		    }
	    }
	catch(Exception e)
	    {
		System.err.println("ERROR\n");
	    }
		
	JScrollPane diffPane=new JScrollPane(diff);
		
	diffPane.setMinimumSize(new Dimension(480,190));
		
	JPanel buttonPanel=new JPanel();
	buttonPanel.setLayout(new BorderLayout());		
	JButton close=new JButton("Close");
	close.setPreferredSize(new Dimension(120,30));
	close.addActionListener(this);
	buttonPanel.setMinimumSize(new Dimension(480,50));
	buttonPanel.add(close,BorderLayout.EAST);
		
	root.add(diffPane,BorderLayout.CENTER);
		
	root.add(buttonPanel,BorderLayout.SOUTH);
		
	this.addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent e) {
		    System.exit(0);
		}
	    });
    }
    public void actionPerformed(ActionEvent e)
    {
	if(e.getActionCommand().equals("Close"))
	    {
		this.dispose();
	    }
		
			
    }

}

	
