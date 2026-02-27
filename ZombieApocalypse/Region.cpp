#include "Region.h"
#include <cstdlib>
#include <iomanip>
#include <algorithm>

Region::Region() {
    stats.name = "Unknown";
    stats.totalPopulation = 0;
    stats.healthy = 0;
    stats.infected = 0;
    stats.zombies = 0;
    stats.dead = 0;
    stats.immune = 0;
    stats.evacuated = 0;
    stats.militaryStrength = 0.0;
    stats.medicalCapacity = 0.0;
    stats.governmentStability = 0.0;
    stats.infrastructure = 0.0;
    stats.populationDensity = 0.0;
    stats.urbanization = 0.0;
    stats.climate = TEMPERATE;
    stats.avgTemperature = 15.0;
    stats.avgHumidity = 0.5;
    stats.waterAccess = 0.5;
    stats.borderSecurity = 0.5;
    stats.quarantineActive = false;
    stats.martialLaw = false;
    stats.evacuationInProgress = false;
    stats.researchProgress = 0.0;
    stats.daysSinceOutbreak = 0;
}

Region::Region(const std::string& name, long long population, double density,
               double urban, double military, double medical,
               double govStability, double infra, Climate climate,
               double temp, double humidity, double water, double border) {
    stats.name = name;
    stats.totalPopulation = population;
    stats.healthy = population;
    stats.infected = 0;
    stats.zombies = 0;
    stats.dead = 0;
    stats.immune = 0;
    stats.evacuated = 0;
    stats.militaryStrength = military;
    stats.medicalCapacity = medical;
    stats.governmentStability = govStability;
    stats.infrastructure = infra;
    stats.populationDensity = density;
    stats.urbanization = urban;
    stats.climate = climate;
    stats.avgTemperature = temp;
    stats.avgHumidity = humidity;
    stats.waterAccess = water;
    stats.borderSecurity = border;
    stats.quarantineActive = false;
    stats.martialLaw = false;
    stats.evacuationInProgress = false;
    stats.researchProgress = 0.0;
    stats.daysSinceOutbreak = 0;
}

void Region::displayStatus() const {
    std::string status = getStatusLabel();

    std::cout << "  " << std::left << std::setw(22) << stats.name
              << " | Pop: " << std::setw(12) << stats.healthy
              << " | Inf: " << std::setw(10) << stats.infected
              << " | Z: " << std::setw(10) << stats.zombies
              << " | Dead: " << std::setw(10) << stats.dead
              << " | " << status << "\n";
}

void Region::displayDetailedStatus() const {
    std::cout << "\n  ---- " << stats.name << " ----\n";
    std::cout << "  Healthy:       " << std::setw(14) << stats.healthy << "\n";
    std::cout << "  Infected:      " << std::setw(14) << stats.infected << "\n";
    std::cout << "  Zombies:       " << std::setw(14) << stats.zombies << "\n";
    std::cout << "  Dead:          " << std::setw(14) << stats.dead << "\n";
    std::cout << "  Immune:        " << std::setw(14) << stats.immune << "\n";
    std::cout << "  Evacuated:     " << std::setw(14) << stats.evacuated << "\n";
    std::cout << "  Military:      " << std::setw(14) << (stats.militaryStrength * 100) << "%\n";
    std::cout << "  Medical:       " << std::setw(14) << (stats.medicalCapacity * 100) << "%\n";
    std::cout << "  Government:    " << std::setw(14) << (stats.governmentStability * 100) << "%\n";
    std::cout << "  Infrastructure:" << std::setw(14) << (stats.infrastructure * 100) << "%\n";
    std::cout << "  Climate:       " << std::setw(14) << getClimateStr()
              << " (" << stats.avgTemperature << "C, " << (stats.avgHumidity * 100) << "% humid)\n";
    std::cout << "  Quarantine:    " << std::setw(14) << (stats.quarantineActive ? "ACTIVE" : "None") << "\n";
    std::cout << "  Martial Law:   " << std::setw(14) << (stats.martialLaw ? "ACTIVE" : "None") << "\n";
    std::cout << "  Cure Research: " << std::setw(14) << (stats.researchProgress * 100) << "%\n";
    std::cout << "  Status:        " << getStatusLabel() << "\n";
}

void Region::spreadInfection(double virusInfectionRate, double incubationHours,
                              double lethalityRate, double reanimationRate,
                              double immunityChance, double zombieThreat) {
    if (stats.healthy <= 0 && stats.infected <= 0) return;

    // --- Contact infection from zombies ---
    double contactRate = virusInfectionRate;

    // Density increases spread
    double densityMultiplier = 1.0 + (stats.populationDensity / 1000.0) * 0.5;
    // Urban areas spread faster
    double urbanMultiplier = 1.0 + stats.urbanization * 0.5;
    // Quarantine reduces spread
    double quarantineMultiplier = stats.quarantineActive ? 0.4 : 1.0;
    // Military presence reduces zombie contact
    double militaryMultiplier = 1.0 - (stats.militaryStrength * 0.3);

    double effectiveRate = contactRate * densityMultiplier * urbanMultiplier
                           * quarantineMultiplier * militaryMultiplier;
    effectiveRate = clamp(effectiveRate, 0.0, 0.95);

    // Each zombie can infect multiple people per day based on population density
    double contactsPerZombie = 2.0 * densityMultiplier * (1.0 + zombieThreat * 0.5);
    long long potentialContacts = static_cast<long long>(stats.zombies * contactsPerZombie);
    potentialContacts = std::min(potentialContacts, stats.healthy);

    long long newInfected = static_cast<long long>(potentialContacts * effectiveRate);

    // Natural immunity
    long long newImmune = static_cast<long long>(newInfected * immunityChance);
    newInfected -= newImmune;

    // Ensure we don't go negative
    newInfected = std::max(0LL, std::min(newInfected, stats.healthy));
    newImmune = std::max(0LL, std::min(newImmune, stats.healthy - newInfected));

    stats.healthy -= (newInfected + newImmune);
    stats.infected += newInfected;
    stats.immune += newImmune;

    // --- Process incubation (infected -> zombie or dead) ---
    // Fraction that turn each day based on incubation period
    double turnFraction = 24.0 / std::max(incubationHours, 0.1);
    turnFraction = clamp(turnFraction, 0.0, 1.0);

    long long turningToday = static_cast<long long>(stats.infected * turnFraction);

    // Some die outright instead of turning
    long long dieOutright = static_cast<long long>(turningToday * lethalityRate);
    long long becomeZombie = turningToday - dieOutright;

    // Of those who die, some reanimate
    long long reanimate = static_cast<long long>(dieOutright * reanimationRate);

    stats.infected -= turningToday;
    stats.dead += (dieOutright - reanimate);
    stats.zombies += (becomeZombie + reanimate);

    // Clamp everything
    stats.infected = std::max(0LL, stats.infected);
    stats.healthy = std::max(0LL, stats.healthy);
}

void Region::militaryResponse(double zombieDurability) {
    if (stats.zombies <= 0 || stats.militaryStrength <= 0.01) return;

    // Military kills zombies based on strength
    double killEfficiency = stats.militaryStrength * (1.0 - zombieDurability * 0.7);
    killEfficiency = clamp(killEfficiency, 0.01, 0.8);

    // Military can handle a certain number per day
    long long militaryCapacity = static_cast<long long>(
        stats.totalPopulation * 0.001 * stats.militaryStrength * 10.0);

    long long zombiesEngaged = std::min(static_cast<long long>(stats.zombies * 0.3), militaryCapacity);
    long long zombiesKilled = static_cast<long long>(zombiesEngaged * killEfficiency);

    stats.zombies -= zombiesKilled;
    stats.dead += zombiesKilled;

    // Military suffers losses
    double militaryLoss = 0.005 * (1.0 - stats.militaryStrength * 0.5) *
                          (static_cast<double>(stats.zombies) / std::max(1.0, static_cast<double>(stats.healthy) * 0.01));
    stats.militaryStrength -= clamp(militaryLoss, 0.0, 0.05);
    stats.militaryStrength = clamp(stats.militaryStrength, 0.0, 1.0);

    // Activate martial law if things get bad
    double zombieRatio = static_cast<double>(stats.zombies) /
                         std::max(1.0, static_cast<double>(stats.totalPopulation));
    if (zombieRatio > 0.05 && !stats.martialLaw) {
        stats.martialLaw = true;
    }
    if (zombieRatio > 0.01 && !stats.quarantineActive) {
        stats.quarantineActive = true;
    }
}

void Region::civilianResponse() {
    // Government stability decreases as situation worsens
    double zombieRatio = static_cast<double>(stats.zombies + stats.infected) /
                         std::max(1.0, static_cast<double>(stats.totalPopulation));

    double stabilityLoss = zombieRatio * 0.05;
    stats.governmentStability -= stabilityLoss;
    stats.governmentStability = clamp(stats.governmentStability, 0.0, 1.0);

    // Infrastructure degrades
    double infraLoss = zombieRatio * 0.03;
    stats.infrastructure -= infraLoss;
    stats.infrastructure = clamp(stats.infrastructure, 0.0, 1.0);

    // Medical capacity adjusts
    if (stats.infrastructure < 0.3) {
        stats.medicalCapacity *= 0.95;
    }

    // Evacuation starts if things are very bad
    if (zombieRatio > 0.15 && !stats.evacuationInProgress && stats.governmentStability > 0.2) {
        stats.evacuationInProgress = true;
    }

    // Evacuate people
    if (stats.evacuationInProgress && stats.healthy > 0) {
        long long evacuateCount = static_cast<long long>(
            stats.healthy * 0.02 * stats.infrastructure * stats.governmentStability);
        evacuateCount = std::max(0LL, std::min(evacuateCount, stats.healthy));
        stats.healthy -= evacuateCount;
        stats.evacuated += evacuateCount;
    }

    // Civilians fight back (poorly) when desperate
    if (zombieRatio > 0.3 && stats.zombies > 0) {
        long long civilianKills = static_cast<long long>(stats.healthy * 0.001);
        civilianKills = std::min(civilianKills, stats.zombies);
        stats.zombies -= civilianKills;
        stats.dead += civilianKills;

        // Civilian casualties from fighting
        long long civilianDeaths = static_cast<long long>(civilianKills * 0.5);
        civilianDeaths = std::min(civilianDeaths, stats.healthy);
        stats.healthy -= civilianDeaths;
        stats.dead += civilianDeaths;
    }
}

void Region::researchUpdate(double cureResistance, double globalProgress) {
    if (stats.medicalCapacity < 0.1 || stats.infrastructure < 0.1) return;

    double researchRate = stats.medicalCapacity * stats.infrastructure *
                          stats.governmentStability * (1.0 - cureResistance * 0.8);

    // Benefit from global research pool
    researchRate += globalProgress * 0.1;

    stats.researchProgress += researchRate * 0.005;
    stats.researchProgress = clamp(stats.researchProgress, 0.0, 1.0);
}

void Region::decayZombies(double decayRate) {
    if (stats.zombies <= 0 || decayRate <= 0.0) return;

    long long decayed = static_cast<long long>(stats.zombies * decayRate * 0.01);
    decayed = std::min(decayed, stats.zombies);
    stats.zombies -= decayed;
    stats.dead += decayed;
}

void Region::processDay() {
    stats.daysSinceOutbreak++;
}

long long Region::getSpilloverInfected(double borderSec) const {
    if (stats.zombies <= 0 && stats.infected <= 0) return 0;

    double spillRate = (1.0 - borderSec) * 0.01;
    long long spillover = static_cast<long long>((stats.infected + stats.zombies * 0.1) * spillRate);
    return std::max(0LL, spillover);
}

void Region::receiveInfected(long long count) {
    if (count <= 0) return;
    // Incoming infected reduce healthy pop
    count = std::min(count, stats.healthy);
    stats.healthy -= count;
    stats.infected += count;
}

RegionStats& Region::getStats() {
    return stats;
}

const RegionStats& Region::getStats() const {
    return stats;
}

bool Region::isCollapsed() const {
    return stats.governmentStability <= 0.05 && stats.militaryStrength <= 0.05;
}

bool Region::isCleared() const {
    return stats.zombies == 0 && stats.infected == 0;
}

double Region::getInfectionPercentage() const {
    if (stats.totalPopulation <= 0) return 0.0;
    return static_cast<double>(stats.infected + stats.zombies) /
           static_cast<double>(stats.totalPopulation) * 100.0;
}

std::string Region::getClimateStr() const {
    switch (stats.climate) {
        case TROPICAL:     return "Tropical";
        case ARID:         return "Arid";
        case TEMPERATE:    return "Temperate";
        case CONTINENTAL:  return "Continental";
        case POLAR:        return "Polar";
        default:           return "Unknown";
    }
}

std::string Region::getStatusLabel() const {
    if (isCleared()) return "[CLEARED]";
    if (isCollapsed()) return "[COLLAPSED]";
    double pct = getInfectionPercentage();
    if (pct < 1.0) return "[CONTAINED]";
    if (pct < 10.0) return "[SPREADING]";
    if (pct < 30.0) return "[CRITICAL]";
    if (pct < 60.0) return "[OVERRUN]";
    return "[FALLEN]";
}

double Region::clamp(double val, double minVal, double maxVal) const {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}
