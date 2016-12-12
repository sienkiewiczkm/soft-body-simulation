#include "SoftBox.hpp"
#include <cassert>
#include <random>

namespace application
{

SoftBox::SoftBox():
    _particleMatrixSize{4, 4, 4}
{
}

SoftBox::~SoftBox()
{
}

void SoftBox::distributeUniformly(const fw::AABB<glm::dvec3>& box)
{
    _particleSystem.clear();

    for (auto z = 0; z < _particleMatrixSize.z; ++z)
    {
        auto zCoord = glm::mix(
            box.min.z,
            box.max.z,
            static_cast<double>(z) / (_particleMatrixSize.z - 1)
        );

        for (auto y = 0; y < _particleMatrixSize.y; ++y)
        {
            auto yCoord = glm::mix(
                box.min.y,
                box.max.y,
                static_cast<double>(y) / (_particleMatrixSize.y - 1)
            );

            for (auto x = 0; x < _particleMatrixSize.x; ++x)
            {
                auto xCoord = glm::mix(
                    box.min.x,
                    box.max.x,
                    static_cast<double>(x) / (_particleMatrixSize.x - 1)
                );

                _particleSystem.addParticle({
                    {xCoord, yCoord, zCoord},
                    {0.0, 0.0, 0.0}
                });
            }
        }
    }

    fixCurrentBoxPositionUsingSprings();
}

glm::ivec3 SoftBox::getParticleMatrixSize() const
{
    return _particleMatrixSize;
}

const std::vector<ParticleState> SoftBox::getSoftBoxParticles() const
{
    return _particleSystem.getParticleStates();
}

const ParticleState& SoftBox::getSoftBoxParticle(glm::ivec3 index) const
{
    return getSoftBoxParticles()[getParticleIndex(index)];
}

int SoftBox::getParticleIndex(glm::ivec3 coordinate) const
{
    assert(coordinate.x >= 0 && coordinate.x < _particleMatrixSize.x);
    assert(coordinate.y >= 0 && coordinate.y < _particleMatrixSize.y);
    assert(coordinate.z >= 0 && coordinate.z < _particleMatrixSize.z);

    auto particleIndex =
        _particleMatrixSize.x * _particleMatrixSize.y * coordinate.z
        + _particleMatrixSize.x * coordinate.y
        + coordinate.x;

    return particleIndex;
}

void SoftBox::update(double dt)
{
    _particleSystem.update(dt);
}

void SoftBox::fixCurrentBoxPositionUsingSprings()
{
    auto matrixSize = getParticleMatrixSize();
    for (auto z = 0; z < matrixSize.z; ++z)
    for (auto y = 0; y < matrixSize.y; ++y)
    for (auto x = 0; x < matrixSize.x; ++x)
    {
        auto& mainParticle = getSoftBoxParticle({x, y, z});
        auto mainIndex = getParticleIndex({x, y, z});

        for (auto i = -1; i <= 1; ++i)
        for (auto j = -1; j <= 1; ++j)
        for (auto k = 0; k <= 1; ++k)
        {
            if (i == 0 && j == 0 && k == 0) { continue; }
            if (x + i < 0 || x + i >= matrixSize.x
                || y + j < 0 || y + j >= matrixSize.y
                || z + k < 0 || z + k >= matrixSize.z)
            {
                continue;
            }

            glm::ivec3 coordinate{x + i, y + j, z + k};
            auto& otherParticle = getSoftBoxParticle(coordinate);
            auto otherIndex = getParticleIndex(coordinate);

            SpringConstraint constraint;
            constraint.springLength = glm::length(
                mainParticle.position - otherParticle.position
            );

            constraint.a = mainIndex;
            constraint.b = otherIndex;

            _particleSystem.addConstraint(constraint);
        }
    }
}

void SoftBox::applyRandomDisturbance()
{
    _particleSystem.applyRandomDisturbance();
}

}
