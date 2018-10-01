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
    // Data of last clicked result
    string title, href;
    GdkPixbuf *pixbuf;
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
		this->title = title;
		this->href = href;
		this->pixbuf = pixbuf;
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
		actorsHistoryModel->saveActorsModel(model);
		model.init(title, href, pixbuf);
		setActorsModel();
		task->start();
	}
	
	void click() {
		view->onActorsClick(model.isEmpty());
		if(!href.empty() && href != model.getUrl() && 
		   view->isBtnActorsActive()) {
			startTask();
		} 
	}
	
	private:
	void setActorsModel() {
	    view->setModel(&model);
	    dynamicLinksController->setModel(&model);
	    savedItemsController->setModel(&model);	
	}
};
