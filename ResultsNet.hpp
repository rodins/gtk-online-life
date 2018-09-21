// ResultsNet.hpp

class ResultsNet {
	CURL *curl_handle;
	ResultsParser *parser;
	string url;
	public:
	ResultsNet(ResultsParser *parser) {
		this->parser = parser;
		/* init the curl session */
		curl_handle = curl_easy_init();	
		if(curl_handle) {
			/* remove crash bug */
			curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
			
			/* send all data to this function */		
			curl_easy_setopt(curl_handle, 
			                 CURLOPT_WRITEFUNCTION, 
			                 ResultsNet::results_writer);
			curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);	
			curl_easy_setopt(curl_handle,
			                 CURLOPT_WRITEDATA, 
			                 parser);
		}	
	}
	
	~ResultsNet() {
		/* cleanup curl stuff */
	    curl_easy_cleanup(curl_handle);
	}
	
	void resetFirstItem() {
		parser->resetFirstItem();
	}
	
	gboolean isEmpty() {
		return parser->isEmpty();
	}
	
	void setMode(gboolean isPage) {
		if(isPage) {
			url = parser->getNextLink();
		}else {
			url = parser->getLink();
		}
	}
	
	CURLcode getResultsFromNet() {
		CURLcode res;		
		if(curl_handle) {		    
		    /* set url to get here */
			curl_easy_setopt(curl_handle, 
			                 CURLOPT_URL, 
			                 url.c_str());			
			                 			
			/* get it */
			res = curl_easy_perform(curl_handle);
		}
		return res;
	}
	
	private:
	
	static int results_writer(char *data, size_t size, size_t nmemb,
	                      ResultsParser *parser)
	{   
		if(parser == NULL) {
			return 0;
		}
		
	    parser->divs_parser(data);
	    return size*nmemb;
	}
};
