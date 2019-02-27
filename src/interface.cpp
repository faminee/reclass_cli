#include "interface.h"

tab::tab(const std::string& name) : m_name(name) {
}

memory_view::memory_view(interface* i, const std::string& name, handle* h, uintptr_t base_address, const std::string& class_name) : tab(name), m_interface(i), m_handle(h), m_base_address(base_address), m_class_name(class_name) {
}

void memory_view::print(WINDOW* w) {
	mvwprintw(w, 1, 1, std::string("[0x" + utils::to_hex_string(this->m_base_address) + "]	" + "Class	" + this->m_class_name + "	[" + std::to_string(this->m_bytes_to_show) +"]").c_str());
	uint8_t* memory = this->m_handle->rpm(this->m_base_address, this->m_bytes_to_show);
	for(uintptr_t i = 0; i <= this->m_bytes_to_show / 4; i++) {
		if(this->selected_line == i) {
			wattron(w, A_REVERSE);
		}
		uintptr_t current_address = this->m_base_address + i * 4;
		mvwprintw(w, 2 + i, 1, std::string(utils::to_hex_string(current_address) + "	" + utils::to_hex_string(i * 4) + "	").c_str());
		for(uintptr_t j = current_address; j < current_address + 4; j++) {
			wprintw(w, std::string("  " + utils::to_hex_string(*(memory + (j - this->m_base_address)))).c_str());
		}
		wprintw(w, std::string("	").c_str());
		uint8_t* byte = memory + i * 4;
		wprintw(w, std::string(std::to_string(*reinterpret_cast<float*>(byte)).substr(0, 12) + "		" + std::to_string(*reinterpret_cast<int32_t*>(byte))).c_str());
		wattroff(w, A_REVERSE);
	}
	delete memory;
}

void memory_view::input(WINDOW* w) {
	wchar_t c = wgetch(w);
	if(c == KEY_UP && this->selected_line != 0) {
		this->selected_line--;
	}
	if(c == KEY_DOWN && this->selected_line != this->m_bytes_to_show / 4) {
		this->selected_line++;
	}
	if(c == 'p') {
		this->m_bytes_to_show += 64;
	}
	if(c == KEY_END) {
		this->m_interface->add_tab(new generated_code_view("generated_code_view", this));
	}
}

generated_code_view::generated_code_view(const std::string& name, memory_view* m) : tab(name), m_memory_view(m) {
}

void generated_code_view::print(WINDOW* w) {
	// std::string code = "class " + this->m_memory_view->m_class_name + "{\npublic:\n";
	// for(reconstructed_data* data = this->m_memory_view->m_reconstructed_data->begin(); data != this->m_memory_view->m_reconstructed_data->end(); data++) {
	// 	std::string pad;
	// 	if(data == this->m_memory_view->m_reconstructed_data->begin()) {
	// 		pad = "uint8_t[" + std::to_string(data.offset) + "]\n";
	// 	}else{
	// 		pad = "uint8_t[" + std::to_string(data.offset - (data - 1)->offset) + "]\n";
	// 	}
	// 	code += pad + code.type + code.name + "\n" + "";
	// }
	// mvwprintw(w, 1, 1 code);
}

void generated_code_view::input(WINDOW* w) {
}

interface::interface(handle* h, uintptr_t base_address, const std::string& class_name) {
	this->add_tab(new memory_view(this, "memory_view", h, base_address, class_name));
	this->current_tab = this->m_tabs.at(0);
	initscr();
	cbreak();
	noecho();
	for(const tab* const t : this->m_tabs) {
		attron(A_REVERSE);
		printw(std::string("[" + t->m_name + "]").c_str());
		attroff(A_REVERSE);
	}
	refresh();
	this->view = newwin(utils::terminal::n_row, utils::terminal::n_col, 1, 0);
	keypad(this->view, true);
	this->tick();
}

interface::~interface() {
	endwin();
	for(const tab* const t : this->m_tabs) {
		delete t;
	}
}

void interface::add_tab(tab* t) {
	this->m_tabs.push_back(t);
	this->current_tab = t;
}

void interface::tick() {
	werase(this->view);
	box(this->view, 0, 0);
	this->current_tab->print(this->view);
	this->current_tab->input(this->view);
	wrefresh(this->view);
}