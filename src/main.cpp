#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <vector>
#include <unistd.h>
#include <sys/ioctl.h>

#include "utils.h"
#include "interface.h"

std::vector<handle> get_process_list() {
	std::vector<handle> process_list;
	for(const std::experimental::filesystem::directory_entry& directory : std::experimental::filesystem::directory_iterator("/proc/")) {
		std::string pid = utils::split(directory.path(), '/').back();
		if(!utils::is_integer(pid)) {
			continue;
		}
		handle h(std::stoi(pid));
		if(h.m_process_name != "") {
			process_list.push_back(h);
		}
	}
	return process_list;
}

int main() {
	if(getuid()) {
		std::cout << "run as root!\n";
		return 1;
	}
	std::cin.unsetf(std::ios::dec);
	utils::terminal::update_terminal_size();
	std::vector<handle> process_list = get_process_list();
	for(uint32_t i = 0; i < process_list.size(); i++) {
		const handle& process = process_list.at(i);
		std::cout << "	[" << i << "]	" << process.m_process_name << "\n";
	}
	std::cout << "choose process: ";
	uint16_t chosen_process;
	std::cin >> chosen_process;
	if(chosen_process > process_list.size() || chosen_process < 0) {
		utils::terminal::clear();
		std::cout << "invalid process!\n";
		return 1;
	}
	handle* process_handle = new handle(process_list.at(chosen_process).m_pid);
	utils::terminal::clear();
	std::cout << "base address: ";
	uintptr_t base_address;
	std::cin >> base_address;
	std::cout << "class name: ";
	std::string class_name;
	std::cin >> class_name;
	utils::terminal::update_terminal_size();
	interface* i = new interface(process_handle, base_address, class_name);
	while(true) {
		usleep(50000);
		i->tick();
	}
	delete i;
	return 0;
}