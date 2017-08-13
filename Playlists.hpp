//Playlists.hpp
#include "PlayItem.hpp"
#include "Playlist.hpp"

class Playlists {
	vector<Playlist> playlists;
	public:
	
	vector<Playlist>& getPlaylists() {
		return playlists;
	}
	
	void parse(string json) {
		parse_playlists(json);
        if(playlists.empty()) {
            Playlist playlist = parse_playlist(json, "");
            playlists.push_back(playlist);
		}
	}
	
	PlayItem parse_play_item(string page) {
		PlayItem play_item;
		//Search for file
		size_t file_begin = page.find("\"file\"");
		size_t file_end = page.find("\"", file_begin+10);
		if(file_begin != string::npos && file_end != string::npos) {
		    size_t file_length = file_end - file_begin;
		    string file = page.substr(file_begin+8, file_length-8);
		    play_item.set_file(file);
		}
		
		//Search for download
		size_t download_begin = page.find("\"download\"");
		size_t download_end = page.find("\"", download_begin+12);
		if(download_begin != string::npos && download_end != string::npos) {
			size_t download_length = download_end - download_begin;
			string download = page.substr(download_begin+12, download_length-12);
			play_item.set_download(download);
		}
		
		//Search for comment
		size_t comment_begin = page.find("\"comment\"");
		size_t comment_end = page.find("\"", comment_begin+11);
		if(comment_begin != string::npos && comment_end != string::npos) {
			size_t comment_length = comment_end - comment_begin;
			string comment = page.substr(comment_begin+11, comment_length-11);
			play_item.set_comment(comment);
		}
		return play_item;
	}
	
	private:
	
	Playlist parse_playlist(string items, string comment) {
		Playlist playlist(comment);
		size_t item_start = items.find("{");
		size_t item_end = items.find("}", item_start+1);
		while(item_start != string::npos && item_end != string::npos) {
		    size_t item_length = item_end - item_start;
		    string item = items.substr(item_start, item_length);
					
			PlayItem pl_item = parse_play_item(item);
			playlist.push_back(pl_item);
					
			item_start = items.find("{", item_end+1);
			item_end = items.find("}", item_start+1);
		}
		return playlist;
	}
	
	void parse_playlists(string page) {
		playlists.clear();
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
			    
				Playlist playlist = parse_playlist(items, comment);
				playlists.push_back(playlist);
		    }
			
			// increment
			playlist_begin = page.find(begin, playlist_end+2);
			playlist_end = page.find(end, playlist_begin+1);
		}
	} 
};
