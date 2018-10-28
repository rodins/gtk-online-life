// DynamicLinksParser.hpp

class DynamicLinksParser {
	public:
    static string parsePlayerForUrl(string &player) {
		size_t begin = player.find("http%3A");
		size_t end = player.find("\"", begin);
		if(begin != string::npos && end != string::npos) {
			size_t length = end - begin;
		    return player.substr(begin, length);
		}
		return "";
	}
	
};
