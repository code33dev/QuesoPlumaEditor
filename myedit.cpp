#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <regex>

#define CTRL_KEY(k) ((k) & 0x1f)

struct Editor {
    std::vector<std::string> lines;
    std::stack<std::vector<std::string>> undo_stack, redo_stack;
    std::string clipboard;
    int cursor_x = 0, cursor_y = 0;
    int select_x = -1, select_y = -1;
    int offset_y = 0;
    int screen_rows, screen_cols;
    bool selecting = false;
    std::string filename;

    void loadFile(const std::string& fname) {
        filename = fname;
        std::ifstream file(fname);
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        if (lines.empty()) lines.push_back("");
    }
    void saveFile() {
        std::ofstream file(filename);
        for (const auto& line : lines) {
            file << line << "\n";
        }
    }

    void drawEditor() {
        werase(stdscr);
        getmaxyx(stdscr, screen_rows, screen_cols);
        int numWidth = 6;

        // Draw Title Bar
        attron(A_BOLD | COLOR_PAIR(9));
        mvprintw(0, (screen_cols - filename.length() - 12) / 2, "Editing: %s", filename.c_str());
        attroff(A_BOLD | COLOR_PAIR(9));

        for (int i = 1; i < screen_rows - 2; ++i) { // Shift down by 1 for title bar
            int line_index = i + offset_y - 1; // Adjust for title bar shift
            if (line_index >= (int)lines.size() || line_index < 0) break;

            move(i, 0);
            clrtoeol();

            if (line_index == cursor_y) attron(A_REVERSE);
            printw("%*d | ", numWidth - 2, line_index + 1);
            attroff(A_REVERSE);

            highlightSyntax(i, numWidth, lines[line_index]);
        }

        drawStatusBar();
        move(cursor_y - offset_y + 1, cursor_x + numWidth); // Adjust for title bar
        refresh();
    }
    void drawStatusBar() {
        move(screen_rows - 2, 0);
        clrtoeol();
        attron(A_REVERSE);
        printw("[File: %s] Cursor: (%d, %d) | Selection: %s | [Shift+B] Start Selection | [CTRL+B] Stop | [CTRL+C] Copy | [CTRL+X] Cut | [CTRL+V] Paste | [DEL] Delete",
               filename.c_str(), cursor_y + 1, cursor_x, selecting ? "ON" : "OFF");
        attroff(A_REVERSE);
    }

 void highlightSyntax(int row, int col, const std::string &line) {
    static const std::regex control_structures(R"(\b(if|else|switch|case|for|while|do|break|continue|return|goto|try|catch|throw)\b)");
    static const std::regex data_types(R"(\b(int|float|double|char|bool|void|short|long|signed|unsigned|wchar_t|auto|decltype|struct|union|enum|class|template|typename|concept)\b)");
    static const std::regex operators_modifiers(R"(\b(const|static|inline|explicit|virtual|volatile|mutable|constexpr|final|override|private|public|protected|friend|typedef|using|alignas|alignof|sizeof|typeof|thread_local|reinterpret_cast|static_cast|dynamic_cast|const_cast|new|delete|operator|co_await|co_yield|co_return)\b)");
    
    // Fix for `std::sregex_iterator`, `std::vector`, etc.
    static const std::regex stl_methods(R"(\b(std::(?:[a-zA-Z_][a-zA-Z0-9_]*))\b)");  

    static const std::regex preprocessor(R"(\b(#include|#define|#ifdef|#ifndef|#endif|#pragma|#error|#line|#if|#else|#elif|#endif|#undef)\b)");
    static const std::regex literals(R"(\b(true|false|nullptr)\b)");
    static const std::regex keywords(R"(\b(static_assert|typeid|typename|namespace|export|this|sizeof|constexpr|alignof|decltype|concept|noexcept|requires|explicit)\b)");

    mvprintw(row, col, "%s", line.c_str());

    std::sregex_iterator it(line.begin(), line.end(), control_structures), end;
    for (; it != end; ++it) {
        attron(COLOR_PAIR(2));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(2));
    }

    it = std::sregex_iterator(line.begin(), line.end(), data_types);
    for (; it != end; ++it) {
        attron(COLOR_PAIR(3));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(3));
    }

    it = std::sregex_iterator(line.begin(), line.end(), operators_modifiers);
    for (; it != end; ++it) {
        attron(COLOR_PAIR(4));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(4));
    }

    it = std::sregex_iterator(line.begin(), line.end(), stl_methods);
    for (; it != end; ++it) {
        attron(COLOR_PAIR(5));  
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(5));
    }

    it = std::sregex_iterator(line.begin(), line.end(), preprocessor);
    for (; it != end; ++it) {
        attron(COLOR_PAIR(6));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(6));
    }

    it = std::sregex_iterator(line.begin(), line.end(), literals);
    for (; it != end; ++it) {
        attron(COLOR_PAIR(7));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(7));
    }

    it = std::sregex_iterator(line.begin(), line.end(), keywords);
    for (; it != end; ++it) {
        attron(COLOR_PAIR(8));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(8));
    }
}

    void handleMouseEvent(MEVENT event) {
        int numWidth = 6;
        int y = event.y + offset_y;
        int x = event.x - numWidth;

        if (event.bstate & BUTTON1_PRESSED) {  
            if (y < (int)lines.size() && x >= 0 && x < (int)lines[y].size()) {
                cursor_x = x;
                cursor_y = y;
                if (selecting) {
                    select_x = cursor_x;
                    select_y = cursor_y;
                }
            }
        }

        if (event.bstate & BUTTON4_PRESSED) {  
            if (offset_y > 0) offset_y--;
        }

        if (event.bstate & BUTTON5_PRESSED) {  
            if (offset_y < (int)lines.size() - screen_rows + 2) offset_y++;
        }

        drawEditor();
    }

    void handleInput() {
        int ch;
        MEVENT event;
  while (true) {
        ch = getch();
        
        if (ch == CTRL_KEY('q')) break; // Quit condition

        if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                handleMouseEvent(event);
            }
            continue;
        }

        switch (ch) {
            case KEY_UP:
                if (cursor_y > 0) cursor_y--;
                if (cursor_y < offset_y) offset_y--;
                break;
            case KEY_DOWN:
                if (cursor_y < (int)lines.size() - 1) cursor_y++;
                if (cursor_y >= offset_y + screen_rows - 2) offset_y++;
                break;
            case KEY_LEFT:
                if (cursor_x > 0) cursor_x--;
                break;
            case KEY_RIGHT:
                if (cursor_x < (int)lines[cursor_y].size()) cursor_x++;
                break;
            case CTRL_KEY('b'): // Toggle selection mode
                selecting = !selecting;
                select_x = selecting ? cursor_x : -1;
                select_y = selecting ? cursor_y : -1;
                break;
            case KEY_BACKSPACE:
            case 127:
                if (cursor_x > 0) {
                    lines[cursor_y].erase(cursor_x - 1, 1);
                    cursor_x--;
                }
                break;
            case '\n':
                lines.insert(lines.begin() + cursor_y + 1, lines[cursor_y].substr(cursor_x));
                lines[cursor_y] = lines[cursor_y].substr(0, cursor_x);
                cursor_y++;
                cursor_x = 0;
                break;
            case CTRL_KEY('s'):
                saveFile();
                break;
            default:
                if (ch >= 32 && ch <= 126) {  
                    lines[cursor_y].insert(cursor_x, 1, ch);
                    cursor_x++;
                }
                break;
        }

        drawEditor();
    }
	}



    void run(const std::string& fname) {
        initscr();
        raw();
        keypad(stdscr, TRUE);
        noecho();
        mousemask(ALL_MOUSE_EVENTS, nullptr);

        start_color();
	  init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Default text on dark blue
    init_pair(2, COLOR_GREEN, COLOR_BLUE);   // Control Structures (if, else, switch, etc.)
    init_pair(3, COLOR_YELLOW, COLOR_BLUE);  // Data Types (int, float, double, etc.)
    init_pair(4, COLOR_CYAN, COLOR_BLUE);    // Operators & Modifiers (const, static, virtual, etc.)
    init_pair(5, COLOR_MAGENTA, COLOR_BLUE); // STL Methods (std::vector, std::map, etc.)
    init_pair(6, COLOR_RED, COLOR_BLUE);     // Preprocessor Directives (#include, #define, etc.)
    init_pair(7, COLOR_CYAN, COLOR_BLUE);    // Literals (true, false, nullptr)
    init_pair(8, COLOR_WHITE, COLOR_BLUE);   // Extra Keywords (constexpr, requires, typeid, etc.)
        init_pair(9, COLOR_BLACK, COLOR_WHITE); // Title bar: Black text on white background

		 bkgd(COLOR_PAIR(1));
		 refresh();
        loadFile(fname);
        drawEditor();
        handleInput();
        endwin();
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./editor <filename>\n";
        return 1;
    }

    Editor editor;
    editor.run(argv[1]);
    return 0;
}
