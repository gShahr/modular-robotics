#include <string>
#include <fstream>
#include <filesystem>
#include <execution>
#include "MoveManager.h"

void Move::RotateAnim(Move::AnimType& anim, const int a, const int b) {
    // For easily rotating move types
    static std::unordered_map<AnimType, std::vector<int>> AnimToOffset = {
            {Z_SLIDE, {0, 0, 1}},
            {Y_SLIDE, {0, 1, 0}},
            {X_SLIDE, {1, 0, 0}},
            {GEN_SLIDE, {0, 0, 0}},
            {PIVOT_PX, { 1,  0,  0}},
            {PIVOT_PY, { 0,  1,  0}},
            {PIVOT_PZ, { 0,  0,  1}},
            {PIVOT_NX, {-1,  0,  0}},
            {PIVOT_NY, { 0, -1,  0}},
            {PIVOT_NZ, { 0,  0, -1}},
            {RD_PXPY, { 1,  1,  0}},
            {RD_PXNY, { 1, -1,  0}},
            {RD_NXPY, {-1,  1,  0}},
            {RD_NXNY, {-1, -1,  0}},
            {RD_PXPZ, { 1,  0,  1}},
            {RD_PXNZ, { 1,  0, -1}},
            {RD_NXPZ, {-1,  0,  1}},
            {RD_NXNZ, {-1,  0, -1}},
            {RD_PYPZ, { 0,  1,  1}},
            {RD_PYNZ, { 0,  1, -1}},
            {RD_NYPZ, { 0, -1,  1}},
            {RD_NYNZ, { 0, -1, -1}}
    };

    static std::map<std::vector<int>, AnimType> OffsetToSlideAnim = {
            {{0, 0, 1}, Z_SLIDE},
            {{0, 1, 0}, Y_SLIDE},
            {{1, 0, 0}, X_SLIDE},
            {{0, 0, 0}, GEN_SLIDE}
    };

    static std::map<std::vector<int>, AnimType> OffsetToAnim = {
            {{ 1,  0,  0}, PIVOT_PX},
            {{ 0,  1,  0}, PIVOT_PY},
            {{ 0,  0,  1}, PIVOT_PZ},
            {{-1,  0,  0}, PIVOT_NX},
            {{ 0, -1,  0}, PIVOT_NY},
            {{ 0,  0, -1}, PIVOT_NZ},
            {{ 1,  1,  0}, RD_PXPY},
            {{ 1, -1,  0}, RD_PXNY},
            {{-1,  1,  0}, RD_NXPY},
            {{-1, -1,  0}, RD_NXNY},
            {{ 1,  0,  1}, RD_PXPZ},
            {{ 1,  0, -1}, RD_PXNZ},
            {{-1,  0,  1}, RD_NXPZ},
            {{-1,  0, -1}, RD_NXNZ},
            {{ 0,  1,  1}, RD_PYPZ},
            {{ 0,  1, -1}, RD_PYNZ},
            {{ 0, -1,  1}, RD_NYPZ},
            {{ 0, -1, -1}, RD_NYNZ}
    };

    auto offset = AnimToOffset[anim];
    std::swap(offset[a], offset[b]);
    if (anim > GEN_SLIDE) {
        anim = OffsetToAnim[offset];
    } else {
        anim = OffsetToSlideAnim[offset];
    }
}

bool MoveBase::FreeSpaceCheck(const CoordTensor<int> &tensor, const std::valarray<int> &coords) {
    return std::all_of(std::execution::par_unseq, moves.begin(), moves.end(), [&coords = std::as_const(coords), &tensor = std::as_const(tensor)](auto& move) {
        if ( !move.second && (tensor[coords + move.first] > FREE_SPACE)) {
            return false;
        }
        return true;
    });
}

void MoveBase::Rotate(const int a, const int b) {
    std::swap(initPos[a], initPos[b]);
    std::swap(finalPos[a], finalPos[b]);
    std::swap(bounds[a], bounds[b]);
    for (auto&[offset, check] : moves) {
        std::swap(offset[a], offset[b]);
    }
    for (auto&[type, offset] : animSequence) {
        std::swap(offset[a], offset[b]);
        Move::RotateAnim(type, a, b);
    }
}

void MoveBase::Reflect(const int index) {
    initPos[index] *= -1;
    finalPos[index] *= -1;
    std::swap(bounds[index].first, bounds[index].second);
    for (auto&[offset, check] : moves) {
        offset[index] *= -1;
    }
    for (auto&[type, offset] : animSequence) {
        type = Move::AnimReflectionMap.at(type)[index];
        offset[index] *= -1;
    }
}

const std::valarray<int>& MoveBase::MoveOffset() const {
    return finalPos;
}

const std::vector<std::pair<Move::AnimType, std::valarray<int>>>& MoveBase::AnimSequence() const {
    return animSequence;
}

bool MoveBase::operator==(const MoveBase &rhs) const {
    std::valarray valArrComparison = finalPos == rhs.finalPos;
    for (const auto result : valArrComparison) {
        if (!result) {
            return false;
        }
    }
    if (moves.size() != rhs.moves.size()) {
        return false;
    }
    for (auto it = moves.begin(), it2 = rhs.moves.begin(); it != moves.end(); ++it, ++it2) {
        valArrComparison = it->first == it2->first;
        for (const auto result : valArrComparison) {
            if (!result) {
                return false;
            }
        }
        if (it->second != it->second) {
            return false;
        }
    }
    return true;
}


Move2d::Move2d() {
    order = 2;
    bounds.resize(order, {0, 0});
}

MoveBase* Move2d::MakeCopy() const {
    const auto copy = new Move2d();
    *copy = *this;
    return copy;
}

void Move2d::InitMove(const nlohmann::basic_json<>& moveDef) {
    int x = 0, y = 0;
    std::valarray<int> maxBounds = {0, 0};
    for (const std::string line : moveDef["def"][0]) {
        for (const char c : line) {
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
    for (auto&[offset, check] : moves) {
        offset -= initPos;
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
        DEBUG("Check Offset: " << offset[0] << ", " << offset[1] << (check ? " Static" : " Empty") << std::endl);
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

bool Move2d::MoveCheck(const CoordTensor<int>& tensor, const Module& mod) {
    // Bounds checking
#if MOVEMANAGER_BOUNDS_CHECKS
    for (int i = 0; i < order; i++) {
        if (mod.coords[i] - bounds[i].first < 0 || mod.coords[i] + bounds[i].second >= Lattice::AxisSize()) {
            return false;
        }
    }
#endif
    // Move Check
    return std::all_of(std::execution::par_unseq, moves.begin(), moves.end(), [&mod = std::as_const(mod), &tensor = std::as_const(tensor)](auto& move) {
        if ((tensor[mod.coords + move.first] < 0) == move.second) {
            return false;
        }
        return true;
    });
}

Move3d::Move3d() {
    order = 3;
    bounds.resize(3, {0, 0});
}

MoveBase* Move3d::MakeCopy() const {
    const auto copy = new Move3d();
    *copy = *this;
    return copy;
}

void Move3d::InitMove(const nlohmann::basic_json<>& moveDef) {
    int x = 0, y = 0, z = 0;
    std::valarray<int> maxBounds = {0, 0, 0};
    for (const std::vector<std::string> slice : moveDef["def"]) {
        for (const auto& line : slice) {
            for (const auto c: line) {
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
    for (auto&[offset, check] : moves) {
        offset -= initPos;
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
        DEBUG("Check Offset: " << offset[0] << ", " << offset[1] << ", " << offset[2] << (check ? " Static" : " Empty") << std::endl);
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

bool Move3d::MoveCheck(const CoordTensor<int> &tensor, const Module &mod) {
    // Bounds checking
#if MOVEMANAGER_BOUNDS_CHECKS
    for (int i = 0; i < order; i++) {
        if (mod.coords[i] - bounds[i].first < 0 || mod.coords[i] + bounds[i].second >= tensor.AxisSize()) {
            return false;
        }
    }
#endif
    // Move Check
    return std::all_of(std::execution::par_unseq, moves.begin(), moves.end(), [&mod = std::as_const(mod), &tensor = std::as_const(tensor)](auto& move) {
        if ((tensor[mod.coords + move.first] < 0) == move.second) {
            return false;
        }
        return true;
    });
}

std::vector<MoveBase*> MoveManager::_moves;
CoordTensor<std::vector<MoveBase*>> MoveManager::_movesByOffset(1, 1, {});
std::vector<std::valarray<int>> MoveManager::_offsets;

void MoveManager::InitMoveManager(const int order, const int maxDistance) {
    _movesByOffset = std::move(CoordTensor<std::vector<MoveBase*>>(order, 2 * maxDistance,
            {}, std::valarray<int>(maxDistance, order)));
}

void MoveManager::GenerateMovesFrom(MoveBase* origMove) {
    auto list = Isometry::GenerateTransforms(origMove);
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
    DEBUG("Generated " << list.size() << " moves from initial definition." << std::endl);
    int dupesAvoided = 0;
#endif
    for (const auto move: list) {
        if (std::none_of(_moves.begin(), _moves.end(), [&move](auto& existingMove) {
            return *existingMove == *dynamic_cast<MoveBase*>(move);
        })) {
            _moves.push_back(dynamic_cast<MoveBase*>(move));
        } else {
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
            dupesAvoided++;
#endif
        }
    }
#if MOVEMANAGER_VERBOSE > MM_LOG_NONE
    DEBUG("Registered " << list.size() - dupesAvoided << '/' << list.size() << " generated moves." << std::endl);
    DEBUG("Duplicate moves avoided: " << dupesAvoided << std::endl);
#endif
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
                const auto move = new Move2d();
                Isometry::transformsToFree.push_back(move);
                move->InitMove(moveDef);
            } else if (Lattice::order == 3) {
                const auto move = new Move3d();
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
        if (const auto id = Lattice::coordTensor[mod.coords + moveOffset]; id == OUT_OF_BOUNDS || id >= 0) continue;
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

#define MOVEMANAGER_CHECK_BY_OFFSET true
std::vector<MoveBase*> MoveManager::CheckAllMovesAndConnectivity(CoordTensor<int> &tensor, Module &mod) {
    std::vector<MoveBase*> legalMoves = {};
#if MOVEMANAGER_CHECK_BY_OFFSET
    for (const auto& moveOffset : _offsets) {
        for (auto move : _movesByOffset[moveOffset]) {
            if (move->MoveCheck(tensor, mod) && checkConnected(tensor, mod, move)) {
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

bool MoveManager::checkConnected(const CoordTensor<int>& tensor, const Module& mod, const MoveBase* move) {
    return false;
}

std::pair<Module*, MoveBase*> MoveManager::FindMoveToState(const std::set<ModuleData>& modData) {
    Module* modToMove = nullptr;
    std::valarray<int> destination;
    std::unordered_set<int> candidates;
    for (int id = 0; id < ModuleIdManager::MinStaticID(); id++) {
        candidates.insert(id);
    }
    for (const auto& info : modData) {
        if (auto id = Lattice::coordTensor[info.Coords()]; id >= 0) {
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
    const auto offset = destination - modToMove->coords;
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