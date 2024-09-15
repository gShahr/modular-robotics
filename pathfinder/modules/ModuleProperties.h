#ifndef MODULEPROPERTIES_H
#define MODULEPROPERTIES_H
#include <string>
#include <unordered_set>
#include <boost/container_hash/hash.hpp>
#include <nlohmann/json.hpp>

// An interface for properties that a module might have, ex: Color, Direction, etc.
// This
class IModuleProperty {
protected:
    std::string key;

    virtual bool CompareProperty(const IModuleProperty& right) = 0;

    [[nodiscard]]
    virtual IModuleProperty* MakeCopy() const = 0;

    [[nodiscard]]
    virtual std::uint_fast64_t AsInt() const = 0;

public:
    virtual std::size_t GetHash() = 0;

    virtual ~IModuleProperty() = default;

    friend class ModuleProperties;
};

// These properties can change as a result of certain events, such as moving, or even having a module move adjacent to
// the affected module.
class IModuleDynamicProperty : public IModuleProperty {
protected:
    virtual void UpdateProperty(const std::valarray<int>& moveInfo) = 0;

    friend class ModuleProperties;

    [[nodiscard]]
    IModuleDynamicProperty* MakeCopy() const override = 0;
};

// Class used by modules to track and update their properties (other than coordinate info)
class ModuleProperties {
private:
    // Static data for keeping track of JSON keys
    static std::vector<std::string>& PropertyKeys();

    // Static data for mapping JSON keys to constructors
    static std::unordered_map<std::string, IModuleProperty* (*)(const nlohmann::basic_json<>& propertyDef)>& Constructors();

    // Properties of a module
    std::unordered_set<IModuleProperty*> _properties;

    // Dynamic properties
    std::unordered_set<IModuleDynamicProperty*> _dynamicProperties;
public:
    ModuleProperties() = default;

    //ModuleProperties(const ModuleProperties&) = delete;
    ModuleProperties(const ModuleProperties& other);

    void InitProperties(const nlohmann::basic_json<>& propertyDefs);

    void UpdateProperties(const std::valarray<int>& moveInfo) const;

    bool operator==(const ModuleProperties& right) const;

    bool operator!=(const ModuleProperties& right) const;

    ModuleProperties& operator=(const ModuleProperties& right);

    IModuleProperty* Find(const std::string& key) const;

    [[nodiscard]]
    std::uint_fast64_t AsInt() const;

    ~ModuleProperties();

    friend class IModuleProperty;
    friend struct PropertyInitializer;
    friend class boost::hash<ModuleProperties>;
};

// Used by property classes to add their constructor to the constructor map
struct PropertyInitializer {
    template<class Prop>
    static IModuleProperty* InitProperty(const nlohmann::basic_json<>& propertyDef) {
        return new Prop(propertyDef);
    }

    PropertyInitializer(const std::string& name, IModuleProperty* (*constructor)(const nlohmann::basic_json<>& propertyDef));

    static IModuleProperty* GetProperty(const nlohmann::basic_json<>& propertyDef);
};

template<>
struct boost::hash<ModuleProperties> {
    std::size_t operator()(const ModuleProperties& moduleProperties) const noexcept;
};

#endif //MODULEPROPERTIES_H
