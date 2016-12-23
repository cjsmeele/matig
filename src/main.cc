/**
 * \file
 * \brief     REPL.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#include "common.hh"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

#include "read.hh"
#include "eval.hh"
#include "print.hh"

#include <unistd.h>

/**
 * \brief Remove an optional shebang (#! line) from the beginning of a
 *        stream.
 *
 * \param stream
 */
static void slurpShebang(std::istream &stream) {
    char c, c2;
    if (!stream.get(c)) throw ProgramError("Unexpected EOF");
    if (c == '#') {
        if (!stream.get(c2)) throw ProgramError("Unexpected EOF");
        if (c2 == '!') {
            do {
                stream.get(c);
            } while (c != '\n' && stream);
        } else {
            if (!stream.putback(c2)) throw std::runtime_error("Could not istream::putback char 2");
            if (!stream.putback(c)) throw std::runtime_error("Could not istream::putback char 1");
        }
    } else {
        if (!stream.putback(c)) throw std::runtime_error("Could not istream::putback char 1");
    }
}

int main(int argc, char **argv) {

    std::istream  *in = &std::cin;
    std::ifstream file;

    std::string prompt = "\x1b[1;36m" "Matig" "\x1b[0m" "> ";

    bool isRepl = false;

    {
        auto printUsage = [argv]{
            std::cerr << "usage: " << argv[0]
                      << " [-r] [--] [file|-]\n";
        };

        // Parse arguments.
        bool dashed = false;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];

            if (!dashed && arg == "--") {
                dashed = true;

            } else if (!dashed && arg == "-r") {
                isRepl = true;

            } else if (dashed || (arg.length() && arg[0] != '-')) {
                if (file.is_open()) {
                    printUsage();
                    return 1;
                }

                file.open(arg);
                if (!file)
                    throw std::runtime_error("Could not open file '"s
                                            + arg + "' for reading.");
            } else if (!dashed && arg == "-"){
                if (file.is_open()) {
                    printUsage();
                    return 1;
                }
            } else {
                printUsage();
                return 1;
            }
        }
    }


    if (file.is_open())
        in = &file;

    bool isInteractive = in == &std::cin && isatty(fileno(stdin));
    isRepl |= isInteractive;

    if (!isInteractive)
        slurpShebang(*in);

    EnvPtr rootEnv = std::make_shared<Env>();

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
            Eptr result = eval(expr, rootEnv);

            if (isRepl)
                print(result);

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
