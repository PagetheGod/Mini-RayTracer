#pragma once

#include "HittableList.h"


class Camera
{
public:
	Camera() = default;
	void Initialize();
	void RenderWorld(HittableList& World);


public:

private:
	Color CalculateHitColor(const Ray& R, HittableList& World);
};