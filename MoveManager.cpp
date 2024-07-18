#include <string>
#include <fstream>
#include <filesystem>
#include "MoveManager.h"

void MoveBase::RotateMove(int index) {
    std::swap(initPos[0], initPos[index]);
    std::swap(finalPos[0], finalPos[index]);
    std::swap(bounds[0], bounds[index]);
    for (auto& move : moves) {
        std::swap(move.first[0], move.first[index]);
    }
    for (auto& anim : animSequence) {
        anim.first = Move::AnimRotationMap.at(anim.first)[index];
        std::swap(anim.second[0], anim.second[index]);
    }
}

void MoveBase::ReflectMove(int index) {
    initPos[index] *= -1;
    finalPos[index] *= -1;
    std::swap(bounds[index].first, bounds[index].second);
    for (auto& move : moves) {
        move.first[index] *= -1;
    }
    for (auto& anim : animSequence) {
        anim.first = Move::AnimReflectionMap.at(anim.first)[index];
        anim.second[index] *= -1;
    }
}

const std::valarray<int>& MoveBase::MoveOffset() const {
    return finalPos;
}

const std::vector<std::pair<Move::AnimType, std::valarray<int>>>& MoveBase::AnimSequence() const {
    return animSequence;
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

void Move2d::InitMove(const nlohmann::basic_json<>& moveDef) {
    int x = 0, y = 0;
    std::valarray<int> maxBounds = {0, 0};
    for (const std::string line : moveDef["def"][0]) {
        for (char c : line) {
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
    DEBUG("Move Offset: " << finalPos[0] << ", " << finalPos[1] << std::endl);
    maxBounds -= initPos;
    bounds[0].second = maxBounds[0];
    bounds[1].second = maxBounds[1];
    // Set up animation data
    for (const auto& animDef : moveDef["animSeq"]) {
        Move::AnimType animType = Move::StrAnimMap.at(animDef[0]);
        std::valarray<int> animOffset = animDef[1];
        animSequence.emplace_back(animType, animOffset);
    }
    // Generate move permutations if necessary (it pretty much always is)
    if (moveDef["permGen"] == true) {
        MoveManager::GenerateMovesFrom(this);
    } else {
        MoveManager::RegisterSingleMove(this);
    }
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

void Move3d::InitMove(const nlohmann::basic_json<>& moveDef) {
    int x = 0, y = 0, z = 0;
    std::valarray<int> maxBounds = {0, 0, 0};
    for (const std::vector<std::string> slice : moveDef["def"]) {
        for (const auto& line : slice) {
            for (auto c: line) {
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
                    case Move::STATIC:
                        moves.push_back({{x, y, z}, true});
                        break;
                    case Move::INITIAL:
                        initPos = {x, y, z};
                        bounds = {{x, 0},
                                  {y, 0},
                                  {z, 0}};
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
        y = 0;
        z++;
    }
    for (auto& move : moves) {
        move.first -= initPos;
        DEBUG("Check Offset: " << move.first[0] << ", " << move.first[1] << ", " << move.first[2] << (move.second ? " Static" : " Empty") << std::endl);
    }
    finalPos -= initPos;
    DEBUG("Move Offset: " << finalPos[0] << ", " << finalPos[1] << ", " << finalPos[2] << std::endl);
    maxBounds -= initPos;
    bounds[0].second = maxBounds[0];
    bounds[1].second = maxBounds[1];
    bounds[2].second = maxBounds[2];
    // Set up animation data
    for (const auto& animDef : moveDef["animSeq"]) {
        Move::AnimType animType = Move::StrAnimMap.at(animDef[0]);
        std::valarray<int> animOffset = animDef[1];
        animSequence.emplace_back(animType, animOffset);
    }
    // Generate move permutations if necessary (it pretty much always is)
    if (moveDef["permGen"] == true) {
        MoveManager::GenerateMovesFrom(this);
    } else {
        MoveManager::RegisterSingleMove(this);
    }
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

void MoveManager::RegisterAllMoves(const std::string& movePath) {
    nlohmann::json moveJson;
    for (const auto& moveFile : std::filesystem::recursive_directory_iterator(movePath)) {
        std::ifstream(moveFile.path()) >> moveJson;
        for (const auto& moveDef : moveJson["moves"]) {
            if (moveDef["order"] == 2) {
                DEBUG("Registering 2d move " << moveDef["name"] << std::endl);
                auto move = new Move2d();
                move->InitMove(moveDef);
                _movesToFree.push_back(move);
            } else if (moveDef["order"] == 3) {
                DEBUG("Registering 3d move " << moveDef["name"] << std::endl);
                auto move = new Move3d();
                move->InitMove(moveDef);
                _movesToFree.push_back(move);
            } else {
                // Not currently supported
                std::cout << "Attempted to create move of order != 2 or 3, moveDef at: " << moveFile.path() << std::endl;
            }
        }
        // might need to close the ifstream idk yet
    }
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