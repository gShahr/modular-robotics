#include "ModuleManager.h"

Module::Module(const std::valarray<int>& coords, bool isStatic) : id(ModuleIdManager::GetNextId()), coords(coords), moduleStatic(isStatic) {
    ModuleIdManager::RegisterModule(*this);
}

int ModuleIdManager::_nextId = 0;
std::vector<Module> ModuleIdManager::_modules;

void ModuleIdManager::RegisterModule(Module &module) {
    _modules.emplace_back(module);
}

int ModuleIdManager::GetNextId() {
    return _nextId++;
}

std::vector<Module>& ModuleIdManager::Modules() {
    return _modules;
}

std::ostream& operator<<(std::ostream& out, const Module& mod) {
    out << "Module with ID " << mod.id << " at ";
    std::string sep = "(";
    for (auto coord : mod.coords) {
        out << sep << coord;
        sep = ", ";
    }
    out << ")";
    return out;
}