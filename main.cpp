#include "helpers.h"

void runSerial(const char* stanicePath, const char* mereniPath) {
    //creating maps for preprocessed data
    std::unordered_map<int, std::vector<Measurement>> stations;
    std::unordered_map<int, StationLocation> stations_location;

    //loading the file
    loadCsv(stanicePath, stations);

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

    //loading locations of stations
    stations_location = loadStationsCSV(mereniPath);

    //drawing the values into svg
    generateMonthSVGs(stations_location, processed_stations, global_min, global_max);
}

void runParallel(const char* stanicePath, const char* mereniPath) {

}

int main(int argc, const char* argv[]) {

    if (argc < 4) {
        std::cout << "Malo argumentu! \n" << "Zadejte ve formatu program stanice.csv mereni.csv --serial/parallel" << std::endl;
    }

    if (argv[3] == "--serial")
    {
        runSerial(argv[1], argv[2]);
    }
    else if (argv[3] == "--parallel")
    {
        runParallel(argv[1], argv[2]);
    }
    else
    {
        std::cout << "wrong parameter! Use --parallel or --serial" << std::endl;
        return 1;
    }

    

	return 0;
}