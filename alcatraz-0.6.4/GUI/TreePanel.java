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
import javax.swing.event.*;
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.text.*;
import java.net.*;
import java.io.*;
import javax.swing.border.*;
import javax.swing.text.*;

//Class for Optimized Tree display

public class TreePanel extends JPanel
{
    Vector filePaths;
    static int LENGTH;
    public Collator sortCollator;
    DefaultMutableTreeNode top;
    final JTree tree;

    //The greatest index value corresponds to highest priority
    final String[] PriorityList = { "dev","usr","home","etc","root"}; 
    int count=1;
    JTable table;
    Vector columnNames;
    Vector rowData;
    SimpleAttributeSet boldBlackText = new SimpleAttributeSet();
    SimpleAttributeSet plainDarkBlueText = new SimpleAttributeSet();
    JTextPane Text;
    Vector Data;
    JButton exec;

    //constructor
    public TreePanel(Vector v,Vector data,JTextPane text,JButton exc)	 
    {
	exec=exc;
	Data=new Vector();
	Data=data;
	Text=new JTextPane();
	Text=text;
	sortCollator = Collator.getInstance(new Locale("en", "US"));

    	//Create the top node.

        top = new DefaultMutableTreeNode("/");

        //Create a tree that allows one selection at a time.
        tree = new JTree(top);

       
        tree.getSelectionModel().setSelectionMode
	    (TreeSelectionModel.SINGLE_TREE_SELECTION);

	StyleConstants.setForeground(boldBlackText, Color.black);
	StyleConstants.setBold(boldBlackText, true);
	StyleConstants.setFontFamily(boldBlackText, "Arial");
	StyleConstants.setFontSize(boldBlackText, 13);
		
		
	StyleConstants.setForeground(plainDarkBlueText, new Color(0,0,128));
	StyleConstants.setFontFamily(plainDarkBlueText, "Arial");
	StyleConstants.setFontSize(plainDarkBlueText, 13);
		
		


        //Listen for when the selection changes.
       	tree.addTreeSelectionListener(new TreeSelectionListener() {

		public void valueChanged(TreeSelectionEvent e)
		{
		    DefaultMutableTreeNode node =
			(DefaultMutableTreeNode)tree.getLastSelectedPathComponent();
			
		    if (node == null) return;
      
		    TreeNode a[]=node.getPath();
		    String str="/";
		    for(int i=1;i<a.length;i++)
			str=str+a[i].toString()+"/";
		    str=str.substring(0,str.length()-1);
				
		    int indx=getMatchingIndex(str);
				
		    if(indx!=-1) {	
			String arr[]=(String [])Data.elementAt(indx);
			if(arr[2].equals("MD") || arr[2].equals("CR"))
			    exec.setVisible(true);
			else
			    exec.setVisible(false);
			
			try {
			    Document doc = Text.getDocument();
			    doc.remove(0,doc.getLength());
			    doc.insertString(doc.getLength(), " Path:\n", 
					     boldBlackText);
			    doc.insertString(doc.getLength(), "   "+arr[1]+
					     "\n\n", plainDarkBlueText);
					    
			    doc.insertString(doc.getLength(), " File Type:\n", 
					     boldBlackText);
			    if(!arr[5].equals("F"))
				doc.insertString(doc.getLength(),"  "+ 
					"Directory"+"\n\n", plainDarkBlueText);
				else
				    doc.insertString(doc.getLength(), "  "+ 
					"File"+"\n\n", plainDarkBlueText);
			    
			}
			catch(BadLocationException exp) {
			    exp.printStackTrace();
			}
		    }
		    else {
			try {
			    Document doc = Text.getDocument();
			    
			    doc.remove(0,doc.getLength());
			}
			catch(BadLocationException exp) {
			    exp.printStackTrace();
			}
			exec.setVisible(false);
		    }
		}
	    });


    	JScrollPane treeView = new JScrollPane(tree);
	
        treeView.setMinimumSize(new Dimension(416,370));
        treeView.setBackground(Color.white);
	
        this.setLayout(new BorderLayout());
        this.add(treeView,BorderLayout.CENTER);

	//Vector to hold the strings(filepaths)
	filePaths = new Vector();
	filePaths = v;

	//sort the array of strings using quicksort
	quickSortStrings(filePaths,0,filePaths.size()-1);

	for(int i=0;i<filePaths.size()-1;i++) {
	    for(int j=i+1;j<filePaths.size();j++) {
		String s1 = (String)filePaths.elementAt(i);
		String s2 = (String)filePaths.elementAt(j);
		String ft1 = getFileType(s1);
		String ft2 = getFileType(s2);			
		int ind1  = s1.lastIndexOf(47);
		int ind2  = s2.lastIndexOf(47);
		String subs1 = s1.substring(0,ind1);
		String subs2 = s2.substring(0,ind2);
		
		if(subs1.equals(subs2)) {
		    if(!ft1.equals(ft2) && ft1.equals("F")) {
			filePaths.setElementAt(s2,i);
			filePaths.setElementAt(s1,j);		
		    }				   	
		}		
	    }
	    
	}

	//first call ... returns vector of DefaultMutableTreeNode
	Vector V = findChildren(filePaths);
	
	for(int i=0; i<V.size() ; i++) {
	    //add all nodes that have children first
	    DefaultMutableTreeNode node=(DefaultMutableTreeNode)V.elementAt(i);
	    if(!node.isLeaf()) {
		String str=node.toString();
		int lastindex=str.lastIndexOf(47);
		str=str.substring(1,str.length());
		node.setUserObject(str);
		top.add(node);
	    }
	}
	for(int i=0; i<V.size() ; i++) {
	    //add all the leaves
	    DefaultMutableTreeNode node=(DefaultMutableTreeNode)V.elementAt(i);
	    if(node.isLeaf()) {
		String str=node.toString();
		int lastindex=str.lastIndexOf(47);
		str=str.substring(1,str.length());
		node.setUserObject(str);
		top.add(node);
	    }
	}
	
	tree.setCellRenderer(new MyRenderer(data));
    }
    
    void displayNodes()
    {      
	tree.expandPath(new TreePath(top.getPath()));   
    }
	
    String getFileType(String str)
    {
	for(int i=0;i<Data.size();i++) {
	    String arr[]=(String [])Data.elementAt(i);
	    if(str.equals(arr[1]))
		return arr[4];
	}
	return "";
    }

    int getMatchingIndex(String str)
    {
	for(int i=0;i<Data.size();i++) {
	    String arr[]=(String [])Data.elementAt(i);
	    if(str.equals(arr[1]))
		return i;
	}
	return -1;
    }
    
    //recursive function returns vector of DefaultMutableTreeNode Objects
    Vector findChildren(Vector Strings)
    {
	//end condition --- if all strings in the vector are leaves ie
	// in '/xyz' form with no other "/" in the string

	int index=0;
	int flag=0;
	Vector v = new Vector();
	if(Strings.size()==0)
	    return null;
	do {
	    String s = (String)Strings.elementAt(index);
	    int i = s.lastIndexOf(47);			 // index of "/"
	    
	    if(i!=0) {
		flag=1;		   // if there is any other "/" other than the
		break; 		   // first one
	    }

	    index++;
	} while (index < Strings.size());

	if(flag==0) {              // end condition satisfied. 
		for(int i=0; i<Strings.size(); i++)
		    {
			String s=(String)Strings.elementAt(i);
			DefaultMutableTreeNode node = 
			    new DefaultMutableTreeNode(s);
			v.add(node);
		    }
	    }
	else {
	    //get the common roots and assign a node to each
	    Vector Common = getCommons(Strings);
	    for(int i=0; i<Common.size(); i++) {
		int flg=0;
		CommonStringAndIndexes com =
		    (CommonStringAndIndexes)Common.elementAt(i);
		DefaultMutableTreeNode node =
		    new DefaultMutableTreeNode(com.str);
		
		Vector newStrings = new Vector();
		
		//build next input of Strings as a vector
		for(int j=com.stindex;j<=com.endindex;j++) {
		    String s1 = (String)Strings.elementAt(j);
		    String s2 = com.str;
		    if(s1.equals(s2)) {
			flg=1;
			break;
		    }
		    s1 = s1.substring(s2.length(),s1.length());
		    newStrings.addElement(s1);
		}
		
		if(flg==0) {
		    Vector newV = new Vector();
		    
		    // recursive call
		    newV = findChildren(newStrings);
		    for(int j=0;j< newV.size();j++) {
			//add all nodes which have children first
			DefaultMutableTreeNode newnode
			    = (DefaultMutableTreeNode)newV.elementAt(j);
			if(!newnode.isLeaf()) {
			    String s=newnode.toString();
			    int lastindex=s.lastIndexOf(47);
			    s=s.substring(1,s.length());
			    newnode.setUserObject(s);
			    node.add(newnode);
			}
		    }
		    for(int j=0; j<newV.size() ; j++) {
			//then add the leaves which are directories
			DefaultMutableTreeNode newnode=
			    (DefaultMutableTreeNode)newV.elementAt(j); 

			if(newnode.isLeaf()) {
			    String s=newnode.toString();
			    int lastindex=s.lastIndexOf(47);
			    s=s.substring(1,s.length());
			    newnode.setUserObject(s);
			    node.add(newnode);
			}
		    }
					
		}
		v.addElement(node);
	    }
	}
	return v;
    }

    //function to get common roots(prefixes) given a vector of strings
    Vector getCommons(Vector Strings)
    {
	int index=0,stindex=0,endindex=0,lastindex=0;
	Vector v = new Vector();
	String s,prevs;
	while(index < Strings.size()) {
	    s = (String)Strings.elementAt(index);
	    prevs = s;
	    stindex = index;

	    while(!s.equals("")) {
		if((index+1)==Strings.size())
		    break;

		int len=s.length();
		String nexts=(String)Strings.elementAt(index+1);
		if(len >= nexts.length()) {
		    lastindex=s.lastIndexOf(47);
		    s=s.substring(0,lastindex);
		    continue;
		}
		else {
		    String substr=nexts.substring(0,len);
		    if(nexts.charAt(len) == '/') {
			if(s.equals(substr)) {
			    prevs=s;
			    index++;
			    continue;
			}
			else {
			    lastindex=s.lastIndexOf(47);
			    s=s.substring(0,lastindex);
			    continue;
			}
			
		    }
		    else {
			lastindex=s.lastIndexOf(47);
			s=s.substring(0,lastindex);
			continue;
		    }
		}
	    }
	    endindex=index;
	    /* to take care of the directory itself appearing in the list */
	    if(prevs.equals((String)Strings.elementAt(stindex)) 
	       && (stindex!=endindex))			
		stindex++;

	    CommonStringAndIndexes comstr =
		new CommonStringAndIndexes(prevs,stindex,endindex);
	    v.add(comstr);
	    index++;
	}
	return v;
    }

    //recursive qsort
    public void quickSortStrings(Vector A,int p,int r)
    {
	int q;
    	if(p<r) {
	    q = quickPartition(A,p,r);
	    quickSortStrings(A,p,q);
	    quickSortStrings(A,q+1,r);
	}
    }


    public int quickPartition(Vector A,int p,int r)
    {
	String x = (String) A.elementAt(p);
    	int i = p-1;
    	int j = r+1;
    	while(true) {
	    do {
		j--;
	    } while(compare((String)A.elementAt(j),x)>0);

	    do {
		i++;
	    } while(compare((String)A.elementAt(i),x)<0);

	    if(i<j)
		swap(A,i,j);
	    else
		return j;
	}
    }
    

    int compare(String a,String b)
    {
	int pa=priority(a);
	int pb=priority(b);
	String fta=getFileType(a);
	String ftb=getFileType(b);
				

	if(pa > pb)
	    return -1;
	else if(pa < pb)
	    return 1;
	else {
	    if(pa==-1 || pb==-1)
		return(sortCollator.compare(a,b));
	    else {
		String x=a.substring(1,a.length());
		x=x.substring(x.indexOf(47)+1,x.length());
		String y=b.substring(1,b.length());
		y=y.substring(y.indexOf(47)+1,y.length());
		return(sortCollator.compare(x,y));
	    }
	}
    }
    

    int priority(String a)
    {
	a=a.substring(1,a.length());
	if(a.indexOf(47)!=-1)
	    a=a.substring(0,a.indexOf(47));
	for(int i=0;i<PriorityList.length;i++)
	    if(a.equals(PriorityList[i]))
		return i;
	return -1;
    }

    //swap
    private void swap(Vector v, int i, int j)
    {
	String T;
	T = (String)v.elementAt(i);			 // T    = a[i]
    	v.setElementAt(v.elementAt(j),i);		 // a[i] = a[j]
    	v.setElementAt(T,j);				 // a]j] = T
    }


	
}

// class to hold one common root(prefix) and the indexes between which it lies
// ie it captures the span of common prefix(specifying that a particular prefix
// occurs in files from "startindex" to "endindex" in the vector containing 
// file names).
class CommonStringAndIndexes
{
    String str;
    int stindex;
    int endindex;

    public CommonStringAndIndexes(String s,int st,int end)
    {
	str = s;
	stindex=st;
	endindex=end;
    }
}
