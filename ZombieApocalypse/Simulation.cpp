#include "Simulation.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <set>

Simulation::Simulation() : simulationOver(false) {
    globalStats.totalHealthy = 0;
    globalStats.totalInfected = 0;
    globalStats.totalZombies = 0;
    globalStats.totalDead = 0;
    globalStats.totalImmune = 0;
    globalStats.totalEvacuated = 0;
    globalStats.worldPopulation = 0;
    globalStats.currentDay = 0;
    globalStats.regionsCollapsed = 0;
    globalStats.regionsCleared = 0;
    globalStats.globalCureProgress = 0.0;
    globalStats.cureFound = false;
    globalStats.humanityExtinct = false;
    globalStats.humanityWon = false;
}

void Simulation::setVirus(const Virus& v) {
    virus = v;
}

void Simulation::setZombieBehavior(const ZombieBehavior& b) {
    zombieBehavior = b;
}

void Simulation::initializeWorld() {
    regions.clear();

    //                     Name                    Pop          Density  Urban  Mil   Med   Gov   Infra Climate      Temp  Humid Water Border
    regions.push_back(Region("North America",       380000000LL, 25.0,   0.82, 0.90, 0.85, 0.80, 0.90, TEMPERATE,   15.0, 0.55, 0.90, 0.70));
    regions.push_back(Region("Central America",      180000000LL, 90.0,   0.65, 0.35, 0.45, 0.50, 0.55, TROPICAL,    27.0, 0.75, 0.60, 0.30));
    regions.push_back(Region("South America",        430000000LL, 25.0,   0.84, 0.40, 0.50, 0.55, 0.60, TROPICAL,    25.0, 0.70, 0.70, 0.25));
    regions.push_back(Region("Western Europe",       400000000LL, 180.0,  0.78, 0.70, 0.90, 0.85, 0.92, TEMPERATE,   12.0, 0.65, 0.95, 0.60));
    regions.push_back(Region("Eastern Europe",       290000000LL, 45.0,   0.69, 0.65, 0.60, 0.55, 0.65, CONTINENTAL, 8.0,  0.60, 0.80, 0.50));
    regions.push_back(Region("Russia",               145000000LL, 9.0,    0.74, 0.80, 0.55, 0.60, 0.60, CONTINENTAL, -5.0, 0.55, 0.75, 0.65));
    regions.push_back(Region("Middle East",          410000000LL, 55.0,   0.72, 0.55, 0.50, 0.50, 0.65, ARID,        30.0, 0.25, 0.35, 0.55));
    regions.push_back(Region("North Africa",         250000000LL, 30.0,   0.52, 0.35, 0.35, 0.40, 0.45, ARID,        28.0, 0.30, 0.40, 0.30));
    regions.push_back(Region("Sub-Saharan Africa",   1200000000LL, 50.0,  0.42, 0.20, 0.25, 0.35, 0.30, TROPICAL,    26.0, 0.60, 0.45, 0.15));
    regions.push_back(Region("South Asia",           2000000000LL, 400.0, 0.35, 0.50, 0.40, 0.55, 0.50, TROPICAL,    28.0, 0.70, 0.55, 0.35));
    regions.push_back(Region("East Asia",            1600000000LL, 150.0, 0.62, 0.75, 0.70, 0.70, 0.80, TEMPERATE,   14.0, 0.60, 0.80, 0.75));
    regions.push_back(Region("Southeast Asia",       700000000LL, 155.0,  0.50, 0.35, 0.45, 0.50, 0.55, TROPICAL,    28.0, 0.80, 0.65, 0.30));
    regions.push_back(Region("Oceania",              45000000LL,  3.5,    0.86, 0.55, 0.80, 0.85, 0.88, TEMPERATE,   22.0, 0.50, 0.90, 0.90));

    buildAdjacency();

    // Calculate total world population
    globalStats.worldPopulation = 0;
    for (const auto& r : regions) {
        globalStats.worldPopulation += r.getStats().totalPopulation;
    }

    updateGlobalStats();
}

void Simulation::buildAdjacency() {
    // Index reference:
    // 0: North America      1: Central America    2: South America
    // 3: Western Europe      4: Eastern Europe     5: Russia
    // 6: Middle East         7: North Africa       8: Sub-Saharan Africa
    // 9: South Asia         10: East Asia         11: Southeast Asia
    // 12: Oceania

    adjacency.resize(regions.size());

    auto connect = [&](int a, int b) {
        adjacency[a].push_back(b);
        adjacency[b].push_back(a);
    };

    connect(0, 1);   // North America - Central America
    connect(0, 3);   // North America - Western Europe (transatlantic)
    connect(0, 5);   // North America - Russia (Arctic)
    connect(1, 2);   // Central America - South America
    connect(2, 7);   // South America - North Africa (transatlantic)
    connect(3, 4);   // Western Europe - Eastern Europe
    connect(3, 7);   // Western Europe - North Africa (Mediterranean)
    connect(4, 5);   // Eastern Europe - Russia
    connect(4, 6);   // Eastern Europe - Middle East
    connect(5, 10);  // Russia - East Asia
    connect(6, 7);   // Middle East - North Africa
    connect(6, 8);   // Middle East - Sub-Saharan Africa
    connect(6, 9);   // Middle East - South Asia
    connect(7, 8);   // North Africa - Sub-Saharan Africa
    connect(9, 10);  // South Asia - East Asia
    connect(9, 11);  // South Asia - Southeast Asia
    connect(10, 11); // East Asia - Southeast Asia
    connect(10, 5);  // East Asia - Russia
    connect(11, 12); // Southeast Asia - Oceania
}

void Simulation::setPatientZeroRegion(int regionIndex) {
    if (regionIndex < 0 || regionIndex >= static_cast<int>(regions.size())) return;

    // Start with a small initial infection
    long long initialInfected = std::max(1LL, regions[regionIndex].getStats().totalPopulation / 10000000);
    regions[regionIndex].getStats().healthy -= initialInfected;
    regions[regionIndex].getStats().infected += initialInfected;

    logEvent(regions[regionIndex].getStats().name,
             "PATIENT ZERO: " + std::to_string(initialInfected) + " initial infections detected");
    updateGlobalStats();
}

void Simulation::advanceDay() {
    if (simulationOver) return;

    globalStats.currentDay++;
    const VirusConfig& vc = virus.getConfig();
    const ZombieConfig& zc = zombieBehavior.getConfig();

    double zombieThreat = zombieBehavior.calculateThreatLevel() / 10.0;

    // Phase 1: Process each region independently
    for (auto& region : regions) {
        RegionStats& rs = region.getStats();

        // Calculate effective infection rate for this region's climate
        double effectiveRate = virus.getEffectiveInfectionRate(rs.avgTemperature, rs.avgHumidity);

        // Waterborne bonus
        if (virus.hasTransmission(WATERBORNE)) {
            effectiveRate *= (1.0 + (1.0 - rs.waterAccess) * 0.3);
        }

        // Airborne bonus
        if (virus.hasTransmission(AIRBORNE)) {
            effectiveRate *= (1.0 + rs.urbanization * 0.4);
        }

        // Vector bonus in tropical regions
        if (virus.hasTransmission(VECTOR) && rs.climate == TROPICAL) {
            effectiveRate *= 1.3;
        }

        // Spread infection
        region.spreadInfection(effectiveRate, vc.incubationHours,
                               vc.lethalityRate, vc.reanimationRate,
                               vc.immunityChance, zombieThreat);

        // Military response
        region.militaryResponse(zc.durability + zc.regeneration * 0.5);

        // Civilian response
        region.civilianResponse();

        // Zombie decay
        region.decayZombies(zc.decayRate);

        // Cure research
        region.researchUpdate(vc.cureResistance, globalStats.globalCureProgress);

        // Process day counter
        region.processDay();
    }

    // Phase 2: Cross-region spread
    spreadBetweenRegions();

    // Phase 3: Virus mutation (periodic)
    if (globalStats.currentDay % 7 == 0) {
        int prevGen = virus.getMutationGeneration();
        virus.mutate();
        if (virus.getMutationGeneration() > prevGen) {
            logEvent("GLOBAL", "VIRUS MUTATION: Strain has evolved to generation " +
                     std::to_string(virus.getMutationGeneration()) + "!");
        }
    }

    // Phase 4: Update global stats
    updateGlobalStats();

    // Phase 5: Check for milestones and events
    checkMilestones();

    // Phase 6: Check end conditions
    checkEndConditions();
}

void Simulation::advanceDays(int count) {
    for (int i = 0; i < count && !simulationOver; i++) {
        advanceDay();
    }
}

void Simulation::runUntilEnd(int maxDays) {
    while (!simulationOver && globalStats.currentDay < maxDays) {
        advanceDay();
    }
}

void Simulation::spreadBetweenRegions() {
    // Cross-border infection spread
    std::vector<std::pair<int, long long>> incoming;

    for (int i = 0; i < static_cast<int>(regions.size()); i++) {
        long long spillover = regions[i].getSpilloverInfected(regions[i].getStats().borderSecurity);
        if (spillover <= 0) continue;

        // Distribute to adjacent regions
        for (int adj : adjacency[i]) {
            double adjBorder = regions[adj].getStats().borderSecurity;
            long long transfer = static_cast<long long>(spillover * (1.0 - adjBorder) /
                                 adjacency[i].size());
            if (transfer > 0) {
                incoming.push_back({adj, transfer});

                if (transfer > 100 && regions[adj].getStats().infected == 0) {
                    logEvent(regions[adj].getStats().name,
                             "BORDER BREACH: Infection spreading from " + regions[i].getStats().name);
                }
            }
        }
    }

    for (const auto& p : incoming) {
        regions[p.first].receiveInfected(p.second);
    }
}

void Simulation::updateGlobalStats() {
    globalStats.totalHealthy = 0;
    globalStats.totalInfected = 0;
    globalStats.totalZombies = 0;
    globalStats.totalDead = 0;
    globalStats.totalImmune = 0;
    globalStats.totalEvacuated = 0;
    globalStats.regionsCollapsed = 0;
    globalStats.regionsCleared = 0;
    globalStats.globalCureProgress = 0.0;

    int researchRegions = 0;

    for (const auto& r : regions) {
        const RegionStats& rs = r.getStats();
        globalStats.totalHealthy += rs.healthy;
        globalStats.totalInfected += rs.infected;
        globalStats.totalZombies += rs.zombies;
        globalStats.totalDead += rs.dead;
        globalStats.totalImmune += rs.immune;
        globalStats.totalEvacuated += rs.evacuated;

        if (r.isCollapsed()) globalStats.regionsCollapsed++;
        if (r.isCleared()) globalStats.regionsCleared++;

        if (rs.researchProgress > 0.0) {
            globalStats.globalCureProgress += rs.researchProgress;
            researchRegions++;
        }
    }

    if (researchRegions > 0) {
        globalStats.globalCureProgress /= researchRegions;
    }
}

void Simulation::checkMilestones() {
    double infPct = static_cast<double>(globalStats.totalInfected + globalStats.totalZombies) /
                    std::max(1LL, globalStats.worldPopulation) * 100.0;

    // Milestone events
    static std::set<int> triggeredPcts;

    int pctInt = static_cast<int>(infPct);
    std::vector<int> milestones = {1, 5, 10, 25, 50, 75, 90};
    for (int m : milestones) {
        if (pctInt >= m && triggeredPcts.find(m) == triggeredPcts.end()) {
            triggeredPcts.insert(m);
            logEvent("GLOBAL", "MILESTONE: " + std::to_string(m) + "% of world population infected or turned");
        }
    }

    // Check for region collapse events
    for (const auto& r : regions) {
        if (r.isCollapsed()) {
            static std::set<std::string> collapsedRegions;
            if (collapsedRegions.find(r.getStats().name) == collapsedRegions.end()) {
                collapsedRegions.insert(r.getStats().name);
                logEvent(r.getStats().name, "REGION COLLAPSED: Government and military have fallen");
            }
        }
    }

    // Cure progress milestones
    if (globalStats.globalCureProgress >= 0.99 && !globalStats.cureFound) {
        globalStats.cureFound = true;
        logEvent("GLOBAL", "CURE DEVELOPED! Scientists have found a treatment!");
    }
}

void Simulation::checkEndConditions() {
    // Humanity extinct
    if (globalStats.totalHealthy + globalStats.totalImmune + globalStats.totalEvacuated <= 0) {
        globalStats.humanityExtinct = true;
        simulationOver = true;
        logEvent("GLOBAL", "EXTINCTION: Humanity has fallen. The dead inherit the earth.");
        return;
    }

    // All zombies cleared
    if (globalStats.totalZombies == 0 && globalStats.totalInfected == 0 && globalStats.currentDay > 1) {
        globalStats.humanityWon = true;
        simulationOver = true;
        logEvent("GLOBAL", "VICTORY: The last zombie has fallen. Humanity survives.");
        return;
    }

    // Cure found and being distributed
    if (globalStats.cureFound) {
        // Cure gradually eliminates infection
        for (auto& region : regions) {
            RegionStats& rs = region.getStats();
            if (rs.infrastructure > 0.2 && rs.medicalCapacity > 0.2) {
                long long cured = static_cast<long long>(rs.infected * 0.1 * rs.medicalCapacity);
                rs.infected -= cured;
                rs.immune += cured;

                // Cure also weakens zombies (increased decay)
                long long weakened = static_cast<long long>(rs.zombies * 0.02 * rs.medicalCapacity);
                rs.zombies -= weakened;
                rs.dead += weakened;
            }
        }
    }
}

void Simulation::displayWorldStatus() const {
    std::cout << "\n================================================================"
              << "====================================================\n";
    std::cout << "  DAY " << globalStats.currentDay << " | WORLD STATUS"
              << (globalStats.cureFound ? " | *** CURE FOUND ***" : "") << "\n";
    std::cout << "================================================================"
              << "====================================================\n";

    for (const auto& r : regions) {
        r.displayStatus();
    }

    std::cout << "----------------------------------------------------------------"
              << "----------------------------------------------------\n";

    double survivalRate = static_cast<double>(globalStats.totalHealthy + globalStats.totalImmune +
                          globalStats.totalEvacuated) / std::max(1LL, globalStats.worldPopulation) * 100.0;

    std::cout << "  GLOBAL: Alive=" << (globalStats.totalHealthy + globalStats.totalImmune + globalStats.totalEvacuated)
              << " | Infected=" << globalStats.totalInfected
              << " | Zombies=" << globalStats.totalZombies
              << " | Dead=" << globalStats.totalDead
              << " | Survival: " << std::fixed << std::setprecision(1) << survivalRate << "%\n";
    std::cout << "  Cure Progress: " << std::fixed << std::setprecision(1)
              << (globalStats.globalCureProgress * 100) << "% | "
              << "Regions Collapsed: " << globalStats.regionsCollapsed << "/" << regions.size() << " | "
              << "Regions Cleared: " << globalStats.regionsCleared << "/" << regions.size() << "\n";
    std::cout << "================================================================"
              << "====================================================\n";
}

void Simulation::displayRegionDetail(int regionIndex) const {
    if (regionIndex < 0 || regionIndex >= static_cast<int>(regions.size())) {
        std::cout << "  Invalid region index.\n";
        return;
    }
    regions[regionIndex].displayDetailedStatus();
}

void Simulation::displayGlobalStats() const {
    std::cout << "\n========================================\n";
    std::cout << "         GLOBAL STATISTICS\n";
    std::cout << "========================================\n";
    std::cout << "  Day:              " << globalStats.currentDay << "\n";
    std::cout << "  World Population: " << globalStats.worldPopulation << "\n";
    std::cout << "  Healthy:          " << globalStats.totalHealthy << "\n";
    std::cout << "  Infected:         " << globalStats.totalInfected << "\n";
    std::cout << "  Zombies:          " << globalStats.totalZombies << "\n";
    std::cout << "  Dead:             " << globalStats.totalDead << "\n";
    std::cout << "  Immune:           " << globalStats.totalImmune << "\n";
    std::cout << "  Evacuated:        " << globalStats.totalEvacuated << "\n";
    std::cout << "  Cure Progress:    " << std::fixed << std::setprecision(1)
              << (globalStats.globalCureProgress * 100) << "%\n";
    std::cout << "  Virus Generation: " << virus.getMutationGeneration() << "\n";
    std::cout << "========================================\n";
}

void Simulation::displayEventLog(int lastN) const {
    std::cout << "\n========================================\n";
    std::cout << "           EVENT LOG\n";
    std::cout << "========================================\n";

    int start = static_cast<int>(eventLog.size()) - lastN;
    if (start < 0) start = 0;

    for (int i = start; i < static_cast<int>(eventLog.size()); i++) {
        std::cout << "  [Day " << std::setw(4) << eventLog[i].day << "] "
                  << std::left << std::setw(22) << eventLog[i].region
                  << " " << eventLog[i].description << "\n";
    }

    if (eventLog.empty()) {
        std::cout << "  No events recorded yet.\n";
    }
    std::cout << "========================================\n";
}

void Simulation::displayFinalReport() const {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "              ZOMBIE APOCALYPSE - FINAL REPORT\n";
    std::cout << "================================================================\n\n";

    if (globalStats.humanityExtinct) {
        std::cout << "  RESULT: EXTINCTION\n";
        std::cout << "  Humanity has been completely wiped out.\n";
        std::cout << "  The undead now roam a silent world.\n\n";
    } else if (globalStats.humanityWon) {
        std::cout << "  RESULT: HUMANITY SURVIVES\n";
        std::cout << "  The zombie threat has been eliminated.\n";
        std::cout << "  Rebuilding can begin.\n\n";
    } else {
        std::cout << "  RESULT: SIMULATION ENDED (Time limit)\n\n";
    }

    std::cout << "  Duration:         " << globalStats.currentDay << " days ("
              << (globalStats.currentDay / 30) << " months)\n";
    std::cout << "  World Population: " << globalStats.worldPopulation << "\n";

    long long survivors = globalStats.totalHealthy + globalStats.totalImmune + globalStats.totalEvacuated;
    double survivalRate = static_cast<double>(survivors) / std::max(1LL, globalStats.worldPopulation) * 100.0;
    std::cout << "  Survivors:        " << survivors
              << " (" << std::fixed << std::setprecision(2) << survivalRate << "%)\n";
    std::cout << "  Total Dead:       " << globalStats.totalDead << "\n";
    std::cout << "  Remaining Zombies:" << globalStats.totalZombies << "\n";
    std::cout << "  Immune:           " << globalStats.totalImmune << "\n";
    std::cout << "  Evacuated:        " << globalStats.totalEvacuated << "\n";
    std::cout << "  Cure Found:       " << (globalStats.cureFound ? "Yes" : "No") << "\n";
    std::cout << "  Virus Mutations:  " << virus.getMutationGeneration() << "\n\n";

    std::cout << "  --- Region Final Status ---\n";
    for (const auto& r : regions) {
        const RegionStats& rs = r.getStats();
        double regionSurvival = static_cast<double>(rs.healthy + rs.immune + rs.evacuated) /
                                std::max(1LL, rs.totalPopulation) * 100.0;
        std::cout << "  " << std::left << std::setw(22) << rs.name
                  << " Survival: " << std::fixed << std::setprecision(1) << std::setw(6) << regionSurvival << "% "
                  << r.getStatusLabel() << "\n";
    }

    std::cout << "\n  --- Key Events ---\n";
    int eventsToShow = std::min(20, static_cast<int>(eventLog.size()));
    int start = static_cast<int>(eventLog.size()) - eventsToShow;
    for (int i = start; i < static_cast<int>(eventLog.size()); i++) {
        std::cout << "  [Day " << std::setw(4) << eventLog[i].day << "] "
                  << eventLog[i].description << "\n";
    }

    std::cout << "\n================================================================\n";
}

const GlobalStats& Simulation::getGlobalStats() const {
    return globalStats;
}

const std::vector<Region>& Simulation::getRegions() const {
    return regions;
}

int Simulation::getRegionCount() const {
    return static_cast<int>(regions.size());
}

bool Simulation::isSimulationOver() const {
    return simulationOver;
}

int Simulation::getCurrentDay() const {
    return globalStats.currentDay;
}

void Simulation::logEvent(const std::string& region, const std::string& description) {
    eventLog.push_back({globalStats.currentDay, region, description});
}
