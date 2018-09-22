// ResultsController.hpp
#include "ResultsParser.hpp"
#include "ResultsNet.hpp" 
#include "ResultsHistory.hpp"
#include "ResultsTask.hpp"

class ResultsController {
	CenterView *view;
    ResultsNet *net;
    ResultsHistory *history;
    ResultsModel model;
    ResultsParser *parser;
    ResultsTask *task;
    PlaylistsTask *playlistsTask; // Needed for error button click
    set<int> *imageIndices;
    public:
    ResultsController(CenterView *view,
                      map<string, GdkPixbuf*> *imagesCache,
                      set<int> *imageIndices,
                      PlaylistsTask *playlistsTask) {
		this->view = view;
		parser = new ResultsParser(view);
		history = new ResultsHistory(view);
	    net = new ResultsNet(parser);
	    task = new ResultsTask(view, net, history);
	    this->playlistsTask = playlistsTask;
	    model.setImagesCache(imagesCache);
	    this->imageIndices = imageIndices;
	}
	
	~ResultsController() {
		g_free(task);
		g_free(net);
		g_free(history);
		g_free(parser);
	}
	
	void btnSavedItemsClicked() {
		if(view->isSavedItemsPressed()) {
			view->showSavedItems();
		}else {
			view->setResultsModel(model);
			history->updatePrevNextButtons();
			if(!model.isEmpty()) {
			    view->setSensitiveReferesh();
			}
		}
	}
	
	void btnUpClicked() {
		view->setSensitiveUp(FALSE);
		view->showResultsData();
		if(!view->isSavedItemsPressed()) {
			history->updatePrevNextButtons();
		}	
	}
	
	void btnPrevClicked() {
		history->saveToForwardStack(model);
		model = history->restoreFromBackStack();
		setModel();
	}
	
	void btnNextClicked() {
		history->saveToBackStack(model);
		model = history->restoreFromForwardStack();
		setModel();
	}
    
    void newResults(string title, string link) {
		if(!task->isStarted()) {
		    history->saveToBackStack(model);
			history->clearForwardStack();
			view->setSavedItemsActive(FALSE);
			model.init(title, link);
			setModel();
			task->setTitle(title); // Needed to remove duplicates from history
			task->newMode();
			task->start();	
		}
	}
	
	void append() {
		if(!task->isStarted() && !view->isSavedItemsPressed()) {
			task->appendMode();
		    task->start();
		}
	}
	
	void repeat() {
		if(task->isError()) {
		    task->start();
		}else {
			playlistsTask->start();
		}
	}
	
	void refresh() {
		model.clearModel();
		task->newMode();
		task->start();
	} 
	
	private:
	
	void setModel() {
		view->showResultsData(); // Needed when switching history from error
		view->setResultsModel(model);
		parser->setModel(&model);
		imageIndices->clear();
	}
};
