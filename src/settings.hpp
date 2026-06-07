#pragma once

#include <nwo5.silly-api/include/settings/include.hpp>

using namespace nwo5::settings::prelude;

namespace Settings {
    inline Setting<bool> listAnd{"list-and"};
    inline Setting<std::string> searchMode{"search-mode"};
    inline Setting<bool> autoDelete{"auto-delete"};
    inline Setting<bool> includeCurrentSelection{"include-current-selection"};
    inline Setting<bool> useSelectionAsFilter{"use-selection-as-filter"};
    inline Setting<bool> moveCameraToSelection{"move-camera-to-selection"};
    inline Setting<float> zoomLimit{"zoom-limit"};
    inline Setting<bool> closeOnSelect{"close-on-select"};
    inline Setting<bool> logs{"logs"};
}