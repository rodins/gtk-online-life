// PlaylistsUtils.hpp

class PlaylistsUtils {
	public:
    static string getHrefId(string url) {
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
	
	static string getUrl(string id) {
		return "http://dterod.com/js.php?id=" + id;
	}
	
	static string getReferer(string id) {
		return "http://dterod.com/player.php?newsid=" + id;
	}
	
	static string getCidwoUrl(string id) {
		return "http://play.cidwo.com/js.php?id" + id;
	}
	
	static string getCidwoReferer(string id) {
		return "http://play.cidwo.com/player.php?newsid=" + id;
	}
	
	static string getTrailerId(string &page) {
		size_t begin = page.find("?trailer_id=");
		size_t end = page.find("' ", begin+13);
		if(begin != string::npos && end != string::npos) {
			size_t length = end - begin;
			string trailerId = page.substr(begin+12, length-12);
			return trailerId;
		}
		return "";
	}
	
	static string getTrailerUrl(string id) {
		return "http://dterod.com/js.php?id=" + id + "&trailer=1";
	}
	
	static string getTrailerReferer(string id) {
		return "http://dterod.com/player.php?trailer_id=" + id;
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
	
	static PlayItem parse_play_item(string &page, bool isUtf8 = TRUE) {
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
