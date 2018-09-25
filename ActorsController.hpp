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
    public:
    ActorsController(ActorsView *view) {
	    this->view = view;
	    view->setModel(&model);
	    parser = new ActorsParser(&model);
	    net = new ActorsNet(parser);
	    task = new ActorsTask(view, net);		 
	}
	
	~ActorsController() {
		g_free(task);
		g_free(net);
		g_free(parser);
	}
	
	void setResultData(string title, string href) {
		model.init(title, href);
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
