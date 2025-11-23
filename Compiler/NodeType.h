//
// Created by Nithin Kondabathini on 23/11/2025.
//

#ifndef NAND2TETRIS_NODETYPE_H
#define NAND2TETRIS_NODETYPE_H

enum class NodeType {
	// Non-terminals (Jack grammar)
	Class,
	ClassVarDec,
	SubroutineDec,
	ParameterList,
	VarDec,
	Statements,
	LetStatement,
	IfStatement,
	WhileStatement,
	DoStatement,
	ReturnStatement,
	Expression,
	Term,
	ExpressionList,

	// Terminals (leaf categories)
	Identifier,
	IntegerConstant,
	StringConstant,
	Keyword,
	Symbol,
};

#endif //NAND2TETRIS_NODETYPE_H