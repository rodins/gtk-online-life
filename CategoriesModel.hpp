// CategoriesModel.hpp
#include "CategoriesParser.hpp"

class CategoriesModel {
    GtkWidget *tvCategories;
    public:
    CategoriesModel(GtkWidget *tvCategories) {
	    this->tvCategories = tvCategories;	
	}
	
	void setData(string page) {
		CategoriesParser parser(page);
		GtkTreeModel *model;
		model = parser.getModel();
	    gtk_tree_view_set_model(GTK_TREE_VIEW(tvCategories), model);
		g_object_unref(model);
	}
};
