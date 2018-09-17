// ResultsNet.hpp

class ResultsNet {
	CURL *curl_handle;
	public:
	ResultsNet(ResultsParser *parser) {
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
	
	CURLcode getResultsFromNet(string url) {
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
	    parser->divs_parser(data);
	    return size*nmemb;
	}
};
