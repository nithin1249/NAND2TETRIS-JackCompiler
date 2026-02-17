//
// Created by Nithin Kondabathini on 14/2/2026.
//

#ifndef NAND2TETRIS_ASTVISITOR_H
#define NAND2TETRIS_ASTVISITOR_H

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
	 * Implement this to create new backends like XML printers or LLVM generators.
	 */

	class ASTVisitor {
	public:
		virtual ~ASTVisitor() = default;

		// Structure
		virtual void visit(ClassNode& node) = 0;
		virtual void visit(ClassVarDecNode& node) = 0;
		virtual void visit(SubroutineDecNode& node) = 0;
		virtual void visit(VarDecNode& node) = 0;

		// Statements
		virtual void visit(LetStatementNode& node) = 0;
		virtual void visit(IfStatementNode& node) = 0;
		virtual void visit(WhileStatementNode& node) = 0;
		virtual void visit(DoStatementNode& node) = 0;
		virtual void visit(ReturnStatementNode& node) = 0;

		// Expressions
		virtual void visit(IntegerLiteralNode& node) = 0;
		virtual void visit(FloatLiteralNode& node) = 0;
		virtual void visit(StringLiteralNode& node) = 0;
		virtual void visit(KeywordLiteralNode& node) = 0;
		virtual void visit(BinaryOpNode& node) = 0;
		virtual void visit(UnaryOpNode& node) = 0;
		virtual void visit(CallNode& node) = 0;
		virtual void visit(IdentifierNode& node) = 0;
		virtual void visit(ArrayAccessNode& node) = 0;
	};


}

#endif //NAND2TETRIS_ASTVISITOR_H