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

import java.io.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.border.*;
import javax.swing.table.*;
import java.util.*;
import javax.swing.text.*;
import javax.swing.tree.*;


class ExecuteDialog extends JDialog implements ActionListener
{
	
    JTextField txtCommand;
    JCheckBox chkTerminal;
    String oldFile, newFile;
	
    public ExecuteDialog(String oldFile, String newFile)
    {
	this.oldFile = oldFile;
	this.newFile = newFile;
		
	setTitle("Execute");
	JPanel base = (JPanel)getContentPane();
	base.setLayout(new GridLayout(3,1));		
		
		
	//Command Panel
	JPanel commandPanel = new JPanel();
	commandPanel.setLayout(new FlowLayout(FlowLayout.LEFT));		
	JLabel lblCommand = new JLabel("Command: ");
	txtCommand = new JTextField();	
	txtCommand.setPreferredSize(new Dimension(Constants.EXECUTE_DIALOG_WIDTH-getFontMetrics(lblCommand.getFont()).stringWidth("Command: ")-Constants.TEXTFIELD_PADDING,25));
	commandPanel.add(lblCommand);
	commandPanel.add(txtCommand);		
		
	//check box Panel
	JPanel checkPanel = new JPanel();
	checkPanel.setLayout(new FlowLayout(FlowLayout.LEFT));		
	chkTerminal = new JCheckBox("Run in Terminal");
	checkPanel.add(chkTerminal);
		
	//old, new and ok buttons Panel
	JPanel buttonsPanel = new JPanel();
	JButton btnOldFile  = new JButton("Old File");
	JButton btnNewFile  = new JButton("New File");
	JButton btnClose = new JButton("Close");
	JButton btnOk = new JButton("Ok");
	btnOldFile.setPreferredSize(new Dimension(Constants.STANDARD_BUTTON_WIDTH,Constants.STANDARD_BUTTON_HEIGHT));
	btnNewFile.setPreferredSize(new Dimension(Constants.STANDARD_BUTTON_WIDTH,Constants.STANDARD_BUTTON_HEIGHT));	
	btnClose.setPreferredSize(new Dimension(Constants.STANDARD_BUTTON_WIDTH,Constants.STANDARD_BUTTON_HEIGHT));	
	btnOk.setPreferredSize(new Dimension(Constants.STANDARD_BUTTON_WIDTH,Constants.STANDARD_BUTTON_HEIGHT));
	btnOldFile.addActionListener(this);
	btnNewFile.addActionListener(this);
	btnOk.addActionListener(this);		
	btnClose.addActionListener(this);		
	buttonsPanel.add(btnOldFile);
	buttonsPanel.add(btnNewFile);
	buttonsPanel.add(btnOk);
	buttonsPanel.add(btnClose);

	base.add(commandPanel);
	base.add(checkPanel);
	base.add(buttonsPanel);
		
	setResizable(false);		
	setSize(Constants.EXECUTE_DIALOG_WIDTH,Constants.EXECUTE_DIALOG_HEIGHT);
	setLocation((Constants.SCREEN_WIDTH-Constants.EXECUTE_DIALOG_WIDTH)/2,(Constants.SCREEN_HEIGHT-Constants.EXECUTE_DIALOG_HEIGHT)/2);
	setModal(true);
    }
	
    public void actionPerformed(ActionEvent e)
    {
	if(e.getActionCommand().equals("Old File"))
	    {
		txtCommand.setText(txtCommand.getText() + " _OLD_FILE_");
	    }
	else if(e.getActionCommand().equals("New File"))
	    {
		txtCommand.setText(txtCommand.getText() + " _NEW_FILE_");
	    }
	else if(e.getActionCommand().equals("Close"))
	    {
		dispose();
	    }
	else if(e.getActionCommand().equals("Ok"))
	    {
		if(txtCommand.getText().equals(""))
		    {
			JOptionPane.showMessageDialog(this,"Please type in a command.","Error!",JOptionPane.ERROR_MESSAGE);
		    }
		else
		    {
			String str = txtCommand.getText();
				
			str = replaceAllStrings(str,"_OLD_FILE_",oldFile);
			str = replaceAllStrings(str,"_NEW_FILE_",newFile);  
			ExecutionThread et = new ExecutionThread(str,
						    chkTerminal.isSelected());
			et.start();
		    }
	    }
    }
	
    String replaceAllStrings(String str,String toreplace,String with)
    {
	int ind = str.indexOf(toreplace);
	while(ind!=-1)
	    {
		str = str.substring(0,ind) + with + str.substring(ind+toreplace.length(),str.length());
		ind = str.indexOf(toreplace);
	    }
	return str;		
    }
		
    /*public static void main(String args[])
      {
      ExecuteDialog ed = new ExecuteDialog("abc","xyz");
      ed.setVisible(true);
      }*/
	
		
}


class ExecutionThread extends Thread {
	
    String command;
    boolean terminal;
	
    public ExecutionThread(String command,boolean terminal) {
        this.command = command;
        this.terminal = terminal;
    }
    
    public void run()
    {
	try
	    {
		Runtime rt = Runtime.getRuntime();
   	    	Process p = null;		   	    	   	    	
   	    	command.trim();
   	    	
   	    	String cmd[];
   	    	int i;
   	    	
   	    	if (terminal)
		    {   	    		
		    	StringTokenizer st = new StringTokenizer(command," ");
   	    		cmd  = new String [3+st.countTokens()];
   	    		cmd[0] = "xterm";
   	    		cmd[1] = "-hold";
   	    		cmd[2] = "-e";
   	    		i=3;
	   	    	for(;i<cmd.length;i++)
			    {
		    		cmd[i] = st.nextToken();
			    }
		
		    }
   	    	else
		    {
   	    		cmd = new String [3];
   	    		cmd[0] = "sh";
   	    		cmd[1] = "-c";
   	    		cmd[2] = command;
		    }
   	    	
   	    	
   	    	
		p = rt.exec(cmd);			    
		p.waitFor();
	    }
	catch(IOException ioE)
	    {
	    }		   
	catch(InterruptedException intE)
	    {
	    }		   
    }
}

		
