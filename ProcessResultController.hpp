// ProcessResultController.hpp

class ProcessResultController {
	ActorsController *actorsController;
	ConstantLinksTask *constantLinksTask;
    public:
    ProcessResultController(ActorsController *actorsController,
                            ConstantLinksTask *constantLinksTask) {
		this->actorsController = actorsController;
		this->constantLinksTask = constantLinksTask;
	}
	
	void onClick(string title, string href, GdkPixbuf *pixbuf) {
		actorsController->setResultData(title, href);
		if(actorsController->isActorsActive()) {
			actorsController->startTask();
		}else {
			constantLinksTask->start(title, href);
		}
	}
    
};
