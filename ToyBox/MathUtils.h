#pragma once
#include <cmath>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include "Vector2.h"
#include "Vector3.h"
#include "Ray.h"

namespace MOON {
	#define MOON_m 0x100000000LL
	#define MOON_c 0xB16
	#define MOON_a 0x5DEECE66DLL
	/// actually it's pow(255, 4) = 0xfc05fc01
	/// but range of int is ±2147483647, smaller than 0xfc05fc01
	#define MOON_IDBOUNDARY std::numeric_limits<int>::max()
	#define PI acos(-1.0)
	#define Rad2Deg 180/PI
	#define Deg2Rad PI/180
	#define INFINITY std::numeric_limits<float>::max()
	#define EPSILON 0.001

	class MoonMath {
	private:
		static unsigned long long seed;
	public:

		inline static std::vector<int> TENtoNBase(int num, int n) {
			std::vector<int> v;
			while (num) {
				v.push_back(num % n);
				num /= n;
			}
			return v;
		}

		inline static void GammaCorrection(Vector3 &GammaSpaceCol) {
			GammaSpaceCol.x = sqrt(GammaSpaceCol.x);
			GammaSpaceCol.y = sqrt(GammaSpaceCol.y);
			GammaSpaceCol.z = sqrt(GammaSpaceCol.z);
		}

		inline static double drand48() {
			seed = (MOON_a * seed + MOON_c) & 0xFFFFFFFFFFFFLL;
			unsigned int x = seed >> 16;
			return  ((double)x / (double)MOON_m);
		}

		inline static void srand48(unsigned int i) {
			seed = (((long long int)i) << 16) | rand();
		}

		inline static float clamp(const float &num, const float &min, const float &max) {
			return std::max(min, std::min(max, num));
		}

		inline static Vector3 RandomInUnitSphere() { // get a random vector inside unit sphere
			Vector3 p;
			do {
				p = 2.0 * Vector3(drand48(), drand48(), drand48()) - Vector3::ONE();
			} while (p.MagSquared() >= 1.0);
			return p;
		}

		inline static Vector3 RandomInUnitDisk() {
			Vector3 p;
			do {
				p = 2.0 * Vector3(drand48(), drand48(), 0) - Vector3(1, 1, 0);
			} while (p.dot(p) >= 1.0);
			return p;
		}

		inline static Vector3 Reflect(const Vector3 &v, const Vector3 &n) {
			return v - 2 * v.dot(n) * n;
		}

		inline static bool Refract(const Vector3 &v, const Vector3 &n, const float &ni_over_nt, Vector3 &refracted) {
			Vector3 uv = Vector3::Normalize(v);
			float dt = uv.dot(n);
			float discriminant = 1.0 - pow(ni_over_nt, 2) * (1 - pow(dt, 2));
			if (discriminant > 0) {
				refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
				return true;
			} else return false;
		}

		inline static float Schlick(float cosine, float ref_idx) {
			float r0 = (1 - ref_idx) / (1 + ref_idx);
			r0 = r0 * r0;
			return r0 + (1 - r0) * pow((1 - cosine), 5);
		}

		inline static bool RayTriangleIntersect(const Ray &ray, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, float &t, float &u, float &v) {
			Vector3 v0v1 = v1 - v0;
			Vector3 v0v2 = v2 - v0;
			Vector3 pvec = ray.dir.cross(v0v2);
			float det = v0v1.dot(pvec);

			// ray and triangle are parallel if det is close to 0
			if (fabs(det) < EPSILON) return false;

			float invDet = 1.0 / det;
			Vector3 tvec = ray.pos - v0;
			u = tvec.dot(pvec) * invDet;
			if (u < 0 || u > 1) return false;

			Vector3 qvec = tvec.cross(v0v1);
			v = ray.dir.dot(qvec) * invDet;
			if (v < 0 || u + v > 1) return false;

			t = v0v2.dot(qvec) * invDet;

			//return (t > 0) ? true : false;
			return true;
		}

		// A test to see if P1 is on the same side as P2 of a line segment ab
		inline static bool SameSide(const Vector3 &p1, const Vector3 &p2, const Vector3 &a, const Vector3 &b) {
			Vector3 cp1 = Vector3::Cross(b - a, p1 - a);
			Vector3 cp2 = Vector3::Cross(b - a, p2 - a);

			if (Vector3::Dot(cp1, cp2) >= 0)
				return true;
			else
				return false;
		}

		// Generate a cross produect normal for a triangle
		inline static Vector3 GenTriNormal(const Vector3 &t1, const Vector3 &t2, const Vector3 &t3) {
			Vector3 u = t2 - t1;
			Vector3 v = t3 - t1;

			Vector3 normal = Vector3::Cross(u, v);

			return normal;
		}

		// Check to see if a Vector3 Point is within a 3 Vector3 Triangle
		inline static bool IsInTriangle(const Vector3 &point, const Vector3 &tri1, const Vector3 &tri2, const Vector3 &tri3) {
			// Test to see if it is within an infinite prism that the triangle outlines.
			bool within_tri_prisim = SameSide(point, tri1, tri2, tri3) &&
									 SameSide(point, tri2, tri1, tri3) &&
									 SameSide(point, tri3, tri1, tri2);

			// If it isn't it will never be on the triangle
			if (!within_tri_prisim) return false;

			// Calulate Triangle's Normal
			Vector3 n = GenTriNormal(tri1, tri2, tri3);

			// Project the point onto this normal
			Vector3 proj = Vector3::Projection(point, n);

			// If the distance from the triangle to the point is 0，it lies on the triangle
			if (proj.magnitude() == 0) return true;
			else return false;
		}

		// Rotate 2D point around origin point (0, 0)
		inline static Vector2 RotateAround(const Vector2 &targetPoint, const Vector2 &rotCenter, const float theta) {
			// discard y val
			float cx = rotCenter.x, cy = rotCenter.y; // rotCenter
			float px = targetPoint.x, py = targetPoint.y; // point

			float s = sin(theta);
			float c = cos(theta);

			// translate point back to rotCenter:
			px -= cx;
			py -= cy;

			// rotate point
			float xnew = px * c + py * s;
			float ynew = -px * s + py * c;

			// translate point back:
			px = xnew + cx;
			py = ynew + cy;

			return Vector2(px, py);
		}

		// Rotate 3D point around given axis
		// *NOTE: not tested yet!
		static Vector3 RotateAround(const Vector3 &position, const Vector3 &center, const Vector3 &axis, const float &angle);

		/// <summary>
		/// 计算射线与平面的空间状态判断
		/// 平面法线方向必须是射线起点所在的平面一侧方向
		/// 返回等于0时,射线与平面平行,无交点
		/// 返回大于0时,射线射向与平面相反的方向,无交点
		/// </summary>
		inline static float IsRayPlaneIntersect(const Ray &ray, Vector3 planeNormal, const Vector3 &planePoint, const bool cullBack = true) {
			if (!cullBack) planeNormal *= Vector3::Dot(planePoint - ray.pos, planeNormal) > 0 ? -1 : 1;
			return Vector3::Dot(ray.dir, planeNormal);
		}

		inline static Vector3 RayPlaneIntersect(const Ray &ray, const Vector3 &planeNormal, const Vector3 &planePoint) {
			float d = Vector3::Dot(planePoint - ray.pos, planeNormal) / Vector3::Dot(ray.dir, planeNormal);
			return ray.PointAtParameter(d);
		}

		/// <summary>
		/// 计算空间线段与平面的交点
		/// accuracy（容差）越小对于直线端点刚好为交点的情况求交越精确
		/// </summary>
		inline static bool LinePlaneIntersect(const Vector3 &pointA, const Vector3 &pointB, const Vector3 &planeNormal, const Vector3 &planePoint, Vector3 &intersect, const float &accuracy = 0.0001f) {
			// first, treat the line as an ray
			Vector3 direction = pointB - pointA;
			int determine = Vector3::Dot(planePoint - pointA, planeNormal) > 0 ? -1 : 1;

			// check for intersect
			if (IsRayPlaneIntersect(Ray(pointA, direction), planeNormal * determine, planePoint) < 0) {
				// calculate intersect point
				intersect = RayPlaneIntersect(Ray(pointA, Vector3::Normalize(direction)), planeNormal * determine, planePoint);
				// check if the intersect point is inside line
				if ((intersect - pointA).magnitude() <= (direction.magnitude() + accuracy)) return true;
			}

			return false;
		}

		inline static bool RayDiskIntersect(const Ray& ray, const Vector3 &diskPoint, const Vector3 &diskNormal, const float &diskRadius, Vector3 &intersect, const float &accuracy = 0.0001f) {
			int determine = Vector3::Dot(diskPoint - ray.pos, diskNormal) > 0 ? -1 : 1;

			if (IsRayPlaneIntersect(Ray(ray.pos, ray.dir), diskNormal * determine, diskPoint) < 0) {
				intersect = RayPlaneIntersect(Ray(ray.pos, Vector3::Normalize(ray.dir)), diskNormal * determine, diskPoint);
				if ((intersect - diskPoint).magnitude() <= diskRadius + accuracy) return true;
			}

			return false;
		}

		inline static float PointLineDistance(const Vector3 &point, const Vector3 &linePoint1, const Vector3 &linePoint2) {
			float fProj = Vector3::Dot(point - linePoint1, Vector3::Normalize(linePoint1 - linePoint2));
			return std::sqrtf((point - linePoint1).magnitude() - fProj * fProj);
		}

		// ---------------------------------------------------------------------------------
		inline static float Determinant(const Vector3 &a, const Vector3 &v1, const Vector3 &v2) {
			return a[0] * (v1[1] * v2[2] - v1[2] * v2[1]) + a[1] * (v1[2] * v2[0] - v1[0] * v2[2]) + a[2] * (v1[0] * v2[1] - v1[1] * v2[0]);
		}
		inline static float closestDistanceBetweenLines(const Ray &a0, const Ray &a1, Vector3 &p0, Vector3 &p1, const float &len0 = 1.0f, const float &len1 = 1.0f) {
			return closestDistanceBetweenLines(a0.pos, a0.PointAtParameter(len0), a1.pos, a1.PointAtParameter(len1), p0, p1);
		}
		// Given two lines defined by (a0,a1,b0,b1)
		// Return the closest points on each segment and their distance
		static float closestDistanceBetweenLines(const Vector3 &a0, const Vector3 &a1, const Vector3 &b0, const Vector3 &b1, Vector3 &Line1Closest, Vector3 &Line2Closest);

	};
}