#ifndef MODULEPROPERTIES_H
#define MODULEPROPERTIES_H
#include <string>
#include <unordered_set>
#include <boost/container_hash/hash.hpp>
#include <boost/dll.hpp>
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

    void CallFunction(const std::string& funcKey);

    friend class ModuleProperties;
};

//template<typename T, class C, class... Args>
//class PropertyFunction;

// These properties can change as a result of certain events, such as moving, or even having a module move adjacent to
// the affected module.
class IModuleDynamicProperty : public IModuleProperty {
protected:
    virtual void UpdateProperty(const std::valarray<int>& moveInfo) = 0;

    friend class ModuleProperties;

    [[nodiscard]]
    IModuleDynamicProperty* MakeCopy() const override = 0;
};

template<typename T>
T& ResultHolder() {
    static T result;
    return result;
}

// Class used by modules to track and update their properties (other than coordinate info)
class ModuleProperties {
private:
    // Static data for keeping track of JSON keys
    static std::vector<std::string>& PropertyKeys();

    // Static data for mapping JSON keys to constructors
    static std::unordered_map<std::string, IModuleProperty* (*)(const nlohmann::basic_json<>& propertyDef)>& Constructors();

    // Static data for mapping strings to static property functions
    static std::unordered_map<std::string, void (*)()>& Functions();

    // Static data for mapping strings to dynamic property functions
    static std::unordered_map<std::string, void (*)(IModuleProperty*)>& InstFunctions();
//    // Static data for mapping JSON keys to static property functions
//    template<typename T, class... Args>
//    static std::unordered_map<std::string, T (*)(Args...)>& Functions();

//    template<typename T, class... Args>
//    static std::unordered_map<std::string, T(PropertyFunction<T, Args...>::*)(Args...)>& InstFunctions();

    // # of properties linked
    static int _propertiesLinkedCount;

    // Properties of a module
    std::unordered_set<IModuleProperty*> _properties;

    // Dynamic properties
    std::unordered_set<IModuleDynamicProperty*> _dynamicProperties;
public:
    ModuleProperties() = default;

    //ModuleProperties(const ModuleProperties&) = delete;
    ModuleProperties(const ModuleProperties& other);

    static void LinkProperties();

    static int PropertyCount();

    static void CallFunction(const std::string& funcKey);

//    template<typename T, class... Args>
//    static void MapStaticFunction(const std::string& key, T (*function)(Args...));

//    template<typename T>
//    T CallFunction(const std::string& propKey, const std::string& funcKey) const;

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
//    template<typename T, class C, class... Args>
//    friend class PropertyFunction;
    friend struct PropertyInitializer;
    friend class boost::hash<ModuleProperties>;
};

//template<typename T, class C, class... Args>
//class PropertyFunction {
//    T (C::*_function)(Args...);
//public:
//    PropertyFunction(const std::string &key, T (C::*function)(Args...)) {
//        _function = function;
//        ModuleProperties::InstFunctions<T, Args...>()[key] = Call;
//    }
//
//    T Call(void* invoker, Args&&... args) {
//        return (invoker->*_function)(std::forward<Args>(args)...);
//    }
//};

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
