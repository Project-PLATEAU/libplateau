#pragma once

//#include <citygml/vecs.hpp>
#include "vectors.h"
#include "geo_coordinate.h"
#include <libplateau_api.h>

namespace plateau::geometry {

    /**
     * 座標変換の基準を保持し、座標変換します。
     *
     * zoneID についてはこちらを参照してください :
     * https://www.gsi.go.jp/sokuchikijun/jpc.html
     */
    class LIBPLATEAU_EXPORT GeoReference {
    public:
        explicit GeoReference(int coordinate_zone_id, const Vector3d& reference_point = Vector3d(0, 0, 0),
                              float unit_scale = 1.0, CoordinateSystem coordinate_system = CoordinateSystem::ENU);

        /**
         * 緯度・経度・高さで表現される座標を平面直角座標系に変換します。
         * 座標軸変換を含みます。
         */
        Vector3d project(const GeoCoordinate& point) const;
        Vector3d project(const Vector3d& lat_lon) const;
        /// project の座標軸変換をしない版です。座標軸は ENU → ENU であるとします。 reference_point_ は ENUに変換されます。
        Vector3d projectWithoutAxisConvert(const Vector3d& lat_lon) const;
        static Vector3d convertAxisFromENUTo(CoordinateSystem axis, const Vector3d& vertex);
        static Vector3d convertAxisToENU(CoordinateSystem axis, const Vector3d& vertex);

        GeoCoordinate unproject(const Vector3d& point) const;

        void setReferencePoint(Vector3d point);
        Vector3d getReferencePoint() const;
        int getZoneID() const;
        void setZoneID(int value);
        float getUnitScale() const;
        CoordinateSystem getCoordinateSystem() const;

    private:
        Vector3d reference_point_;
        int zone_id_;
        CoordinateSystem coordinate_system_;
        float unit_scale_;
    };
}
