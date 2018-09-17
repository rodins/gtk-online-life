// ResultsRepository.hpp
#include "ResultsParser.hpp"
#include "ResultsNet.hpp" 

class ResultsRepository {
    CenterView *view;
    GThreadPool *threadPool;
    ResultsModel model;
    ResultsParser *parser;
    ResultsNet *net;
    bool isPage;
    int modelCount;
    string link;
    bool isThreadStarted;
    
    public:
    ResultsRepository(CenterView *view, 
                      map<string, GdkPixbuf*> *imagesCache) {
		this->view = view;
		parser = new ResultsParser(view, &model);
	    net = new ResultsNet(parser);
	    isPage = FALSE;
	    isThreadStarted = FALSE;
	    modelCount = 0;
	    model.setImagesCache(imagesCache);
		threadPool = g_thread_pool_new(ResultsRepository::resultsTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	}
	
	~ResultsRepository() {
		free(net);
		free(parser);
	}
	
	// New result: give link and title
	void getData(string title, string link) {
		this->link = link;
		isPage = FALSE;
		model.init(modelCount++, title, link);
		view->setResultsModel(model);
		getLinkData();
	}
	
	// Append results, use model next link
	void getData() {
		isPage = TRUE;
		link = model.getNextLink();
		getLinkData();
	}
	
	private:
	
	void getLinkData() {
		if(link.empty()) {
			return;
		}
		if(!isThreadStarted) {
			isThreadStarted = TRUE;
			view->showLoadingIndicator(isPage);
			parser->resetFirstItem(); // needed to hide loading indicator
		    g_thread_pool_push(threadPool,
		                      (gpointer)link.c_str(),
		                       NULL);
		}
	}
	
	void onPostExecute(CURLcode res) {
		isThreadStarted = FALSE;
		if(res == CURLE_OK) {
			if(model.isEmpty()) {
				//view->showEmpty();
			}
		}else {
			view->showError(isPage);
		}
	}
	
	static void resultsTask(gpointer arg, gpointer arg1) {
		// On pre execute
		gdk_threads_enter();
		string url((char*)arg);
		ResultsRepository *repo = (ResultsRepository*)arg1;
	    gdk_threads_leave();
	    // async part
		CURLcode res = repo->net->getResultsFromNet(url);
		gdk_threads_enter();
		repo->onPostExecute(res);                                 
		gdk_threads_leave();
	}  
};
