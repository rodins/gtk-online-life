// CategoriesModel.hpp

class CategoriesModel {
    GdkPixbuf *categoryIcon;
	GdkPixbuf *itemIcon;
	
	GtkTreeStore *treestore;
	GtkTreeIter topLevel, child;
	int count;
    public:
    CategoriesModel() {
	 	categoryIcon = IconsFactory::getFolderIcon();
	    itemIcon = IconsFactory::getLinkIcon();
	    
	    treestore = gtk_tree_store_new(TREE_NUM_COLS, 
		                               GDK_TYPE_PIXBUF,
					                   G_TYPE_STRING, 
					                   G_TYPE_STRING);
		count = 0;
	}
	
	~CategoriesModel() {
		g_object_unref(treestore);
	}
	
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
		count++;
	}
	
	gboolean isEmpty() {
		return count == 0;
	}
	
	GtkTreeModel *getTreeModel() {
		return GTK_TREE_MODEL(treestore);		
	}
};
