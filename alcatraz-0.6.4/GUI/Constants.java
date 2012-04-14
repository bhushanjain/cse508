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

import java.lang.Math;

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
import javax.swing.plaf.basic.*;
import javax.swing.plaf.*;


public class Constants {
    public static final int SCREEN_WIDTH  = (int)Toolkit.getDefaultToolkit().getScreenSize().getWidth();
    public static final int SCREEN_HEIGHT = (int)Toolkit.getDefaultToolkit().getScreenSize().getHeight();

    public static final int APPLN_PANEL_WIDTH   = 800;
    public static final int APPLN_PANEL_HEIGHT  = 575;
    
    public static final int PROPERTIES_PANEL_WIDTH   = 300;
    public static final int PROPERTIES_PANEL_HEIGHT  = 450;
    

    public static final int EXECUTE_DIALOG_WIDTH   = 550;
    public static final int EXECUTE_DIALOG_HEIGHT  = 140;

    public static final int STANDARD_BUTTON_HEIGHT  = 30;
    public static final int STANDARD_BUTTON_WIDTH = STANDARD_BUTTON_HEIGHT*4;
    
    public static final int SPECIAL_BUTTON_HEIGHT  = 36;

    public static final int TEXTFIELD_PADDING  = 25;
    
    //   runtime constants
    public static int TREE_NODE_HEIGHT;
    public static int MAX_NODES_FOR_DISPLAY;		
    
}

