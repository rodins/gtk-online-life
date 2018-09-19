// ResultsParser.hpp

class ResultsParser {
	bool isFirst;
	set<string> titles;
	string partial_div;
	bool divBeginFound;
	bool isFirstItem;
	
	CenterView *view;
	ResultsModel *model;
    
    public:
    ResultsParser(CenterView *view, ResultsModel *model) {
		isFirst = TRUE;
		divBeginFound = FALSE;
		isFirstItem = TRUE;
		
		this->view = view;
		this->model = model;
	}
	
	void setModel(ResultsModel *model) {
		this->model = model;
	}
	
	void resetFirstItem() {
		isFirstItem = TRUE;
	}
  
    void divs_parser(string strData) {
	    // Find end
	    string strEnd("</table>");
	    size_t end = strData.find(strEnd);
	    
	    // Find div
	    size_t starting_point = strData.find("tom-pos");
	    // If starting point is not found, don't parse divs
	    if(starting_point == string::npos && isFirst && !divBeginFound) {
			return;
		}
	    size_t div_end_first = strData.find("</div>");
	    size_t div_begin = strData.find("<div");
	    size_t div_end = strData.find("</div>", div_begin+3);
	    
	    if(div_end_first != string::npos && divBeginFound) {
			divBeginFound = FALSE;
			partial_div += strData.substr(0, div_end_first+6);
			// On first item found clear found end
			if(isFirst) {
				isFirst = FALSE;
				end = string::npos;
			}
			find_item(partial_div, titles);
			find_pager(partial_div);
		}
	    
	    while(div_begin != string::npos && div_end != string::npos) {
			string div = strData.substr(div_begin, div_end - div_begin + 6);
			// On first item found clear found end
			if(isFirst) {
				isFirst = FALSE;
				end = string::npos;
			}
			find_item(div, titles);
			find_pager(div);
			div_begin = strData.find("<div", div_end+4);
	        div_end = strData.find("</div>", div_begin+3);
		}
		
		if(div_begin != string::npos) {
			divBeginFound = TRUE;
			partial_div = strData.substr(div_begin);
		}

		// Detect end
		if(end != string::npos && !isFirst) {
			isFirst = TRUE;
			titles.clear();
			//return CURL_READFUNC_ABORT; 
		}
	}
	
	private:
	
	void item_parser(string div, set<string> &titles) {
		//Find title
		size_t title_begin = div.find("/>");
		size_t title_end = div.find("</a>", title_begin + 1);
		if(title_begin != string::npos && title_end != string::npos) {
			size_t title_length = title_end - title_begin - 2;
			string title = div.substr(title_begin+2, title_length);
			size_t title_new_line = title.find("\n");
			if(title_new_line != string::npos) {//delete last char if last char is new line
			    title = div.substr(title_begin+2, title_new_line + 1);
				title.erase(title.size()-1);
			}
			
			if(titles.count(to_utf8(title)) == 0) {
				titles.insert(to_utf8(title));
				
				//Find href
				size_t href_begin = div.find("href=");
				size_t href_end = div.find(".html", href_begin + 1);
				if(href_begin != string::npos && href_end != string::npos) {
					size_t href_length = href_end - href_begin; 
					string href = div.substr(href_begin+6, href_length-1);
					//cout << "Href: " << href << endl;
					//Find image
					size_t image_begin = div.find("src=");
					size_t image_end = div.find("&", image_begin + 1);
					if(image_begin != string::npos && image_end != string::npos) {
						size_t image_length = image_end - image_begin;
						string image = div.substr(image_begin+5, image_length-5);
						unescape_html(title);
						// Remove newlines from image
						size_t pos = image.find("\r\n");
						while(pos != string::npos) {
							image.erase(pos, 2);
							pos = image.find("\r\n", pos+1);
						}
					    model->appendToStore(to_utf8(title),
					                         href,
					                         image + "&w=165&h=236&zc=1");
					                         //       ^^^^^^^^^^^^   set size of image
					}
				}
			}
		}
	}
	
	void parse_anchor(string anchor) {
		size_t link_begin = anchor.find("=\"");
		size_t link_end = anchor.find(">", link_begin + 5);
		if(link_begin != string::npos && link_end != string::npos) {
			size_t link_length = link_end - link_begin;
			string link = anchor.substr(link_begin+2, link_length-3);
			string title = to_utf8(anchor.substr(link_end+1));
			
			if(title == "Вперед") {
				model->setNextLink(link);
			}
		}
	}
	
	void pager_parser(string pager) {
		model->setNextLink("");
        //Find menu pager anchors
        // <a href="http://google.com.ua">Google</a>
        string begin = "<a href";
	    string end = "</a>";
        size_t anchor_begin = pager.find(begin);
        size_t anchor_end = pager.find(end, anchor_begin+1);
        string anchor;
        while(anchor_end != string::npos && anchor_begin != string::npos) {
			size_t anchor_length = anchor_end - anchor_begin;
			anchor = pager.substr(anchor_begin, anchor_length);
			
			parse_anchor(anchor);
			
		    anchor_begin = pager.find(begin, anchor_end);
            anchor_end = pager.find(end, anchor_begin);
		}
		
		//Find search pager anchors
	    begin = "onclick";
        anchor_begin = pager.find(begin);
        anchor_end = pager.find(end, anchor_begin+1);
        while(anchor_end != string::npos && anchor_begin != string::npos) {
			size_t anchor_length = anchor_end - anchor_begin;
			anchor = pager.substr(anchor_begin, anchor_length);
			
			size_t page_begin = anchor.find("(");
			size_t page_end = anchor.find(")", page_begin+1);
			if(page_begin != string::npos && page_end != string::npos) {
				size_t page_length = page_end - page_begin;
				string page_num = anchor.substr(page_begin+1, page_length-1);
				
				size_t title_begin = anchor.find(">");
				string title = to_utf8(anchor.substr(title_begin+1));
				
				if(title == "Вперед") {
					model->setNextLink(model->getBaseUrl() + 
					                     "&search_start=" +
					                     page_num);
				}
			}
			
		    anchor_begin = pager.find(begin, anchor_end);
            anchor_end = pager.find(end, anchor_begin);
		}
	}
	
	void preParser(string div, set<string> &titles) {
		// Prepare to show results when first result item comes
		if(isFirstItem) {
			isFirstItem = FALSE;
			// No next link, new results
			if(model->getNextLink().empty()) {
				view->scrollToTopOfList();
			}
			view->showResultsData();
		}
		item_parser(div, titles);
	}
	
    void find_item(string &div, set<string> &titles) {				  
		size_t item_begin = div.find("<div class=\"custom-poster\"");
		size_t item_end = div.find("</a>", item_begin+3);
		if(item_begin != string::npos && item_end != string::npos) {
			string item = div.substr(item_begin, item_end - item_begin + 4);
			gdk_threads_enter();
			preParser(item, titles);
			gdk_threads_leave();
		}
	}
	
	void find_pager(string &div) {
		size_t pager_begin = div.find("class=\"navigation\"");
	    size_t pager_end = div.find("</div>", pager_begin+1);
	    if(pager_begin != string::npos && pager_end != string::npos) {
			string pager = div.substr(pager_begin, pager_end - pager_begin + 6);
			pager_parser(pager);
		}
	}
};
