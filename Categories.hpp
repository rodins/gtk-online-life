//Categories.hpp
#include "CategoryItem.hpp"

class Categories {
	vector<CategoryItem> results;
	public:
	
	string getTitle() {
		return string("Online Life - Categories");
	}
	
	vector<CategoryItem>& getCategories() {
		return results;
	}
	
	void parse_categories(string page) {
		results.clear();
		size_t nav_begin = page.find("<div class=\"nav\">");
		size_t nav_end = page.find("</div>", nav_begin+1);
		if(nav_begin != string::npos && nav_end != string::npos) {
			size_t nav_length = nav_end - nav_begin + 6; 
			string nav = page.substr(nav_begin, nav_length);
			
			size_t drop_begin = nav.find("<li class=\"drop\">");
			size_t drop_end = nav.find("</ul>", drop_begin+1);
			while(drop_begin != string::npos && nav_end != string::npos) {
				size_t drop_length = drop_end - drop_begin;
				string drop = nav.substr(drop_begin, drop_length);
				
				string begin = "<a href=";
			    string end = "</a>";
				size_t anchor_begin = drop.find(begin);
				size_t anchor_end = drop.find(end, anchor_begin+1);
				bool first = true;
				CategoryItem categoryItem;
				while(anchor_begin != string::npos && anchor_end != string::npos) {
					size_t anchor_length = anchor_end - anchor_begin;
					string anchor = drop.substr(anchor_begin, anchor_length);
					
					size_t link_begin = anchor.find("\"");
					size_t link_end = anchor.find("\"", link_begin+1);
					CategoryItem subcategoryItem;
					if(link_begin != string::npos && link_end != string::npos) {
						size_t link_length = link_end - link_begin;
						string link = anchor.substr(link_begin+1, link_length-1);
						if(first) {
							categoryItem.set_link(link);
						}else {
							subcategoryItem.set_link(link);
						}
						
						size_t title_begin = anchor.find(">");
						if(title_begin != string::npos) {
							string title = anchor.substr(title_begin+1);
							if(first) {
								categoryItem.set_title(to_utf8(title));
								first = false;
							}else {
								subcategoryItem.set_title(to_utf8(title));
								categoryItem.push_back(subcategoryItem);
							}
						}
					}
					
					anchor_begin = drop.find(begin, anchor_end+1);
			        anchor_end = drop.find(end, anchor_begin+1);
				}
				results.push_back(categoryItem);
				
				drop_begin = nav.find("<li class=\"drop\">", drop_end+1);
			    drop_end = nav.find("</ul>", drop_begin+1);
			}
		}
	}
};
