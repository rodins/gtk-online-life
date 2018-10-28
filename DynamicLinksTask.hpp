// DynamicLinksTask.hpp
#include "DynamicLinksParser.hpp"

class DynamicLinksTask {
	GThreadPool *pool;
	DynamicLinksView *view;
	ActorsModel *model;
	public:
	DynamicLinksTask(DynamicLinksView *view) {
		this->view = view;
		this->model = NULL;
		pool = g_thread_pool_new(DynamicLinksTask::task,
	                             this,
	                             1, // Run one thread at the time
	                             FALSE,
	                             NULL);
	}
	
	void setModel(ActorsModel *model) {
		this->model = model;
	}

    void start() {
		if(model != NULL) {
			g_thread_pool_push(pool, (gpointer)1, NULL);
		}
	}
	
	private:
	static void task(gpointer arg, gpointer arg2) {
		DynamicLinksTask *task = (DynamicLinksTask*)arg2;
	    // On pre execute
		gdk_threads_enter();
		string actorsUrl = task->model->getUrl();
		string playerUrl = task->model->getPlayerUrl();
		// Show links spinner
		task->view->showLoadingIndicator();
		gdk_threads_leave();	
		string player = HtmlString::getPage(playerUrl, actorsUrl);
		//cout << "Player: " << player << endl; // use it if you ever want to try to parse player
		string urlEncoded = DynamicLinksParser::parsePlayerForUrl(player);
		string browserUrl = HtmlString::urlDecode(urlEncoded);
		
		if(!browserUrl.empty()) {
			task->view->showFilmButton();
			task->model->setLinksMode(LINKS_MODE_FILM);
			task->model->setBrowserUrl(browserUrl);
		}else {
			task->view->showError();
			task->model->setLinksMode(LINKS_MODE_ERROR);
		}
	}
};
