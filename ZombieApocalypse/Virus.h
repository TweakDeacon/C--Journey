#ifndef VIRUS_H
#define VIRUS_H

#include <string>
#include <iostream>
#include <vector>

// Transmission method flags
enum TransmissionMode {
    CONTACT    = 1 << 0,   // Bites, scratches, direct contact
    AIRBORNE   = 1 << 1,   // Spreads through air
    WATERBORNE = 1 << 2,   // Contaminates water supply
    BLOODBORNE = 1 << 3,   // Only through blood/fluid exchange
    VECTOR     = 1 << 4    // Carried by animals/insects
};

// How the virus evolves over time
enum MutationTendency {
    STABLE,         // Virus stays the same
    SLOW_DRIFT,     // Gradual minor changes
    MODERATE_SHIFT, // Periodic notable changes
    RAPID_MUTATION, // Constantly evolving
    HYPERMUTATION   // Extremely unstable genome
};

struct VirusConfig {
    // --- Core Spread Parameters ---
    std::string name;
    double infectionRate;         // 0.0 - 1.0: probability of infection per contact
    double incubationHours;       // Hours before an infected person turns
    double lethalityRate;         // 0.0 - 1.0: chance of death instead of turning
    double reanimationRate;       // 0.0 - 1.0: chance a dead person reanimates

    // --- Transmission ---
    int transmissionModes;        // Bitmask of TransmissionMode flags
    double airborneRange;         // Meters (only matters if AIRBORNE set)
    double environmentalSurvival; // 0.0 - 1.0: how long virus survives on surfaces

    // --- Evolution ---
    MutationTendency mutationTendency;
    double mutationRate;          // 0.0 - 1.0: speed of mutation per cycle
    double cureResistance;        // 0.0 - 1.0: how hard it is to develop a cure

    // --- Environmental Factors ---
    double heatVulnerability;     // 0.0 - 1.0: weakened by hot climates
    double coldVulnerability;     // 0.0 - 1.0: weakened by cold climates
    double humidityBonus;         // 0.0 - 1.0: strengthened by humidity

    // --- Host Interaction ---
    double immunityChance;        // 0.0 - 1.0: natural immunity in population
    double partialResistance;     // 0.0 - 1.0: chance of slower infection
};

class Virus {
public:
    Virus();

    // Configure all parameters interactively
    void configure();

    // Apply a preset configuration
    void applyPreset(int presetIndex);

    // Display current configuration
    void displayConfig() const;

    // Getters
    const VirusConfig& getConfig() const;

    // Calculate effective infection rate for a given climate
    double getEffectiveInfectionRate(double temperature, double humidity) const;

    // Check if a transmission mode is active
    bool hasTransmission(TransmissionMode mode) const;

    // Simulate a mutation event — may alter virus stats
    void mutate();

    // Get mutation generation count
    int getMutationGeneration() const;

    // Get a description of the transmission modes
    std::string getTransmissionDescription() const;

    // Get mutation tendency as string
    std::string getMutationTendencyStr() const;

private:
    VirusConfig config;
    int mutationGeneration;

    void showPresets() const;
    double clamp(double val, double minVal, double maxVal) const;
    double promptDouble(const std::string& prompt, double minVal, double maxVal, double defaultVal) const;
    int promptInt(const std::string& prompt, int minVal, int maxVal, int defaultVal) const;
};

#endif
