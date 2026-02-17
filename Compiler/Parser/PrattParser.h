//
// Created by Nithin Kondabathini on 15/2/2026.
//

#ifndef NAND2TETRIS_PRATT_PARSER_H
#define NAND2TETRIS_PRATT_PARSER_H
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>

#include "AST.h"
#include "../Tokenizer/Tokenizer.h"


namespace nand2tetris::jack{
	enum class Precedence {
		LOWEST,
		EQUALS,         // ==
		LESS_GREATER,   // < or >
		SUM,            // + or -
		PRODUCT,        // * or /
		PREFIX,         // -x or ~x
		CALL,           // obj.method()
		INDEX,          // arr[i]
		ACCESS          // Generic <T> or .
	};

	struct ParseError {
		int line;
		int column;
		std::string message;
	};



	class PrattParser {
		public:
			/**
			 * @brief Function pointer types for Pratt parsing functions.
			 */
			using NudFunc= std::unique_ptr<ExpressionNode> (PrattParser::*)();
			using LedFunc= std::unique_ptr<ExpressionNode> (PrattParser::*)(std::unique_ptr<ExpressionNode>);
			/**
			 * @brief A single entry in the Pratt Dispatch Table.
			 */
			struct ParseRule {
				NudFunc nud;      // Null Denotation (Prefix)
				LedFunc led;      // Left Denotation (Infix/Postfix)
				Precedence precedence;  // Precedence level
			};

		private:
			Tokenizer& tokenizer;
			const Token* currentToken=nullptr;

			// --- The Hybrid Dispatch Table ---
			// Maps generic types (Identifiers, Ints, Strings)
			std::unordered_map<TokenType, ParseRule> typeRules;

			// Maps specific text ("+", ".", "while", "-")
			std::unordered_map<std::string_view, ParseRule> textRules;

			std::vector<ParseError> errors;

			// --- The Core Pratt Engine ---
			void initializeRules();
			const ParseRule& getRule(const Token* token) const;
			std::unique_ptr<ExpressionNode> parseExpression(Precedence precedence = Precedence::LOWEST);


			// --- Token Navigation & Error Helpers ---
			void advance();
			bool check(TokenType type, std::string_view value = "") const;
			bool match(TokenType type, std::string_view value = "") ;
			void expect(TokenType type, std::string_view value = "");
			void reportError(std::string_view message);
			void synchronize();

			// --- Type & High-Level Declaration Helpers ---
			std::unique_ptr<Type> parseType(bool allowVoid=false); // Handles Generics like Array<T>
			std::vector<Parameter> parseParameterList();

			std::unique_ptr<ClassNode> parseClass();
			std::unique_ptr<ClassVarDecNode> parseClassVarDec();
			std::vector<std::unique_ptr<VarDecNode>> parseLocalVars();
			std::unique_ptr<SubroutineDecNode> parseSubroutineDec();



			// --- Statements ---
			std::vector<std::unique_ptr<StatementNode>> parseStatements();
			std::unique_ptr<StatementNode> parseLetStatement();
			std::unique_ptr<StatementNode> parseIfStatement();
			std::unique_ptr<StatementNode> parseWhileStatement();
			std::unique_ptr<StatementNode> parseDoStatement();
			std::unique_ptr<StatementNode> parseReturnStatement();

			// --- Nud (Prefix) Handlers for Expressions ---
			std::unique_ptr<ExpressionNode> parseIntegerNud();
			std::unique_ptr<ExpressionNode> parseFloatNud();
			std::unique_ptr<ExpressionNode> parseStringNud();
			std::unique_ptr<ExpressionNode> parseKeywordNud();
			std::unique_ptr<ExpressionNode> parseIdentifierNud();
			std::unique_ptr<ExpressionNode> parseUnaryNud();
			std::unique_ptr<ExpressionNode> parseGroupNud(); // For '('

			// --- Led (Infix) Handlers for Expressions ---
			std::unique_ptr<ExpressionNode> parseBinaryLed(std::unique_ptr<ExpressionNode> left);
			std::unique_ptr<ExpressionNode> parseCallLed(std::unique_ptr<ExpressionNode> left);
			std::unique_ptr<ExpressionNode> parseIndexLed(std::unique_ptr<ExpressionNode> left);
			std::vector<std::unique_ptr<ExpressionNode>> parseExpressionList();

		public:
			explicit PrattParser(Tokenizer& tokenizer);
			std::unique_ptr<ClassNode> parse();
			bool hasErrors() const { return !errors.empty(); }
			const std::vector<ParseError>& getErrors() const { return errors; }

	};
};
#endif //NAND2TETRIS_PRATT_PARSER_H