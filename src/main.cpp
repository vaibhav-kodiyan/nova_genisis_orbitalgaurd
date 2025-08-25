#include "project_includes.h"
#include "types.h"
#include "SGP4/SGP4/SGP4.h"

// simple helper to compute Euclidean distance (km)

static double dist3(const double a[3], const double b[3]){
    double dx = a[0]-b[0];
    double dy = a[1]-b[1];
    double dz = a[2]-b[2];
    return sqrt(dx*dx + dy*dy + dz*dz);
}

// helper to write coordinates JSON with timestamp
static void writeCoordinatesJSON(const vector<string>& names, const vector<array<double,3>>& positions, const vector<array<double,3>>& velocities, double currentTime){
    ofstream jf("tests/coordinates.json");
    jf << "{\n";
    jf << "  \"timestamp_minutes\": " << fixed << setprecision(6) << currentTime << ",\n";
    jf << "  \"satellites\": [\n";
    for(size_t i = 0; i < names.size(); ++i){
        jf << "    {\n";
        jf << "      \"name\": \"" << names[i] << "\",\n";
        jf << fixed << setprecision(6);
        jf << "      \"position_km\": [" << positions[i][0] << ", " << positions[i][1] << ", " << positions[i][2] << "],\n";
        jf << "      \"velocity_km_s\": [" << velocities[i][0] << ", " << velocities[i][1] << ", " << velocities[i][2] << "]\n";
        jf << "    }" << (i+1<names.size()? ",\n" : "\n");
    }
    jf << "  ]\n";
    jf << "}\n";
}

// helper to write conjunctions JSON with timestamp
static void writeConjunctionsJSON(const vector<string>& names, const vector<array<double,3>>& positions, double currentTime){
    struct PairOut { int a; int b; double d; };
    vector<PairOut> pairs;
    for(size_t i = 0; i < positions.size(); ++i){
        for(size_t j = i+1; j < positions.size(); ++j){
            const double ra[3] = { positions[i][0], positions[i][1], positions[i][2] };
            const double rb[3] = { positions[j][0], positions[j][1], positions[j][2] };
            double d = dist3(ra, rb);
            if(d < THRESHOLD_DISTANCE){
                pairs.push_back({(int)i, (int)j, d});
            }
        }
    }

    ofstream jf("tests/conjunctions.json");
    jf << "{\n";
    jf << "  \"timestamp_minutes\": " << fixed << setprecision(6) << currentTime << ",\n";
    jf << "  \"conjunction_pairs\": [\n";
    for(size_t k = 0; k < pairs.size(); ++k){
        auto p = pairs[k];
        jf << "    {\n";
        jf << "      \"sat1\": { \"name\": \"" << names[p.a] << "\", \"position_km\": ["
           << fixed << setprecision(6) << positions[p.a][0] << ", " << positions[p.a][1] << ", " << positions[p.a][2]
           << "] },\n";
        jf << "      \"sat2\": { \"name\": \"" << names[p.b] << "\", \"position_km\": ["
           << fixed << setprecision(6) << positions[p.b][0] << ", " << positions[p.b][1] << ", " << positions[p.b][2]
           << "] },\n";
        jf << "      \"distance_km\": " << fixed << setprecision(6) << p.d << "\n";
        jf << "    }" << (k+1<pairs.size()? ",\n" : "\n");
    }
    jf << "  ]\n";
    jf << "}\n";
}

int main(){
    // 1) Load TLEs
    vector<TLE> tles = parseTLEfile("data/sample3.tle");
    if(tles.empty()){
        cout << "No TLEs found." << endl;
        return 0;
    }

    // 2) Build satrecs for Vallado SGP4
    vector<elsetrec> satrecs;
    vector<string> names;
    satrecs.reserve(tles.size());
    names.reserve(tles.size());

    for(auto &t : tles){
        elsetrec rec{};
        double startmfe=0.0, stopmfe=0.0, deltamin=0.0;
        // twoline2rv expects mutable C strings
        char l1[130]; char l2[130];
        size_t i=0; while(t.line1[i] != '\0' && i+1<sizeof(l1)){ l1[i]=t.line1[i]; ++i; } l1[i]='\0';
        i=0; while(t.line2[i] != '\0' && i+1<sizeof(l2)){ l2[i]=t.line2[i]; ++i; } l2[i]='\0';
        SGP4Funcs::twoline2rv(l1, l2, 'v', 'e', 'i', wgs84, startmfe, stopmfe, deltamin, rec);
        satrecs.push_back(rec);
        names.push_back(t.name);
    }

    // 3) Simulation parameters
    const double dt = 180.0; // 3 hours step (180 minutes)
    double currentTime = 0.0; // start from TLE epoch
    const double maxMinutes = 24.0 * 60.0; // run up to 24 hours
    
    cout << "Starting 24-hour simulation (3-hour steps)..." << endl;
    cout << "JSON files will be updated for 3D frontend." << endl;

    // 4) Propagation loop - runs until 24 hours of simulated time
    while(currentTime <= maxMinutes){
        vector<array<double,3>> currentR(tles.size());
        vector<array<double,3>> currentV(tles.size());

        // Propagate all satellites to current time (stateful - each step builds on previous)
        for(size_t s = 0; s < satrecs.size(); ++s){
            double r[3] = {0,0,0};
            double v[3] = {0,0,0};
            SGP4Funcs::sgp4(satrecs[s], currentTime, r, v);
            currentR[s] = {r[0], r[1], r[2]};
            currentV[s] = {v[0], v[1], v[2]};
        }

        // Write JSON files with timestamp for frontend synchronization
        writeCoordinatesJSON(names, currentR, currentV, currentTime);
        writeConjunctionsJSON(names, currentR, currentTime);
        
        cout << "t=" << fixed << setprecision(1) << currentTime << " min - JSON updated for 3D simulation" << endl;
        
        // Advance time for next propagation step
        currentTime += dt;
        
        // Small delay to prevent overwhelming the filesystem (adjust as needed)
        // For real-time simulation, you might want to sync this with actual time
        // or remove the delay entirely for maximum speed
        // usleep(100000); // 0.1 second delay - uncomment if needed
    }
    
    cout << "Simulation complete at t=" << fixed << setprecision(1) << maxMinutes << " min - 24 hours reached." << endl;
    return 0;
}
