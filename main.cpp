#include "helpers.h"

int main() {
    //creating map for preprocessed data
    std::unordered_map<int, std::vector<Measurement>> stations;

    //loading the file
    loadCsv("mereni.csv", stations);

    //filtering the data
    filterStations(stations); 

    //structs for data
    std::unordered_map<int, StationStats> processed_stations;
    double global_min = 0.0;
    double global_max = 0.0;

    //calculating averages for other calculations
    calculateAverages(stations, processed_stations, global_min, global_max);

    //detecting fluctuations
    detectFluctuations(processed_stations);



     
	return 0;
}