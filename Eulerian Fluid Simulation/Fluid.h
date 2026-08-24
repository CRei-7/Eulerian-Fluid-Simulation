#pragma once

#include <vector>
#include <glm/glm.hpp>

class Fluid
{
public:
    Fluid(int _cellCountX, int _cellCountY, float _cellSize, float _density, float _dt);
    ~Fluid();

    float CalcDivergence(int cellX, int cellY);

    void CellPressureSolver(int cellX, int cellY);
    void PressureSolver();

    void UpdateVelocities();

    float GetPressure(int x, int y);

    const bool* GetSolidData() const { return solidCell; }

private:
    int cellCountX;
    int cellCountY;
    float cellSize;

    float* velocityX;
    float* velocityY;

    float density;
    float deltaTime;

    float* cellPressure;

    bool* solidCell;

    int indexXY(int x, int y);
    int indexVX(int x, int y);
    int indexVY(int x, int y);

    bool isSolid(int cellX, int cellY);
};