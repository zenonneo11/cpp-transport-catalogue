#pragma once
#include <string>
#include <vector>
#include "geo.h"

namespace transport_catalogue {

    struct Stop {
        bool is_exists = true;
        std::string name;
        geo::Coordinates coordinates;

    };

    struct Bus {
        bool is_exists = true;
        std::string name;
        bool is_loop;
        std::vector<const Stop*> stops;
    };
}
