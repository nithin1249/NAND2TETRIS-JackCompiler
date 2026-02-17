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

            virtual void accept(ASTVisitor& visitor)=0;

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
        std::string_view baseType; // e.g., "Array", "int"
        std::vector<std::unique_ptr<Type>> genericArgs; // e.g., <int>
        bool isConst = false;   // For strict C++ style typing

        // Essential for LLVM: maps this Jack type to a native bit-width
        int getBitWidth() const {
            if (baseType == "int") return 32;
            if (baseType == "char" || baseType == "boolean") return 8;
            return 64; // Pointers/Objects in 64-bit systems
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
        std::shared_ptr<Type> resolvedType=nullptr;
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
            void accept(ASTVisitor& v) override { v.visit(*this); }
            int32_t getInt() const{return value;}
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
            void accept(ASTVisitor& v) override { v.visit(*this); }
            double getFloat() const{return value;}
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
            void accept(ASTVisitor& v) override { v.visit(*this); }
            std::string_view getString() const{return value;}
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

            void accept(ASTVisitor& v) override { v.visit(*this); }
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

            void accept(ASTVisitor& v) override { v.visit(*this); }

            char getOp() const{return op;}
            ExpressionNode* getLeft() const { return left.get(); }
            ExpressionNode* getRight() const { return right.get(); }
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
            void accept(ASTVisitor& v) override { v.visit(*this); }
            char getOp() const { return op; }
            ExpressionNode* getTerm() const { return term.get(); }
    };

    /**
     * @brief Represents an identifier expression, possibly with an array index.
     *
     * Example: `x`, `arr[i]`
     */
    class IdentifierNode final : public ExpressionNode {
        protected:
            std::string_view name; ///< The name of the identifier.
            std::vector<std::unique_ptr<Type>> genericArgs; // Holds <int>, <String>, etc.
        public:
            /**
             * @brief Constructs an IdentifierNode.
             * @param n The identifier name.
             * @param generics stores the types
             * @param l The line number.
             * @param c The column number.
             */
            IdentifierNode(const std::string_view n,std::vector<std::unique_ptr<Type>> generics,const int l, const int
                c): ExpressionNode(l,c) ,name(n), genericArgs(std::move(generics)) {}
            ~IdentifierNode() override = default;

            void accept(ASTVisitor& v) override { v.visit(*this); }
            std::string_view getName() const { return name; }
            const std::vector<std::unique_ptr<Type>>& getGenericArgs() const { return genericArgs; }
    };

    class ArrayAccessNode final : public ExpressionNode {
        std::unique_ptr<ExpressionNode> base;  // The array (e.g., 'arr')
        std::unique_ptr<ExpressionNode> index; // The subscript (e.g., 'i')
    public:
        ArrayAccessNode(std::unique_ptr<ExpressionNode> b, std::unique_ptr<ExpressionNode> idx, int l, int c)
            : ExpressionNode(l, c), base(std::move(b)), index(std::move(idx)) {}

        void accept(ASTVisitor& v) override { v.visit(*this); }

        ExpressionNode* getBase() const { return base.get(); }
        ExpressionNode* getIndex() const { return index.get(); }
    };

    /**
     * @brief Represents a subroutine call expression.
     *
     * Example: `foo()`, `Math.sqrt(x)`, `obj.method()`
     */
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

            void accept(ASTVisitor& v) override { v.visit(*this); }
            bool isCall() const override { return true; }
            ExpressionNode* getReceiver() const { return receiver.get(); }
            std::string_view getFunctionName() const { return functionName; }
            const std::vector<std::unique_ptr<ExpressionNode>>& getArgs() const { return arguments; }
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
            std::shared_ptr<Type> type; ///< The data type of the variable(s) (e.g., "int", "boolean", "MyClass").
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
            ClassVarDecNode(const ClassVarKind k, std::shared_ptr<Type> t, std::vector<std::string_view> names,
                const int l, const int c):Node(l,c),kind(k),type(std::move(t)), varNames(std::move(names)) {};
            ~ClassVarDecNode() override = default;
            void accept(ASTVisitor& v) override { v.visit(*this); }

            ClassVarKind getKind() const { return kind; }
            const Type* getType() const { return type.get(); }
            const std::vector<std::string_view>& getVarNames() const { return varNames; }

    };

    /**
     * @brief Represents a local variable declaration within a subroutine.
     *
     * Example: `var int i, sum;`
     */
    class VarDecNode final : public Node {
        protected:
           std::shared_ptr<Type> type; // Changed from string_view ///< The data type of the variable(s).
            std::vector<std::string_view> varNames; ///< A list of variable names declared.
        public:
            /**
             * @brief Constructs a VarDecNode.
             *
             * @param t The type of the variable.
             * @param names A vector of variable names.
             * @param l The line number.
             * @param c The column number.
             */
            VarDecNode(std::shared_ptr<Type> t, std::vector<std::string_view> names, const int l, const int c)
                : Node(l,c),type(std::move(t)), varNames(std::move(names)) {};
            ~VarDecNode() override = default;
            void accept(ASTVisitor& v) override { v.visit(*this); }

            Type* getType() const { return type.get(); }
            const std::vector<std::string_view>& getVarNames() const { return varNames; }
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
        std::shared_ptr<Type> type; ///< The data type of the parameter.
        std::string_view name; ///< The name of the parameter.
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

            void accept(ASTVisitor& v) override { v.visit(*this); }

            std::string_view getVarName() const { return varName; }
            ExpressionNode* getIndex() const { return indexExpr.get(); }
            ExpressionNode* getValue() const { return valueExpr.get(); }
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

            void accept(ASTVisitor& v) override { v.visit(*this); }

            ExpressionNode* getCondition() const { return condition.get(); }
            const std::vector<std::unique_ptr<StatementNode>>& getIfBranch() const { return ifStatements; }
            const std::vector<std::unique_ptr<StatementNode>>& getElseBranch() const { return elseStatements; }
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

            void accept(ASTVisitor& v) override { v.visit(*this); }
            ExpressionNode* getCondition() const { return condition.get(); }
            const std::vector<std::unique_ptr<StatementNode>>& getBody() const { return body; }
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

            void accept(ASTVisitor& v) override { v.visit(*this); }
            CallNode* getCall() const { return callExpression.get(); }
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
            void accept(ASTVisitor& v) override { v.visit(*this); }
            ExpressionNode* getExpression() const { return expression.get(); }
    };

    /**
     * @brief Represents a subroutine declaration (constructor, function, or method).
     */
    class SubroutineDecNode final : public Node {
        protected:
            SubroutineType subType; ///< The type of subroutine (constructor, function, method).
            std::shared_ptr<Type> returnType; ///< The return type (e.g., "void", "int", "MyClass").
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
            SubroutineDecNode(const SubroutineType st, std::shared_ptr<Type> ret, const std::string_view n,
                std::vector<Parameter> parameters, std::vector<std::unique_ptr<VarDecNode>> vars,
                std::vector<std::unique_ptr<StatementNode>> stmts,const int l, const int c)
                : Node(l,c),subType(st), returnType(std::move(ret)), name(n),parameters(std::move
                    (parameters)),localVars(std::move
                    (vars)),statements(std::move(stmts)) {};

            ~SubroutineDecNode() override = default;

            void accept(ASTVisitor& v) override { v.visit(*this); }

            SubroutineType getSubType() const { return subType; }
            Type* getReturnType() const { return returnType.get(); }
            std::string_view getName() const { return name; }
            const std::vector<Parameter>& getParams() const { return parameters; }
            const std::vector<std::unique_ptr<VarDecNode>>& getLocals() const { return localVars; }
            const std::vector<std::unique_ptr<StatementNode>>& getStatements() const { return statements;}
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
            void accept(ASTVisitor& visitor)override{visitor.visit(*this);}
            std::string_view getClassName()const { return className; }
            const std::vector<std::unique_ptr<ClassVarDecNode>>& getClassVars() const { return classVars; }
            const std::vector<std::unique_ptr<SubroutineDecNode>>& getSubroutines() const { return subroutineDecs; }
    };

}

#endif //NAND2TETRIS_AST_H