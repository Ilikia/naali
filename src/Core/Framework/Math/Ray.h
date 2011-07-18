/** @file Ray.h
    @author Jukka Jyl�nki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

#ifdef OGRE_INTEROP
#include <OgreRay.h>
#endif

/// A ray in 3D space is a line that starts from an origin point and extends to infinity in one direction.
class Ray
{
public:
    /// Specifies the origin of this ray.
    float3 pos;

    /// The normalized direction vector of this ray.
    float3 dir;

    /// @note The default ctor does not initialize any member values.
    Ray() {}

    /// Constructs a new ray.
    /// @param pos The origin position of the ray.
    /// @param dir The direction of the ray. This vector must be normalized beforehand when calling this function.
    Ray(const float3 &pos, const float3 &dir);
    explicit Ray(const Line &line);
    explicit Ray(const LineSegment &lineSegment);

    /// Gets a point along the ray at the given distance.
    float3 GetPoint(float distance) const;

    /// Applies a transformation to this ray, in-place.
    void Transform(const float3x3 &transform);
    void Transform(const float3x4 &transform);
    void Transform(const float4x4 &transform);
    void Transform(const Quat &transform);

    /// Returns true if this ray contains the given point, i.e. if the squared
    /// distance to this point is smaller than the given threshold epsilon.
    bool Contains(const float3 &point, float distanceThreshold = 1e-3f) const;

    /// Returns the distance of the given point to this ray.
    /// @param d [out] This element will receive the distance along this ray that specifies the closest point on this ray to the given point. This value can be negative.
    float Distance(const float3 &point, float *d = 0) const;

    /// Returns the distance of the given ray/line/linesegment to this ray.
    /// @param d [out] Receives the distance along this ray that specifies the closest point on this ray to the given point.
    /// @param d2 [out] Receives the distance along the other line that specifies the closest point on that line to this line.
    float Distance(const Ray &other, float *d = 0, float *d2 = 0) const;
    float Distance(const Line &other, float *d = 0, float *d2 = 0) const;
    float Distance(const LineSegment &other, float *d = 0, float *d2 = 0) const;

    /// Returns the closest point on <b>this</b> ray to the given object.
    float3 ClosestPoint(const float3 &targetPoint, float *d = 0) const;
    float3 ClosestPoint(const Ray &other, float *d = 0, float *d2 = 0) const;
    float3 ClosestPoint(const Line &other, float *d = 0, float *d2 = 0) const;
    float3 ClosestPoint(const LineSegment &other, float *d = 0, float *d2 = 0) const;

    bool Intersects(const Triangle &triangle, float *d, float3 *intersectionPoint) const;
    bool Intersects(const Plane &plane, float *d) const;
    bool Intersects(const Sphere &s, float3 *intersectionPoint = 0, float3 *intersectionNormal = 0, float *d = 0) const;
    bool Intersects(const AABB &aabb, float *dNear, float *dFar) const;
    bool Intersects(const OBB &obb, float *dNear, float *dFar) const;

/*
    bool Intersect(const Plane &plane) const;
    bool Intersect(const Plane &plane, float &outDistance) const;

    bool Intersect(const Sphere &sphere) const;
    bool Intersect(const Sphere &sphere, float &outDistance) const;

    bool Intersect(const Ellipsoid &ellipsoid) const;
    bool Intersect(const Ellipsoid &ellipsoid, float &outDistance) const;

    bool Intersect(const AABB &aabb) const;
    bool Intersect(const AABB &aabb, float &outDistance) const;

    bool Intersect(const OBB &aabb) const;
    bool Intersect(const OBB &aabb, float &outDistance) const;

    bool Intersect(const Triangle &triangle) const;
    bool Intersect(const Triangle &triangle, float &outDistance) const;

    bool Intersect(const Cylinder &cylinder) const;
    bool Intersect(const Cylinder &cylinder, float &outDistance) const;

//    bool Intersect(const Torus &torus) const;
//    bool Intersect(const Torus &torus, float &outDistance) const;

    bool Intersect(const Frustum &frustum) const;
    bool Intersect(const Frustum &frustum, float &outDistance) const;

 //   bool Intersect(const Polyhedron &polyhedron) const;
 //   bool Intersect(const Polyhedron &polyhedron, float &outDistance) const;
*/
    Line ToLine() const;
    LineSegment ToLineSegment(float d) const;

#ifdef OGRE_INTEROP
    Ray(const Ogre::Ray &other) { pos = other.getOrigin(); dir = other.getDirection(); }
    operator Ogre::Ray() const { return Ogre::Ray(pos, dir); }
#endif

};

Ray operator *(const float3x3 &transform, const Ray &ray);
Ray operator *(const float3x4 &transform, const Ray &ray);
Ray operator *(const float4x4 &transform, const Ray &ray);
Ray operator *(const Quat &transform, const Ray &ray);

#ifdef QT_INTEROP
Q_DECLARE_METATYPE(Ray)
Q_DECLARE_METATYPE(Ray*)
#endif
