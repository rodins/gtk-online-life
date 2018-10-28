// ActorsModel.hpp
#include "LinksMode.hpp"

class ActorsModel {
    GtkListStore *store;
    GtkTreeIter iter;
    GdkPixbuf *item;
    GdkPixbuf *pixbuf;
    gboolean empty;
    LinksMode linksMode;
    string title, link;
    string info;
    string playerUrl;
    string browserUrl;
    public:
    ActorsModel() {
		empty = TRUE;
		item = IconsFactory::getLinkIcon();
	}
	
	void init(string title, string link, GdkPixbuf *pixbuf) {
		
		this->title = stripNewLine(title);
		this->info = this->title;
		this->link = link;
		this->pixbuf = pixbuf;
		empty = TRUE;
		linksMode = LINKS_MODE_EMPTY;
		store = gtk_list_store_new(TREE_NUM_COLS, 
                                   GDK_TYPE_PIXBUF,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING);
		playerUrl = "";
	    browserUrl = "";
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
	
	void setBrowserUrl(string browserUrl) {
		this->browserUrl = browserUrl;
	}
	
	string getBrowserUrl() {
		return browserUrl;
	}
	
	gboolean isEmpty() {
		return empty;
	}
	
	GdkPixbuf *getPixbuf() {
		return pixbuf;
	}
	
	void setLinksMode(LinksMode linksMode) {
	    this->linksMode = linksMode;	
	}
	
	LinksMode getLinksMode() {
	    return linksMode;	
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
	
	private:
	string stripNewLine(string title) {
		size_t pos = title.find("\n");
		if(pos != string::npos) {
		    return title.substr(0, pos);	
		}
		return title;
	}
};
