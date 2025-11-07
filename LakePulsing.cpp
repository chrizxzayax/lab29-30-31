//LakePulsing
//skeleton for the Clownfish Population Dynamics Simulator ("LakePulse").
// This file contains only includes, type definitions, function signatures, and detailed comments
// describing what each function will implement. Use this wireframe as a guide to implement
// the actual simulation functions in separate milestones.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <array>
#include <list>
#include <vector>
#include <random>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>

using namespace std;


  const int MONTHS_PER_YEAR = 12;
  const int TOTAL_YEARS = 12;
  const int TOTAL_MONTHS = 144;
  const int JUVENILE_AGE_THRESHOLD = 6;
  const int SENIOR_AGE_THRESHOLD = 60;
  const double BASE_REPRO_RATE = 0.25;

  const double NATURAL_MORTALITY = 0.005;
  const double POLLUTION_MORT_MULT = 0.6;
  const int OVERCROWDING_CAPACITY = 50;
  const int SNAPSHOT_INTERVAL = 12;


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

// RNG helper function prototype
static std::mt19937 rng_engine((unsigned)time(nullptr)); // Seed with current time or fixed seed for testing
inline double uniform01() { return std::uniform_real_distribution<double>(0.0, 1.0)(rng_engine); }// I arbirtrarily chose 0.0 to 1.0 as the ranges
inline int randint(int a, int b){ return std::uniform_int_distribution<int>(a, b)(rng_engine); }

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
  ifstream fin(filename);
  if(fin){
    string line;
    int lines = 0;
    while(getline(fin, line)){
      if(line.empty()) continue;
      ++lines;
      stringstream ss(line);
      string zone,name,age_s,health_s,tol_s,sex_s;
      if(!getline(ss, zone, ',')) continue;
      if(!getline(ss, name, ',')) continue;
      if(!getline(ss, age_s, ',')) continue;
      if(!getline(ss, health_s, ',')) continue;
      if(!getline(ss, tol_s, ',')) continue;
      if(!getline(ss, sex_s, ',')) continue;
      {
         sex_s = "M";
      }
      int age = stoi(age_s);
      double health = stod(health_s);
      double tol = stod(tol_s);
      char sex = sex_s.empty() ? 'M' : sex_s[0];
      Clownfish cf(name, age, health, tol, sex);
      if(lake_map.find(zone) == lake_map.end())
      {
        lake_map[zone] = ZoneValue();
        env_map[zone] = ZoneEnv();
      }
      int bucket = age_bucket(age);
      lake_map[zone][bucket].push_back(cf);
    }
    fin.close();
    if(lines >= 100)
    return true;
  }
  vector<string> zones = {"Inlet", "ReefNorth", "ReefSouth", "DeepPool", "Outlet"};
  int serial = 1;
  for(size_t z=0; z<zones.size(); ++z){
    lake_map[zones[z]] = ZoneValue{};
    env_map[zones[z]] = ZoneEnv{};
    env_map[zones[z]].pollution_rate = 0.005 + 0.01 * static_cast<double>(z);
    
  }
      int total_needed = 120;
      for(int i=0;i<total_needed;++i){
          string zone = zones[randint(0,(int)zones.size()-1)];
          string id = make_id(serial++);
          int age = randint(0,72);
          double health = 0.7 + 0.3 * uniform01();
          double tol = 0.2 + 0.8 * uniform01();
          char sex = (uniform01() < 0.5) ? 'M' : 'F';
          Clownfish cf(id, age, health, tol, sex);
          int idx = age_bucket(age);
          lake_map[zone][idx].push_back(cf);
      }
  return true;
}

void print_snapshot(int month, const LakeMap &lake_map, const EnvMap &env_map){
    int year = (month) / MONTHS_PER_YEAR;
    cout << "///////////////////////////////////////////////////\n";
    cout << "Snapshot - Month: " << month << " Year: " << year << "\n";
    cout << left << setw(14) << "Zone" 
        << setw(16) << "Water Quality" 
        << setw(8) << "Juveniles" 
        << setw(8) << "Adults" 
        << setw(8) << "Seniors" 
        << setw(8) << "Total" << "\n";
    for(auto &p : lake_map){
          const string &zone = p.first;
          const ZoneValue &zv = p.second;
          int j = (int)zv[0].size();
          int a = (int)zv[1].size();
          int s = (int)zv[2].size();
          int tot = j+a+s;
          double wq = 0.0;
          auto itenv = env_map.find(zone);
          if(itenv!=env_map.end()) wq = itenv->second.water_quality;
          cout << left << setw(14) << zone << setw(8) << fixed << setprecision(2) << wq 
              << setw(8) << tot << setw(8) << j << setw(8) << a << setw(8) << s << "\n";
    }
    cout << "---------------------------------------------------\n";
}

// 3) update_zone_environment
void update_zone_environment(ZoneEnv &env, int month) {
    const double ALPHA = 0.15;
    const double BETA = 0.04;
    const double SEASONAL_PULSE = 0.02;

    env.water_quality -= ALPHA * env.pollution_rate;
    env.water_quality += BETA * (1.0 - env.water_quality);
    if(month % 12 == 3) env.water_quality -= SEASONAL_PULSE;
    env.water_quality = max(0.0, min(1.0, env.water_quality));
}

// 4) simulate_mortality
array<int, 3> simulate_mortality(ZoneValue &zv, const ZoneEnv &env) {
    array<int, 3> dead = {0, 0, 0};


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
int main_driver(const string &filename) {
  LakeMap lake_map;
  EnvMap env_map;

    load_initial_data(filename, lake_map, env_map);
    print_snapshot(0, lake_map, env_map);

  if(!load_initial_data(filename, lake_map, env_map)){
    cerr << "Error loading initial data from " << filename << endl;
    return 1;
  }
  return 0;
}

// 9) Utility helpers (small functions)
// - int age_bucket(int age_months) -> returns 0/1/2
// - string generate_unique_id(...) -> create unique id for newborns
// - double rand_uniform() -> uniform double in [0,1)
// - void ensure_zone_exists(lake_map, env_map, const string &zone) -> helper to initialize structures
// - File parsing helpers and validation helpers


int main(int argc, char** argv) {
  string filename = "clownfish_initial.csv";
  if(argc > 1){
    filename = argv[1];
  }
  int rc = main_driver(filename);
  cout << "LakePulsing ALPHA - finished.\n";
  return rc;
} 