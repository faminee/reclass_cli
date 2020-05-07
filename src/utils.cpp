#include "utils.h"

bool utils::is_integer(const std::string& s) {
	return std::regex_match(s, std::regex("[(-|+)|][0-9]+"));
}

uint16_t utils::type_to_size(handle* h, const std::string& s) {
	uint8_t platform = 0;
	if(!h->is_64bit) {
		platform = 2;
	}
	if(s.back() == '*') {
		return sizeof(uintptr_t) / platform;
	}
	if(s == "uintptr_t") {
		return sizeof(uintptr_t) / platform;
	}
	if(s == "int8_t") {
		return sizeof(int32_t);
	}
	if(s == "uint8_t") {
		return sizeof(int32_t);
	}
	if(s == "int32_t") {
		return sizeof(int32_t);
	}
	if(s == "uint32_t") {
		return sizeof(int32_t);
	}
	if(s == "int16_t") {
		return sizeof(int16_t);
	}
	if(s == "uint16_t") {
		return sizeof(int16_t);
	}
	if(s == "int64_t") {
		return sizeof(int64_t);
	}
	if(s == "uint64_t") {
		return sizeof(int64_t);
	}
	if(s == "bool") {
		return sizeof(bool);
	}
	if(s == "float") {
		return sizeof(float);
	}
	return 0;
}

std::string utils::strip_leading_zeros(const std::string& s) {
	return s.substr(s.find_first_not_of('0'), s.size());
}

std::vector<std::string> utils::split(const std::string& s, char delimeter) {
	std::istringstream stream(s);
	std::vector<std::string> tokens;
	std::string token;
	while(std::getline(stream, token, delimeter)) {
		tokens.push_back(token);
	}
	return tokens;
}

uint16_t utils::terminal::n_row = 0;
uint16_t utils::terminal::n_col = 0;

void utils::terminal::update_terminal_size() {
	struct winsize terminal_size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size);
	utils::terminal::n_row = terminal_size.ws_row;
	utils::terminal::n_col = terminal_size.ws_col;
}

void utils::terminal::clear() {
	for(int i = 0; i < utils::terminal::n_row; i++) {
		std::cout << "\n";
	}
}