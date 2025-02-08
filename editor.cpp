#include "editor.hpp"

Editor::Editor() {}

void Editor::loadFile(const std::string &fname)
{
    filename = fname;
    std::ifstream file(fname);
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    if (lines.empty())
        lines.push_back("");
}

void Editor::saveFile()
{
    std::ofstream file(filename);
    for (const auto &line : lines)
    {
        file << line << "\n";
    }
    modified = false; // Reset modified flag

    // Display "File Saved" in the title bar for 2 seconds
    attron(A_REVERSE | COLOR_PAIR(9));
    move(0, 0);
    for (int i = 0; i < screen_cols; ++i)
        printw(" "); // Fill title bar
    mvprintw(0, (screen_cols - 20) / 2, " File Saved Successfully! ");
    attroff(A_REVERSE | COLOR_PAIR(9));
    refresh();

    napms(2000); // Wait for 2 seconds

    // Restore normal title bar with filename and saved status
    attron(A_REVERSE | COLOR_PAIR(9));
    move(0, 0);
    for (int i = 0; i < screen_cols; ++i)
        printw(" "); // Refill title bar
    mvprintw(0, (screen_cols - filename.length() - 9) / 2, " Editing: %s [Saved] ", filename.c_str());
    attroff(A_REVERSE | COLOR_PAIR(9));
    refresh();

    drawEditor(); // Redraw screen
}

void Editor::drawEditor()
{
    werase(stdscr);
    getmaxyx(stdscr, screen_rows, screen_cols);
    int numWidth = 6; // Width of line numbers

    // Draw Menu Bar First
    drawMenuBar();

    // Draw Title Bar Below Menu Bar
    attron(A_REVERSE | COLOR_PAIR(9));
    move(1, 0);
    for (int i = 0; i < screen_cols; ++i)
        printw(" "); // Fill the title bar
    mvprintw(1, (screen_cols - filename.length() - (modified ? 11 : 9)) / 2, " Editing: %s %s ",
             filename.c_str(), modified ? "[Modified]" : "[Saved]");
    attroff(A_REVERSE | COLOR_PAIR(9));

    // Draw Text Editor Content (starting from row 2, below menu & title)
    for (int i = 2; i < screen_rows - 1; ++i) // Reserve 2 rows (menu + title)
    {
        int line_index = i + offset_y - 2;
        if (line_index >= (int)lines.size() || line_index < 0)
            break;

        move(i, 0);
        clrtoeol();

        // **Make the line number background match the title bar**
        attron(COLOR_PAIR(4));
        printw("%*d | ", numWidth - 2, line_index + 1);
        attroff(COLOR_PAIR(4));

        highlightSyntax(i, numWidth, lines[line_index]);
    }

    drawStatusBar();                                    // Call status bar at the last row
    move(cursor_y - offset_y + 2, cursor_x + numWidth); // Adjusted for menu & title bar
    refresh();
}
void Editor::drawMenuBar()
{
    move(0, 0); // Move to the first row (topmost)
    clrtoeol();
    attron(A_BOLD | A_REVERSE | COLOR_PAIR(9));
    printw(" Queso Pluma PLX viewer -> Written by: El Alberto =D "); // Simple menu bar
    attron(A_BOLD | A_REVERSE | COLOR_PAIR(9));
}

void Editor::drawStatusBar()
{
    move(screen_rows - 1, 0);
    clrtoeol();
    attron(A_BOLD | A_REVERSE | COLOR_PAIR(9));
    printw("[File: %s] Cursor: (%d, %d) | Selection: %s | [Shift+B] Start Selection | [CTRL+B] Stop | [CTRL+C] Copy | [CTRL+X] Cut | [CTRL+V] Paste | [DEL] Delete",
           filename.c_str(), cursor_y + 1, cursor_x, selecting ? "ON" : "OFF");
    attron(A_BOLD | A_REVERSE | COLOR_PAIR(9));
}

void Editor::highlightSyntax(int row, int col, const std::string &line)
{
    static const std::regex control_structures(R"(\b(IF|THEN|ELSE|DO|END|SELECT|WHEN|OTHERWISE|LEAVE|ITERATE|CALL|RETURN|GOTO|ON ERROR)\b)");
    static const std::regex data_types(R"(\b(FIXED|FLOAT|DECIMAL|CHARACTER|BIT|LABEL|FILE|ENTRY|POINTER|PICTURE|AUTOMATIC|STATIC|CONTROLLED|BASED|DEFINED)\b)");
    static const std::regex operators_modifiers(R"(\b(CONST|STATIC|INLINE|EXPLICIT|VIRTUAL|VOLATILE|MUTABLE|FINAL|OVERRIDE|PRIVATE|PUBLIC|PROTECTED|FRIEND|TYPEDEF|USING|ALIGNAS|ALIGNOF|SIZEOF|THREAD_LOCAL|NEW|DELETE|OPERATOR)\b)");
    
    // Fix for `std::sregex_iterator`, `std::vector`, etc.
    static const std::regex pl1_builtins(R"(\b(DECLARE|PROC|ENTRY|CALL|PUT|GET|ON|IF|DO|END|GO TO|SELECT|WHEN|OTHERWISE|RETURN)\b)");
    
    static const std::regex preprocessor(R"(\b(%INCLUDE|%DECLARE|%IF|%THEN|%ELSE|%DO|%END)\b)");
    static const std::regex literals(R"(\b(TRUE|FALSE|NULL)\b)");
    static const std::regex keywords(R"(\b(STATIC_ASSERT|TYPEID|NAMESPACE|EXPORT|THIS|SIZEOF|ALIGNOF|DECLTYPE|CONCEPT|NOEXCEPT|REQUIRES|EXPLICIT)\b)");
    mvprintw(row, col, "%s", line.c_str());


    std::sregex_iterator it(line.begin(), line.end(), pl1_builtins), end;
    for (; it != end; ++it)
    {
        attron(COLOR_PAIR(4));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(4));
    }
    
    it = std::sregex_iterator(line.begin(), line.end(), control_structures), end;
    for (; it != end; ++it)
    {
        attron(COLOR_PAIR(2));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(2));
    }

    it = std::sregex_iterator(line.begin(), line.end(), data_types);
    for (; it != end; ++it)
    {
        attron(COLOR_PAIR(3));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(3));
    }

    it = std::sregex_iterator(line.begin(), line.end(), operators_modifiers);
    for (; it != end; ++it)
    {
        attron(COLOR_PAIR(4));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(4));
    }

    // it = std::sregex_iterator(line.begin(), line.end(), stl_methods);
    // for (; it != end; ++it)
    // {
    //     attron(COLOR_PAIR(5));
    //     mvprintw(row, col + it->position(), "%s", it->str().c_str());
    //     attroff(COLOR_PAIR(5));
    // }

    it = std::sregex_iterator(line.begin(), line.end(), preprocessor);
    for (; it != end; ++it)
    {
        attron(COLOR_PAIR(6));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(6));
    }

    it = std::sregex_iterator(line.begin(), line.end(), literals);
    for (; it != end; ++it)
    {
        attron(COLOR_PAIR(7));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(7));
    }

    it = std::sregex_iterator(line.begin(), line.end(), keywords);
    for (; it != end; ++it)
    {
        attron(COLOR_PAIR(8));
        mvprintw(row, col + it->position(), "%s", it->str().c_str());
        attroff(COLOR_PAIR(8));
    }
}

void Editor::handleMouseEvent(MEVENT event)
{
    int numWidth = 6;
    int y = event.y + offset_y;
    int x = event.x - numWidth;

    if (event.bstate & BUTTON1_PRESSED)
    {
        if (y < (int)lines.size() && x >= 0 && x < (int)lines[y].size())
        {
            cursor_x = x;
            cursor_y = y;
            if (selecting)
            {
                select_x = cursor_x;
                select_y = cursor_y;
            }
        }
    }

    if (event.bstate & BUTTON4_PRESSED)
    {
        if (offset_y > 0)
            offset_y--;
    }

    if (event.bstate & BUTTON5_PRESSED)
    {
        if (offset_y < (int)lines.size() - screen_rows + 2)
            offset_y++;
    }

    drawEditor();
}

void Editor::handleInput()
{
    int ch;
    MEVENT event;
    while (true)
    {
        ch = getch();

        if (ch == CTRL_KEY('q'))
            break; // Quit condition

        if (ch == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                handleMouseEvent(event);
            }
            continue;
        }

        switch (ch)
        {
        case KEY_HOME: // Move to the beginning of the line
            cursor_x = 0;
            break;

        case KEY_END: // Move to the end of the line
            cursor_x = lines[cursor_y].size();
            break;

        case KEY_PPAGE: // Page Up (Move up by screen height)
            cursor_y -= (screen_rows - 2);
            if (cursor_y < 0)
                cursor_y = 0;
            if (cursor_x > (int)lines[cursor_y].size())
                cursor_x = lines[cursor_y].size(); // Adjust if shorter
            if (cursor_y < offset_y)
                offset_y = cursor_y;
            break;

        case KEY_NPAGE: // Page Down (Move down by screen height)
            cursor_y += (screen_rows - 2);
            if (cursor_y >= (int)lines.size())
                cursor_y = lines.size() - 1;
            if (cursor_x > (int)lines[cursor_y].size())
                cursor_x = lines[cursor_y].size(); // Adjust if shorter
            if (cursor_y >= offset_y + screen_rows - 2)
                offset_y = cursor_y - screen_rows + 3;
            break;
        case KEY_UP: // Move cursor up
            if (cursor_y > 0)
            {
                cursor_y--;
                if (lines[cursor_y].empty()) // 🛠 If line is empty, set cursor_x to 0
                    cursor_x = 0;
                else if (cursor_x > (int)lines[cursor_y].size()) // Move to last character if needed
                    cursor_x = lines[cursor_y].size();
                else if (cursor_x < (int)lines[cursor_y].size()) // Move to last character if needed
                    cursor_x = lines[cursor_y].size();

                if (cursor_y < offset_y)
                    offset_y--;
            }
            break;

        case KEY_DOWN: // Move cursor down
            if (cursor_y < (int)lines.size() - 1)
            {
                cursor_y++;
                if (lines[cursor_y].empty()) // 🛠 If line is empty, set cursor_x to 0
                    cursor_x = 0;
                else if (cursor_x > (int)lines[cursor_y].size()) // Move to last character if needed
                    cursor_x = lines[cursor_y].size();
                else if (cursor_x < (int)lines[cursor_y].size()) // Move to last character if needed
                    cursor_x = lines[cursor_y].size();
                if (cursor_y >= offset_y + screen_rows - 2)
                    offset_y++;
            }
            break;

            // case KEY_UP:
            //     if (cursor_y > 0)
            //         cursor_y--;
            //     if (cursor_y < offset_y)
            //         offset_y--;
            //     break;
            // case KEY_DOWN:
            //     if (cursor_y < (int)lines.size() - 1)
            //     {
            //         cursor_y++;

            //         // Ensure cursor_x is within the bounds of the new line
            //         if (cursor_x > (int)lines[cursor_y].size())
            //         {
            //             cursor_x = lines[cursor_y].size(); // Move to the last character
            //         }
            //     }
            //     // Adjust scrolling if necessary
            //     if (cursor_y >= offset_y + screen_rows - 2)
            //         offset_y++;
            //     break;

        case KEY_LEFT:
            if (cursor_x > 0)
                cursor_x--;
            break;
        case KEY_RIGHT:
            if (cursor_x < (int)lines[cursor_y].size())
                cursor_x++;
            break;
        case CTRL_KEY('b'): // Toggle selection mode
            selecting = !selecting;
            select_x = selecting ? cursor_x : -1;
            select_y = selecting ? cursor_y : -1;
            break;
        case '\n':
            lines.insert(lines.begin() + cursor_y + 1, lines[cursor_y].substr(cursor_x));
            lines[cursor_y] = lines[cursor_y].substr(0, cursor_x);
            cursor_y++;
            cursor_x = 0;
            modified = true; // Mark as modified
            break;
        case CTRL_KEY('s'):
            saveFile();
            break;
        case CTRL_KEY('p'):
            autoFormat();
            break;
        case KEY_BACKSPACE:
        case 127:
            if (cursor_x > 0)
            {
                lines[cursor_y].erase(cursor_x - 1, 1);
                cursor_x--;
                modified = true; // Mark as modified
            }
            else if (cursor_y > 0)
            {
                int prevLineLength = lines[cursor_y - 1].size();
                lines[cursor_y - 1] += lines[cursor_y];
                lines.erase(lines.begin() + cursor_y);
                cursor_y--;
                cursor_x = prevLineLength;
                modified = true; // Mark as modified
            }
            break;

        case KEY_DC: // Delete key
            if (cursor_x < (int)lines[cursor_y].size())
            {
                lines[cursor_y].erase(cursor_x, 1);
                modified = true; // Mark as modified
            }
            else if (cursor_y < (int)lines.size() - 1)
            {
                lines[cursor_y] += lines[cursor_y + 1];
                lines.erase(lines.begin() + cursor_y + 1);
                modified = true; // Mark as modified
            }
            break;
        case KEY_IC: // 🛠 Insert key to toggle overwrite mode
            overwrite_mode = !overwrite_mode;
            break;

        default:
            if (ch >= 32 && ch <= 126) // 🛠 Handle printable characters
            {
                // 🛠 Ensure the line expands with spaces if cursor_x is beyond the current line length
                if (cursor_x >= (int)lines[cursor_y].size())
                {
                    lines[cursor_y].resize(cursor_x, ' '); // Fill with spaces up to cursor_x
                }

                if (overwrite_mode && cursor_x < (int)lines[cursor_y].size())
                {
                    // 🛠 Overwrite Mode: Replace the character at cursor_x
                    lines[cursor_y][cursor_x] = ch;
                }
                else
                {
                    // 🛠 Insert Mode: Insert character at cursor_x
                    lines[cursor_y].insert(cursor_x, 1, ch);
                }

                cursor_x++;      // Move cursor forward
                modified = true; // Mark as modified
            }
            break;
        }

        // ✅ Always update screen after handling input
        drawEditor();
    }
}

void Editor::run(const std::string &fname)
{

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    mousemask(ALL_MOUSE_EVENTS, nullptr);

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);   // Default text on dark blue
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Control Structures (if, else, switch, etc.)
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Data Types (int, float, double, etc.)
    init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Operators & Modifiers (const, static, virtual, etc.)
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // STL Methods (std::vector, std::map, etc.)
    init_pair(6, COLOR_RED, COLOR_BLACK);     // Preprocessor Directives (#include, #define, etc.)
    init_pair(7, COLOR_CYAN, COLOR_BLACK);    // Literals (true, false, nullptr)
    init_pair(8, COLOR_WHITE, COLOR_BLACK);   // Extra Keywords (constexpr, requires, typeid, etc.)
    init_pair(9, COLOR_BLUE, COLOR_WHITE);    // Title bar: Black text on white background

    bkgd(COLOR_PAIR(1));
    refresh();
    loadFile(fname);
    drawEditor();
    handleInput();
    endwin();
}

void Editor::autoFormat()
{
    std::vector<std::string> formattedLines;
    int indentLevel = 0;
    const int indentSize = 4; // Spaces per indent level

    for (auto &line : lines)
    {
        std::string trimmed = std::regex_replace(line, std::regex("^\\s+|\\s+$"), ""); // Trim spaces
        if (trimmed.empty())
        {
            formattedLines.push_back(""); // Keep empty lines
            continue;
        }

        if (trimmed[0] == '}')
        {
            indentLevel = std::max(0, indentLevel - 1);
        }

        formattedLines.push_back(std::string(indentLevel * indentSize, ' ') + trimmed);

        if (trimmed.back() == '{')
        {
            indentLevel++;
        }
    }

    lines = formattedLines;
    drawEditor(); // Refresh screen
}
