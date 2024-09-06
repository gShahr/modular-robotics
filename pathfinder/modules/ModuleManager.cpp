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

void ModuleProperties::UpdateProperties(const std::valarray<int>& moveInfo) {
    for (auto property : _dynamicProperties) {
        property->UpdateProperty(moveInfo);
    }
}

bool ModuleProperties::operator==(const ModuleProperties& right) const {
    if (_properties.size() != right._properties.size()) {
        return false;
    }

    std::unordered_set<IModuleProperty*> leftProps = _properties;
    for (auto rProp : right._properties) {
        auto lProp = Find(rProp->key);
        if (lProp != nullptr && lProp->CompareProperty(*rProp)) {
            leftProps.erase(lProp);
        } else {
            return false;
        }
    }

    return leftProps.empty();
}

bool ModuleProperties::operator!=(const ModuleProperties& right) const {
    if (_properties.size() != right._properties.size()) {
        return true;
    }
    return _properties != right._properties;
}

ModuleProperties& ModuleProperties::operator=(const ModuleProperties& right) {
    _properties.clear();
    for (auto property : right._properties) {
        _properties.insert(property->MakeCopy());
    }
    _dynamicProperties.clear();
    if (right._dynamicProperties.empty()) {
        return *this;
    }
    for (auto dynamicProperty : right._dynamicProperties) {
        _dynamicProperties.insert(dynamicProperty->MakeCopy());
    }
    return *this;
}

IModuleProperty* ModuleProperties::Find(const std::string& key) const {
    for (auto property : _properties) {
        if (property->key == key) {
            return property;
        }
    }
    return nullptr;
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
    std::hash<ModuleBasic> hasher;
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

std::size_t std::hash<ModuleBasic>::operator()(ModuleBasic& modData) const {
    if (!modData.hashCacheValid) {
        boost::hash<ModuleBasic> hasher;
        modData.hash = hasher(modData);
        modData.hashCacheValid = true;
    }
    return modData.hash;
}

std::size_t boost::hash<ModuleBasic>::operator()(const ModuleBasic& modData) const {
    auto coordHash = boost::hash_range(begin(modData.Coords()), end(modData.Coords()));
    if (!Lattice::ignoreProperties) {
        boost::hash<ModuleProperties> propertyHasher;
        auto propertyHash = propertyHasher(modData.Properties());
        boost::hash_combine(coordHash, propertyHash);
    }
    return coordHash;
}

std::size_t boost::hash<ModuleProperties>::operator()(const ModuleProperties& moduleProperties) {
    //std::size_t prev = 0;
    auto cmp = [](int a, int b) { return a < b; };
    std::set<std::size_t, decltype(cmp)> hashes(cmp);
    for (auto property : moduleProperties._properties) {
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

Module::Module(const std::valarray<int>& coords, bool isStatic, const nlohmann::basic_json<>& propertyDefs) : id(ModuleIdManager::GetNextId()), coords(coords), moduleStatic(isStatic) {
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
    for (auto coord : mod.coords) {
        out << sep << coord;
        sep = ", ";
    }
    out << ")";
    return out;
}