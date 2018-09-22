// PlaylistsParser.hpp

class PlaylistsParser {
    PlaylistsModel *model;
    public:
    PlaylistsParser(PlaylistsModel *model) {
		this->model = model;
	}
	
	void parse(string json) {
		model->clear();
		parse_playlists(json);
        if(model->isEmpty()) {
            parse_playlist(json, "");
		}
	}
	
	private:
	
	void parse_playlist(string items, string comment) {
		if(!comment.empty()) {
			model->addListToTopLevel(comment);
		}
		size_t item_start = items.find("{");
		size_t item_end = items.find("}", item_start+1);
		while(item_start != string::npos && item_end != string::npos) {
		    size_t item_length = item_end - item_start;
		    string item = items.substr(item_start, item_length);
					
			PlayItem playItem = PlaylistsUtils::parse_play_item(item);
			if(comment.empty()) {
				if(!playItem.comment.empty()) {
					model->addItemToTopLevel(playItem.comment, 
				                  playItem.file, 
				                  playItem.download);
				}
			}else {
				if(!playItem.comment.empty()) {
					model->addItemToChild(playItem.comment,
				                   playItem.file, 
				                   playItem.download);
				}
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
