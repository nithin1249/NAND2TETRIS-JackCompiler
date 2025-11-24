#include <iostream>
#include "Compiler/Tokenizer.h"

using namespace nand2tetris::jack;

int main(int const argc, char** argv) {
	if (argc<2) {
		std::cerr << "Usage: jack_tokens <file.jack>\n";
		return 1;
	}
	const std::string filePath=std::string(argv[1]);

	Tokenizer tokenizer(filePath);

	while (tokenizer.hasMoreTokens()) {
		const Token& t = tokenizer.current();
        const auto tt = t.getType();

        std::cout << "Token: ";

        switch (tt) {
            case TokenType::KEYWORD: {
                auto* kt = dynamic_cast<const KeywordToken*>(&t);
                if (!kt) {
                    std::cout << "KEYWORD(<bad cast>)";
                } else {
                    std::cout << "KEYWORD(" << keywordToString(kt->getKeyword()) << ")";
                }
                break;
            }

            case TokenType::SYMBOL: {
                auto* txt = dynamic_cast<const TextToken*>(&t);
                if (!txt) {
                    std::cout << "SYMBOL(<bad cast>)";
                } else {
                    std::cout << "SYMBOL(" << txt->getText() << ")";
                }
                break;
            }

            case TokenType::IDENTIFIER: {
                auto* txt = dynamic_cast<const TextToken*>(&t);
                if (!txt) {
                    std::cout << "IDENTIFIER(<bad cast>)";
                } else {
                    std::cout << "IDENTIFIER(" << txt->getText() << ")";
                }
                break;
            }

            case TokenType::INT_CONST: {
                auto* it = dynamic_cast<const IntToken*>(&t);
                if (!it) {
                    std::cout << "INT(<bad cast>)";
                } else {
                    std::cout << "INT(" << it->getInt() << ")";
                }
                break;
            }

            case TokenType::STRING_CONST: {
                auto* txt = dynamic_cast<const TextToken*>(&t);
                if (!txt) {
                    std::cout << "STRING(<bad cast>)";
                } else {
                    std::cout << "STRING(\"" << txt->getText() << "\")";
                }
                break;
            }

            case TokenType::END_OF_FILE:
            default:
                // Do not print EOF here — loop will end after this iteration
                break;
        }

        std::cout << "\n";

        tokenizer.advance();  // move to next token
    }

}

