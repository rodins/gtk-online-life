// DynamicLinksTask.hpp

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
		cout << "Player: " << player << endl;
		string urlEncoded = PlaylistsUtils::parsePlayerForUrl(player);
		string url = HtmlString::urlDecode(urlEncoded);
		string js = HtmlString::getPage(url, playerUrl);
		gdk_threads_enter();
		task->model->setJs(js);
		if(js.length() > 1000) { // Serial
            task->view->showSerialButton();
            task->model->setLinksMode(LINKS_MODE_SERIAL);
		}else { // Not serial
		    if(js.length() > 500) { // Movie
			    task->view->showFilmButton();
			    task->model->setLinksMode(LINKS_MODE_FILM);
			}else if(js.length() > 0) {
				task->view->showEmpty();
				task->model->setLinksMode(LINKS_MODE_EMPTY);
		    }else{ //  Links error
			    task->view->showError();
			    task->model->setLinksMode(LINKS_MODE_ERROR);
		    }
		}
		gdk_threads_leave();
	}
};
