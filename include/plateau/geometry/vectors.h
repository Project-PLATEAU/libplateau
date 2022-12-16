#pragma once
#include <sstream>
#include <iostream>
#include <math.h>
#include <string.h>

namespace plateau::geometry {
    // 3D vector class.

    template< class T > class Vector3 {
    public:
        union {
            T xyz[3];
            T rgb[3];
            struct {
                T x, y, z;
            };
            struct {
                T r, g, b;
            };
        };

    public:
        Vector3(const T x = (T)0, const T y = (T)0, const T z = (T)0);
        Vector3(const T vec[]);

        inline T length() const;
        inline T sqrLength() const;

        T dot(const Vector3<T>& vec) const;
        Vector3 cross(const Vector3<T>& vec) const;

        Vector3  normal() const;
        Vector3& normalEq();
        Vector3& normalEq(const T length);

        Vector3 operator+(const Vector3<T>& rhs) const;
        Vector3 operator+(const T& _v) const;
        Vector3 operator-(const Vector3<T>& rhs) const;
        Vector3 operator-(const T& _v) const;
        Vector3 operator-() const;
        Vector3 operator*(const T& rhs) const;
        Vector3 operator*(const Vector3<T>& rhs) const;
        Vector3 operator/(const T& rhs) const;
        Vector3 operator/(const Vector3<T>& rhs) const;

        inline bool operator==(const Vector3<T>& rhs) const;
        inline bool operator!=(const Vector3<T>& rhs) const;

        inline operator T* () {
            return xyz;
        }
        inline operator const T* () const {
            return xyz;
        }
    };

    template< class T > inline Vector3<T>::Vector3(const T x, const T y, const T z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    template< class T >	inline Vector3<T>::Vector3(const T vec[]) {
        memcpy(xyz, vec, 3 * sizeof(T));
    }

    template< class T > inline T Vector3<T>::length() const {
        return (T)sqrt(x * x + y * y + z * z);
    }

    template< class T > inline T Vector3<T>::sqrLength() const {
        return x * x + y * y + z * z;
    }

    template< class T > inline T Vector3<T>::dot(const Vector3<T>& vec) const {
        return x * vec.x + y * vec.y + z * vec.z;
    }

    template< class T > inline Vector3<T> Vector3<T>::cross(const Vector3<T>& vec) const {
        return Vector3<T>(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x);
    }

    template< class T > inline Vector3<T> Vector3<T>::normal() const {
        const T len = length();
        const T tmp = (len != (T)0) ? ((T)1 / len) : (T)0;
        return Vector3<T>(x * tmp, y * tmp, z * tmp);
    }

    template< class T > inline Vector3<T>& Vector3<T>::normalEq() {
        const T len = length();
        const T tmp = (len != (T)0) ? ((T)1 / len) : (T)0;
        x *= tmp;
        y *= tmp;
        z *= tmp;
        return *this;
    }

    template< class T > inline Vector3<T>& Vector3<T>::normalEq(const T length) {
        const T len = Vector3<T>::length();
        const T tmp = (len != (T)0) ? length / len : (T)0;
        x *= tmp;
        y *= tmp;
        z *= tmp;
        return *this;
    }

    template< class T > inline Vector3<T> Vector3<T>::operator+(const Vector3<T>& rhs) const {
        return Vector3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    template< class T >	inline Vector3<T> Vector3<T>::operator+(const T& _v) const {
        return Vector3<T>(x + _v, y + _v, z + _v);
    }

    template< class T > inline Vector3<T> Vector3<T>::operator-(const Vector3<T>& rhs) const {
        return Vector3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    template< class T > inline Vector3<T> Vector3<T>::operator-(const T& _v) const {
        return Vector3<T>(x - _v, y - _v, z - _v);
    }

    template< class T >	inline Vector3<T> Vector3<T>::operator-() const {
        return Vector3<T>(-x, -y, -z);
    }

    template< class T > inline Vector3<T> Vector3<T>::operator*(const T& rhs) const {
        return Vector3<T>(x * rhs, y * rhs, z * rhs);
    }

    template< class T >	inline Vector3<T> Vector3<T>::operator*(const Vector3<T>& rhs) const {
        return Vector3<T>(x * rhs.x, y * rhs.y, z * rhs.z);
    }

    template< class T > inline Vector3<T> Vector3<T>::operator/(const T& rhs) const {
        return Vector3<T>(x / rhs, y / rhs, z / rhs);
    }

    template< class T > inline Vector3<T> Vector3<T>::operator/(const Vector3<T>& rhs) const {
        return Vector3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
    }

    template< class T > inline bool Vector3<T>::operator==(const Vector3<T>& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    template< class T > inline bool Vector3<T>::operator!=(const Vector3<T>& rhs) const {
        return x != rhs.x || y != rhs.y || z != rhs.z;
    }

    template< class T >	inline Vector3<T> operator*(const T& val, const Vector3<T>& vec) {
        return Vector3<T>(vec.x * val, vec.y * val, vec.z * val);
    }

    template<class T> inline std::ostream& operator<<(std::ostream& os, const Vector3<T>& v) {
        return os << std::fixed << v.x << " " << std::fixed << v.y << " " << std::fixed << v.z;
    }

    template<class T> inline std::istream& operator>>(std::istream& is, Vector3<T>& v) {
        return is >> v.x >> v.y >> v.z;
    }

    typedef Vector3< float >			Vector3f;
    typedef Vector3< double >			Vector3d;

}