//Playlists.hpp

class Playlists {
	GdkPixbuf *directory;
	GdkPixbuf *item;
	
	GtkTreeStore *treestore;
	GtkTreeIter topLevel, child;
	
	int count;
	string url;
	
	public:
	
	Playlists(GtkTreeModel *model) {
		directory = IconsFactory::getFolderIcon();
	    item = IconsFactory::getLinkIcon();
	    treestore = GTK_TREE_STORE(model);
	}
	
	void setUrl(gpointer args) {
		string href((gchar*)args);
		if(!href.empty()) { // empty link means repeat (use saved link)
			url = href;
		}
	}
	
	string getUrl() {
		return url;
	}
	
	int getCount() {
		return count;
	}
	
	void parse(string json) {
		count = 0;
		gtk_tree_store_clear(treestore);
		parse_playlists(json);
        if(count == 0) {
            parse_playlist(json, "");
		}
	}
	
	private:
	
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
	
	void parse_playlist(string items, string comment) {
		if(!comment.empty()) {
			addListToTopLevel(comment);
		}
		size_t item_start = items.find("{");
		size_t item_end = items.find("}", item_start+1);
		while(item_start != string::npos && item_end != string::npos) {
		    size_t item_length = item_end - item_start;
		    string item = items.substr(item_start, item_length);
					
			PlayItem playItem = PlaylistsUtils::parse_play_item(item);
			if(comment.empty()) {
				addItemToTopLevel(playItem.comment, 
				                  playItem.file, 
				                  playItem.download);
			}else {
				addItemToChild(playItem.comment,
				               playItem.file, 
				               playItem.download);
			}
					
			item_start = items.find("{", item_end+1);
			item_end = items.find("}", item_start+1);
		}
	}
	
	void parse_playlists(string page) {
		string begin = "\"comment\"";
		string end = "]";
		size_t playlist_begin = page.find(begin);
		size_t playlist_end = page.find(end);
		while(playlist_begin != string::npos && playlist_end != string::npos) {
			size_t playlist_length = playlist_end - playlist_begin;
			string playlist = page.substr(playlist_begin, playlist_length);
			
			//Search for comment
		    size_t comment_begin = playlist.find("\"comment\"");
		    size_t comment_end = playlist.find("[", comment_begin+11);
		    if(comment_begin != string::npos && comment_end != string::npos) {
			    size_t comment_length = comment_end - comment_begin;
			    string comment = playlist.substr(comment_begin+11, comment_length-11);
			    size_t comment_new_line = comment.find("\",");
				if(comment_new_line != string::npos) {
				    comment = playlist.substr(comment_begin+11, comment_new_line);
				}
			    
				size_t items_length = playlist.length() - comment_end;
				string items = playlist.substr(comment_end +1, items_length);
			    
				parse_playlist(items, comment);
		    }
			
			// increment
			playlist_begin = page.find(begin, playlist_end+2);
			playlist_end = page.find(end, playlist_begin+1);
		}
	} 
};
