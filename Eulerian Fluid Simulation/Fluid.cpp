#include "Fluid.h"

void Fluid::RandomizeVelocities(float t) {
	for (int x = 0; x < cellCountX + 1; x++) {
		for (int y = 0; y < cellCountY; y++) {
			velocityX[indexVX(x, y)] = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * t;
		}
	}

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY + 1; y++) {
			velocityY[indexVY(x, y)] = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * t;
		}
	}
}

Fluid::Fluid(int _cellCountX, int _cellCountY, float _cellSize, float _density, float dt) {
	cellCountX = _cellCountX;
	cellCountY = _cellCountY;
	cellSize = _cellSize;

	density = _density;
	deltaTime = dt;

	velocityX = new float[(cellCountX + 1) * cellCountY]();
	velocityY = new float[cellCountX * (cellCountY + 1)]();

	velocityX_temp = new float[(cellCountX + 1) * cellCountY]();
	velocityY_temp = new float[cellCountX * (cellCountY + 1)]();

	cellPressure = new float[cellCountX * cellCountY]();
	solidCell = new bool[cellCountX * cellCountY]();//Initializes false

	for (int i = 0; i < cellCountX; i++) {// Boundary cells are solid cells
		solidCell[indexXY(i, 0)] = true;
		solidCell[indexXY(i, cellCountY - 1)] = true;
	}

	for (int i = 0; i < cellCountY; i++) {
		solidCell[indexXY(0, i)] = true;
		solidCell[indexXY(cellCountX - 1, i)] = true;
	}

	boundsSize = glm::vec2(static_cast<float>(cellCountX), static_cast<float>(cellCountY)) * cellSize;
	bottomLeft = -(boundsSize / 2.0f);
	halfCellSize = cellSize / 2.0f;
}

std::vector<float> Fluid::GetVelocityMagnitudes() {
	std::vector<float> magnitudes(cellCountX * cellCountY);

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			glm::vec2 vel = GetVelocity(CellCenter(x, y)); // reuses existing bilinear sampling
			magnitudes[indexXY(x, y)] = glm::length(vel);
		}
	}

	return magnitudes;
}

void Fluid::Simulate(int iterations) {
	std::fill(cellPressure, cellPressure + (cellCountX * cellCountY), 0.0f); // resets the Pressure field

	for (int i = 0; i < iterations; i++) {
		PressureSolver();
	}

	UpdateVelocities(); // applying the converged pressure gradient once
	AdvectVelocity(); // moves the now divergence free field through itself
}

void Fluid::AdvectVelocity() {
	for (int x = 0; x < cellCountX + 1; x++){
		for (int y = 0; y < cellCountY; y++){
			if (isSolid(x - 1, y) || isSolid(x, y)){
				velocityX_temp[indexVX(x, y)] =	velocityX[indexVX(x, y)];
				continue;
			}

			glm::vec2 pos = LeftEdgeCenter(x, y);

			glm::vec2 vel = GetVelocity(pos);

			glm::vec2 posPrev =	pos - vel * deltaTime;

			velocityX_temp[indexVX(x, y)] =	GetVelocity(posPrev).x;
		}
	}

	for (int x = 0; x < cellCountX; x++){
		for (int y = 0; y < cellCountY + 1; y++){
			if (isSolid(x, y - 1) || isSolid(x, y)){
				velocityY_temp[indexVY(x, y)] =	velocityY[indexVY(x, y)];

				continue;
			}

			glm::vec2 pos = BottomEdgeCenter(x, y);

			glm::vec2 vel = GetVelocity(pos);

			glm::vec2 posPrev =	pos - vel * deltaTime;

			velocityY_temp[indexVY(x, y)] =	GetVelocity(posPrev).y;
		}
	}

	for (int x = 0; x < cellCountX + 1; x++){
		for (int y = 0; y < cellCountY; y++){
			velocityX[indexVX(x, y)] = velocityX_temp[indexVX(x, y)];
		}
	}

	for (int x = 0; x < cellCountX; x++){
		for (int y = 0; y < cellCountY + 1; y++){
			velocityY[indexVY(x, y)] =	velocityY_temp[indexVY(x, y)];
		}
	}
}

float Fluid::CalcDivergence(int cellX, int cellY) {
	float velocityTop = velocityY[indexVY(cellX, cellY + 1)];
	float velocityLeft = velocityX[indexVX(cellX, cellY)];
	float velocityRight = velocityX[indexVX(cellX + 1, cellY)];
	float velocityBottom = velocityY[indexVY(cellX, cellY)];

	//u_i = (u_i+1/2 - u_i-1/2)/w
	float gradientX = (velocityRight - velocityLeft) / cellSize;
	float gradientY = (velocityTop - velocityBottom) / cellSize;

	return (gradientX + gradientY);
}

void Fluid::UpdateVelocities() {
	for (int i = 0; i < (cellCountX + 1); i++) {
		for (int j = 0; j < cellCountY; j++) {
			if (isSolid(i, j) || isSolid(i - 1, j)) {
				velocityX[indexVX(i, j)] = 0;
				continue;
			}

			float pressureRight = GetPressure(i, j);
			float pressureLeft = GetPressure(i - 1, j);
			velocityX[indexVX(i, j)] -= (deltaTime / (density * cellSize)) * (pressureRight - pressureLeft);
		}
	}

	for (int i = 0; i < cellCountX; i++) {
		for (int j = 0; j < (cellCountY + 1); j++) {
			if (isSolid(i, j) || isSolid(i, j - 1)) {
				velocityY[indexVY(i, j)] = 0;
				continue;
			}

			float pressureTop = GetPressure(i, j);
			float pressureBottom = GetPressure(i, j - 1);
			velocityY[indexVY(i, j)] -= (deltaTime / (density * cellSize)) * (pressureTop - pressureBottom);
		}
	}
}

void Fluid::PressureSolver() {
	for (int i = 0; i < cellCountX; i++) {
		for (int j = 0; j < cellCountY; j++) {
			CellPressureSolver(i, j);
		}
	}
}

void Fluid::CellPressureSolver(int cellX, int cellY) {
	int edgeCount = (isSolid(cellX, cellY + 1) ? 0 : 1) +
		(isSolid(cellX - 1, cellY) ? 0 : 1) +
		(isSolid(cellX + 1, cellY) ? 0 : 1) +
		(isSolid(cellX, cellY - 1) ? 0 : 1);

	if (isSolid(cellX, cellY) || edgeCount == 0) {
		cellPressure[indexXY(cellX, cellY)] = 0;
		return;
	}

	float pressureTop = GetPressure(cellX, cellY + 1);
	float pressureLeft = GetPressure(cellX - 1, cellY);
	float pressureRight = GetPressure(cellX + 1, cellY);
	float pressureBottom = GetPressure(cellX, cellY - 1);

	float velocityTop = velocityY[indexVY(cellX, cellY + 1)];
	float velocityLeft = velocityX[indexVX(cellX, cellY)];
	float velocityRight = velocityX[indexVX(cellX + 1, cellY)];
	float velocityBottom = velocityY[indexVY(cellX, cellY)];

	float totalPressure = pressureTop + pressureLeft + pressureRight + pressureBottom;
	float deltaVelocity = velocityRight - velocityLeft + velocityTop - velocityBottom;

	float newPressure = (totalPressure - density * cellSize * deltaVelocity / deltaTime) / edgeCount;

	float oldPressure = cellPressure[indexXY(cellX, cellY)];

	cellPressure[indexXY(cellX, cellY)] = oldPressure + (newPressure - oldPressure) * 1.7f;
}


float Fluid::Bilinear(float* edgeValues, int edgeCountX, int edgeCountY, float cellSize, glm::vec2 position) {
	float width = (edgeCountX - 1) * cellSize;
	float height = (edgeCountY - 1) * cellSize;

	float px = (position.x + width / 2) / cellSize;// normalized indices of the current cell
	float py = (position.y + height / 2) / cellSize;

	int left = glm::clamp(static_cast<int>(px), 0, edgeCountX - 2);
	int bottom = glm::clamp(static_cast<int>(py), 0, edgeCountY - 2);
	int right = left + 1;
	int top = bottom + 1;

	float xFrac = glm::clamp(px - static_cast<float>(left), 0.0f, 1.0f);// Takes fractional part to calculate how far the point is in the cell
	float yFrac = glm::clamp(py - static_cast<float>(bottom), 0.0f, 1.0f);

	// Helper for accessing flattened 2D data
	auto at = [edgeValues, edgeCountX](int x, int y) -> float
		{
			return edgeValues[x + y * edgeCountX];
		};

	float valueTop = glm::mix(at(left, top), at(right, top), xFrac);// Linear interpolation at top
	float valueBottom =	glm::mix(at(left, bottom), at(right, bottom), xFrac);

	return glm::mix(valueBottom, valueTop, yFrac);// Linear interpolation at the position using top and bottom values
}

glm::vec2 Fluid::GetVelocity(glm::vec2 position) {
	float xVel = Bilinear(velocityX, cellCountX + 1, cellCountY, cellSize, position);
	float yVel = Bilinear(velocityY, cellCountX, cellCountY + 1, cellSize, position);

	return glm::vec2(xVel, yVel);
}

void Fluid::AddVelocity(glm::vec2 uv, glm::vec2 velocity, float radiusCells) {
	glm::vec2 worldPos = bottomLeft + uv * boundsSize;
	float radius = radiusCells * cellSize;

	// Splat onto horizontal velocity edges
	for (int x = 0; x < cellCountX + 1; x++) {
		for (int y = 0; y < cellCountY; y++) {
			if (isSolid(x - 1, y) || isSolid(x, y)) 
				continue;

			float dist = glm::length(LeftEdgeCenter(x, y) - worldPos);
			if (dist > radius) 
				continue;

			float falloff = 1.0f - (dist / radius); // linear falloff, full strength at center
			velocityX[indexVX(x, y)] += velocity.x * falloff;
		}
	}

	// Splat onto vertical velocity edges
	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY + 1; y++) {
			if (isSolid(x, y - 1) || isSolid(x, y)) 
				continue;

			float dist = glm::length(BottomEdgeCenter(x, y) - worldPos);
			if (dist > radius) 
				continue;

			float falloff = 1.0f - (dist / radius);
			velocityY[indexVY(x, y)] += velocity.y * falloff;
		}
	}
}

glm::vec2 Fluid::CellCenter(int x, int y)
{
	return bottomLeft +	glm::vec2(x + 0.5f,	y + 0.5f) * cellSize;
}

glm::vec2 Fluid::LeftEdgeCenter(int x, int y)
{
	return CellCenter(x, y) - glm::vec2(halfCellSize, 0.0f);
}

glm::vec2 Fluid::BottomEdgeCenter(int x, int y)
{
	return CellCenter(x, y) - glm::vec2(0.0f, halfCellSize);
}

float Fluid::GetPressure(int x, int y) {
	if (x < 0 || x >= cellCountX || y < 0 || y >= cellCountY)
		return 0;

	return cellPressure[indexXY(x, y)];
}

int Fluid::indexXY(int x, int y) {
	return x + y * cellCountX;
}

int Fluid::indexVX(int x, int y) {
	return x + y * (cellCountX + 1);//Since 1D index, accessing values as x + y * offSet (column + row * row number)
}

int Fluid::indexVY(int x, int y) {
	return x + y * cellCountX;
}

bool::Fluid::isSolid(int cellX, int cellY) {
	if (cellX < 0 || cellX >= cellCountX || cellY < 0 || cellY >= cellCountY)
		return true;
	return solidCell[indexXY(cellX, cellY)];
}

Fluid::~Fluid() {
	delete[] velocityX;
	delete[] velocityY;
	delete[] velocityX_temp;   
	delete[] velocityY_temp;   
	delete[] cellPressure;     
	delete[] solidCell;
}