#pragma once

#include "Vector3D.h"

using Color = Vector3D;

//A header that defines color aliasing for Vector3D and its related utilities

void WriteColor(std::ostream& OutFileStream, const Color& PixelColor);
Color NormalizeColor(const Color& PixelColor);