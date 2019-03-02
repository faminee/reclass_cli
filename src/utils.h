#pragma once

#include <iostream>
#include <vector>
#include <regex>
#include <unistd.h>
#include <sys/ioctl.h>

namespace utils {
	bool is_integer(const std::string& s);
	template<class T>
	std::string to_hex_string(T t) {
		static const char* const digits = "0123456789ABCDEF";
		size_t hex_length = sizeof(t) << 1;
		std::string result(hex_length, '0');
		for(size_t i = 0, j = (hex_length - 1) * 4; i < hex_length; i++, j -= 4) {
			result[i] = digits[(t >> j) & 0x0F];
		}
		return result;
	}
	std::string strip_leading_zeros(const std::string& s);
	std::vector<std::string> split(const std::string& s, char delimeter);
	namespace terminal {
		extern uint16_t current_row;
		extern uint16_t current_col;
		extern uint16_t n_row;
		extern uint16_t n_col;
		void update_terminal_size();
		void print(const std::string& s);
		void print(const char* c);
		void print(const char* c, uint64_t times);
		void clear();
	}
}