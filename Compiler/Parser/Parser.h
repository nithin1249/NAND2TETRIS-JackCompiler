//
// Created by Nithin Kondabathini on 13/1/2026.
//

#ifndef NAND2TETRIS_PARSER_H
#define NAND2TETRIS_PARSER_H

#include "../Tokenizer/Tokenizer.h"
#include "../Tokenizer/TokenTypes.h"
#include "AST.h"

namespace nand2tetris::jack {

    /**
     * @brief A recursive descent parser for the Jack language.
     *
     * The Parser takes a stream of tokens from the Tokenizer and constructs an
     * Abstract Syntax Tree (AST) representing the structure of the Jack program.
     * It follows the grammar rules specified for the Jack language.
     */
    class Parser {
        Tokenizer& tokenizer;           ///< Reference to the tokenizer providing the token stream.
        const Token* currentToken = nullptr; ///< Pointer to the current token being processed.

        // --- Helper Methods ---

        /**
         * @brief Advances to the next token in the stream.
         *
         * Updates `currentToken` to point to the new token.
         */
        void advance();

        /**
         * @brief Checks if the current token matches a specific type.
         *
         * @param type The expected token type.
         * @return true if the current token's type matches, false otherwise.
         */
        bool check(TokenType type) const;

        /**
         * @brief Checks if the current token matches a specific text value (e.g., a specific symbol or keyword).
         *
         * @param text The expected text content.
         * @return true if the current token's text matches, false otherwise.
         */
        bool check(std::string_view text) const;

        /**
         * @brief Consumes the current token if it matches the expected type, otherwise throws an error.
         *
         * @param type The expected token type.
         * @param errorMessage The message to display if the check fails.
         * @throws std::runtime_error if the token does not match.
         */
        void consume(TokenType type, std::string_view errorMessage);

        /**
         * @brief Consumes the current token if it matches the expected text, otherwise throws an error.
         *
         * @param text The expected text content.
         * @param errorMessage The message to display if the check fails.
         * @throws std::runtime_error if the token does not match.
         */
        void consume(std::string_view text, std::string_view errorMessage);

        // --- Grammar Rules (Recursive Descent) ---

        /**
         * @brief Compiles a complete class declaration.
         *
         * Grammar: 'class' className '{' classVarDec* subroutineDec* '}'
         *
         * @return A unique_ptr to the resulting ClassNode.
         */
        std::unique_ptr<ClassNode> compileClass();

        /**
         * @brief Compiles a static or field variable declaration.
         *
         * Grammar: ('static' | 'field') type varName (',' varName)* ';'
         *
         * @return A unique_ptr to the resulting ClassVarDecNode.
         */
        std::unique_ptr<ClassVarDecNode> compileClassVarDec();

        /**
         * @brief Compiles a subroutine (constructor, function, or method).
         *
         * Grammar: ('constructor' | 'function' | 'method') ('void' | type) subroutineName '(' parameterList ')' subroutineBody
         *
         * @return A unique_ptr to the resulting SubroutineDecNode.
         */
        std::unique_ptr<SubroutineDecNode> compileSubroutine();

        /**
         * @brief Compiles a local variable declaration.
         *
         * Grammar: 'var' type varName (',' varName)* ';'
         *
         * @return A unique_ptr to the resulting VarDecNode.
         */
        std::unique_ptr<VarDecNode> compileVarDec();

        /**
         * @brief Compiles a sequence of statements.
         *
         * Grammar: statement*
         *
         * @return A vector of unique_ptrs to StatementNodes.
         */
        std::vector<std::unique_ptr<StatementNode>> compileStatements();

        /**
         * @brief Compiles a single statement.
         *
         * Dispatches to specific statement compilers (let, if, while, do, return) based on the keyword.
         *
         * @return A unique_ptr to the resulting StatementNode.
         */
        std::unique_ptr<StatementNode> compileStatement();

        /**
         * @brief Compiles a 'let' statement.
         *
         * Grammar: 'let' varName ('[' expression ']')? '=' expression ';'
         *
         * @return A unique_ptr to the resulting LetStatementNode.
         */
        std::unique_ptr<LetStatementNode> compileLetStatement();

        /**
         * @brief Compiles an 'if' statement.
         *
         * Grammar: 'if' '(' expression ')' '{' statements '}' ('else' '{' statements '}')?
         *
         * @return A unique_ptr to the resulting IfStatementNode.
         */
        std::unique_ptr<IfStatementNode> compileIfStatement();

        /**
         * @brief Compiles a 'while' statement.
         *
         * Grammar: 'while' '(' expression ')' '{' statements '}'
         *
         * @return A unique_ptr to the resulting WhileStatementNode.
         */
        std::unique_ptr<WhileStatementNode> compileWhileStatement();

        /**
         * @brief Compiles a 'do' statement.
         *
         * Grammar: 'do' subroutineCall ';'
         *
         * @return A unique_ptr to the resulting DoStatementNode.
         */
        std::unique_ptr<DoStatementNode> compileDoStatement();

        /**
         * @brief Compiles a 'return' statement.
         *
         * Grammar: 'return' expression? ';'
         *
         * @return A unique_ptr to the resulting ReturnStatementNode.
         */
        std::unique_ptr<ReturnStatementNode> compileReturnStatement();

        /**
         * @brief Compiles an expression.
         *
         * Grammar: term (op term)*
         *
         * @return A unique_ptr to the resulting ExpressionNode.
         */
        std::unique_ptr<ExpressionNode> compileExpression();

        /**
         * @brief Compiles a term within an expression.
         *
         * Grammar: integerConstant | stringConstant | keywordConstant | varName | varName '[' expression ']' | subroutineCall | '(' expression ')' | unaryOp term
         *
         * @return A unique_ptr to the resulting ExpressionNode (which is a specific type of term).
         */
        std::unique_ptr<ExpressionNode> compileTerm();

        /**
         * @brief Compiles a comma-separated list of expressions.
         *
         * Used in subroutine calls.
         * Grammar: (expression (',' expression)*)?
         *
         * @return A vector of expression nodes.
         */
        std::vector<std::unique_ptr<ExpressionNode>> compileExpressionList();

        /**
         * @brief Compiles a subroutine call.
         *
         * Grammar: subroutineName '(' expressionList ')' | (className | varName) '.' subroutineName '(' expressionList ')'
         *
         * @return A unique_ptr to the resulting CallNode.
         */
        std::unique_ptr<CallNode> compileSubroutineCall();

        /**
         * @brief Checks if the current token is a binary operator.
         *
         * Operators: +, -, *, /, &, |, <, >, =
         *
         * @return true if the current token is a binary operator, false otherwise.
         */
        bool isBinaryOp() const;

        public:
            /**
             * @brief Constructs a Parser with a given Tokenizer.
             *
             * @param tokenizer The tokenizer instance to use.
             */
            explicit Parser(Tokenizer& tokenizer);

            /**
             * @brief Parses the entire token stream into an Abstract Syntax Tree.
             *
             * Starts parsing from the 'class' rule.
             *
             * @return A unique_ptr to the root ClassNode of the AST.
             */
            std::unique_ptr<ClassNode> parse();
    };
};

#endif //NAND2TETRIS_PARSER_H