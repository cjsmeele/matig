#include "common.hh"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

#include "read.hh"
#include "eval.hh"
#include "print.hh"

#include <unistd.h>

int main(int argc, char **argv) {

    std::istream  *in = &std::cin;
    std::ifstream file;

    std::string prompt = "\x1b[1;36m" "Matig" "\x1b[0m" "> ";

    if (argc > 1 && std::strcmp(argv[1], "-")) {
        file.open(argv[1]);
        if (!file)
            throw std::runtime_error("Could not open file '"s
                                     + argv[1] + "' for reading.");
    }

    if (file.is_open())
        in = &file;

    bool isInteractive = in == &std::cin && isatty(fileno(stdin));

    while (true) {
        if (isInteractive) {
            std::cout << prompt;
            std::cout.flush();
        }
        try {
            Eptr expr = read(*in);

            // Stop once no more expressions can be read (EOF / IO error).
            if (!expr)
                break;

            // Eptr result = expr;
            Eptr result = eval(*expr);

            if (isInteractive)
                print(*result);

        } catch (ProgramError &e) {
            std::cerr << "Program error: " << e.what() << "\n";
        } catch (LogicError &e) {
            std::cerr << "BUG (LogicError): " << e.what() << "\n";
        } catch (std::exception &e) {
            std::cerr << "BUG (other): " << e.what() << "\n";
        }
    }

    return 0;
}
