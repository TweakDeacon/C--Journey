#ifndef SIMULATION_H
#define SIMULATION_H

#include "Virus.h"
#include "ZombieBehavior.h"
#include "Region.h"
#include <vector>
#include <string>

struct GlobalStats {
    long long totalHealthy;
    long long totalInfected;
    long long totalZombies;
    long long totalDead;
    long long totalImmune;
    long long totalEvacuated;
    long long worldPopulation;
    int currentDay;
    int regionsCollapsed;
    int regionsCleared;
    double globalCureProgress;
    bool cureFound;
    bool humanityExtinct;
    bool humanityWon;
};

struct SimEvent {
    int day;
    std::string region;
    std::string description;
};

class Simulation {
public:
    Simulation();

    // Setup
    void setVirus(const Virus& virus);
    void setZombieBehavior(const ZombieBehavior& behavior);
    void initializeWorld();
    void setPatientZeroRegion(int regionIndex);

    // Simulation control
    void advanceDay();
    void advanceDays(int count);
    void runUntilEnd(int maxDays);

    // Display
    void displayWorldStatus() const;
    void displayRegionDetail(int regionIndex) const;
    void displayGlobalStats() const;
    void displayEventLog(int lastN) const;
    void displayFinalReport() const;

    // Getters
    const GlobalStats& getGlobalStats() const;
    const std::vector<Region>& getRegions() const;
    int getRegionCount() const;
    bool isSimulationOver() const;
    int getCurrentDay() const;

private:
    Virus virus;
    ZombieBehavior zombieBehavior;
    std::vector<Region> regions;
    GlobalStats globalStats;
    std::vector<SimEvent> eventLog;
    bool simulationOver;

    // Internal simulation steps
    void updateGlobalStats();
    void spreadBetweenRegions();
    void checkMilestones();
    void checkEndConditions();
    void logEvent(const std::string& region, const std::string& description);

    // World region adjacency (simplified)
    std::vector<std::vector<int>> adjacency;
    void buildAdjacency();
};

#endif
