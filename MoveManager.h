#include <vector>
#include <valarray>
#include "Lattice.h"

#ifndef MODULAR_ROBOTICS_MOVEMANAGER_H
#define MODULAR_ROBOTICS_MOVEMANAGER_H

namespace Move {
    enum State {
        NOCHECK = ' ',
        EMPTY = 'x',
        INITIAL = '?',
        FINAL = '!',
        STATIC = '#',
        ANCHOR = '@'
    };
}

class MoveBase {
protected:
    // each pair represents a coordinate offset to check and whether a module should be there or not
    std::vector<std::pair<std::valarray<int>, bool>> moves;
    // bounds ex: {(2, 1), (0, 1)} would mean bounds extend from -2 to 1 on x-axis and 0 to 1 on y-axis
    std::vector<std::pair<int, int>> bounds;
    std::valarray<int> initPos, finalPos, anchorPos;
    int order = -1;
public:
    // Create a copy of a move
    [[nodiscard]]
    virtual MoveBase* CopyMove() const = 0;
    // Load in move info from a given file
    virtual void InitMove(std::ifstream& moveFile) = 0;
    // Check to see if move is possible for a given module
    virtual bool MoveCheck(CoordTensor<int>& tensor, const Module& mod) = 0;

    void RotateMove(int index);

    void ReflectMove(int index);

    const std::valarray<int>& MoveOffset();

    const std::valarray<int>& AnchorOffset();

    virtual ~MoveBase() = default;

    friend class MoveManager;
};

class Move2d : public MoveBase {
public:
    Move2d();
    MoveBase* CopyMove() const override;
    void InitMove(std::ifstream& moveFile) override;
    bool MoveCheck(CoordTensor<int>& tensor, const Module& mod) override;
};

class Move3d : public MoveBase {
public:
    Move3d();
    MoveBase* CopyMove() const override;
    void InitMove(std::ifstream& moveFile) override;
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

    // Get what moves can be made by a module
    static std::vector<MoveBase*> CheckAllMoves(CoordTensor<int>& tensor, Module& mod);

    // Get a pair containing which module has to make what move in order to reach an adjacent state
    static std::pair<Module*, MoveBase*> FindMoveToState(Lattice& lattice, const CoordTensor<bool>& state);

    // Clean up generated moves
    static void CleanMoves();
};

#endif //MODULAR_ROBOTICS_MOVEMANAGER_H
