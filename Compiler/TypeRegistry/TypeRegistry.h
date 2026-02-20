//
// Created by Nithin Kondabathini on 20/2/2026.
//

#ifndef NAND2TETRIS_TYPEREGISTRY_H
#define NAND2TETRIS_TYPEREGISTRY_H
#include "../AST/AST.h"
#include <unordered_set>
#include <memory>
#include <string_view>

namespace nand2tetris::jack {
	/**
	 * @brief Custom Hasher for Type objects.
	 * Fingerprints complex recursive types for O(1) lookups.
	 */
	struct TypeHasher {
		size_t operator()(const std::unique_ptr<Type>& t) const;
		size_t operator()(const Type& t) const;
	};

	/**
	 * @brief Custom Equality for Type objects.
	 * Uses the recursive comparison logic from AST.h.
	 */
	struct TypeEq {
		bool operator()(const std::unique_ptr<Type>& a, const std::unique_ptr<Type>& b) const;
	};

	/**
	 * @brief The Type Pool Manager.
	 * Owns all unique Type instances and provides read-only observer pointers.
	 */
	class TypeRegistry {
	private:
		// The Hash Set: Ensures O(1) lookup and unique ownership.
		std::unordered_set<std::unique_ptr<Type>, TypeHasher, TypeEq> pool;

	public:
		TypeRegistry() = default;

		// Prevent copying to ensure single-source-of-truth for types.
		TypeRegistry(const TypeRegistry&) = delete;
		TypeRegistry& operator=(const TypeRegistry&) = delete;

		/**
		 * @brief Retrieves an existing type or creates a new master instance.
		 * @return A Triple-Locked observer pointer.
		 */
		[[nodiscard]] const Type* const getOrCreate(const Type& targetType);

		/**
		 * @brief Convenience helper for primitive types (int, char, etc).
		 */
		[[nodiscard]] const Type* const getPrimitive(std::string_view base);
	};

}

#endif //NAND2TETRIS_TYPEREGISTRY_H