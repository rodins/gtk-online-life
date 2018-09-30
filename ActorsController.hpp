// ActorsController.hpp
#include "ActorsParser.hpp"
#include "ActorsNet.hpp"
#include "ActorsTask.hpp"

class ActorsController {
	ActorsView *view;
	ActorsModel model;
	ActorsParser *parser;
	ActorsNet *net;
    ActorsTask *task;
    DynamicLinksController *dynamicLinksController;
    SavedItemsController *savedItemsController;
    ActorsHistoryModel *actorsHistoryModel;
    public:
    ActorsController(ActorsView *view, 
                     DynamicLinksController *dynamicLinksController,
                     SavedItemsController *savedItemsController,
                     ActorsHistoryModel *actorsHistoryModel) {
	    this->view = view;
	    this->dynamicLinksController = dynamicLinksController;
	    this->savedItemsController = savedItemsController;
	    this->actorsHistoryModel = actorsHistoryModel;
	    parser = new ActorsParser(&model);
	    net = new ActorsNet(parser);
	    task = new ActorsTask(view, net, dynamicLinksController);	 
	}
	
	~ActorsController() {
		g_free(task);
		g_free(net);
		g_free(parser);
	}
	
	void setResultData(string title, string href, GdkPixbuf *pixbuf) {
		actorsHistoryModel->saveActorsModel(model);
		model.init(title, href, pixbuf);
		setActorsModel();
	}
	
	gboolean isActorsActive() {
		return view->isBtnActorsActive();
	}
	
	void changed(string key) {
		actorsHistoryModel->saveActorsModel(model);
		model = actorsHistoryModel->getActorsModel(key);
		setActorsModel();
	}
	
	void startTask() {
		task->start();
	}
	
	void click() {
		view->onActorsClick(model.isEmpty());
	}
	
	private:
	void setActorsModel() {
	    view->setModel(&model);
	    dynamicLinksController->setModel(&model);
	    savedItemsController->setModel(&model);	
	}
};
