// DynamicLinksParser.hpp

class DynamicLinksParser {
	public:
    static string parsePlayerForUrl(string &player) {
		size_t begin = player.find("ref_url:");
		size_t end = player.find("\"", begin+15);
		if(begin != string::npos && end != string::npos) {
			size_t length = end - begin - 10;
		    return player.substr(begin+10, length);
		}
		return "";
	}
	
};
