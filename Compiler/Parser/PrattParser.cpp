//
// Created by Nithin Kondabathini on 15/2/2026.
//

#include "PrattParser.h"

namespace nand2tetris::jack {
	PrattParser::PrattParser(Tokenizer& tokenizer):tokenizer(tokenizer) {
		currentToken = &tokenizer.current();
		initializeRules();
	}

	std::unique_ptr<ClassNode> PrattParser::parse() {
		// A Jack file is exactly one class
		auto classNode = parseClass();

		// Ensure there is no "junk" at the end of the file
		if (!check(TokenType::END_OF_FILE)) {
			reportError("Unexpected tokens after class definition. A single file can contain only one class");
		}

		return classNode;
	}

	//Dispatch Table and Core Expression engine
	const PrattParser::ParseRule& PrattParser::getRule(const Token* token) const {
		if (token->getType() == TokenType::SYMBOL || token->getType() == TokenType::KEYWORD) {
			const auto it = textRules.find(token->getValue());
			if (it != textRules.end()) return it->second;
		}

		// 2. Otherwise, look up the generic Type (e.g., IDENTIFIER, INT_CONST)
		const auto it = typeRules.find(token->getType());
		if (it != typeRules.end()) return it->second;

		// 3. Fallback
		static ParseRule nullRule = { nullptr, nullptr, Precedence::LOWEST };
		return nullRule;
	}

	void PrattParser::initializeRules() {
        // 1. Generic Types (Using typeRules)
        typeRules[TokenType::INT_CONST]    = { &PrattParser::parseIntegerNud, nullptr, Precedence::LOWEST };
        typeRules[TokenType::FLOAT_CONST]  = { &PrattParser::parseFloatNud,   nullptr, Precedence::LOWEST };
        typeRules[TokenType::STRING_CONST] = { &PrattParser::parseStringNud,  nullptr, Precedence::LOWEST };
        typeRules[TokenType::IDENTIFIER]   = { &PrattParser::parseIdentifierNud, nullptr, Precedence::LOWEST };

        // 2. Specific Symbols (Using textRules)
        textRules["("] = { &PrattParser::parseGroupNud, nullptr, Precedence::LOWEST };
        textRules["~"] = { &PrattParser::parseUnaryNud, nullptr, Precedence::PREFIX };
        textRules["-"] = { &PrattParser::parseUnaryNud, &PrattParser::parseBinaryLed, Precedence::SUM };

        textRules["+"] = { nullptr, &PrattParser::parseBinaryLed, Precedence::SUM };
        textRules["*"] = { nullptr, &PrattParser::parseBinaryLed, Precedence::PRODUCT };
        textRules["/"] = { nullptr, &PrattParser::parseBinaryLed, Precedence::PRODUCT };
        textRules["&"] = { nullptr, &PrattParser::parseBinaryLed, Precedence::PRODUCT };
        textRules["|"] = { nullptr, &PrattParser::parseBinaryLed, Precedence::SUM };
        textRules["="] = { nullptr, &PrattParser::parseBinaryLed, Precedence::EQUALS };
        textRules["<"] = { nullptr, &PrattParser::parseBinaryLed, Precedence::LESS_GREATER };
        textRules[">"] = { nullptr, &PrattParser::parseBinaryLed, Precedence::LESS_GREATER };

        // 3. High Precedence Chaining
        textRules["."] = { nullptr, &PrattParser::parseCallLed, Precedence::CALL };
        textRules["["] = { nullptr, &PrattParser::parseIndexLed, Precedence::INDEX };

        // 4. Keywords (Example: 'this', 'true', 'false', 'null')
        textRules["this"]  = { &PrattParser::parseKeywordNud, nullptr, Precedence::LOWEST };
        textRules["true"]  = { &PrattParser::parseKeywordNud, nullptr, Precedence::LOWEST };
        textRules["false"] = { &PrattParser::parseKeywordNud, nullptr, Precedence::LOWEST };
        textRules["null"]  = { &PrattParser::parseKeywordNud, nullptr, Precedence::LOWEST };
    }

	std::unique_ptr<ExpressionNode> PrattParser::parseExpression(const Precedence precedence) {
		const NudFunc prefixRule = getRule(currentToken).nud;
		if (!prefixRule) {
			reportError("Unexpected token starting an expression");
			synchronize();
			return nullptr;
		}

		std::unique_ptr<ExpressionNode> left = (this->*prefixRule)();

		while (precedence < getRule(currentToken).precedence) {
			const LedFunc infixRule = getRule(currentToken).led;
			if (!infixRule) break;
			left = (this->*infixRule)(std::move(left));
		}

		return left;
	}

	void PrattParser::advance() {
		tokenizer.advance();
		currentToken = &tokenizer.current();
	}

	bool PrattParser::check(const TokenType type, const std::string_view value)const{
		if (currentToken->getType() != type) return false;
		if (!value.empty() && currentToken->getValue() != value) return false;
		return true;
	}

	bool PrattParser::match(const TokenType type, const std::string_view value){
		if (check(type, value)) {
			advance();
			return true;
		}

		return false;;
	}

	void PrattParser::expect(const TokenType type, const std::string_view value) {
		// 1. The Happy Path: The token matches. Consume it and move on.
		if (match(type, value)) return;

		// 2. The Error Path: The token is wrong. Build a helpful message.
		const std::string expected = value.empty() ? "Token Type " + std::string(typeToString(type)) : std::string(value);
		auto found = std::string(currentToken->getValue());
		if (found.empty()) found = "EOF or Unknown"; // Fallback for edge cases

		// Log it!
		reportError("Expected '" + expected + "' but found '" + found + "'");

		// 3. Trigger Panic Mode to recover the parser's state.
		synchronize();

	}

	void PrattParser::reportError(const std::string_view message) {
		errors.push_back({
		currentToken->getLine(),
		currentToken->getColumn(), // Make sure this matches your Token class getter
		std::string(message)
		});
	}

	void PrattParser::synchronize() {
		// Always advance past the token that caused the immediate error
		advance();

		// Keep eating tokens until we hit EOF
		while (currentToken->getType() != TokenType::END_OF_FILE) {

			// Safe Harbor 1: The end of a statement.
			if (check(TokenType::SYMBOL, ";")) {
				advance(); // Consume the ';' so the parser starts fresh on the next line
				return;
			}

			// Safe Harbor 2: The start of a major declaration or statement block.
			if (currentToken->getType() == TokenType::KEYWORD) {
				const std::string_view val = currentToken->getValue();
				if (val == "class" || val == "constructor" || val == "function" ||
					val == "method" || val == "var" || val == "let" ||
					val == "do" || val == "if" || val == "while" || val == "return") {
					return; // Stop eating tokens. The parser can safely resume here.
					}
			}

			// Token is garbage, skip it and loop again.
			advance();
		}
	}

	std::unique_ptr<Type> PrattParser::parseType(const bool allowVoid) {
		auto type = std::make_unique<Type>();
		const auto val = currentToken->getValue();

		const bool isPrimitive = (val == "int" || val == "char" || val == "boolean" || val =="float");
		const bool isVoid = (val == "void");
		const bool isClass = (currentToken->getType() == TokenType::IDENTIFIER);

		if (isPrimitive || isClass || (isVoid && allowVoid)) {
			type->baseType = val;
			advance();
		} else {
			if (isVoid && !allowVoid) {
				reportError("Variable cannot be of type 'void'.");
			} else {
				reportError("Expected a valid type.");
			}
			return nullptr;
		}

		if (match(TokenType::SYMBOL, "<")) {
			do {
				if (std::unique_ptr<Type> arg = parseType()) {
					type->genericArgs.push_back(std::move(arg));
				}
			} while (match(TokenType::SYMBOL, ","));

			expect(TokenType::SYMBOL, ">");
		}
		return type;
	}

	std::vector<Parameter> PrattParser::parseParameterList() {
		std::vector<Parameter> parameters;

		if (check(TokenType::SYMBOL, ")")) {
			return parameters;
		}

		do {
			Parameter param;

			// 1. Parse the Type (int, char, MyClass<T>, etc.)
			param.type = parseType();

			if (!param.type) {
				// Error reported in parseType.
				// We don't sync here; let the caller (parseSubroutineDec) handle it.
				return parameters;
			}

			// 2. Parse the Identifier (the name of the parameter)
			if (currentToken->getType() == TokenType::IDENTIFIER) {
				param.name = currentToken->getValue();
				advance();
			} else {
				reportError("Expected parameter name after type.");
				return parameters;
			}

			parameters.push_back(std::move(param));

			// 3. If there's a comma, keep going.
		} while (match(TokenType::SYMBOL, ","));

		return parameters;
	}

	std::unique_ptr<ClassNode> PrattParser::parseClass() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		expect(TokenType::KEYWORD, "class");
		auto className = currentToken->getValue();
		expect(TokenType::IDENTIFIER);
		expect(TokenType::SYMBOL, "{");

		std::vector<std::unique_ptr<ClassVarDecNode>> vars;
		std::vector<std::unique_ptr<SubroutineDecNode>> subs;
		bool hasConstructor = false; // Track if we find a constructor

		while (!check(TokenType::SYMBOL, "}") && !check(TokenType::END_OF_FILE)) {
			const std::string_view val = currentToken->getValue();

			if (val == "static" || val == "field") {
				if (!subs.empty()) {
					reportError("Class variables must be declared before subroutines.");
					synchronize();
					continue;
				}
				if (auto varDec = parseClassVarDec()) {
					vars.push_back(std::move(varDec));
				}
			}
			else if (val == "constructor" || val == "function" || val == "method") {
				if (val == "constructor") hasConstructor = true;

				if (auto subDec = parseSubroutineDec()) {
					subs.push_back(std::move(subDec));
				}
			}
			else {
				reportError("Only 'static', 'field', 'constructor', 'function', or 'method' allowed in class scope.");
				synchronize();
			}
		}

		// Strict Rule: Every class must have a constructor
		if (!hasConstructor) {
			reportError("Class '" + std::string(className) + "' must have at least one constructor.");
		}

		expect(TokenType::SYMBOL, "}");

		return std::make_unique<ClassNode>(className, std::move(vars), std::move(subs), line, col);
	}

	std::unique_ptr<ClassVarDecNode> PrattParser::parseClassVarDec() {
		int line =currentToken->getLine();
		int column = currentToken->getColumn();

		//determine if it is static or field
		ClassVarKind kind=(currentToken->getValue()=="static")? ClassVarKind::STATIC : ClassVarKind::FIELD;
		advance();

		//parse the type
		std::shared_ptr type = parseType();
		if (!type) {
			synchronize(); // This is a statement-level function, so we sync here.
			return nullptr;
		}

		//parse the list of variable names
		std::vector<std::string_view> varNames;
		do {
			if (currentToken->getType() == TokenType::IDENTIFIER) {
				varNames.push_back(currentToken->getValue());
				advance();
			} else {
				reportError("Expected variable name in class variable declaration.");
				synchronize();
				return nullptr;
			}
		} while (match(TokenType::SYMBOL, ","));

		expect(TokenType::SYMBOL, ";");

		return std::make_unique<ClassVarDecNode>(kind,type,std::move(varNames),line,column);
	}

	std::vector<std::unique_ptr<VarDecNode>> PrattParser::parseLocalVars() {
		std::vector<std::unique_ptr<VarDecNode>> declarations;

		while (match(TokenType::KEYWORD, "var")) {
			int line = currentToken->getLine();
			int column = currentToken->getColumn();

			if (std::shared_ptr type = parseType()) {
				std::vector<std::string_view> names;
				do {
					if (currentToken->getType() == TokenType::IDENTIFIER) {
						names.push_back(currentToken->getValue());
						advance();
					}else {
						reportError("Expected variable name after type in 'var' declaration.");
						break;
					}
				}while (match(TokenType::SYMBOL, ","));

				expect(TokenType::SYMBOL, ";");
				declarations.push_back(std::make_unique<VarDecNode>(type, std::move(names), line, column));
			}else {
				synchronize();
			}
		}

		return declarations;
	}

	std::unique_ptr<SubroutineDecNode> PrattParser::parseSubroutineDec() {
		int line = currentToken->getLine();
		int column = currentToken->getColumn();

		// Subroutine Type (constructor, function, method)
		SubroutineType subType;
		const std::string_view val = currentToken->getValue();
		if (val == "constructor") subType = SubroutineType::CONSTRUCTOR;
		else if (val == "function") subType = SubroutineType::FUNCTION;
		else subType = SubroutineType::METHOD;
		advance();

		// Return Type and Name
		const std::shared_ptr returnType = parseType(true);
		if (!returnType) { synchronize(); return nullptr; }

		std::string_view name = currentToken->getValue();
		expect(TokenType::IDENTIFIER);

		// Parameter List: (type name, type name, ...)
		expect(TokenType::SYMBOL, "(");
		auto params = parseParameterList();
		expect(TokenType::SYMBOL, ")");

		// Subroutine Body: { varDecs statements }
		expect(TokenType::SYMBOL, "{");
		auto locals = parseLocalVars();
		auto body = parseStatements();
		expect(TokenType::SYMBOL, "}");

		return std::make_unique<SubroutineDecNode>(
			subType, returnType, name, std::move(params),
			std::move(locals), std::move(body), line, column
		);
	}

	std::vector<std::unique_ptr<StatementNode>> PrattParser::parseStatements() {
		std::vector<std::unique_ptr<StatementNode>> statements;

		// Statements continue until we hit a '}' or EOF

		while (!check(TokenType::SYMBOL, "}") && !check(TokenType::END_OF_FILE)) {
			std::unique_ptr<StatementNode> stmt = nullptr;

			const std::string_view val = currentToken->getValue();

			if (val == "let")    stmt = parseLetStatement();
			else if (val == "if")     stmt = parseIfStatement();
			else if (val == "while")  stmt = parseWhileStatement();
			else if (val == "do")     stmt = parseDoStatement();
			else if (val == "return") stmt = parseReturnStatement();
			else {
				reportError("Expected a statement (let, if, while, do, return).");
				synchronize();
				continue;
			}

			if (stmt) {
				statements.push_back(std::move(stmt));
			}
		}

		if (check(TokenType::END_OF_FILE) && !check(TokenType::SYMBOL, "}")) {
			reportError("Missing '}' at end of subroutine.");
		}

		return statements;
	}

	std::unique_ptr<StatementNode> PrattParser::parseLetStatement() {
		int line = currentToken->getLine();
		int column = currentToken->getColumn();
		advance();

		//get the name
		std::string_view varName = currentToken->getValue();
		expect(TokenType::IDENTIFIER);

		//Handle optional array index: arr[expression]
		std::unique_ptr<ExpressionNode> indexExpr = nullptr;
		if (match(TokenType::SYMBOL, "[")) {
			indexExpr = parseExpression(Precedence::LOWEST);
			expect(TokenType::SYMBOL, "]");
		}

		//Parse the assignment: = expression;
		expect(TokenType::SYMBOL, "=");
		auto valueExpr = parseExpression(Precedence::LOWEST);

		if (!check(TokenType::SYMBOL, ";") && !check(TokenType::SYMBOL, ",") && !check(TokenType::SYMBOL, "]")) {
			reportError("Expected an operator or ';' but found '" + std::string(currentToken->getValue()) + "'");
			synchronize();
		}
		expect(TokenType::SYMBOL, ";");

		return std::make_unique<LetStatementNode>(
			varName, std::move(indexExpr), std::move(valueExpr), line, column);
	}

	std::unique_ptr<StatementNode> PrattParser::parseIfStatement() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();
		advance(); // consume 'if'

		// 1. Condition: ( expression )
		expect(TokenType::SYMBOL, "(");
		auto condition = parseExpression(Precedence::LOWEST);
		if (!condition) return nullptr;

		if (!check(TokenType::SYMBOL, ")")) {
			reportError("Expected operator or ')' but found '" + std::string(currentToken->getValue()) + "'");
			synchronize();
		}

		expect(TokenType::SYMBOL, ")");

		// 2. If Block: { statements }
		expect(TokenType::SYMBOL, "{");
		auto ifBranch = parseStatements();
		expect(TokenType::SYMBOL, "}");

		// 3. Optional Else Block
		std::vector<std::unique_ptr<StatementNode>> elseBranch;
		if (match(TokenType::KEYWORD, "else")) {
			expect(TokenType::SYMBOL, "{");
			elseBranch = parseStatements();
			expect(TokenType::SYMBOL, "}");
		}

		return std::make_unique<IfStatementNode>(
			std::move(condition), std::move(ifBranch), std::move(elseBranch), line, col
		);
	}

	std::unique_ptr<StatementNode> PrattParser::parseWhileStatement() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();
		advance(); // consume 'while'

		expect(TokenType::SYMBOL, "(");
		auto condition = parseExpression(Precedence::LOWEST);

		if (!condition) return nullptr;

		if (!check(TokenType::SYMBOL, ")")) {
			reportError("Expected operator or ')' but found '" + std::string(currentToken->getValue()) + "'");
			synchronize();
		}
		expect(TokenType::SYMBOL, ")");

		expect(TokenType::SYMBOL, "{");
		auto body = parseStatements();
		expect(TokenType::SYMBOL, "}");

		return std::make_unique<WhileStatementNode>(std::move(condition), std::move(body), line, col);
	}

	std::unique_ptr<StatementNode> PrattParser::parseDoStatement() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();
		advance(); // consume 'do'

		// Parse the expression as a CallNode
		auto expr = parseExpression(Precedence::LOWEST);

		if (!expr) return nullptr;

		if (!expr->isCall()) {
			reportError("The 'do' keyword must be followed by a subroutine call.");
			return nullptr;
		}

		if (!check(TokenType::SYMBOL, ";")) {
			reportError("Expected ';' after subroutine call but found '" + std::string(currentToken->getValue()) + "'");
			synchronize();
		}

		auto call = std::unique_ptr<CallNode>(static_cast<CallNode*>(expr.release()));  // NOLINT(*-pro-type-static-cast-downcast)


		expect(TokenType::SYMBOL, ";");
		return std::make_unique<DoStatementNode>(std::move(call), line, col);
	}

	std::unique_ptr<StatementNode> PrattParser::parseReturnStatement() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();
		advance(); // consume 'return'

		std::unique_ptr<ExpressionNode> expr = nullptr;
		if (!check(TokenType::SYMBOL, ";")) {
			expr = parseExpression(Precedence::LOWEST);

			if (!expr) {
				reportError("Expected expression after 'return'");
				// No need to sync here as parseExpression already did it
				return nullptr;
			}
		}

		expect(TokenType::SYMBOL, ";");
		return std::make_unique<ReturnStatementNode>(std::move(expr), line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseIntegerNud() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		// get the int
		auto* token= static_cast<const IntToken*>(currentToken); // NOLINT(*-pro-type-static-cast-downcast)
		int32_t value = token->getInt();
		advance();

		return std::make_unique<IntegerLiteralNode>(value, line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseFloatNud() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		// get the float
		auto* token= static_cast<const FloatToken*>(currentToken); // NOLINT(*-pro-type-static-cast-downcast)
		double value = token->getFloat();
		advance();

		return std::make_unique<FloatLiteralNode>(value, line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseStringNud() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		std::string_view value = currentToken->getValue();
		advance();

		return std::make_unique<StringLiteralNode>(value, line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseIdentifierNud() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		std::string_view name = currentToken->getValue();
		advance();

		std::vector<std::unique_ptr<Type>> generics;
		if (name == "Array" && check(TokenType::SYMBOL, "<")) {
			advance(); // Consume the '<'

			do {
				if (auto arg = parseType()) {
					generics.push_back(std::move(arg));
				}
			} while (match(TokenType::SYMBOL, ","));

			expect(TokenType::SYMBOL, ">");
		}

		if (match(TokenType::SYMBOL, "(")) {
			// We found a '(', so this identifier is actually a method/function name!
			auto args = parseExpressionList();
			expect(TokenType::SYMBOL, ")");

			// Return a CallNode. The receiver is nullptr because there is no "object." prefix.
			return std::make_unique<CallNode>(nullptr, name, std::move(args), line, col);
		}

		return std::make_unique<IdentifierNode>(name, std::move(generics), line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseUnaryNud() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		const char op = currentToken->getValue()[0];
		advance();

		// Parse the right side with PREFIX precedence
		auto operand = parseExpression(Precedence::PREFIX);

		return std::make_unique<UnaryOpNode>(op, std::move(operand), line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseGroupNud() {
		advance(); // Consume '('

		// Reset precedence to LOWEST inside parentheses
		auto expr = parseExpression(Precedence::LOWEST);

		if (!expr) return nullptr;

		expect(TokenType::SYMBOL, ")");
		return expr; // No need for a "GroupNode", just return the inner expression
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseKeywordNud() {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		auto* token= static_cast<const KeywordToken*>(currentToken); // NOLINT(*-pro-type-static-cast-downcast)
		Keyword value = token->getKeyword();

		advance();

		// You can use a dedicated KeywordLiteralNode or reuse a literal node
		return std::make_unique<KeywordLiteralNode>(value, line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseBinaryLed(std::unique_ptr<ExpressionNode> left) {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		// Capture the operator as a char for your slim AST nodes
		char op = currentToken->getValue()[0];

		// Get precedence of current operator to pass to the right-hand side
		const Precedence precedence = getRule(currentToken).precedence;
		advance();

		// Parse the right-hand expression
		// For Left-Associativity (Jack default): pass 'precedence' as is.
		// For Right-Associativity (like '=' or '^'): pass 'static_cast<Precedence>(static_cast<int>(precedence) - 1)'
		const bool isRightAssoc = (op == '=');
		const Precedence nextPrecedence = isRightAssoc ? static_cast<Precedence>(static_cast<int>(precedence) - 1) :
		precedence;
		auto right = parseExpression(nextPrecedence);
		if (!right) return nullptr;

		return std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right), line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseCallLed(std::unique_ptr<ExpressionNode> left) {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		advance(); // consume '.'

		std::string_view methodName = currentToken->getValue();
		expect(TokenType::IDENTIFIER);

		expect(TokenType::SYMBOL, "(");
		auto args = parseExpressionList();
		expect(TokenType::SYMBOL, ")");

		return std::make_unique<CallNode>(std::move(left), methodName, std::move(args), line, col);
	}

	std::unique_ptr<ExpressionNode> PrattParser::parseIndexLed(std::unique_ptr<ExpressionNode> left) {
		int line = currentToken->getLine();
		int col = currentToken->getColumn();

		advance(); // consume '['

		// Reset to LOWEST to allow any expression inside brackets
		auto index = parseExpression(Precedence::LOWEST);

		if (!index) return nullptr;

		if (!check(TokenType::SYMBOL, "]")) {
			reportError("Expected operator or ']' but found '" + std::string(currentToken->getValue()) + "'");
			synchronize();
		}

		expect(TokenType::SYMBOL, "]");

		return std::make_unique<ArrayAccessNode>(std::move(left), std::move(index), line, col);
	}

	std::vector<std::unique_ptr<ExpressionNode>> PrattParser::parseExpressionList() {
		std::vector<std::unique_ptr<ExpressionNode>> expressions;

		// Check for empty list: '()'
		if (check(TokenType::SYMBOL, ")")) {
			return expressions;
		}

		do {
			if (auto expr = parseExpression(Precedence::LOWEST)) {
				expressions.push_back(std::move(expr));
			} else {
				return expressions; // parseExpression already reported error
			}
			if (!check(TokenType::SYMBOL, ",") && !check(TokenType::SYMBOL, ")")) {
				reportError("Expected ',' or ')' but found '" + std::string(currentToken->getValue()) + "'");
				synchronize();
				break;
			}
		} while (match(TokenType::SYMBOL, ","));

		return expressions;
	}

}
