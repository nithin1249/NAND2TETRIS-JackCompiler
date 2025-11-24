//
// Created by Nithin Kondabathini on 23/11/2025.
//

#ifndef NAND2TETRIS_TOKEN_H
#define NAND2TETRIS_TOKEN_H

#include<string>
#include <utility>

namespace nand2tetris::jack {
	enum class TokenType {
		KEYWORD,
		SYMBOL,
		IDENTIFIER,
		INT_CONST,
		STRING_CONST,
		END_OF_FILE
	};

	enum class Keyword {
		CLASS,METHOD,FUNCTION,CONSTRUCTOR,
		INT, BOOLEAN, CHAR, VOID,
		VAR, STATIC, FIELD, LET, DO, IF,
		ELSE, WHILE, RETURN, TRUE_, FALSE_, NULL_, THIS_

	};


	struct Token {
		private:
			TokenType type=TokenType::END_OF_FILE;

		public:
			explicit Token(const TokenType t):type(t){}
			TokenType getType() const {return type;}
			virtual ~Token() = default;
	};

	struct TextToken final : public Token {
		private:
			std::string text;

		public:
			TextToken(const TokenType t, std::string  text):Token(t),text(std::move(text)){};
			std::string getText() const {return text;}
	};

	struct IntToken final : public Token {
		private:
			int intVal=0;
		public:
			explicit IntToken(const int val):Token(TokenType::INT_CONST), intVal(val){}
			int getInt() const {return intVal;}
	};

	struct KeywordToken final : public Token {
		private:
			Keyword keyword{};
		public:
			explicit KeywordToken(const Keyword keyword):Token(TokenType::KEYWORD), keyword(keyword){}
			Keyword getKeyword() const {return keyword;}
	};

	struct EofToken final : public Token {
		EofToken() : Token(TokenType::END_OF_FILE) {}
	};

	inline const char* keywordToString(Keyword kw) {
		using K = Keyword;
		switch (kw) {
			case K::CLASS:       return "class";
			case K::METHOD:      return "method";
			case K::FUNCTION:    return "function";
			case K::CONSTRUCTOR: return "constructor";
			case K::INT:         return "int";
			case K::CHAR:        return "char";
			case K::BOOLEAN:     return "boolean";
			case K::VOID:        return "void";
			case K::VAR:         return "var";
			case K::STATIC:      return "static";
			case K::FIELD:       return "field";
			case K::LET:         return "let";
			case K::DO:          return "do";
			case K::IF:          return "if";
			case K::ELSE:        return "else";
			case K::WHILE:       return "while";
			case K::RETURN:      return "return";
			case K::TRUE_:       return "true";
			case K::FALSE_:      return "false";
			case K::NULL_:       return "null";
			case K::THIS_:       return "this";
		}
		return "<unknown>";
	}


}

#endif