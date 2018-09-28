// DynamicLinksController.hpp
#include "DynamicLinksTask.hpp"

class DynamicLinksController {
	DynamicLinksView *view;
	DynamicLinksTask *task;
	PlaylistsTask *playlistsTask;
	PlayItemProcessor *processor;
    ActorsModel *model;
    public:
    DynamicLinksController(DynamicLinksView *view,
                           PlaylistsTask *playlistsTask,
                           PlayItemProcessor *processor) {
		this->view = view;
		this->playlistsTask = playlistsTask;
		this->processor = processor;					   
	}
	
	~DynamicLinksController() {
		g_free(task);
	}
	
	void setModel(ActorsModel *model) {
		this->model = model;
		task = new DynamicLinksTask(view, model);
	}
	
	void btnFilmClicked() {
		processor->process(model->getJs());
	}
	
	void btnSerialClicked() {
		playlistsTask->start(model->getTitle(), model->getJs());
	}
	
	void startTask() {
		task->start();
	}
	
	void hideLinksButtons() {
		view->showEmpty();
	}
};
