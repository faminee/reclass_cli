#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

#include "utils.h"

class handle {
public:
	handle(uint16_t pid);
	uint8_t* rpm(uintptr_t address, uint64_t size);
	const uint16_t m_pid;
	std::string m_process_name;
};