#include "types.h"

//will be used later to determine the risk factor
string severity_to_string(int level) {
    switch (level) {
        case 0:
            return "No risk";
            break;
        case 1:
            return "Low risk";
            break;
        case 2:
            return "Medium risk";
            break;
        case 3:
            return "High risk";
            break;
        case 4:
            return "Collision imminent";
            break;
        default:
            return "Unknown";
    }
}

//reading all the TLE data for each satellite
vector<TLE> parseTLEfile(const string &filename){
    vector<TLE> tles;
    ifstream file(filename);

    if(!file.is_open()){
        cout<<"ERROR COULD NOT OPEN"<<endl;
        return tles;
    }
    string line;
    TLE current_tle{}; // value-initialize; name empty, line1/line2 indeterminate, we'll set them
    // initialize C-style lines as empty strings
    current_tle.line1[0] = '\0';
    current_tle.line2[0] = '\0';

    // small helper to copy std::string into fixed C buffer with truncation and null-termination
    auto copy_to_cbuf = [](const string &src, char *dst, size_t cap){
        size_t i = 0;
        while (i < src.size() && (i + 1) < cap){
            dst[i] = src[i];
            ++i;
        }
        dst[i] = '\0';
    };

    while(getline(file,line)){
        // remove trailing \r if present (Windows line endings)
        if(!line.empty() && line.back()=='\r'){
            line.pop_back();
        }

        // skip empty lines
        if(line.empty()){continue;}

        // name line (does not start with '1' or '2')
        if(line[0]!='1' && line[0]!='2'){
            // if previous TLE is complete, store it
            if(!current_tle.name.empty() && current_tle.line1[0] != '\0' && current_tle.line2[0] != '\0'){
                tles.push_back(current_tle);
            }

            // reset for new TLE
            current_tle = TLE{};
            current_tle.name = line; // set new satellite name
            current_tle.line1[0] = '\0';
            current_tle.line2[0] = '\0';
        }
        else if(line[0]=='1'){
            copy_to_cbuf(line, current_tle.line1, sizeof(current_tle.line1));
        }
        else{ // line[0] == '2'
            copy_to_cbuf(line, current_tle.line2, sizeof(current_tle.line2));
        }

    }
    // push the final TLE only if complete
    if(!current_tle.name.empty() && current_tle.line1[0] != '\0' && current_tle.line2[0] != '\0'){
        tles.push_back(current_tle);
    }
    file.close();

    return tles;
}