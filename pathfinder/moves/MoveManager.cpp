#include <string>
#include <fstream>
#include <filesystem>
#include "MoveManager.h"

void Move::RotateAnim(Move::AnimType& anim, int a, int b) {
    // For easily rotating move types
    static std::unordered_map<AnimType, std::vector<int>> AnimToOffset = {
            {Z_SLIDE, {0, 0, 1}},
            {Y_SLIDE, {0, 1, 0}},
            {X_SLIDE, {1, 0, 0}},
            {GEN_SLIDE, {0, 0, 0}},
            {PIVOT_PX, {1, 0, 0}},
            {PIVOT_PY, {0, 1, 0}},
            {PIVOT_PZ, {0, 0, 1}},
            {PIVOT_NX, {-1, 0, 0}},
            {PIVOT_NY, {0, -1, 0}},
            {PIVOT_NZ, {0, 0, -1}}
    };

    static std::map<std::vector<int>, AnimType> OffsetToSlideAnim = {
            {{0, 0, 1}, Z_SLIDE},
            {{0, 1, 0}, Y_SLIDE},
            {{1, 0, 0}, X_SLIDE},
            {{0, 0, 0}, GEN_SLIDE}
    };

    static std::map<std::vector<int>, AnimType> OffsetToAnim = {
            {{1, 0, 0}, PIVOT_PX},
            {{0, 1, 0}, PIVOT_PY},
            {{0, 0, 1}, PIVOT_PZ},
            {{-1, 0, 0}, PIVOT_NX},
            {{0, -1, 0}, PIVOT_NY},
            {{0, 0, -1}, PIVOT_NZ}
    };

    auto offset = AnimToOffset[anim];
    std::swap(offset[a], offset[b]);
    if (anim > GEN_SLIDE) {
        anim = OffsetToAnim[offset];
    } else {
        anim = OffsetToSlideAnim[offset];
    }
}

void MoveBase::Rotate(int a, int b) {
    std::swap(initPos[a], initPos[b]);
    std::swap(finalPos[a], finalPos[b]);
    std::swap(bounds[a], bounds[b]);
    for (auto& move : moves) {
        std::swap(move.first[a], move.first[b]);
    }
    for (auto& anim : animSequence) {
        std::swap(anim.second[a], anim.second[b]);
        Move::RotateAnim(anim.first, a, b);
    }
}

void MoveBase::Reflect(int index) {
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

MoveBase* Move2d::MakeCopy() const {
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
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
        DEBUG("Check Offset: " << move.first[0] << ", " << move.first[1] << (move.second ? " Static" : " Empty") << std::endl);
#endif
    }
    finalPos -= initPos;
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
    DEBUG("Move Offset: " << finalPos[0] << ", " << finalPos[1] << std::endl);
#endif
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

MoveBase* Move3d::MakeCopy() const {
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
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
        DEBUG("Check Offset: " << move.first[0] << ", " << move.first[1] << ", " << move.first[2] << (move.second ? " Static" : " Empty") << std::endl);
#endif
    }
    finalPos -= initPos;
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
    DEBUG("Move Offset: " << finalPos[0] << ", " << finalPos[1] << ", " << finalPos[2] << std::endl);
#endif
    maxBounds -= initPos;
    bounds[0].second = maxBounds[0];
    bounds[1].second = maxBounds[1];
    bounds[2].second = maxBounds[2];
    // Set up animation data
    if (moveDef.contains("animSeq") == true) {
        for (const auto& animDef : moveDef["animSeq"]) {
            Move::AnimType animType = Move::StrAnimMap.at(animDef[0]);
            std::valarray<int> animOffset = animDef[1];
            animSequence.emplace_back(animType, animOffset);
        }
    }
    // Generate move permutations if necessary (it pretty much always is)
    if (moveDef.contains("PermGen") == false || moveDef["permGen"] == true) {
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
std::vector<std::valarray<int>> MoveManager::_offsets;

void MoveManager::InitMoveManager(int order, int maxDistance) {
    _movesByOffset = std::move(CoordTensor<std::vector<MoveBase*>>(order, 2 * maxDistance,
            {}, std::valarray<int>(maxDistance, order)));
}

void MoveManager::GenerateMovesFrom(MoveBase* origMove) {
    auto list = Isometry::GenerateTransforms(origMove);
    for (auto move: list) {
        _moves.push_back(dynamic_cast<MoveBase*>(move));
    }
    // Add move to offset map
    for (auto move : _moves) {
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
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
                DEBUG("Registering 2d move in " << Lattice::order << "d space: " << moveDef["name"] << std::endl);
#endif
                //auto move = new Move2d();
                //Isometry::transformsToFree.push_back(move);
                //move->InitMove(moveDef);
            } else if (moveDef["order"] == 3) {
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
                DEBUG("Registering 3d move " << Lattice::order << "d space: " << moveDef["name"] << std::endl);
#endif
                //auto move = new Move3d();
                //Isometry::transformsToFree.push_back(move);
                //move->InitMove(moveDef);
            } else {
                // Not currently supported
                std::cout << "Attempted to create move of order != 2 or 3, moveDef at: " << moveFile.path() << std::endl;
            }
            if (Lattice::order == 2) {
                auto move = new Move2d();
                Isometry::transformsToFree.push_back(move);
                move->InitMove(moveDef);
            } else if (Lattice::order == 3) {
                auto move = new Move3d();
                Isometry::transformsToFree.push_back(move);
                move->InitMove(moveDef);
            }
        }
        // might need to close the ifstream idk yet
    }
}

#define MOVEMANAGER_CHECK_BY_OFFSET true
std::vector<MoveBase*> MoveManager::CheckAllMoves(CoordTensor<int> &tensor, Module &mod) {
    std::vector<MoveBase*> legalMoves = {};
#if MOVEMANAGER_CHECK_BY_OFFSET
    for (const auto& moveOffset : _offsets) {
        for (auto move : _movesByOffset[moveOffset]) {
            if (move->MoveCheck(tensor, mod)) {
#if MOVEMANAGER_VERBOSE == MM_LOG_MOVE_CHECKS
                DEBUG("passed!\n");
#endif
                legalMoves.push_back(move);
                break;
#if MOVEMANAGER_VERBOSE == MM_LOG_MOVE_CHECKS
            } else {
                DEBUG("failed!\n");
#endif
            }
        }
    }
#else
    for (auto move : _moves) {
        if (move->MoveCheck(tensor, mod)) {
#if MOVEMANAGER_VERBOSE == MM_LOG_MOVE_CHECKS
            DEBUG("passed!\n");
#endif
            legalMoves.push_back(move);
#if MOVEMANAGER_VERBOSE == MM_LOG_MOVE_CHECKS
        } else {
            DEBUG("failed!\n");
#endif
        }
    }
#endif
    return legalMoves;
}

std::pair<Module*, MoveBase*> MoveManager::FindMoveToState(const std::set<ModuleData>& modData) {
    Module* modToMove = nullptr;
    std::valarray<int> destination;
    std::unordered_set<int> candidates;
    for (int id = 0; id < ModuleIdManager::MinStaticID(); id++) {
        candidates.insert(id);
    }
    for (const auto& info : modData) {
        auto id = Lattice::coordTensor[info.Coords()];
        if (id >= 0) {
            candidates.erase(id);
        } else {
            destination = info.Coords();
        }
    }
    if (candidates.size() != 1) {
        return {nullptr, nullptr};
    }
    modToMove = &ModuleIdManager::GetModule(*candidates.begin());
    if (modToMove == nullptr) {
        return {nullptr, nullptr};
    }
    auto offset = destination - modToMove->coords;
    for (auto move : _movesByOffset[offset]) {
        if (move->MoveCheck(Lattice::coordTensor, *modToMove)) {
            return {modToMove, move};
        }
    }
    return {modToMove, nullptr};
    // Find module to move
    /*for (size_t i = 0; i < Lattice::stateTensor.GetArrayInternal().size(); i++) {
        if (Lattice::stateTensor.GetElementDirect(i) != state.GetElementDirect(i) && !state.GetElementDirect(i)) {
            modToMove = &ModuleIdManager::Modules()[Lattice::coordTensor.GetElementDirect(i)];
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
            if (move->MoveCheck(Lattice::coordTensor, *modToMove)) {
                return {modToMove, move};
            }
        }
    }
    return {modToMove, nullptr};*/
}