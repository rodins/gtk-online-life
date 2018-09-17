// CategoriesRepository.hpp

class CategoriesRepository {
    GThreadPool *categoriesThreadPool;
    CategoriesView *view;
    CategoriesModel *model;
    gboolean isEmpty;
    public:
    CategoriesRepository(CategoriesView *view, CategoriesModel *model) {
		this->view = view;
		this->model = model;
		isEmpty = TRUE;
		// GThreadPool for categories
	    categoriesThreadPool = g_thread_pool_new(CategoriesRepository::categoriesTask,
	                                             this,
	                                             1, // Run one thread at the time
	                                             FALSE,
	                                             NULL);
	}
	
	void btnCategoriesClicked() {
		view->btnCategoriesClicked();
		if(isEmpty) {
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
			repo->model->setData(page);
			repo->view->showData();
			repo->isEmpty = FALSE;
		}else {
			repo->view->showError();
		}
		gdk_threads_leave();
	}
};
