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
    while (getline(fin, line)) {
    if (line.empty()) continue;
    // Peek first token before parsing; header expected like: zone,name,age_months,health,tolerance,sex
    string firstToken;
    {
        string tmp = line;
        stringstream ss(tmp);
        if (!getline(ss, firstToken, ',')) firstToken = "";
    }
    // Case-insensitive check for "zone" or "Zone"
    string firstLower = firstToken;
    transform(firstLower.begin(), firstLower.end(), firstLower.begin(), ::tolower);
    if (firstLower == "zone" || firstLower == "zone ") {
        // this is a header line: skip it and continue with next lines
    } else {
        // not a header: process this line as data
        stringstream ss(line);
        string zone,name,age_s,health_s,tol_s,sex_s;
        if(!getline(ss,zone,',')) continue;
        if(!getline(ss,name,',')) continue;
        if(!getline(ss,age_s,',')) continue;
        if(!getline(ss,health_s,',')) continue;
        if(!getline(ss,tol_s,',')) continue;
        if(!getline(ss,sex_s,',')) sex_s = "M";
        // parse values and push into maps (same code as before)
        int age = stoi(age_s);
        double health = stod(health_s);
        double tol = stod(tol_s);
        char sex = sex_s.empty() ? 'M' : sex_s[0];
        Clownfish cf(name, age, health, tol, sex);
        if(lake_map.find(zone)==lake_map.end()){
            lake_map[zone] = ZoneValue{};
            env_map[zone] = ZoneEnv{};
        }
        int idx = age_bucket(age);
        lake_map[zone][idx].push_back(cf);
        ++lines;
    }
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
        << setw(8) << "Youngs" 
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
    double pollution_factor = 1.0 - env.water_quality;
    const double SENIOR_MULT = 1.5;
    for(int idx=0; idx<3; ++idx){
        for(auto it = zv[idx].begin(); it != zv[idx].end(); ){
             Clownfish &f = *it;
            double vuln = 1.0 - f.tolerance;// higher tolerance = lower vulnerability
            double mort = NATURAL_MORTALITY + pollution_factor * POLLUTION_MORT_MULT * vuln;
            if(f.age_months >= SENIOR_AGE_THRESHOLD) mort *= SENIOR_MULT;
            double r = uniform01();
            if(r < mort){// fish dies
                it = zv[idx].erase(it);
                dead[idx]++;
            } else {
              ++it;
            }
        }
    }
    return dead;
}


// 5) simulate_reproduction
int simulate_reproduction(ZoneValue &zv, const ZoneEnv &env) {
    int adult_count = (int)zv[1].size();

    if(adult_count == 0) return 0;

    double env_suit = env.water_quality;
    int total = (int)(zv[0].size() + zv[1].size() + zv[2].size());
    double overcrowd = max(0.0, (double)(total - OVERCROWDING_CAPACITY) / OVERCROWDING_CAPACITY);
    double expected_births = BASE_REPRO_RATE * adult_count * env_suit * overcrowd;
    int births = (int)floor(expected_births + uniform01());
    static int birth_serial = 10000;

    for (int i=0;i<births;++i){
        string id = make_id(birth_serial++);
        int age = 0;
        double health = 0.7 + 0.2 * uniform01();
        double tol = 0.2 + 0.6 * uniform01();
        char sex = (uniform01() < 0.5) ? 'M' : 'F';
        Clownfish baby(id, age, health, tol, sex);
        zv[0].push_back(baby);
    }
    return births;
}

// 6) age_and_transfer

void age_and_transfer(ZoneValue &zv){
    for(auto it = zv[0].begin(); it != zv[0].end(); ){
        it->age_months += 1;
        if(it->age_months >= JUVENILE_AGE_THRESHOLD){
            zv[1].splice(zv[1].end(), zv[0], it++);
        } else {
            ++it;
        }
    }
    for(auto it = zv[1].begin(); it != zv[1].end(); ){
        it->age_months += 1;
        if(it->age_months >= SENIOR_AGE_THRESHOLD){
            zv[2].splice(zv[2].end(), zv[1], it++);
        } else {
            ++it;
        }
    }
    for(auto &f : zv[2]){
        f.age_months += 1;
    }
}

// 7) compute_stats
map<string, tuple<int, int, double>> compute_stats(const LakeMap &lake_map) {
    map<string, tuple<int, int, double>> stats;
    for(auto &p : lake_map){
        const ZoneValue &zv = p.second;
        int j=(int)zv[0].size(), 
        a=(int)zv[1].size(), s=(int)zv[2].size();
        stats[p.first] = make_tuple(j, a, s);
    }
    return stats;
}

// 8) main_driver (orchestrator)
int main_driver(const string &filename) {
  LakeMap lake_map;
  EnvMap env_map;

    load_initial_data(filename, lake_map, env_map);
    print_snapshot(0, lake_map, env_map);

  for(int month=1; month<=TOTAL_MONTHS; ++month){
      for(auto &ep : env_map){
          update_zone_environment(ep.second, month);
      }
      for(auto &zp : lake_map){
          ZoneValue &zv = zp.second;
          ZoneEnv &env = env_map[zp.first];
          auto dead = simulate_mortality(zv, env);
          int births = simulate_reproduction(zv, env);
          age_and_transfer(zv);

      }
      
      if(month % SNAPSHOT_INTERVAL == 0){
          print_snapshot(month, lake_map, env_map);
      }
  }
  auto stats = compute_stats(lake_map);
    cout << "\nFINAL SUMMARY (alpha)\n";
    for(auto &p : stats){
        int j,a,s; tie(j,a,s) = p.second;
        cout << p.first << ": J="<<j<<" A="<<a<<" S="<<s<<"\n";
    }
    return 0;
  
}

int main(int argc, char** argv) {
  string filename = "clownfish_initial.csv";
  if(argc >= 2){
    filename = argv[1];
  }
  cout << "LakePulsing ALPHA - starting with file: " << TOTAL_MONTHS << "\n";
  int rc = main_driver(filename);
  cout << "LakePulsing ALPHA - finished.\n";
  return rc;
} 