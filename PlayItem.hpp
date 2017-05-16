class PlayItem {
	string _file;
	string _download;
	string _comment;
	public:
	PlayItem() {
		_file = _download = _comment = "";
	}
	
	string get_file() {
		return _file;
	}
	
	void set_file(string file) {
		_file = file;
	}
	
	string get_download() {
		return _download;
	}
	void set_download(string download) {
		_download = download;
	}
	
	string get_comment() {
		return _comment;
	}
	void set_comment(string comment) {
		_comment = comment;
	}
	
	string get_title() {
		return _comment;
	}
};