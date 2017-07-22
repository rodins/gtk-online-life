class Item {
	string _title;
	string _href;
	string _image_link;
	public:
	Item() {
		_title = _href = "";
	}
	
	Item(string title, string href, string image) {
		_title = to_utf8(title);
		_href = href;
		_image_link = image + "&w=165&h=236&zc=1"; //set size of image
		//"&w=41&h=59&zc=1"  "&w=82&h=118&zc=1"
	}
	
	Item(string title, string href) { //constructor for actors, do not need to convert to utf8
		_title = title;
		_href = href;
	}
	
	string& get_href() {
		return _href;
	}
	
	string& get_title() {
		return _title;
	}
	
	string& get_image_link() {
		return _image_link;
	}

};
