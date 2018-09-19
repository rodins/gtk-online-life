// ResultsModel.hpp

class ResultsModel {
	int id;
    string next_link;
	string base_url;
	string title;
	string url;
	bool refresh;
	
	GtkListStore *iconViewStore;
	GdkPixbuf *defaultPixbuf;
	// TODO: try to make imagesCache non pointer, use links
	map<string, GdkPixbuf*> *imagesCache;
	string index; // save position of iconView
	
	int count;
	public:
	
	ResultsModel() {
		count = 0;
		id = -1;
		// Initialize default pixbuf for ivResults here
        defaultPixbuf = IconsFactory::getBlankIcon();
        refresh = FALSE;
        iconViewStore = NULL;
	}
	
	int getId() {
		return id;
	}
	
	void setImagesCache(map<string, GdkPixbuf*> *imagesCache) {
		this->imagesCache = imagesCache;
	}
	
	void init(int id,
	          string title, 
	          string url) {
		this->id = id;
		this->title = title;
		this->url = url;
		this->base_url = url;
		refresh = FALSE;
        count = 0;
        
        iconViewStore = gtk_list_store_new(
		     ICON_NUM_COLS,   // Number of columns
		     GDK_TYPE_PIXBUF, // Image poster
		     G_TYPE_STRING,   // Title
		     G_TYPE_STRING,   // Href
		     G_TYPE_STRING    // Image link
		);
		
	    next_link = "";
	}
	
	bool isRefresh() {
		return refresh;
	}
	
	void setRefresh(bool r) {
		refresh = r;
	}
	
	string getPosition() {
		return index;
	}
	
	void setPosition(string i) {
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
	
	GtkTreeModel *getTreeModel() {
		return GTK_TREE_MODEL(iconViewStore);
	}
	
	bool isEmpty() {
		return count == 0;
	}
	
	void setNextLink(string next_link) {
		this->next_link = next_link;
	}
	
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
	}
};
