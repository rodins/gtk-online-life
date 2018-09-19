// ResultsRepository.hpp
#include "ResultsParser.hpp"
#include "ResultsNet.hpp" 
#include "ResultsHistory.hpp"

class ResultsRepository {
    CenterView *view;
    GThreadPool *threadPool;
    ResultsModel model;
    ResultsParser *parser;
    ResultsNet *net;
    bool isPage;
    string link;
    bool isThreadStarted;
    set<int> *imageIndices;
    ResultsHistory *history;
    
    public:
    ResultsRepository(CenterView *view,
                      map<string, GdkPixbuf*> *imagesCache,
                      set<int> *imageIndices) {
		this->view = view;
		parser = new ResultsParser(view);
	    net = new ResultsNet(parser);
	    history = new ResultsHistory(view);
	    isPage = FALSE;
	    isThreadStarted = FALSE;
	    model.setImagesCache(imagesCache);
	    this->imageIndices = imageIndices;
		threadPool = g_thread_pool_new(ResultsRepository::resultsTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	}
	
	~ResultsRepository() {
		free(history);
		free(net);
		free(parser);
	}
	
	void btnPrevClicked() {
		history->saveToForwardStack(model);
		model = history->restoreFromBackStack();
		setModel();
	}
	
	void btnNextClicked() {
		history->saveToBackStack(model);
		model = history->restoreFromForwardStack();
		setModel();
	}
	
	// New result: give link and title
	void getData(string title, string link) {
		this->link = link;
		isPage = FALSE;
		history->saveToBackStack(model);
		history->clearForwardStack();
		model.init(title, link);
		setModel();
		getLinkData();
	}
	
	void refresh() {
		isPage = FALSE;
		model.clearModel();
		link = model.getUrl();
		getLinkData();
	}
	
	void repeat() {
		getLinkData();
	}
	
	// Append results, use model next link
	void getData() {
		isPage = TRUE;
		link = model.getNextLink();
		getLinkData();
	}
	
	private:
	
	void setModel() {
		view->setResultsModel(model);
		parser->setModel(&model);
		imageIndices->clear();
	}
	
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
				view->showResultsData();
			}else {
				view->setSensitiveReferesh();
				history->removeBackStackDuplicate(model.getTitle());
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
