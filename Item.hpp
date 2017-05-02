class Item {
	string _title;
	string _id;
	string _href;
	public:
	Item() {
		_title = _id = _href = "";
	}
	
	Item(string title, string id, string href) {
		_title = to_utf8(title);
		_id = id;
		_href = href;
	}
	
	Item(string title, string href) { //constructor for actors, do not need to convert to utf8
		_title = title;
		_href = href;
	}
	
	string get_href() {
		return _href;
	}
	
	string get_title() {
		return _title;
	}
	
	string get_id() {
		return _id;
	}
	
	string get_player_link() {
		string player_link = "http://dterod.com/player.php?newsid=";
		return player_link.append(_id);
	}
	
	string get_js_link() {
		string js_link = "http://dterod.com/js.php?id=";
		return js_link.append(_id);
	}
};
