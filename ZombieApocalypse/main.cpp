#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <thread>
#include <chrono>

#include "Virus.h"
#include "ZombieBehavior.h"
#include "Region.h"
#include "Simulation.h"

void clearScreen() {
    std::cout << "\033[2J\033[H";
}

void printBanner() {
    std::cout << R"(
    ╔══════════════════════════════════════════════════════════════╗
    ║                                                              ║
    ║       ███████╗ ██████╗ ███╗   ███╗██████╗ ██╗███████╗       ║
    ║       ╚══███╔╝██╔═══██╗████╗ ████║██╔══██╗██║██╔════╝       ║
    ║         ███╔╝ ██║   ██║██╔████╔██║██████╔╝██║█████╗         ║
    ║        ███╔╝  ██║   ██║██║╚██╔╝██║██╔══██╗██║██╔══╝         ║
    ║       ███████╗╚██████╔╝██║ ╚═╝ ██║██████╔╝██║███████╗       ║
    ║       ╚══════╝ ╚═════╝ ╚═╝     ╚═╝╚═════╝ ╚═╝╚══════╝       ║
    ║                                                              ║
    ║          A P O C A L Y P S E   S I M U L A T O R            ║
    ║                                                              ║
    ║            Worldwide Zombie Outbreak Simulation               ║
    ║                                                              ║
    ╚══════════════════════════════════════════════════════════════╝
    )" << "\n";
}

void printMainMenu() {
    std::cout << "\n========================================\n";
    std::cout << "            COMMAND CENTER\n";
    std::cout << "========================================\n";
    std::cout << "  [1] Advance 1 day\n";
    std::cout << "  [2] Advance 7 days\n";
    std::cout << "  [3] Advance 30 days\n";
    std::cout << "  [4] Advance custom days\n";
    std::cout << "  [5] Run until conclusion\n";
    std::cout << "  [6] View world status\n";
    std::cout << "  [7] View region detail\n";
    std::cout << "  [8] View global statistics\n";
    std::cout << "  [9] View event log\n";
    std::cout << "  [10] View virus profile\n";
    std::cout << "  [11] View zombie profile\n";
    std::cout << "  [0] End simulation\n";
    std::cout << "========================================\n";
    std::cout << "  Command: ";
}

int getInt(const std::string& prompt, int minVal, int maxVal) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val && val >= minVal && val <= maxVal) {
            return val;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid input. Enter a number between "
                  << minVal << " and " << maxVal << ".\n";
    }
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    clearScreen();
    printBanner();

    std::cout << "  Press Enter to begin the apocalypse...";
    std::cin.get();

    // ==========================================
    // PHASE 1: Configure the Virus
    // ==========================================
    clearScreen();
    std::cout << "\n========================================\n";
    std::cout << "    PHASE 1: CONFIGURE THE PATHOGEN\n";
    std::cout << "========================================\n";
    std::cout << "  Define the properties of the zombie virus.\n";
    std::cout << "  This determines how fast it spreads, how\n";
    std::cout << "  it transmits, and how it evolves.\n";

    Virus virus;
    virus.configure();

    std::cout << "\n  Virus configured. Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    // ==========================================
    // PHASE 2: Configure the Zombies
    // ==========================================
    clearScreen();
    std::cout << "\n========================================\n";
    std::cout << "    PHASE 2: CONFIGURE THE UNDEAD\n";
    std::cout << "========================================\n";
    std::cout << "  Define zombie behavior, abilities, and\n";
    std::cout << "  physical attributes. This determines how\n";
    std::cout << "  dangerous they are individually and in groups.\n";

    ZombieBehavior zombies;
    zombies.configure();

    std::cout << "\n  Zombies configured. Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    // ==========================================
    // PHASE 3: Select Patient Zero Location
    // ==========================================
    clearScreen();
    std::cout << "\n========================================\n";
    std::cout << "    PHASE 3: PATIENT ZERO\n";
    std::cout << "========================================\n";
    std::cout << "  Select where the outbreak begins:\n\n";

    std::cout << "   [0]  North America       (380M pop, High military)\n";
    std::cout << "   [1]  Central America      (180M pop, Low military)\n";
    std::cout << "   [2]  South America        (430M pop, Moderate military)\n";
    std::cout << "   [3]  Western Europe       (400M pop, High medical)\n";
    std::cout << "   [4]  Eastern Europe       (290M pop, Strong military)\n";
    std::cout << "   [5]  Russia               (145M pop, Strong military, Cold)\n";
    std::cout << "   [6]  Middle East          (410M pop, Arid climate)\n";
    std::cout << "   [7]  North Africa         (250M pop, Arid, Low infra)\n";
    std::cout << "   [8]  Sub-Saharan Africa   (1.2B pop, Tropical, Low infra)\n";
    std::cout << "   [9]  South Asia           (2.0B pop, Dense, Tropical)\n";
    std::cout << "   [10] East Asia            (1.6B pop, Dense, High tech)\n";
    std::cout << "   [11] Southeast Asia       (700M pop, Tropical, Dense)\n";
    std::cout << "   [12] Oceania              (45M pop, Isolated, High border)\n";
    std::cout << "\n";

    int patientZero = getInt("  Select outbreak origin (0-12): ", 0, 12);

    // ==========================================
    // PHASE 4: Initialize and Run Simulation
    // ==========================================
    clearScreen();
    Simulation sim;
    sim.setVirus(virus);
    sim.setZombieBehavior(zombies);
    sim.initializeWorld();
    sim.setPatientZeroRegion(patientZero);

    std::cout << "\n========================================\n";
    std::cout << "     OUTBREAK DETECTED\n";
    std::cout << "========================================\n";
    std::cout << "  Virus: " << virus.getConfig().name << "\n";
    std::cout << "  Zombie Type: " << zombies.getSpeedStr() << " / " << zombies.getIntelligenceStr() << "\n";
    std::cout << "  Threat Level: " << zombies.calculateThreatLevel() << "/10\n";
    std::cout << "  Origin: " << sim.getRegions()[patientZero].getStats().name << "\n";
    std::cout << "  World Population: " << sim.getGlobalStats().worldPopulation << "\n";
    std::cout << "========================================\n";
    std::cout << "\n  The simulation is ready. Press Enter to open Command Center...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    // ==========================================
    // MAIN SIMULATION LOOP
    // ==========================================
    bool running = true;

    while (running && !sim.isSimulationOver()) {
        clearScreen();
        sim.displayWorldStatus();
        printMainMenu();

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1:
                sim.advanceDay();
                break;

            case 2:
                std::cout << "\n  Advancing 7 days...\n";
                sim.advanceDays(7);
                break;

            case 3:
                std::cout << "\n  Advancing 30 days...\n";
                sim.advanceDays(30);
                break;

            case 4: {
                int days = getInt("  How many days to advance (1-3650): ", 1, 3650);
                std::cout << "\n  Advancing " << days << " days...\n";
                sim.advanceDays(days);
                break;
            }

            case 5: {
                std::cout << "\n  Running simulation to conclusion (max 3650 days / 10 years)...\n";
                sim.runUntilEnd(3650);
                break;
            }

            case 6:
                clearScreen();
                sim.displayWorldStatus();
                std::cout << "\n  Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;

            case 7: {
                std::cout << "\n  Regions:\n";
                const auto& regs = sim.getRegions();
                for (int i = 0; i < static_cast<int>(regs.size()); i++) {
                    std::cout << "    [" << i << "] " << regs[i].getStats().name << "\n";
                }
                int idx = getInt("  Select region (0-" +
                                 std::to_string(sim.getRegionCount() - 1) + "): ",
                                 0, sim.getRegionCount() - 1);
                sim.displayRegionDetail(idx);
                std::cout << "\n  Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;
            }

            case 8:
                sim.displayGlobalStats();
                std::cout << "\n  Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;

            case 9:
                sim.displayEventLog(30);
                std::cout << "\n  Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;

            case 10:
                virus.displayConfig();
                std::cout << "\n  Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;

            case 11:
                zombies.displayConfig();
                std::cout << "\n  Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;

            case 0:
                running = false;
                break;

            default:
                std::cout << "  Invalid command.\n";
                break;
        }
    }

    // ==========================================
    // FINAL REPORT
    // ==========================================
    clearScreen();
    sim.displayFinalReport();

    std::cout << "\n  Press Enter to exit...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    return 0;
}
