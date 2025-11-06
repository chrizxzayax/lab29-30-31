//LakePulsing
//skeleton for the Clownfish Population Dynamics Simulator ("LakePulse").
// This file contains only includes, type definitions, function signatures, and detailed comments
// describing what each function will implement. Use this wireframe as a guide to implement
// the actual simulation functions in separate milestones.

#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <list>
#include <map>

using namespace std;


  const int MONTHS_PER_YEAR = 12;
  const int TOTAL_YEARS = 12;
  const int TOTAL_MONTHS = 144;
  const int JUVENILE_AGE_THRESHOLD = 6;
  const int SENIOR_AGE_THRESHOLD = 60;
  const double BASE_REPRO_RATE = 0.25;
/*
  NATURAL_MORTALITY = 0.005
  POLLUTION_MORT_MULT = 0.6
  OVERCROWDING_CAPACITY = 50
  SNAPSHOT_INTERVAL = 12
*/

// -------------------------
// Data types
// -------------------------
struct Clownfish {
    string id;          // unique id or name
    int age_months;     // age in months
    double health;      // 0.0 .. 1.0
    double tolerance;   // 0.0 .. 1.0 (higher = more pollution tolerant)
    char sex;           // 'M' or 'F' (optional)
    Clownfish(const string &id_, int age_, double health_, double tol_, char sex_)
        : id(id_), age_months(age_), health(health_), tolerance(tol_), sex(sex_) {}
};

struct ZoneEnv {
    double water_quality;   // 0.0 (poor) .. 1.0 (excellent)
    double pollution_rate;  // rate of pollution increase per month
    ZoneEnv() : water_quality(1.0), pollution_rate(0.01) {}
};

// Top-level structures:
using ZoneValue = array<list<Clownfish>, 3>;
using LakeMap = map<string, ZoneValue>;
using EnvMap  = map<string, ZoneEnv>;

// -------------------------
// Function prototypes (what each will do)
// -------------------------

// 1) load_initial_data
// - Input: filename (string), outputs: LakeMap & EnvMap (passed by reference)
// - Open CSV file with format: zone,name,age_months,health,tolerance,sex
// - For each line:
//     - parse fields and create Clownfish object
//     - determine bucket index: 0 juvenile (<JUV thresh), 1 adult, 2 senior
//     - if new zone: initialize LakeMap[zone] (3 empty lists) and EnvMap[zone] default ZoneEnv
//     - push_back the clownfish into the corresponding list
// - Count lines read; if < 100 -> return error or warning
// - Validate fields and handle malformed lines gracefully
// - Use deterministic RNG seed option for testing

// RNG helper function prototype
static std::mt19937 rng_engine((unsigned)time(nullptr)); // Seed with current time or fixed seed for testing
inline double uniform01() { return std::uniform_real_distribution<double>(0.0, 1.0)(rng_engine); }// I arbirtrarily chose 0.0 to 1.0 as the ranges

int age_bucket(int age_months) {
    if (age_months < JUVENILE_AGE_THRESHOLD) return 0;
    else if (age_months < SENIOR_AGE_THRESHOLD) return 1;
    else return 2;
}
string make_id(int serial){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "CF%04d", serial);
    return string(buffer);
}

// load initial data
bool load_initial_data(const string &filename, LakeMap &lake_map, EnvMap &env_map) {
    // Implementation goes here
    return true; // placeholder
}
// 2) print_snapshot
// - Input: month (int), lake_map, env_map
// - Output formatted console text:
//      - header: Month, Year
//      - for each zone:
//           - water_quality (formatted decimal), counts J/A/S, total
//           - optionally list 1-3 sample fish per class with (id, age, health, tolerance)
// - Use setw and aligned columns for neatness

// 3) update_zone_environment
// - Input: ZoneEnv &env, current month
// - Update env.water_quality using:
//     env.water_quality -= ALPHA * env.pollution_rate
//     env.water_quality += BETA * (1.0 - env.water_quality)
//   Optionally decrease water_quality by SEASONAL_PULSE at a chosen month each year
// - Clamp water_quality to [0,1]
// - No return (mutate ZoneEnv)

// 4) simulate_mortality
// - Input: ZoneValue &zone_lists, const ZoneEnv &env
// - For each fish in each of the three lists:
//     - compute pollution_factor = 1.0 - env.water_quality
//     - vuln = 1.0 - fish.tolerance
//     - mort_prob = NATURAL_MORTALITY + pollution_factor * POLLUTION_MORT_MULT * vuln
//     - if fish.age_months >= SENIOR_AGE_THRESHOLD: mort_prob *= SENIOR_MULTIPLIER
//     - draw random r in [0,1); if r < mort_prob => erase fish (iterator-safe removal)
// - Return statistics: number dead per age class (array<int,3>) or total dead

// 5) simulate_reproduction
// - Input: ZoneValue &zone_lists, const ZoneEnv &env
// - adult_count = size(zone_lists[1])
// - env_suit = env.water_quality
// - total_pop = sum sizes of all three lists
// - overcrowd_factor = min(1.0, OVERCROWDING_CAPACITY / total_pop)
// - repro_rate = BASE_REPRO_RATE * env_suit * overcrowd_factor
// - Return births (int)

// 6) age_and_transfer
// - Input: ZoneValue &zone_lists
// - For juveniles (index 0): increment ages; if age >= JUVENILE_AGE_THRESHOLD -> move to adults
// - For adults (index 1): increment ages; if age >= SENIOR_AGE_THRESHOLD -> move to seniors
// - For seniors (index 2): increment ages only

// 7) compute_stats
// - Input: LakeMap &lake_map
// - Output: aggregated data structure (map<string, tuple<int,int,int>> or print directly)
// - Used by print_snapshot and final summary

// 8) main_driver (orchestrator)
// - Input: filename for initial CSV
// - Steps:
//     - call load_initial_data(filename,...)
//     - print_snapshot(0,...)
//     - for month = 1 .. TOTAL_MONTHS:
//         - for each zone: update_zone_environment(env, month)
//         - for each zone: simulate_mortality(...), births = simulate_reproduction(...), age_and_transfer(...)
//         - if month % SNAPSHOT_INTERVAL == 0: print_snapshot(month,...)
//     - print final summary and optionally write CSV logs

// 9) Utility helpers (small functions)
// - int age_bucket(int age_months) -> returns 0/1/2
// - string generate_unique_id(...) -> create unique id for newborns
// - double rand_uniform() -> uniform double in [0,1)
// - void ensure_zone_exists(lake_map, env_map, const string &zone) -> helper to initialize structures
// - File parsing helpers and validation helpers


int main(int argc, char** argv) {
    // In real code: parse command line args for filename and options (e.g., seed, months)
    // Example: string filename = "clownfish_initial.csv";
    // return main_driver(filename);
  string filename = "clownfish_initial.csv";
}