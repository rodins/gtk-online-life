#include <curl/curl.h>

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
	
	static int results_writer(char *data, size_t size, size_t nmemb,
	                      string *writerData)
	{
	    if (writerData == NULL)
	       return 0;    
	       
	    string strData(data);
	    // Find begining
	    size_t begin = strData.find("<div class=\"custom-poster\"");
	    // Find end
	    string strEnd("</table>");
	    size_t end = strData.find(strEnd);
	    
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
			return CURL_READFUNC_ABORT; 
		}
	
	    return size *nmemb;
	}
	
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
};