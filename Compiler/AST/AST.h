//
// Created by Nithin Kondabathini on 13/1/2026.
//

#ifndef NAND2TETRIS_AST_H
#define NAND2TETRIS_AST_H

#include <utility>
#include <vector>
#include <memory>
#include<iostream>
#include "../Tokenizer/TokenTypes.h"
#include "ASTVisitor.h"

namespace nand2tetris::jack {

    /**
     * @brief Enumeration of all possible AST node types.
     *
     * Used for runtime type identification of AST nodes.
     */
    enum class ASTNodeType {
        // --- High-Level Structure ---
        CLASS,              ///< ClassNode
        CLASS_VAR_DEC,      ///< ClassVarDecNode (static/field)
        SUBROUTINE_DEC,     ///< SubroutineDecNode (function/method/constructor)
        VAR_DEC,            ///< VarDecNode (local vars)

        // --- Statements ---
        LET_STATEMENT,      ///< LetStatementNode
        IF_STATEMENT,       ///< IfStatementNode
        WHILE_STATEMENT,    ///< WhileStatementNode
        DO_STATEMENT,       ///< DoStatementNode
        RETURN_STATEMENT,   ///< ReturnStatementNode

        // --- Expressions (Terms) ---
        INTEGER_LITERAL,    ///< IntegerLiteralNode
        FLOAT_LITERAL,      ///< FloatLiteralNode
        STRING_LITERAL,     ///< StringLiteralNode
        KEYWORD_LITERAL,    ///< KeywordLiteralNode
        BINARY_OP,          ///< BinaryOpNode
        UNARY_OP,           ///< UnaryOpNode
        SUBROUTINE_CALL,    ///< CallNode
        IDENTIFIER          ///< IdentifierNode
    };

    /**
     * @brief Base class for all nodes in the Abstract Syntax Tree (AST).
     *
     * All specific AST nodes inherit from this class. It provides a virtual destructor
     * to ensure proper cleanup of derived classes. It also stores the location (line, column)
     * of the node in the source code for error reporting.
     */
    class Node {
        public:
            /**
             * @brief Constructs a Node.
             *
             * @param l The line number in the source code.
             * @param c The column number in the source code.
             */
            explicit Node(const int l, const int c):line(l),column(c){};
            virtual ~Node() = default;

            virtual void accept(ASTVisitor& visitor)const =0;

            /**
             * @brief Gets the line number of the node.
             * @return The line number.
             */
            int getLine() const { return line; }

            /**
             * @brief Gets the column number of the node.
             * @return The column number.
             */
            int getCol() const { return column; }
        protected:
            const int line;   ///< Line number in source.
            const int column; ///< Column number in source.
    };

    struct Type {
        private:
            std::string_view baseType; // e.g., "Array", "int"
            std::vector<std::unique_ptr<Type>> genericArgs; // e.g., <int>
            bool isConst = false;   // For strict C++ style typing
        public:
            // Constructors
            Type() : baseType("void") {}
            explicit Type(const std::string_view base) : baseType(base) {}

            Type(const Type& other) : baseType(other.baseType), isConst(other.isConst) {
                    // Deep copy each generic argument by creating new unique_ptrs
                    for (const auto& arg : other.genericArgs) {
                        if (arg) {
                            genericArgs.push_back(std::make_unique<Type>(*arg)); // Recursive copy
                        }
                    }
                }

            // You should also define the move constructor to keep it efficient
            Type(Type&&) noexcept = default;
            Type& operator=(Type&&) noexcept = default;

            bool operator==(const Type& other) const {
                // 1. Check base types (e.g., "Array" == "Array")
                if (baseType != other.baseType) return false;

                // 2. Check generic counts (e.g., <int> vs <int, float>)
                if (genericArgs.size() != other.genericArgs.size()) return false;

                // 3. Deep recursive comparison of every generic argument
                for (size_t i = 0; i < genericArgs.size(); ++i) {
                    if (!(*genericArgs[i] == *other.genericArgs[i])) return false;
                }

                return true;
            }
            [[nodiscard]] std::string_view getBaseType() const { return baseType; }
            [[nodiscard]] bool is_Const() const { return isConst; }
            [[nodiscard]] const std::vector<std::unique_ptr<Type>>& getGenericArgs() const { return genericArgs; }

            std::string formatType() const {
                auto s = std::string(this->getBaseType());
                if (this->isGeneric()) {
                    s += "<";
                    const auto& generics=this->getGenericArgs();
                    for (size_t i = 0; i < generics.size(); ++i) {
                        s += generics[i]->formatType();
                        if (i < generics.size() - 1) s += ", ";
                    }
                    s += ">";
                }
                return s;
            }

            void addGenericArg(std::unique_ptr<Type> arg) {
                    genericArgs.push_back(std::move(arg));
            }

            void setConst(const bool value) { isConst = value; }

            [[nodiscard]] int getBitWidth() const {
                    if (baseType == "int") return 32;
                    if (baseType == "char" || baseType == "boolean") return 8;
                    return 64; // Pointers/Objects
            }

            [[nodiscard]] bool isPrimitive() const {
                    return (baseType == "int" || baseType == "char" || baseType == "boolean"||baseType=="float");
            }

            [[nodiscard]] bool isGeneric() const {
                    return !genericArgs.empty();
            }

    };

    /**
     * @brief Base class for all statement nodes.
     *
     * Statements perform actions (e.g., let, if, while, do, return).
     */
    class StatementNode : public Node {public:using Node::Node;};

    /**
     * @brief Base class for all expression nodes.
     *
     * Expressions evaluate to a value.
     */
    class ExpressionNode : public Node {
        public: using Node::Node;
        const Type* const resolvedType=nullptr;
        virtual bool isCall() const { return false; }
    };


    /**
     * @brief Represents an integer literal expression.
     *
     * Example: `42`
     */
    class IntegerLiteralNode final : public ExpressionNode {
        protected:
            int32_t value; ///< The integer value.
        public:
            /**
             * @brief Constructs an IntegerLiteralNode.
             * @param val The integer value.
             * @param l The line number.
             * @param c The column number.
             */
            explicit IntegerLiteralNode(const int32_t val,const int l, const int c): ExpressionNode (l,c),value(val) {}
            ~IntegerLiteralNode() override = default;
            void accept(ASTVisitor& v) const override { v.visit(*this); }
            const int32_t getInt() const{return value;}
    };

    /**
     * @brief Represents an float literal expression.
     *
     * Example: `42.3`
     */
    class FloatLiteralNode final : public ExpressionNode {
        protected:
            double value;
        public:
            /**
             * @brief Constructs an FloatLiteralNode.
             * @param val The integer value.
             * @param l The line number.
             * @param c The column number.
             */
            explicit FloatLiteralNode(const double val, const int l, const int c)
                :ExpressionNode (l,c),value(val) {}
            ~FloatLiteralNode() override = default;
            void accept(ASTVisitor& v)const override { v.visit(*this); }
            const double getFloat() const{return value;}
    };



    /**
     * @brief Represents a string literal expression.
     *
     * Example: `"Hello World"`
     */
    class StringLiteralNode final : public ExpressionNode {
        protected:
            std::string_view value; ///< The string value (without quotes).
        public:
            /**
             * @brief Constructs a StringLiteralNode.
             * @param val The string content.
             * @param l The line number.
             * @param c The column number.
             */
            explicit StringLiteralNode(const std::string_view val,const int l, const int c) : ExpressionNode(l,c),value(val) {}
            ~StringLiteralNode() override = default;
            void accept(ASTVisitor& v) const override { v.visit(*this); }
            const std::string_view getString() const{return value;}
    };

    /**
     * @brief Represents a keyword literal expression.
     *
     * Example: `true`, `false`, `null`, `this`
     */
    class KeywordLiteralNode final : public ExpressionNode {
        protected:
            Keyword value; ///< The keyword value.
        public:
            /**
             * @brief Constructs a KeywordLiteralNode.
             * @param val The keyword.
             * @param l The line number.
             * @param c The column number.
             */
            explicit KeywordLiteralNode(const Keyword val, const int l, const int c) :ExpressionNode(l,c),value(val) {}
            ~KeywordLiteralNode() override = default;

            void accept(ASTVisitor& v)const override { v.visit(*this); }
            Keyword getKeyword() const{return value;}
    };


    /**
     * @brief Represents a binary operation expression.
     *
     * Example: `x + y`, `a < b`
     */
    class BinaryOpNode final : public ExpressionNode {
        protected:
            std::unique_ptr<ExpressionNode> left; ///< The left operand.
            char op; ///< The operator symbol ('+', '-', '*', '/', '&', '|', '<', '>', '=').
            std::unique_ptr<ExpressionNode> right; ///< The right operand.
        public:
            /**
             * @brief Constructs a BinaryOpNode.
             * @param l The left operand.
             * @param o The operator character.
             * @param r The right operand.
             * @param line The line number.
             * @param column The column number.
             */
            BinaryOpNode(std::unique_ptr<ExpressionNode> l, const char o, std::unique_ptr<ExpressionNode> r,const int
                line, const int column)
                : ExpressionNode(line, column),left(std::move(l)), op(o), right(std::move(r)) {}
            ~BinaryOpNode() override = default;

            void accept(ASTVisitor& v)const override { v.visit(*this); }

            const char getOp() const{return op;}
            const ExpressionNode& getLeft() const { return *left; }
            const ExpressionNode& getRight() const { return *right; }
    };


    /**
     * @brief Represents a unary operation expression.
     *
     * Example: `-x`, `~found`
     */
    class UnaryOpNode final : public ExpressionNode {
        protected:
            char op; ///< The operator symbol ('-', '~').
            std::unique_ptr<ExpressionNode> term; ///< The operand.
        public:
            /**
             * @brief Constructs a UnaryOpNode.
             * @param o The operator character.
             * @param t The operand.
             * @param line The line number.
             * @param column The column number.
             */
            UnaryOpNode(const char o, std::unique_ptr<ExpressionNode> t,const int line, const int column)
                : ExpressionNode(line, column),op(o), term(std::move(t)) {}
            ~UnaryOpNode() override = default;
            void accept(ASTVisitor& v)const override { v.visit(*this); }
            const char getOp() const { return op; }
            const ExpressionNode& getTerm() const { return *term; }
    };

    /**
     * @brief Represents an identifier expression, possibly with an array index.
     *
     * Example: `x`, `arr[i]`
     */
    //
    class IdentifierNode final : public ExpressionNode {
        protected:
            std::string_view name; ///< The name of the identifier.
            std::vector<const Type*> genericArgs; // Holds <int>, <String>, etc.
        public:
            /**
             * @brief Constructs an IdentifierNode.
             * @param n The identifier name.
             * @param generics stores the types
             * @param l The line number.
             * @param c The column number.
             */
            IdentifierNode(const std::string_view n, std::vector<const Type*> generics,const int l, const int
                c): ExpressionNode(l,c) ,name(n), genericArgs(std::move(generics)) {}
            ~IdentifierNode() override = default;

            void accept(ASTVisitor& v) const override { v.visit(*this); }
            const std::string_view getName() const { return name; }
            const std::vector<const Type*>& getGenericArgs() const { return genericArgs;}
    };

    class ArrayAccessNode final : public ExpressionNode {
        std::unique_ptr<ExpressionNode> base;  // The array (e.g., 'arr')
        std::unique_ptr<ExpressionNode> index; // The subscript (e.g., 'i')
    public:
        ArrayAccessNode(std::unique_ptr<ExpressionNode> b, std::unique_ptr<ExpressionNode> idx, int l, int c)
            : ExpressionNode(l, c), base(std::move(b)), index(std::move(idx)) {}

        void accept(ASTVisitor& v)const override { v.visit(*this); }

        const ExpressionNode& getBase() const { return *base; }
        const ExpressionNode& getIndex() const { return *index; }
    };

    /**
     * @brief Represents a subroutine call expression.
     *
     * Example: `foo()`, `Math.sqrt(x)`, `obj.method()`
     */
    //
    class CallNode final : public ExpressionNode {
        protected:
            std::unique_ptr<ExpressionNode> receiver;
            std::string_view functionName;   ///< The name of the subroutine being called.
            std::vector<std::unique_ptr<ExpressionNode>> arguments; ///< The list of arguments passed to the call.
        public:
            /**
             * @brief Constructs a CallNode.
             * @param rec The class or variable name.
             * @param fn The function name.
             * @param args The arguments.
             * @param l The line number.
             * @param c The column number.
             */
            CallNode(std::unique_ptr<ExpressionNode> rec, const std::string_view fn,
                std::vector<std::unique_ptr<ExpressionNode>> args, const int l, const int c)
                : ExpressionNode(l, c),receiver(std::move(rec)), functionName(fn), arguments(std::move(args)) {}

            ~CallNode() override = default;

            void accept(ASTVisitor& v) const override { v.visit(*this); }
            bool isCall() const override { return true; }
            const ExpressionNode* const getReceiver() const { return receiver.get(); }
            const std::string_view getFunctionName() const { return functionName; }
            const std::vector<const ExpressionNode*> getArgs() const {
                std::vector<const ExpressionNode*> views;
                views.reserve(arguments.size());
                for (const auto& arg : arguments) {
                    views.push_back(arg.get()); // Provides a raw observer pointer
                }
                return views; // Implicitly converts to const std::vector on return
            }
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
        protected:
            ClassVarKind kind; ///< The kind of variable (static or field).
            const Type* const type; ///< The data type of the variable(s) (e.g., "int", "boolean", "MyClass").
            std::vector<std::string_view> varNames; ///< A list of variable names declared in this statement.
        public:
            /**
             * @brief Constructs a ClassVarDecNode.
             *
             * @param k The kind of variable.
             * @param t The type of the variable.
             * @param names A vector of variable names.
             * @param l the line on source code.
             * @param c the column on source code.
             */
            ClassVarDecNode(const ClassVarKind k, const Type*const t, std::vector<std::string_view> names,
                const int l, const int c):Node(l,c),kind(k),type(t), varNames(std::move(names)) {};
            ~ClassVarDecNode() override = default;
            void accept(ASTVisitor& v)const override { v.visit(*this); }

            const ClassVarKind getKind() const { return kind; }
            const Type& getType() const { return *type; }
            const std::vector<std::string_view>& getVarNames() const { return varNames; }

    };

    /**
     * @brief Represents a local variable declaration within a subroutine.
     *
     * Example: `var int i, sum;`
     */
    class VarDecNode final : public Node {
        protected:
            const Type* const type; // Changed from string_view ///< The data type of the variable(s).
            std::vector< std::string_view> varNames; ///< A list of variable names declared.
        public:
            /**
             * @brief Constructs a VarDecNode.
             *
             * @param t The type of the variable.
             * @param names A vector of variable names.
             * @param l The line number.
             * @param c The column number.
             */
            VarDecNode(const Type* const t, std::vector<std::string_view> names, const int l, const int c)
                : Node(l,c),type(t), varNames(std::move(names)) {};
            ~VarDecNode() override = default;
            void accept(ASTVisitor& v) const override { v.visit(*this); }

            const Type& getType() const { return *type; }
            std::vector<std::string_view> getVarNames() const { return varNames; }
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
    class Parameter {
            const Type* const type; // The encapsulated Type heart
            std::string_view name;      // The parameter's identifier
        public:
            Parameter(const Type* const paramType, const std::string_view paramName)
            : type(paramType), name(paramName) {}

            // --- Public Getters (Read-Only) ---
            [[nodiscard]] const Type& getType() const { return *type; }
            [[nodiscard]] const std::string_view getName() const { return name; }
    };


    /**
     * @brief Represents a 'let' statement (assignment).
     *
     * Example: `let x = 5;`, `let arr[i] = y;`
     */
    class LetStatementNode final : public StatementNode {
        protected:
            std::string_view varName; ///< The name of the variable being assigned to.
            std::unique_ptr<ExpressionNode> indexExpr; ///< The index expression for array assignment (optional).
            std::unique_ptr<ExpressionNode> valueExpr; ///< The expression evaluating to the new value.
        public:
            /**
             * @brief Constructs a LetStatementNode.
             * @param name The variable name.
             * @param idx The index expression (can be nullptr).
             * @param val The value expression.
             * @param l The line number.
             * @param c The column number.
             */
            LetStatementNode(const std::string_view name, std::unique_ptr<ExpressionNode> idx,
                             std::unique_ptr<ExpressionNode> val,const int l, const int c)
                : StatementNode(l,c) ,varName(name), indexExpr(std::move(idx)), valueExpr(std::move(val)) {}
            ~LetStatementNode() override = default;

            void accept(ASTVisitor& v)const override { v.visit(*this); }

            std::string_view getVarName() const { return varName; }
            const ExpressionNode*const getIndex() const { return indexExpr.get(); }
            const ExpressionNode& getValue() const { return *valueExpr; }
    };

    /**
     * @brief Represents an 'if' statement.
     *
     * Example: `if (x > 0) { ... } else { ... }`
     */
    class IfStatementNode final : public StatementNode {
        protected:
            std::unique_ptr<ExpressionNode> condition; ///< The condition expression.
            std::vector<std::unique_ptr<StatementNode>> ifStatements; ///< The statements to execute if true.
            std::vector<std::unique_ptr<StatementNode>> elseStatements; ///< The statements to execute if false (optional).
        public:
            /**
             * @brief Constructs an IfStatementNode.
             * @param cond The condition.
             * @param ifStmts The 'if' block statements.
             * @param elseStmts The 'else' block statements.
             * @param l The line number.
             * @param c The column number.
             */
            IfStatementNode(std::unique_ptr<ExpressionNode> cond, std::vector<std::unique_ptr<StatementNode>> ifStmts,
                            std::vector<std::unique_ptr<StatementNode>> elseStmts,const int l, const int c)
                : StatementNode(l,c) ,condition(std::move(cond)), ifStatements(std::move(ifStmts)),
                elseStatements(std::move(elseStmts)){};
            ~IfStatementNode() override = default;

            void accept(ASTVisitor& v)const override { v.visit(*this); }

            const ExpressionNode& getCondition() const { return *condition; }
            const std::vector<const StatementNode*> getIfBranch() const {
                std::vector<const StatementNode*> views;
                views.reserve(ifStatements.size());
                for (const auto& stmt : ifStatements) {
                    views.push_back(stmt.get()); // Provides a raw observer pointer
                }
                return views;
            }

            const std::vector<const StatementNode*> getElseBranch() const {
                std::vector<const StatementNode*> views;
                views.reserve(elseStatements.size());
                for (const auto& stmt : elseStatements) {
                    views.push_back(stmt.get()); // Provides a raw observer pointer
                }
                return views;
            }
    };

    /**
     * @brief Represents a 'while' statement.
     *
     * Example: `while (x > 0) { ... }`
     */
    class WhileStatementNode final : public StatementNode {
        protected:
            std::unique_ptr<ExpressionNode> condition; ///< The loop condition.
            std::vector<std::unique_ptr<StatementNode>> body; ///< The loop body statements.
        public:
            /**
             * @brief Constructs a WhileStatementNode.
             * @param cond The condition.
             * @param b The body statements.
             * @param l The line number.
             * @param c The column number.
             */
            WhileStatementNode(std::unique_ptr<ExpressionNode> cond, std::vector<std::unique_ptr<StatementNode>> b,
                const int l,const int c)
                : StatementNode(l,c),condition(std::move(cond)), body(std::move(b)) {}
            ~WhileStatementNode() override = default;

            void accept(ASTVisitor& v)const override { v.visit(*this); }
            const ExpressionNode& getCondition() const { return *condition; }
            const std::vector<const StatementNode*> getBody() const {
                std::vector<const StatementNode*> views;
                views.reserve(body.size());
                for (const auto& stmt : body) {
                    views.push_back(stmt.get()); // Provides a raw observer pointer
                }
                return views;
            }
    };

    /**
     * @brief Represents a 'do' statement.
     *
     * Example: `do Output.printInt(x);`
     * 'do' statements are used to call subroutines for their side effects, ignoring the return value.
     */
    class DoStatementNode final : public StatementNode {
        protected:
            std::unique_ptr<CallNode> callExpression; ///< The subroutine call expression.
        public:
            /**
             * @brief Constructs a DoStatementNode.
             * @param call The call expression.
             * @param l The line number.
             * @param c The column number.
             */
            explicit DoStatementNode(std::unique_ptr<CallNode> call,const int l, const int c) : StatementNode(l,c),callExpression(std::move(call)){};
            ~DoStatementNode() override = default;

            void accept(ASTVisitor& v)const override { v.visit(*this); }
            const CallNode& getCall() const { return *callExpression; }
    };

    /**
     * @brief Represents a 'return' statement.
     *
     * Example: `return;`, `return x + 1;`
     */
    class ReturnStatementNode final : public StatementNode {
        protected:
            std::unique_ptr<ExpressionNode> expression; ///< The return value expression (optional).

        public:
            /**
             * @brief Constructs a ReturnStatementNode.
             * @param expr The return expression (can be nullptr).
             * @param l The line number.
             * @param c The column number.
             */
            explicit ReturnStatementNode(std::unique_ptr<ExpressionNode> expr,const int l,const int c) : StatementNode
                (l,c), expression(std::move(expr)) {}
            ~ReturnStatementNode() override = default;
            void accept(ASTVisitor& v)const override { v.visit(*this); }
            const ExpressionNode*const getExpression() const { return expression.get(); }
    };

    /**
     * @brief Represents a subroutine declaration (constructor, function, or method).
     */
    class SubroutineDecNode final : public Node {
        protected:
            SubroutineType subType; ///< The type of subroutine (constructor, function, method).
            const Type* const returnType; ///< The return type (e.g., "void", "int", "MyClass").
            std::string_view name; ///< The name of the subroutine.
            std::vector<Parameter> parameters; ///< The list of parameters.

            std::vector<std::unique_ptr<VarDecNode>> localVars; ///< The local variable declarations.
            std::vector<std::unique_ptr<StatementNode>> statements; ///< The body statements.
        public:
            /**
             * @brief Constructs a SubroutineDecNode.
             * @param st The subroutine type.
             * @param ret The return type.
             * @param n The name.
             * @param parameters The list of parameters.
             * @param vars The local variables.
             * @param stmts The body statements.
             * @param l The line number.
             * @param c The column number.
             */
            SubroutineDecNode(const SubroutineType st, const Type* const ret, const std::string_view n,
                std::vector<Parameter> parameters, std::vector<std::unique_ptr<VarDecNode>> vars,
                std::vector<std::unique_ptr<StatementNode>> stmts,const int l, const int c)
                : Node(l,c),subType(st), returnType(ret), name(n),parameters(std::move
                    (parameters)),localVars(std::move
                    (vars)),statements(std::move(stmts)) {};

            ~SubroutineDecNode() override = default;

            void accept(ASTVisitor& v) const override { v.visit(*this); }

            const SubroutineType getSubType() const { return subType; }
            const Type& getReturnType() const { return *returnType; }
            const std::string_view getName() const { return name; }

            const std::vector<const Parameter*> getParams() const {
                std::vector<const Parameter*> views;
                views.reserve(parameters.size());
                for (const auto& param : parameters) {
                    views.push_back(&param); // Capture address of the parameter
                }
                return views;
            }

            const std::vector<const VarDecNode*> getLocals() const {
                std::vector<const VarDecNode*> views;
                views.reserve(localVars.size());
                for (const auto& var : localVars) {
                    views.push_back(var.get()); // Observer pointer
                }
                return views;
            }

            const std::vector<const StatementNode*> getStatements() const {
                std::vector<const StatementNode*> views;
                views.reserve(statements.size());
                for (const auto& stmt : statements) {
                    views.push_back(stmt.get()); // Observer pointer
                }
                return views;
            }
    };

    /**
     * @brief Represents a complete Jack class.
     *
     * This is the root node of the AST for a single file.
     */
    class ClassNode final : public Node {
        protected:
            std::string_view className; ///< The name of the class.
            std::vector<std::unique_ptr<ClassVarDecNode>> classVars; ///< The class-level variable declarations.
            std::vector<std::unique_ptr<SubroutineDecNode>> subroutineDecs; ///< The subroutine declarations.
        public:
            /**
             * @brief Constructs a ClassNode.
             * @param className The name of the class.
             * @param classVars The list of class variables (Static and Field)
             * @param subroutineDecs The list of subroutine declarations (constructors, method, function)
             * @param l The line number.
             * @param c The column number.
             */
            explicit ClassNode(const std::string_view className,std::vector<std::unique_ptr<ClassVarDecNode>>
                classVars,std::vector<std::unique_ptr<SubroutineDecNode>> subroutineDecs,const int l, const int c) :
                Node(l,c),className(className),classVars(std::move(classVars)), subroutineDecs(std::move(subroutineDecs)) {};
            ~ClassNode() override = default;
            void accept(ASTVisitor& visitor)const override{visitor.visit(*this);}
            const std::string_view getClassName()const { return className; }

            const std::vector<const ClassVarDecNode*> getClassVars() const {
                std::vector<const ClassVarDecNode*> views;
                views.reserve(classVars.size());
                for (const auto& var : classVars) {
                    views.push_back(var.get()); //
                }
                return views; // Implicitly converts to const std::vector on return
            }

            const std::vector<const SubroutineDecNode*> getSubroutines() const {
                std::vector<const SubroutineDecNode*> views;
                views.reserve(subroutineDecs.size());
                for (const auto& sub : subroutineDecs) {
                    views.push_back(sub.get()); //
                }
                return views;
            }
    };

}

#endif //NAND2TETRIS_AST_H