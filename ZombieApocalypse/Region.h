#ifndef REGION_H
#define REGION_H

#include <string>
#include <iostream>
#include <vector>
#include <cmath>

enum Climate {
    TROPICAL,
    ARID,
    TEMPERATE,
    CONTINENTAL,
    POLAR
};

struct RegionStats {
    std::string name;

    // --- Population ---
    long long totalPopulation;
    long long healthy;
    long long infected;       // Infected but not yet turned
    long long zombies;
    long long dead;
    long long immune;
    long long evacuated;

    // --- Infrastructure ---
    double militaryStrength;    // 0.0 - 1.0
    double medicalCapacity;     // 0.0 - 1.0
    double governmentStability; // 0.0 - 1.0
    double infrastructure;      // 0.0 - 1.0 (roads, power, comms)

    // --- Geography ---
    double populationDensity;   // people per sq km
    double urbanization;        // 0.0 - 1.0: fraction living in cities
    Climate climate;
    double avgTemperature;      // Celsius
    double avgHumidity;         // 0.0 - 1.0
    double waterAccess;         // 0.0 - 1.0: access to clean water
    double borderSecurity;      // 0.0 - 1.0: how sealed borders are

    // --- Response State ---
    bool quarantineActive;
    bool martialLaw;
    bool evacuationInProgress;
    double researchProgress;    // 0.0 - 1.0: cure research progress
    int daysSinceOutbreak;
};

class Region {
public:
    Region();
    Region(const std::string& name, long long population, double density,
           double urbanization, double military, double medical,
           double govStability, double infra, Climate climate,
           double temp, double humidity, double water, double border);

    // Display region summary
    void displayStatus() const;
    void displayDetailedStatus() const;

    // Simulation updates
    void spreadInfection(double virusInfectionRate, double incubationHours,
                         double lethalityRate, double reanimationRate,
                         double immunityChance, double zombieThreat);
    void militaryResponse(double zombieDurability);
    void civilianResponse();
    void researchUpdate(double cureResistance, double globalProgress);
    void decayZombies(double decayRate);
    void processDay();

    // Cross-region spread
    long long getSpilloverInfected(double borderSecurity) const;
    void receiveInfected(long long count);

    // State
    RegionStats& getStats();
    const RegionStats& getStats() const;
    bool isCollapsed() const;
    bool isCleared() const;
    double getInfectionPercentage() const;
    std::string getClimateStr() const;
    std::string getStatusLabel() const;

private:
    RegionStats stats;
    double clamp(double val, double minVal, double maxVal) const;
};

#endif
