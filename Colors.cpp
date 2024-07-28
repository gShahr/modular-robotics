#include "Colors.h"

namespace Colors {
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