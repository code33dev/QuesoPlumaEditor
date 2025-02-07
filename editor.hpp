#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <regex>

#define CTRL_KEY(k) ((k) & 0x1f)

class Editor
{
private:
    std::vector<std::string> lines;
    std::stack<std::vector<std::string>> undo_stack, redo_stack;
    std::string clipboard;
    int cursor_x = 0, cursor_y = 0;
    int select_x = -1, select_y = -1;
    int offset_y = 0;
    int screen_rows, screen_cols;
    bool selecting = false;
    std::string filename;
    bool modified = false;

    void autoFormat();

    void drawStatusBar();
    void drawMenuBar();
    void drawEditor();
    void highlightSyntax(int row, int col, const std::string &line);
    void handleMouseEvent(MEVENT event);
    void handleInput();
    void loadFile(const std::string &fname);
    void saveFile();

public:
    Editor();
    void run(const std::string &fname);
};

#endif // EDITOR_HPP
