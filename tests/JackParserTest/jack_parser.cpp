//
// Created by Nithin Kondabathini on 16/1/2026.
//

#include <iostream>
#include <memory>
#include <stdexcept>
#include "../../Compiler/Parser/Parser.h"
#include "../../Compiler/Parser/AST.h"

using namespace nand2tetris::jack;

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: JackParserTest <inputfile.jack>" << std::endl;
		return 1;
	}

	try {
		// 1. Initialize Tokenizer with the Jack file
		// Ensure your Tokenizer constructor/setup matches this
		Tokenizer tokenizer(argv[1]);

		// 2. Initialize Parser
		Parser parser(tokenizer);

		std::cout << "Parsing file: " << argv[1] << "..." << std::endl;

		// 3. Start parsing at the Class level (the root of Jack grammar)
		std::unique_ptr<ClassNode> astRoot = parser.parse();

		// 4. If we reached here, the syntax is 100% correct
		std::cout << "\n========================================" << std::endl;
		std::cout << "SUCCESS: File parsed successfully!" << std::endl;
		std::cout << "Class Name: " << astRoot->className << std::endl;
		std::cout << "Subroutines found: " << astRoot->subroutineDecs.size() << std::endl;
		std::cout << "========================================" << std::endl;

	} catch (const std::runtime_error& e) {
		// This catches the fatal errors thrown by your Tokenizer::errorAt
		std::cerr << "\nSYNTAX ERROR DETECTED:" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "\nAN UNEXPECTED ERROR OCCURRED:" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}