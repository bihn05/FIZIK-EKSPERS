#pragma once

enum CollisionType { ELASTIC = 0, EXCITATION, IONIZATION };

struct CrossSection {
    virtual ~CrossSection() = default;
    virtual float sigma(float energy_eV, CollisionType type) const = 0;
    virtual float ionization_threshold() const = 0;
    virtual float mass_ratio() const = 0; // m_electron / m_ion
};
