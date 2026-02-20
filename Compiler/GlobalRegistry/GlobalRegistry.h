//
// Created by Nithin Kondabathini on 23/1/2026.
//

#ifndef NAND2TETRIS_GLOBAL_REGISTRY_H
#define NAND2TETRIS_GLOBAL_REGISTRY_H
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <fstream>
#include "../AST/AST.h"

namespace nand2tetris::jack {

    /**
     * @brief Represents the signature of a Jack subroutine (method, function, or constructor).
     */
    struct MethodSignature {
        private:
            const Type* const returnType;        ///< The return type of the subroutine (e.g., "int", "void").
            std::vector<const Type*> parameters; ///< List of parameter types.
            SubroutineType subKind;
            int line;                           ///< Line number of the declaration.
            int column;                         ///< Column number of the declaration.
        public:
            MethodSignature(const Type* const ret, std::vector<const Type*> params, const SubroutineType kind, const
                int l,const int c): returnType(ret), parameters(std::move(params)), subKind(kind), line(l), column(c) {}


            [[nodiscard]] const Type& getReturnType() const { return *returnType; }
            [[nodiscard]] const std::vector<const Type*>& getParameters() const { return parameters; }
            [[nodiscard]] const SubroutineType getKind() const { return subKind; }
            [[nodiscard]] const int getLine() const { return line; }
            [[nodiscard]] const int getCol() const { return column; }

            [[nodiscard]] const bool isStatic() const {
                return subKind == SubroutineType::FUNCTION || subKind == SubroutineType::CONSTRUCTOR;
            }
    };

    /**
     * @brief A thread-safe registry for tracking all classes and their methods across the entire program.
     *
     * This class is used to perform semantic analysis, such as checking if a called method exists
     * and if the arguments match the expected parameters. It acts as a global symbol table for
     * class and subroutine definitions.
     */
    class GlobalRegistry {
            // Map: ClassName -> (MethodName -> Signature)
            std::unordered_map<std::string_view,std::unordered_map<std::string_view,MethodSignature>> methods;
            // Set: ClassNames
            std::unordered_set<std::string_view> classes;
            mutable std::mutex mtx; // Thread safety
            //void loadStandardLibrary();
        public:
            GlobalRegistry();
            ~GlobalRegistry()=default;

            /** @return true if method was registered, false if already defined in this class */
            bool registerClass(std::string_view className);

            /** @return true if method was registered, false if already defined in this class */
            bool registerMethod(std::string_view className, std::string_view methodName, MethodSignature sig);

            /**
             * @brief Checks if a class exists in the registry.
             *
             * Also returns true for built-in types (int, boolean, char, void).
             *
             * @param className The name of the class to check.
             * @return True if the class exists or is a built-in type, false otherwise.
             */
            [[nodiscard]] bool classExists(std::string_view className) const;

            /**
             * @brief Checks if a method exists within a specific class.
             *
             * @param className The name of the class.
             * @param methodName The name of the method.
             * @return True if the method exists, false otherwise.
             */
            [[nodiscard]] bool methodExists(std::string_view className,std::string_view methodName)const;

            /** @brief Returns signature; caller should check methodExists first */
            [[nodiscard]] MethodSignature getSignature(std::string_view className,std::string_view methodName) const;

            /**
             * @brief Returns the number of registered classes.
             * @return The count of classes.
             */
            int getClassCount()const;

            /**
             * @brief Exports the entire registry to a JSON file for debugging.
             * @param filename The output path (e.g., "registry_dump.json").
             */
            void dumpToJSON(const std::string& filename) const;
    };
}


#endif //NAND2TETRIS_GLOBAL_REGISTRY_H