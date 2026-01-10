#include "../Public/Color.h"

void WriteColor(std::ostream& OutFileStream, const Color& PixelColor)
{
	float R = PixelColor.R();
	float G = PixelColor.G();
	float B = PixelColor.B();

	//In case we want to run to a ppm file, not needed for direct render to screen
	int AdjustedRed = 255.999f * R;
	int AdjustedGreen = 255.999f * G;
	int AdjustedBlue = 255.999f * B;

	OutFileStream << AdjustedRed << ' ' << AdjustedGreen << ' ' << AdjustedBlue << '\n';
}
