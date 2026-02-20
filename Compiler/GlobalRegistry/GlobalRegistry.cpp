//
// Created by Nithin Kondabathini on 23/1/2026.
//

#include "GlobalRegistry.h"

namespace nand2tetris::jack {
    GlobalRegistry::GlobalRegistry() {
        //loadStandardLibrary();
    }

    bool GlobalRegistry::registerClass(const std::string_view className) {
        std::scoped_lock lock(mtx);
        if (classes.count(className)) return false;
        classes.insert(className);
        return true;
    }

    bool GlobalRegistry::registerMethod(const std::string_view className, const std::string_view methodName,
        MethodSignature sig) {
        std::scoped_lock lock(mtx);

        auto& classMethods = methods[className];

        // Check if the method already exists
        if (classMethods.count(methodName)) return false;

        classMethods.emplace(methodName, std::move(sig));
        return true;
    }

    bool GlobalRegistry::classExists(const std::string_view className) const {
        // Built-ins always "exist"
        if (className == "int" || className == "char" ||
            className == "boolean" || className == "float" || className == "void") {
            return true;
            }
        std::scoped_lock lock(mtx);
        return classes.count(className);
    }

    bool GlobalRegistry::methodExists(const std::string_view className, const std::string_view methodName) const {
        std::scoped_lock lock(mtx);
        const auto it = methods.find(className);
        if (it == methods.end()) return false;
        return it->second.count(methodName);
    }

    MethodSignature GlobalRegistry::getSignature(const std::string_view className,
                                                  const std::string_view methodName) const {
        std::scoped_lock lock(mtx);
        // At() is safe if caller followed the protocol of checking exists() first
        return methods.at(className).at(methodName);
    }

    int GlobalRegistry::getClassCount() const {
        return static_cast<int>(classes.size());
    }
    /*
    void GlobalRegistry::loadStandardLibrary() {
        // --- MATH CLASS ---
        registerClass("Math");
        registerMethod("Math", "init",     MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Math", "abs",      MethodSignature(Type("int"),  {Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Math", "multiply", MethodSignature(Type("int"),  {Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Math", "divide",   MethodSignature(Type("int"),  {Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Math", "min",      MethodSignature(Type("int"),  {Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Math", "max",      MethodSignature(Type("int"),  {Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Math", "sqrt",     MethodSignature(Type("int"),  {Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Math", "bit",      MethodSignature(Type("boolean"), {Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));

        // --- STRING CLASS ---
        registerClass("String");
        registerMethod("String", "new",           MethodSignature(Type("String"), {Type("int")}, SubroutineType::CONSTRUCTOR, 0, 0));
        registerMethod("String", "dispose",       MethodSignature(Type("void"),   {}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "length",        MethodSignature(Type("int"),    {}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "charAt",        MethodSignature(Type("char"),   {Type("int")}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "setCharAt",     MethodSignature(Type("void"),   {Type("int"), Type("char")}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "appendChar",    MethodSignature(Type("String"), {Type("char")}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "eraseLastChar", MethodSignature(Type("void"),   {}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "intValue",      MethodSignature(Type("int"),    {}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "setInt",        MethodSignature(Type("void"),   {Type("int")}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "backSpace",     MethodSignature(Type("char"),   {}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "doubleQuote",   MethodSignature(Type("char"),   {}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "newLine",       MethodSignature(Type("char"),   {}, SubroutineType::METHOD, 0, 0));
        registerMethod("String", "int2String",    MethodSignature(Type("void"),   {}, SubroutineType::METHOD, 0, 0)); // Added back!

        // --- ARRAY CLASS ---
        registerClass("Array");
        registerMethod("Array", "new",     MethodSignature(Type("Array"), {Type("int")}, SubroutineType::CONSTRUCTOR, 0, 0));
        registerMethod("Array", "dispose", MethodSignature(Type("void"),  {}, SubroutineType::METHOD, 0, 0));

        // --- OUTPUT CLASS (Added back!) ---
        registerClass("Output");
        registerMethod("Output", "init",            MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "moveCursor",      MethodSignature(Type("void"), {Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "printChar",       MethodSignature(Type("void"), {Type("char")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "printString",     MethodSignature(Type("void"), {Type("String")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "printInt",        MethodSignature(Type("void"), {Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "println",         MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "backSpace",       MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "initMap",         MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "create",          MethodSignature(Type("void"), {Type("int"), Type("int"), Type("int"), Type("int"), Type("int"), Type("int"), Type("int"), Type("int"), Type("int"), Type("int"), Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "getMap",          MethodSignature(Type("Array"), {Type("char")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "incrementCursor", MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Output", "decrementCursor", MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));

        // --- SCREEN CLASS (Added back!) ---
        registerClass("Screen");
        registerMethod("Screen", "init",          MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Screen", "clearScreen",   MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Screen", "setColor",      MethodSignature(Type("void"), {Type("boolean")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Screen", "drawPixel",     MethodSignature(Type("void"), {Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Screen", "drawLine",      MethodSignature(Type("void"), {Type("int"), Type("int"), Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Screen", "drawRectangle", MethodSignature(Type("void"), {Type("int"), Type("int"), Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Screen", "drawCircle",    MethodSignature(Type("void"), {Type("int"), Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));

        // --- KEYBOARD CLASS (Added back!) ---
        registerClass("Keyboard");
        registerMethod("Keyboard", "init",       MethodSignature(Type("void"),   {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Keyboard", "keyPressed", MethodSignature(Type("char"),   {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Keyboard", "readChar",   MethodSignature(Type("char"),   {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Keyboard", "readLine",   MethodSignature(Type("String"), {Type("String")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Keyboard", "readInt",    MethodSignature(Type("int"),    {Type("String")}, SubroutineType::FUNCTION, 0, 0));

        // --- MEMORY CLASS ---
        registerClass("Memory");
        registerMethod("Memory", "init",    MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Memory", "peek",    MethodSignature(Type("int"),  {Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Memory", "poke",    MethodSignature(Type("void"), {Type("int"), Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Memory", "alloc",   MethodSignature(Type("int"),  {Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Memory", "deAlloc", MethodSignature(Type("void"), {Type("Array")}, SubroutineType::FUNCTION, 0, 0));

        // --- SYS CLASS ---
        registerClass("Sys");
        registerMethod("Sys", "init",  MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Sys", "halt",  MethodSignature(Type("void"), {}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Sys", "error", MethodSignature(Type("void"), {Type("int")}, SubroutineType::FUNCTION, 0, 0));
        registerMethod("Sys", "wait",  MethodSignature(Type("void"), {Type("int")}, SubroutineType::FUNCTION, 0, 0));
    }*/

    void GlobalRegistry::dumpToJSON(const std::string &filename) const {
        std::ofstream out(filename);
        if (!out.is_open()) return;

        out << "{\n";
        out << "  \"registry\": [\n";

        bool firstMethod = true;

        // Lock the registry to safely read during parallel operations
        std::scoped_lock lock(mtx);

        // Iterate over all classes
        for (const auto& [className, methodMap] : methods) {

            // Iterate over all methods in this class
            for (const auto& [methodName, sig] : methodMap) {
                if (!firstMethod) out << ",\n";
                firstMethod = false;

                // Resolve the subroutine kind
                std::string subTypeStr;
                if (sig.getKind() == SubroutineType::CONSTRUCTOR) subTypeStr = "constructor";
                else if (sig.getKind() == SubroutineType::FUNCTION) subTypeStr = "function";
                else subTypeStr = "method";

                out << "    {\n";
                out << "      \"class\": \"" << className << "\",\n";
                out << "      \"method\": \"" << methodName << "\",\n";
                out << "      \"type\": \"" << subTypeStr << "\",\n";

                // Use formatType to convert the Type object (including generics) to a string
                out << "      \"return\": \"" << sig.getReturnType().formatType() << "\",\n";

                // Format parameters: "int, Array<School>"
                out << "      \"params\": \"";
                const auto& params = sig.getParameters();
                for (size_t i = 0; i < params.size(); ++i) {
                    out << params[i]->formatType();
                    if (i < params.size() - 1) out << ", ";
                }
                out << "\"\n";
                out << "    }";
            }
        }

        out << "\n  ]\n";
        out << "}\n";
        out.close();
    }
}
