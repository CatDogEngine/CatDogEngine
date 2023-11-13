// Test functions are generated by GPT 3.5.

#include <cassert>

#include "Math/Quaternion.hpp"

void TestVector()
{
	// Test TVector::Zero and TVector::One
	{
		cd::TVector<int, 3> v1 = cd::TVector<int, 3>::Zero();
		assert(v1[0] == 0 && v1[1] == 0 && v1[2] == 0);

		cd::TVector<float, 2> v2 = cd::TVector<float, 2>::One();
		assert(v2[0] == 1.0f && v2[1] == 1.0f);
	}

	// Test TVector::Lerp
	{
		cd::TVector<float, 2> v1(1.0f, 2.0f);
		cd::TVector<float, 2> v2(3.0f, 6.0f);
		cd::TVector<float, 2> lerpV = cd::TVector<float, 2>::Lerp(v1, v2, 0.5f);

		assert(lerpV[0] == 2.0f && lerpV[1] == 4.0f);
	}

	// Test TVector::GetUpAxis
	{
		cd::AxisSystem axisSystem(cd::Handedness::Left, cd::UpVector::ZAxis, cd::FrontVector::ParityEven);
		cd::TVector<float, 3> upAxis = cd::TVector<float, 3>::GetUpAxis(axisSystem);

		assert(upAxis[0] == 0.0f && upAxis[1] == 0.0f && upAxis[2] == 1.0f);
	}

	// Test TVector::GetFrontAxis
	{
		cd::AxisSystem axisSystem(cd::Handedness::Left, cd::UpVector::ZAxis, cd::FrontVector::ParityEven);
		cd::TVector<float, 3> frontAxis = cd::TVector<float, 3>::GetFrontAxis(axisSystem);

		assert(frontAxis[0] == 1.0f && frontAxis[1] == 0.0f && frontAxis[2] == 0.0f);
	}

	// Test TVector constructors and member functions
	{
		// Default constructor
		cd::TVector<int, 3> v1;

		// Single value constructor
		cd::TVector<double, 4> v2(1.0);
		assert(v2[0] == 1.0 && v2[1] == 1.0 && v2[2] == 1.0 && v2[3] == 1.0);

		// N parameters constructor
		cd::TVector<float, 2> v3(1.0f, 2.0f);
		assert(v3[0] == 1.0f && v3[1] == 2.0f);

		// Set
		v1.Set(1);
		assert(v1[0] == 1 && v1[1] == 1 && v1[2] == 1);

		// Clear
		v1.Clear();
		assert(v1[0] == 0 && v1[1] == 0 && v1[2] == 0);
	}

	// Test copy construction and assignment
	{
		cd::TVector<int, 2> vec1(1, 2);
		cd::TVector<int, 2> vec2(vec1);
		assert(vec2[0] == 1 && vec2[1] == 2);

		cd::TVector<int, 2> vec3;
		vec3 = vec1;
		assert(vec3[0] == 1 && vec3[1] == 2);
	}

	// Test iterator access
	{
		cd::TVector<int, 4> vec(1, 2, 3, 4);
		int sum = 0;
		for (auto value : vec)
		{
			sum += value;
		}
		assert(sum == 10);
	}

	// Test element access
	{
		cd::TVector<float, 3> vec(1.0f, 2.0f, 3.0f);
		assert(vec[0] == 1.0f && vec[1] == 2.0f && vec[2] == 3.0f);
		vec[0] = 4.0f;
		assert(vec[0] == 4.0f);
	}

	// Test named element access
	{
		cd::TVector<double, 4> vec(1.0, 2.0, 3.0, 4.0);
		assert(vec.x() == 1.0 && vec.y() == 2.0 && vec.z() == 3.0 && vec.w() == 4.0);
		vec.x() = 5.0;
		assert(vec.x() == 5.0);
	}

	// Test sub-vector access
	{
		cd::TVector<int, 4> vec(1, 2, 3, 4);
		auto subVec = vec.xxx();
		assert(subVec[0] == 1 && subVec[1] == 1 && subVec[2] == 1);

		subVec = vec.zzz();
		assert(subVec[0] == 3 && subVec[1] == 3 && subVec[2] == 3);

		subVec = vec.xyz();
		assert(subVec[0] == 1 && subVec[1] == 2 && subVec[2] == 3);
	}

	// Test NaN validation
	{
		cd::TVector<double, 3> vec1(0.0, std::nan(""), 0.0);
		assert(vec1.ContainsNan() == true);
		assert(vec1.IsValid() == false);

		cd::TVector<float, 2> vec2(0.0f, 1.0f);
		assert(vec2.ContainsNan() == false);
	}

	// Test zero validation
	{
		cd::TVector<double, 3> vec1(0.0, 0.0, 0.0);
		assert(vec1.SameWith(0.0f) == true);

		cd::TVector<float, 2> vec2(0.0f, 1.0f);
		assert(vec2.SameWith(0.0f) == false);
	}

	// Test Contains()
	{
		cd::TVector<double, 2> v(1.1, 2.2);
		assert(v.Contains(1.1) && v.Contains(2.2) && !v.Contains(3.3));
	}

	// Test Sum()
	{
		cd::TVector<int, 4> v(1, 2, 3, 4);
		assert(v.Sum() == 10);
	}

	// Test Length() and LengthSquare()
	{
		cd::TVector<float, 3> v(1.f, 2.f, 3.f);
		assert(std::abs(v.Length() - std::sqrt(14.f)) < 1e-6f && v.LengthSquare() == 14.f);
	}

	// Test Normalize()
	{
		cd::TVector<double, 4> v(1.0, 2.0, 3.0, 4.0);
		v.Normalize();
		assert(std::abs(v.Length() - 1.0) < 1e-9 && std::abs(v[0] - 0.1825741858) < 1e-9 &&
			std::abs(v[1] - 0.3651483717) < 1e-9 && std::abs(v[2] - 0.5477225575) < 1e-9 &&
			std::abs(v[3] - 0.7302967433) < 1e-9);
	}

	// Test Dot()
	{
		cd::TVector<double, 3> v1(1.0, 2.0, 3.0), v2(-1.0, 2.0, -3.0);
		assert(std::abs(v1.Dot(v2) + 6.0) < 1e-12);
	}

	// Test Cross()
	{
		cd::TVector<double, 3> v1(1.0, 2.0, 3.0), v2(4.0, 5.0, 6.0), result(-3.0, 6.0, -3.0);
		assert(v1.Cross(v2) == result);
	}

	// Test operator== and operator!=
	{
		cd::TVector<double, 2> v1{ 1.0, 2.0 };
		cd::TVector<double, 2> v2{ 1.0, 2.0 };
		cd::TVector<double, 2> v3{ 1.0, 3.0 };
		assert(v1 == v2);
		assert(v1 != v3);
	}

	// Test operator+ and operator+=
	{
		cd::TVector<int, 3> v1{ 1, 2, 3 };
		cd::TVector<int, 3> v2{ 4, 5, 6 };
		cd::TVector<int, 3> v3 = v1 + v2;
		assert(v3.x() == 5 && v3.y() == 7 && v3.z() == 9);

		v1 += v2;
		assert(v1 == v3);
	}

	// Test operator- and operator-=
	{
		cd::TVector<float, 2> v1{ 2.0f, 3.0f };
		cd::TVector<float, 2> v2{ 1.0f, 1.0f };
		cd::TVector<float, 2> v3 = v1 - v2;
		assert(v3[0] == 1.0f && v3[1] == 2.0f);

		v1 -= v2;
		assert(v1 == v3);
	}

	// Test xxx, yyy, zzz, xyz
	{
		cd::TVector<double, 4> v{ 1.0, 2.0, 3.0, 4.0 };
		assert((v.xxx() == cd::TVector<double, 3>{1.0, 1.0, 1.0}));
		assert((v.yyy() == cd::TVector<double, 3>{2.0, 2.0, 2.0}));
		assert((v.zzz() == cd::TVector<double, 3>{3.0, 3.0, 3.0}));
		assert((v.xyz() == cd::TVector<double, 3>{1.0, 2.0, 3.0}));
	}
}

void TestQuaternion()
{
	// test TQuaternion::Identity
	{
		const auto q = cd::TQuaternion<float>::Identity();
		assert(q.w() == 1.0f);
		assert(q.x() == 0.0f);
		assert(q.y() == 0.0f);
		assert(q.z() == 0.0f);
	}

	// test TQuaternion::RotateX
	{
		const auto q = cd::TQuaternion<float>::RotateX(3.1415926f);
		assert(q.w() == std::cos(3.1415926f / 2));
		assert(q.x() == std::sin(3.1415926f / 2));
		assert(q.y() == 0.0f);
		assert(q.z() == 0.0f);
	}

	// test TQuaternion::RotateY
	{
		const auto q = cd::TQuaternion<float>::RotateY(3.1415926f);
		assert(q.w() == std::cos(3.1415926f / 2));
		assert(q.x() == 0.0f);
		assert(q.y() == std::sin(3.1415926f / 2));
		assert(q.z() == 0.0f);
	}

	// test TQuaternion::RotateZ
	{
		const auto q = cd::TQuaternion<float>::RotateZ(3.1415926f);
		assert(q.w() == std::cos(3.1415926f / 2));
		assert(q.x() == 0.0f);
		assert(q.y() == 0.0f);
		assert(q.z() == std::sin(3.1415926f / 2));
	}

	// test TQuaternion::FromAxisAngle
	{
		const cd::TVector<float, 3> axis(1.0f, 0.0f, 0.0f);
		const auto q = cd::TQuaternion<float>::FromAxisAngle(axis, 3.1415926f);
		assert(q.w() == std::cos(3.1415926f / 2));
		assert(q.x() == std::sin(3.1415926f / 2));
		assert(q.y() == 0.0f);
		assert(q.z() == 0.0f);
	}

	// test TQuaternion::FromPitchYawRoll
	//{
	//	const auto q = cd::TQuaternion<float>::FromPitchYawRoll(45.0f, 90.0f, 180.0f);
	//	const auto expected_q = cd::TQuaternion<float>(0.270598f, 0.653281f, -0.270598f, 0.653281f);
	//	const float epsilon = 0.0001f;
	//	assert(std::abs(q.w() - expected_q.w()) < epsilon);
	//	assert(std::abs(q.x() - expected_q.x()) < epsilon);
	//	assert(std::abs(q.y() - expected_q.y()) < epsilon);
	//	assert(std::abs(q.z() - expected_q.z()) < epsilon);
	//}

		// Test for Lerp
	//{
	//	cd::TQuaternion<float> a(0, 0, 0, 1);
	//	cd::TQuaternion<float> b(0, 0, 1, 0);
	//	cd::TQuaternion<float> c = cd::TQuaternion<float>::Lerp(a, b, 0.5f);
	//	assert(std::fabs(c.x() - 0) < 1e-6f);
	//	assert(std::fabs(c.y() - 0) < 1e-6f);
	//	assert(std::fabs(c.z() - 0.707106f) < 1e-6f);
	//	assert(std::fabs(c.w() - 0.707106f) < 1e-6f);
	//
	//	cd::TQuaternion<double> d(0, 0, 0, 1);
	//	cd::TQuaternion<double> e(0, 0, 1, 0);
	//	cd::TQuaternion<double> f = cd::TQuaternion<double>::Lerp(d, e, 0.5);
	//	assert(std::fabs(f.x() - 0) < 1e-10);
	//	assert(std::fabs(f.y() - 0) < 1e-10);
	//	assert(std::fabs(f.z() - 0.7071067811) < 1e-10);
	//	assert(std::fabs(f.w() - 0.7071067811) < 1e-10);
	//}

	// Test for LerpNormalized
	//{
	//	cd::TQuaternion<float> a(0, 0, 0, 1);
	//	cd::TQuaternion<float> b(0, 0, 1, 0);
	//	cd::TQuaternion<float> c = cd::TQuaternion<float>::LerpNormalized(a, b, 0.5f);
	//	assert(std::fabs(c.x() - 0) < 1e-6f);
	//	assert(std::fabs(c.y() - 0) < 1e-6f);
	//	assert(std::fabs(c.z() - 0.707106f) < 1e-6f);
	//	assert(std::fabs(c.w() - 0.707106f) < 1e-6f);
	//
	//	cd::TQuaternion<double> d(0, 0, 0, 1);
	//	cd::TQuaternion<double> e(0, 0, 1, 0);
	//	cd::TQuaternion<double> f = cd::TQuaternion<double>::LerpNormalized(d, e, 0.5);
	//	assert(std::fabs(f.x() - 0) < 1e-10);
	//	assert(std::fabs(f.y() - 0) < 1e-10);
	//	assert(std::fabs(f.z() - 0.7071067811) < 1e-10);
	//	assert(std::fabs(f.w() - 0.7071067811) < 1e-10);
	//}
}

int main()
{
	TestVector();
	TestQuaternion();

	return 0;
}