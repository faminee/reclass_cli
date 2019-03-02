#include "interface.h"

tab::tab(interface* i, const std::string& name) : m_interface(i), m_name(name) {
}

void tab::input(WINDOW* w) {
	wchar_t c = wgetch(this->m_interface->tab_view);
	if(c == KEY_RIGHT && this->m_interface->current_tab != this->m_interface->m_tabs.data() + this->m_interface->m_tabs.size() - 1) {
		this->m_interface->current_tab++;
	}
	if(c == KEY_LEFT && this->m_interface->current_tab != this->m_interface->m_tabs.data()) {
		this->m_interface->current_tab--;
	}
	this->custom_input(c);
}

memory_view::memory_view(interface* i, const std::string& name, handle* h, uintptr_t base_address, const std::string& class_name) : tab(i, name), m_handle(h), m_base_address(base_address), m_class_name(class_name) {
	this->m_reconstructed_data.push_back(reconstructed_data());
	this->m_current_reconstruction = this->m_reconstructed_data.data();
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

void memory_view::custom_input(wchar_t c) {
	if(c == KEY_UP && this->selected_line != 0) {
		this->selected_line--;
	}
	if(c == KEY_DOWN && this->selected_line != this->m_bytes_to_show / 4) {
		this->selected_line++;
	}
	if(c == 'p') {
		this->m_bytes_to_show += 64;
	}
	static bool in_reconstruction = false;
	if(c == 'o') {
		in_reconstruction = !in_reconstruction;
	}
	if(c == KEY_ENTER && in_reconstruction) {
		if(this->m_current_reconstruction->type == "") {
			// grab type from selected window
		}
		if(this->m_current_reconstruction->name == "") {
			// grab name from dialog
			this->m_current_reconstruction->offset = this->selected_line * 4;
			in_reconstruction = false;
		}
	}
	if(c == KEY_END) {
		bool has_code_gen = false;
		for(const tab* const t : this->m_interface->m_tabs) {
			const generated_code_view* const code_view = dynamic_cast<const generated_code_view* const>(t);
			if(code_view && code_view->m_memory_view == this) {
				has_code_gen = true;
			}
		}
		if(!has_code_gen) {
			this->m_interface->add_tab(new generated_code_view(this->m_interface, "generated_code_view", this));
		}
	}
}

generated_code_view::generated_code_view(interface* i, const std::string& name, memory_view* m) : tab(i, name), m_memory_view(m) {
}

void generated_code_view::print(WINDOW* w) {
	std::string code = "class " + this->m_memory_view->m_class_name + " {\n public:\n ";
	std::sort(this->m_memory_view->m_reconstructed_data.begin(), this->m_memory_view->m_reconstructed_data.end(), reconstructed_data());
	for(auto it = this->m_memory_view->m_reconstructed_data.begin(); it != this->m_memory_view->m_reconstructed_data.end(); it++) {
		std::string pad;
		if(it == this->m_memory_view->m_reconstructed_data.begin()) {
			if(it->offset) {
				pad = "uint8_t[0x" + utils::strip_leading_zeros(utils::to_hex_string(it->offset)) + "];\n ";
			}
		}else{
			pad = "uint8_t[0x" + utils::strip_leading_zeros(utils::to_hex_string(it->offset - (it - 1)->offset) + "];\n ");
		}
		code += pad + it->type + " " + it->name + ";\n" ;
	}
	mvwprintw(w, 1, 1, (code + " };").c_str());
}

void generated_code_view::custom_input(wchar_t c) {
}

interface::interface(handle* h, uintptr_t base_address, const std::string& class_name) {
	this->add_tab(new memory_view(this, "memory_view", h, base_address, class_name));
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	this->tab_view = newwin(1, utils::terminal::n_col, 0, 0);
	keypad(this->tab_view, true);
	nodelay(this->tab_view, true);
	this->view = newwin(utils::terminal::n_row, utils::terminal::n_col, 1, 0);
	keypad(this->view, true);
	nodelay(this->view, true);
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
	this->current_tab = this->m_tabs.data() + this->m_tabs.size() - 1;
}

void interface::tick() {
	werase(this->tab_view);
	werase(this->view);
	(*(this->current_tab))->print(this->view);
	box(this->view, 0, 0);
	for(const tab* const t : this->m_tabs) {
		if(t == *this->current_tab) {
			wattron(this->tab_view, A_REVERSE);
		}
		wprintw(this->tab_view, std::string("[" + t->m_name + "]").c_str());
		wattroff(this->tab_view, A_REVERSE);
	}
	(*(this->current_tab))->input(this->view);
	wrefresh(this->tab_view);
	wrefresh(this->view);
}