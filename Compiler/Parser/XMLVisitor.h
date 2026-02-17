//
// Created by Nithin Kondabathini on 18/2/2026.
//

#ifndef NAND2TETRIS_XMLVISITOR_H
#define NAND2TETRIS_XMLVISITOR_H

#include "ASTVisitor.h"
#include "AST.h"
#include <iostream>
#include <string>

namespace nand2tetris::jack {

	class XMLVisitor : public ASTVisitor {
		std::ostream& out;
		int indentLevel = 0;

		void printIndent() const {
			for (int i = 0; i < indentLevel; ++i) {
				out << "  ";
			}
		}

		void openTag(const std::string& tag) {
			printIndent();
			out << "<" << tag << ">\n";
			indentLevel++;
		}

		void closeTag(const std::string& tag) {
			indentLevel--;
			printIndent();
			out << "</" << tag << ">\n";
		}

		static std::string escapeXML(const std::string_view text){
			std::string result;
			for (char c : text) {
				switch (c) {
					case '<': result += "&lt;"; break;
					case '>': result += "&gt;"; break;
					case '&': result += "&amp;"; break;
					case '"': result += "&quot;"; break;
					default:  result += c; break;
				}
			}
			return result;
		}

		void printInline(const std::string& tag, const std::string_view value) const {
			printIndent();
			out << "<" << tag << "> " << escapeXML(value) << " </" << tag << ">\n";
		}

		std::string formatType(const Type& type) {
			std::string s = std::string(type.baseType);
			if (!type.genericArgs.empty()) {
				s += "<";
				for (size_t i = 0; i < type.genericArgs.size(); ++i) {
					s += formatType(*type.genericArgs[i]);
					if (i < type.genericArgs.size() - 1) s += ", ";
				}
				s += ">";
			}
			return s;
		}

		public:
			explicit XMLVisitor(std::ostream& outputStream) : out(outputStream) {}

			void print(Node* node) {
				if (node) {
					node->accept(*this);
				}
			}

			void visit(ClassNode& node) override {
				openTag("classNode");
				printInline("className", node.getClassName());

				for (const auto& var : node.getClassVars()) print(var.get());
				for (const auto& sub : node.getSubroutines()) print(sub.get());

				closeTag("classNode");
			}

			void visit(ClassVarDecNode& node) override {
				openTag("classVarDec");
				const std::string kindStr = (node.getKind() == ClassVarKind::STATIC) ? "static" : "field";
				printInline("kind", kindStr);
				if (node.getType()) {
					printInline("type", formatType(*node.getType())); // Now shows Array<School>
				}
				for (const auto& name : node.getVarNames()) {
					printInline("name", name);
				}
				closeTag("classVarDec");
			}

			void visit(VarDecNode& node) override {
				openTag("varDec");
				if (node.getType()) {
					printInline("type", formatType(*node.getType())); // Now shows Array<School>
				}
				for (const auto& name : node.getVarNames()) {
					printInline("name", name);
				}
				closeTag("varDec");
			}

			void visit(SubroutineDecNode& node) override {
				openTag("subroutineDec");

				std::string subType;
				if (node.getSubType() == SubroutineType::CONSTRUCTOR) subType = "constructor";
				else if (node.getSubType() == SubroutineType::FUNCTION) subType = "function";
				else subType = "method";

				printInline("subroutineType", subType);

				if (node.getReturnType()) {
					printInline("returnType", formatType(*node.getReturnType()));
				}

				printInline("name", node.getName());

				openTag("parameterList");
				for (const auto&[type, name] : node.getParams()) {
					openTag("parameter");
					if (type) printInline("type", formatType(*type));
					printInline("name", name);
					closeTag("parameter");
				}
				closeTag("parameterList");

				openTag("subroutineBody");
				for (const auto& local : node.getLocals()) print(local.get());

				openTag("statements");
				for (const auto& stmt : node.getStatements()) print(stmt.get());
				closeTag("statements");

				closeTag("subroutineBody");
				closeTag("subroutineDec");
			}

			void visit(LetStatementNode& node) override {
				openTag("letStatement");
				printInline("varName", node.getVarName());
				if (node.getIndex()) {
					openTag("index");
					print(node.getIndex());
					closeTag("index");
				}
				openTag("value");
				print(node.getValue());
				closeTag("value");
				closeTag("letStatement");
			}

			void visit(IfStatementNode& node) override {
				openTag("ifStatement");
				openTag("condition");
				print(node.getCondition());
				closeTag("condition");

				openTag("ifBranch");
				for (const auto& stmt : node.getIfBranch()) print(stmt.get());
				closeTag("ifBranch");

				if (!node.getElseBranch().empty()) {
					openTag("elseBranch");
					for (const auto& stmt : node.getElseBranch()) print(stmt.get());
					closeTag("elseBranch");
				}
				closeTag("ifStatement");
			}

			void visit(WhileStatementNode& node) override {
				openTag("whileStatement");
				openTag("condition");
				print(node.getCondition());
				closeTag("condition");

				openTag("body");
				for (const auto& stmt : node.getBody()) print(stmt.get());
				closeTag("body");
				closeTag("whileStatement");
			}

			void visit(DoStatementNode& node) override {
				openTag("doStatement");
				print(node.getCall());
				closeTag("doStatement");
			}

			void visit(ReturnStatementNode& node) override {
				openTag("returnStatement");
				if (node.getExpression()) {
					print(node.getExpression());
				}
				closeTag("returnStatement");
			}

			void visit(CallNode& node) override {
				openTag("callNode");
				if (node.getReceiver()) {
					openTag("receiver");
					print(node.getReceiver());
					closeTag("receiver");
				}
				printInline("methodName", node.getFunctionName());

				openTag("expressionList");
				for (const auto& arg : node.getArgs()) {
					print(arg.get());
				}
				closeTag("expressionList");
				closeTag("callNode");
			}

			void visit(IdentifierNode& node) override {
				openTag("identifierNode");
				printInline("name", node.getName());
				if (!node.getGenericArgs().empty()) {
					openTag("generics");
					for (const auto& type : node.getGenericArgs()) {
						printInline("typeArg",formatType(*type)); // Use formatType here too!
					}
					closeTag("generics");
				}
				closeTag("identifierNode");
			}

			void visit(BinaryOpNode& node) override {
				openTag("binaryOpNode");
				openTag("left");
				print(node.getLeft());
				closeTag("left");

				printInline("op", std::string(1, node.getOp()));

				openTag("right");
				print(node.getRight());
				closeTag("right");
				closeTag("binaryOpNode");
			}

			void visit(UnaryOpNode& node) override {
				openTag("unaryOpNode");
				printInline("op", std::string(1, node.getOp()));
				print(node.getTerm());
				closeTag("unaryOpNode");
			}

			void visit(ArrayAccessNode& node) override {
				openTag("arrayAccessNode");
				openTag("base");
				print(node.getBase());
				closeTag("base");

				openTag("index");
				print(node.getIndex());
				closeTag("index");
				closeTag("arrayAccessNode");
			}
			void visit(IntegerLiteralNode& node) override {
				printInline("integerConstant", std::to_string(node.getInt()));
			}

			void visit(FloatLiteralNode& node) override {
				printInline("floatConstant", std::to_string(node.getFloat()));
			}

			void visit(StringLiteralNode& node) override {
				printInline("stringConstant", node.getString());
			}

			void visit(KeywordLiteralNode& node) override {
				// Assuming you have a way to convert Keyword enum to string
				printInline("keywordConstant", keywordToString(node.getKeyword()));
			}
	};

}

#endif //NAND2TETRIS_XMLVISITOR_H