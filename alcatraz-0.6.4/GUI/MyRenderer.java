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
import javax.swing.tree.*;
import java.awt.*;
import java.util.*;
import java.awt.event.*;
import java.text.*;
import javax.swing.text.*;

// Tree Cell Rendered to show file status using images
class MyRenderer extends JPanel implements TreeCellRenderer
{
    ImageIcon created, modified, deleted, permchanged, owgrpchanged, 
	binaries, libraries;
    ImageIcon smile, dir, dircreated, dirmodified, dirdeleted, dirpermchanged,
	dirowgrpchanged, dirbinaries, dirlibraries;
    HashMap eventModStatusTable;
    HashMap nodeIcons;   
    JLabel labelText;
    Vector data;

    public MyRenderer(Vector v)
    {
    	data = new Vector();
    	data = v;    	
    	this.eventModStatusTable=eventModStatusTable;    	
    	nodeIcons = new HashMap();
	nodeIcons.put("CR",new ImageIcon( "gifs/cr.gif" ));
	nodeIcons.put("MD",new ImageIcon( "gifs/md.gif" ));
	nodeIcons.put("DE",new ImageIcon( "gifs/de.gif" ));
	nodeIcons.put("PC",new ImageIcon( "gifs/pc.gif" ));
	nodeIcons.put("OG",new ImageIcon( "gifs/og.gif" ));    
	nodeIcons.put("BI",new ImageIcon( "gifs/bi.gif" ));
	nodeIcons.put("LI",new ImageIcon( "gifs/li.gif" ));    
	nodeIcons.put("DIR",new ImageIcon( "gifs/dir.gif" ));    
	dir = new ImageIcon( "gifs/dir.gif" );
    }
	
    public Component getTreeCellRendererComponent(
						  JTree tree,
						  Object value,
						  boolean sel,
						  boolean expanded,
						  boolean leaf,
						  int row,
						  boolean hasFocus)
    {

	removeAll();
	invalidate();
	validate();
	repaint();
			
	//this.add(new JLabel("Hi"));
	DefaultMutableTreeNode node = (DefaultMutableTreeNode)value;
        if (leaf) {
	    TreeNode arr[]=node.getPath();
	    String str="/";
	    for(int i=1;i<arr.length;i++)
		str=str+arr[i].toString()+"/";
	    str=str.substring(0,str.length()-1);
	    
	    String status = getStatus(str);	
	    String fileType = getFileType(str);	
	    if(fileType.equals("D"))
		this.add(new JLabel((ImageIcon)nodeIcons.get("DIR"),
				    SwingConstants.CENTER));	    
	    this.add(new JLabel((ImageIcon)nodeIcons.get(status),
				SwingConstants.CENTER));
	    JLabel nodeText = new JLabel(node.toString());
	    nodeText.setFont(new Font("Arial",Font.PLAIN,12));
	    if(sel) {
		nodeText.setForeground(Color.red);
		this.setBackground(new Color(249,246,229));
	    }
	    else {
		nodeText.setForeground(new Color(0,0,128));
		this.setBackground(Color.white);
	    }
	    this.add(nodeText);
	}         
        else {
	    TreeNode arr[]=node.getPath();
	    String str="/";
	    for(int i=1;i<arr.length;i++)
		str=str+arr[i].toString()+"/";
	    str=str.substring(0,str.length()-1);
	    
	    this.add(new JLabel(dir,SwingConstants.CENTER));
	    String status = getStatus(str);	
	    this.add(new JLabel((ImageIcon)nodeIcons.get(status),
				SwingConstants.CENTER));
	    
	    JLabel nodeText;
	    if(!expanded)
		nodeText = new JLabel(node.toString()+"("+
				      node.getChildCount()+")");
	    else
		nodeText = new JLabel(node.toString());
	    
	    nodeText.setFont(new Font("Arial",Font.PLAIN,12));
	    if(sel) {
		nodeText.setForeground(Color.red);
		this.setBackground(new Color(249,246,229));
	    }
	    else {
		nodeText.setForeground(new Color(0,0,128));
		this.setBackground(Color.white);
	    }
	    this.add(nodeText);
	}      		
	
	invalidate();
	validate();
	repaint();

        return this;
    } 
    
    String getStatus(String str)
    {
	for(int i=0;i<data.size();i++)
	    {
		String arr[]=(String [])data.elementAt(i);
		if(str.equals(arr[1]))
		    return arr[2];
	    }
	return "";
    }
	
	
    String getFileType(String str)
    {
	for(int i=0;i<data.size();i++)
	    {
		String arr[]=(String [])data.elementAt(i);
		if(str.equals(arr[1]))
		    return arr[4];
	    }
	return "";
    }

}

