// DomainFactory.hpp

class DomainFactory {
    public:
    
    static std::string getDomain() {
		return "http://online-life.club";
	}
	
	static std::string getWwwDomain() { // For categories parser
		return "http://www.online-life.club";
	}
	
	static std::string getWwwDomainNoSuffix() {
		return "http://www.online-life.";
	}
};
