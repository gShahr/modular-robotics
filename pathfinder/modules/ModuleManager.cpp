#include <set>
#include "../lattice/Lattice.h"
#include "../coordtensor/debug_util.h"
#include "ModuleManager.h"

IModuleProperty* PropertyInitializer::GetProperty(const nlohmann::basic_json<> &propertyDef) {
    return ModuleProperties::Constructors()[propertyDef["name"]](propertyDef);
}

std::vector<std::string>& ModuleProperties::PropertyKeys() {
    static std::vector<std::string> _propertyKeys = {};
    return _propertyKeys;
}

std::unordered_map<std::string, IModuleProperty* (*)(const nlohmann::basic_json<>& propertyDef)>& ModuleProperties::Constructors() {
    static std::unordered_map<std::string, IModuleProperty* (*)(const nlohmann::basic_json<>& propertyDef)> _constructors;
    return _constructors;
}

ModuleProperties::ModuleProperties(const ModuleProperties& other) {
    _properties.clear();
    for (const auto& property : other._properties) {
        _properties.insert(property->MakeCopy());
    }
    _dynamicProperties.clear();
    if (other._dynamicProperties.empty()) {
        return;
    }
    for (const auto& dynamicProperty : other._dynamicProperties) {
        _dynamicProperties.insert(dynamicProperty->MakeCopy());
    }
}

void ModuleProperties::InitProperties(const nlohmann::basic_json<> &propertyDefs) {
    for (const auto& key : PropertyKeys()) {
        if (propertyDefs.contains(key)) {
            auto property = Constructors()[key](propertyDefs[key]);
            _properties.insert(property);
            if (propertyDefs[key].contains("static") && propertyDefs[key]["static"] == false) {
                auto dynamicProperty = dynamic_cast<IModuleDynamicProperty*>(property);
                if (dynamicProperty == nullptr) {
                    std::cerr << "Property definition for " << key
                    << " is marked as non-static but implementation class does not inherit from IModuleDynamicProperty."
                    << std::endl;
                } else {
                    _dynamicProperties.insert(dynamicProperty);
                }
            }
        }
    }
}

void ModuleProperties::UpdateProperties(const std::valarray<int>& moveInfo) const {
    for (const auto property : _dynamicProperties) {
        property->UpdateProperty(moveInfo);
    }
}

bool ModuleProperties::operator==(const ModuleProperties& right) const {
    if (_properties.size() != right._properties.size()) {
        return false;
    }

    for (const auto rProp : right._properties) {
        if (const auto lProp = Find(rProp->key); lProp == nullptr || !lProp->CompareProperty(*rProp)) {
            return false;
        }
    }

    return true;
}

bool ModuleProperties::operator!=(const ModuleProperties& right) const {
    if (_properties.size() != right._properties.size()) {
        return true;
    }

    for (const auto rProp : right._properties) {
        if (const auto lProp = Find(rProp->key); lProp == nullptr || !lProp->CompareProperty(*rProp)) {
            return true;
        }
    }

    return false;
}

ModuleProperties& ModuleProperties::operator=(const ModuleProperties& right) {
    _properties.clear();
    for (const auto property : right._properties) {
        _properties.insert(property->MakeCopy());
    }
    _dynamicProperties.clear();
    if (right._dynamicProperties.empty()) {
        return *this;
    }
    for (const auto dynamicProperty : right._dynamicProperties) {
        _dynamicProperties.insert(dynamicProperty->MakeCopy());
    }
    return *this;
}

IModuleProperty* ModuleProperties::Find(const std::string& key) const {
    for (const auto property : _properties) {
        if (property->key == key) {
            return property;
        }
    }
    return nullptr;
}

std::uint_fast64_t ModuleProperties::AsInt() const {
    if (_properties.empty()) {
        return 0;
    }
    if (_properties.size() == 1) {
        return (*_properties.begin())->AsInt();
    }
    std::cerr << "Representing multiple properties as an integer is not supported." << std::endl;
    exit(1);
}

ModuleProperties::~ModuleProperties() {
    for (const auto property : _properties) {
        delete property;
    }
}

PropertyInitializer::PropertyInitializer(const std::string& name, IModuleProperty* (*constructor)(const nlohmann::basic_json<>&)) {
    DEBUG("Adding " << name << " constructor to property constructor map." << std::endl);
    ModuleProperties::PropertyKeys().push_back(name);
    ModuleProperties::Constructors()[name] = constructor;
}

ModuleBasic::ModuleBasic(const std::valarray<int>& coords, const ModuleProperties& properties) : coords(coords), properties(properties) {
    constexpr std::hash<ModuleBasic> hasher;
    hasher(*this);
}

const std::valarray<int>& ModuleBasic::Coords() const {
    return coords;
}

const ModuleProperties& ModuleBasic::Properties() const {
    return properties;
}

bool ModuleBasic::operator==(const IModuleBasic& right) const {
    const auto r = reinterpret_cast<const ModuleBasic&>(right);
    if (coords.size() != r.coords.size()) {
        return false;
    }
    for (int i = 0; i < coords.size(); i++) {
        if (coords[i] != r.coords[i]) {
            return false;
        }
    }
    return properties == r.properties;
}

bool ModuleBasic::operator<(const IModuleBasic& right) const {
    const auto r = reinterpret_cast<const ModuleBasic&>(right);
    return hash < r.hash;
}

std::unordered_map<std::uint_fast64_t, ModuleProperties> ModuleInt64::propertyMap;

ModuleInt64::ModuleInt64(const std::valarray<int> &coords, const ModuleProperties &properties) {
    constexpr std::uint_fast64_t propertyMask = 0xFFFFFFFFFF000000;
    modInt = 0;
    for (int i = 0; i < coords.size(); i++) {
        modInt += coords[i] << (i * 8);
    }
    modInt += properties.AsInt() << 24;
    if (propertyMap.count(modInt & propertyMask) == 0) {
        propertyMap[modInt & propertyMask] = properties;
    }
}

const std::valarray<int>& ModuleInt64::Coords() const {
    constexpr std::uint_fast64_t coordMask = 0xFF;
    static std::valarray<int> result(0, Lattice::Order());
    for (int i = 0; i < result.size(); i++) {
        result[i] = (modInt >> (i * 8)) & coordMask; // NOLINT(*-narrowing-conversions) (Mask should handle it)
    }
    return result;
}

const ModuleProperties& ModuleInt64::Properties() const {
    constexpr std::uint_fast64_t propertyMask = 0xFFFFFFFFFF000000;
    return propertyMap[modInt & propertyMask];
}

bool ModuleInt64::operator==(const IModuleBasic& right) const {
    const auto r = reinterpret_cast<const ModuleInt64&>(right);
    return modInt == r.modInt;
}

bool ModuleInt64::operator<(const IModuleBasic& right) const {
    const auto r = reinterpret_cast<const ModuleInt64&>(right);
    return modInt < r.modInt;
}

ModuleData::ModuleData(const ModuleData &modData) {
#if CONFIG_MOD_DATA_STORAGE == MM_DATA_FULL
    module = std::make_unique<ModuleBasic>(modData.Coords(), modData.Properties());
#else
    module = std::make_unique<ModuleInt64>(modData.Coords(), modData.Properties());
#endif
}


ModuleData::ModuleData(const std::valarray<int> &coords, const ModuleProperties &properties) {
#if CONFIG_MOD_DATA_STORAGE == MM_DATA_FULL
    module = std::make_unique<ModuleBasic>(coords, properties);
#else
    module = std::make_unique<ModuleInt64>(coords, properties);
#endif
}

const std::valarray<int>& ModuleData::Coords() const {
    return module->Coords();
}

const ModuleProperties& ModuleData::Properties() const {
    return module->Properties();
}

bool ModuleData::operator==(const IModuleBasic& right) const {
    return *module == *reinterpret_cast<const ModuleData&>(right).module;
}

bool ModuleData::operator<(const IModuleBasic& right) const {
    return *module < *reinterpret_cast<const ModuleData&>(right).module;
}

std::size_t std::hash<ModuleData>::operator()(const ModuleData& modData) const noexcept {
#if CONFIG_MOD_DATA_STORAGE == MM_DATA_FULL
    auto m = reinterpret_cast<const ModuleBasic&>(*modData.module);
    constexpr std::hash<ModuleBasic> hasher;
    return hasher(m);
#else
    auto m = reinterpret_cast<const ModuleInt64&>(*modData.module);
    return m.modInt;
#endif
}

std::size_t boost::hash<ModuleData>::operator()(const ModuleData &modData) const noexcept {
    constexpr std::hash<ModuleData> hasher;
    return hasher(modData);
}


std::size_t std::hash<ModuleBasic>::operator()(ModuleBasic& modData) const noexcept {
    if (!modData.hashCacheValid) {
        constexpr boost::hash<ModuleBasic> hasher;
        modData.hash = hasher(modData);
        modData.hashCacheValid = true;
    }
    return modData.hash;
}

std::size_t boost::hash<ModuleBasic>::operator()(const ModuleBasic& modData) const noexcept {
    auto coordHash = boost::hash_range(begin(modData.Coords()), end(modData.Coords()));
    if (!Lattice::ignoreProperties) {
        constexpr boost::hash<ModuleProperties> propertyHasher;
        const auto propertyHash = propertyHasher(modData.Properties());
        boost::hash_combine(coordHash, propertyHash);
    }
    return coordHash;
}

std::size_t boost::hash<ModuleProperties>::operator()(const ModuleProperties& moduleProperties) const noexcept {
    //std::size_t prev = 0;
    auto cmp = [](int a, int b) { return a < b; };
    std::set<std::size_t, decltype(cmp)> hashes(cmp);
    for (const auto property : moduleProperties._properties) {
        //auto current = property->GetHash();
        hashes.insert(property->GetHash());
        //boost::hash_combine(prev, current);
    }
    //return prev;
    return boost::hash_range(hashes.begin(), hashes.end());
}

Module::Module(Module&& mod) noexcept {
    coords = mod.coords;
    moduleStatic = mod.moduleStatic;
    properties = mod.properties;
    id = mod.id;
}

Module::Module(const std::valarray<int>& coords, bool isStatic, const nlohmann::basic_json<>& propertyDefs) : coords(coords), moduleStatic(isStatic), id(ModuleIdManager::GetNextId()) {
    properties.InitProperties(propertyDefs);
}

int ModuleIdManager::_nextId = 0;
std::vector<DeferredModCnstrArgs> ModuleIdManager::_deferredInitMods;
std::vector<Module> ModuleIdManager::_modules;
int ModuleIdManager::_staticStart = 0;

void ModuleIdManager::RegisterModule(const std::valarray<int>& coords, bool isStatic, const nlohmann::basic_json<>& propertyDefs, bool deferred) {
    if (!deferred && isStatic) {
        DeferredModCnstrArgs args;
        args.coords = coords;
        args.isStatic = isStatic;
        args.propertyDefs = propertyDefs;
        _deferredInitMods.emplace_back(args);
    } else {
        _modules.emplace_back(coords, isStatic, propertyDefs);
        if (!isStatic) {
            _staticStart++;
        }
    }
}

void ModuleIdManager::DeferredRegistration() {
    for (const auto& args : _deferredInitMods) {
        RegisterModule(args.coords, args.isStatic, args.propertyDefs, true);
    }
}

int ModuleIdManager::GetNextId() {
    return _nextId++;
}

std::vector<Module>& ModuleIdManager::Modules() {
    return _modules;
}

Module& ModuleIdManager::GetModule(int id) {
    return _modules[id];
}

int ModuleIdManager::MinStaticID() {
    return _staticStart;
}

std::ostream& operator<<(std::ostream& out, const Module& mod) {
    out << "Module with ID " << mod.id << " at ";
    std::string sep = "(";
    for (const auto coord : mod.coords) {
        out << sep << coord;
        sep = ", ";
    }
    out << ")";
    return out;
}