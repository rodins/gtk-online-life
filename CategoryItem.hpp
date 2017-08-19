class CategoryItem {
    string _title;
    string _link;
    vector<CategoryItem> _subctgs;
    public:
	
	void set_title(string title) {
		_title = title;
	}
	
	string get_title() {
		return _title;
	}
	
	void set_link(string link) {
		string domain = "http://www.online-life.club";
		if(link.find(domain) == string::npos) {
			_link = domain + link;
		}else {
			_link = link;
		}
	}
	
	string& get_link() {
		return _link;
	}
	
	void push_back(CategoryItem item) {
		_subctgs.push_back(item);
	}
	
	vector<CategoryItem>& get_subctgs() {
		return _subctgs;
	}
};
