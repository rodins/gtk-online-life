// SavedItemsModel.hpp

class SavedItemsModel {
	GtkListStore *store;
	GtkTreeIter iter;
	int count;
	public:
	SavedItemsModel() {
		store = gtk_list_store_new(
		     ICON_NUM_COLS,   // Number of columns
		     GDK_TYPE_PIXBUF, // Image poster
		     G_TYPE_STRING,   // Title
		     G_TYPE_STRING,   // Href
		     G_TYPE_STRING    // Image link
		);
		count = 0;
	}
	
	bool isEmpty() {
		return count == 0;
	}
	
	GtkTreeModel *getTreeModel() {
		return GTK_TREE_MODEL(store);
	}
	
	void clear() {
		gtk_list_store_clear(store);
		count = 0;
	}
	
	void add(const gchar *filename, GdkPixbuf *icon, string &href) {
	    gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, 
                           &iter,
                           ICON_IMAGE_COLUMN, 
                           icon,
                           ICON_TITLE_COLUMN, 
                           filename,
                           ICON_HREF, 
                           href.c_str(),
                           ICON_IMAGE_LINK, 
                           "", 
                           -1);
		count++;	
	}
};
