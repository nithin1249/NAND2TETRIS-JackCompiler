//
// Created by Nithin Kondabathini on 20/2/2026.
//

#include "TypeRegistry.h"

namespace nand2tetris::jack {

	size_t TypeHasher::operator()(const Type& t) const {
		// Hash the base string (e.g., "Array")
		size_t h = std::hash<std::string_view>{}(t.getBaseType());

		// Combine with hashes of generic arguments recursively
		for (const auto& arg : t.getGenericArgs()) {
			// Standard hash combining bit-math (Golden Ratio constant)
			h ^= operator()(*arg) + 0x9e3779b9 + (h << 6) + (h >> 2);
		}
		return h;
	}

	size_t TypeHasher::operator()(const std::unique_ptr<Type>& t) const {
		return operator()(*t);
	}

	bool TypeEq::operator()(const std::unique_ptr<Type>& a, const std::unique_ptr<Type>& b) const {
		return *a == *b; // Uses recursive operator== from AST.h
	}

	const Type* const TypeRegistry::getOrCreate(const Type& targetType) {
		// 1. Create a temporary for lookup
		auto temp = std::make_unique<Type>(targetType);

		// 2. Check if it's already there
		const auto it = pool.find(temp);
		if (it != pool.end()) {
			return it->get(); // Found existing: return the stable pointer
		}

		// 3. Not found: Move into the pool FIRST
		// We use the result of insert to get a stable iterator to the new element
		auto [insertedIt, success] = pool.insert(std::move(temp));

		// 4. Return the address of the now-permanent object
		return insertedIt->get();
	}

	const Type* const TypeRegistry::getPrimitive(std::string_view base) {
		return getOrCreate(Type(base));
	}

}