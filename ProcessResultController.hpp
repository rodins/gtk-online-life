// ProcessResultController.hpp

class ProcessResultController {
	ActorsController *actorsController;
    public:
    ProcessResultController(ActorsController *actorsController) {
		this->actorsController = actorsController;
	}
	
	void onClick(string title, string href, GdkPixbuf *pixbuf) {
		actorsController->setResultData(title, href, pixbuf);
		actorsController->startTask();
	}
    
};
