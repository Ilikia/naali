/** @file
    @author Jukka Jyl�nki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

/// Specifies a triangle through three points in 3D space.
class Triangle
{
public:
    float3 a;
    float3 b;
    float3 c;

    Triangle() {}
    Triangle(const float3 &a, const float3 &b, const float3 &c);

    /// Expresses the given point in barycentric (u,v,w) coordinates with respect to this triangle.
    float3 Barycentric(const float3 &point) const;

    /// Returns true if the given barycentric coordinates lie inside a triangle.
    /// That is, 0 <= u,v,w <= 1 and u+v+w==1.
    static bool BarycentricInsideTriangle(const float3 &barycentric);

    /// Returns a point at the given barycentric coordinates.
    /// The inputted u, v and w should sum up to 1.
    float3 Point(float u, float v, float w) const;
    float3 Point(float u, float v) const { return Point(u, v, 1.f - u - v); }
    float3 Point(const float3 &barycentric) const;

    /// Returns the surface area of this triangle.
    float Area() const;

    /// Returns the plane this triangle lies on.
    Plane GetPlane() const;

    /// Returns the triangle normal. This points to the direction where this plane is viewed to wind in CCW direction.
    float3 Normal() const;

    /// Returns the unnormalized triangle normal.
    float3 UnnormalizedNormal() const;

    /// Returns the surface area of the given 2D triangle.
    static float Area2D(const float2 &p1, const float2 &p2, const float2 &p3);

    /// Relates the barycentric coordinate of the given point to the surface area of triangle abc.
    static float SignedArea(const float3 &point, const float3 &a, const float3 &b, const float3 &c);

    /// Returns true if this triangle is degenerate.
    /// @param epsilon If two of the vertices of this triangle are closer than this, the triangle is considered
    /// degenerate.
    bool IsDegenerate(float epsilon = 1e-3f) const;

    /// Returns true if the given triangle defined by the three given points is degenerate (zero surface area).
    static bool IsDegenerate(const float3 &p1, const float3 &p2, const float3 &p3, float epsilon = 1e-3f);

    /// Returns true if the given point is contained in this triangle.
    /// @param triangleThickness The epsilon value to use for this test. This specifies the maximum distance
    /// the point lie from the plane defined by this triangle.
    bool Contains(const float3 &point, float triangleThickness = 1e-3f) const;

    /// Returns the distance of the given point to this triangle.
    bool Distance(const float3 &point);

    bool Intersects(const LineSegment &lineSegment, float *d, float3 *intersectionPoint) const;
    bool Intersects(const Line &line, float *d, float3 *intersectionPoint) const;
    bool Intersects(const Ray &ray, float *d, float3 *intersectionPoint) const;
    bool Intersects(const Plane &plane) const;
    bool Intersects(const Sphere &sphere, float3 *closestPointOnTriangle) const;
    /// @param outLine [out] Specifies the line segment of intersection if the two triangles intersect.
    bool Intersects(const Triangle &triangle, LineSegment *outLine) const;
    bool Intersects(const AABB &aabb) const;
    bool Intersects(const OBB &obb) const;

    /// Projects this Triangle to the given axis.
    /// @param axis The axis to project onto. This vector can be unnormalized.
    /// @param dMin [out] Returns the minimum extent of this triangle on the given axis.
    /// @param dMax [out] Returns the maximum extent of this triangle on the given axis.
    void ProjectToAxis(const float3 &axis, float &dMin, float &dMax) const;

    /// Returns the closest point on this triangle to the target object.
    float3 ClosestPoint(const float3 &targetPoint) const;
    float3 ClosestPoint(const LineSegment &other, float3 *otherPt) const;
    float3 ClosestPoint(const Ray &other, float3 *otherPt) const;
    float3 ClosestPoint(const Line &other, float3 *otherPt) const;
    float3 ClosestPoint(const Triangle &other, float3 *otherPt) const;
};

Triangle operator *(const float3x3 &transform, const Triangle &t);
Triangle operator *(const float3x4 &transform, const Triangle &t);
Triangle operator *(const float4x4 &transform, const Triangle &t);
Triangle operator *(const Quat &transform, const Triangle &t);

#ifdef QT_INTEROP
Q_DECLARE_METATYPE(Triangle)
Q_DECLARE_METATYPE(Triangle*)
#endif
