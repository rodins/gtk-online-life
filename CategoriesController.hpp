// CategoriesController.hpp

class CategoriesController {
    CategoriesTask *task;
    public:
    CategoriesController(CategoriesTask *task) {
		this->task = task;
	}
	
	void click() {
		task->start();
	}
	
	void repeat() {
	    task->repeat();	
	}
};
