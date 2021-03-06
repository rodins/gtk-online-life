// DynamicLinksController.hpp
#include "DynamicLinksTask.hpp"

class DynamicLinksController {
	DynamicLinksView *view;
	DynamicLinksTask *task;
	PlayItemProcessor *processor;
    ActorsModel *model;
    public:
    DynamicLinksController(DynamicLinksView *view,
                           PlayItemProcessor *processor) {
		this->view = view;
		this->processor = processor;
		task = new DynamicLinksTask(view);					   
	}
	
	~DynamicLinksController() {
		g_free(task);
	}
	
	void setModel(ActorsModel *model) {
		this->model = model;
		task->setModel(model);
		showLinksType();
	}
	
	void btnFilmClicked() {
		processor->process(model->getBrowserUrl());
	}
	
	void startTask() {
		if(!model->getPlayerUrl().empty()) {
			task->start();
		}
	}
	
	void hideLinksButtons() {
		view->showEmpty();
	}
	
	private:
	void showLinksType() {
		//Links button change
	    switch(model->getLinksMode()) {
			case LINKS_MODE_FILM:
			    view->showFilmButton();
			break;
			case LINKS_MODE_EMPTY:
			    view->showEmpty();
			break;
			case LINKS_MODE_ERROR:
			    view->showError();
		}
	}
};
