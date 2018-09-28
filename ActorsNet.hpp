// ActorsNet.hpp

class ActorsNet {
    ActorsParser *parser;
    CURL *curl_handle;
    public:
    ActorsNet(ActorsParser *parser) {
		this->parser = parser;
		/* init the curl session */
		curl_handle = curl_easy_init();
		if(curl_handle) {
			/* remove crash bug */
			curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
			curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl_handle, 
			                 CURLOPT_WRITEFUNCTION, 
			                 ActorsNet::writer);
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, parser);
		}
	}
	
	~ActorsNet() {
		/* cleanup curl stuff */
	    curl_easy_cleanup(curl_handle);
	}
	
	ActorsParser* getParser() {
		return parser;
	}
	
	CURLcode getData() {
		CURLcode res;
		if(curl_handle) {
			parser->init();
			string url = parser->getModel()->getUrl();
			curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
			/* get it */
			res = curl_easy_perform(curl_handle);
			gdk_threads_enter();
			if(res == CURLE_WRITE_ERROR) { // This error means OK
				parser->parsePage();
			}
			gdk_threads_leave();
		}
		return res;
	}
	
	private:
	static int writer(char *data, size_t size, size_t nmemb,
	                      ActorsParser *parser) {
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
