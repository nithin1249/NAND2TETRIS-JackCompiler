//
// Created by Nithin Kondabathini on 24/1/2026.
//

#ifndef NAND2TETRIS_SEMANTIC_ANALYSER_H
#define NAND2TETRIS_SEMANTIC_ANALYSER_H


#include "GlobalRegistry.h"
#include "SymbolTable.h"
#include "../Parser/AST.h"

namespace nand2tetris::jack{

	class SemanticAnalyser {
		public:
			explicit SemanticAnalyser(const GlobalRegistry& registry);
			void analyseClass(const ClassNode& class_node);
		private:
			const GlobalRegistry& registry;

			// State
			std::string_view currentClassName;
			std::string_view currentSubroutineName;
			std::string_view currentSubroutineKind; // "function", "method", "constructor"

			[[noreturn]] void error(std::string_view message, const Node& node) const;

			void checkTypeMatch(std::string_view expected, std::string_view actual,const Node& locationNode) const;


			void analyseSubroutine(const SubroutineDecNode& sub, const SymbolTable& masterTable);

			void analyseStatements(const std::vector<std::unique_ptr<StatementNode>>& stmts, SymbolTable& table) const;
			void analyseLet(const LetStatementNode& node, SymbolTable& table)const;
			void analyseIf(const IfStatementNode& node, SymbolTable& table)const;
			void analyseWhile(const WhileStatementNode& node, SymbolTable& table)const;
			void analyseDo(const DoStatementNode& node, SymbolTable& table)const;
			void analyseReturn(const ReturnStatementNode& node, SymbolTable& table) const;

			std::string_view analyseExpression(const ExpressionNode& node, SymbolTable& table)const;

			std::string_view analyseSubroutineCall(std::string_view classNameOrVar,
											   std::string_view functionName,
											   const std::vector<std::unique_ptr<ExpressionNode>>& args,
											   SymbolTable& table,
											   const Node& locationNode)const;
	};
}


#endif //NAND2TETRIS_SEMANTIC_ANALYSER_H