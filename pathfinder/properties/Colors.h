#ifndef MODULAR_ROBOTICS_COLORS_H
#define MODULAR_ROBOTICS_COLORS_H

#include <map>
#include <string>
#include "../modules/ModuleProperties.h"

// Name of the color property in JSON
#define COLOR_PROP_NAME "colorProperty"
// Name of the actual color value in JSON
#define COLOR "color"

namespace Colors {
    struct ColorsRGB {
        int red;
        int green;
        int blue;

        ColorsRGB(int r, int g, int b);

        explicit ColorsRGB(int rgbInt);
    };

    extern std::map<std::string, ColorsRGB> colorToRGB;

    ColorsRGB convertColorNameToRGB(const std::string& colorName);

    extern std::map<std::string, int> colorToInt;

    extern std::map<int, std::string> intToColor;
}

class ColorProperty final : public IModuleProperty {
private:
    // Every (non-abstract) property needs this to ensure constructor is in the constructor map
    static PropertyInitializer initializer;

    int color = -1;

    static std::unordered_set<int> allColors;

protected:
    bool CompareProperty(const IModuleProperty& right) override;

    [[nodiscard]]
    IModuleProperty* MakeCopy() const override;

    [[nodiscard]]
    std::uint_fast64_t AsInt() const override;

public:
    // Need a GetHash function
    std::size_t GetHash() override;

    // Every (non-abstract) property needs a JSON constructor
    explicit ColorProperty(const nlohmann::basic_json<>& propertyDef);

    [[nodiscard]]
    int GetColorInt() const;

    static const std::unordered_set<int>& Palette();
};

#endif //MODULAR_ROBOTICS_COLORS_H
