#include "editor.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./editor <filename>\n";
        return 1;
    }

    Editor editor;
    editor.run(argv[1]);
    return 0;
}
