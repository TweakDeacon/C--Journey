#include "Virus.h"
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <limits>

Virus::Virus() : mutationGeneration(0) {
    // Default: moderate zombie virus
    config.name = "Z-Virus Alpha";
    config.infectionRate = 0.3;
    config.incubationHours = 24.0;
    config.lethalityRate = 0.1;
    config.reanimationRate = 0.8;
    config.transmissionModes = CONTACT | BLOODBORNE;
    config.airborneRange = 0.0;
    config.environmentalSurvival = 0.3;
    config.mutationTendency = SLOW_DRIFT;
    config.mutationRate = 0.1;
    config.cureResistance = 0.5;
    config.heatVulnerability = 0.3;
    config.coldVulnerability = 0.2;
    config.humidityBonus = 0.1;
    config.immunityChance = 0.02;
    config.partialResistance = 0.05;
}

void Virus::showPresets() const {
    std::cout << "\n========================================\n";
    std::cout << "         VIRUS STRAIN PRESETS\n";
    std::cout << "========================================\n\n";

    std::cout << "  [1] Classic Romero\n";
    std::cout << "      Slow spread, contact only, low mutation\n";
    std::cout << "      Think: Night of the Living Dead\n\n";

    std::cout << "  [2] Rage Virus\n";
    std::cout << "      Instant turning, bloodborne, very aggressive\n";
    std::cout << "      Think: 28 Days Later\n\n";

    std::cout << "  [3] Cordyceps Evolved\n";
    std::cout << "      Airborne spores, long incubation, high mutation\n";
    std::cout << "      Think: The Last of Us\n\n";

    std::cout << "  [4] Solanum\n";
    std::cout << "      100% lethal, contact only, no cure possible\n";
    std::cout << "      Think: World War Z (book)\n\n";

    std::cout << "  [5] Necroa Virus\n";
    std::cout << "      Multi-vector, high reanimation, evolving\n";
    std::cout << "      Think: Plague Inc.\n\n";

    std::cout << "  [6] Custom Configuration\n";
    std::cout << "      Set every parameter yourself\n\n";
}

void Virus::applyPreset(int presetIndex) {
    switch (presetIndex) {
        case 1: // Classic Romero
            config.name = "Romero Strain";
            config.infectionRate = 0.25;
            config.incubationHours = 48.0;
            config.lethalityRate = 0.05;
            config.reanimationRate = 0.95;
            config.transmissionModes = CONTACT;
            config.airborneRange = 0.0;
            config.environmentalSurvival = 0.1;
            config.mutationTendency = STABLE;
            config.mutationRate = 0.02;
            config.cureResistance = 0.3;
            config.heatVulnerability = 0.4;
            config.coldVulnerability = 0.1;
            config.humidityBonus = 0.05;
            config.immunityChance = 0.01;
            config.partialResistance = 0.03;
            break;

        case 2: // Rage Virus
            config.name = "Rage Pathogen";
            config.infectionRate = 0.85;
            config.incubationHours = 0.5; // 30 seconds
            config.lethalityRate = 0.0;   // Doesn't kill, transforms
            config.reanimationRate = 0.0; // Not undead, just enraged
            config.transmissionModes = BLOODBORNE | CONTACT;
            config.airborneRange = 0.0;
            config.environmentalSurvival = 0.05;
            config.mutationTendency = STABLE;
            config.mutationRate = 0.01;
            config.cureResistance = 0.4;
            config.heatVulnerability = 0.2;
            config.coldVulnerability = 0.3;
            config.humidityBonus = 0.0;
            config.immunityChance = 0.001;
            config.partialResistance = 0.01;
            break;

        case 3: // Cordyceps Evolved
            config.name = "Cordyceps Brain Infection";
            config.infectionRate = 0.45;
            config.incubationHours = 72.0;
            config.lethalityRate = 0.15;
            config.reanimationRate = 0.7;
            config.transmissionModes = AIRBORNE | CONTACT | VECTOR;
            config.airborneRange = 5.0;
            config.environmentalSurvival = 0.8;
            config.mutationTendency = MODERATE_SHIFT;
            config.mutationRate = 0.25;
            config.cureResistance = 0.7;
            config.heatVulnerability = 0.1;
            config.coldVulnerability = 0.5;
            config.humidityBonus = 0.4;
            config.immunityChance = 0.05;
            config.partialResistance = 0.1;
            break;

        case 4: // Solanum
            config.name = "Solanum";
            config.infectionRate = 0.6;
            config.incubationHours = 23.0;
            config.lethalityRate = 1.0; // Always kills
            config.reanimationRate = 1.0; // Always reanimates
            config.transmissionModes = CONTACT | BLOODBORNE;
            config.airborneRange = 0.0;
            config.environmentalSurvival = 0.0;
            config.mutationTendency = STABLE;
            config.mutationRate = 0.0;
            config.cureResistance = 1.0; // No cure
            config.heatVulnerability = 0.2;
            config.coldVulnerability = 0.0; // Freezes but doesn't die
            config.humidityBonus = 0.0;
            config.immunityChance = 0.0;
            config.partialResistance = 0.0;
            break;

        case 5: // Necroa
            config.name = "Necroa Virus";
            config.infectionRate = 0.55;
            config.incubationHours = 36.0;
            config.lethalityRate = 0.3;
            config.reanimationRate = 0.9;
            config.transmissionModes = CONTACT | AIRBORNE | WATERBORNE | VECTOR;
            config.airborneRange = 3.0;
            config.environmentalSurvival = 0.6;
            config.mutationTendency = RAPID_MUTATION;
            config.mutationRate = 0.5;
            config.cureResistance = 0.6;
            config.heatVulnerability = 0.15;
            config.coldVulnerability = 0.15;
            config.humidityBonus = 0.2;
            config.immunityChance = 0.03;
            config.partialResistance = 0.08;
            break;

        default:
            break;
    }
}

void Virus::configure() {
    showPresets();

    int choice = promptInt("Select strain preset (1-6)", 1, 6, 1);

    if (choice >= 1 && choice <= 5) {
        applyPreset(choice);
        std::cout << "\n  Loaded preset: " << config.name << "\n";

        std::cout << "\n  Would you like to tweak any parameters? (1=Yes, 0=No): ";
        int tweak = promptInt("Tweak parameters?", 0, 1, 0);
        if (tweak == 0) {
            displayConfig();
            return;
        }
    }

    // Full custom configuration
    std::cout << "\n========================================\n";
    std::cout << "       CUSTOM VIRUS CONFIGURATION\n";
    std::cout << "========================================\n";

    std::cout << "\n  Enter virus name: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, config.name);
    if (config.name.empty()) config.name = "Z-Virus Alpha";

    std::cout << "\n--- SPREAD PARAMETERS ---\n";
    config.infectionRate = promptDouble(
        "Infection rate (0.0=harmless, 1.0=guaranteed)", 0.0, 1.0, config.infectionRate);
    config.incubationHours = promptDouble(
        "Incubation period in hours (0.1=instant, 168=one week)", 0.1, 720.0, config.incubationHours);
    config.lethalityRate = promptDouble(
        "Lethality rate (0.0=always turns, 1.0=always kills outright)", 0.0, 1.0, config.lethalityRate);
    config.reanimationRate = promptDouble(
        "Reanimation rate (0.0=stay dead, 1.0=always reanimate)", 0.0, 1.0, config.reanimationRate);

    std::cout << "\n--- TRANSMISSION MODES ---\n";
    std::cout << "  Select active transmission modes:\n";
    config.transmissionModes = 0;

    std::cout << "  Contact/bite transmission? (1=Yes, 0=No): ";
    if (promptInt("Contact?", 0, 1, 1)) config.transmissionModes |= CONTACT;

    std::cout << "  Airborne transmission? (1=Yes, 0=No): ";
    if (promptInt("Airborne?", 0, 1, 0)) {
        config.transmissionModes |= AIRBORNE;
        config.airborneRange = promptDouble(
            "Airborne range in meters", 0.5, 50.0, 3.0);
    }

    std::cout << "  Waterborne transmission? (1=Yes, 0=No): ";
    if (promptInt("Waterborne?", 0, 1, 0)) config.transmissionModes |= WATERBORNE;

    std::cout << "  Bloodborne transmission? (1=Yes, 0=No): ";
    if (promptInt("Bloodborne?", 0, 1, 1)) config.transmissionModes |= BLOODBORNE;

    std::cout << "  Vector transmission (animals/insects)? (1=Yes, 0=No): ";
    if (promptInt("Vector?", 0, 1, 0)) config.transmissionModes |= VECTOR;

    config.environmentalSurvival = promptDouble(
        "Environmental survival (0.0=fragile, 1.0=indestructible outside host)", 0.0, 1.0, config.environmentalSurvival);

    std::cout << "\n--- EVOLUTION ---\n";
    std::cout << "  Mutation tendency:\n";
    std::cout << "    0 = Stable\n";
    std::cout << "    1 = Slow Drift\n";
    std::cout << "    2 = Moderate Shift\n";
    std::cout << "    3 = Rapid Mutation\n";
    std::cout << "    4 = Hypermutation\n";
    int mt = promptInt("Mutation tendency (0-4)", 0, 4, static_cast<int>(config.mutationTendency));
    config.mutationTendency = static_cast<MutationTendency>(mt);

    config.mutationRate = promptDouble(
        "Mutation speed (0.0=never, 1.0=every cycle)", 0.0, 1.0, config.mutationRate);
    config.cureResistance = promptDouble(
        "Cure resistance (0.0=easy cure, 1.0=impossible to cure)", 0.0, 1.0, config.cureResistance);

    std::cout << "\n--- ENVIRONMENTAL FACTORS ---\n";
    config.heatVulnerability = promptDouble(
        "Heat vulnerability (0.0=heat-proof, 1.0=destroyed by heat)", 0.0, 1.0, config.heatVulnerability);
    config.coldVulnerability = promptDouble(
        "Cold vulnerability (0.0=cold-proof, 1.0=destroyed by cold)", 0.0, 1.0, config.coldVulnerability);
    config.humidityBonus = promptDouble(
        "Humidity bonus (0.0=no effect, 1.0=thrives in humidity)", 0.0, 1.0, config.humidityBonus);

    std::cout << "\n--- HOST INTERACTION ---\n";
    config.immunityChance = promptDouble(
        "Natural immunity chance (0.0=nobody, 1.0=everyone immune)", 0.0, 1.0, config.immunityChance);
    config.partialResistance = promptDouble(
        "Partial resistance chance (slower infection)", 0.0, 1.0, config.partialResistance);

    displayConfig();
}

void Virus::displayConfig() const {
    std::cout << "\n========================================\n";
    std::cout << "    VIRUS PROFILE: " << config.name << "\n";
    std::cout << "========================================\n";
    std::cout << "  Infection Rate:      " << (config.infectionRate * 100) << "%\n";
    std::cout << "  Incubation Period:   " << config.incubationHours << " hours\n";
    std::cout << "  Lethality Rate:      " << (config.lethalityRate * 100) << "%\n";
    std::cout << "  Reanimation Rate:    " << (config.reanimationRate * 100) << "%\n";
    std::cout << "  Transmission:        " << getTransmissionDescription() << "\n";
    if (hasTransmission(AIRBORNE))
        std::cout << "  Airborne Range:      " << config.airborneRange << "m\n";
    std::cout << "  Env. Survival:       " << (config.environmentalSurvival * 100) << "%\n";
    std::cout << "  Mutation Tendency:   " << getMutationTendencyStr() << "\n";
    std::cout << "  Mutation Rate:       " << (config.mutationRate * 100) << "%\n";
    std::cout << "  Cure Resistance:     " << (config.cureResistance * 100) << "%\n";
    std::cout << "  Heat Vulnerability:  " << (config.heatVulnerability * 100) << "%\n";
    std::cout << "  Cold Vulnerability:  " << (config.coldVulnerability * 100) << "%\n";
    std::cout << "  Humidity Bonus:      " << (config.humidityBonus * 100) << "%\n";
    std::cout << "  Natural Immunity:    " << (config.immunityChance * 100) << "%\n";
    std::cout << "  Partial Resistance:  " << (config.partialResistance * 100) << "%\n";
    std::cout << "  Mutation Generation: " << mutationGeneration << "\n";
    std::cout << "========================================\n";
}

const VirusConfig& Virus::getConfig() const {
    return config;
}

double Virus::getEffectiveInfectionRate(double temperature, double humidity) const {
    double rate = config.infectionRate;

    // Hot climates reduce infection if virus is heat-vulnerable
    if (temperature > 30.0) {
        double heatPenalty = config.heatVulnerability * ((temperature - 30.0) / 20.0);
        rate *= (1.0 - clamp(heatPenalty, 0.0, 0.8));
    }

    // Cold climates reduce infection if virus is cold-vulnerable
    if (temperature < 0.0) {
        double coldPenalty = config.coldVulnerability * (std::abs(temperature) / 40.0);
        rate *= (1.0 - clamp(coldPenalty, 0.0, 0.8));
    }

    // Humidity boosts infection
    rate *= (1.0 + config.humidityBonus * humidity * 0.5);

    return clamp(rate, 0.0, 1.0);
}

bool Virus::hasTransmission(TransmissionMode mode) const {
    return (config.transmissionModes & mode) != 0;
}

void Virus::mutate() {
    double roll = static_cast<double>(rand()) / RAND_MAX;
    double threshold;

    switch (config.mutationTendency) {
        case STABLE:         threshold = 0.02; break;
        case SLOW_DRIFT:     threshold = 0.10; break;
        case MODERATE_SHIFT: threshold = 0.25; break;
        case RAPID_MUTATION: threshold = 0.50; break;
        case HYPERMUTATION:  threshold = 0.80; break;
        default:             threshold = 0.10; break;
    }

    if (roll < threshold * config.mutationRate) {
        mutationGeneration++;

        // Random stat shifts
        double drift = (static_cast<double>(rand()) / RAND_MAX - 0.5) * 0.1;
        config.infectionRate = clamp(config.infectionRate + drift, 0.0, 1.0);

        drift = (static_cast<double>(rand()) / RAND_MAX - 0.5) * 4.0;
        config.incubationHours = clamp(config.incubationHours + drift, 0.1, 720.0);

        drift = (static_cast<double>(rand()) / RAND_MAX - 0.5) * 0.05;
        config.lethalityRate = clamp(config.lethalityRate + drift, 0.0, 1.0);

        drift = (static_cast<double>(rand()) / RAND_MAX - 0.5) * 0.05;
        config.cureResistance = clamp(config.cureResistance + drift, 0.0, 1.0);

        // Small chance of gaining a new transmission mode
        if (static_cast<double>(rand()) / RAND_MAX < 0.05 * config.mutationRate) {
            int newMode = 1 << (rand() % 5);
            if (!(config.transmissionModes & newMode)) {
                config.transmissionModes |= newMode;
            }
        }
    }
}

int Virus::getMutationGeneration() const {
    return mutationGeneration;
}

std::string Virus::getTransmissionDescription() const {
    std::string desc;
    if (config.transmissionModes & CONTACT) desc += "Contact ";
    if (config.transmissionModes & AIRBORNE) desc += "Airborne ";
    if (config.transmissionModes & WATERBORNE) desc += "Waterborne ";
    if (config.transmissionModes & BLOODBORNE) desc += "Bloodborne ";
    if (config.transmissionModes & VECTOR) desc += "Vector ";
    if (desc.empty()) desc = "None";
    return desc;
}

std::string Virus::getMutationTendencyStr() const {
    switch (config.mutationTendency) {
        case STABLE:         return "Stable";
        case SLOW_DRIFT:     return "Slow Drift";
        case MODERATE_SHIFT: return "Moderate Shift";
        case RAPID_MUTATION: return "Rapid Mutation";
        case HYPERMUTATION:  return "Hypermutation";
        default:             return "Unknown";
    }
}

double Virus::clamp(double val, double minVal, double maxVal) const {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

double Virus::promptDouble(const std::string& prompt, double minVal, double maxVal, double defaultVal) const {
    double val;
    std::cout << "  " << prompt << " [" << defaultVal << "]: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return defaultVal;
    try {
        val = std::stod(line);
        return clamp(val, minVal, maxVal);
    } catch (...) {
        return defaultVal;
    }
}

int Virus::promptInt(const std::string& prompt, int minVal, int maxVal, int defaultVal) const {
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
