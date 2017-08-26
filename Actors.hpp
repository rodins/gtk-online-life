//Actors.hpp

class Actors {
    string actorsTitle;
    
    GtkListStore *store;
    GtkTreeIter iter;
    
    GdkPixbuf *item;
    
    int count;
    
    string url;
    public:
    
    Actors() {
	    count = 0;	
	}
	
	void setUrl(string u) {
		url = u;
	}
	
	string getUrl() {
		return url;
	}
    
	GtkTreeModel *getModel() {
		return GTK_TREE_MODEL(store);
	}
	
	int getCount() {
		return count;
	}
    
    void setTitle(string t) {
		actorsTitle = t;
	}
	
	string& getTitle() {
		return actorsTitle;
	}
    
    void parse(string &page) {
		count = 0;
		item = create_pixbuf("link_16.png");
		store = gtk_list_store_new(TREE_NUM_COLS, 
                                   GDK_TYPE_PIXBUF,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING);
		
		page = to_utf8(page);
		string year = parse_simple_info(page, "Год: ");
		string country = parse_simple_info(page, "Страна: ");
		actorsTitle = actorsTitle + " - " + year + " - " + country;
		parse_info(page, "Режиссер:", " (режиссер)");
		parse_info(page, "В ролях:", "");
	}
	
	private:
	
	void addToStore(string title, string link) {
		gtk_list_store_append(store, &iter);
        gtk_list_store_set(store,
                           &iter,
                           TREE_IMAGE_COLUMN,
                           item,
                           TREE_TITLE_COLUMN, 
                           title.c_str(),
                           TREE_HREF_COLUMN,
                           link.c_str(),
                           -1);
        count++;
	}
	
	string parse_simple_info(string &page, string query) {
		string end = "\n";
		size_t info_begin = page.find(query);
		size_t info_end = page.find(end, info_begin);
		if(info_begin != string::npos && info_end != string::npos) {
			size_t info_length = info_end - info_begin - query.size();
			string info = page.substr(info_begin + query.size(), info_length);
			return info;
		}
		return "";
	}
	
	void parse_info(string page, string query, string director) {
		string begin = query; // "В ролях:";
		string end = "</p>";
		size_t actors_begin = page.find(begin);
		size_t actors_end = page.find(end, actors_begin+1);
		if(actors_begin != string::npos && actors_end != string::npos) {
			size_t actors_length = actors_end - actors_begin;
			string actors = page.substr(actors_begin, actors_length);
			//cout << actors << endl;
			
			string begin = "<a href";
		    string end = "</a>";
	        size_t anchor_begin = actors.find(begin);
	        size_t anchor_end = actors.find(end, anchor_begin+1);
	        string anchor;
	        while(anchor_end != string::npos && anchor_begin != string::npos) {
				size_t anchor_length = anchor_end - anchor_begin;
				anchor = actors.substr(anchor_begin, anchor_length);
				//cout << anchor << endl;
				
				size_t title_begin = anchor.find(">");
				if(title_begin != string::npos) {
					string title = anchor.substr(title_begin+1);
					//cout << title << endl;
					unescape_html(title);
					//Find href
					size_t href_begin = anchor.find("href=");
					size_t href_end = anchor.find(">", href_begin + 1);
					if(href_begin != string::npos && href_end != string::npos) {
						size_t href_length = href_end - href_begin; 
						string href = anchor.substr(href_begin+6, href_length-7);
						//cout << href << endl;
						
					    addToStore(title + director, href);
					}
				}
				
				anchor_begin = actors.find(begin, anchor_end);
	            anchor_end = actors.find(end, anchor_begin);
			}
		}
	}
};
