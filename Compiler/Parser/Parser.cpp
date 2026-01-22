//
// Created by Nithin Kondabathini on 13/1/2026.
//

#include "Parser.h"
#include <numeric>

namespace nand2tetris::jack {
    Parser::Parser(Tokenizer &tokenizer):tokenizer(tokenizer) {
        // Initialize the parser by pointing to the first token available in the tokenizer.
        // The tokenizer is assumed to be already initialized and pointing to the first token.
        currentToken=&tokenizer.current();
    }

    std::unique_ptr<ClassNode> Parser::parse() {
        // The entry point for parsing a Jack file. Every Jack file must contain exactly one class.
        return compileClass();
    }


    void Parser::advance() {
        // Move the tokenizer forward to the next token and update our local reference.
        tokenizer.advance();
        currentToken = &tokenizer.current();
    }

    bool Parser::check(const TokenType type) const {
        // Check if the current token matches the expected type without consuming it.
        return currentToken->getType()==type;
    }

    bool Parser::check(const std::string_view text) const {
        // Check if the current token's text value matches the expected string without consuming it.
        return currentToken->getValue()==text;
    }

    void Parser::consume(const TokenType type, const std::string_view errorMessage) {
        // If the current token matches the expected type, consume it (advance).
        // Otherwise, report a syntax error at the current token's location.
        if (check(type)) {
            advance();
        } else {
            tokenizer.errorAt(currentToken->getLine(),currentToken->getColumn(), errorMessage);
        }
    }

    void Parser::consume(const std::string_view text, const std::string_view errorMessage) {
        // If the current token matches the expected text, consume it (advance).
        // Otherwise, report a syntax error.
        if (check(text)) {
            advance();
        } else {
            tokenizer.errorAt(currentToken->getLine(),currentToken->getColumn(), errorMessage);
        }
    }

    std::unique_ptr<ClassNode> Parser::compileClass() {
        // Grammar: 'class' className '{' classVarDec* subroutineDec* '}'

        // 1. Expect 'class' keyword
        consume("class", "Expected 'class' keyword");

        // 2. Expect class name (identifier)
        std::string_view className = currentToken->getValue();
        consume(TokenType::IDENTIFIER, "Expected class name");

        // 3. Expect opening brace '{'
        consume("{", "Expected '{'");

        std::vector<std::unique_ptr<ClassVarDecNode>> classVars;
        std::vector<std::unique_ptr<SubroutineDecNode>> subroutineDecs;

        // 4. Parse class body: variable declarations followed by subroutine declarations.
        // We loop until we hit the closing brace '}'.
        while (!check("}")) {
            const std::string_view val=currentToken->getValue();

            // Distinguish between class variables (static/field) and subroutines (constructor/method/function).
            if (val=="static"||val=="field") {
                classVars.push_back(compileClassVarDec());
            }else if (val=="constructor"||val=="method"||val=="function") {
                subroutineDecs.push_back(compileSubroutine());
            }else {
                // If we encounter anything else, it's a syntax error.
                tokenizer.errorAt(currentToken->getLine(),currentToken->getColumn(), "Expected class variable or subroutine declaration");
            }
        }

        // 5. Expect closing brace '}'
        consume("}", "Expected '}' to close class body");

        return std::make_unique<ClassNode>(className, std::move(classVars), std::move(subroutineDecs));
    }

    std::unique_ptr<ClassVarDecNode> Parser::compileClassVarDec() {
        // Grammar: ('static' | 'field') type varName (',' varName)* ';'

        // 1. Determine if it's a static or field variable.
        // We assume the caller has already verified the token is "static" or "field".
        ClassVarKind kind;
        if (check("static")) {
            kind=ClassVarKind::STATIC;
        }else {
            kind=ClassVarKind::FIELD;
        }

        advance(); // Consume 'static' or 'field'

        // 2. Parse the type (int, char, boolean, or a class name).
        std::string_view type = currentToken->getValue();
        if (check("int")||check("boolean")||check("char")||check(TokenType::IDENTIFIER)) {
            advance();
        }else {
            tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), "Expected variable type (int, char, boolean, or class name)");
        }

        // 3. Parse the list of variable names.
        std::vector<std::string_view> names;

        // The first variable name is mandatory.
        names.push_back(currentToken->getValue());
        consume(TokenType::IDENTIFIER, "Expected variable name");

        // Handle multiple variables declared in the same line (e.g., static int x, y, z;)
        while (true) {
            if (check(",")) {
                advance(); // Consume the comma
            } else if (check(TokenType::IDENTIFIER)) {
                // Predictive error handling: if we see an identifier but no comma, it's a likely syntax error.
                tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), "Missing ',' between variable identifiers");
            } else if (check(";")) {
                // Valid end of the list
                break;
            }else {
                tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(),"Expected ',' or ';' after variable name");
            }

            // Consume the next variable name
            names.push_back(currentToken->getValue());
            consume(TokenType::IDENTIFIER, "Expected variable name");
        }

        // 4. Expect the closing semicolon.
        consume(";", "Expected ';' at the end of variable declaration");

        return std::make_unique<ClassVarDecNode>(kind, type, std::move(names));
    }

    std::unique_ptr<SubroutineDecNode> Parser::compileSubroutine() {
        // Grammar: ('constructor' | 'function' | 'method') ('void' | type) subroutineName '(' parameterList ')'
        // subroutineBody: '{' varDec* statements '}'

        // 1. Determine the subroutine type (constructor, function, or method).
        SubroutineType type;
        if (currentToken->getValue()=="constructor") {
            type=SubroutineType::CONSTRUCTOR;
        }else if (currentToken->getValue()=="function") {
            type=SubroutineType::FUNCTION;
        }else {
            type=SubroutineType::METHOD;
        }
        advance(); // Consume the subroutine type keyword

        // 2. Parse the return type.
        // Can be 'void', a primitive type (int, boolean, char), or a class name (identifier).
        std::string_view returnType;
        if (currentToken->getValue()=="void") {
            returnType="void";
            advance();
        }else if (check("int")||check("boolean")||check("char")||check(TokenType::IDENTIFIER)) {
            returnType=currentToken->getValue();
            advance();
        }else {
            tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), "Expected return type void, int, char, boolean, or class name");
        }

        // 3. Parse the subroutine name.
        std::string_view subroutineName=currentToken->getValue();
        consume(TokenType::IDENTIFIER, "Expected subroutine name");

        // 4. Parse the parameter list enclosed in parentheses.
        std::vector<Parameter> parameters;
        consume("(","Expected '(' to open parameter list");

        // Check if parameter list is empty by looking for the closing parenthesis.
        if (!check(")")) {
            // Loop to parse parameters separated by commas.
            while (true) {
                // Parse parameter type
                const std::string_view pType = currentToken->getValue();
                if (check("int")||check("boolean")||check("char")||check(TokenType::IDENTIFIER)) {
                    advance();
                }else {
                    tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), "Expected parameter type (int, char, boolean, or class name)");
                }

                // Parse parameter name
                const std::string_view pName = currentToken->getValue();
                consume(TokenType::IDENTIFIER, "Expected parameter name");

                parameters.push_back({pType, pName});

                // Check for comma to continue or closing parenthesis to break
                if (check(",")) {
                    advance(); // Found a comma, expect another parameter
                } else if (check(")")) {
                    break; // Found closing parenthesis, end of list
                }else {
                    // PREDICTIVE ERROR: If we see a type-like keyword or an identifier,
                    // they definitely just forgot the comma.
                    if (check("int") || check("boolean") || check("char") || check(TokenType::IDENTIFIER)) {
                        tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), "Missing ',' between parameters");
                    }
                    // If it's not a type, they probably forgot to close the list.
                    tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), "Expected ')' to close parameter list");
                }
            }
        }


        consume(")", "Expected ')' to close parameter list");

        // 5. Parse the subroutine body.
        consume("{","Expected '{' to open subroutine body");

        std::vector<std::unique_ptr<VarDecNode>> localVars;

        // Parse local variable declarations (must come before statements).
        while (check("var")) {
            localVars.push_back(compileVarDec());
        }

        // Parse statements until the closing brace.
        std::vector<std::unique_ptr<StatementNode> > statements = compileStatements();

        consume("}","Expected '}' to close subroutine body");

        return std::make_unique<SubroutineDecNode>(type,returnType,subroutineName,parameters,std::move(localVars),std::move(statements));
    }

    std::unique_ptr<VarDecNode> Parser::compileVarDec() {
        // Grammar: 'var' type varName (',' varName)* ';'

        // 1. Consume 'var' keyword.
        consume("var", "Expected 'var' keyword");

        // 2. Parse the type (int, char, boolean, or class name).
        std::string_view type = currentToken->getValue();
        if (check("int")||check("boolean")||check("char")||check(TokenType::IDENTIFIER)) {
            advance();
        }else {
            tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), "Expected variable type (int, char, boolean, or class name)");
        }

        // 3. Parse the list of variable names.
        std::vector<std::string_view> names;

        // First variable name is mandatory.
        names.push_back(currentToken->getValue());
        consume(TokenType::IDENTIFIER, "Expected variable name");

        // Handle multiple variables declared in the same line (e.g., var int x, y, z;)
        while (true) {
            if (check(",")) {
                advance(); // Consume the comma
            } else if (check(TokenType::IDENTIFIER)) {
                // Predictive error handling: if we see an identifier but no comma, it's a likely syntax error.
                tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), "Missing ',' between variable identifiers");
            } else {
                break; // No comma and no identifier? The list is finished.
            }

            // Consume the next variable name
            names.push_back(currentToken->getValue());
            consume(TokenType::IDENTIFIER, "Expected variable name");
        }

        // 4. Expect the closing semicolon.
        consume(";", "Expected ';' at the end of variable declaration");

        return std::make_unique<VarDecNode>(type,std::move(names));
    }

    std::vector<std::unique_ptr<StatementNode>> Parser::compileStatements() {
        std::vector<std::unique_ptr<StatementNode>> list;
        while (!check("}")) {
            list.push_back(compileStatement());
        }
        return list;
    }

    std::unique_ptr<StatementNode> Parser::compileStatement() {
        if (check("let")) {
            return compileLetStatement();
        }
        if (check("if")) {
            return compileIfStatement();
        }
        if (check("while")) {
            return compileWhileStatement();
        }
        if (check("do")) {
            return compileDoStatement();
        }
        if (check("return")) {
            return compileReturnStatement();
        }

        // The 'Junk' Handler:
        tokenizer.errorAt(currentToken->getLine(),currentToken->getColumn(), "Unknown statement or unexpected text");
    }

    std::unique_ptr<LetStatementNode> Parser::compileLetStatement() {
        // Grammar: 'let' varName ('[' expression ']')? '=' expression ';'

        //move past let:
        consume("let","Expected a 'let' keyword");

        //get the variable name
        std::string_view varName=currentToken->getValue();
        consume(TokenType::IDENTIFIER,"Expected variable name");

        std::unique_ptr<ExpressionNode> indexExpr=nullptr; ///< The index expression for array assignment (optional).
        if (check("[")) {
            advance();
            indexExpr=compileExpression();
            consume("]","Expected ']' to close array index");
        }
        // Checkpoint 2: If we didn't see '[', we MUST see '='.
        // If we see an identifier here, it's a specific error.
        else if (check(TokenType::IDENTIFIER)) {
            tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(),
                              "Unexpected identifier; perhaps you forgot a '[' for an array?");
        }
        // If we see anything else that isn't '=', it's a general assignment error.
        else if (!check("=")) {
            tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(),
                              "Expected '=' after variable name");
        }

        consume("=","Expected an `=`");

        std::unique_ptr<ExpressionNode> exp=compileExpression();

        consume(";", "Expected ';' at end of let statement");

        return std::make_unique<LetStatementNode>(varName, std::move(indexExpr), std::move(exp));
    }

    std::unique_ptr<IfStatementNode> Parser::compileIfStatement() {
        // Grammar: 'if' '(' expression ')' '{' statements '}' ('else' '{' statements '}')?

        consume("if", "Expected 'if' keyword");

        // 1. Condition Header '('
        consume("(", "Expected '(' after 'if'");
        std::unique_ptr<ExpressionNode> condition=compileExpression();
        // 2. Condition Closer ')'
        // If it's missing, we check if they accidentally started the block '{' early.
        if (check("{")) {
            tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(),
                              "Missing ')' before opening brace '{'");
        }

        consume(")", "Expected ')' after if-condition");


        // 3. If-Body '{ statements }'
        consume("{", "Expected '{' to start if-block");
        std::vector<std::unique_ptr<StatementNode>> ifStatements=compileStatements();
        consume("}", "Expected '}' to close if-block");

        std::vector<std::unique_ptr<StatementNode>> elseStatements;
        if (check("else")) {
            advance(); // consume 'else'
            consume("{", "Expected '{' to start else-block");
            elseStatements = compileStatements();
            consume("}", "Expected '}' to close else-block");
        }

        return std::make_unique<IfStatementNode>(std::move(condition), std::move(ifStatements), std::move(elseStatements));
    }


    std::unique_ptr<WhileStatementNode> Parser::compileWhileStatement() {
        // Grammar: 'while' '(' expression ')' '{' statements '}'

        consume("while", "Expected 'while' keyword");

        // 1. Condition
        consume("(", "Expected '(' after 'while'");
        std::unique_ptr<ExpressionNode> condition = compileExpression();
        if (check("{")) {
            tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(),"Missing ')' before opening brace '{'");
        }
        consume(")", "Expected ')' after while-condition");

        // 2. Body
        consume("{", "Expected '{' to start while-loop body");
        std::vector<std::unique_ptr<StatementNode>> body = compileStatements();
        consume("}", "Expected '}' to close while-loop body");

        return std::make_unique<WhileStatementNode>(std::move(condition), std::move(body));
    }

    std::unique_ptr<ReturnStatementNode> Parser::compileReturnStatement() {
        // Grammar: 'return' expression? ';'

        consume("return", "Expected 'return' keyword");

        std::unique_ptr<ExpressionNode> value = nullptr;

        // 1. Check if there is an expression to return.
        // In Jack, if the next token is not ';', it MUST be an expression.
        if (!check(";")) {
            // Handle "junk" or missing semicolon cases:
            // If we see a '}' or another statement keyword, they likely forgot the ';'
            if (check("}") || check("let") || check("if") || check("while") || check("do")) {
                tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(),
                                  "Missing ';' after return keyword");
            }
            // It's not a ';' and not a new block/statement, so it must be an expression.
            value = compileExpression();
        }

        // 2. Final check for the semicolon
        consume(";", "Expected ';' after return statement");

        return std::make_unique<ReturnStatementNode>(std::move(value));
    }


    std::unique_ptr<DoStatementNode> Parser::compileDoStatement() {
        //Grammar: `do' subroutineName '('expressionList')'|(className|varName)`.` subroutineName
        consume("do","Expected 'do' keyword");
        std::unique_ptr<CallNode> call = compileSubroutineCall();
        consume(";", "Expected ';' after do subroutine call");
        return std::make_unique<DoStatementNode>(std::move(call));

    }

    std::unique_ptr<ExpressionNode> Parser::compileExpression() {
        // Grammar: term (op term)*
        // op: + - * / & | < > =

        // 1. Compile the first term
        std::unique_ptr<ExpressionNode> left_term=compileTerm();

        // 2. Look for binary operators: +, -, *, /, &, |, <, >, =
        while (isBinaryOp()) {
            char op = currentToken->getValue()[0];
            advance(); // consume the operator
            std::unique_ptr<ExpressionNode> right_term = compileTerm();

            // Wrap the existing 'left' and the new 'right' into a new BinaryOpNode
            // This handles left-associativity (e.g., 1 + 2 + 3)
            left_term = std::make_unique<BinaryOpNode>(std::move(left_term), op, std::move(right_term));
        }

        return left_term;
    }

    bool Parser::isBinaryOp() const {
        // Helper to check if the current token is one of the Jack binary operators.
        if (currentToken->getType()!=TokenType::SYMBOL) {
            return false;
        }

        const std::string_view val = currentToken->getValue();

        return val == "+" || val == "-" || val == "*" || val == "/" ||
           val == "&" || val == "|" || val == "<" || val == ">" || val == "=";
    }


    std::unique_ptr<ExpressionNode> Parser::compileTerm() {
        // Grammar: integerConstant | stringConstant | keywordConstant | varName |
        //          varName '[' expression ']' | subroutineCall | '(' expression ')' | unaryOp term

        // 1. Integer Constant
        if (check(TokenType::INT_CONST)) {
            const std::string_view valView = currentToken->getValue();
            int val = std::accumulate(valView.begin(), valView.end(), 0, [](int res, char c) { return res * 10 + (c - '0'); });
            advance();
            return std::make_unique<IntegerLiteralNode>(val);
        }

        // 2. String Constant
        if (check(TokenType::STRING_CONST)) {
            std::string_view val = currentToken->getValue();
            advance();
            return std::make_unique<StringLiteralNode>(val);
        }

        // 3. Keyword Constant (true, false, null, this)
        if (currentToken->getType()==TokenType::KEYWORD) {
            const std::string_view val=currentToken->getValue();

            if (val == "true") {
                advance();
                return std::make_unique<KeywordLiteralNode>(Keyword::TRUE_);
            } else if (val == "false") {
                advance();
                return std::make_unique<KeywordLiteralNode>(Keyword::FALSE_);
            } else if (val == "null") {
                advance();
                return std::make_unique<KeywordLiteralNode>(Keyword::NULL_);
            } else if (val == "this") {
                advance();
                return std::make_unique<KeywordLiteralNode>(Keyword::THIS_);
            }else {
                tokenizer.errorAt(currentToken->getLine(),currentToken->getColumn(),"Inappropriate keyword used in expression.");
            }
        }


        // 4. Identifier (Variable, Array Access, or Subroutine Call)
        if (check(TokenType::IDENTIFIER)) {
            std::string_view name = currentToken->getValue();

            // Use PEEK to distinguish between x, x[i], and x.method()
            const Token& next = tokenizer.peek();
            if (next.getValue()=="[") {
                // Array Access: varName '[' expression ']'
                advance(); //consume name
                advance(); // consume '['
                std::unique_ptr<ExpressionNode> exp=compileExpression();
                consume("]", "Expected ']' after array index");
                return std::make_unique<IdentifierNode>(name, std::move(exp));
            }else if (next.getValue()=="("||next.getValue()==".") {
                // Subroutine Call
                return compileSubroutineCall();
            }else {
                // Simple Variable
                advance();
                return std::make_unique<IdentifierNode>(name);
            }
        }

        // 5. Parenthesized Expression: '(' expression ')'
        if (check("(")) {
            advance();
            std::unique_ptr<ExpressionNode> expr = compileExpression();
            consume(")", "Expected ')' to close expression");
            return expr;
        }

        // 6. Unary Operators: '-' or '~'
        if (check("-") || check("~")) {
            char op = currentToken->getValue()[0];
            advance();
            std::unique_ptr<ExpressionNode> term = compileTerm();
            return std::make_unique<UnaryOpNode>(op, std::move(term));
        }

        const std::string err = "Expected an expression term, but found '" + std::string(currentToken->getValue()) + "'";
        tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(), err);
    }


    std::vector<std::unique_ptr<ExpressionNode>> Parser::compileExpressionList() {
        std::vector<std::unique_ptr<ExpressionNode>> list;

        // 1. Handle the empty list case: do method()
        if (check(")")) {
            return list;
        }

        // 2. Parse the first mandatory expression (must exist if not followed immediately by ')'
        list.push_back(compileExpression());

        // 3. Loop to handle subsequent comma-separated expressions
        while (true) {
            if (check(",")) {
                advance(); // Consume the comma
                list.push_back(compileExpression());
            }
            else if (check(")")) {
                // Success: reached the end of the argument list
                break;
            }
            else {
                // FOCUSED ERROR: If we find a new term/junk without a comma or closing ')'.
                // Because errorAt throws an exception, this safely terminates the parse.
                tokenizer.errorAt(currentToken->getLine(), currentToken->getColumn(),
                                  "Expected ',' between arguments");
            }
        }

        return list;
    }

    std::unique_ptr<CallNode> Parser::compileSubroutineCall() {
        // Save the first identifier to determine context later
        const std::string_view firstPart = currentToken->getValue();
        consume(TokenType::IDENTIFIER, "Expected subroutine, class, or variable name");

        std::string_view classNameOrVar;
        std::string_view subroutineName;

        // 1. Check for the dot '.' symbol (indicates a call on an object or a static class method)
        if (check(".")) {
            advance(); // Move past '.'
            classNameOrVar = firstPart; // The first part was the class/variable name
            subroutineName = currentToken->getValue();
            consume(TokenType::IDENTIFIER, "Expected subroutine name after '.'");
        } else {
            // 2. Direct call (e.g., draw()): The first part was the actual subroutine name
            subroutineName = firstPart;
        }

        // 3. Parse the argument list enclosed in parentheses
        consume("(", "Expected '(' for argument list");

        // Call our helper to parse zero or more expressions
        std::vector<std::unique_ptr<ExpressionNode>> agrs = compileExpressionList();

        // Ensure the argument list is properly closed
        consume(")", "Expected ')' to close argument list");

        // Return the AST node with all captured information
        return std::make_unique<CallNode>(classNameOrVar, subroutineName, std::move(agrs));
    }
    
}
