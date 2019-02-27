#include "utils.h"

bool utils::is_integer(const std::string& s) {
	return std::regex_match(s, std::regex("[(-|+)|][0-9]+"));
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