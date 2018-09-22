// PlaylistsTask.hpp
#include "PlaylistsModel.hpp"
#include "PlaylistsParser.hpp"

class PlaylistsTask {
    CenterView *view;
    PlaylistsModel *model;
    PlaylistsParser *parser;
    GThreadPool *threadPool;
    string title;
    string link;
    public:
    PlaylistsTask(CenterView *view, GtkTreeModel *treemodel) {
		this->view = view;
		model = new PlaylistsModel(treemodel);
		parser = new PlaylistsParser(model);
	    threadPool = g_thread_pool_new(PlaylistsTask::task,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	}
	
	~PlaylistsTask() {
		g_free(parser);
		g_free(model);
	}
    
    void start(string title, string js) {
		this->title = title;
		link = PlaylistsUtils::get_txt_link(js);
		if(!link.empty()) { // Playlists found
			g_thread_pool_push(threadPool, (gpointer)1, NULL);
		}
	}
	
	private:
	
	static void task(gpointer args, gpointer args2) {
		PlaylistsTask *task = (PlaylistsTask*)args2;
		
		// On pre execute
		gdk_threads_enter();
		// Show spinner fullscreen
		task->view->showLoadingIndicator(FALSE);
		gdk_threads_leave();
		
		string json = HtmlString::getPage(task->link);
		
		gdk_threads_enter();
		task->parser->parse(json);
		if(!task->model->isEmpty()) {
		    task->view->setTitle(task->title);
			task->view->showPlaylistsData();
		}else {
			task->view->showError(FALSE);
		}
		gdk_threads_leave();
	}
};
