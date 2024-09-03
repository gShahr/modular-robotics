#include <vector>
#include "../lattice/Lattice.h"

#ifndef MODULAR_ROBOTICS_CONFIGURATIONSPACE_H
#define MODULAR_ROBOTICS_CONFIGURATIONSPACE_H

// Verbosity Constants (Don't change these)
#define CS_LOG_NONE 0
#define CS_LOG_FINAL_DEPTH 1
#define CS_LOG_EVERY_DEPTH 2
/* Verbosity Configuration
 * NONE: No output from BFS
 * FINAL_DEPTH: Output final depth and configuration upon BFS completion
 * EVERY_DEPTH: Output current depth and configuration every time BFS depth increases
 */
#define CONFIG_VERBOSE CS_LOG_EVERY_DEPTH
/* JSON Output Configuration
 * In order to output JSON successfully logging must be enabled for every depth
 */
#define CONFIG_OUTPUT_JSON true
#if CONFIG_VERBOSE != CS_LOG_EVERY_DEPTH && CONFIG_OUTPUT_JSON
#warning "JSON output disabled due to insufficient logging!"
#define CONFIG_OUTPUT_JSON false
#endif

class BFSExcept : std::exception {
public:
    [[nodiscard]]
    const char * what() const noexcept override;
};

// For comparing the state of a lattice and a configuration
class HashedState {
private:
    size_t seed;
    std::set<ModuleBasic> moduleData;
public:
    HashedState() = delete;

    explicit HashedState(const std::set<ModuleBasic>& modData);

    HashedState(const HashedState& other);

    [[nodiscard]]
    size_t GetSeed() const;

    [[nodiscard]]
    const std::set<ModuleBasic>& GetState() const;

    bool operator==(const HashedState& other) const;

    bool operator!=(const HashedState& other) const;
};

template<>
struct std::hash<HashedState> {
    size_t operator()(const HashedState& state) const;
};

// For tracking the state of a lattice
class Configuration {
private:
    Configuration* parent = nullptr;
    std::vector<Configuration*> next;
    HashedState hash;
    int cost;
public:
    int depth = 0;

    explicit Configuration(const std::set<ModuleBasic>& modData);

    ~Configuration();

    std::vector<std::set<ModuleBasic>> MakeAllMoves();

    void AddEdge(Configuration* configuration);

    Configuration* GetParent();

    std::vector<Configuration*> GetNext();

    [[nodiscard]]
    const HashedState& GetHash() const;

    [[nodiscard]]
    const std::set<ModuleBasic>& GetModData() const;

    void SetParent(Configuration* configuration);

    friend std::ostream& operator<<(std::ostream& out, const Configuration& config);

    int GetCost();

    void SetCost(int cost);

    template <typename Heuristic>
    auto CompareConfiguration(Configuration* final, Heuristic heuristic);

    struct ValarrayComparator {
        bool operator()(const std::valarray<int>& lhs, const std::valarray<int>& rhs) const;
    };

    float ManhattanDistance(Configuration* final);

    int SymmetricDifferenceHeuristic(Configuration* final);

    int ChebyshevDistance(Configuration* final);
};

namespace ConfigurationSpace {
    extern int depth;

    std::vector<Configuration*> BFS(Configuration* start, Configuration* final);

    std::vector<Configuration*> BFSParallelized(Configuration* start, Configuration* final);

    std::vector<Configuration*> AStar(Configuration* start, Configuration* final);

    std::vector<Configuration*> FindPath(Configuration* start, Configuration* final);

    Configuration GenerateRandomFinal(int targetMoves = 24);
}

#endif //MODULAR_ROBOTICS_CONFIGURATIONSPACE_H
