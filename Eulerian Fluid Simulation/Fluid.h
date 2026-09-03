#pragma once

#include <vector>
#include <glm/glm.hpp>

class Fluid
{
public:
    Fluid(int _cellCountX, int _cellCountY, float _cellSize, float _density, float _dt);
    ~Fluid();

    void CellPressureSolver(int cellX, int cellY);
    void PressureSolver();

    void UpdateVelocities();

    void AdvectVelocity(); 

    const bool* GetSolidData() const { return solidCell; }

    float Bilinear(float* edgeValues, int edgeCountX, int edgeCountY, float cellSize, glm::vec2 position);

    float GetPressure(int x, int y);
    glm::vec2 GetVelocity(glm::vec2 position);

    void Simulate(int iterations);

    void RandomizeVelocities(float t);
    std::vector<float> GetVelocityMagnitudes();

    void AddVelocity(glm::vec2 uv, glm::vec2 velocity, float radiusCells);

    void SetupWindTunnel(float _inflowSpeed, float jetCentreUVY, float jetHeightCells, glm::vec2 obstacleCenterUV, float obstacleRadiusCells);

    void AdvectDye();
    void EmitDye(glm::vec2 uv, float radiusCells, float strength = 1.0f);
    void ClearDye();

    void SampleDyeMinMax(float* field, glm::vec2 position, float& outMin, float& outMax);

    const float* GetDyeData() const { return dye; }
	void SetDyeDecay(float decay) { dyeDecay = decay; }

    void resetVelocityX();
    void resetVelocityY();
    void resetCellPressure();

private:
    int cellCountX;
    int cellCountY;
    float cellSize;

    float* velocityX;
    float* velocityY;

    float* velocityX_temp;
    float* velocityY_temp;

    float density;
    float deltaTime;

    float* cellPressure;

    bool* solidCell;

    glm::vec2 boundsSize;
    glm::vec2 bottomLeft;
    float halfCellSize;

    int indexXY(int x, int y);
    int indexVX(int x, int y);
    int indexVY(int x, int y);

    bool isSolid(int cellX, int cellY);

    glm::vec2 CellCenter(int x, int y);
    glm::vec2 LeftEdgeCenter(int x, int y);
    glm::vec2 BottomEdgeCenter(int x, int y);

    bool windTunnel;
    float inflowSpeed;

    void ApplyInflow();

    float* dye;
    float* dye_temp;
    float* dye_back;
    float dyeDecay;
};