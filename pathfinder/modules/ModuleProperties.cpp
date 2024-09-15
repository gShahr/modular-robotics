#include <iostream>
#include <set>
#include "ModuleProperties.h"
#include "../coordtensor/debug_util.h"

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
                if (auto dynamicProperty = dynamic_cast<IModuleDynamicProperty*>(property); dynamicProperty == nullptr) {
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

std::size_t boost::hash<ModuleProperties>::operator()(const ModuleProperties& moduleProperties) const noexcept {
    //std::size_t prev = 0;
    auto cmp = [](const int a, const int b) { return a < b; };
    std::set<std::size_t, decltype(cmp)> hashes(cmp);
    for (const auto property : moduleProperties._properties) {
        //auto current = property->GetHash();
        hashes.insert(property->GetHash());
        //boost::hash_combine(prev, current);
    }
    //return prev;
    return boost::hash_range(hashes.begin(), hashes.end());
}
