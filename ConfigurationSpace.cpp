#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <queue>
#include <utility>
#include "MoveManager.h"
#include "ConfigurationSpace.h"

const char * BFSExcept::what() const noexcept {
    return "BFS exhausted without finding a path!";
}

HashedState::HashedState() : seed(0) {}

HashedState::HashedState(size_t seed) : seed(seed) {}

HashedState::HashedState(const std::unordered_set<ModuleBasic>& modData) {
    seed = 0;
    for (const auto& data : modData) {
        boost::hash<ModuleBasic> modBasicHasher;
        boost::hash_combine(seed, modBasicHasher(data));
    }
    //seed = boost::hash_range(modData.begin(), modData.end());
}

HashedState::HashedState(const HashedState& other) : seed(other.GetSeed()) {}

size_t HashedState::GetSeed() const {
    return seed;
}

bool HashedState::operator==(const HashedState& other) const {
    return seed == other.GetSeed();
}

bool HashedState::operator!=(const HashedState& other) const {
    return seed != other.GetSeed();
}

size_t std::hash<HashedState>::operator()(const HashedState& state) const {
    return std::hash<size_t>()(state.GetSeed());
}

Configuration::Configuration(std::unordered_set<ModuleBasic> modData) : _nonStatModData(modData) {
    hash = HashedState(modData);
}

Configuration::~Configuration() {
    for (auto i = next.rbegin(); i != next.rend(); i++) {
        delete *i;
    }
}

std::vector<std::unordered_set<ModuleBasic>> Configuration::MakeAllMoves() {
    std::vector<std::unordered_set<ModuleBasic>> result;
    Lattice::UpdateFromModuleInfo(_nonStatModData);
    std::vector<Module*> movableModules = Lattice::MovableModules();
    for (auto module: movableModules) {
        auto legalMoves = MoveManager::CheckAllMoves(Lattice::coordTensor, *module);
        for (auto move : legalMoves) {
            Lattice::MoveModule(*module, move->MoveOffset());
            result.emplace_back(Lattice::GetModuleInfo());
            Lattice::MoveModule(*module, -move->MoveOffset());
        }
    }
    return result;
}

void Configuration::AddEdge(Configuration* configuration) {
    next.push_back(configuration);
}

Configuration* Configuration::GetParent() {
    return parent;
}

std::vector<Configuration*> Configuration::GetNext() {
    return next;
}

const HashedState& Configuration::GetHash() const {
    return hash;
}

const std::unordered_set<ModuleBasic>& Configuration::GetModData() const {
    return _nonStatModData;
}

/*void Configuration::SetStateAndHash(const std::vector<ModuleBasic>& modData) {
    _nonStatModData = modData;
    hash = HashedState(modData);
}*/

void Configuration::SetParent(Configuration* configuration) {
    parent = configuration;
}

std::ostream& operator<<(std::ostream& out, const Configuration& config) {
    out << "Configuration: " << config.hash.GetSeed() << std::endl;
    return out;
}

int ConfigurationSpace::depth = -1;

std::vector<Configuration*> ConfigurationSpace::BFS(Configuration* start, Configuration* final) {
    std::queue<Configuration*> q;
    std::unordered_set<HashedState> visited;
    //start->SetStateAndHash(start->GetModData());
    //final->SetStateAndHash(final->GetModData());
    q.push(start);
    visited.insert(start->GetHash());
    while (!q.empty()) {
        Configuration* current = q.front();
        Lattice::UpdateFromModuleInfo(q.front()->GetModData());
#if CONFIG_VERBOSE > CS_LOG_NONE
        if (q.front()->depth != depth) {
            depth++;
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
            std::cout << "bfs depth: " << q.front()->depth << std::endl << Lattice::ToString() << std::endl;
#endif
        }
#endif
        q.pop();
        //if (current->GetHash() == final->GetHash()) {
        if (current->GetModData() == final->GetModData()) {
#if CONFIG_VERBOSE == CS_LOG_FINAL_DEPTH
            std::cout << "bfs depth: " << depth << std::endl << Lattice::ToString() << std::endl;
#endif
            return FindPath(start, current);
        }
        auto adjList = current->MakeAllMoves();
        for (const auto& moduleInfo : adjList) {
            if (visited.find(HashedState(moduleInfo)) == visited.end()) {
                auto nextConfiguration = new Configuration(moduleInfo);
                nextConfiguration->SetParent(current);
                //nextConfiguration->SetStateAndHash(moduleInfo);
                q.push(nextConfiguration);
                current->AddEdge(nextConfiguration);
                nextConfiguration->depth = current->depth + 1;
                visited.insert(HashedState(moduleInfo));
            }
        }
    }
    throw BFSExcept();
}

std::vector<Configuration*> ConfigurationSpace::FindPath(Configuration* start, Configuration* final) {
    std::vector<Configuration*> path;
    Configuration* current = final;
    while (current->GetHash() != start->GetHash()) {
        path.push_back(current);
        current = current->GetParent();
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}