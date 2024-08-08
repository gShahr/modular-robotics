#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <queue>
#include <set>
#include <utility>
#include <omp.h>
#include "MoveManager.h"
#include "ConfigurationSpace.h"

const char * BFSExcept::what() const noexcept {
    return "BFS exhausted without finding a path!";
}

HashedState::HashedState() : seed(0) {}

HashedState::HashedState(size_t seed) : seed(seed) {}

HashedState::HashedState(const std::set<ModuleBasic>& modData) {
    seed = boost::hash_range(modData.begin(), modData.end());
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
    //return state.GetSeed();
}

Configuration::Configuration(std::set<ModuleBasic> modData) : _nonStatModData(modData) {
    hash = HashedState(modData);
}

Configuration::~Configuration() {
    for (auto i = next.rbegin(); i != next.rend(); i++) {
        delete *i;
    }
}

std::vector<std::set<ModuleBasic>> Configuration::MakeAllMoves() {
    std::vector<std::set<ModuleBasic>> result;
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

const std::set<ModuleBasic>& Configuration::GetModData() const {
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
    int dupesAvoided = 0;
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
            std::cout << "bfs depth: " << q.front()->depth << std::endl
            << "duplicate states avoided: " << dupesAvoided << std::endl
            << "states visited: " << visited.size() << std::endl
            << Lattice::ToString() << std::endl;
#endif
        }
#endif
        q.pop();
        if (current->GetHash() == final->GetHash()) {
        //if (current->GetModData() == final->GetModData()) {
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
            } else {
                dupesAvoided++;
            }
        }
    }
    throw BFSExcept();
}

std::vector<Configuration*> ConfigurationSpace::BFSParallelized(Configuration* start, Configuration* final) {
    std::queue<Configuration*> q;
    std::unordered_set<HashedState> visited;
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
        if (current->GetModData() == final->GetModData()) {
#if CONFIG_VERBOSE == CS_LOG_FINAL_DEPTH
            std::cout << "bfs depth: " << depth << std::endl << Lattice::ToString() << std::endl;
#endif
            return FindPath(start, current);
        }
        auto adjList = current->MakeAllMoves();
        #pragma omp parallel for
        for (const auto& moduleInfo : adjList) {
            int thread_id = omp_get_thread_num();
            std::cout << "Thread " << thread_id << " is processing moduleInfo." << std::endl;
            HashedState hashedState(moduleInfo);
            bool isVisited = false;
            #pragma omp critical
            {
                isVisited = (visited.find(hashedState) != visited.end());
            }
            if (!isVisited) {
                auto nextConfiguration = new Configuration(moduleInfo);
                nextConfiguration->SetParent(current);
                #pragma omp critical
                {
                    q.push(nextConfiguration);
                }
                current->AddEdge(nextConfiguration);
                nextConfiguration->depth = current->depth + 1;
                visited.insert(HashedState(moduleInfo));
            }
        }
    }
    throw BFSExcept();
}

int Configuration::GetCost() {
    return cost;
}

void Configuration::SetCost(int cost) {
    this->cost = cost;
}

float Configuration::Heuristic(Configuration* final) {
    auto currentData = this->GetModData();
    auto finalData = final->GetModData();
    auto currentIt = currentData.begin();
    auto finalIt = finalData.begin();
    float h = 0;
    while (currentIt != currentData.end() && finalIt != finalData.end()) {
        const auto& currentModule = *currentIt;
        const auto& finalModule = *finalIt;
        std::valarray<int> diff = currentModule.coords - finalModule.coords;
        for (auto& val : diff) {
            h += std::abs(val);
        }
        currentIt++;
        finalIt++;
    }
    //TODO: find out what the right number is
    return h / 6;
}

struct ValarrayComparator {
    bool operator()(const std::valarray<int>& lhs, const std::valarray<int>& rhs) const {
        for (size_t i = 0; i < std::min(lhs.size(), rhs.size()); ++i) {
            if (lhs[i] < rhs[i]) return true;
            if (lhs[i] > rhs[i]) return false;
        }
        return lhs.size() < rhs.size();
    }
};

int Configuration::SymmetricDifferenceHeuristic(Configuration* final) {
    auto currentData = this->GetModData();
    auto finalData = final->GetModData();
    auto currentIt = currentData.begin();
    auto finalIt = finalData.begin();
    int h = 0;
    std::set<std::valarray<int>, ValarrayComparator> unionCoords;
    while (currentIt != currentData.end() && finalIt != finalData.end()) {
        const auto& currentModule = *currentIt;
        const auto& finalModule = *finalIt;
        unionCoords.insert(currentModule.coords);
        unionCoords.insert(finalModule.coords);
        currentIt++;
        finalIt++;
    }
    int symDifference = 2 * unionCoords.size() - (currentData.size() + finalData.size());
    return symDifference / 2;
}

std::vector<Configuration*> ConfigurationSpace::AStar(Configuration* start, Configuration* final) {
    int dupesAvoided = 0;
    auto CompareConfiguration = [final](Configuration* c1, Configuration* c2) {
        return (c1->GetCost() + c1->Heuristic(final) == c2->GetCost() + c2->Heuristic(final)) ?
        c1->GetCost() > c2->GetCost() :
        c1->GetCost() + c1->Heuristic(final) > c2->GetCost() + c2->Heuristic(final);
    };
    using CompareType = decltype(CompareConfiguration);
    std::priority_queue<Configuration*, std::vector<Configuration*>, CompareType> pq(CompareConfiguration);
    std::unordered_set<HashedState> visited;
    start->SetCost(0);
    pq.push(start);
    visited.insert(start->GetHash());

    while (!pq.empty()) {
        Configuration* current = pq.top();
        Lattice::UpdateFromModuleInfo(current->GetModData());
#if CONFIG_VERBOSE > CS_LOG_NONE
        if (current->depth != depth) {
            depth++;
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
            std::cout << "A* depth: " << current->depth << std::endl
                    << "duplicate states avoided: " << dupesAvoided << std::endl
                    << "states visited: " << visited.size() << std::endl
                    << Lattice::ToString() << std::endl;
#endif
        }
#endif
        pq.pop();
        //if (current->GetModData() == final->GetModData()) {
        if (current->GetHash() == final->GetHash()) {
#if CONFIG_VERBOSE == CS_LOG_FINAL_DEPTH
            std::cout << "A* depth: " << depth << std::endl << Lattice::ToString() << std::endl;
#endif
            return FindPath(start, current);
        }
        auto adjList = current->MakeAllMoves();
        for (const auto& moduleInfo : adjList) {
            HashedState hashedState(moduleInfo);
            if (visited.find(hashedState) == visited.end()) {
                auto nextConfiguration = new Configuration(moduleInfo);
                nextConfiguration->SetParent(current);
                nextConfiguration->SetCost(current->GetCost() + 1);
                pq.push(nextConfiguration);
                current->AddEdge(nextConfiguration);
                nextConfiguration->depth = current->depth + 1;
                visited.insert(hashedState);
            } else {
                dupesAvoided++;
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