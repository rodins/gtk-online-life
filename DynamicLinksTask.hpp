// DynamicLinksTask.hpp

class DynamicLinksTask {
	GThreadPool *pool;
	DynamicLinksView *view;
	ActorsModel *model;
	string url, playerUrl;
	public:
	DynamicLinksTask(DynamicLinksView *view, ActorsModel *model) {
		this->view = view;
		this->model = model;
		pool = g_thread_pool_new(DynamicLinksTask::task,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	}

    void start() {
		g_thread_pool_push(pool, (gpointer)1, NULL);
	}
	
	private:
	static void task(gpointer arg, gpointer arg2) {
		DynamicLinksTask *task = (DynamicLinksTask*)arg2;
	    // On pre execute
		gdk_threads_enter();
		string href = task->model->getUrl();
		string referer = task->model->getPlayerUrl();
		// Show links spinner
		task->view->showLoadingIndicator();
		gdk_threads_leave();	
		string player = HtmlString::getPage(referer);
		string url = PlaylistsUtils::parsePlayerForUrl(player);
		string js = HtmlString::getPage(url, referer);
		gdk_threads_enter();
		task->model->setJs(js);
		if(js.length() > 1000) { // Serial
            task->view->showSerialButton();
		}else { // Not serial
		    if(js.length() > 500) { // Movie
			    task->view->showFilmButton();
			}else if(js.length() > 0) {
				task->view->showEmpty();
		    }else{ //  Links error
			    task->view->showError();
		    }
		}
		gdk_threads_leave();
	}
};
