// Fill out your copyright notice in the Description page of Project Settings.

#include "PerlinNoise.h"

UPerlinNoise::UPerlinNoise()
{
	//p = new int32[512];
	p.Init(0, 512);
	for (int32 x = 0; x < 512; x++)
		p[x] = permutation[x % 256];
}

float UPerlinNoise::OctavePerlin(float x, float y, float z, int32 octaves, float persistence)
{
	float total = 0;
	float frequency = 1;
	float amplitude = 1;
	float maxValue = 0;			// Used for normalizing result to 0.0 - 1.0
	for (int32 i = 0; i < octaves; i++)
	{
		total += SimplePerlin(x * frequency, y * frequency, z * frequency) * amplitude;

		maxValue += amplitude;

		amplitude *= persistence;
		frequency *= 2;
	}

	return total / maxValue;
}

float UPerlinNoise::SimplePerlin(float x, float y, float z)
{
	if (repeat > 0)
	{									// If we have any repeat on, change the coordinates to their "local" repetitions
		x = (int32)x % repeat;
		y = (int32)y % repeat;
		z = (int32)z % repeat;
	}

	int32 xi = (int32)x & 255;								// Calculate the "unit cube" that the point32 asked will be located in
	int32 yi = (int32)y & 255;								// The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
	int32 zi = (int32)z & 255;								// plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
	float xf = x - (int32)x;								// We also fade the location to smooth the result.
	float yf = y - (int32)y;
	float zf = z - (int32)z;
	float u = fade(xf);
	float v = fade(yf);
	float w = fade(zf);

	int32 aaa, aba, aab, abb, baa, bba, bab, bbb;
	aaa = p[p[p[xi] + yi] + zi];
	aba = p[p[p[xi] + inc(yi)] + zi];
	aab = p[p[p[xi] + yi] + inc(zi)];
	abb = p[p[p[xi] + inc(yi)] + inc(zi)];
	baa = p[p[p[inc(xi)] + yi] + zi];
	bba = p[p[p[inc(xi)] + inc(yi)] + zi];
	bab = p[p[p[inc(xi)] + yi] + inc(zi)];
	bbb = p[p[p[inc(xi)] + inc(yi)] + inc(zi)];

	float x1, x2, y1, y2;
	x1 = lerp(grad(aaa, xf, yf, zf),				// The gradient function calculates the dot product between a pseudorandom
		grad(baa, xf - 1, yf, zf),				// gradient vector and the vector from the input coordinate to the 8
		u);										// surrounding point32s in its unit cube.
	x2 = lerp(grad(aba, xf, yf - 1, zf),				// This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
		grad(bba, xf - 1, yf - 1, zf),				// values we made earlier.
		u);
	y1 = lerp(x1, x2, v);

	x1 = lerp(grad(aab, xf, yf, zf - 1),
		grad(bab, xf - 1, yf, zf - 1),
		u);
	x2 = lerp(grad(abb, xf, yf - 1, zf - 1),
		grad(bbb, xf - 1, yf - 1, zf - 1),
		u);
	y2 = lerp(x1, x2, v);

	return (lerp(y1, y2, w) + 1) / 2;						// For convenience we bound it to 0 - 1 (theoretical min/max before is -1 - 1)
}

int32 UPerlinNoise::inc(int32 num)
{
	num++;
	if (repeat > 0)
		num %= repeat;

	//UE_LOG(LogTemp, Warning, TEXT("inc %d"), *num);
	return num;
}

float UPerlinNoise::grad(int32 hash, float x, float y, float z)
{
	int32 h = hash & 15;									// Take the hashed value and take the first 4 bits of it (15 == 0b1111)
	float u = h < 8 /* 0b1000 */ ? x : y;				// If the most significant bit (MSB) of the hash is 0 then set u = x.  Otherwise y.

	float v;											// In Ken Perlin's original implementation this was another conditional operator (?:).  I
														// expanded it for readability.

	if (h < 4 /* 0b0100 */)								// If the first and second significant bits are 0 set v = y
		v = y;
	else if (h == 12 /* 0b1100 */ || h == 14 /* 0b1110*/)// If the first and second significant bits are 1 set v = x
		v = x;
	else 												// If the first and second significant bits are not equal (0/1, 1/0) set v = z
		v = z;

	float val = ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	//UE_LOG(LogTemp, Warning, TEXT("grad %f"), val);
	return val; // Use the last 2 bits to decide if u and v are positive or negative.  Then return their addition.
}

float UPerlinNoise::fade(float t)
{
	// Fade function as defined by Ken Perlin.  This eases coordinate values
	// so that they will "ease" towards int32egral values.  This ends up smoothing
	// the final output.
	float val = t * t * t * (t * (t * 6 - 15) + 10);
	//UE_LOG(LogTemp, Warning, TEXT("fade %f"), val);
	return val;			// 6t^5 - 15t^4 + 10t^3
}

float UPerlinNoise::lerp(float a, float b, float x)
{
	float val = a + x * (b - a);
	//UE_LOG(LogTemp, Warning, TEXT("lerp %f"), val);
	return val;
}


