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
    SavedItemsController *savedItemsController;
    public:
    ActorsController(ActorsView *view, 
                     DynamicLinksController *dynamicLinksController,
                     SavedItemsController *savedItemsController) {
	    this->view = view;
	    this->savedItemsController = savedItemsController;
	    view->setModel(&model);
	    dynamicLinksController->setModel(&model);
	    savedItemsController->setModel(&model);
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
		model.init(title, href, pixbuf);
		savedItemsController->showSaveOrDelete();
	}
	
	gboolean isActorsActive() {
		return view->isBtnActorsActive();
	}
	
	void startTask() {
		task->start();
	}
	
	void click() {
		view->onActorsClick(model.isEmpty());
	}
};
