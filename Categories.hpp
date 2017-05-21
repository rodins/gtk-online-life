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
			CategoryItem main;
			main.set_link(DOMAIN);
			main.set_title("Главная");
			
			// Find nodrop items
			size_t nodrop_begin = nav.find("<li class=\"pull-right");
			size_t nodrop_end = nav.find("</li>", nodrop_begin+1);
			while(nodrop_begin != string::npos && nodrop_end != string::npos) {
				size_t nodrop_length = nodrop_end - nodrop_begin;
				string nodrop = nav.substr(nodrop_begin, nodrop_length);
				main.push_back(parse_anchor(nodrop));
				nodrop_begin = nav.find("<li class=\"pull-right", nodrop_end+1);
			    nodrop_end = nav.find("</li>", nodrop_begin+1);
			}
			// Find trailer
			size_t trailer_begin = nav.find("<li class=\"nodrop\" ");
			size_t trailer_end = nav.find("</a>", trailer_begin+1);
			if(trailer_begin != string::npos && trailer_end != string::npos) {
			    size_t trailer_length = trailer_end - trailer_begin;
			    string trailer = nav.substr(trailer_begin, trailer_length+4);
			    main.push_back(parse_anchor(trailer));	
			}
			results.push_back(main);
			// Find drop items
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
	
	private:
	
	CategoryItem parse_anchor(string anchor) {
		CategoryItem item;
		size_t link_begin = anchor.find("<a href=");
		size_t link_end = anchor.find("\"", link_begin+10);
		if(link_begin != string::npos && link_end != string::npos) {
			size_t link_length = link_end - link_begin;
			string link = anchor.substr(link_begin+9, link_length-9);
			item.set_link(link);
			size_t title_begin = anchor.find(">", link_end);
			size_t title_end = anchor.find("</a>");
			if(title_end != string::npos) {
				size_t title_length = title_end - title_begin;
				string title = anchor.substr(title_begin+1, title_length-1);
				item.set_title(to_utf8(title));
			}
		}
		return item;
	}
};
