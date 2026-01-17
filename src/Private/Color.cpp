#include "../Public/Color.h"
#include "../Public/Interval.h"

//This function is kind of outdated, keeping it here so the RenderToPPM function does not break
void WriteColor(std::ostream& OutFileStream, const Color& PixelColor)
{
	float R = PixelColor.R();
	float G = PixelColor.G();
	float B = PixelColor.B();

	//In case we want to run to a ppm file, not needed for direct render to screen
	int AdjustedRed = (int)(255.999f * R);
	int AdjustedGreen = (int)(255.999f * G);
	int AdjustedBlue = (int)(255.999f * B);

	OutFileStream << AdjustedRed << ' ' << AdjustedGreen << ' ' << AdjustedBlue << '\n';
}
//Clamp color to 0 to 1
Color NormalizeColor(const Color& PixelColor)
{
	Interval Intensity(0.f, 0.999f);

	float AdjustedRed = Intensity.Clamp(PixelColor.R());
	float AdjustedGreen = Intensity.Clamp(PixelColor.G());
	float AdjustedBlue = Intensity.Clamp(PixelColor.B());
	
	return Color(AdjustedRed, AdjustedGreen, AdjustedBlue);
}
