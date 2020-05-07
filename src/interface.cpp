#include "interface.h"

tab::tab(interface* i, const std::string& name) : m_interface(i), m_name(name) {
}

void tab::input(WINDOW* w) {
	wchar_t c = wgetch(this->m_interface->tab_view);
	if(this->m_interface->m_menu.active) {
		this->m_interface->m_menu.input(c);
	}else{
		if(c == KEY_RIGHT && this->m_interface->current_tab != this->m_interface->m_tabs.data() + this->m_interface->m_tabs.size() - 1) {
			this->m_interface->current_tab++;
		}
		if(c == KEY_LEFT && this->m_interface->current_tab != this->m_interface->m_tabs.data()) {
			this->m_interface->current_tab--;
		}
		this->custom_input(c);
	}
}

memory_view::memory_view(interface* i, const std::string& name, handle* h, uintptr_t base_address, const std::string& class_name) : tab(i, name), m_handle(h), m_base_address(base_address), m_class_name(class_name) {
	this->m_reconstructed_data.push_back(reconstructed_data());
	this->m_current_reconstruction = this->m_reconstructed_data.data();
}

void memory_view::print(WINDOW* w) {
	mvwprintw(w, 1, 1, std::string("[0x" + utils::to_hex_string(this->m_base_address) + "]	" + "Class	" + this->m_class_name + "	[" + std::to_string(this->m_bytes_to_show) +"]").c_str());
	uint8_t* memory = this->m_handle->rpm(this->m_base_address, this->m_bytes_to_show);
	uint16_t terminal = utils::terminal::n_row - 4;
	uint16_t delta = 0;
	if(this->selected_line > terminal) {
		delta = this->selected_line - terminal;
	}
	for(uintptr_t i = delta; i <= this->m_bytes_to_show / 4; i++) {
		if(this->selected_line == i) {
			wattron(w, A_REVERSE);
		}
		uintptr_t current_address = this->m_base_address + i * 4;
		mvwprintw(w, 2 + i - delta, 1, std::string(utils::to_hex_string(current_address) + "	" + utils::to_hex_string(i * 4) + "	").c_str());
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
	static bool in_reconstruction = false;
	if(c == 'o') {
		this->m_interface->m_menu.callee = this;
		this->m_interface->m_menu.active = true;
	}
	if(this->m_interface->m_menu.active) {
		return;
	}
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

std::vector<reconstructed_data> sort(std::vector<reconstructed_data> data) {
	data.pop_back();
	sort(data.begin(), data.end(), reconstructed_data());
	return data;
}

void generated_code_view::print(WINDOW* w) {
	std::string code = "class " + this->m_memory_view->m_class_name + " {\n public:\n";
	std::vector<reconstructed_data> data = sort(this->m_memory_view->m_reconstructed_data);
	for(const reconstructed_data& r : data) {
		const reconstructed_data* it = &r;
		std::string pad = " ";
		if(it == data.data()) {
			if(it->offset) {
				pad = "uint8_t[0x" + utils::strip_leading_zeros(utils::to_hex_string(it->offset)) + "];\n ";
			}else if(this->m_memory_view->m_base_address) {
				pad = "uint8_t[0x" + utils::strip_leading_zeros(utils::to_hex_string(this->m_memory_view->m_base_address)) + "];\n ";
			}
		}else{
			pad = "uint8_t[0x" + utils::strip_leading_zeros(utils::to_hex_string(it->offset - (it - 1)->offset - utils::type_to_size(this->m_memory_view->m_handle, (it - 1)->type)) + "];\n ");
		}
		code += pad + it->type + " " + it->name + "; // 0x" + utils::to_hex_string(it->offset) + "\n";
	}
	mvwprintw(w, 1, 1, (code + " };").c_str());
}

void generated_code_view::custom_input(wchar_t c) {
}

menu::menu() {
}

menu::menu(WINDOW* w) : window(w), callee(nullptr) {
}

void menu::print() {
}

#define KEY_ESC 27

uint8_t print_window_scaled(WINDOW* w, uint8_t scale_factor, const std::string& s, uint8_t y = 1) {
	int x = getmaxx(w) / scale_factor - s.size() / 2;
	mvwprintw(w, y, x, s.c_str());
	return x + s.size();
}

void menu::input(wchar_t c) {
	if(c == KEY_ESC) {
		this->active = false;
		this->text_input.clear();
	}
	std::locale locale;
	if(std::isalpha(c, locale) || std::isdigit(c, locale) || std::ispunct(c, locale)) {
		this->text_input += c;
	}
	if(c == KEY_BACKSPACE || c == 127 || c == '\b') {
		if(!this->text_input.empty()) {
			this->text_input.pop_back();
		}
	}
	print_window_scaled(this->window, 2, "Reconstruction");
	uint8_t input_y = getmaxy(this->window) / 2;
	if(dynamic_cast<memory_view*>(this->callee)) {
		memory_view* callee = dynamic_cast<memory_view*>(this->callee);
		if(c == KEY_ESC) {
			callee->m_current_reconstruction->clear();
		}
		if(callee->m_current_reconstruction->type == "") {
			uint8_t offset = print_window_scaled(this->window, 4, "Type:", input_y);
			mvwprintw(this->window, input_y, offset + 1, this->text_input.c_str());
			if(c == '\n') {
				if(utils::type_to_size(callee->m_handle, this->text_input)) {
					 callee->m_current_reconstruction->type = this->text_input;
					 this->text_input.clear();
					 if(callee->m_current_reconstruction->type == "*") {
					 	callee->m_current_reconstruction->type = "uintptr_t";
					 }
				}
			}
			return;
		}
		if(callee->m_current_reconstruction->name == "" && callee->m_current_reconstruction->type != "") {
			uint8_t offset = print_window_scaled(this->window, 4, "Name:", input_y);
			mvwprintw(this->window, input_y, offset + 1, this->text_input.c_str());
			if(c == '\n') {
				callee->m_current_reconstruction->name = this->text_input;
				callee->m_current_reconstruction->offset = callee->selected_line * 4;
				callee->m_reconstructed_data.push_back(reconstructed_data());
				callee->m_current_reconstruction = &callee->m_reconstructed_data.back();
				this->text_input.clear();
				this->callee = nullptr;
				this->active = false;
			}
			return;
		}
		return;
	}
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
	this->m_menu = menu(newwin(6, 33, utils::terminal::n_row / 4, utils::terminal::n_col / 4 + 16));
	keypad(this->m_menu.window, true);
	nodelay(this->m_menu.window, true);
	this->m_menu.active = false;
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
	werase(this->m_menu.window);
	(*(this->current_tab))->print(this->view);
	box(this->view, 0, 0);
	for(const tab* const t : this->m_tabs) {
		if(t == *this->current_tab) {
			wattron(this->tab_view, A_REVERSE);
		}
		wprintw(this->tab_view, std::string("[" + t->m_name + "]").c_str());
		wattroff(this->tab_view, A_REVERSE);
	}
	if(this->m_menu.active) {
		box(this->m_menu.window, 0, 0);
	}
	(*(this->current_tab))->input(this->view);
	wrefresh(this->tab_view);
	wrefresh(this->view);
	if(this->m_menu.active) {
		wrefresh(this->m_menu.window);
	}
}