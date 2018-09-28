// ActorsModel.hpp

class ActorsModel {
    GtkListStore *store;
    GtkTreeIter iter;
    GdkPixbuf *item;
    gboolean empty;
    string title, link;
    string info;
    string playerUrl;
    string js;
    public:
    ActorsModel() {
		empty = TRUE;
		item = IconsFactory::getLinkIcon();
		store = gtk_list_store_new(TREE_NUM_COLS, 
                                   GDK_TYPE_PIXBUF,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING);
	}
	
	void init(string title, string link) {
		this->title = title;
		this->info = title;
		this->link = link;
		empty = TRUE;
		gtk_list_store_clear(store);
		playerUrl = "";
		js = "";
	}
	
	GtkTreeModel* getTreeModel() {
		return GTK_TREE_MODEL(store);
	}
	
	string getTitle() {
		return title;
	}
	
	string getInfo() {
	    return info;	
	}
	
    void setInfo(string year, string country) {
		info = title + " - " + year + " - " + country;
	}
	
	string getUrl() {
		return link;
	}
	
	void setPlayerUrl(string playerUrl) {
		this->playerUrl = playerUrl;
	}
	
	string getPlayerUrl() {
		return playerUrl;
	}
	
	void setJs(string js) {
		this->js = js;
	}
	
	string getJs() {
		return js;
	}
	
	gboolean isEmpty() {
		return empty;
	}
	
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
        empty = FALSE;
	}
};
