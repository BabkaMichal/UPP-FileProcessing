#include <iostream>
#include <unordered_map>
#include <vector>
#include <set>
#include <cstdint>
#include <fstream>
#include <string>

struct Measurement
{
	int station_id;
	int ordinal;
	int16_t year;
	int8_t month;
	int8_t day;
	float value;
};

const double AVERAGE_FILTER = 100;
const int YEARS_IN_ROW_FILTER = 5;

void loadCsv(const std::string& filename, std::unordered_map<int, std::vector<Measurement>>& stations) {
	//opening the file for reading
	std::ifstream file(filename);

	//sanity check
	if (!file.is_open()) {
		std::cerr << "Error, cant open file " << filename << "!" << std::endl;
		return;
	}

	//variables
	std::string line;

	//skiping header
	std::getline(file, line);

    //reading until end of file
    while (std::getline(file, line)) {
        //skiping empty lines
        if (line.empty()) continue;

        //variables
        Measurement m;
        size_t pos = 0;
        size_t next_pos = 0;

        //station_id
        next_pos = line.find(';', pos);
        m.station_id = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //ordinal
        next_pos = line.find(';', pos);
        m.ordinal = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //year
        next_pos = line.find(';', pos);
        m.year = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //month
        next_pos = line.find(';', pos);
        m.month = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //day
        next_pos = line.find(';', pos);
        m.day = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //value 
        m.value = std::stof(line.substr(pos));

        //adding the struct into the map
        stations[m.station_id].push_back(m);
    }

    std::cout << "File loaded! " << std::endl;
}

void filterStations(std::unordered_map<int, std::vector<Measurement>>& stations) {
    int original_count = stations.size();

    //iterating through map
    for (auto it = stations.begin(); it != stations.end(); ) {
        const auto& measurements = it->second;

        //getting unique years due to set logic
        std::set<int> unique_years;
        for (const auto& m : measurements) {
            unique_years.insert(m.year);
        }

        //sanity check - defensive
        if (unique_years.empty()) {
            it = stations.erase(it);
            continue;
        }

        //calculating the average per year with all measurements / amount of years
        double avg_per_year = static_cast<double>(measurements.size()) / unique_years.size();

        //remove if less than 100
        if (avg_per_year < AVERAGE_FILTER) {
            it = stations.erase(it);
            continue;
        }

        bool has_n_consecutive = false;

        //if we have more than years in row we need, try to find if consequential
        if (unique_years.size() >= YEARS_IN_ROW_FILTER) {
            int streak = 1;
            auto year_it = unique_years.begin();
            int prev_year = *year_it;
            ++year_it;

            //previous = current + 1, pass
            for (; year_it != unique_years.end(); ++year_it) {
                if (*year_it == prev_year + 1) {
                    streak++;
                    if (streak >= YEARS_IN_ROW_FILTER) {
                        has_n_consecutive = true;
                        break;
                    }
                }
                //reset streak
                else {
                    streak = 1;
                }
                prev_year = *year_it;
            }
        }

        //if failed:remove, else ++iterator
        if (!has_n_consecutive) {
            it = stations.erase(it);
        }
        else {
            ++it;
        }
    }
}

int main() {
    //creating the map
    std::unordered_map<int, std::vector<Measurement>> stations;

    //loading the file
    loadCsv("mereni.csv", stations);

    //filtering the data
    filterStations(stations); 

	return 0;
}