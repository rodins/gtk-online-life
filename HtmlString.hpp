#include <curl/curl.h>
#include "DisplayMode.hpp"

using namespace std;

class HtmlString {	
	
	static int writer(char *data, size_t size, size_t nmemb,
	                      string *writerData)
	{
	    if (writerData == NULL)
	       return 0;
	
	    writerData->append(data, size*nmemb);
	
	    return size *nmemb;
	}
	
	static int categories_writer(char *data, size_t size, size_t nmemb,
	                      string *writerData)
	{
	    if (writerData == NULL)
	       return 0;
	       
	    string strData(data);
	    
	    // Find begining
	    size_t begin = strData.find("<div class=\"nav\">");
	    // Find end
	    size_t end = strData.find("</div>");
	    
	    // Append begining
	    if(begin != string::npos && writerData->empty()) {
			string data_begin = strData.substr(begin);
			writerData->append(data_begin);
			return size *nmemb;
		}
		
		// Append middle
		if(end == string::npos && !writerData->empty()) {
			writerData->append(data, size *nmemb);
			return size *nmemb;
		}
		
		// Append end
		if(end != string::npos && !writerData->empty()) {
			string data_end = strData.substr(0, end + 6);
			writerData->append(data_end);
			return CURL_READFUNC_ABORT; 
		}
		return size *nmemb;
	}
	
	static void parser(int &count, string div, set<string> &titles) {
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
					count++;
					cout << count << ": " << to_utf8(title) << endl;
				}
				
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
						/*unescape_html(title);
					    appendToStore(to_utf8(title),
					                  href,
					                  image + "&w=165&h=236&zc=1");*/
					                  //       ^^^^^^^^^^^^   set size of image
					}
				}
			}
	}
	
	static int results_writer(char *data, size_t size, size_t nmemb,
	                      string *writerData)
	{
	    if (writerData == NULL)
	       return 0;    
	    static int count = 0;
	    static bool isBegin = FALSE;
	    static string partial_item;
	    static set<string> titles;
	    
	    string strData(data);
	    string itemBegin = "om-po"; // same as line below, but shorter
	    string itemEnd = "</a>";
	    // Find begining
	    size_t begin = strData.find("<div class=\"custom-poster\"");
	    size_t item_begin;
	    size_t item_end;
	    
	    // Find end
	    string strEnd("</table>");
	    size_t end = strData.find(strEnd);
	    
	    // Find only end part of item
	    if(isBegin) {
			item_end = strData.find(itemEnd);
			if(item_end != string::npos) {
				string item_end_part = strData.substr(0, item_end + itemEnd.size());
				isBegin = FALSE;
				partial_item += item_end_part;
				//cout << "PARTIAL ITEM: " << partial_item << endl;
				parser(count, partial_item, titles);
				item_begin = strData.find(itemBegin, item_end);
			    item_end = strData.find(itemEnd, item_begin);
			}
		}else {
			item_begin = strData.find(itemBegin);
			item_end = strData.find(itemEnd, item_begin);
		}
	    
	    // Find whole item
	    while(item_begin != string::npos && item_end != string::npos && item_begin < item_end) {
			string whole_item = strData.substr(item_begin, item_end - item_begin + itemEnd.size());
			//cout << "WHOLE ITEM: " << whole_item << endl;
			parser(count, whole_item, titles);
			item_begin = strData.find(itemBegin, item_end);
			item_end = strData.find(itemEnd, item_begin);
		}
		
		// Find only begining part of item
		if(item_begin != string::npos && item_end == string::npos) {
			string item_begin_part = strData.substr(item_begin);
			//cout << "BEGIN PART: " << item_begin_part << endl;
			isBegin = TRUE;
			partial_item = item_begin_part;
		}
	    
	    // Append begining
	    if(begin != string::npos && writerData->empty()) {
			string data_begin = strData.substr(begin);
			writerData->append(data_begin);
			return size *nmemb;
		}
		
		// Append middle
		if(end == string::npos && !writerData->empty()) {
			writerData->append(data, size *nmemb);
			return size *nmemb;
		}
		
		// Append end
		if(end != string::npos && !writerData->empty()) {
			string data_end = strData.substr(0, end + strEnd.size());
			writerData->append(data_end);
			cout << "Count: " << count << endl;
			count = 0;
			titles.clear();
			return CURL_READFUNC_ABORT; 
		}
	
	    return size *nmemb;
	}
	
	//static int results_writer(char *data, size_t size, size_t nmemb,
	                      //string *writerData)
	//{
	    //if (writerData == NULL)
	       //return 0;    
	       
	    //string strData(data);
	    //// Find begining
	    //size_t begin = strData.find("<div class=\"custom-poster\"");
	    //// Find end
	    //string strEnd("</table>");
	    //size_t end = strData.find(strEnd);
	    
	    //// Append begining
	    //if(begin != string::npos && writerData->empty()) {
			//string data_begin = strData.substr(begin);
			//writerData->append(data_begin);
			//return size *nmemb;
		//}
		
		//// Append middle
		//if(end == string::npos && !writerData->empty()) {
			//writerData->append(data, size *nmemb);
			//return size *nmemb;
		//}
		
		//// Append end
		//if(end != string::npos && !writerData->empty()) {
			//string data_end = strData.substr(0, end + strEnd.size());
			//writerData->append(data_end);
			//return CURL_READFUNC_ABORT; 
		//}
	
	    //return size *nmemb;
	//}
	
	static int actors_writer(char *data, size_t size, size_t nmemb,
	                      string *writerData)
	{
	    if (writerData == NULL)
	       return 0;
	
	    string strData(data);
	    // Find begining
	    size_t begin = strData.find(to_cp1251("Название:"));
	    // Find end
	    string strEnd = to_cp1251("Премьера в мире:");
	    size_t end = strData.find(strEnd);
	    
	    // Append begining
	    if(begin != string::npos && writerData->empty()) {
			if(end != string::npos) { // End found in first line
				string data_substr = strData.substr(begin, end - begin);
                writerData->append(data_substr);
				return CURL_READFUNC_ABORT;
			}else {
				string data_begin = strData.substr(begin);
				writerData->append(data_begin);
				return size *nmemb;
			}
		}
		
		// Append middle
		if(end == string::npos && !writerData->empty()) {
			writerData->append(data, size *nmemb);
			return size *nmemb;
		}

		
		// Append end
		if(end != string::npos && !writerData->empty()) {
			string data_end = strData.substr(0, end + strEnd.size());
			writerData->append(data_end);
			return CURL_READFUNC_ABORT; 
		}
	
	    return size *nmemb;
	}
	
	public:
	
	// Получение html странице в виде строки
	static string getPage(string url, 
	                      string referer_link = "", 
	                      DisplayMode mode = NONE)
	{
		CURL *curl_handle;
		string buffer;
		
		/* init the curl session */
		curl_handle = curl_easy_init();
		
		if(curl_handle) {
			/* remove crash bug */
			curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
		    
		    /* set url to get here */
			curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
			
			/* send all data to this function */
			
			// Download only part of html which needed
			if(mode == CATEGORIES) {
				curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, HtmlString::categories_writer);
			}else if(mode == RESULTS) {
				curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, HtmlString::results_writer);
		    }else if(mode == ACTORS) {
				curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, HtmlString::actors_writer);
			}else {
				curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, HtmlString::writer);
			}

			curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
			
			if(referer_link != "") {
				curl_easy_setopt(curl_handle, CURLOPT_REFERER, referer_link.c_str());
			}
			
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &buffer);

			/* get it */
			curl_easy_perform(curl_handle);
			
			/* cleanup curl stuff */
			curl_easy_cleanup(curl_handle);
		}
		return buffer;
	}
	
	static string getCategoriesPage() {
		return getPage(DOMAIN, "", CATEGORIES);
	}
	
	static string getResultsPage(string link) {
		return getPage(link, "", RESULTS);
	}
	
	static string getActorsPage(string link) {
		return getPage(link, "", ACTORS);
	}
};
