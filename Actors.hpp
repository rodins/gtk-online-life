//Actors.hpp

class Actors {
    vector<Item> items;
    public:
    
    vector<Item> getActors() {
        return items;
    }
    
    void parse(string &page) {
		items.clear();
		page = to_utf8(page);
		parse_info(page, "Режиссер:", " (режиссер)");
		parse_info(page, "В ролях:", "");
	}
	
	private:
	
	void parse_info(string page, string query, string director) {
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
					
					//Find href
					size_t href_begin = anchor.find("href=");
					size_t href_end = anchor.find(">", href_begin + 1);
					if(href_begin != string::npos && href_end != string::npos) {
						size_t href_length = href_end - href_begin; 
						string href = anchor.substr(href_begin+6, href_length-7);
						//cout << href << endl;
						
						Item item(title + director, href);
					    items.push_back(item);
					}
				}
				
				anchor_begin = actors.find(begin, anchor_end);
	            anchor_end = actors.find(end, anchor_begin);
			}
		}
	}
};