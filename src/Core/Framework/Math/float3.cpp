/** @file
    @author Jukka Jyl�nki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"
#ifdef MATH_ENABLE_STL_SUPPORT
#include <cassert>
#include <utility>
#endif
#include <stdlib.h>

#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float3x3.h"
#include "MathFunc.h"

using namespace std;

float3::float3(float x_, float y_, float z_)
:x(x_), y(y_), z(z_)
{
}

float3::float3(const float2 &xy, float z_)
:x(xy.x), y(xy.y), z(z_)
{
}

float3::float3(const float *data)
{
    assert(data);
    x = data[0];
    y = data[1];
    z = data[2];
}

float *float3::ptr()
{ 
    return &x;
} 

const float *float3::ptr() const
{ 
    return &x;
} 

CONST_WIN32 float float3::operator [](int index) const
{ 
    assert(index >= 0);
    assert(index < Size);
    return ptr()[index];
}

float &float3::operator [](int index)
{ 
    assert(index >= 0);
    assert(index < Size);
    return ptr()[index];
}

float2 float3::xy() const
{
    return float2(x, y);
}

float float3::LengthSq() const
{ 
    return x*x + y*y + z*z;
}

float float3::Length() const
{ 
    return sqrtf(LengthSq());
}

float float3::Normalize()
{ 
    float lengthSq = LengthSq();
    if (lengthSq > 1e-6f)
    {
        float length = sqrtf(lengthSq); 
        *this *= 1.f / length;
        return length;
    }
    else
    {
//        printf("float3::Normalize called on a vector with 0 length!\n");
        Set(1.f, 0.f, 0.f);
        return 0;
    }
}

float3 float3::Normalized() const
{
    float3 copy = *this;
    float oldLength = copy.Normalize();
    assume(oldLength > 0.f && "float3::Normalized() failed!");
    return copy;
}

float float3::ScaleToLength(float newLength)
{
    float length = LengthSq();
    if (length < 1e-6f)
        return 0.f;

    length = sqrtf(length);
    float scalar = newLength / length;
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return length;
}

float3 float3::ScaledToLength(float newLength) const
{
	assume(!IsZero());

	float3 v = *this;
	v.ScaleToLength(newLength);
	return v;
}

bool float3::IsNormalized(float epsilonSq) const
{
    return fabs(LengthSq()-1.f) <= epsilonSq;
}

bool float3::IsZero(float epsilonSq) const
{
    return fabs(LengthSq()) <= epsilonSq;
}

bool float3::IsFinite() const
{
    return isfinite(x) && isfinite(y) && isfinite(z);
}

bool float3::IsPerpendicular(const float3 &other, float epsilon) const
{
    return fabs(Dot(other)) <= epsilon;
}

#ifdef MATH_ENABLE_STL_SUPPORT
std::string float3::ToString() const
{ 
    char str[256];
    sprintf(str, "(%.3f, %.3f, %.3f)", x, y, z);
    return std::string(str);
}

std::string float3::SerializeToString() const
{ 
    char str[256];
    sprintf(str, "%f %f %f", x, y, z);
    return std::string(str);
}
#endif

float3 float3::FromString(const char *str)
{
    assume(str);
    if (!str)
        return float3::nan;
    if (*str == '(')
        ++str;
    float3 f;
    f.x = strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    f.y = strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    f.z = strtod(str, const_cast<char**>(&str));
    return f;
}

float float3::SumOfElements() const
{
    return x + y + z;
}

float float3::ProductOfElements() const
{
    return x * y * z;
}

float float3::AverageOfElements() const
{
    return (x + y + z) / 3.f;
}

float float3::MinElement() const
{
    return ::Min(::Min(x, y), z);
}

int float3::MinElementIndex() const
{
    if (x <= y && x <= z)
        return 0;
    else
        return (y <= z) ? 1 : 2;
}

float float3::MaxElement() const
{
    return ::Max(::Max(x, y), z);
}

int float3::MaxElementIndex() const
{
    if (x >= y && x >= z)
        return 0;
    else
        return (y >= z) ? 1 : 2;
}

float3 float3::Abs() const
{
    return float3(fabs(x), fabs(y), fabs(z));
}

float3 float3::Min(float ceil) const
{
    return float3(::Min(x, ceil), ::Min(y, ceil), ::Min(z, ceil));
}

float3 float3::Min(const float3 &ceil) const
{
    return float3(::Min(x, ceil.x), ::Min(y, ceil.y), ::Min(z, ceil.z));
}

float3 float3::Max(float floor) const
{
    return float3(::Max(x, floor), ::Max(y, floor), ::Max(z, floor));
}

float3 float3::Max(const float3 &floor) const
{
    return float3(::Max(x, floor.x), ::Max(y, floor.y), ::Max(z, floor.z));
}

float3 float3::Clamp(const float3 &floor, const float3 &ceil) const
{
    return Min(ceil).Max(floor);
}

float3 float3::Clamp01() const
{
    return Clamp(0.f, 1.f);
}

float3 float3::Clamp(float floor, float ceil) const
{
    return Min(ceil).Max(floor);
}

float float3::DistanceSq(const float3 &rhs) const
{
    float dx = x - rhs.x;
    float dy = y - rhs.y;
    float dz = z - rhs.z;
    return dx*dx + dy*dy + dz*dz;
}

float float3::Distance(const float3 &rhs) const
{
    return sqrtf(DistanceSq(rhs));
}

float float3::Dot(const float3 &rhs) const
{
    return x * rhs.x + y * rhs.y + z * rhs.z;
}

/** dst = A x B - The standard cross product:
\code
        |a cross b| = |a||b|sin(alpha)
    
        i        j        k        i        j        k        units (correspond to x,y,z)
        a        b        c        a        b        c        this vector
        d        e        f        d        e        f        vector v
        -cei    -afj    -bdk    bfi    cdj    aek    result
    
        x = bfi - cei = (bf-ce)i;
        y = cdj - afj = (cd-af)j;
        z - aek - bdk = (ae-bd)k;
\endcode

Cross product is anti-commutative, i.e. a x b == -b x a.
It distributes over addition, meaning that a x (b + c) == a x b + a x c,
and combines with scalar multiplication: (sa) x b == a x (sb). 
i x j == -(j x i) == k,
(j x k) == -(k x j) == i,
(k x i) == -(i x k) == j. */
float3 float3::Cross(const float3 &rhs) const
{
    float3 dst;
    dst.x = y * rhs.z - z * rhs.y;
    dst.y = z * rhs.x - x * rhs.z;
    dst.z = x * rhs.y - y * rhs.x;
    return dst;
}

float3x3 float3::OuterProduct(const float3 &rhs) const
{
    const float3 &u = *this;
    const float3 &v = rhs;
    return float3x3(u[0]*v[0], u[0]*v[1], u[0]*v[2],
                    u[1]*v[0], u[1]*v[1], u[1]*v[2],
                    u[2]*v[0], u[2]*v[1], u[2]*v[2]);
}

float3 float3::Perpendicular(const float3 &hint, const float3 &hint2) const
{
    assume(!this->IsZero());
    assume(hint.IsNormalized());
    assume(hint2.IsNormalized());
    float3 v = this->Cross(hint);
    float len = v.Normalize();
    if (len == 0)
        return hint2;
    else
        return v;
}

float3 float3::AnotherPerpendicular(const float3 &hint, const float3 &hint2) const
{
    assume(!this->IsZero());
    assume(hint.IsNormalized());
    assume(hint2.IsNormalized());
    float3 firstPerpendicular = Perpendicular(hint, hint2);
    float3 v = this->Cross(firstPerpendicular);
    return v.Normalized();
}

float float3::ScalarTripleProduct(const float3 &u, const float3 &v, const float3 &w)
{
    return u.Cross(v).Dot(w);
}

float3 float3::Reflect(const float3 &normal) const
{
    assume(normal.IsNormalized());
    return 2.f * this->ProjectToNorm(normal) - *this;
}

float3 float3::Refract(const float3 &normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const
{
    assume(false && "Not implemented!"); ///\todo
    return float3(0.f, 0.f, 0.f);
}

float3 float3::ProjectTo(const float3 &direction) const
{
    assume(!direction.IsZero());
    return direction * this->Dot(direction) / direction.LengthSq();
}

float3 float3::ProjectToNorm(const float3 &direction) const
{
    assume(direction.IsNormalized());
    return direction * this->Dot(direction);
}

float float3::AngleBetween(const float3 &other) const
{
    return acos(Dot(other)) / sqrt(LengthSq() * other.LengthSq());
}

float float3::AngleBetweenNorm(const float3 &other) const
{
    assume(this->IsNormalized());
    assume(other.IsNormalized());
    float cosa = Dot(other);
    if (cosa >= 1.f)
        return 0.f;
    else if (cosa <= -1.f)
        return pi;
    else
        return acos(cosa);
}

void float3::Decompose(const float3 &direction, float3 &outParallel, float3 &outPerpendicular) const
{
    assume(direction.IsNormalized());
    outParallel = this->ProjectToNorm(direction);
    outPerpendicular = *this - outParallel;
}

float3 float3::Lerp(const float3 &b, float t) const
{
    assume(0.f <= t && t <= 1.f);
    return (1.f - t) * *this + t * b;
}

float3 float3::Lerp(const float3 &a, const float3 &b, float t)
{
    return a.Lerp(b, t);
}

void float3::Orthogonalize(const float3 &a, float3 &b)
{
    if (!a.IsZero())
        b -= b.ProjectTo(a);
}

void float3::Orthogonalize(const float3 &a, float3 &b, float3 &c)
{
    if (!a.IsZero())
    {
        b -= b.ProjectTo(a);
        c -= c.ProjectTo(a);
    }
    if (!b.IsZero())
        c -= c.ProjectTo(b);
}

bool float3::AreOrthogonal(const float3 &a, const float3 &b, float epsilon)
{
    return a.IsPerpendicular(b, epsilon);
}

bool float3::AreOrthogonal(const float3 &a, const float3 &b, const float3 &c, float epsilon)
{
    return a.IsPerpendicular(b, epsilon) &&
           a.IsPerpendicular(c, epsilon) &&
           b.IsPerpendicular(c, epsilon);
}

void float3::Orthonormalize(float3 &a, float3 &b)
{
    assume(!a.IsZero());
    assume(!b.IsZero());
    a.Normalize();
    b -= b.ProjectToNorm(a);
    b.Normalize();
}

void float3::Orthonormalize(float3 &a, float3 &b, float3 &c)
{
    assume(!a.IsZero());
    a.Normalize();
    b -= b.ProjectToNorm(a);
    assume(!b.IsZero());
    b.Normalize();
    c -= c.ProjectToNorm(a);
    c -= c.ProjectToNorm(b);
    assume(!c.IsZero());
    c.Normalize();
}

bool float3::AreOrthonormal(const float3 &a, const float3 &b, float epsilon)
{
    return a.IsPerpendicular(b, epsilon) && a.IsNormalized(epsilon*epsilon) && b.IsNormalized(epsilon*epsilon);
}

bool float3::AreOrthonormal(const float3 &a, const float3 &b, const float3 &c, float epsilon)
{
    return a.IsPerpendicular(b, epsilon) &&
           a.IsPerpendicular(c, epsilon) &&
           b.IsPerpendicular(c, epsilon) &&
           a.IsNormalized(epsilon*epsilon) &&
           b.IsNormalized(epsilon*epsilon) &&
           c.IsNormalized(epsilon*epsilon);
}

float3 float3::FromScalar(float scalar)
{ 
    return float3(scalar, scalar, scalar);
}

void float3::SetFromScalar(float scalar)
{
    x = scalar;
    y = scalar;
    z = scalar;
}

void float3::Set(float x_, float y_, float z_)
{
    x = x_;
    y = y_;
    z = z_;
}

bool float3::Equals(const float3 &other, float epsilon) const
{
    return fabs(x - other.x) < epsilon &&
           fabs(y - other.y) < epsilon &&
           fabs(z - other.z) < epsilon;
}

bool float3::Equals(float x_, float y_, float z_, float epsilon) const
{
    return fabs(x - x_) < epsilon &&
           fabs(y - y_) < epsilon &&
           fabs(z - z_) < epsilon;
}

float4 float3::ToPos4() const
{
    return float4(*this, 1.f);
}

float4 float3::ToDir4() const
{
    return float4(*this, 0.f);
}

float3 float3::operator +(const float3 &rhs) const
{
    return float3(x + rhs.x, y + rhs.y, z + rhs.z);
}

float3 float3::operator -(const float3 &rhs) const
{
    return float3(x - rhs.x, y - rhs.y, z - rhs.z);
}

float3 float3::operator -() const
{
    return float3(-x, -y, -z);
}

/*
float3 float3::operator *(const float3 &rhs) const
{
    return float3(x * rhs.x, y * rhs.y, z * rhs.z);
}
*/
float3 float3::operator *(float scalar) const
{
    return float3(x * scalar, y * scalar, z * scalar);
}

float3 operator *(float scalar, const float3 &rhs)
{
    return float3(scalar * rhs.x, scalar * rhs.y, scalar * rhs.z);
}
/*
float3 float3::operator /(const float3 &rhs) const
{
    return float3(x / rhs.x, y / rhs.y, z / rhs.z);
}
*/
float3 float3::operator /(float scalar) const
{
    float invScalar = 1.f / scalar;
    return float3(x * invScalar, y * invScalar, z * invScalar);
}

float3 operator /(float scalar, const float3 &rhs)
{
    return float3(scalar / rhs.x, scalar / rhs.y, scalar / rhs.z);
}

float3 &float3::operator +=(const float3 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;

    return *this;
}

float3 &float3::operator -=(const float3 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;

    return *this;
}
/*
float3 &float3::operator *=(const float3 &rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;

    return *this;
}
*/
float3 &float3::operator *=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;

    return *this;
}
/*
float3 &float3::operator /=(const float3 &rhs)
{
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;

    return *this;
}
*/

float3 float3::Mul(const float3 &rhs) const
{
    return float3(x * rhs.x, y * rhs.y, z * rhs.z);
}

float3 &float3::operator /=(float scalar)
{
    float invScalar = 1.f / scalar;
    x *= invScalar;
    y *= invScalar;
    z *= invScalar;

    return *this;
}

#ifdef MATH_ENABLE_STL_SUPPORT
std::ostream &operator <<(std::ostream &out, const float3 &rhs)
{
    std::string str = rhs.ToString();
    out << str;
    return out;
}
#endif

const float3 float3::zero = float3(0, 0, 0);
const float3 float3::one = float3(1, 1, 1);
const float3 float3::unitX = float3(1, 0, 0);
const float3 float3::unitY = float3(0, 1, 0);
const float3 float3::unitZ = float3(0, 0, 1);
const float3 float3::nan = float3(FLOAT_NAN, FLOAT_NAN, FLOAT_NAN);
const float3 float3::inf = float3(FLOAT_INF, FLOAT_INF, FLOAT_INF);
