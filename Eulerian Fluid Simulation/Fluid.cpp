#include "Fluid.h"

void Fluid::RandomizeVelocities(float t) {
	for (int x = 0; x < cellCountX + 1; x++) {
		for (int y = 0; y < cellCountY; y++) {
			if (isSolid(x - 1, y) || isSolid(x, y))//exclude solid cells from randomization
				continue;
			velocityX[indexVX(x, y)] = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * t;
		}
	}

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY + 1; y++) {
			if (isSolid(x, y - 1) || isSolid(x, y))//exclude solid cells from randomization
				continue;
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

	windTunnel = false;
	inflowSpeed = 0.0f;

	velocityX = new float[(cellCountX + 1) * cellCountY]();
	velocityY = new float[cellCountX * (cellCountY + 1)]();

	velocityX_temp = new float[(cellCountX + 1) * cellCountY]();
	velocityY_temp = new float[cellCountX * (cellCountY + 1)]();

	cellPressure = new float[cellCountX * cellCountY]();
	solidCell = new bool[cellCountX * cellCountY]();//Initializes false

	dye = new float[cellCountX * cellCountY]();
	dye_temp = new float[cellCountX * cellCountY]();
	dye_back = new float[cellCountX * cellCountY]();
	dyeDecay = 0.0f;

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

void Fluid::SetupWindTunnel(float _inflowSpeed, float jetCentreUVY, float jetHeightCells, glm::vec2 obstacleCenterUV, float obstacleRadiusCells) {
	windTunnel = true;
	inflowSpeed = _inflowSpeed;

	int jetLo = static_cast<int>(round(jetCentreUVY * cellCountY - jetHeightCells / 2.0f));// round to nearest cell index
	int jetHi = jetLo + static_cast<int>(jetHeightCells);// round to nearest cell index

	jetLo = glm::clamp(jetLo, 1, cellCountY - 2);
	jetHi = glm::clamp(jetHi, jetLo + 1, cellCountY - 1);

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			solidCell[indexXY(x, y)] = false;
		}
	}

	for (int x = 0; x < cellCountX; x++) {// Top and bottom boundaries are solid cells
		solidCell[indexXY(x, 0)] = true;
		solidCell[indexXY(x, cellCountY - 1)] = true;
	}

	for(int y = 0; y < cellCountY; y++) {
		if (y < jetLo || y >= jetHi) {
			solidCell[indexXY(0, y)] = true;// Left boundary is solid except for the inflow jet
		}
	}

	//Circular Obstacle
	glm::vec2 center = bottomLeft + obstacleCenterUV * boundsSize;
	float radius = obstacleRadiusCells * cellSize;

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			if (glm::length(CellCenter(x, y) - center) < radius)
				solidCell[indexXY(x, y)] = true;
		}
	}

	// Initialize the velocity and pressure fields to zero, then set the inflow speed on the left boundary faces.
	resetVelocityX();
	resetVelocityY();
	resetCellPressure();

	for (int x = 0; x < cellCountX + 1; x++) {
		for (int y = 0; y < cellCountY; y++) {
			if (isSolid(x - 1, y) || isSolid(x, y))
				continue;
			velocityX[indexVX(x, y)] = inflowSpeed;
		}
	}

	ClearDye();
}

void Fluid::ApplyInflow() {
	if (!windTunnel)
		return;

	for (int y = 0; y < cellCountY; y++) {
		if (isSolid(0, y))
			continue;
		velocityX[indexVX(0, y)] = inflowSpeed;// Set the inflow speed on the left boundary faces
	}
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
	ApplyInflow();

	for (int i = 0; i < iterations; i++) {
		PressureSolver();
	}

	UpdateVelocities(); // applying the converged pressure gradient once
	AdvectDye();
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

			glm::vec2 posMid = pos - vel * (deltaTime * 0.5f);
			glm::vec2 posPrev = pos - GetVelocity(posMid) * deltaTime;

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

			glm::vec2 posMid = pos - vel * (deltaTime * 0.5f);
			glm::vec2 posPrev = pos - GetVelocity(posMid) * deltaTime;

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

void Fluid::EmitDye(glm::vec2 xy, float radiusCells, float strength) {
	glm::vec2 worldPos = bottomLeft + xy * boundsSize;
	float radius = radiusCells * cellSize;

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			if (isSolid(x, y))
				continue;

			float dist = glm::length(CellCenter(x, y) - worldPos);
			if (dist > radius)
				continue;

			dye[indexXY(x, y)] = glm::max(dye[indexXY(x, y)], strength);
		}
	}
}

void Fluid::AdvectDye() {
	//Advection is done in two steps to reduce numerical diffusion. First, we advect the dye field forward in time to a temporary field, then we advect it back to the original field. This is the MacCormack method.
	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			if (isSolid(x, y)) {
				dye_temp[indexXY(x, y)] = 0.0f;
				continue;
			}

			glm::vec2 pos = CellCenter(x, y);
			glm::vec2 vel = GetVelocity(pos);
			glm::vec2 posMid = pos - vel * (deltaTime * 0.5f);
			glm::vec2 posPrev = pos - GetVelocity(posMid) * deltaTime;

			dye_temp[indexXY(x, y)] = Bilinear(dye, cellCountX, cellCountY, cellSize, posPrev);
		}
	}

	//advect the dye backward in time
	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			if (isSolid(x, y)) {
				dye_back[indexXY(x, y)] = 0.0f;
				continue;
			}

			glm::vec2 pos = CellCenter(x, y);
			glm::vec2 vel = GetVelocity(pos);
			glm::vec2 posMid = pos + vel * (deltaTime * 0.5f);
			glm::vec2 posNext = pos + GetVelocity(posMid) * deltaTime;

			dye_back[indexXY(x, y)] = Bilinear(dye_temp, cellCountX, cellCountY, cellSize, posNext);
		}
	}

	// Apply dye decay
	float fade = (dyeDecay > 0.0f) ? exp(-deltaTime * dyeDecay) : 1.0f;

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			if (isSolid(x, y)) {
				dye_back[indexXY(x, y)] = 0.0f;
				continue;
			}

			glm::vec2 pos = CellCenter(x, y);
			glm::vec2 vel = GetVelocity(pos);
			glm::vec2 posMid = pos - vel * (deltaTime * 0.5f);
			glm::vec2 posPrev = pos - GetVelocity(posMid) * deltaTime;

			float stencilMin, stencilMax;// Find the minimum and maximum dye values of the surrounding cells to prevent overshooting during the MacCormack correction step
			SampleDyeMinMax(dye, posPrev, stencilMin, stencilMax);

			float corrected = dye_temp[indexXY(x, y)] + 0.5f * (dye[indexXY(x, y)] - dye_back[indexXY(x, y)]);// MacCormack correction step

			dye_back[indexXY(x, y)] = glm::clamp(corrected, stencilMin, stencilMax) * fade;//Makes sure the dye value is within the bounds of the surrounding cells and applies decay
		}
	}

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			dye[indexXY(x, y)] = dye_back[indexXY(x, y)];
		}
	}
}

void Fluid::ClearDye() {
	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			dye[indexXY(x, y)] = 0.0f;
		}
	}

	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			dye_temp[indexXY(x, y)] = 0.0f;
		}
	}
}

void Fluid::UpdateVelocities() {
	for (int i = 0; i < (cellCountX + 1); i++) {
		for (int j = 0; j < cellCountY; j++) {
			if (windTunnel && i == 0 && !isSolid(0, j))
				continue;

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

	//px and py represent the position relative to the grid's cell indices.
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

void Fluid::SampleDyeMinMax(float* field, glm::vec2 position, float& outMin, float& outMax) {
	float width = (cellCountX - 1) * cellSize;
	float height = (cellCountY - 1) * cellSize;

	//px and py represent the position relative to the grid's cell indices.
	float px = (position.x + width / 2) / cellSize;// normalized indices of the current cell
	float py = (position.y + height / 2) / cellSize;

	int left = glm::clamp(static_cast<int>(px), 0, cellCountX - 2);
	int bottom = glm::clamp(static_cast<int>(py), 0, cellCountY - 2);
	int right = left + 1;
	int top = bottom + 1;

	// Get the dye values at the four corners of the 2x2 stencil.
	float bottomLeftValue = field[left + bottom * cellCountX];
	float bottomRightValue = field[right + bottom * cellCountX];
	float topLeftValue = field[left + top * cellCountX];
	float topRightValue = field[right + top * cellCountX];

	outMin = glm::min(glm::min(bottomLeftValue, bottomRightValue), glm::min(topLeftValue, topRightValue));
	outMax = glm::max(glm::max(bottomLeftValue, bottomRightValue), glm::max(topLeftValue, topRightValue));
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

void Fluid::resetVelocityX() {
	for (int x = 0; x < (cellCountX + 1); x++) {
		for (int y = 0; y < cellCountY; y++) {
			velocityX[indexVX(x, y)] = 0.0f;
		}
	}
}

void Fluid::resetVelocityY() {
	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < (cellCountY + 1); y++) {
			velocityY[indexVY(x, y)] = 0.0f;
		}
	}
}

void Fluid::resetCellPressure() {
	for (int x = 0; x < cellCountX; x++) {
		for (int y = 0; y < cellCountY; y++) {
			cellPressure[indexXY(x, y)] = 0.0f;
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
	if (cellX < 0 || cellY < 0 || cellY >= cellCountY)
		return true;

	if (cellX >= cellCountX)// The right boundary is not solid in a wind tunnel, so we don't return true here.
		return !windTunnel;

	return solidCell[indexXY(cellX, cellY)];
}

Fluid::~Fluid() {
	delete[] velocityX;
	delete[] velocityY;
	delete[] velocityX_temp;   
	delete[] velocityY_temp;   
	delete[] cellPressure;     
	delete[] solidCell;
	delete[] dye;
	delete[] dye_temp;
	delete[] dye_back;
}