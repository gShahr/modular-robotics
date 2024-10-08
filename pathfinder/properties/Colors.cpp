#include <iostream>
#include "Colors.h"
#include "../utility/color_util.h"

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
    constexpr boost::hash<int> hash;
    return hash(color);
}

ColorProperty::ColorProperty(const nlohmann::basic_json<>& propertyDef) {
    key = COLOR_PROP_NAME;
    if (propertyDef[COLOR].is_array()) {
        if (std::all_of(propertyDef[COLOR].begin(), propertyDef[COLOR].end(),
                        [](const nlohmann::basic_json<>& i){return i.is_number_integer();})) {
            color = 0;
            int chShift = 0;
            for (int channel : propertyDef[COLOR]) {
                channel <<= chShift;
                chShift += 8;
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

boost::any Palette() {
    return std::cref(ColorProperty::Palette());
}

boost::any GetColorInt(IModuleProperty* prop) {
    const auto colorProp = reinterpret_cast<ColorProperty*>(prop);
    return colorProp->GetColorInt();
}

// This function is never called but removing it causes runtime crashing on ColorProperty constructor calls
void Dummy() {
    ResultHolder<int>() = 4000;
}
