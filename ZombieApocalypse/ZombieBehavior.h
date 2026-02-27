#ifndef ZOMBIE_BEHAVIOR_H
#define ZOMBIE_BEHAVIOR_H

#include <string>
#include <iostream>

// Movement speed classification
enum SpeedClass {
    CRAWLER,    // Broken/decayed, dragging along ground
    SHAMBLER,   // Classic slow zombie
    WALKER,     // Steady walking pace
    JOGGER,     // Faster than walking, sustainable pace
    RUNNER,     // Full running speed
    SPRINTER    // Burst speed, faster than most humans
};

// Cognitive ability level
enum IntelligenceLevel {
    MINDLESS,    // Pure instinct, no problem solving
    BASIC,       // Can open doors, navigate simple obstacles
    CUNNING,     // Sets ambushes, remembers locations
    TACTICAL,    // Coordinates with other zombies
    STRATEGIC    // Plans, uses tools, leads hordes
};

// Aggression behavior
enum AggressionType {
    DORMANT,       // Only attacks when disturbed
    PASSIVE,       // Wanders, attacks if prey is close
    TERRITORIAL,   // Defends an area aggressively
    AGGRESSIVE,    // Actively hunts prey
    BERSERK        // Relentless, attacks anything that moves
};

// What triggers zombie awareness
enum SensoryStrength {
    BLIND,     // Relies on sound/smell only
    POOR,      // Very limited vision
    NORMAL,    // Human-level senses
    ENHANCED,  // Better than human senses
    PREDATORY  // Exceptional tracking ability
};

struct ZombieConfig {
    // --- Movement ---
    SpeedClass speed;
    double stamina;             // 0.0 - 1.0: how long they can sustain speed
    bool canClimb;              // Can scale walls/fences
    bool canSwim;               // Can cross water

    // --- Intelligence ---
    IntelligenceLevel intelligence;
    bool usesTools;             // Can pick up and use objects
    bool opensDoors;            // Can figure out doors/latches
    bool avoidsTraps;           // Can detect and avoid traps

    // --- Aggression ---
    AggressionType aggression;
    double attackDamage;        // 1.0 - 10.0: damage multiplier
    bool attacksAnimals;        // Targets non-human creatures
    bool attacksOtherZombies;   // Infighting possible

    // --- Senses ---
    SensoryStrength sightLevel;
    SensoryStrength hearingLevel;
    SensoryStrength smellLevel;
    double detectionRange;      // Meters: base detection range
    bool nightVision;           // Can see in darkness

    // --- Social / Horde ---
    double hordeTendency;       // 0.0 - 1.0: likelihood of forming groups
    int maxHordeSize;           // Maximum horde size
    bool hordeCoordination;     // Hordes act as a unit
    bool alphaZombies;          // Special leader zombies emerge

    // --- Durability ---
    double durability;          // 0.0 - 1.0: how much damage they can take
    double decayRate;           // 0.0 - 1.0: how fast they deteriorate
    bool headshotOnly;          // Only headshots kill them
    double regeneration;        // 0.0 - 1.0: can they heal/repair

    // --- Special Abilities ---
    bool canInfectWithScream;   // Sonic-based infection spread
    bool explodesOnDeath;       // Detonates when killed
    bool canPlayDead;           // Feigns death to ambush
    double strengthMultiplier;  // 1.0 - 5.0: physical strength vs human
};

class ZombieBehavior {
public:
    ZombieBehavior();

    // Configure all parameters interactively
    void configure();

    // Apply a preset
    void applyPreset(int presetIndex);

    // Display current configuration
    void displayConfig() const;

    // Getters
    const ZombieConfig& getConfig() const;

    // Calculate threat level (composite score)
    double calculateThreatLevel() const;

    // Get speed as string
    std::string getSpeedStr() const;
    std::string getIntelligenceStr() const;
    std::string getAggressionStr() const;
    std::string getSensoryStr(SensoryStrength s) const;

    // Get effective speed in km/h
    double getSpeedKmh() const;

private:
    ZombieConfig config;

    void showPresets() const;
    double clamp(double val, double minVal, double maxVal) const;
    double promptDouble(const std::string& prompt, double minVal, double maxVal, double defaultVal) const;
    int promptInt(const std::string& prompt, int minVal, int maxVal, int defaultVal) const;
    bool promptBool(const std::string& prompt, bool defaultVal) const;
};

#endif
