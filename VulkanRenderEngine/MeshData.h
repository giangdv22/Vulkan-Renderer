#pragma once

struct VertexWithColor
{
	float x, y, z, w; // Vertex Pos
	float r, g, b, a; // Color
};

struct VertexWithUV
{
	float x, y, z, w;
	float u, v;
};

static const VertexWithUV textureData[] = {
{ -1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f }, // -X side
{ -1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
{ -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
{ -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
{ -1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 0.0f },
{ -1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f },
};
static const VertexWithColor triangleData[] =
{
	{ 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0 },
	{ 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0 },
	{ -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0 }
};

static const VertexWithColor squareData[] =
{
{ -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0 },
{ 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0 },
{ 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0 },
{ -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0 },
};

uint32_t squareIndices[] = { 0,1,2, 2,3,0 };

static const VertexWithColor geometryData[] =
{
	{  1, -1, -1, 1.0f,		0.f, 0.f, 0.f, 1.0f },
	{ -1, -1, -1, 1.0f,		1.f, 0.f, 0.f, 1.0f },
	{  1,  1, -1, 1.0f,		0.f, 1.f, 0.f, 1.0f },
	{  1,  1, -1, 1.0f,		0.f, 1.f, 0.f, 1.0f },
	{ -1, -1, -1, 1.0f,		1.f, 0.f, 0.f, 1.0f },
	{ -1,  1, -1, 1.0f,		1.f, 1.f, 0.f, 1.0f },

	{  1, -1, 1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{  1,  1, 1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{ -1, -1, 1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{ -1, -1, 1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{  1,  1, 1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{ -1,  1, 1, 1.0f,		1.f, 1.f, 1.f, 1.0f },

	{ 1, -1,  1, 1.0f,		1.f, 1.f, 1.f, 1.0f },
	{ 1, -1, -1, 1.0f,		1.f, 1.f, 0.f, 1.0f },
	{ 1,  1,  1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{ 1,  1,  1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{ 1, -1, -1, 1.0f,		1.f, 1.f, 0.f, 1.0f },
	{ 1,  1, -1, 1.0f,		1.f, 0.f, 0.f, 1.0f },

	{ -1, -1,  1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{ -1,  1,  1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{ -1, -1, -1, 1.0f,		0.f, 1.f, 0.f, 1.0f },
	{ -1, -1, -1, 1.0f,		0.f, 1.f, 0.f, 1.0f },
	{ -1,  1,  1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{ -1,  1, -1, 1.0f,		0.f, 0.f, 0.f, 1.0f },

	{  1, 1, -1, 1.0f,		1.f, 1.f, 1.f, 1.0f },
	{ -1, 1, -1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{  1, 1,  1, 1.0f,		1.f, 1.f, 0.f, 1.0f },
	{  1, 1,  1, 1.0f,		1.f, 1.f, 0.f, 1.0f },
	{ -1, 1, -1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{ -1, 1,  1, 1.0f,		0.f, 1.f, 0.f, 1.0f },

	{  1, -1, -1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{  1, -1,  1, 1.0f,		1.f, 0.f, 0.f, 1.0f },
	{ -1, -1, -1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{ -1, -1, -1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{  1, -1,  1, 1.0f,		1.f, 0.f, 0.f, 1.0f },
	{ -1, -1,  1, 1.0f,		0.f, 0.f, 0.f, 1.0f },
};