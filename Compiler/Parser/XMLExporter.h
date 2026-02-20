//
// Created by Nithin Kondabathini on 18/2/2026.
//

#ifndef NAND2TETRIS_XMLEXPORTER_H
#define NAND2TETRIS_XMLEXPORTER_H

#include "XMLVisitor.h"


namespace nand2tetris::jack {

	/**
	 * @brief Public API for serializing a Jack AST into XML format.
	 */
	class XMLExporter {
	public:
		// Delete constructors to make this a purely static utility class
		XMLExporter() = delete;

		/**
		 * @brief Generates XML from an AST and writes it to the provided output stream.
		 */
		static void generate(const Node* astRoot, std::ostream& out);

		/**
		 * @brief Helper function to generate XML directly to a file.
		 */
		static bool generateToFile(const Node* astRoot, const std::string& filename);
	};
}


#endif //NAND2TETRIS_XMLEXPORTER_H