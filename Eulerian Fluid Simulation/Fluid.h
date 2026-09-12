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

    void SetupWindTunnel(float _inflowSpeed, float _jetCentreUVY, float _jetHeightCells, glm::vec2 _obstacleCenterUV, float _obstacleRadiusCells);

    void AdvectDye();
    void EmitDye(glm::vec2 uv, float radiusCells, float strength = 1.0f);
    void ClearDye();

    void SampleDyeMinMax(float* field, glm::vec2 position, float& outMin, float& outMax);

    const float* GetDyeData() const { return dye; }
	void SetDyeDecay(float decay) { dyeDecay = decay; }

    void resetVelocityX();
    void resetVelocityY();
    void resetCellPressure();

    enum class ObstacleType { None, Circle, VerticalLine };

    void SetWindTunnelEnabled(bool enabled);
    void SetWindTunnelExitOpen(bool open);

    void SetObstacleEnabled(bool enabled);
    void SetObstacle(ObstacleType type, glm::vec2 obstacleCenterUV, float obstacleRadiusCells); //circular obstacle
	void SetObstacleLine(glm::vec2 obstacleCenterUV, float obstacleHeightCells, float obstacleThicknessCells); //vertical line obstacle 

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

    bool windTunnelExitOpen;
    float jetCentreUVY;
    float jetHeightCells;

    bool obstacleEnabled;
    ObstacleType obstacleType;
    glm::vec2 obstacleCenterUV;
    float obstacleRadiusCells;
    float obstacleHeightCells;
    float obstacleThicknessCells;

	void RebuildBoundaries(); //Updates solidCell[] to include the current boundaries (wind tunnel, obstacle, etc.)
	void ApplyObstacle(); //Updates solidCell[] to include the current obstacle (if any)

    void ApplyInflow();

    float* dye;
    float* dye_temp;
    float* dye_back;
    float dyeDecay;
};