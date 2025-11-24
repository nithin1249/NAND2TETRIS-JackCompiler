//
// Created by Nithin Kondabathini on 23/11/2025.
//

#ifndef NAND2TETRIS_PARSENODE_H
#define NAND2TETRIS_PARSENODE_H

#include "NodeType.h"
#include <vector>
#include <memory>
#include "TokenTypes.h"

namespace nand2tetris::jack {
	class ParseNode {
		public:
			using Ptr  = std::unique_ptr<ParseNode>;
			using List = std::vector<Ptr>;
			static Ptr make(NodeType type); // Factory for non-terminal node
			static Ptr makeLeaf(const TokenTypes* tok); //Factory for terminal nodes

			//Deleted copy, allowed move
			ParseNode(const ParseNode&)=delete;
			ParseNode& operator=(const ParseNode&)=delete;

			ParseNode(ParseNode&&)=delete;
			ParseNode& operator=(ParseNode&&)=delete;

			//Accessors
			NodeType kind() const;
			bool isLeaf() const;
			const TokenTypes* token() const;
			const List& children() const;
			List& children();

			// Tree-building operations
			ParseNode& addChild(Ptr child);
			ParseNode& emplaceChild(NodeType kind);
			ParseNode& addLeafChild(const TokenTypes* tok);
		private:
			NodeType kind_;
			const TokenTypes* token_;   // nullptr for non-terminals
			List children_;

		    //Private Constructors
			explicit ParseNode(NodeType kind);
			explicit ParseNode(const TokenTypes* tok);
	};
}

#endif //NAND2TETRIS_PARSENODE_H