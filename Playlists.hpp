//Playlists.hpp
#include "PlayItem.hpp"

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
	
	string getHrefId() {
		//Find id
		string domain = DomainFactory::getWwwDomainNoSuffix();
		size_t id_begin = url.find(domain);
		// Make parser domain end independent
		if(id_begin != string::npos) {
			id_begin = url.find("/", id_begin+1);
		}
		size_t id_end = url.find("-", id_begin + domain.length());
		if(id_begin != string::npos && id_end != string::npos) {
			size_t id_length = id_end - id_begin - domain.length();
			string id_str = url.substr(id_begin + domain.length(), id_length);
			//cout << "Id: " << id_str << endl;
			return id_str;
		}
		return "";
	}
	
	string getTrailerId(string &page) {
		size_t begin = page.find("?trailer_id=");
		size_t end = page.find("' ", begin+13);
		if(begin != string::npos && end != string::npos) {
			size_t length = end - begin;
			string trailerId = page.substr(begin+12, length-12);
			return trailerId;
		}
		return "";
	}
	
	string get_txt_link(string page) {
		string begin = " {";
		string end = "\"};";
		size_t json_begin = page.find(begin);
		size_t json_end = page.find(end);
		if(json_end != string::npos && json_begin != string::npos) {
			size_t json_length = json_end - json_begin;
			string json = page.substr(json_begin+2, json_length-2);
	        
	        // Find link
	        size_t link_begin = json.find("pl:");
	        size_t link_end = json.find("\"", link_begin+4);
	        if(link_begin != string::npos && link_end != string::npos) {
				size_t link_length = link_end - link_begin;
				string link = json.substr(link_begin+4, link_length-4);
				return link;
			}
		}
		return "";
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
	
	PlayItem parse_play_item(string page) {
		PlayItem play_item;
		string comment, file, download;
		//Search for file
		size_t file_begin = page.find("\"file\"");
		size_t file_end = page.find("\"", file_begin+10);
		if(file_begin != string::npos && file_end != string::npos) {
		    size_t file_length = file_end - file_begin;
		    file = page.substr(file_begin+8, file_length-8);
		    play_item.file = file;
		}
		
		//Search for download
		size_t download_begin = page.find("\"download\"");
		size_t download_end = page.find("\"", download_begin+12);
		if(download_begin != string::npos && download_end != string::npos) {
			size_t download_length = download_end - download_begin;
			download = page.substr(download_begin+12, download_length-12);
			play_item.download = download;
		}
		
		//Search for comment
		size_t comment_begin = page.find("\"comment\"");
		size_t comment_end = page.find("\"", comment_begin+11);
		if(comment_begin != string::npos && comment_end != string::npos) {
			size_t comment_length = comment_end - comment_begin;
			comment = page.substr(comment_begin+11, comment_length-11);
			play_item.comment = comment;
		}
		
		return play_item;
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
					
			PlayItem playItem = parse_play_item(item);
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
