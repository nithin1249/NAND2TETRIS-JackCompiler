//
// Created by Nithin Kondabathini on 18/2/2026.
//

#include "XMLExporter.h"

#include <fstream>

namespace nand2tetris::jack {

	void XMLExporter::generate(const Node* astRoot, std::ostream& out) {
		if (!astRoot) return;

		// Instantiate the hidden visitor and kick off the traversal
		XMLVisitor visitor(out);
		astRoot->accept(visitor);
	}

	bool XMLExporter::generateToFile(const Node* astRoot, const std::string& filename) {
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "[-] Error: Could not open file " << filename << " for XML export.\n";
			return false;
		}

		generate(astRoot, outFile);
		return true;
	}
}
