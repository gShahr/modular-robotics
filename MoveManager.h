#include <vector>
#include <unordered_map>
#include <valarray>
#include <nlohmann/json.hpp>
#include "Lattice.h"
#include "Isometry.h"

#ifndef MODULAR_ROBOTICS_MOVEMANAGER_H
#define MODULAR_ROBOTICS_MOVEMANAGER_H

// Verbosity Constants (Don't change these)
#define MM_LOG_NONE 0
#define MM_LOG_MOVE_DEFS 1
#define MM_LOG_MOVE_CHECKS 2
/* Verbosity Configuration
 * NONE: No output from MoveManager
 * MOVE_DEFS: Output move def information
 * MOVE_CHECK: Output move def information and move check results
 */
#define MOVEMANAGER_VERBOSE MM_LOG_MOVE_DEFS

namespace Move {
    enum State {
        NOCHECK = ' ',
        EMPTY = 'x',
        INITIAL = '?',
        FINAL = '!',
        STATIC = '#'
    };

    enum AnimType {
        Z_SLIDE = -3,
        Y_SLIDE = -2,
        X_SLIDE = -1,
        GEN_SLIDE = 0,
        PIVOT_PX = 1,
        PIVOT_PY = 2,
        PIVOT_PZ = 3,
        PIVOT_NX = 4,
        PIVOT_NY = 5,
        PIVOT_NZ = 6
    };

    // For easily converting from string to enum
    const std::unordered_map<std::string, AnimType> StrAnimMap = {
            {"z-slide", Z_SLIDE},
            {"y-slide", Y_SLIDE},
            {"x-slide", X_SLIDE},
            {"slide", GEN_SLIDE},
            {"pivot+x", PIVOT_PX},
            {"pivot+y", PIVOT_PY},
            {"pivot+z", PIVOT_PZ},
            {"pivot-x", PIVOT_NX},
            {"pivot-y", PIVOT_NY},
            {"pivot-z", PIVOT_NZ}
    };

    // For easily rotating move types
    const std::unordered_map<AnimType, std::vector<AnimType>> AnimRotationMap = {
            {Z_SLIDE, {Z_SLIDE, Z_SLIDE, X_SLIDE}},
            {Y_SLIDE, {Y_SLIDE, X_SLIDE, Y_SLIDE}},
            {X_SLIDE, {X_SLIDE, Y_SLIDE, Z_SLIDE}},
            {GEN_SLIDE, {GEN_SLIDE, GEN_SLIDE, GEN_SLIDE}},
            {PIVOT_PX, {PIVOT_PX, PIVOT_PY, PIVOT_PZ}},
            {PIVOT_PY, {PIVOT_PY, PIVOT_PX, PIVOT_PY}},
            {PIVOT_PZ, {PIVOT_PZ, PIVOT_PX, PIVOT_PY}},
            {PIVOT_NX, {PIVOT_NX, PIVOT_NY, PIVOT_NZ}},
            {PIVOT_NY, {PIVOT_NY, PIVOT_NX, PIVOT_NY}},
            {PIVOT_NZ, {PIVOT_NZ, PIVOT_NX, PIVOT_NY}}
    };

    // For easily reflecting move types
    const std::unordered_map<AnimType, std::vector<AnimType>> AnimReflectionMap = {
            {Z_SLIDE, {Z_SLIDE, Z_SLIDE, Z_SLIDE}},
            {Y_SLIDE, {Y_SLIDE, Y_SLIDE, Y_SLIDE}},
            {X_SLIDE, {X_SLIDE, X_SLIDE, X_SLIDE}},
            {GEN_SLIDE, {GEN_SLIDE, GEN_SLIDE, GEN_SLIDE}},
            {PIVOT_PX, {PIVOT_NX, PIVOT_PX, PIVOT_PX}},
            {PIVOT_PY, {PIVOT_PY, PIVOT_NY, PIVOT_PY}},
            {PIVOT_PZ, {PIVOT_PZ, PIVOT_PZ, PIVOT_NZ}},
            {PIVOT_NX, {PIVOT_PX, PIVOT_NX, PIVOT_NX}},
            {PIVOT_NY, {PIVOT_NY, PIVOT_PY, PIVOT_NY}},
            {PIVOT_NZ, {PIVOT_NZ, PIVOT_NZ, PIVOT_PZ}}
    };
}

class MoveBase : public ITransformable {
protected:
    // each pair represents a coordinate offset to check and whether a module should be there or not
    std::vector<std::pair<std::valarray<int>, bool>> moves;
    // bounds ex: {(2, 1), (0, 1)} would mean bounds extend from -2 to 1 on x-axis and 0 to 1 on y-axis
    std::vector<std::pair<int, int>> bounds;
    std::valarray<int> initPos, finalPos;
    std::vector<std::pair<Move::AnimType, std::valarray<int>>> animSequence;
public:
    // Load in move info from a given file
    // virtual void InitMove(std::ifstream& moveFile) = 0;
    virtual void InitMove(const nlohmann::basic_json<>& moveDef) = 0;
    // Check to see if move is possible for a given module
    virtual bool MoveCheck(CoordTensor<int>& tensor, const Module& mod) = 0;

    MoveBase* MakeCopy() const override = 0;

    void Rotate(int a, int b) override;

    void Reflect(int index) override;

    [[nodiscard]]
    const std::valarray<int>& MoveOffset() const;

    [[nodiscard]]
    const std::vector<std::pair<Move::AnimType, std::valarray<int>>>& AnimSequence() const;

    friend class MoveManager;
};

class Move2d : public MoveBase {
public:
    Move2d();
    [[nodiscard]]
    MoveBase* MakeCopy() const override;
    //void InitMove(std::ifstream& moveFile) override;
    void InitMove(const nlohmann::basic_json<>& moveDef) override;
    bool MoveCheck(CoordTensor<int>& tensor, const Module& mod) override;
};

class Move3d : public MoveBase {
public:
    Move3d();
    [[nodiscard]]
    MoveBase* MakeCopy() const override;
    //void InitMove(std::ifstream& moveFile) override;
    void InitMove(const nlohmann::basic_json<>& moveDef) override;
    bool MoveCheck(CoordTensor<int>& tensor, const Module& mod) override;
};

class MoveManager {
private:
    // Vector containing every move
    static std::vector<MoveBase*> _moves;
    // Map from offset to move
    static CoordTensor<std::vector<MoveBase*>> _movesByOffset;
    // Vector containing only generated moves
    static std::vector<MoveBase*> _movesToFree;
    // Vector containing all move offsets
    static std::vector<std::valarray<int>> _offsets;
public:
    // Never instantiate MoveManager
    MoveManager() = delete;
    MoveManager(const MoveManager&) = delete;

    // Initialize Move Manager
    static void InitMoveManager(int order, int maxDistance);

    // Generate multiple moves from a single move definition
    static void GenerateMovesFrom(MoveBase* origMove);

    // Register a move without generating additional moves
    static void RegisterSingleMove(MoveBase* move);

    static void RegisterAllMoves(const std::string& movePath = "./Moves");

    // Get what moves can be made by a module
    static std::vector<MoveBase*> CheckAllMoves(CoordTensor<int>& tensor, Module& mod);

    // Get a pair containing which module has to make what move in order to reach an adjacent state
    static std::pair<Module*, MoveBase*> FindMoveToState(const CoordTensor<bool>& state);
};

#endif //MODULAR_ROBOTICS_MOVEMANAGER_H
