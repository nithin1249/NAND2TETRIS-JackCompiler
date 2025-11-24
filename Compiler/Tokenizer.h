//
// Created by Nithin Kondabathini on 23/11/2025.
//

#ifndef NAND2TETRIS_TOKENIZER_H
#define NAND2TETRIS_TOKENIZER_H

#include <string>
#include <iostream>
#include <memory>
#include "TokenTypes.h"

namespace nand2tetris::jack {
	class Tokenizer{
		public:
			explicit Tokenizer(const std::string& filePath);
			bool hasMoreTokens() const;
			void advance();
			const Token& current() const;
			static bool isKeywordString(const std::string& s, Keyword& outKw);

		private:
			std::string src;
			std::size_t pos=0;
			std::unique_ptr<Token> currentToken;

			void loadFile(const std::string& filePath);
			void skipWhitespaceAndComments();
			std::unique_ptr<Token> nextToken();

			std::unique_ptr<Token> readIdentifierOrKeyword();
			std::unique_ptr<Token> readNumber();
			std::unique_ptr<Token> readString();

	};
}

#endif