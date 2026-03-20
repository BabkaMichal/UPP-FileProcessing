#include <sstream>
#include "helpers.h"

//loading stations and their LOCATION
std::unordered_map<int, StationLocation> loadStationsCSV(const std::string& filename) {
    //variables
    std::unordered_map<int, StationLocation> locations;
    std::ifstream file(filename);
    std::string line;

    //sanity check
    if (!file.is_open()) {
        return locations;
    }

    //skipping header
    std::getline(file, line);

    //reading the rest
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        StationLocation loc;
        size_t pos = 0, next_pos = 0;

        next_pos = line.find(';', pos);
        loc.id = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        next_pos = line.find(';', pos);
        loc.name = line.substr(pos, next_pos - pos);
        pos = next_pos + 1;

        next_pos = line.find(';', pos);
        loc.latitude = std::stod(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        loc.longitude = std::stod(line.substr(pos));
        locations[loc.id] = loc;
    }
    return locations;
}

//Color interpolation
std::string getColorForTemperature(double temp, double global_min, double global_max) {
    double mid = (global_min + global_max) / 2.0;
    int r, g, b;

    if (temp <= mid) {
        double ratio = (temp - global_min) / (mid - global_min);
        r = static_cast<int>(ratio * 255);
        g = static_cast<int>(ratio * 255);
        b = static_cast<int>(255 - ratio * 255);
    }
    else {
        double ratio = (temp - mid) / (global_max - mid);
        r = 255;
        g = static_cast<int>(255 - ratio * 255);
        b = 0;
    }

    return "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
}

//generating maps
void generateMonthSVGs(const std::unordered_map<int, StationLocation>& locations, const std::unordered_map<int, StationStats>& stats, double global_min, double global_max) {
    std::ifstream base_map_file("czmap.svg");
    std::stringstream buffer;
    buffer << base_map_file.rdbuf();
    std::string base_svg = buffer.str();

    //finding the end tag
    size_t svg_end_pos = base_svg.rfind("</svg>");

    //erasing it or break
    if (svg_end_pos != std::string::npos) {
        base_svg.erase(svg_end_pos);
    }
    else {
        std::cerr << "Chyba: Podkladove SVG nema ukoncujici tag </svg>\n";
        return;
    }

    //months
    const std::string month_names[] = { "leden", "unor", "brezen", "duben", "kveten", "cerven", "cervenec", "srpen", "zari", "rijen", "listopad", "prosinec" };

    //generating map for each month
    for (int month = 1; month <= 12; ++month) {
        std::string out_filename = month_names[month - 1] + ".svg";
        std::ofstream out_file(out_filename);

        //writing the base map
        out_file << base_svg;

        //writing the new station points
        for (const auto& pair : stats) {
            int station_id = pair.first;

            //sanity checks
            if (locations.find(station_id) == locations.end()) continue;
            if (pair.second.months.find(month) == pair.second.months.end()) continue;

            const StationLocation& loc = locations.at(station_id);
            double avg_temp = pair.second.months.at(month).overall_average;

            //coordinates interpolation
            double cx = ((loc.longitude - LON_MIN) / (LON_MAX - LON_MIN)) * SVG_WIDTH;
            double cy = ((LAT_MAX - loc.latitude) / (LAT_MAX - LAT_MIN)) * SVG_HEIGHT;

            std::string color = getColorForTemperature(avg_temp, global_min, global_max);

            //drawing circle
            out_file << "<circle cx=\"" << cx << "\" cy=\"" << cy
                << "\" r=\"5\" fill=\"" << color << "\" "
                << "title=\"" << loc.name << " (" << avg_temp << " C)\" />\n";
        }

        //closing the svg
        out_file << "</svg>";
        out_file.close();
        std::cout << "Vygenerovano: " << out_filename << "\n";
    }
}