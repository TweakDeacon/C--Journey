#include "ZombieBehavior.h"
#include <limits>

ZombieBehavior::ZombieBehavior() {
    // Default: classic shambler zombie
    config.speed = SHAMBLER;
    config.stamina = 0.8;
    config.canClimb = false;
    config.canSwim = false;

    config.intelligence = MINDLESS;
    config.usesTools = false;
    config.opensDoors = false;
    config.avoidsTraps = false;

    config.aggression = AGGRESSIVE;
    config.attackDamage = 3.0;
    config.attacksAnimals = false;
    config.attacksOtherZombies = false;

    config.sightLevel = POOR;
    config.hearingLevel = ENHANCED;
    config.smellLevel = ENHANCED;
    config.detectionRange = 30.0;
    config.nightVision = false;

    config.hordeTendency = 0.6;
    config.maxHordeSize = 500;
    config.hordeCoordination = false;
    config.alphaZombies = false;

    config.durability = 0.4;
    config.decayRate = 0.15;
    config.headshotOnly = true;
    config.regeneration = 0.0;

    config.canInfectWithScream = false;
    config.explodesOnDeath = false;
    config.canPlayDead = false;
    config.strengthMultiplier = 1.5;
}

void ZombieBehavior::showPresets() const {
    std::cout << "\n========================================\n";
    std::cout << "       ZOMBIE BEHAVIOR PRESETS\n";
    std::cout << "========================================\n\n";

    std::cout << "  [1] Classic Shambler\n";
    std::cout << "      Slow, dumb, headshot-only, forms hordes\n";
    std::cout << "      Threat: LOW individually, HIGH in numbers\n\n";

    std::cout << "  [2] Rage Infected\n";
    std::cout << "      Sprinting, berserk, no horde coordination\n";
    std::cout << "      Threat: EXTREME individually, chaotic in groups\n\n";

    std::cout << "  [3] Clicker/Stalker\n";
    std::cout << "      Blind but echolocation, ambush predators\n";
    std::cout << "      Threat: HIGH — unpredictable and stealthy\n\n";

    std::cout << "  [4] Evolved Horde Mind\n";
    std::cout << "      Tactical, alpha leaders, coordinated attacks\n";
    std::cout << "      Threat: CATASTROPHIC — organized warfare\n\n";

    std::cout << "  [5] Necromorph\n";
    std::cout << "      Regenerating, durable, special abilities\n";
    std::cout << "      Threat: EXTREME — hard to put down\n\n";

    std::cout << "  [6] Custom Configuration\n";
    std::cout << "      Set every parameter yourself\n\n";
}

void ZombieBehavior::applyPreset(int presetIndex) {
    switch (presetIndex) {
        case 1: // Classic Shambler
            config.speed = SHAMBLER;
            config.stamina = 1.0; // Never stops
            config.canClimb = false;
            config.canSwim = false;
            config.intelligence = MINDLESS;
            config.usesTools = false;
            config.opensDoors = false;
            config.avoidsTraps = false;
            config.aggression = AGGRESSIVE;
            config.attackDamage = 2.0;
            config.attacksAnimals = false;
            config.attacksOtherZombies = false;
            config.sightLevel = POOR;
            config.hearingLevel = NORMAL;
            config.smellLevel = ENHANCED;
            config.detectionRange = 25.0;
            config.nightVision = false;
            config.hordeTendency = 0.8;
            config.maxHordeSize = 10000;
            config.hordeCoordination = false;
            config.alphaZombies = false;
            config.durability = 0.3;
            config.decayRate = 0.2;
            config.headshotOnly = true;
            config.regeneration = 0.0;
            config.canInfectWithScream = false;
            config.explodesOnDeath = false;
            config.canPlayDead = false;
            config.strengthMultiplier = 1.2;
            break;

        case 2: // Rage Infected
            config.speed = SPRINTER;
            config.stamina = 0.4; // Burns out fast
            config.canClimb = true;
            config.canSwim = false;
            config.intelligence = BASIC;
            config.usesTools = false;
            config.opensDoors = true; // Smash through
            config.avoidsTraps = false;
            config.aggression = BERSERK;
            config.attackDamage = 5.0;
            config.attacksAnimals = true;
            config.attacksOtherZombies = false;
            config.sightLevel = ENHANCED;
            config.hearingLevel = ENHANCED;
            config.smellLevel = NORMAL;
            config.detectionRange = 60.0;
            config.nightVision = false;
            config.hordeTendency = 0.3;
            config.maxHordeSize = 50;
            config.hordeCoordination = false;
            config.alphaZombies = false;
            config.durability = 0.5;
            config.decayRate = 0.4; // Burns out fast
            config.headshotOnly = false;
            config.regeneration = 0.0;
            config.canInfectWithScream = false;
            config.explodesOnDeath = false;
            config.canPlayDead = false;
            config.strengthMultiplier = 2.5;
            break;

        case 3: // Clicker/Stalker
            config.speed = WALKER;
            config.stamina = 0.7;
            config.canClimb = true;
            config.canSwim = false;
            config.intelligence = CUNNING;
            config.usesTools = false;
            config.opensDoors = false;
            config.avoidsTraps = true;
            config.aggression = TERRITORIAL;
            config.attackDamage = 7.0; // One-hit kill potential
            config.attacksAnimals = true;
            config.attacksOtherZombies = false;
            config.sightLevel = BLIND;
            config.hearingLevel = PREDATORY;
            config.smellLevel = PREDATORY;
            config.detectionRange = 40.0;
            config.nightVision = false; // Blind
            config.hordeTendency = 0.2;
            config.maxHordeSize = 20;
            config.hordeCoordination = false;
            config.alphaZombies = false;
            config.durability = 0.7;
            config.decayRate = 0.05; // Fungal armor
            config.headshotOnly = false;
            config.regeneration = 0.1;
            config.canInfectWithScream = true;
            config.explodesOnDeath = false;
            config.canPlayDead = true;
            config.strengthMultiplier = 3.0;
            break;

        case 4: // Evolved Horde Mind
            config.speed = JOGGER;
            config.stamina = 0.8;
            config.canClimb = true;
            config.canSwim = true;
            config.intelligence = STRATEGIC;
            config.usesTools = true;
            config.opensDoors = true;
            config.avoidsTraps = true;
            config.aggression = AGGRESSIVE;
            config.attackDamage = 4.0;
            config.attacksAnimals = false;
            config.attacksOtherZombies = false;
            config.sightLevel = ENHANCED;
            config.hearingLevel = ENHANCED;
            config.smellLevel = ENHANCED;
            config.detectionRange = 80.0;
            config.nightVision = true;
            config.hordeTendency = 0.95;
            config.maxHordeSize = 50000;
            config.hordeCoordination = true;
            config.alphaZombies = true;
            config.durability = 0.6;
            config.decayRate = 0.1;
            config.headshotOnly = true;
            config.regeneration = 0.05;
            config.canInfectWithScream = false;
            config.explodesOnDeath = false;
            config.canPlayDead = true;
            config.strengthMultiplier = 2.0;
            break;

        case 5: // Necromorph
            config.speed = RUNNER;
            config.stamina = 0.9;
            config.canClimb = true;
            config.canSwim = true;
            config.intelligence = BASIC;
            config.usesTools = false;
            config.opensDoors = true;
            config.avoidsTraps = false;
            config.aggression = BERSERK;
            config.attackDamage = 8.0;
            config.attacksAnimals = true;
            config.attacksOtherZombies = true;
            config.sightLevel = ENHANCED;
            config.hearingLevel = ENHANCED;
            config.smellLevel = ENHANCED;
            config.detectionRange = 50.0;
            config.nightVision = true;
            config.hordeTendency = 0.4;
            config.maxHordeSize = 100;
            config.hordeCoordination = false;
            config.alphaZombies = false;
            config.durability = 0.95;
            config.decayRate = 0.01;
            config.headshotOnly = false;
            config.regeneration = 0.5;
            config.canInfectWithScream = true;
            config.explodesOnDeath = true;
            config.canPlayDead = true;
            config.strengthMultiplier = 4.0;
            break;

        default:
            break;
    }
}

void ZombieBehavior::configure() {
    showPresets();

    int choice = promptInt("Select zombie preset (1-6)", 1, 6, 1);

    if (choice >= 1 && choice <= 5) {
        applyPreset(choice);
        std::cout << "\n  Loaded preset: " << getSpeedStr() << " / " << getIntelligenceStr() << "\n";

        int tweak = promptInt("  Tweak parameters? (1=Yes, 0=No)", 0, 1, 0);
        if (tweak == 0) {
            displayConfig();
            return;
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "     CUSTOM ZOMBIE CONFIGURATION\n";
    std::cout << "========================================\n";

    std::cout << "\n--- MOVEMENT ---\n";
    std::cout << "  Speed class:\n";
    std::cout << "    0 = Crawler (~0.5 km/h)\n";
    std::cout << "    1 = Shambler (~2 km/h)\n";
    std::cout << "    2 = Walker (~5 km/h)\n";
    std::cout << "    3 = Jogger (~10 km/h)\n";
    std::cout << "    4 = Runner (~20 km/h)\n";
    std::cout << "    5 = Sprinter (~35 km/h)\n";
    int sp = promptInt("Speed class (0-5)", 0, 5, static_cast<int>(config.speed));
    config.speed = static_cast<SpeedClass>(sp);

    config.stamina = promptDouble("Stamina (0.0=collapses immediately, 1.0=tireless)", 0.0, 1.0, config.stamina);
    config.canClimb = promptBool("Can climb walls/fences?", config.canClimb);
    config.canSwim = promptBool("Can swim/cross water?", config.canSwim);

    std::cout << "\n--- INTELLIGENCE ---\n";
    std::cout << "  Intelligence level:\n";
    std::cout << "    0 = Mindless (pure instinct)\n";
    std::cout << "    1 = Basic (opens doors, simple navigation)\n";
    std::cout << "    2 = Cunning (ambushes, remembers locations)\n";
    std::cout << "    3 = Tactical (coordinates with others)\n";
    std::cout << "    4 = Strategic (plans, uses tools, leads)\n";
    int il = promptInt("Intelligence (0-4)", 0, 4, static_cast<int>(config.intelligence));
    config.intelligence = static_cast<IntelligenceLevel>(il);

    config.usesTools = promptBool("Can use tools/objects?", config.usesTools);
    config.opensDoors = promptBool("Can open doors?", config.opensDoors);
    config.avoidsTraps = promptBool("Can detect and avoid traps?", config.avoidsTraps);

    std::cout << "\n--- AGGRESSION ---\n";
    std::cout << "  Aggression type:\n";
    std::cout << "    0 = Dormant (attacks only when disturbed)\n";
    std::cout << "    1 = Passive (wanders, attacks if prey is close)\n";
    std::cout << "    2 = Territorial (defends area)\n";
    std::cout << "    3 = Aggressive (actively hunts)\n";
    std::cout << "    4 = Berserk (relentless, attacks anything)\n";
    int ag = promptInt("Aggression (0-4)", 0, 4, static_cast<int>(config.aggression));
    config.aggression = static_cast<AggressionType>(ag);

    config.attackDamage = promptDouble("Attack damage multiplier (1.0-10.0)", 1.0, 10.0, config.attackDamage);
    config.attacksAnimals = promptBool("Attacks animals?", config.attacksAnimals);
    config.attacksOtherZombies = promptBool("Attacks other zombies (infighting)?", config.attacksOtherZombies);

    std::cout << "\n--- SENSES ---\n";
    std::cout << "  Sensory levels: 0=Blind, 1=Poor, 2=Normal, 3=Enhanced, 4=Predatory\n";
    int si = promptInt("Sight level (0-4)", 0, 4, static_cast<int>(config.sightLevel));
    config.sightLevel = static_cast<SensoryStrength>(si);
    int hi = promptInt("Hearing level (0-4)", 0, 4, static_cast<int>(config.hearingLevel));
    config.hearingLevel = static_cast<SensoryStrength>(hi);
    int smi = promptInt("Smell level (0-4)", 0, 4, static_cast<int>(config.smellLevel));
    config.smellLevel = static_cast<SensoryStrength>(smi);

    config.detectionRange = promptDouble("Base detection range (meters)", 1.0, 200.0, config.detectionRange);
    config.nightVision = promptBool("Night vision?", config.nightVision);

    std::cout << "\n--- HORDE BEHAVIOR ---\n";
    config.hordeTendency = promptDouble("Horde tendency (0.0=loners, 1.0=always group)", 0.0, 1.0, config.hordeTendency);
    config.maxHordeSize = promptInt("Maximum horde size", 1, 100000, config.maxHordeSize);
    config.hordeCoordination = promptBool("Hordes coordinate as a unit?", config.hordeCoordination);
    config.alphaZombies = promptBool("Alpha/leader zombies emerge?", config.alphaZombies);

    std::cout << "\n--- DURABILITY ---\n";
    config.durability = promptDouble("Durability (0.0=fragile, 1.0=tank)", 0.0, 1.0, config.durability);
    config.decayRate = promptDouble("Decay rate (0.0=never decays, 1.0=rots in days)", 0.0, 1.0, config.decayRate);
    config.headshotOnly = promptBool("Headshot-only kill?", config.headshotOnly);
    config.regeneration = promptDouble("Regeneration (0.0=none, 1.0=wolverine-level)", 0.0, 1.0, config.regeneration);

    std::cout << "\n--- SPECIAL ABILITIES ---\n";
    config.canInfectWithScream = promptBool("Sonic/scream infection spread?", config.canInfectWithScream);
    config.explodesOnDeath = promptBool("Explodes on death (area damage)?", config.explodesOnDeath);
    config.canPlayDead = promptBool("Can feign death to ambush?", config.canPlayDead);
    config.strengthMultiplier = promptDouble("Strength multiplier vs human (1.0-5.0)", 1.0, 5.0, config.strengthMultiplier);

    displayConfig();
}

void ZombieBehavior::displayConfig() const {
    double threat = calculateThreatLevel();
    std::string threatLabel;
    if (threat < 2.0) threatLabel = "LOW";
    else if (threat < 4.0) threatLabel = "MODERATE";
    else if (threat < 6.0) threatLabel = "HIGH";
    else if (threat < 8.0) threatLabel = "EXTREME";
    else threatLabel = "CATASTROPHIC";

    std::cout << "\n========================================\n";
    std::cout << "        ZOMBIE BEHAVIOR PROFILE\n";
    std::cout << "========================================\n";
    std::cout << "  THREAT LEVEL: " << threatLabel << " (" << threat << "/10)\n";
    std::cout << "----------------------------------------\n";

    std::cout << "  Speed:          " << getSpeedStr() << " (" << getSpeedKmh() << " km/h)\n";
    std::cout << "  Stamina:        " << (config.stamina * 100) << "%\n";
    std::cout << "  Can Climb:      " << (config.canClimb ? "Yes" : "No") << "\n";
    std::cout << "  Can Swim:       " << (config.canSwim ? "Yes" : "No") << "\n";
    std::cout << "----------------------------------------\n";

    std::cout << "  Intelligence:   " << getIntelligenceStr() << "\n";
    std::cout << "  Uses Tools:     " << (config.usesTools ? "Yes" : "No") << "\n";
    std::cout << "  Opens Doors:    " << (config.opensDoors ? "Yes" : "No") << "\n";
    std::cout << "  Avoids Traps:   " << (config.avoidsTraps ? "Yes" : "No") << "\n";
    std::cout << "----------------------------------------\n";

    std::cout << "  Aggression:     " << getAggressionStr() << "\n";
    std::cout << "  Attack Damage:  " << config.attackDamage << "x\n";
    std::cout << "  Attacks Animals:" << (config.attacksAnimals ? " Yes" : " No") << "\n";
    std::cout << "  Infighting:     " << (config.attacksOtherZombies ? "Yes" : "No") << "\n";
    std::cout << "----------------------------------------\n";

    std::cout << "  Sight:          " << getSensoryStr(config.sightLevel) << "\n";
    std::cout << "  Hearing:        " << getSensoryStr(config.hearingLevel) << "\n";
    std::cout << "  Smell:          " << getSensoryStr(config.smellLevel) << "\n";
    std::cout << "  Detection:      " << config.detectionRange << "m\n";
    std::cout << "  Night Vision:   " << (config.nightVision ? "Yes" : "No") << "\n";
    std::cout << "----------------------------------------\n";

    std::cout << "  Horde Tendency: " << (config.hordeTendency * 100) << "%\n";
    std::cout << "  Max Horde Size: " << config.maxHordeSize << "\n";
    std::cout << "  Horde Coord:    " << (config.hordeCoordination ? "Yes" : "No") << "\n";
    std::cout << "  Alpha Zombies:  " << (config.alphaZombies ? "Yes" : "No") << "\n";
    std::cout << "----------------------------------------\n";

    std::cout << "  Durability:     " << (config.durability * 100) << "%\n";
    std::cout << "  Decay Rate:     " << (config.decayRate * 100) << "%/day\n";
    std::cout << "  Headshot Only:  " << (config.headshotOnly ? "Yes" : "No") << "\n";
    std::cout << "  Regeneration:   " << (config.regeneration * 100) << "%\n";
    std::cout << "  Strength:       " << config.strengthMultiplier << "x human\n";
    std::cout << "----------------------------------------\n";

    std::cout << "  Scream Infect:  " << (config.canInfectWithScream ? "Yes" : "No") << "\n";
    std::cout << "  Explodes:       " << (config.explodesOnDeath ? "Yes" : "No") << "\n";
    std::cout << "  Plays Dead:     " << (config.canPlayDead ? "Yes" : "No") << "\n";
    std::cout << "========================================\n";
}

const ZombieConfig& ZombieBehavior::getConfig() const {
    return config;
}

double ZombieBehavior::calculateThreatLevel() const {
    double threat = 0.0;

    // Speed contribution (0-2)
    threat += static_cast<int>(config.speed) * 0.4;

    // Intelligence contribution (0-2)
    threat += static_cast<int>(config.intelligence) * 0.4;

    // Aggression contribution (0-1.5)
    threat += static_cast<int>(config.aggression) * 0.3;

    // Durability (0-1)
    threat += config.durability;

    // Regeneration (0-1)
    threat += config.regeneration;

    // Horde factor (0-1)
    threat += config.hordeTendency * (config.hordeCoordination ? 1.0 : 0.5);

    // Special abilities (0-1.5)
    if (config.canInfectWithScream) threat += 0.5;
    if (config.explodesOnDeath) threat += 0.5;
    if (config.canPlayDead) threat += 0.3;
    if (config.alphaZombies) threat += 0.5;

    // Headshot only is actually a weakness (easier to plan against)
    if (!config.headshotOnly) threat += 0.3;

    return clamp(threat, 0.0, 10.0);
}

std::string ZombieBehavior::getSpeedStr() const {
    switch (config.speed) {
        case CRAWLER:  return "Crawler";
        case SHAMBLER: return "Shambler";
        case WALKER:   return "Walker";
        case JOGGER:   return "Jogger";
        case RUNNER:   return "Runner";
        case SPRINTER: return "Sprinter";
        default:       return "Unknown";
    }
}

std::string ZombieBehavior::getIntelligenceStr() const {
    switch (config.intelligence) {
        case MINDLESS:  return "Mindless";
        case BASIC:     return "Basic";
        case CUNNING:   return "Cunning";
        case TACTICAL:  return "Tactical";
        case STRATEGIC: return "Strategic";
        default:        return "Unknown";
    }
}

std::string ZombieBehavior::getAggressionStr() const {
    switch (config.aggression) {
        case DORMANT:     return "Dormant";
        case PASSIVE:     return "Passive";
        case TERRITORIAL: return "Territorial";
        case AGGRESSIVE:  return "Aggressive";
        case BERSERK:     return "Berserk";
        default:          return "Unknown";
    }
}

std::string ZombieBehavior::getSensoryStr(SensoryStrength s) const {
    switch (s) {
        case BLIND:    return "Blind";
        case POOR:     return "Poor";
        case NORMAL:   return "Normal";
        case ENHANCED: return "Enhanced";
        case PREDATORY:return "Predatory";
        default:       return "Unknown";
    }
}

double ZombieBehavior::getSpeedKmh() const {
    switch (config.speed) {
        case CRAWLER:  return 0.5;
        case SHAMBLER: return 2.0;
        case WALKER:   return 5.0;
        case JOGGER:   return 10.0;
        case RUNNER:   return 20.0;
        case SPRINTER: return 35.0;
        default:       return 2.0;
    }
}

double ZombieBehavior::clamp(double val, double minVal, double maxVal) const {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

double ZombieBehavior::promptDouble(const std::string& prompt, double minVal, double maxVal, double defaultVal) const {
    std::cout << "  " << prompt << " [" << defaultVal << "]: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return defaultVal;
    try {
        double val = std::stod(line);
        return clamp(val, minVal, maxVal);
    } catch (...) {
        return defaultVal;
    }
}

int ZombieBehavior::promptInt(const std::string& prompt, int minVal, int maxVal, int defaultVal) const {
    int val;
    std::cout << "  " << prompt << " [" << defaultVal << "]: ";
    if (std::cin >> val) {
        if (val < minVal) return minVal;
        if (val > maxVal) return maxVal;
        return val;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return defaultVal;
}

bool ZombieBehavior::promptBool(const std::string& prompt, bool defaultVal) const {
    std::cout << "  " << prompt << " (" << (defaultVal ? "Y/n" : "y/N") << "): ";
    std::string line;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, line);
    if (line.empty()) return defaultVal;
    return (line[0] == 'y' || line[0] == 'Y' || line[0] == '1');
}
