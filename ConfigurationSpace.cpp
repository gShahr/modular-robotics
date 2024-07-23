#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <queue>
#include "MoveManager.h"
#include "ConfigurationSpace.h"

HashedState::HashedState() : seed(0) {}

HashedState::HashedState(size_t seed) : seed(seed) {}

HashedState::HashedState(const CoordTensor<bool>& state, const CoordTensor<std::string>& colors) {
    HashCoordTensor(state, colors);
}

HashedState::HashedState(const HashedState& other) : seed(other.GetSeed()) {}

size_t HashedState::GetSeed() const {
    return seed;
}

/*void HashedState::HashLattice(const Lattice& lattice) {
    seed = boost::hash_range(lattice.stateTensor.GetArrayInternal().begin(), lattice.stateTensor.GetArrayInternal().end());
}*/

void HashedState::HashCoordTensor(const CoordTensor<bool>& state, const CoordTensor<std::string>& colors) {
    seed = boost::hash_range(state.GetArrayInternal().begin(), state.GetArrayInternal().end());
    for (const auto& color : colors.GetArrayInternal()) {
        boost::hash_combine(seed, boost::hash_value(color));
    }
}

bool HashedState::operator==(const HashedState& other) const {
    return seed == other.GetSeed();
}

size_t std::hash<HashedState>::operator()(const HashedState& state) const {
    return std::hash<size_t>()(state.GetSeed());
}

Configuration::Configuration(CoordTensor<bool> state) : _state(std::move(state)), _colors(CoordTensor<std::string>(state.Order(), state.AxisSize(), "")){}

Configuration::Configuration(CoordTensor<bool> state, CoordTensor<std::string> colors) : _state(std::move(state)), _colors(std::move(colors)) {}

Configuration::~Configuration() {
    for (auto i = next.rbegin(); i != next.rend(); i++) {
        delete *i;
    }
}

std::vector<std::pair<CoordTensor<bool>, CoordTensor<std::string>>> Configuration::MakeAllMoves() {
    std::vector<std::pair<CoordTensor<bool>, CoordTensor<std::string>>> result;
    //lattice =_state;
    Lattice::UpdateFromState(_state);
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

const CoordTensor<std::string>& Configuration::GetColors() const {
    return _colors;
}

const HashedState& Configuration::GetHash() const {
    return hash;
}

void Configuration::SetStateAndHash(const CoordTensor<bool>& state, const CoordTensor<std::string>& colors) {
    _state = state;
    hash = HashedState(state, colors);
}

void Configuration::SetParent(Configuration* configuration) {
    parent = configuration;
}

std::ostream& operator<<(std::ostream& out, const Configuration& config) {
    out << "Configuration: " << config.hash.GetSeed() << std::endl;
    return out;
}

std::vector<Configuration*> ConfigurationSpace::BFS(Configuration* start, Configuration* final) {
#if CONFIG_VERBOSE > CS_LOG_NONE
    int depth = -1;
#endif
    std::queue<Configuration*> q;
    std::unordered_set<HashedState> visited;
    q.push(start);
    visited.insert(start->GetHash());
    while (!q.empty()) {
        Configuration* current = q.front();
        Lattice::UpdateFromState(q.front()->GetState());
#if CONFIG_VERBOSE > CS_LOG_NONE
        if (q.front()->depth != depth) {
            depth++;
#if CONFIG_VERBOSE > CS_LOG_FINAL_DEPTH
            std::cout << "bfs depth: " << q.front()->depth << std::endl << Lattice::ToString() << std::endl;
#endif
        }
#endif
        q.pop();
        if (current->GetState() == final->GetState() && current->GetColors() == final->GetColors()) {
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
    return {};
}

std::vector<Configuration*> ConfigurationSpace::FindPath(Configuration* start, Configuration* final) {
    std::vector<Configuration*> path;
    Configuration* current = final;
    while (current->GetState() != start->GetState()) {
        path.push_back(current);
        current = current->GetParent();
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}