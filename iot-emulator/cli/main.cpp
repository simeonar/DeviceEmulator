// Entry point for CLI application
#include <iostream>
#include <string>
int main(int argc, char* argv[]) {
    std::cout << "IoT Emulator CLI started." << std::endl;
    if (argc > 1) {
        std::string cmd = argv[1];
        if (cmd == "test") {
            std::cout << "Test command executed successfully!" << std::endl;
            return 0;
        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
            return 1;
        }
    } else {
        // Interactive REPL mode
        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);
            if (input == "exit" || input == "quit") {
                std::cout << "Exiting CLI." << std::endl;
                break;
            } else if (input == "test") {
                std::cout << "Test command executed successfully!" << std::endl;
            } else if (input.empty()) {
                continue;
            } else {
                std::cout << "Unknown command: " << input << std::endl;
            }
        }
    }
    return 0;
}
