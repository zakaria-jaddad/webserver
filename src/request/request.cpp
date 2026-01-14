#include "./request.hpp"

char hexToChar(std::string hex) {
	int ret;
	std::stringstream ss;
	ss << std::hex << hex;
	ss >> ret;
	return (ret);
}

bool isHex(char c) {
	return (!(c < '0' || (c > '9' && c < 'A') || (c > 'F' && c < 'a') || c > 'f'));
}

bool isValidPercentEncoding(std::string str) {
	std::cout << "to check :" << str << std::endl;
	for (size_t i=0; i < str.length(); i++) {
		if (i > 1 || !isHex(str[i]))
			return (0);
	}
	return (1);
}

std::string urlDecode(std::string &str) {
	std::string dec = "";

	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] == '%' && isValidPercentEncoding(str.substr(i+1, 2))) {
			dec += hexToChar(str.substr(i+1, 2));
			i += 2;
		} else if (str[i] == '%') {
			return (""); //error
		}
		else
			dec += str[i];
	}
	return (dec);
}

int HttpParser::parseRequestURI(t_reqURI& reqURI, std::string uri) {
	//t_reqURI *reqURI = new t_reqURI;
	std::stringstream ss(uri);
	std::string param;
	std::string key;
	std::string val;

	getline(ss, reqURI.path, '?');
	while (getline(ss, param, '&')) {
		key = "";
		val = "";
		std::stringstream paramStream(param);
		getline(paramStream, key, '=');
		paramStream >> val;
		reqURI.params[key] = val;
		std::cout << "-------------------" << std::endl;
		std::cout << "key : " << key << std::endl;
		std::cout << "val : " << val << std::endl;
		std::cout << "-------------------" << std::endl;
	}
	reqURI.path = urlDecode(reqURI.path);
	return (0);
}
