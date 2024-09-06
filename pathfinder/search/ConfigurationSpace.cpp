#include <random>
#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <queue>
#include <set>
#include <utility>
#include <omp.h>
#include "../moves/MoveManager.h"
#include "ConfigurationSpace.h"
#include "SearchAnalysis.h"

const char * BFSExcept::what() const noexcept {
    return "BFS exhausted without finding a path!";
}

HashedState::HashedState(const std::set<ModuleBasic>& modData) {
    seed = boost::hash_range(modData.begin(), modData.end());
    moduleData = modData;
}

HashedState::HashedState(const HashedState& other) : seed(other.GetSeed()), moduleData(other.GetState()) {}

size_t HashedState::GetSeed() const {
    return seed;
}

const std::set<ModuleBasic>& HashedState::GetState() const {
    return moduleData;
}

bool HashedState::operator==(const HashedState& other) const {
    return seed == other.GetSeed() && moduleData == other.GetState();
}

bool HashedState::operator!=(const HashedState& other) const {
    return seed != other.GetSeed();
}

size_t std::hash<HashedState>::operator()(const HashedState& state) const {
    return state.GetSeed();
}

Configuration::Configuration(const std::set<ModuleBasic>& modData) : hash(HashedState(modData)) {}

Configuration::~Configuration() {
    for (auto i = next.rbegin(); i != next.rend(); i++) {
        delete *i;
    }
}

std::vector<std::set<ModuleBasic>> Configuration::MakeAllMoves() {
    std::vector<std::set<ModuleBasic>> result;
    Lattice::UpdateFromModuleInfo(GetModData());
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

std::vector<std::set<ModuleBasic>> Configuration::MakeAllMovesForAllVertices() {
    std::vector<std::set<ModuleBasic>> result;
    Lattice::UpdateFromModuleInfo(_nonStatModData);
    std::vector<Module*> movableModules;
    for (int id = 0; id < Lattice::AxisSize() * Lattice::Order(); id++) {
        movableModules.push_back(&ModuleIdManager::GetModule(id));
    }
    for (auto module: movableModules) {
        auto legalMoves = MoveManager::CheckAllMoves(Lattice::coordTensor, *module);
        for (auto move: legalMoves) {
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
    return hash.GetState();
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
#if CONFIG_OUTPUT_JSON
    SearchAnalysis::EnterGraph("BFSDepthOverTime");
    SearchAnalysis::LabelGraph("BFS Depth over Time");
    SearchAnalysis::LabelAxes("Time (μs)", "Depth");
    SearchAnalysis::SetInterpolationOrder(0);
    SearchAnalysis::EnterGraph("BFSStatesOverTime");
    SearchAnalysis::LabelGraph("BFS States visited over Time");
    SearchAnalysis::LabelAxes("Time (μs)", "States visited");
    SearchAnalysis::SetInterpolationOrder(1);
    SearchAnalysis::StartClock();
#endif
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
#if CONFIG_OUTPUT_JSON
        SearchAnalysis::PauseClock();
#endif
        if (q.front()->depth != depth) {
            depth++;
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
            std::cout << "bfs depth: " << q.front()->depth << std::endl
            << "duplicate states avoided: " << dupesAvoided << std::endl
            << "states visited: " << visited.size() << std::endl
            << Lattice::ToString() << std::endl;
#if CONFIG_OUTPUT_JSON
            SearchAnalysis::EnterGraph("BFSDepthOverTime");
            SearchAnalysis::InsertTimePoint(depth);
            SearchAnalysis::EnterGraph("BFSStatesOverTime");
            SearchAnalysis::InsertTimePoint(visited.size());
#endif
#endif
        }
#if CONFIG_OUTPUT_JSON
        SearchAnalysis::ResumeClock();
#endif
#endif
        q.pop();
        if (current->GetHash() == final->GetHash()) {
        //if (current->GetModData() == final->GetModData()) {
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
#if CONFIG_OUTPUT_JSON
            SearchAnalysis::PauseClock();
#endif
            std::cout << "bfs final depth: " << q.front()->depth << std::endl
            << "duplicate states avoided: " << dupesAvoided << std::endl
            << "states visited: " << visited.size() << std::endl
            << Lattice::ToString() << std::endl;
#if CONFIG_OUTPUT_JSON
            SearchAnalysis::EnterGraph("BFSDepthOverTime");
            SearchAnalysis::InsertTimePoint(depth);
            SearchAnalysis::EnterGraph("BFSStatesOverTime");
            SearchAnalysis::InsertTimePoint(visited.size());
#endif
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
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
            std::cout << "bfs final depth: " << depth << std::endl << Lattice::ToString() << std::endl;
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

template <typename Heuristic>
auto CompareConfiguration(Configuration* final, Heuristic heuristic) {
    return [final, heuristic](Configuration* c1, Configuration* c2) {
        int cost1 = c1->GetCost() + (c1->*heuristic)(final);
        int cost2 = c2->GetCost() + (c2->*heuristic)(final);
        return (cost1 == cost2) ? c1->GetCost() > c2->GetCost() : cost1 > cost2;
    };
}

bool Configuration::ValarrayComparator::operator()(const std::valarray<int>& lhs, const std::valarray<int>& rhs) const {
    for (size_t i = 0; i < std::min(lhs.size(), rhs.size()); ++i) {
        if (lhs[i] < rhs[i]) return true;
        if (lhs[i] > rhs[i]) return false;
    }
    return lhs.size() < rhs.size();
}

float Configuration::ManhattanDistance(Configuration* final) {
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
    return h / 3;
}

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

int Configuration::ChebyshevDistance(Configuration* final) {
    auto currentData = this->GetModData();
    auto finalData = final->GetModData();
    auto currentIt = currentData.begin();
    auto finalIt = finalData.begin();
    int h = 0;
    while (currentIt != currentData.end() && finalIt != finalData.end()) {
        const auto& currentModule = *currentIt;
        const auto& finalModule = *finalIt;
        std::valarray<int> diff = currentModule.coords - finalModule.coords;
        int maxDiff = 0;
        for (auto& val : diff) {
            maxDiff = std::max(maxDiff, std::abs(val));
        }
        h += maxDiff;
        currentIt++;
        finalIt++;
    }
    return h;
}

std::vector<Configuration*> ConfigurationSpace::AStar(Configuration* start, Configuration* final) {
#if CONFIG_OUTPUT_JSON
    SearchAnalysis::EnterGraph("AStarDepthOverTime");
    SearchAnalysis::LabelGraph("A* Depth over Time");
    SearchAnalysis::LabelAxes("Time (μs)", "Depth");
    SearchAnalysis::SetInterpolationOrder(0);
    SearchAnalysis::EnterGraph("AStarStatesOverTime");
    SearchAnalysis::LabelGraph("A* States visited over Time");
    SearchAnalysis::LabelAxes("Time (μs)", "States visited");
    SearchAnalysis::SetInterpolationOrder(1);
    SearchAnalysis::StartClock();
#endif
    int dupesAvoided = 0;
    auto compare = CompareConfiguration(final, &Configuration::ManhattanDistance);
    using CompareType = decltype(compare);
    std::priority_queue<Configuration*, std::vector<Configuration*>, CompareType> pq(compare);
    std::unordered_set<HashedState> visited;
    start->SetCost(0);
    pq.push(start);
    visited.insert(start->GetHash());

    while (!pq.empty()) {
        Configuration* current = pq.top();
        Lattice::UpdateFromModuleInfo(current->GetModData());
#if CONFIG_VERBOSE > CS_LOG_NONE
#if CONFIG_OUTPUT_JSON
        SearchAnalysis::PauseClock();
#endif
        if (current->depth != depth) {
            depth = current->depth;
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
            std::cout << "A* depth: " << current->depth << std::endl
                    << "duplicate states avoided: " << dupesAvoided << std::endl
                    << "states visited: " << visited.size() << std::endl
                    << Lattice::ToString() << std::endl;
#if CONFIG_OUTPUT_JSON
            SearchAnalysis::EnterGraph("AStarDepthOverTime");
            SearchAnalysis::InsertTimePoint(depth);
            SearchAnalysis::EnterGraph("AStarStatesOverTime");
            SearchAnalysis::InsertTimePoint(visited.size());
#endif
#endif
        }
#if CONFIG_OUTPUT_JSON
        SearchAnalysis::ResumeClock();
#endif
#endif
        pq.pop();
        //if (current->GetModData() == final->GetModData()) {
        if (current->GetHash() == final->GetHash()) {
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
#if CONFIG_OUTPUT_JSON
            SearchAnalysis::PauseClock();
#endif
            std::cout << "A* final depth: " << current->depth << std::endl
                    << "duplicate states avoided: " << dupesAvoided << std::endl
                    << "states visited: " << visited.size() << std::endl
                    << Lattice::ToString() << std::endl;
#if CONFIG_OUTPUT_JSON
            SearchAnalysis::EnterGraph("AStarDepthOverTime");
            SearchAnalysis::InsertTimePoint(depth);
            SearchAnalysis::EnterGraph("AStarStatesOverTime");
            SearchAnalysis::InsertTimePoint(visited.size());
#endif
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

Configuration ConfigurationSpace::GenerateRandomFinal(int targetMoves) {
    std::unordered_set<HashedState> visited;
    std::set<ModuleBasic> initialState = Lattice::GetModuleInfo();
    std::set<ModuleBasic> nextState;

    for (int i = 0; i < targetMoves; i++) {
        // Get current configuration
        Configuration current(Lattice::GetModuleInfo());
        // Get adjacent configurations
        auto adjList = current.MakeAllMoves();
        // Shuffle the adjacent configurations
        std::shuffle(adjList.begin(), adjList.end(), std::mt19937{std::random_device{}()});
        // Search through shuffled configurations until an unvisited one is found
        nextState = {};
        for (const auto& state: adjList) {
            if (visited.find(HashedState(state)) == visited.end()) {
                nextState = state;
                break;
            }
        }
        // Check to see if a valid adjacent state was found
        if (nextState.empty()) {
            // If no adjacent state was found, return early
            std::cerr << "GenerateRandomFinal returning early (" << i << "/" << targetMoves << " moves) due to lack of new moves" << std::endl;
            Lattice::UpdateFromModuleInfo(initialState);
            return current;
        }
        // Otherwise, update lattice with new state and resume loop
        visited.insert(HashedState(nextState));
        Lattice::UpdateFromModuleInfo(nextState);
    }

    // Reset lattice to original state and return
    Lattice::UpdateFromModuleInfo(initialState);
    return Configuration(nextState);
}