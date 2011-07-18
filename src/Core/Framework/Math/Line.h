/** @file Line.h
    @author Jukka Jyl�nki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

/// A line in 3D space runs through two points and extends to infinity in two directions.
class Line
{
public:
    /// @note The default ctor does not initialize any member values.
    Line() {}
    Line(const float3 &pos, const float3 &dir);
    explicit Line(const Ray &ray);
    explicit Line(const LineSegment &lineSegment);

    float3 pos;
    float3 dir;

    /// Returns a point on this line.
    /// GetPoint(0) returns the point 'pos'.
    /// GetPoint with a positive value returns points towards the vector 'dir'.
    float3 GetPoint(float distance) const;

    /// Applies a transformation to this line, in-place.
    void Transform(const float3x3 &transform);
    void Transform(const float3x4 &transform);
    void Transform(const float4x4 &transform);
    void Transform(const Quat &transform);

    /// Returns true if this ray contains the given point, i.e. if the squared
    /// distance to this point is smaller than the given threshold epsilon.
    bool Contains(const float3 &point, float distanceThreshold = 1e-3f) const;

    /// Returns the distance of the given point to this line.
    /// @param d [out] This element will receive the distance along this line that specifies the closest point on this line to the given point.
    float Distance(const float3 &point, float *d = 0) const;

    /// Returns the distance of the given ray/line/linesegment to this line.
    /// @param d [out] Receives the distance along this line that specifies the closest point on this line to the given point.
    /// @param d2 [out] Receives the distance along the other line that specifies the closest point on that line to this line.
    float Distance(const Ray &other, float *d = 0, float *d2 = 0) const;
    float Distance(const Line &other, float *d = 0, float *d2 = 0) const;
    float Distance(const LineSegment &other, float *d = 0, float *d2 = 0) const;

    bool Intersects(const Triangle &triangle, float *d, float3 *intersectionPoint) const;
    bool Intersects(const Plane &plane, float *d) const;
    bool Intersects(const Sphere &s, float3 *intersectionPoint = 0, float3 *intersectionNormal = 0, float *d = 0) const;
    bool Intersects(const AABB &aabb, float *dNear, float *dFar) const;
    bool Intersects(const OBB &obb, float *dNear, float *dFar) const;

    /// Returns the closest point on <b>this</b> line to the given object.
    float3 ClosestPoint(const float3 &targetPoint, float *d = 0) const;
    float3 ClosestPoint(const Ray &other, float *d = 0, float *d2 = 0) const;
    float3 ClosestPoint(const Line &other, float *d = 0, float *d2 = 0) const;
    float3 ClosestPoint(const LineSegment &other, float *d = 0, float *d2 = 0) const;
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
*/
//    bool Intersect(const Triangle &triangle) const;
//    bool Intersect(const Triangle &triangle, float &outDistance) const;

//    bool Intersect(const Cylinder &cylinder) const;
//    bool Intersect(const Cylinder &cylinder, float &outDistance) const;

//    bool Intersect(const Torus &torus) const;
//    bool Intersect(const Torus &torus, float &outDistance) const;

//    bool Intersect(const Frustum &frustum) const;
//    bool Intersect(const Frustum &frustum, float &outDistance) const;

//    bool Intersect(const Polyhedron &polyhedron) const;
//    bool Intersect(const Polyhedron &polyhedron, float &outDistance) const;

    Ray ToRay() const;
    LineSegment ToLineSegment(float d) const;

    /// Returns true if the given three points are collinear (they all lie on the same line).
    static bool AreCollinear(const float3 &p1, const float3 &p2, const float3 &p3, float epsilon = 1e-3f);
};

Line operator *(const float3x3 &transform, const Line &line);
Line operator *(const float3x4 &transform, const Line &line);
Line operator *(const float4x4 &transform, const Line &line);
Line operator *(const Quat &transform, const Line &line);

// Internal helper functions.
float Dmnop(const float3 *v, int m, int n, int o, int p);
float3 LineLine(float3 start0, float3 end0, float3 start1, float3 end1, float *d, float *d2);

#ifdef QT_INTEROP
Q_DECLARE_METATYPE(Line)
Q_DECLARE_METATYPE(Line*)
#endif
