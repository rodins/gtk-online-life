#include "Item.hpp"

class Results {
	HtmlString html_string;
    string prev_link, next_link;
	string domain;
	string base_url;
	vector<Item> results;
	string current_page;
	string page;
	public:
	Results(GtkWidget *pb) {
		html_string.setProgressBar(pb);
		domain = "http://www.online-life.cc/";
	}
	
	void getResultsPage(string p) {
	    page = p;
	    parse_results();
	    parse_pager();         	
	}
	
	void setBaseUrl(string bu) {
	    base_url = bu;
	}
	
	vector<Item> getResults() {
	    return results;	
	}
	
	string getPrevLink() {
		return prev_link;
	}
	
	string getNextLink() {
		return next_link;
	}
	
	string getCurrentPage() {
		return current_page;
	}
	
	private:
	//Parse search results
	void parse_results() {
		results.clear();
	    string begin = "<div class=\"custom-poster\"";
		string end = "</a>";
		size_t div_begin = page.find(begin);
		size_t div_end = page.find(end, div_begin+1);
		while(div_end != string::npos && div_begin != string::npos) {
			size_t div_length = div_end - div_begin + end.length(); 
			string div = page.substr(div_begin, div_length);
			//cout << "DIV START: " <<  div << ". END" << endl;
			
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
				//cout << title << endl;
				
				//Find href
				size_t href_begin = div.find("href=");
				size_t href_end = div.find(".html", href_begin + 1);
				if(href_begin != string::npos && href_end != string::npos) {
					size_t href_length = href_end - href_begin; 
					string href = div.substr(href_begin+6, href_length-1);
					//cout << href << endl;
					
					//Find id
					size_t id_begin = href.find(domain);
					size_t id_end = href.find("-", id_begin + domain.length());
					if(id_begin != string::npos && id_end != string::npos) {
						size_t id_length = id_end - id_begin - domain.length();
						string id_str = href.substr(id_begin + domain.length(), id_length);
						//cout << id_str << endl;
						Item item(title, id_str, href);
						results.push_back(item);
					}
				}
			}
			
			div_begin = page.find(begin, div_end+1);
		    div_end = page.find(end, div_begin+1);
		}
	}
	
	void parse_pager() {
		prev_link = next_link = current_page = "";
		size_t pager_begin = page.find("class=\"navigation\"");
		size_t pager_end = page.find("</div>", pager_begin+1);
		if(pager_end != string::npos && pager_begin != string::npos) {
			size_t pager_length = pager_end - pager_begin;
			string pager = page.substr(pager_begin+2, pager_length-2);
	        //cout << pager << endl;
	        
	        // Find spans
	        string begin_span = "<span>";
	        string end_span = "</span>";
	        size_t span_begin = pager.find(begin_span);
	        size_t span_end = pager.find(end_span);
	        int count_span = 0;
	        string spans[2];
	        int pages[2];
	        while(span_end != string::npos && span_begin != string::npos) {
				size_t span_length = span_end - span_begin;
				spans[count_span] = pager.substr(span_begin+6, span_length-6);
				pages[count_span] = atoi(spans[count_span].c_str());
				//cout << atoi(spans[count_span].c_str()) << endl;
				count_span++;
				span_begin = pager.find(begin_span, span_end);
	            span_end = pager.find(end_span, span_begin);
			}
			//cout << span << " " << count_span << endl;
			current_page = "";
			if(count_span == 1) {
				current_page = spans[0];
			}else {
				if(pages[0] != 0) {
					current_page = spans[0];
				}
				if(pages[1] != 0) {
					current_page = spans[1];
				}
			}
	        
	        //Find menu pager anchors
	        string begin = "<a href";
		    string end = "\">";
	        size_t anchor_begin = pager.find(begin);
	        size_t anchor_end = pager.find(end, anchor_begin+1);
	        int count_anchor = 0;
	        string anchor;
	        while(anchor_end != string::npos && anchor_begin != string::npos) {
				size_t anchor_length = anchor_end - anchor_begin;
				anchor = pager.substr(anchor_begin+9, anchor_length-9);
				//cout << anchor << endl;
				
				if(count_anchor == 0) { //Get the first link
					if(count_span == 2) { //First or last page
					    if(pages[1] == 0) { //Last page
							prev_link = anchor;
						}else {
							prev_link = ""; //First page
						} 
				    }else {//Page in the middle
					    prev_link = anchor;
				    }
				}
				
				count_anchor++;
			    anchor_begin = pager.find(begin, anchor_end);
	            anchor_end = pager.find(end, anchor_begin);
			}
			
			//Find search pager anchors
	        begin = "onclick=\"javascript:list_submit(";
		    end = ")";
	        anchor_begin = pager.find(begin);
	        anchor_end = pager.find(end, anchor_begin+1);
	        count_anchor = 0;
	        while(anchor_end != string::npos && anchor_begin != string::npos) {
				size_t anchor_length = anchor_end - anchor_begin;
				anchor = pager.substr(anchor_begin+32, anchor_length-32);
				//cout << anchor << " " << anchor.size() << endl;
				
				if(count_anchor == 0) { //Get the first link
					if(count_span == 2) { //First or last page
					    if(pages[1] == 0) { //Last page
							prev_link = base_url + "&search_start=" + anchor;
						} 
				    }else {//Page in the middle
					    prev_link = base_url + "&search_start=" + anchor;
				    }
				}
				
				count_anchor++;
			    anchor_begin = pager.find(begin, anchor_end);
	            anchor_end = pager.find(end, anchor_begin);
			}
			
			//Get the last link
			if(count_span == 2) { //First or last page
				if(pages[0] == 0) { //First page
					anchor.size() > 3 ? next_link = anchor : next_link = base_url + "&search_start=" + anchor;
				} 
			}else {//Page in the middle
			    anchor.size() > 3 ? next_link = anchor : next_link = base_url + "&search_start=" + anchor;
		    }
			
			/*cout << "Prev: " << prev_link << endl;
	        cout << "Current page: " << current_page << endl;
	        cout << "Next: " << next_link << endl;*/
		}
	}

};