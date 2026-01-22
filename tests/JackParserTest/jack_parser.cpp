//
// Created by Nithin Kondabathini on 16/1/2026.
//

#include <iostream>
#include <memory>
#include <stdexcept>
#include <fstream>
#include "../../Compiler/Parser/Parser.h"
#include "../../Compiler/Parser/AST.h"
#include <cstdlib>
#include <filesystem>
#include <chrono>
using namespace nand2tetris::jack;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: JackParserTest <input file.jack>" << std::endl;
		return 1;
	}

	try {
		// 1. Initialize Tokenizer with the Jack file
		// Ensure your Tokenizer constructor/setup matches this
		Tokenizer tokenizer(argv[1]);

		// 2. Initialize Parser
		Parser parser(tokenizer);

		std::cout << "Parsing file: " << argv[1] << "..." << std::endl;

		std::chrono::high_resolution_clock::time_point parseStart=std::chrono::high_resolution_clock::now();

		// 3. Start parsing at the Class level (the root of Jack grammar)
		std::unique_ptr<ClassNode> astRoot = parser.parse();

		std::chrono::high_resolution_clock::time_point parseEnd = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double, std::milli> parseDuration = parseEnd - parseStart;

		// 4. If we reached here, the syntax is 100% correct
		std::cout << "\n========================================" << std::endl;
		std::cout << "SUCCESS: File parsed successfully!" << std::endl;
		std::cout << "Time taken: " << parseDuration.count() << " ms" << std::endl;
		std::cout << "Class Name: " << astRoot->getClassName() << std::endl;
		std::cout << "Class Variables found: " << astRoot->get_Number_of_classVars() << std::endl;
		std::cout << "Subroutines found: " << astRoot->get_Number_of_Subroutines() << std::endl;
		std::cout << "========================================" << std::endl;


		// 5. Generate XML Output
		std::string inputFileName = argv[1];
		std::string outputFileName = inputFileName.substr(0, inputFileName.find_last_of('.')) + ".xml";

		std::ofstream xmlFile(outputFileName);
		if (xmlFile.is_open()) {
			std::cout << "Generating XML tree: " << outputFileName << "..." << std::endl;
			std::chrono::high_resolution_clock::time_point xmlStart = std::chrono::high_resolution_clock::now();
			astRoot->printXml(xmlFile, 0); // Recursive call to all nodes
			std::chrono::high_resolution_clock::time_point xmlEnd = std::chrono::high_resolution_clock::now();
			xmlFile.close();
			std::chrono::duration<double, std::milli> xmlDuration = xmlEnd - xmlStart;
			std::cout << "XML Generation Time: " << xmlDuration.count() << " ms" << std::endl;
		} else {
			std::cerr << "Error: Could not create output file " << outputFileName << std::endl;
		}

		// 6. Automatically launch the Professional Visualizer
		// Using __FILE__ ensures we find the script relative to this source file
		fs::path source_path = fs::path(__FILE__).parent_path();

		// Go up two levels: JackParserTest -> tests -> NAND2TETRIS (Root)
		// Then enter the 'tools' folder
		fs::path script_path = source_path.parent_path().parent_path() / "tools" / "jack_viz.py";

		// Convert to absolute strings for the system call
		std::string abs_script = fs::absolute(script_path).string();
		std::string abs_xml = fs::absolute(outputFileName).string();

		// Build and run the command
		// Using '&' runs it in the background on macOS
		std::cout << "Launching Visualizer: " << abs_script << std::endl;
		std::string command = "python3 \"" + abs_script + "\" \"" + abs_xml + "\" &";

		std::system(command.c_str());
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