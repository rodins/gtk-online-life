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
	
	public:
	
	static CURL *get_curl_handle() {
		static CURL *curl_handle;
		if(curl_handle == NULL) {
			/* init the curl session */
			curl_handle = curl_easy_init();
			if(curl_handle) {
				/* remove crash bug */
				curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
				curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
			}
		}
		return curl_handle;
	}
	
	static void cleanup() {
		curl_easy_cleanup(get_curl_handle());
	}
	
	// Получение html странице в виде строки
	static string getPage(string url, 
	                      string referer_link = "")
	{
		CURL *curl_handle = get_curl_handle();
		string buffer;
		if(curl_handle) {
		    /* set url to get here */
			curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
			
			/* send all data to this function */
			curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, HtmlString::writer);
			
			if(referer_link != "") {
				curl_easy_setopt(curl_handle, CURLOPT_REFERER, referer_link.c_str());
			}else {
				curl_easy_setopt(curl_handle, CURLOPT_REFERER, NULL);
			}
			
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &buffer);
			
			/* get it */
			curl_easy_perform(curl_handle);
		}
		return buffer;
	}
	
	static string urlDecode(string urlEncoded) {
		int urlDecodedSize;
		CURL *curl_handle = get_curl_handle();
		char *urlDecoded = curl_easy_unescape(curl_handle, 
		                                      urlEncoded.c_str(),
		                                      urlEncoded.size(),
		                                      &urlDecodedSize);
		return string(urlDecoded, urlDecodedSize);
	}

};
