
class Results {
    string prev_link, next_link;
	string base_url;
	string current_page;
	string page;
	string title;
	string url;
	bool refresh;
	bool error;
	
	GtkListStore *iconViewStore;
	
	string index; // save position of iconView
	public:
	
	Results() {
		refresh = FALSE;
		error = FALSE;
	}
	
	bool isError() {
		return error;
	}
	
	void setError(bool e) {
		error = e;
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
	
	void createNewModel() {
		iconViewStore = gtk_list_store_new(
		     ICON_NUM_COLS,   // Number of columns
		     GDK_TYPE_PIXBUF, // Image poster
		     G_TYPE_STRING,   // Title
		     G_TYPE_STRING,   // Href
		     G_TYPE_STRING    // Image link
		);
		
		setModel();
		
		// Want to keep my copy of model
		//g_object_unref(iconViewStore);
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
	
	void show(string p) {
	    page = p;
	    parse_results();
	    parse_pager();         	
	}
	
	void setBaseUrl(string bu) {
	    base_url = bu;
	}
	
	string& getBaseUrl() {
		return base_url;
	}
	
	string& getPrevLink() {
		return prev_link;
	}
	
	string& getNextLink() {
		return next_link;
	}
	
	string getCurrentPage() {
		return current_page;
	}
	
	void setModel() {
		gtk_icon_view_set_model(
		    GTK_ICON_VIEW(ivResults),
		    GTK_TREE_MODEL(iconViewStore)
		);
	}
	
	private:
	
	void appendToStore(string title, string href, string image) {
		static GtkTreeIter iter;
		static GdkPixbuf *pixbuf;
		
		gtk_list_store_append(iconViewStore, &iter);
		
		if(imagesCache.count(image) > 0) {
			pixbuf = imagesCache[image];
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
	}
	
	//Parse search results
	void parse_results() {
	    string begin = "<div class=\"custom-poster\"";
		string end = "</a>";
		size_t div_begin = page.find(begin);
		size_t div_end = page.find(end, div_begin+1);
		while(div_end != string::npos && div_begin != string::npos) {
			size_t div_length = div_end - div_begin + end.length(); 
			string div = page.substr(div_begin, div_length);
			//cout << "Div: " << div << " Div END" << endl;
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
				//cout << "Title: " << title << endl;
				
				//Find href
				size_t href_begin = div.find("href=");
				size_t href_end = div.find(".html", href_begin + 1);
				if(href_begin != string::npos && href_end != string::npos) {
					size_t href_length = href_end - href_begin; 
					string href = div.substr(href_begin+6, href_length-1);
					//cout << "Href: " << href << endl;
					//Find image
					size_t image_begin = div.find("src=");
					size_t image_end = div.find(".jpg", image_begin + 1);
					if(image_begin != string::npos && image_end != string::npos) {
						size_t image_length = image_end - image_begin;
						string image = div.substr(image_begin+5, image_length-1);
						unescape_html(title);
					    appendToStore(to_utf8(title),
					                  href,
					                  image + "&w=165&h=236&zc=1");
					                  //       ^^^^^^^^^^^^   set size of image
					}
				}
			}
			
			div_begin = page.find(begin, div_end+1);
		    div_end = page.find(end, div_begin+1);
		}
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
			
			if(title == "Назад") {
				prev_link = link;
			}
		}
	}
	
	void parse_pager() {
		prev_link = next_link = current_page = "";
		size_t pager_begin = page.find("class=\"navigation\"");
		size_t pager_end = page.find("</div>", pager_begin+1);
		if(pager_end != string::npos && pager_begin != string::npos) {
			size_t pager_length = pager_end - pager_begin;
			string pager = page.substr(pager_begin+2, pager_length-2);
	        
	        // Find spans
	        string begin_span = "<span>";
	        string end_span = "</span>";
	        size_t span_begin = pager.find(begin_span);
	        size_t span_end = pager.find(end_span);
	        int count_span = 0;
	        string spans[2];
	        int pages[2];
	        while(span_end != string::npos && span_begin != string::npos) {
				size_t span_length = span_end - span_begin;
				spans[count_span] = pager.substr(span_begin+6, span_length-6);
				pages[count_span] = atoi(spans[count_span].c_str());
				count_span++;
				span_begin = pager.find(begin_span, span_end);
	            span_end = pager.find(end_span, span_begin);
			}
			current_page = "";
			if(count_span == 1) {
				current_page = spans[0];
			}else {
				if(pages[0] != 0) {
					current_page = spans[0];
				}
				if(pages[1] != 0) {
					current_page = spans[1];
				}
			}
	        
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
					
					if(title == "Назад") {
						prev_link = base_url + "&search_start=" + page_num;
					}
				}
				
			    anchor_begin = pager.find(begin, anchor_end);
	            anchor_end = pager.find(end, anchor_begin);
			}
		}
	}
};
