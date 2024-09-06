#include "Colors.h"

namespace Colors {
    ColorsRGB::ColorsRGB(int r, int g, int b) : red(r), green(g), blue(b) {}

    ColorsRGB::ColorsRGB(int rgbInt) {
        red = rgbInt & 0xFF0000;
        green = rgbInt & 0x00FF00;
        blue = rgbInt & 0x0000FF;
    }

    std::map<std::string, ColorsRGB> colorToRGB = {
            {"red", {255, 0, 0}},
            {"green", {0, 255, 0}},
            {"blue", {0, 0, 255}},
            {"cyan", {0, 255, 255}},
            {"pink", {255, 192, 203}},
            {"orange", {255, 165, 0}},
            {"purple", {128, 0, 128}},
            {"yellow", {255, 255, 0}},
            {"brown", {165, 42, 42}},
            {"black", {0, 0, 0}},
            {"white", {255, 255, 255}},
            {"gray", {128, 128, 128}},
            {"lightgray", {211, 211, 211}},
            {"darkgray", {169, 169, 169}},
            {"magenta", {255, 0, 255}}
    };

    ColorsRGB convertColorNameToRGB(const std::string& colorName) {
        auto it = colorToRGB.find(colorName);
        if (it != colorToRGB.end()) {
            return it->second;
        } else {
            return ColorsRGB(0);
        }
    }

    std::map<std::string, int> colorToInt = {
            {"red", 0xFF0000},
            {"green", 0x00FF00},
            {"blue", 0x0000FF},
            {"cyan", 0x00FFFF},
            {"pink", 0xFFC0CB},
            {"orange", 0xFFA500},
            {"purple", 0x800080},
            {"yellow", 0xFFFF00},
            {"brown", 0xA52A2A},
            {"black", 0x000000},
            {"white", 0xFFFFFF},
            {"gray", 0x808080},
            {"lightgray", 0xD3D3D3},
            {"darkgray", 0xA9A9A9},
            {"magenta", 0xFF00FF}
    };

    std::map<int, std::string> intToColor = {
            {0xFF0000,"red"},
            {0x00FF00,"green"},
            {0x0000FF,"blue"},
            {0x00FFFF,"cyan"},
            {0xFFC0CB,"pink"},
            {0xFFA500,"orange"},
            {0x800080,"purple"},
            {0xFFFF00,"yellow"},
            {0xA52A2A,"brown"},
            {0x000000,"black"},
            {0xFFFFFF,"white"},
            {0x808080,"gray"},
            {0xD3D3D3,"lightgray"},
            {0xA9A9A9,"darkgray"},
            {0xFF00FF,"magenta"}
    };
}

// Make sure this properties constructor is in the constructor map
PropertyInitializer ColorProperty::initializer(COLOR_PROP_NAME, &PropertyInitializer::InitProperty<ColorProperty>);

std::unordered_set<int> ColorProperty::allColors;

bool ColorProperty::CompareProperty(const IModuleProperty& right) {
    return color == dynamic_cast<const ColorProperty&>(right).color;
}

IModuleProperty* ColorProperty::MakeCopy() const {
    return new ColorProperty(*this);
}

std::uint_fast64_t ColorProperty::AsInt() const {
    return color;
}

std::size_t ColorProperty::GetHash() {
    boost::hash<int> hash;
    return hash(color);
}

ColorProperty::ColorProperty(const nlohmann::basic_json<>& propertyDef) {
    key = COLOR_PROP_NAME;
    if (propertyDef[COLOR].is_array()) {
        if (std::all_of(propertyDef[COLOR].begin(), propertyDef[COLOR].end(),
                        [](const nlohmann::basic_json<>& i){return i.is_number_integer();})) {
            color = 0;
            for (int channel : propertyDef[COLOR]) {
                color += channel;
            }
        }
    } else if (propertyDef[COLOR].is_string()) {
        color = Colors::colorToInt[propertyDef[COLOR]];
    } else {
        std::cerr << "Color improperly formatted." << std::endl;
        return;
    }
    allColors.insert(color);
}

int ColorProperty::GetColorInt() const {
    return color;
}

const std::unordered_set<int>& ColorProperty::Palette() {
    return allColors;
}