#ifndef MODULAR_ROBOTICS_COLORS_H
#define MODULAR_ROBOTICS_COLORS_H

#include <map>
#include <string>

namespace Colors {
    struct RGB {
        int red;
        int green;
        int blue;

        RGB(int r, int g, int b);

        explicit RGB(int rgbInt);
    };

    extern std::map<std::string, RGB> colorToRGB;

    RGB convertColorNameToRGB(const std::string& colorName);

    extern std::map<std::string, int> colorToInt;

    extern std::map<int, std::string> intToColor;
}

#endif //MODULAR_ROBOTICS_COLORS_H
