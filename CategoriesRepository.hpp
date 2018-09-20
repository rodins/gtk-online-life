// CategoriesRepository.hpp
#include "CategoriesParser.hpp"

class CategoriesRepository {
    GThreadPool *categoriesThreadPool;
    CategoriesView *view;
    CategoriesModel model;
    CategoriesParser *parser;
    public:
    CategoriesRepository(CategoriesView *view) {
		this->view = view;
		parser = new CategoriesParser(&model);
		// GThreadPool for categories
	    categoriesThreadPool = g_thread_pool_new(CategoriesRepository::categoriesTask,
	                                             this,
	                                             1, // Run one thread at the time
	                                             FALSE,
	                                             NULL);
	}
	
	~CategoriesRepository() {
		g_free(parser);
	}
	
	void btnCategoriesClicked() {
		view->btnCategoriesClicked();
		if(model.isEmpty()) {
			getData();
		}
	}
	
	void btnCategoriesRepeatClicked() {
		getData();
	}
	
	private:
	
	void getData() {
		view->showLoadingIndicator();
		g_thread_pool_push(categoriesThreadPool, (gpointer)1, NULL);
	}
	
	static void categoriesTask(gpointer arg, gpointer arg1) {
		CategoriesRepository *repo = (CategoriesRepository*)arg1;
		string page = HtmlString::getCategoriesPage();
		gdk_threads_enter();
		if(!page.empty()) {
			repo->parser->parse(page);
			if(!repo->model.isEmpty()) {
				repo->view->setModel(repo->model);
			    repo->view->showData();
			}else {
				repo->view->showError();
			}
		}else {
			repo->view->showError();
		}
		gdk_threads_leave();
	}
};
