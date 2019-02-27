#pragma once

#include <curses.h>

#include "handle.h"

class interface;

class tab {
public:
	tab(interface* i, const std::string& name);
	virtual void print(WINDOW* w) = 0;
	void input(WINDOW* w);
	virtual void custom_input(wchar_t c) = 0;
	const std::string m_name;
	interface* m_interface;
};

struct reconstructed_data {
	std::string type;
	std::string name;
	uintptr_t offset;
};

class memory_view : public tab {
public:
	memory_view(interface* i, const std::string& name, handle* h, uintptr_t base_address, const std::string& class_name);
	void print(WINDOW* w) override;
	void custom_input(wchar_t c) override;
	const std::string m_class_name;
	std::vector<reconstructed_data> m_reconstructed_data;
private:
	uint32_t selected_line = 0;
	uintptr_t m_base_address;
	handle* m_handle;
	uint64_t m_bytes_to_show = 64;
};

class generated_code_view : public tab {
public:
	generated_code_view(interface* i, const std::string& name, memory_view* m);
	void print(WINDOW* w) override;
	void custom_input(wchar_t c) override;
private:
	memory_view* m_memory_view;
};

class interface {
public:
	interface(handle* h, uintptr_t base_address, const std::string& class_name);
	~interface();
	void add_tab(tab* t);
	void tick();
	WINDOW* tab_view;
	WINDOW* view;
	tab** current_tab;
	std::vector<tab*> m_tabs;
};