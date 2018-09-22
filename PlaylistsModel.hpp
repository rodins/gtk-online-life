// PlaylistsModel.hpp

class PlaylistsModel {
    GdkPixbuf *directory;
	GdkPixbuf *item;
	
	GtkTreeStore *treestore;
	GtkTreeIter topLevel, child;
	
	int count;
	public:
	PlaylistsModel(GtkTreeModel *model) {
		directory = IconsFactory::getFolderIcon();
	    item = IconsFactory::getLinkIcon();
	    treestore = GTK_TREE_STORE(model);
	    count = 0;
	}
	
	void clear() {
		count = 0;
		gtk_tree_store_clear(treestore);
	}
	
	gboolean isEmpty() {
		return count == 0;
	}
	
	void addItemToChild(string comment, string file, string download) {
		gtk_tree_store_append(treestore, &child, &topLevel);
		gtk_tree_store_set(treestore,
		                   &child,
		                   PLAYLIST_IMAGE_COLUMN, 
		                   item, 
		                   PLAYLIST_COMMENT_COLUMN, 
		                   comment.c_str(),
		                   PLAYLIST_FILE_COLUMN,
		                   file.c_str(),
		                   PLAYLIST_DOWNLOAD_COLUMN,
		                   download.c_str(),
		                   -1);
		count++;
	}
	
	void addItemToTopLevel(string comment, string file, string download) {
		gtk_tree_store_append(treestore, &topLevel, NULL);
		gtk_tree_store_set(treestore,
		                   &topLevel,
		                   PLAYLIST_IMAGE_COLUMN, 
		                   item, 
		                   PLAYLIST_COMMENT_COLUMN, 
		                   comment.c_str(),
		                   PLAYLIST_FILE_COLUMN,
		                   file.c_str(),
		                   PLAYLIST_DOWNLOAD_COLUMN,
		                   download.c_str(),
		                   -1);
		count++;
	}
	
	void addListToTopLevel(string comment) {
		gtk_tree_store_append(treestore, &topLevel, NULL);
		gtk_tree_store_set(treestore, 
		                   &topLevel,
		                   PLAYLIST_IMAGE_COLUMN, 
		                   directory, 
		                   PLAYLIST_COMMENT_COLUMN,
		                   comment.c_str(),
		                   -1);
	}
};
