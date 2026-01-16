//
// Created by Nithin Kondabathini on 13/1/2026.
//

#ifndef NAND2TETRIS_AST_H
#define NAND2TETRIS_AST_H

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "../Tokenizer/TokenTypes.h"

namespace nand2tetris::jack {

    /**
     * @brief Base class for all nodes in the Abstract Syntax Tree (AST).
     *
     * All specific AST nodes inherit from this class. It provides a virtual destructor
     * to ensure proper cleanup of derived classes.
     */
    class Node {
        public:
            virtual ~Node() = default;
            virtual void print(std::ostream& out, int indent) const=0;
    };

    /**
     * @brief Enumeration for the kind of class variable.
     */
    enum class ClassVarKind {
        STATIC, ///< A static variable, shared by all instances of the class.
        FIELD   ///< A field variable, unique to each instance of the class.
    };

    /**
     * @brief Represents a class variable declaration (static or field).
     *
     * Example: `static int x, y;` or `field boolean isActive;`
     */
    class ClassVarDecNode final : public Node {
        public:
            ClassVarKind kind; ///< The kind of variable (static or field).
            std::string_view type; ///< The data type of the variable(s) (e.g., "int", "boolean", "MyClass").
            std::vector<std::string_view> varNames; ///< A list of variable names declared in this statement.

            /**
             * @brief Constructs a ClassVarDecNode.
             *
             * @param k The kind of variable.
             * @param t The type of the variable.
             * @param names A vector of variable names.
             */
            ClassVarDecNode(const ClassVarKind k, const std::string_view t, std::vector<std::string_view> names)
                : kind(k), type(t), varNames(std::move(names)) {};
            ~ClassVarDecNode() override = default;

            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<classVarDec>\n";
                out << sp << "  <keyword> " << (kind == ClassVarKind::STATIC ? "static" : "field") << " </keyword>\n";
                out << sp << "  <identifier> " << type << " </identifier>\n";
                for (size_t i = 0; i < varNames.size(); ++i) {
                    out << sp << "  <identifier> " << varNames[i] << " </identifier>\n";
                    if (i < varNames.size() - 1) out << sp << "  <symbol> , </symbol>\n";
                }
                out << sp << "  <symbol> ; </symbol>\n";
                out << sp << "</classVarDec>\n";
            }
    };

    /**
     * @brief Represents a local variable declaration within a subroutine.
     *
     * Example: `var int i, sum;`
     */
    class VarDecNode final : public Node {
        public:
            std::string_view type; ///< The data type of the variable(s).
            std::vector<std::string_view> varNames; ///< A list of variable names declared.

            /**
             * @brief Constructs a VarDecNode.
             *
             * @param t The type of the variable.
             * @param names A vector of variable names.
             */
            VarDecNode(const std::string_view t, std::vector<std::string_view> names)
                : type(t), varNames(std::move(names)) {};
            ~VarDecNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');

                // In the Jack analyzer standard, local variable declarations
                // are wrapped in <varDec> tags.
                out << sp << "<varDec>\n";

                // The 'var' keyword
                out << sp << "  <keyword> var </keyword>\n";

                // The type (int, char, boolean, or className)
                // We treat the type as a keyword if it's a primitive, or an identifier if it's a class.
                if (type == "int" || type == "char" || type == "boolean") {
                    out << sp << "  <keyword> " << type << " </keyword>\n";
                } else {
                    out << sp << "  <identifier> " << type << " </identifier>\n";
                }

                // List of variable names separated by commas
                for (size_t i = 0; i < varNames.size(); ++i) {
                    out << sp << "  <identifier> " << varNames[i] << " </identifier>\n";

                    // Output a comma symbol if there are more names in the list
                    if (i < varNames.size() - 1) {
                        out << sp << "  <symbol> , </symbol>\n";
                    }
                }

                // The mandatory closing semicolon
                out << sp << "  <symbol> ; </symbol>\n";

                out << sp << "</varDec>\n";
            }
    };

    /**
     * @brief Enumeration for the type of subroutine.
     */
    enum class SubroutineType {
        CONSTRUCTOR, ///< A class constructor (creates a new instance).
        FUNCTION,    ///< A static function (belongs to the class).
        METHOD       ///< A method (belongs to an instance).
    };

    /**
     * @brief Represents a single parameter in a subroutine declaration.
     *
     * Example: `int x` in `function void foo(int x)`
     */
    struct Parameter {
        std::string_view type; ///< The data type of the parameter.
        std::string_view name; ///< The name of the parameter.
    };

    /**
     * @brief Base class for all statement nodes.
     *
     * Statements perform actions (e.g., let, if, while, do, return).
     */
    class StatementNode : public Node {
        public:
            ~StatementNode() override = default;
    };

    /**
     * @brief Base class for all expression nodes.
     *
     * Expressions evaluate to a value.
     */
    class ExpressionNode : public Node {
        public:
            ~ExpressionNode() override = default;
    };

    /**
     * @brief Represents an integer literal expression.
     *
     * Example: `42`
     */
    class IntegerLiteralNode final : public ExpressionNode {
        public:
            int value; ///< The integer value.

            /**
             * @brief Constructs an IntegerLiteralNode.
             * @param val The integer value.
             */
            explicit IntegerLiteralNode(const int val) : value(val) {}
            ~IntegerLiteralNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                out << std::string(indent, ' ') << "<integerConstant> " << value << " </integerConstant>\n";
            }
    };

    /**
     * @brief Represents a string literal expression.
     *
     * Example: `"Hello World"`
     */
    class StringLiteralNode final : public ExpressionNode {
        public:
            std::string_view value; ///< The string value (without quotes).

            /**
             * @brief Constructs a StringLiteralNode.
             * @param val The string content.
             */
            explicit StringLiteralNode(const std::string_view val) : value(val) {}
            ~StringLiteralNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                out << std::string(indent, ' ') << "<stringConstant> " << value << " </stringConstant>\n";
            }
    };

    /**
     * @brief Represents a keyword literal expression.
     *
     * Example: `true`, `false`, `null`, `this`
     */
    class KeywordLiteralNode final : public ExpressionNode {
        public:
            Keyword value; ///< The keyword value.

            /**
             * @brief Constructs a KeywordLiteralNode.
             * @param val The keyword.
             */
            explicit KeywordLiteralNode(const Keyword val) : value(val) {}
            void print(std::ostream& out, const int indent) const override {
                    std::string val;
                    switch(value) {
                        case Keyword::TRUE_:  val = "true";  break;
                        case Keyword::FALSE_: val = "false"; break;
                        case Keyword::NULL_:  val = "null";  break;
                        case Keyword::THIS_:  val = "this";  break;
                        default: val="no"; break;
                    }
                    out << std::string(indent, ' ') << "<keyword> " << val << " </keyword>\n";
            }
    };

    /**
     * @brief Represents a binary operation expression.
     *
     * Example: `x + y`, `a < b`
     */

    inline std::string escapeXml(const char op) {
        switch (op) {
            case '<': return "&lt;";
            case '>': return "&gt;";
            case '&': return "&amp;";
            case '"': return "&quot;";
            default:  return std::string(1, op);
        }
    }
    class BinaryOpNode final : public ExpressionNode {
        public:
            std::unique_ptr<ExpressionNode> left; ///< The left operand.
            char op; ///< The operator symbol ('+', '-', '*', '/', '&', '|', '<', '>', '=').
            std::unique_ptr<ExpressionNode> right; ///< The right operand.

            /**
             * @brief Constructs a BinaryOpNode.
             * @param l The left operand.
             * @param o The operator character.
             * @param r The right operand.
             */
            BinaryOpNode(std::unique_ptr<ExpressionNode> l, const char o, std::unique_ptr<ExpressionNode> r)
                : left(std::move(l)), op(o), right(std::move(r)) {}
            ~BinaryOpNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<expression>\n";

                // Left Term
                out << sp << "  <term>\n";
                left->print(out, indent + 4);
                out << sp << "  </term>\n";

                // Operator - ESCAPED HERE
                out << sp << "  <symbol> " << escapeXml(op) << " </symbol>\n";

                // Right Term
                out << sp << "  <term>\n";
                right->print(out, indent + 4);
                out << sp << "  </term>\n";

                out << sp << "</expression>\n";
            }
    };



    /**
     * @brief Represents a unary operation expression.
     *
     * Example: `-x`, `~found`
     */
    class UnaryOpNode final : public ExpressionNode {
        public:
            char op; ///< The operator symbol ('-', '~').
            std::unique_ptr<ExpressionNode> term; ///< The operand.

            /**
             * @brief Constructs a UnaryOpNode.
             * @param o The operator character.
             * @param t The operand.
             */
            UnaryOpNode(const char o, std::unique_ptr<ExpressionNode> t)
                : op(o), term(std::move(t)) {}
            ~UnaryOpNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<term>\n";
                out << sp << "  <symbol> " << op << " </symbol>\n";
                term->print(out, indent + 2);
                out << sp << "</term>\n";
            }
    };

    /**
     * @brief Represents a subroutine call expression.
     *
     * Example: `foo()`, `Math.sqrt(x)`, `obj.method()`
     */
    class CallNode final : public ExpressionNode {
        public:
            std::string_view classNameOrVar; ///< The class name or variable name (optional). Empty if implicit `this`.
            std::string_view functionName;   ///< The name of the subroutine being called.
            std::vector<std::unique_ptr<ExpressionNode>> arguments; ///< The list of arguments passed to the call.

            /**
             * @brief Constructs a CallNode.
             * @param cv The class or variable name (can be empty).
             * @param fn The function name.
             * @param args The arguments.
             */
            CallNode(const std::string_view cv, const std::string_view fn, std::vector<std::unique_ptr<ExpressionNode>> args)
                : classNameOrVar(cv), functionName(fn), arguments(std::move(args)) {}
            ~CallNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                // Note: CallNode is usually wrapped in a <term> or <doStatement> by the caller
                if (!classNameOrVar.empty()) {
                    out << sp << "<identifier> " << classNameOrVar << " </identifier>\n";
                    out << sp << "<symbol> . </symbol>\n";
                }
                out << sp << "<identifier> " << functionName << " </identifier>\n";
                out << sp << "<symbol> ( </symbol>\n";

                out << sp << "<expressionList>\n";
                for (size_t i = 0; i < arguments.size(); ++i) {
                    arguments[i]->print(out, indent + 2);
                    if (i < arguments.size() - 1) {
                        out << sp << "  <symbol> , </symbol>\n";
                    }
                }
                out << sp << "</expressionList>\n";

                out << sp << "<symbol> ) </symbol>\n";
            }
    };

    /**
     * @brief Represents an identifier expression, possibly with an array index.
     *
     * Example: `x`, `arr[i]`
     */
    class IdentifierNode final : public ExpressionNode {
        public:
            std::string_view name; ///< The name of the identifier.
            std::unique_ptr<ExpressionNode> indexExpr; ///< The index expression if it's an array access, otherwise nullptr.

            /**
             * @brief Constructs an IdentifierNode.
             * @param n The identifier name.
             * @param idx The index expression (optional).
             */
            explicit IdentifierNode(const std::string_view n, std::unique_ptr<ExpressionNode> idx = nullptr)
                : name(n), indexExpr(std::move(idx)) {}
            ~IdentifierNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<identifier> " << name << " </identifier>\n";
                if (indexExpr) {
                    out << sp << "<symbol> [ </symbol>\n";
                    indexExpr->print(out, indent + 2);
                    out << sp << "<symbol> ] </symbol>\n";
                }
            }
    };

    /**
     * @brief Represents a 'let' statement (assignment).
     *
     * Example: `let x = 5;`, `let arr[i] = y;`
     */
    class LetStatementNode final : public StatementNode {
        public:
            std::string_view varName; ///< The name of the variable being assigned to.
            std::unique_ptr<ExpressionNode> indexExpr; ///< The index expression for array assignment (optional).
            std::unique_ptr<ExpressionNode> valueExpr; ///< The expression evaluating to the new value.

            /**
             * @brief Constructs a LetStatementNode.
             * @param name The variable name.
             * @param idx The index expression (can be nullptr).
             * @param val The value expression.
             */
            LetStatementNode(const std::string_view name, std::unique_ptr<ExpressionNode> idx,
                             std::unique_ptr<ExpressionNode> val)
                : varName(name), indexExpr(std::move(idx)), valueExpr(std::move(val)) {}
            ~LetStatementNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<letStatement>\n";
                out << sp << "  <keyword> let </keyword>\n";
                out << sp << "  <identifier> " << varName << " </identifier>\n";

                if (indexExpr) {
                    out << sp << "  <symbol> [ </symbol>\n";
                    indexExpr->print(out, indent + 4);
                    out << sp << "  <symbol> ] </symbol>\n";
                }

                out << sp << "  <symbol> = </symbol>\n";
                valueExpr->print(out, indent + 4);
                out << sp << "  <symbol> ; </symbol>\n";
                out << sp << "</letStatement>\n";
            }
    };

    /**
     * @brief Represents an 'if' statement.
     *
     * Example: `if (x > 0) { ... } else { ... }`
     */
    class IfStatementNode final : public StatementNode {
        public:
            std::unique_ptr<ExpressionNode> condition; ///< The condition expression.
            std::vector<std::unique_ptr<StatementNode>> ifStatements; ///< The statements to execute if true.
            std::vector<std::unique_ptr<StatementNode>> elseStatements; ///< The statements to execute if false (optional).

            /**
             * @brief Constructs an IfStatementNode.
             * @param cond The condition.
             * @param ifStmts The 'if' block statements.
             * @param elseStmts The 'else' block statements.
             */
            IfStatementNode(std::unique_ptr<ExpressionNode> cond, std::vector<std::unique_ptr<StatementNode>> ifStmts,
                            std::vector<std::unique_ptr<StatementNode>> elseStmts)
                : condition(std::move(cond)), ifStatements(std::move(ifStmts)), elseStatements(std::move(elseStmts)) {};
            ~IfStatementNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<ifStatement>\n";
                out << sp << "  <keyword> if </keyword>\n";
                out << sp << "  <symbol> ( </symbol>\n";
                condition->print(out, indent + 2);
                out << sp << "  <symbol> ) </symbol>\n";

                out << sp << "  <symbol> { </symbol>\n";
                out << sp << "  <statements>\n";
                for (const auto& stmt : ifStatements) stmt->print(out, indent + 4);
                out << sp << "  </statements>\n";
                out << sp << "  <symbol> } </symbol>\n";

                if (!elseStatements.empty()) {
                    out << sp << "  <keyword> else </keyword>\n";
                    out << sp << "  <symbol> { </symbol>\n";
                    out << sp << "  <statements>\n";
                    for (const auto& stmt : elseStatements) stmt->print(out, indent + 4);
                    out << sp << "  </statements>\n";
                    out << sp << "  <symbol> } </symbol>\n";
                }
                out << sp << "</ifStatement>\n";
            }
    };

    /**
     * @brief Represents a 'while' statement.
     *
     * Example: `while (x > 0) { ... }`
     */
    class WhileStatementNode final : public StatementNode {
        public:
            std::unique_ptr<ExpressionNode> condition; ///< The loop condition.
            std::vector<std::unique_ptr<StatementNode>> body; ///< The loop body statements.

            /**
             * @brief Constructs a WhileStatementNode.
             * @param cond The condition.
             * @param b The body statements.
             */
            WhileStatementNode(std::unique_ptr<ExpressionNode> cond, std::vector<std::unique_ptr<StatementNode>> b)
                : condition(std::move(cond)), body(std::move(b)) {}
            ~WhileStatementNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<whileStatement>\n";
                out << sp << "  <keyword> while </keyword>\n";
                out << sp << "  <symbol> ( </symbol>\n";
                condition->print(out, indent + 2);
                out << sp << "  <symbol> ) </symbol>\n";

                out << sp << "  <symbol> { </symbol>\n";
                out << sp << "  <statements>\n";
                for (const auto& stmt : body) stmt->print(out, indent + 4);
                out << sp << "  </statements>\n";
                out << sp << "  <symbol> } </symbol>\n";
                out << sp << "</whileStatement>\n";
            }
    };

    /**
     * @brief Represents a 'do' statement.
     *
     * Example: `do Output.printInt(x);`
     * 'do' statements are used to call subroutines for their side effects, ignoring the return value.
     */
    class DoStatementNode final : public StatementNode {
        public:
            std::unique_ptr<CallNode> callExpression; ///< The subroutine call expression.

            /**
             * @brief Constructs a DoStatementNode.
             * @param call The call expression.
             */
            explicit DoStatementNode(std::unique_ptr<CallNode> call) : callExpression(std::move(call)) {}
            ~DoStatementNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<doStatement>\n";
                out << sp << "  <keyword> do </keyword>\n";

                // Calls the print method of the CallNode
                callExpression->print(out, indent + 2);

                out << sp << "  <symbol> ; </symbol>\n";
                out << sp << "</doStatement>\n";
            }
    };

    /**
     * @brief Represents a 'return' statement.
     *
     * Example: `return;`, `return x + 1;`
     */
    class ReturnStatementNode final : public StatementNode {
        public:
            std::unique_ptr<ExpressionNode> expression; ///< The return value expression (optional).

            /**
             * @brief Constructs a ReturnStatementNode.
             * @param expr The return expression (can be nullptr).
             */
            explicit ReturnStatementNode(std::unique_ptr<ExpressionNode> expr) : expression(std::move(expr)) {}
            ~ReturnStatementNode() override = default;
            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<returnStatement>\n";
                out << sp << "  <keyword> return </keyword>\n";

                if (expression) {
                    expression->print(out, indent + 2);
                }

                out << sp << "  <symbol> ; </symbol>\n";
                out << sp << "</returnStatement>\n";
            }
    };

    /**
     * @brief Represents a subroutine declaration (constructor, function, or method).
     */
    class SubroutineDecNode final : public Node {
        public:
            SubroutineType subType; ///< The type of subroutine (constructor, function, method).
            std::string_view returnType; ///< The return type (e.g., "void", "int", "MyClass").
            std::string_view name; ///< The name of the subroutine.
            std::vector<Parameter> parameters; ///< The list of parameters.

            std::vector<std::unique_ptr<VarDecNode>> localVars; ///< The local variable declarations.
            std::vector<std::unique_ptr<StatementNode>> statements; ///< The body statements.

            /**
             * @brief Constructs a SubroutineDecNode.
             * @param st The subroutine type.
             * @param ret The return type.
             * @param n The name.
             * @param parameters
             * @param vars The local variables.
             * @param stmts The body statements.
             */
            SubroutineDecNode(const SubroutineType st, const std::string_view ret, const std::string_view n,
                std::vector<Parameter> parameters, std::vector<std::unique_ptr<VarDecNode>> vars,
                std::vector<std::unique_ptr<StatementNode>> stmts)
                : subType(st), returnType(ret), name(n),parameters(std::move(parameters)),localVars(std::move(vars))
                ,statements(std::move(stmts)) {}

            ~SubroutineDecNode() override = default;

            void print(std::ostream& out, const int indent) const override {
                const std::string sp(indent, ' ');
                out << sp << "<subroutineDec>\n";
                std::string typeStr = (subType == SubroutineType::CONSTRUCTOR ? "constructor" :
                                      (subType == SubroutineType::FUNCTION ? "function" : "method"));
                out << sp << "  <keyword> " << typeStr << " </keyword>\n";
                out << sp << "  <identifier> " << returnType << " </identifier>\n";
                out << sp << "  <identifier> " << name << " </identifier>\n";
                out << sp << "  <symbol> ( </symbol>\n";
                out << sp << "  <parameterList>\n";
                for (size_t i = 0; i < parameters.size(); ++i) {
                    out << sp << "    <keyword> " << parameters[i].type << " </keyword>\n";
                    out << sp << "    <identifier> " << parameters[i].name << " </identifier>\n";
                    if (i < parameters.size() - 1) out << sp << "    <symbol> , </symbol>\n";
                }
                out << sp << "  </parameterList>\n";
                out << sp << "  <symbol> ) </symbol>\n";
                out << sp << "  <subroutineBody>\n";
                out << sp << "    <symbol> { </symbol>\n";
                for (const auto& var : localVars) var->print(out, indent + 4);
                for (const auto& stmt : statements) stmt->print(out, indent + 4);
                out << sp << "    <symbol> } </symbol>\n";
                out << sp << "  </subroutineBody>\n";
                out << sp << "</subroutineDec>\n";
            }
    };

    /**
     * @brief Represents a complete Jack class.
     *
     * This is the root node of the AST for a single file.
     */
    class ClassNode final : public Node {
        public:
            std::string_view className; ///< The name of the class.
            std::vector<std::unique_ptr<ClassVarDecNode>> classVars; ///< The class-level variable declarations.
            std::vector<std::unique_ptr<SubroutineDecNode>> subroutineDecs; ///< The subroutine declarations.

            /**
             * @brief Constructs a ClassNode.
             * @param className The name of the class.
             */
            explicit ClassNode(const std::string_view className) : className(className) {};
            ~ClassNode() override = default;
            void print(std::ostream& out, int indent)const override {
                out << "<class>\n";
                out << "  <keyword> class </keyword>\n";
                out << "  <identifier> " << className << " </identifier>\n";
                out << "  <symbol> { </symbol>\n";
                for (const auto& var : classVars) var->print(out, 2);
                for (const auto& sub : subroutineDecs) sub->print(out, 2);
                out << "  <symbol> } </symbol>\n";
                out << "</class>\n";
            }
    };
}

#endif //NAND2TETRIS_AST_H