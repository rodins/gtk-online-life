// LinksSizeNet.hpp

class LinksSizeNet {
    CURL *curl;
    public:
    LinksSizeNet() {
		curl = curl_easy_init();
		if(curl) {
			/* No download if the file */ 
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
			curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
		}
	}
	
	~LinksSizeNet() {
		curl_easy_cleanup(curl);
	}
	
	string getLinkSize(string link) {
		CURLcode res;
		double filesize = 0.0;
		
		if(curl) {
			curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
			res = curl_easy_perform(curl);
			
			if(CURLE_OK == res) {
				/* https://curl.haxx.se/libcurl/c/curl_easy_getinfo.html */ 
				res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
									  &filesize);
				if((CURLE_OK == res) && (filesize>0.0)) {
					int nSize = (int)filesize;
					return string(g_format_size(nSize));
				}
			}
		}
		
		return "";
	}
};
