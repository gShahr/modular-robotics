#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <queue>
#include "MoveManager.h"
#include "ConfigurationSpace.h"

const char * BFSExcept::what() const noexcept {
    return "BFS exhausted without finding a path!";
}

HashedState::HashedState() : seed(0) {}

HashedState::HashedState(size_t seed) : seed(seed) {}

HashedState::HashedState(const CoordTensor<bool>& state, const CoordTensor<int>& colors) {
    HashCoordTensor(state, colors);
}

HashedState::HashedState(const HashedState& other) : seed(other.GetSeed()) {}

size_t HashedState::GetSeed() const {
    return seed;
}

void HashedState::HashCoordTensor(const CoordTensor<bool>& state, const CoordTensor<int>& colors) {
    seed = boost::hash_range(state.GetArrayInternal().begin(), state.GetArrayInternal().end());
    size_t seed2 = boost::hash_range(colors.GetArrayInternal().begin(), colors.GetArrayInternal().end());
    boost::hash_combine(seed, seed2);
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

Configuration::Configuration(CoordTensor<bool> state, CoordTensor<int> colors) : _state(std::move(state)), _colors(std::move(colors)) {}

Configuration::~Configuration() {
    for (auto i = next.rbegin(); i != next.rend(); i++) {
        delete *i;
    }
}

std::vector<std::pair<CoordTensor<bool>, CoordTensor<int>>> Configuration::MakeAllMoves() {
    std::vector<std::pair<CoordTensor<bool>, CoordTensor<int>>> result;
    Lattice::UpdateFromState(_state, _colors);
    std::vector<Module*> movableModules = Lattice::MovableModules();
    for (auto module: movableModules) {
        auto legalMoves = MoveManager::CheckAllMoves(Lattice::coordTensor, *module);
        for (auto move : legalMoves) {
            Lattice::MoveModule(*module, move->MoveOffset());
            result.emplace_back(Lattice::stateTensor, Lattice::colorTensor);
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

const CoordTensor<bool>& Configuration::GetState() const {
    return _state;
}

const CoordTensor<int>& Configuration::GetColors() const {
    return _colors;
}

const HashedState& Configuration::GetHash() const {
    return hash;
}

void Configuration::SetStateAndHash(const CoordTensor<bool>& state, const CoordTensor<int>& colors) {
    _state = state;
    _colors = colors;
    hash = HashedState(state, colors);
}

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
    start->SetStateAndHash(start->GetState(), start->GetColors());
    final->SetStateAndHash(final->GetState(), final->GetColors());
    q.push(start);
    visited.insert(start->GetHash());
    while (!q.empty()) {
        Configuration* current = q.front();
        Lattice::UpdateFromState(q.front()->GetState(), q.front()->GetColors());
#if CONFIG_VERBOSE > CS_LOG_NONE
        if (q.front()->depth != depth) {
            depth++;
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
            std::cout << "bfs depth: " << q.front()->depth << std::endl << Lattice::ToString() << std::endl;
#endif
        }
#endif
        q.pop();
        if (current->GetHash() == final->GetHash()) {
#if CONFIG_VERBOSE == CS_LOG_FINAL_DEPTH
            std::cout << "bfs depth: " << depth << std::endl << Lattice::ToString() << std::endl;
#endif
            return FindPath(start, current);
        }
        auto adjList = current->MakeAllMoves();
        for (const auto& [state, colors]: adjList) {
            if (visited.find(HashedState(state, colors)) == visited.end()) {
                auto nextConfiguration = new Configuration(state, colors);
                nextConfiguration->SetParent(current);
                nextConfiguration->SetStateAndHash(state, colors);
                q.push(nextConfiguration);
                current->AddEdge(nextConfiguration);
                nextConfiguration->depth = current->depth + 1;
                visited.insert(HashedState(state, colors));
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