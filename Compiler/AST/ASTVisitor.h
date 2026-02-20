//
// Created by Nithin Kondabathini on 14/2/2026.
//

#ifndef NAND2TETRIS_AST_VISITOR_H
#define NAND2TETRIS_AST_VISITOR_H

namespace nand2tetris::jack {
	// Forward declarations of all nodes to avoid including AST.h here
	class ClassNode;
	class ClassVarDecNode;
	class SubroutineDecNode;
	class VarDecNode;
	class LetStatementNode;
	class IfStatementNode;
	class WhileStatementNode;
	class DoStatementNode;
	class ReturnStatementNode;
	class IntegerLiteralNode;
	class FloatLiteralNode;
	class StringLiteralNode;
	class KeywordLiteralNode;
	class BinaryOpNode;
	class UnaryOpNode;
	class CallNode;
	class IdentifierNode;
	class ArrayAccessNode;


	/**
	 * @brief Interface for modular AST processing.
	 * Implement this to create new backends like XML printers or LLVMCodeGen generators.
	 */

	class ASTVisitor {
	public:
		virtual ~ASTVisitor() = default;

		// Structure
		virtual void visit(const ClassNode& node) = 0;
		virtual void visit(const ClassVarDecNode& node) = 0;
		virtual void visit(const SubroutineDecNode& node) = 0;
		virtual void visit(const VarDecNode& node) = 0;

		// Statements
		virtual void visit(const LetStatementNode& node) = 0;
		virtual void visit(const IfStatementNode& node) = 0;
		virtual void visit(const WhileStatementNode& node) = 0;
		virtual void visit(const DoStatementNode& node) = 0;
		virtual void visit(const ReturnStatementNode& node) = 0;

		// Expressions
		virtual void visit(const IntegerLiteralNode& node) = 0;
		virtual void visit(const FloatLiteralNode& node) = 0;
		virtual void visit(const StringLiteralNode& node) = 0;
		virtual void visit(const KeywordLiteralNode& node) = 0;
		virtual void visit(const BinaryOpNode& node) = 0;
		virtual void visit(const UnaryOpNode& node) = 0;
		virtual void visit(const CallNode& node) = 0;
		virtual void visit(const IdentifierNode& node) = 0;
		virtual void visit(const ArrayAccessNode& node) = 0;
	};


}

#endif //NAND2TETRIS_AST_VISITOR_H