#include <string>
#include <fstream>
#include "MoveManager.h"

void MoveBase::RotateMove(int index) {
    std::swap(initPos[0], initPos[index]);
    std::swap(finalPos[0], finalPos[index]);
    std::swap(anchorPos[0], anchorPos[index]);
    std::swap(bounds[0], bounds[index]);
    for (auto& move : moves) {
        std::swap(move.first[0], move.first[index]);
    }
}

void MoveBase::ReflectMove(int index) {
    initPos[index] *= -1;
    finalPos[index] *= -1;
    anchorPos[index] *= -1;
    std::swap(bounds[index].first, bounds[index].second);
    for (auto& move : moves) {
        move.first[index] *= -1;
    }
}

const std::valarray<int>& MoveBase::MoveOffset() {
    return finalPos;
}

const std::valarray<int>& MoveBase::AnchorOffset() {
    return anchorPos;
}

Move2d::Move2d() {
    order = 2;
    bounds.resize(order, {0, 0});
}

MoveBase* Move2d::CopyMove() const {
    auto copy = new Move2d();
    *copy = *this;
    return copy;
}

void Move2d::InitMove(std::ifstream &moveFile) {
    int x = 0, y = 0;
    std::valarray<int> maxBounds = {0, 0};
    std::string line;
    while (std::getline(moveFile, line)) {
        for (auto c : line) {
            switch (c) {
                default:
                    DEBUG("Unrecognized Move: " << c << std::endl);
                case Move::NOCHECK:
                    x++;
                    continue;
                case Move::FINAL:
                    finalPos = {x, y};
                case Move::EMPTY:
                    moves.push_back({{x, y}, false});
                    break;
                case Move::ANCHOR:
                    anchorPos = {x, y};
                case Move::STATIC:
                    moves.push_back({{x, y}, true});
                    break;
                case Move::INITIAL:
                    initPos = {x, y};
                    bounds = {{x, 0}, {y, 0}};
                    break;
            }
            if (x > maxBounds[0]) {
                maxBounds[0] = x;
            }
            if (y > maxBounds[1]) {
                maxBounds[1] = y;
            }
            x++;
        }
        x = 0;
        y++;
    }
    for (auto& move : moves) {
        move.first -= initPos;
        DEBUG("Check Offset: " << move.first[0] << ", " << move.first[1] << (move.second ? " Static" : " Empty") << std::endl);
    }
    finalPos -= initPos;
    anchorPos -= initPos;
    DEBUG("Move Offset: " << finalPos[0] << ", " << finalPos[1] << std::endl);
    maxBounds -= initPos;
    bounds[0].second = maxBounds[0];
    bounds[1].second = maxBounds[1];
}

bool Move2d::MoveCheck(CoordTensor<int> &tensor, const Module &mod) {
    // Bounds checking
    for (int i = 0; i < order; i++) {
        if (mod.coords[i] - bounds[i].first < 0 || mod.coords[i] + bounds[i].second >= tensor.AxisSize()) {
            return false;
        }
    }
    // Move Check
    for (const auto& move : moves) {
        if ((tensor[mod.coords + move.first] < 0) == move.second) {
            return false;
        }
    }
    return true;
}

Move3d::Move3d() {
    order = 3;
    bounds.resize(3, {0, 0});
}

MoveBase* Move3d::CopyMove() const {
    auto copy = new Move3d();
    *copy = *this;
    return copy;
}

void Move3d::InitMove(std::ifstream &moveFile) {
    int x = 0, y = 0, z = 0;
    std::valarray<int> maxBounds = {0, 0, 0};
    std::string line;
    while (std::getline(moveFile, line)) {
        if (line.empty()) {
            z++;
            y = 0;
            continue;
        }
        for (auto c : line) {
            switch (c) {
                default:
                    DEBUG("Unrecognized Move: " << c << std::endl);
                case Move::NOCHECK:
                    x++;
                    continue;
                case Move::FINAL:
                    finalPos = {x, y, z};
                case Move::EMPTY:
                    moves.push_back({{x, y, z}, false});
                    break;
                case Move::ANCHOR:
                    anchorPos = {x, y, z};
                case Move::STATIC:
                    moves.push_back({{x, y, z}, true});
                    break;
                case Move::INITIAL:
                    initPos = {x, y, z};
                    bounds = {{x, 0}, {y, 0}, {z, 0}};
                    break;
            }
            if (x > maxBounds[0]) {
                maxBounds[0] = x;
            }
            if (y > maxBounds[1]) {
                maxBounds[1] = y;
            }
            if (z > maxBounds[2]) {
                maxBounds[2] = z;
            }
            x++;
        }
        x = 0;
        y++;
    }
    for (auto& move : moves) {
        move.first -= initPos;
        DEBUG("Check Offset: " << move.first[0] << ", " << move.first[1] << ", " << move.first[2] << (move.second ? " Static" : " Empty") << std::endl);
    }
    finalPos -= initPos;
    anchorPos -= initPos;
    DEBUG("Move Offset: " << finalPos[0] << ", " << finalPos[1] << ", " << finalPos[2] << std::endl);
    maxBounds -= initPos;
    bounds[0].second = maxBounds[0];
    bounds[1].second = maxBounds[1];
    bounds[2].second = maxBounds[2];
}

bool Move3d::MoveCheck(CoordTensor<int> &tensor, const Module &mod) {
    // Bounds checking
    for (int i = 0; i < order; i++) {
        if (mod.coords[i] - bounds[i].first < 0 || mod.coords[i] + bounds[i].second >= tensor.AxisSize()) {
            return false;
        }
    }
    // Move Check
    for (const auto& move : moves) {
        if ((tensor[mod.coords + move.first] < 0) == move.second) {
            return false;
        }
    }
    return true;
}

std::vector<MoveBase*> MoveManager::_moves;
CoordTensor<std::vector<MoveBase*>> MoveManager::_movesByOffset(1, 1, {});
std::vector<MoveBase*> MoveManager::_movesToFree;
std::vector<std::valarray<int>> MoveManager::_offsets;

void MoveManager::InitMoveManager(int order, int maxDistance) {
    _movesByOffset = std::move(CoordTensor<std::vector<MoveBase*>>(order, 2 * maxDistance,
            {}, std::valarray<int>(maxDistance, order)));
}

void MoveManager::GenerateMovesFrom(MoveBase *origMove) {
    std::vector<MoveBase*> movesGen;
    // Add initial move to working vector
    movesGen.push_back(origMove);
    // Add rotations to working vector
    for (int i = 1; i < origMove->order; i++) {
        auto moveRotated = origMove->CopyMove();
        moveRotated->RotateMove(i);
        movesGen.push_back(moveRotated);
        _movesToFree.push_back(moveRotated);
    }
    // Reflections
    for (int i = 0; i < origMove->order; i++) {
        auto movesToReflect = movesGen;
        for (auto move : movesToReflect) {
            auto moveReflected = move->CopyMove();
            moveReflected->ReflectMove(i);
            movesGen.push_back(moveReflected);
            _movesToFree.push_back(moveReflected);
        }
    }
    // Add everything to _moves
    for (auto move : movesGen) {
        _moves.push_back(move);
        if (_movesByOffset[move->finalPos].empty()) {
            _offsets.push_back(move->finalPos);
        }
        _movesByOffset[move->finalPos].push_back(move);
    }
}

void MoveManager::RegisterSingleMove(MoveBase *move) {
    _moves.push_back(move);
}

std::vector<MoveBase*> MoveManager::CheckAllMoves(CoordTensor<int> &tensor, Module &mod) {
    std::vector<MoveBase*> legalMoves = {};
    for (auto move : _moves) {
        if (move->MoveCheck(tensor, mod)) {
            DEBUG("passed!\n");
            legalMoves.push_back(move);
        } else {
            DEBUG("failed!\n");
        }
    }
    return legalMoves;
}

std::pair<Module*, MoveBase*> MoveManager::FindMoveToState(Lattice &lattice, const CoordTensor<bool> &state) {
    Module* modToMove = nullptr;
    // Find module to move
    for (int i = 0; i < lattice.stateTensor.GetArrayInternal().size(); i++) {
        if (lattice.stateTensor.GetElementDirect(i) != state.GetElementDirect(i) && !state.GetElementDirect(i)) {
            modToMove = &ModuleIdManager::Modules()[lattice.coordTensor.GetElementDirect(i)];
            break;
        }
    }
    if (modToMove == nullptr) {
        return {nullptr, nullptr};
    }
    auto& modCoords = modToMove->coords;
    for (auto& offset : _offsets) {
        // Find offset to move to
        if (!state[modCoords + offset]) continue;
        // Find move to get there
        for (auto move : _movesByOffset[offset]) {
            if (move->MoveCheck(lattice.coordTensor, *modToMove)) {
                return {modToMove, move};
            }
        }
    }
    return {modToMove, nullptr};
}

void MoveManager::CleanMoves() {
    for (auto move : _movesToFree) {
        delete move;
    }
}