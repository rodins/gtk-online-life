// CategoriesNet.hpp

class CategoriesNet {
    CURL *curl_handle;
    CategoriesParser *parser;
    public:
    CategoriesNet(CategoriesParser *parser) {
		this->parser = parser;
		/* init the curl session */
		curl_handle = curl_easy_init();
		if(curl_handle) {
			curl_easy_setopt(curl_handle, 
			                 CURLOPT_URL, 
			                 DomainFactory::getDomain().c_str());
			/* remove crash bug */
			curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
			curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl_handle, 
			                 CURLOPT_WRITEFUNCTION, 
			                 CategoriesNet::writer);
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, parser);
		}
	}
	
	void getData() {
		CURLcode res;
		if(curl_handle) {
			/* get it */
			res = curl_easy_perform(curl_handle);
			if(res == CURLE_WRITE_ERROR) { // This error means OK
				parser->parsePage();
				if(!isEmpty()) { // Handle is no longer needed
					/* cleanup curl stuff */
	                curl_easy_cleanup(curl_handle);
				}
			}
		}
	}
	
	gboolean isEmpty() {
		return parser->isModelEmpty();
	}
	
	private:
	static int writer(char *data, size_t size, size_t nmemb,
	                      CategoriesParser *parser) {
	    if(parser == NULL) {
			return 0;
		}
		
	    gboolean end = parser->parseData(data);
	    if(end) {
			return CURL_READFUNC_ABORT;
		}
	    return size*nmemb;						  
	}
};
