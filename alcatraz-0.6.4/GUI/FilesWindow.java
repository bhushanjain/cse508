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
import javax.swing.tree.*;

class FilesWindow extends JFrame implements ActionListener
{

    Vector copyofrowData;
    JCheckBox c1= new JCheckBox("All");
    JCheckBox c2= new JCheckBox("Created");
    JCheckBox c3= new JCheckBox("Modified");
    JCheckBox c4= new JCheckBox("Deleted");
    JCheckBox c5= new JCheckBox("Permission Changed");
    JCheckBox c6= new JCheckBox("Owner/Group Changed");
    JCheckBox c7= new JCheckBox("Binaries");
    JCheckBox c8= new JCheckBox("Libraries");
    JButton exec;
    JTable table;
    JPanel leftPane  = new JPanel();
    JPanel rightPane = new JPanel();
    JPanel base;
    CommServer server;
    Vector Data;  
    Vector files;
    TreePanel treePanel;
    JTextPane text = new JTextPane();
    String file1,file2;
    JButton commit ;
    public FilesWindow(Vector data,CommServer serv)
    {
		
	super("ALCATRAZ");
	file1=new String();
	file2=new String();
		
		
	server=serv;				
	if(data.size()==0) {
	    server.write("Cancel");
	    server.read();
	    server.close();    	   			
	    this.dispose();
	    System.exit(0);
	}
		
	Data =new Vector();
	Data=data;
	files=new Vector();
	for(int i=0;i<data.size();i++) {
	    String arr[]=(String [])data.elementAt(i);
	    files.add(arr[1]);
	}
		
	Border blackline, invisibleline, emptyBorder;
	blackline = BorderFactory.createLineBorder(Color.gray);
	invisibleline = BorderFactory.createLineBorder(Color.lightGray,0);
        emptyBorder =  BorderFactory.createEmptyBorder(5,5,5,5);
	TitledBorder propertiesTitle,filesTitle,buttonsTitle,legendTitle;
	filesTitle = BorderFactory.createTitledBorder(blackline, "Files",
	      TitledBorder.LEFT,TitledBorder.CENTER,
	      new Font("Arial",Font.BOLD,12),Color.darkGray);
	propertiesTitle = BorderFactory.createTitledBorder(
	      blackline, "Properties",
	      TitledBorder.LEFT,TitledBorder.CENTER,
	      new Font("Arial",Font.BOLD,12),Color.darkGray);
	buttonsTitle = BorderFactory.createTitledBorder(
	      invisibleline, "  ",
	      TitledBorder.LEFT,TitledBorder.CENTER,
	      new Font("Arial",Font.PLAIN,3),Color.lightGray);
        legendTitle = BorderFactory.createTitledBorder(
	      blackline, "Legend",
	      TitledBorder.LEFT,TitledBorder.CENTER,
	      new Font("Arial",Font.BOLD,12),Color.darkGray);
	
	base=(JPanel)getContentPane();	    	
	base.setLayout(new BorderLayout());
	
	JPanel leftPane  = new JPanel();
	JPanel rightPane = new JPanel();
	  
	leftPane.setLayout(new BorderLayout());
	leftPane.setBorder(filesTitle);
	rightPane.setLayout(new BorderLayout());			    
	rightPane.setPreferredSize(new Dimension(
	      Constants.PROPERTIES_PANEL_WIDTH, 
	      Constants.PROPERTIES_PANEL_HEIGHT));
                              
	JPanel propertiesPane = new JPanel();	    
	propertiesPane.setLayout(new BorderLayout());
	propertiesPane.setBorder(propertiesTitle);
		
	text.setEditable(false);
		
	JPanel nowrapPanel = new JPanel();
	nowrapPanel.setLayout(new BorderLayout());
	nowrapPanel.add(text,BorderLayout.CENTER);
		
	JScrollPane textPane = new JScrollPane(nowrapPanel);
		
	ImageIcon diff = new ImageIcon("gifs/diff.gif");
	propertiesPane.add(textPane,BorderLayout.CENTER);
	exec = new JButton("Execute Command",diff);
	exec.setPreferredSize(new Dimension(Constants.SPECIAL_BUTTON_HEIGHT*4,
					    Constants.SPECIAL_BUTTON_HEIGHT));
	exec.addActionListener(this);
	propertiesPane.add(exec,BorderLayout.NORTH);
	exec.setVisible(false);
	exec.setForeground(new Color(0,0,128));
	text.setEditable(false);
		

		
	JPanel buttonsPane = new JPanel();
	buttonsPane.setLayout(new BorderLayout());
	buttonsPane.setBorder(buttonsTitle);
    
	ImageIcon ok = new ImageIcon("gifs/ok.gif");
	ImageIcon can = new ImageIcon("gifs/cancel.gif");
	    
	commit = new JButton("Commit",ok);
	commit.setPreferredSize(new Dimension(
	       Constants.SPECIAL_BUTTON_HEIGHT*4, 
	       Constants.SPECIAL_BUTTON_HEIGHT));
	commit.addActionListener(this);
	JButton cancel = new JButton("Cancel",can);
	cancel.setPreferredSize(new Dimension(
	       Constants.SPECIAL_BUTTON_HEIGHT*4,
	       Constants.SPECIAL_BUTTON_HEIGHT));
	cancel.addActionListener(this);
	buttonsPane.add(commit,BorderLayout.NORTH);
	buttonsPane.add(cancel,BorderLayout.SOUTH);
	    
	rightPane.add(propertiesPane,BorderLayout.CENTER);
	rightPane.add(buttonsPane,BorderLayout.NORTH);
	    
	JPanel choice=new JPanel();
	choice.setLayout(new GridLayout(4,3));
	    
	JLabel l1 = new JLabel(" Select");
	l1.setFont(new Font("Arial",Font.BOLD,10));
	l1.setForeground(Color.darkGray);
	    
	c1.setFont(new Font("Arial",Font.PLAIN,11));
	c2.setFont(new Font("Arial",Font.PLAIN,11));
	c3.setFont(new Font("Arial",Font.PLAIN,11));
	c4.setFont(new Font("Arial",Font.PLAIN,11));
	c5.setFont(new Font("Arial",Font.PLAIN,11));
	c6.setFont(new Font("Arial",Font.PLAIN,11));
	c7.setFont(new Font("Arial",Font.PLAIN,11));
	c8.setFont(new Font("Arial",Font.PLAIN,11));
	    
	c1.addActionListener(this);
	c1.setSelected(true);
	c2.addActionListener(this);
	c3.addActionListener(this);
	c4.addActionListener(this);
	c5.addActionListener(this);
	c6.addActionListener(this);
	c7.addActionListener(this);
	c8.addActionListener(this);
	    
	    
	    
	choice.add(l1);    
	choice.add(new JLabel(" "));    
	choice.add(new JLabel(" "));    
	choice.add(c1);
	choice.add(c2);
	choice.add(c3);
	choice.add(c4);
	choice.add(c5);
	choice.add(c6);

	choice.add(new JLabel(" "));    
	    
	leftPane.add(choice,BorderLayout.NORTH);
	   
	treePanel=new TreePanel(files,data,text,exec);
	JPanel legendPanel = new JPanel();
	legendPanel.setBorder(legendTitle);
	ImageIcon i1 = new ImageIcon("gifs/cr.gif");
	ImageIcon i2 = new ImageIcon("gifs/md.gif");
	ImageIcon i3 = new ImageIcon("gifs/de.gif");
	ImageIcon i4 = new ImageIcon("gifs/pc.gif");
	ImageIcon i5 = new ImageIcon("gifs/og.gif");
	ImageIcon i6 = new ImageIcon("gifs/bi.gif");
	ImageIcon i7 = new ImageIcon("gifs/li.gif");
	    
	JLabel li1 = new JLabel("- Created",i1,JLabel.CENTER);
	JLabel li2 = new JLabel("- Modified",i2,JLabel.CENTER);
	JLabel li3 = new JLabel("- Deleted",i3,JLabel.CENTER);
	JLabel li4 = new JLabel("- Permission Changed ",i4,JLabel.CENTER);
	JLabel li5 = new JLabel("- Owner/Group Changed",i5,JLabel.CENTER);
	JLabel li6 = new JLabel("- Binaries",i6,JLabel.CENTER);
	JLabel li7 = new JLabel("- Libraries",i7,JLabel.CENTER);
	    
	li1.setFont(new Font("Arial",Font.PLAIN,12));
	li2.setFont(new Font("Arial",Font.PLAIN,12));
	li3.setFont(new Font("Arial",Font.PLAIN,12));
	li4.setFont(new Font("Arial",Font.PLAIN,12));
	li5.setFont(new Font("Arial",Font.PLAIN,12));
	li6.setFont(new Font("Arial",Font.PLAIN,12));
	li7.setFont(new Font("Arial",Font.PLAIN,12));
	legendPanel.add(li1);
	legendPanel.add(li2);
	legendPanel.add(li3);
	legendPanel.add(li4);
	legendPanel.add(li5);
	legendPanel.add(li6); 
	legendPanel.add(li7);
	    
	leftPane.add(treePanel,BorderLayout.CENTER);	    
		
	   	
	    
	base.add(leftPane,BorderLayout.CENTER);
	base.add(rightPane,BorderLayout.EAST);	    
	base.add(legendPanel,BorderLayout.SOUTH);	    
		
	setSize(Constants.APPLN_PANEL_WIDTH,Constants.APPLN_PANEL_HEIGHT); 
	setVisible(true);		
		
	Constants.TREE_NODE_HEIGHT = treePanel.tree.getRowBounds(0).height;
	Constants.MAX_NODES_FOR_DISPLAY = 
	    treePanel.getHeight() /Constants.TREE_NODE_HEIGHT;
		
				
	treePanel.displayNodes();
	
	
    }
    // funtion to split a string based on adelimiter	
    String [] stringsplit(String str,String delim)
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
	
    public void actionPerformed(ActionEvent e)
    {
	if(e.getActionCommand().equals("All") ||
           e.getActionCommand().equals("Created") ||
           e.getActionCommand().equals("Modified") ||
           e.getActionCommand().equals("Deleted") ||
           e.getActionCommand().equals("Permission Changed") ||
           e.getActionCommand().equals("Owner/Group Changed") ||
           e.getActionCommand().equals("Binaries") ||
           e.getActionCommand().equals("Libraries"))
	    {
		
      		if(e.getActionCommand().equals("All"))
		    {
       			c2.setSelected(false);
       			c3.setSelected(false);
       			c4.setSelected(false);
       			c5.setSelected(false);
       			c6.setSelected(false);
       			c7.setSelected(false);
       			c8.setSelected(false);
		    }
       		else
		    {
       			c1.setSelected(false);
		    }

		try
		    {
			Document doc = text.getDocument();		    
				
			doc.remove(0,doc.getLength());
		    }	
		catch(BadLocationException exp)
		    {
			exp.printStackTrace();
		    }
		exec.setVisible(false);
		int flag=0;
		files.removeAllElements();
		for(int i=0;i<Data.size();i++)
		    {
			String arr[]=(String [])Data.elementAt(i);
	
			if( !c1.isSelected() &&
			    !c2.isSelected() &&
			    !c3.isSelected() &&
			    !c4.isSelected() &&
			    !c5.isSelected() &&
			    !c6.isSelected() &&
			    !c7.isSelected() &&
			    !c8.isSelected())
			    {
				flag=1;
				break;
			    }
					
					
				
			if(c1.isSelected())
			    files.add(arr[1]);
			else
			    {
				if(c2.isSelected() && arr[2].equals("CR"))
				    files.add(arr[1]);
				else if(c3.isSelected() && arr[2].equals("MD"))
				    files.add(arr[1]);
				else if(c4.isSelected() && arr[2].equals("DE"))
				    files.add(arr[1]);
				else if(c5.isSelected() && arr[2].equals("PC"))
				    files.add(arr[1]);
				else if(c6.isSelected() && arr[2].equals("OG"))
				    files.add(arr[1]);
				else if(c7.isSelected() && arr[2].equals("BI"))
				    files.add(arr[1]);
				else if(c8.isSelected() && arr[2].equals("LI"))
				    files.add(arr[1]);
			    }
		    }
			

		JPanel p=(JPanel)base.getComponent(0);
		JPanel p1=(JPanel)p.getComponent(1);
		JScrollPane p2=(JScrollPane)p1.getComponent(0);
		if(p2.getComponentCount()!=0)
		    {
		    	p2.remove(0);
		    	p2.invalidate();
	    		p2.validate();
	    		p2.repaint();
		    }

		if(flag==0 && files.size()>0)
		    {
		    	p.remove(1);
		    	p.invalidate();
	    		p.validate();
	    		p.repaint();
			treePanel=new TreePanel(files,Data,text,exec);
			treePanel.displayNodes();
			p.add(treePanel,BorderLayout.CENTER);
			p.invalidate();
		    	p.validate();
		    	p.repaint();
			commit.setEnabled(true);
		    }
		else
		    {
			commit.setEnabled(false);
		    }
       			
	    }
	else if(e.getActionCommand().equals("Commit"))
	    {      	
       		int n;
       		
		n = JOptionPane.showConfirmDialog(
		       this, "Are you sure you want to Commit the changes?",
		       "Confirm", JOptionPane.YES_NO_OPTION);		
       		
		if (n == JOptionPane.YES_OPTION)
		    {
			Vector arr=getPaths(treePanel.top);
			sendFiles(arr);
				
			server.close();
			this.dispose();	       		
			System.exit(0);
		    }
		else if (n == JOptionPane.NO_OPTION)
		    {
			//do nothing				
	            }
	    }
	else if(e.getActionCommand().equals("Cancel"))
	    {
		int n = JOptionPane.showConfirmDialog(
			 this, "Are you sure you want to discard the changes?",
			 "Confirm", JOptionPane.YES_NO_OPTION); 
             
		if (n == JOptionPane.YES_OPTION)
		    {
	       		server.write("Cancel");
	       		server.read();
			server.close();    	   		
			this.dispose();	       		
			System.exit(0);
		    }
		else if (n == JOptionPane.NO_OPTION)
		    {
			//do nothing
		    }             
	    }
	else if(e.getActionCommand().equals("Execute Command"))
	    {
		try
		    {
			Document doc = text.getDocument();		    
			String props=doc.getText(0,doc.getLength());
	       		String splitstr[];				
        		splitstr = stringsplit(props,"\n");
        		file1=splitstr[1];
        		file1=file1.trim();
        		file2=getPrivPath(file1);		   
		    }	
		catch(BadLocationException exp)
		    {
			exp.printStackTrace();
		    }
		ExecuteDialog ed = new ExecuteDialog(file1,file2);
		ed.setVisible(true);
	    }
    }		
	
    String getPrivPath(String str)
    {
	for(int i=0;i<Data.size();i++)
	    {
		String arr[]=(String [])Data.elementAt(i);
		if(str.equals(arr[1]))
		    return arr[3];
	    }
	return "";
    }
	
    Vector getPaths(DefaultMutableTreeNode top)
    {		
	Vector filepaths = new Vector();
	filepaths.add("Commit");		
	for(Enumeration e=top.breadthFirstEnumeration();e.hasMoreElements();)
	    {
		DefaultMutableTreeNode node = 
		    (DefaultMutableTreeNode)e.nextElement();
		TreeNode a[]=node.getPath();
		String str="/";
		for(int j=1;j<a.length;j++)
		    str=str+a[j].toString()+"/";
		str=str.substring(0,str.length()-1); 
		filepaths.add(str);
	    }
	filepaths.add("EOF");		
	return filepaths;	
    }
	
    void sendFiles(Vector arr)
    {
	for(int i=0;i<arr.size();i++) {			
	    server.write((String)arr.elementAt(i));
	    String s=server.read();
	    if(s!=null) {
		if(s.equals("OK"))
		    continue;
		else {
		    JOptionPane.showMessageDialog(this,
			   "Unexpected Error: Discard Changes");  
		    server.close();    	   			
		    this.dispose();
		    System.exit(0);
		} 
	    }
	    else {
		JOptionPane.showMessageDialog(this,
		    "Unexpected Error: Discard Changes"); 
		server.close();    	   			
		this.dispose();
		System.exit(0);
	    }		
	}	
    }
	
	
    void sendExit()
    {
	server.write("Cancel");
	server.read();
        server.close();    	   			   		
       	this.dispose();	       		
       	System.exit(0);
    }
}
