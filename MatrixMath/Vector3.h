#pragma once
struct Vector3
{
public:
	double x;
	double y;
	double z;
	Vector3 operator +(Vector3 other) {
		return {
			x + other.x,
			y + other.y,
			z + other.z,
		};
	}
	Vector3 operator -(Vector3 other) {
		return {
			x - other.x,
			y - other.y,
			z - other.z,
		};
	}
	double dot(Vector3 other) const {
		return x * other.x + y * other.y + z * other.z;
	}
	Vector3 cross(Vector3 other) {
		//v × w = (v₂w₃ - v₃w₂, v₃w₁ - v₁w₃, v₁w₂ - v₂w₁)
		return {
			y*other.z - z*other.y,
			z*other.x - x*other.z,
			x*other.y - y*other.x,
		};
	}
private:
};

