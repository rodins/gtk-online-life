//Categories.hpp
#include "CategoryItem.hpp"

class Categories {
	GdkPixbuf *categoryIcon;
	GdkPixbuf *itemIcon;
	
	GtkTreeStore *treestore;
	GtkTreeIter topLevel, child;
	
	string domain;
	int count;
	public:
	
	Categories() {
		count = 0;
	}
	
	void init() {
		categoryIcon = create_pixbuf("folder_16.png");
	    itemIcon = create_pixbuf("link_16.png");
	    
	    treestore = gtk_tree_store_new(
	              TREE_NUM_COLS, 
	              GDK_TYPE_PIXBUF,
				  G_TYPE_STRING, 
				  G_TYPE_STRING);
				  
	    domain = "http://www.online-life.club"; 
	}
	
	int getCount() {
		return count;
	}
	
	string getTitle() {
		return string("Online Life - Categories");
	}
	
	GtkTreeModel *getModel() {
		return GTK_TREE_MODEL(treestore);		
	}
	
	void parse_categories(string page) {
		size_t nav_begin = page.find("<div class=\"nav\">");
		size_t nav_end = page.find("</div>", nav_begin+1);
		if(nav_begin != string::npos && nav_end != string::npos) {
			size_t nav_length = nav_end - nav_begin + 6; 
			string nav = page.substr(nav_begin, nav_length);
			
			// Add to top level
			addToTopLevel("Главная", domain);
			
			// Find nodrop items
			// Find new, popular, best
			size_t nodrop_begin = nav.find("<li class=\"pull-right");
			size_t nodrop_end = nav.find("</li>", nodrop_begin+1);
			while(nodrop_begin != string::npos && nodrop_end != string::npos) {
				size_t nodrop_length = nodrop_end - nodrop_begin;
				string nodrop = nav.substr(nodrop_begin, nodrop_length);
				
				// Add to child
				parseAnchorToChild(nodrop);
				
				// Increment
				nodrop_begin = nav.find("<li class=\"pull-right", nodrop_end+1);
			    nodrop_end = nav.find("</li>", nodrop_begin+1);
			}
			// Find trailer
			size_t trailer_begin = nav.find("<li class=\"nodrop\" ");
			size_t trailer_end = nav.find("</a>", trailer_begin+1);
			if(trailer_begin != string::npos && trailer_end != string::npos) {
			    size_t trailer_length = trailer_end - trailer_begin;
			    string trailer = nav.substr(trailer_begin, trailer_length+4);
			    
			    // Add to child
			    parseAnchorToChild(trailer);	
			}
			
			// Find drop items
			size_t drop_begin = nav.find("<li class=\"drop\">");
			size_t drop_end = nav.find("</ul>", drop_begin+1);
			while(drop_begin != string::npos && nav_end != string::npos) {
				size_t drop_length = drop_end - drop_begin;
				string drop = nav.substr(drop_begin, drop_length);
				string begin = "<a href=";
			    string end = "</a>";
				size_t anchor_begin = drop.find(begin);
				size_t anchor_end = drop.find(end, anchor_begin+1);
				bool first = true;
				while(anchor_begin != string::npos && anchor_end != string::npos) {
					size_t anchor_length = anchor_end - anchor_begin;
					string anchor = drop.substr(anchor_begin, anchor_length);
					
					size_t link_begin = anchor.find("\"");
					size_t link_end = anchor.find("\"", link_begin+1);
					if(link_begin != string::npos && link_end != string::npos) {
						size_t link_length = link_end - link_begin;
						string link = anchor.substr(link_begin+1, link_length-1);
						
						size_t title_begin = anchor.find(">");
						if(title_begin != string::npos) {
							string title = anchor.substr(title_begin+1);
							if(first) {
								// Add to top level
				                addToTopLevel(to_utf8(title), addDomainTo(link));
				
								first = false;
							}else {
								// Add to child
								addToChild(to_utf8(title), addDomainTo(link));
							}
						}
					}
					// increment
					anchor_begin = drop.find(begin, anchor_end+1);
			        anchor_end = drop.find(end, anchor_begin+1);
				}
				
				drop_begin = nav.find("<li class=\"drop\">", drop_end+1);
			    drop_end = nav.find("</ul>", drop_begin+1);
			}
		}
	}
	
	private:
	
	void addToChild(string title, string link) {
		gtk_tree_store_append(treestore, &child, &topLevel);
		gtk_tree_store_set(treestore,
		                   &child,
		                   TREE_IMAGE_COLUMN, 
		                   itemIcon, 
		                   TREE_TITLE_COLUMN, 
		                   title.c_str(),
		                   TREE_HREF_COLUMN,
		                   link.c_str(),
		                   -1);
		count++;
	}
	
	void addToTopLevel(string title, string link) {
		gtk_tree_store_append(treestore, &topLevel, NULL);
		gtk_tree_store_set(treestore,
		                   &topLevel,
		                   TREE_IMAGE_COLUMN,
		                   categoryIcon,
		                   TREE_TITLE_COLUMN,
		                   title.c_str(),
		                   TREE_HREF_COLUMN,
		                   link.c_str(),
		                    -1);
	}
	
	void parseAnchorToChild(string anchor) {
		// Parse link
		size_t link_begin = anchor.find("<a href=");
		size_t link_end = anchor.find("\"", link_begin+10);
		if(link_begin != string::npos && link_end != string::npos) {
			size_t link_length = link_end - link_begin;
			string link = anchor.substr(link_begin+9, link_length-9);
			// Parse title
			size_t title_begin = anchor.find(">", link_end);
			size_t title_end = anchor.find("</a>");
			if(title_end != string::npos) {
				size_t title_length = title_end - title_begin;
				string title = anchor.substr(title_begin+1, title_length-1);
				addToChild(to_utf8(title), addDomainTo(link));
			}
		}
	}
	
	string addDomainTo(string link) {
		if(link.find(domain) == string::npos) {
			return domain + link;
		}else {
			return link;
		}
	}
};
