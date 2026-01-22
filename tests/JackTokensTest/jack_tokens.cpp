#include <iostream>
#include "../../Compiler/Tokenizer/Tokenizer.h"
#include <chrono>


using namespace nand2tetris::jack;

int main(int const argc, char** argv) {
	// 1. Basic argument validation
	if (argc < 2) {
		std::cerr << "Usage: jack_tokens <file.jack>\n";
		return 1;
	}

	try {
		const std::string filePath = std::string(argv[1]);

		const std::chrono::high_resolution_clock::time_point Start=std::chrono::high_resolution_clock::now();

		// 2. Initialize the Tokenizer
		// The constructor "primes the pump" by finding the first token
		Tokenizer tokenizer(filePath);

		// 3. Simple, clean loop using Polymorphism
		while (tokenizer.hasMoreTokens()) {
			// Because Token::toString() is virtual, C++ automatically calls
			// the correct version (IntToken, TextToken, etc.) at runtime
			std::cout << tokenizer.current().toString() << "\n";

			// Move to the next meaningful token
			tokenizer.advance();
		}

		const std::chrono::high_resolution_clock::time_point End = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double, std::milli> Duration = End - Start;
		std::cout << "Time taken: " << Duration.count() << " ms" << std::endl;
	}
	catch (const std::exception& e) {
		// This catches your custom errorAt and errorHere messages
		std::cerr << "Compilation Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}

