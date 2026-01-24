//
// Created by Nithin Kondabathini on 24/1/2026.
//

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <future>
#include <mutex>
#include <filesystem>
#include <chrono>
#include <sys/resource.h>

#include "../../Compiler/Tokenizer/Tokenizer.h"
#include "../../Compiler/Parser/Parser.h"
#include "../../Compiler/Parser/AST.h"
#include "../../Compiler/SemanticAnalyser/GlobalRegistry.h"
#include "../../Compiler/SemanticAnalyser/SemanticAnalyser.h"

using namespace nand2tetris::jack;
namespace fs = std::filesystem;

std::mutex consoleMutex;

void log(const std::string& msg) {
    std::scoped_lock lock(consoleMutex);
    std::cout << msg << std::endl;
}

double getPeakMemoryMB() {
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);
#ifdef __APPLE__
    return static_cast<double>(rusage.ru_maxrss) / (1024.0 * 1024.0);
#else
    return (double)rusage.ru_maxrss / 1024.0;
#endif
}

struct CompilationUnit {
    std::unique_ptr<Tokenizer> tokenizer;
    std::unique_ptr<ClassNode> ast;
    std::string filePath;
};

std::vector<std::string> getJackOSFiles() {
    std::vector<std::string> osFiles;
    const std::vector<fs::path> paths = {"JackOS", "../JackOS", "../../JackOS", "../../../JackOS"};
    for (const auto& p : paths) {
        if (fs::exists(p) && fs::is_directory(p)) {
            for (const auto& entry : fs::directory_iterator(p)) {
                if (entry.path().extension() == ".jack") {
                    osFiles.push_back(entry.path().string());
                }
            }
            break;
        }
    }
    return osFiles;
}

CompilationUnit parseJob(const std::string& filePath, GlobalRegistry* registry, bool isOS) {
    auto tokenizer = std::make_unique<Tokenizer>(filePath);
    Parser parser(*tokenizer,*registry);
    auto ast = parser.parse();

    if (ast) {
        registry->registerClass(ast->getClassName());
    }

    if (!isOS) log("[Parsed] " + filePath);
    return {std::move(tokenizer), std::move(ast), filePath};
}

void analyzeJob(const ClassNode* ast, const GlobalRegistry* registry) {
    SemanticAnalyser analyser(*registry);
    analyser.analyseClass(*ast);
    log("[Verified] class " + std::string(ast->getClassName()));
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./jack_semantic <files...>" << std::endl;
        return 1;
    }

    try {
        auto startTotal = std::chrono::high_resolution_clock::now();

        GlobalRegistry registry;
        std::vector<std::string> osFiles = getJackOSFiles();
        std::vector<std::string> userFiles;
        for (int i = 1; i < argc; ++i) userFiles.emplace_back(argv[i]);

        std::vector<std::future<CompilationUnit>> osTasks, userTasks;

        // Phase 1: Parse
        auto startParse = std::chrono::high_resolution_clock::now();
        for (const auto& f : osFiles)
            osTasks.push_back(std::async(std::launch::async, parseJob, f, &registry, true));
        for (const auto& f : userFiles)
            userTasks.push_back(std::async(std::launch::async, parseJob, f, &registry, false));

        std::vector<CompilationUnit> units;
        std::vector<ClassNode*> userASTs;

        units.reserve(osTasks.size());
        for (auto& t : osTasks) units.push_back(t.get());
        for (auto& t : userTasks) {
            auto unit = t.get();
            userASTs.push_back(unit.ast.get());
            units.push_back(std::move(unit));
        }
        auto endParse = std::chrono::high_resolution_clock::now();

        // Phase 2: Analyze
        auto startAnalyze = std::chrono::high_resolution_clock::now();
        std::vector<std::future<void>> analysisTasks;
        analysisTasks.reserve(userASTs.size());
        for (const auto* ast : userASTs) {
            analysisTasks.push_back(std::async(std::launch::async, analyzeJob, ast, &registry));
        }

        for (auto& t : analysisTasks) t.get();
        auto endAnalyze = std::chrono::high_resolution_clock::now();

        auto endTotal = std::chrono::high_resolution_clock::now();

        std::cout << "Build Complete." << std::endl;
        std::cout << "Parsing Time:  " << std::chrono::duration<double, std::milli>(endParse - startParse).count() << " ms" << std::endl;
        std::cout << "Analysis Time: " << std::chrono::duration<double, std::milli>(endAnalyze - startAnalyze).count() << " ms" << std::endl;
        std::cout << "Total Time:    " << std::chrono::duration<double, std::milli>(endTotal - startTotal).count() << " ms" << std::endl;
        std::cout << "Peak Memory:   " << getPeakMemoryMB() << " MB" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}