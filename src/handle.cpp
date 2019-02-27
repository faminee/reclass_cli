#include "handle.h"

handle::handle(uint16_t pid) : m_pid(pid) {
	std::ifstream istream("/proc/" + std::to_string(this->m_pid) + "/cmdline");
	std::string process_path;
	std::getline(istream, process_path);
	std::vector<std::string> stripper = utils::split(process_path, ' ');
	if(stripper.size() > 0) {
		process_path = stripper.front();
	}
	stripper = utils::split(process_path, '/');
	if(stripper.size() > 0) {
		process_path = stripper.back();
	}
	m_process_name = process_path.substr(0, 48);
}

uint8_t* handle::rpm(uintptr_t address, uint64_t size) {
	struct iovec local;
	struct iovec remote;
	uint8_t* memory = new uint8_t[size];
	local.iov_base = memory;
	local.iov_len = size;
	remote.iov_base = reinterpret_cast<void*>(address);
	remote.iov_len = size;
	process_vm_readv(this->m_pid, &local, 1, &remote, 1, 0);
	return memory;
}