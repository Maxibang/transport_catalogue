#pragma once

#include <cmath>

# define M_PI           3.14159265358979323846

namespace transport {
    
namespace detail {

    
struct Distance {
    Distance() = default;
    double gps_dist;
    int real_dist;
};    
    
    
struct Coordinates {
    Coordinates() = default;
    double lat;
    double lng;
    
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

    
inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = M_PI / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}

        
} // End namespace detail 
    
} // End namespace transport