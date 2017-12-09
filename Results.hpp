// Results.hpp

class Results {
    string next_link;
	string base_url;
	string title;
	string url;
	bool refresh;
	
	GtkListStore *iconViewStore;
	GdkPixbuf *defaultPixbuf;
	
	map<string, GdkPixbuf*> *imagesCache;
	string index; // save position of iconView
	
	int count;
	public:
	
	Results(string title, 
	        string url, 
	        map<string, GdkPixbuf*> *cache, 
	        GtkWidget *ivResults) {
		this->title = title;
		this->url = url;
		this->base_url = url;
		
		refresh = FALSE;
		
		// Initialize default pixbuf for ivResults here
        defaultPixbuf = IconsFactory::getBlankIcon();
        
        imagesCache = cache;
        
        count = 0;
        
        
        iconViewStore = gtk_list_store_new(
		     ICON_NUM_COLS,   // Number of columns
		     GDK_TYPE_PIXBUF, // Image poster
		     G_TYPE_STRING,   // Title
		     G_TYPE_STRING,   // Href
		     G_TYPE_STRING    // Image link
		);
		
		setModel(ivResults);
	}
	
	~Results(){
		g_free(iconViewStore);
	}
	
	bool isRefresh() {
		return refresh;
	}
	
	void setRefresh(bool r) {
		refresh = r;
	}
	
	string getIndex() {
		return index;
	}
	
	void setIndex(string i) {
		index = i;
	}
	
	void clearModel() {
		gtk_list_store_clear(iconViewStore);
	}
	
	void setTitle(string t) {
		title = t;
	}
	
	string getTitle() {
		return title;
	}
	
	void setUrl(string u) {
		url = u;
	}
	
	string getUrl() {
		return url;
	} 
	
	void setBaseUrl(string bu) {
	    base_url = bu;
	}
	
	string& getBaseUrl() {
		return base_url;
	}
	
	string& getNextLink() {
		return next_link;
	}
	
	void setModel(GtkWidget *ivResults) {
		gtk_icon_view_set_model(
		    GTK_ICON_VIEW(ivResults),
		    GTK_TREE_MODEL(iconViewStore)
		);
	}
	
	bool isEmpty() {
		return count == 0;
	}
	
	void parser(string div, set<string> &titles) {
		//Find title
		size_t title_begin = div.find("/>");
		size_t title_end = div.find("</a>", title_begin + 1);
		if(title_begin != string::npos && title_end != string::npos) {
			size_t title_length = title_end - title_begin - 2;
			string title = div.substr(title_begin+2, title_length);
			size_t title_new_line = title.find("\n");
			if(title_new_line != string::npos) {//delete last char if last char is new line
			    title = div.substr(title_begin+2, title_new_line + 1);
				title.erase(title.size()-1);
			}
			
			if(titles.count(to_utf8(title)) == 0) {
				titles.insert(to_utf8(title));
				
				//Find href
				size_t href_begin = div.find("href=");
				size_t href_end = div.find(".html", href_begin + 1);
				if(href_begin != string::npos && href_end != string::npos) {
					size_t href_length = href_end - href_begin; 
					string href = div.substr(href_begin+6, href_length-1);
					//cout << "Href: " << href << endl;
					//Find image
					size_t image_begin = div.find("src=");
					size_t image_end = div.find("&", image_begin + 1);
					if(image_begin != string::npos && image_end != string::npos) {
						size_t image_length = image_end - image_begin;
						string image = div.substr(image_begin+5, image_length-5);
						unescape_html(title);
						// Remove newlines from image
						size_t pos = image.find("\r\n");
						while(pos != string::npos) {
							image.erase(pos, 2);
							pos = image.find("\r\n", pos+1);
						}
					    appendToStore(to_utf8(title),
					                  href,
					                  image + "&w=165&h=236&zc=1");
					                  //       ^^^^^^^^^^^^   set size of image
					}
				}
			}
		}
	}
	
	void parse_pager(string pager) {
		next_link = "";
        //Find menu pager anchors
        // <a href="http://google.com.ua">Google</a>
        string begin = "<a href";
	    string end = "</a>";
        size_t anchor_begin = pager.find(begin);
        size_t anchor_end = pager.find(end, anchor_begin+1);
        string anchor;
        while(anchor_end != string::npos && anchor_begin != string::npos) {
			size_t anchor_length = anchor_end - anchor_begin;
			anchor = pager.substr(anchor_begin, anchor_length);
			
			parse_anchor(anchor);
			
		    anchor_begin = pager.find(begin, anchor_end);
            anchor_end = pager.find(end, anchor_begin);
		}
		
		//Find search pager anchors
	    begin = "onclick";
        anchor_begin = pager.find(begin);
        anchor_end = pager.find(end, anchor_begin+1);
        while(anchor_end != string::npos && anchor_begin != string::npos) {
			size_t anchor_length = anchor_end - anchor_begin;
			anchor = pager.substr(anchor_begin, anchor_length);
			
			size_t page_begin = anchor.find("(");
			size_t page_end = anchor.find(")", page_begin+1);
			if(page_begin != string::npos && page_end != string::npos) {
				size_t page_length = page_end - page_begin;
				string page_num = anchor.substr(page_begin+1, page_length-1);
				
				size_t title_begin = anchor.find(">");
				string title = to_utf8(anchor.substr(title_begin+1));
				
				if(title == "Вперед") {
					next_link = base_url + "&search_start=" + page_num;
				}
			}
			
		    anchor_begin = pager.find(begin, anchor_end);
            anchor_end = pager.find(end, anchor_begin);
		}
	}
	
	private:
	
	void appendToStore(string title, string href, string image) {
		static GtkTreeIter iter;
		static GdkPixbuf *pixbuf;
		
		gtk_list_store_append(iconViewStore, &iter);
		
		if(imagesCache->count(image) > 0) {
			pixbuf = imagesCache->operator[](image);
		}else {
			pixbuf = defaultPixbuf;
		}
		
        gtk_list_store_set(iconViewStore, 
                           &iter,
                           ICON_IMAGE_COLUMN, 
                           pixbuf,
                           ICON_TITLE_COLUMN, 
                           title.c_str(),
                           ICON_HREF, 
                           href.c_str(),
                           ICON_IMAGE_LINK, 
                           image.c_str(), 
                           -1);
        count++;
        //cout << count << ": " << title << endl;
	}
	
	void parse_anchor(string anchor) {
		size_t link_begin = anchor.find("=\"");
		size_t link_end = anchor.find(">", link_begin + 5);
		if(link_begin != string::npos && link_end != string::npos) {
			size_t link_length = link_end - link_begin;
			string link = anchor.substr(link_begin+2, link_length-3);
			string title = to_utf8(anchor.substr(link_end+1));
			
			if(title == "Вперед") {
				next_link = link;
			}
		}
	}
};
