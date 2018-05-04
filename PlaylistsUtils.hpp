// PlaylistsUtils.hpp

class PlaylistsUtils {
	public:
	
	static string get_href_id(string href) {
		size_t id_begin = href.rfind("/");
		size_t id_end = href.find("-", id_begin);
		if(id_begin != string::npos && id_end != string::npos) {
			return href.substr(id_begin+1, id_end-id_begin-1);
		}
		return "";
	}
	
	static string parsePlayerForUrl(string &player) {
		size_t js_detect = player.find("js.php");
		if(js_detect != string::npos) {
			size_t js_begin = player.rfind("src=", js_detect);
			size_t js_end = player.find("\"", js_begin+6);
			if(js_begin != string::npos && js_end != string::npos) {
				size_t js_length = js_end - js_begin;
				string js = player.substr(js_begin+5, js_length-5);
				return "http:" + js;
			}
		}
		return "";
	}
	
	static string get_txt_link(string &page) {
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
	
	static PlayItem parse_play_item(string page, bool isUtf8 = TRUE) {
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
			if(isUtf8) {
				play_item.comment = comment;
			}else {
				play_item.comment = to_utf8(comment);
			}
		}
		
		return play_item;
	}
};
