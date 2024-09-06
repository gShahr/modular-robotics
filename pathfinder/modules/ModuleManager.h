#include <iostream>
#include <vector>
#include <unordered_set>
#include <boost/functional/hash.hpp>
#include <valarray>
#include <nlohmann/json.hpp>

#ifndef MODULAR_ROBOTICS_MODULEMANAGER_H
#define MODULAR_ROBOTICS_MODULEMANAGER_H

// Module Data Storage Constants (Don't change these)
#define MM_DATA_FULL 0
#define MM_DATA_INT64 1
/* Module Data Storage Configuration
 * FULL: Data stored as ModuleBasic, contains a complete copy of coordinates and properties.
 * INT64: Data stored as ModuleInt64, much more memory efficient but has limitations:
 *  - Module coordinates must fall within inclusive range of (0, 0, 0) to (255, 255, 255)
 *  - Modules may only have up to one property.
 */
#define CONFIG_MOD_DATA_STORAGE MM_DATA_INT64

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

    void UpdateProperties(const std::valarray<int>& moveInfo);

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

class IModuleBasic {
public:
    [[nodiscard]]
    virtual const std::valarray<int>& Coords() const = 0;

    [[nodiscard]]
    virtual const ModuleProperties& Properties() const = 0;

    virtual bool operator==(const IModuleBasic& right) const = 0;

    virtual bool operator<(const IModuleBasic& right) const = 0;

    virtual ~IModuleBasic() = default;
};

// Class holding a pointer to a class implementing IModuleBasic, exists purely for convenience
class ModuleData : public IModuleBasic {
private:
    std::unique_ptr<IModuleBasic> module;

public:
    ModuleData(const ModuleData& modData);

    ModuleData(const std::valarray<int>& coords, const ModuleProperties& properties);

    [[nodiscard]]
    const std::valarray<int>& Coords() const override;

    [[nodiscard]]
    const ModuleProperties& Properties() const override;

    bool operator==(const IModuleBasic& right) const override;

    bool operator<(const IModuleBasic& right) const override;

    friend class std::hash<ModuleData>;
};

// Class used to hold bare minimum representation of a module, for use in Configuration class
class ModuleBasic : public IModuleBasic {
private:
    std::size_t hash = -1;

    bool hashCacheValid = false;

    std::valarray<int> coords;

    ModuleProperties properties;

public:
    ModuleBasic() = default;

    ModuleBasic(const std::valarray<int>& coords, const ModuleProperties& properties);

    const std::valarray<int>& Coords() const override;

    const ModuleProperties& Properties() const override;

    bool operator==(const IModuleBasic& right) const override;

    bool operator<(const IModuleBasic& right) const override;

    friend class std::hash<ModuleBasic>;
};

// Class used to represent a module as a single 64-bit integer. Conditions for proper functionality:
// - Modules coordinates must fall within (inclusive) range (0, 0, 0) to (255, 255, 255)
// - Modules may only have a single property, which must be able to be represented flawlessly using <= 40 bits
class ModuleInt64 : public IModuleBasic {
private:
    std::uint_fast64_t modInt;

    static std::unordered_map<std::uint_fast64_t, ModuleProperties> propertyMap;
public:
    ModuleInt64(const std::valarray<int>& coords, const ModuleProperties& properties);

    const std::valarray<int>& Coords() const override;

    const ModuleProperties& Properties() const override;

    bool operator==(const IModuleBasic& right) const override;

    bool operator<(const IModuleBasic& right) const override;

    friend class std::hash<ModuleData>;
};

template<>
struct std::hash<ModuleData> {
    std::size_t operator()(const ModuleData& modData) const noexcept;
};

template<>
struct boost::hash<ModuleData> {
    std::size_t operator()(const ModuleData& modData) const noexcept;
};

template<>
struct std::hash<ModuleBasic> {
    std::size_t operator()(ModuleBasic& modData) const;
};

template<>
struct boost::hash<ModuleBasic> {
    std::size_t operator()(const ModuleBasic& modData) const;
};

template<>
struct boost::hash<ModuleProperties> {
    std::size_t operator()(const ModuleProperties& moduleProperties);
};

// Class used to hold info about each module
class Module {
public:
    // Coordinate information
    std::valarray<int> coords;
    // Static module check
    bool moduleStatic = false;
    // Properties
    ModuleProperties properties;
    // Module ID
    int id;

    Module(Module&& mod) noexcept;

    explicit Module(const std::valarray<int>& coords, bool isStatic = false, const nlohmann::basic_json<>& propertyDefs = {});
};

struct DeferredModCnstrArgs {
    std::valarray<int> coords;
    bool isStatic;
    nlohmann::basic_json<> propertyDefs;
};

// Class responsible for module ID assignment and providing a central place where modules are stored
class ModuleIdManager {
private:
    // ID to be assigned to next module during construction
    static int _nextId;
    // Vector holding info needed to construct non-static modules
    static std::vector<DeferredModCnstrArgs> _deferredInitMods;
    // Vector holding all modules, indexed by module ID, starting with non-static modules
    static std::vector<Module> _modules;
    // ID of first static module
    static int _staticStart;

public:
    // Never instantiate ModuleIdManager
    ModuleIdManager() = delete;
    ModuleIdManager(const ModuleIdManager&) = delete;

    // Register a new module
    static void RegisterModule(const std::valarray<int>& coords, bool isStatic, const nlohmann::basic_json<>& propertyDefs = {}, bool deferred = false);

    // Register static modules after non-static modules
    static void DeferredRegistration();

    // Get ID for assignment to newly created module
    [[nodiscard]]
    static int GetNextId();

    // Get vector of modules
    static std::vector<Module>& Modules();

    // Get module by ID
    static Module& GetModule(int id);

    // Get index of first static module
    static int MinStaticID();
};

// Stream insertion operator overloaded for easy printing of module info
std::ostream& operator<<(std::ostream& out, const Module& mod);

#endif //MODULAR_ROBOTICS_MODULEMANAGER_H
