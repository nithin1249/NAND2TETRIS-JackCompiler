//
// Created by Nithin Kondabathini on 18/2/2026.
//

#ifndef NAND2TETRIS_XML_VISITOR_H
#define NAND2TETRIS_XML_VISITOR_H

#include "../AST/ASTVisitor.h"
#include "../AST/AST.h"
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
			for (const char c : text) {
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

		public:
			explicit XMLVisitor(std::ostream& outputStream) : out(outputStream) {}

			void print(const Node* node) {
				if (node) {
					node->accept(*this);
				}
			}

			void visit(const ClassNode& node) override {
				openTag("classNode");
				printInline("className", node.getClassName());


				// Only print if there's actually something there

				for (const auto& var : node.getClassVars()) {
					print(var); // Handles each ClassVarDecNode
				}

				for (const auto& sub:node.getSubroutines()) {
					print(sub);
				}


				closeTag("classNode");
			}

			void visit(const ClassVarDecNode& node) override {
				openTag("classVarDec");
				const std::string kindStr = (node.getKind() == ClassVarKind::STATIC) ? "static" : "field";
				printInline("kind", kindStr);
				printInline("type", node.getType().formatType()); // Now shows Array<School>
				for (const auto& name : node.getVarNames()) {
					printInline("name", name);
				}
				closeTag("classVarDec");
			}

			void visit(const VarDecNode& node) override {
				openTag("varDec");
				printInline("type", node.getType().formatType()); // Now shows Array<School>
				for (const auto& name : node.getVarNames()) {
					printInline("name", name);
				}
				closeTag("varDec");
			}

			void visit(const SubroutineDecNode& node) override {
				openTag("subroutineDec");

				std::string subType;
				if (node.getSubType() == SubroutineType::CONSTRUCTOR) subType = "constructor";
				else if (node.getSubType() == SubroutineType::FUNCTION) subType = "function";
				else subType = "method";

				printInline("subroutineType", subType);

				printInline("returnType",node.getReturnType().formatType());

				printInline("name", node.getName());

				if (!node.getParams().empty()) {
					openTag("parameterList");
					for (const auto& param: node.getParams()) {
						openTag("parameter");
						printInline("type",param->getType().formatType());
						printInline("name", param->getName());
						closeTag("parameter");
					}
					closeTag("parameterList");
				}

				if (!node.getLocals().empty() || !node.getStatements().empty()) {
					openTag("subroutineBody");

					for (const auto& var : node.getLocals()) {
						print(var);
					}

					if (!node.getStatements().empty()) {
						openTag("statements");
						for (const auto& stmt : node.getStatements()) print(stmt);
						closeTag("statements");
					}

					closeTag("subroutineBody");
				}
				closeTag("subroutineDec");
			}

			void visit(const LetStatementNode& node) override {
				openTag("letStatement");
				printInline("varName", node.getVarName());
				if (node.getIndex()) {
					openTag("index");
					print(node.getIndex());
					closeTag("index");
				}
				openTag("value");
				print(&node.getValue());
				closeTag("value");
				closeTag("letStatement");
			}

			void visit(const IfStatementNode& node) override {
				openTag("ifStatement");
				openTag("condition");
				print(&node.getCondition());
				closeTag("condition");

				if (!node.getIfBranch().empty()) {
					openTag("ifBranch");
					for (const auto& stmt : node.getIfBranch()) print(stmt);
					closeTag("ifBranch");
				}

				if (!node.getElseBranch().empty()) {
					openTag("elseBranch");
					for (const auto& stmt : node.getElseBranch()) print(stmt);
					closeTag("elseBranch");
				}
				closeTag("ifStatement");
			}

			void visit(const WhileStatementNode& node) override {
				openTag("whileStatement");
				openTag("condition");
				print(&node.getCondition());
				closeTag("condition");

				if (!node.getBody().empty()) {
					openTag("body");
					for (const auto& stmt : node.getBody()) print(stmt);
					closeTag("body");
				}


				closeTag("whileStatement");
			}

			void visit(const DoStatementNode& node) override {
				openTag("doStatement");
				print(&node.getCall());
				closeTag("doStatement");
			}

			void visit(const ReturnStatementNode& node) override {
				openTag("returnStatement");
				if (node.getExpression()) {
					print(node.getExpression());
				}
				closeTag("returnStatement");
			}

			void visit(const CallNode& node) override {
				openTag("callNode");
				if (node.getReceiver()) {
					openTag("receiver");
					print(node.getReceiver());
					closeTag("receiver");
				}
				printInline("methodName", node.getFunctionName());

				if (!node.getArgs().empty()) {
					openTag("expressionList");
					for (const auto& arg : node.getArgs()) {
						print(arg);
					}
					closeTag("expressionList");
				}

				closeTag("callNode");
			}

			void visit(const IdentifierNode& node) override {
				openTag("identifierNode");
				printInline("name", node.getName());
				if (!node.getGenericArgs().empty()) {
					openTag("generics");
					for (const auto& type : node.getGenericArgs()) {
						printInline("typeArg",type->formatType()); // Use formatType here too!
					}
					closeTag("generics");
				}

				closeTag("identifierNode");
			}

			void visit(const BinaryOpNode& node) override {
				openTag("binaryOpNode");
				openTag("left");
				print(&node.getLeft());
				closeTag("left");

				printInline("op", std::string(1, node.getOp()));

				openTag("right");
				print(&node.getRight());
				closeTag("right");
				closeTag("binaryOpNode");
			}

			void visit(const UnaryOpNode& node) override {
				openTag("unaryOpNode");
				printInline("op", std::string(1, node.getOp()));
				print(&node.getTerm());
				closeTag("unaryOpNode");
			}

			void visit(const ArrayAccessNode& node) override {
				openTag("arrayAccessNode");
				openTag("base");
				print(&node.getBase());
				closeTag("base");

				openTag("index");
				print(&node.getIndex());
				closeTag("index");
				closeTag("arrayAccessNode");
			}
			void visit(const IntegerLiteralNode& node) override {
				printInline("integerConstant", std::to_string(node.getInt()));
			}

			void visit(const FloatLiteralNode& node) override {
				printInline("floatConstant", std::to_string(node.getFloat()));
			}

			void visit(const StringLiteralNode& node) override {
				printInline("stringConstant", node.getString());
			}

			void visit(const KeywordLiteralNode& node) override {
				printInline("keywordConstant", keywordToString(node.getKeyword()));
			}
	};

}

#endif //NAND2TETRIS_XML_VISITOR_H