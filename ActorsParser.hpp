// ActorsParser.hpp

class ActorsParser {
    ActorsModel *model;
    string page;
    public:
    ActorsParser(ActorsModel *model) {
		this->model = model;
	}
	
	ActorsModel* getModel() {
		return model;
	}
	
	void init() {
		page = "";
	}
	
	gboolean parseData(string strData) {
	    // Find begining
	    size_t begin = strData.find(to_cp1251("Название:"));
	    // Find end
	    string strEnd = "</iframe>";
	    size_t end = strData.find(strEnd);
	    
	    // Append begining
	    if(begin != string::npos && page.empty()) {
			if(end != string::npos) { // End found in first line
				string data_substr = strData.substr(begin, end - begin);
                page.append(data_substr);
				return TRUE;
			}else {
				string data_begin = strData.substr(begin);
				page.append(data_begin);
				return FALSE;
			}
		}
		
		// Append middle
		if(end == string::npos && !page.empty()) {
			page.append(strData);
			return FALSE;
		}

		
		// Append end
		if(end != string::npos && !page.empty()) {
			string data_end = strData.substr(0, end + strEnd.size());
			page.append(data_end);
			return TRUE; 
		}
		return FALSE;
	}
	
	void parsePage() {		
		page = to_utf8(page);
		string year = parse_simple_info("Год: ");
		string country = parse_simple_info("Страна: ");
		model->setInfo(year, country);
		parse_info("Режиссер:", " (режиссер)");
		parse_info("В ролях:", "");
		parse_iframe();
	}
	
	private:
	
	void parse_iframe() {
		size_t iframe_begin = page.find("<iframe");
		size_t iframe_end = page.find("</iframe>", iframe_begin+10);
		if(iframe_begin != string::npos && iframe_end != string::npos) {
			size_t iframe_length = iframe_end - iframe_begin;
			string iframe = page.substr(iframe_begin, iframe_length);
			size_t link_begin = iframe.find("src=");
			size_t link_end = iframe.find("\"", link_begin+6);
			if(link_begin != string::npos && link_end != string::npos) {
				size_t link_length = link_end - link_begin;
			    string playerUrl = iframe.substr(link_begin+5, link_length-5);
			    model->setPlayerUrl(playerUrl);
			}
		}
	}
	
	string parse_simple_info(string query) {
		string end = "\n";
		size_t info_begin = page.find(query);
		size_t info_end = page.find(end, info_begin);
		if(info_begin != string::npos && info_end != string::npos) {
			size_t info_length = info_end - info_begin - query.size();
			string info = page.substr(info_begin + query.size(), info_length);
			return info;
		}
		return "";
	}
	
	void parse_info(string query, string director) {
		string begin = query; // "В ролях:";
		string end = "</p>";
		size_t actors_begin = page.find(begin);
		size_t actors_end = page.find(end, actors_begin+1);
		if(actors_begin != string::npos && actors_end != string::npos) {
			size_t actors_length = actors_end - actors_begin;
			string actors = page.substr(actors_begin, actors_length);
			//cout << actors << endl;
			
			string begin = "<a href";
		    string end = "</a>";
	        size_t anchor_begin = actors.find(begin);
	        size_t anchor_end = actors.find(end, anchor_begin+1);
	        string anchor;
	        while(anchor_end != string::npos && anchor_begin != string::npos) {
				size_t anchor_length = anchor_end - anchor_begin;
				anchor = actors.substr(anchor_begin, anchor_length);
				//cout << anchor << endl;
				
				size_t title_begin = anchor.find(">");
				if(title_begin != string::npos) {
					string title = anchor.substr(title_begin+1);
					//cout << title << endl;
					unescape_html(title);
					//Find href
					size_t href_begin = anchor.find("href=");
					size_t href_end = anchor.find(">", href_begin + 1);
					if(href_begin != string::npos && href_end != string::npos) {
						size_t href_length = href_end - href_begin; 
						string href = anchor.substr(href_begin+6, href_length-7);
						//cout << href << endl;
						
					    model->addToStore(title + director, href);
					}
				}
				
				anchor_begin = actors.find(begin, anchor_end);
	            anchor_end = actors.find(end, anchor_begin);
			}
		}
	}
};
