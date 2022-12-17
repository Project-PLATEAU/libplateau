#pragma once

//#include <citygml/vecs.hpp>
#include "plateau/geometry/vectors.h"

namespace plateau::geometry {
    class  PolarToPlaneCartesian {
    public:
        void project(double xyz[], int cartesian_coordinate_system_id);
        void project(Vector3d& position, int cartesian_coordinate_system_id);
        void unproject(Vector3d& lat_lon, int cartesian_coordinate_system_id);

    private:
        double Merid(double phi2);

        int jt_;
        double n_, anh_;
    };
}