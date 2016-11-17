#pragma once

class Util
{
public:
	static unsigned long long GetCurrentMilis();
};

class Vec3
{
public:
	Vec3(float _r, float _g, float _b)
	: r(_r)
	, g(_g)
	, b(_b)
	{}

	Vec3(const Vec3 &src)
	{
		*this = src;
	}

	Vec3& operator =(const Vec3 &src)
	{
		r = src.r;
		g = src.g;
		b = src.b;
		return *this;
	}

	bool operator ==(const Vec3 &src)
	{
		return r == src.r && g == src.g && b == src.b;
	}

	static const Vec3 GetZero()
	{
		return cZero;
	}

	float r;
	float g;
	float b;

private:
	static const Vec3 cZero;
};
