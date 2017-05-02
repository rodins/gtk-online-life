class Playlist {
	string _title;
	vector<PlayItem> _items;
	public:
	Playlist(string title) {
		_title = title;
	}
	
	string get_title() {
		return _title;
	}
	
	void push_back(PlayItem item) {
		_items.push_back(item);
	}
	
	vector<PlayItem> get_items() {
		return _items;
	}
};
