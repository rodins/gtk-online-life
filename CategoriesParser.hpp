//Categories.hpp

class CategoriesParser {
	CategoriesModel *model;
	string page;
	public:
	
	CategoriesParser(CategoriesModel *model) {
		this->model = model;
	}
	
	gboolean isModelEmpty() {
		return model->isEmpty();
	}
	
	gboolean parseData(string strData) {
		// Find begining
	    size_t begin = strData.find("<div class=\"nav\">");
	    // Find end
	    size_t end = strData.find("</div>");
	    
	    // Append begining
	    if(begin != string::npos && page.empty()) {
			string data_begin = strData.substr(begin);
			page.append(data_begin);
			return FALSE;
		}
		
		// Append middle
		if(end == string::npos && !page.empty()) {
			page.append(strData);
			return FALSE;
		}
		
		// Append end
		if(end != string::npos && !page.empty()) {
			string data_end = strData.substr(0, end + 6);
			page.append(data_end);
			return TRUE; 
		}
		return FALSE;
	}
	
	void parsePage() {
		size_t nav_begin = page.find("<div class=\"nav\">");
		size_t nav_end = page.find("</div>", nav_begin+1);
		if(nav_begin != string::npos && nav_end != string::npos) {
			size_t nav_length = nav_end - nav_begin + 6; 
			string nav = page.substr(nav_begin, nav_length);
			
			// Add to top level
			model->addToTopLevel("Главная", DomainFactory::getWwwDomain());
			
			// Find nodrop items
			// Find new, popular, best
			size_t nodrop_begin = nav.find("<li class=\"pull-right");
			size_t nodrop_end = nav.find("</li>", nodrop_begin+1);
			while(nodrop_begin != string::npos && nodrop_end != string::npos) {
				size_t nodrop_length = nodrop_end - nodrop_begin;
				string nodrop = nav.substr(nodrop_begin, nodrop_length);
				
				// Add to child
				parseAnchorToChild(nodrop);
				
				// Increment
				nodrop_begin = nav.find("<li class=\"pull-right", nodrop_end+1);
			    nodrop_end = nav.find("</li>", nodrop_begin+1);
			}
			// Find trailer
			size_t trailer_begin = nav.find("<li class=\"nodrop\" ");
			size_t trailer_end = nav.find("</a>", trailer_begin+1);
			if(trailer_begin != string::npos && trailer_end != string::npos) {
			    size_t trailer_length = trailer_end - trailer_begin;
			    string trailer = nav.substr(trailer_begin, trailer_length+4);
			    
			    // Add to child
			    parseAnchorToChild(trailer);	
			}
			
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
				while(anchor_begin != string::npos && anchor_end != string::npos) {
					size_t anchor_length = anchor_end - anchor_begin;
					string anchor = drop.substr(anchor_begin, anchor_length);
					
					size_t link_begin = anchor.find("\"");
					size_t link_end = anchor.find("\"", link_begin+1);
					if(link_begin != string::npos && link_end != string::npos) {
						size_t link_length = link_end - link_begin;
						string link = anchor.substr(link_begin+1, link_length-1);
						
						size_t title_begin = anchor.find(">");
						if(title_begin != string::npos) {
							string title = anchor.substr(title_begin+1);
							if(first) {
								// Add to top level
				                model->addToTopLevel(to_utf8(title), addDomainTo(link));
				
								first = false;
							}else {
								// Add to child
								model->addToChild(to_utf8(title), addDomainTo(link));
							}
						}
					}
					// increment
					anchor_begin = drop.find(begin, anchor_end+1);
			        anchor_end = drop.find(end, anchor_begin+1);
				}
				
				drop_begin = nav.find("<li class=\"drop\">", drop_end+1);
			    drop_end = nav.find("</ul>", drop_begin+1);
			}
		}
	}
	
	private:
	
	void parseAnchorToChild(string anchor) {
		// Parse link
		size_t link_begin = anchor.find("<a href=");
		size_t link_end = anchor.find("\"", link_begin+10);
		if(link_begin != string::npos && link_end != string::npos) {
			size_t link_length = link_end - link_begin;
			string link = anchor.substr(link_begin+9, link_length-9);
			// Parse title
			size_t title_begin = anchor.find(">", link_end);
			size_t title_end = anchor.find("</a>");
			if(title_end != string::npos) {
				size_t title_length = title_end - title_begin;
				string title = anchor.substr(title_begin+1, title_length-1);
				model->addToChild(to_utf8(title), addDomainTo(link));
			}
		}
	}
	
	string addDomainTo(string link) {
		if(link.find(DomainFactory::getWwwDomain()) == string::npos) {
			return DomainFactory::getWwwDomain() + link;
		}else {
			return link;
		}
	}
};
