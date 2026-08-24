#include "Fluid.h"

Fluid::Fluid(int _cellCountX, int _cellCountY, float _cellSize, float _density, float dt) {
	cellCountX = _cellCountX;
	cellCountY = _cellCountY;
	cellSize = _cellSize;

	density = _density;
	deltaTime = dt;

	velocityX = new float[(cellCountX + 1) * cellCountY];
	velocityY = new float[cellCountX * (cellCountY + 1)];

	cellPressure = new float[cellCountX * cellCountY];
	solidCell = new bool[cellCountX * cellCountY]();//Initializes false

	for (int i = 0; i < cellCountX; i++) {// Boundary cells are solid cells
		solidCell[indexXY(i, 0)] = true;
		solidCell[indexXY(i, cellCountY - 1)] = true;
	}

	for (int i = 0; i < cellCountY; i++) {
		solidCell[indexXY(0, i)] = true;
		solidCell[indexXY(cellCountX - 1, i)] = true;
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

	cellPressure[indexXY(cellX, cellY)] = (totalPressure - density * cellSize * deltaVelocity / deltaTime) / edgeCount;
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
	return solidCell[indexXY(cellX, cellY)];
}

Fluid::~Fluid() {
	delete[] velocityX;
	delete[] velocityY;
}