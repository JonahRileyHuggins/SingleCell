/**
 * @file: ArgParsing.cpp
 * 
 * @authors  Jonah R. Huggins
 * @date 30-05-2025
 * 
 * @brief Creates class for main function to interpret Command line arguments
 */
//===========================Library Import=================================//
//Std Libaries
#include <any>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <optional>
#include <unordered_map>

//Internal Libraries
#include "singlecell/ArgParsing.h"

//=============================Class Details================================//
ArgParsing::ArgParsing(int argc, char* argv[]) {
    // Populate the cli_map from args
    this->cli_map = cliToMap(argc, argv);

    // Help flag
    if (cli_map.count("-h") || cli_map.count("--help")) {
        printUsage();
        return;  // Optional: early return after help
    }

    // Parse numeric arguments
    if (cli_map.count("--start")) {
        std::string v = std::any_cast<std::string>(cli_map["--start"]);
        cli_map["--start"] = parseDoubleArgs("--start", v, 0.0, "--start");
    }

    if (cli_map.count("--stop")) {
        std::string v = std::any_cast<std::string>(cli_map["--stop"]);
        cli_map["--stop"] = parseDoubleArgs("--stop", v, 60.0, "--stop");
    }

    if (cli_map.count("--step")) {
        std::string v = std::any_cast<std::string>(cli_map["--step"]);
        cli_map["--step"] = parseDoubleArgs("--step", v, 1.0, "--step");
    }

    // Parse key-value pair updates (e.g., --update "x=0.1")
    if (cli_map.count("-u") || cli_map.count("--update")) {
        std::string kv_string = std::any_cast<std::string>(cli_map["--update"]);
        parseKeyValuePairs(kv_string); 
    }
}

std::unordered_map<std::string, std::any> ArgParsing::cliToMap(
    int argc, 
    char* argv[]
) {

    // empty map for storing CLI args
    std::unordered_map<std::string, std::any> args;

    for (int i = 0; i < argc; i++) {

        if (argv[i][0] == '-') { //starting character for argument flag

            //flag stored as a key
            const std::string key = argv[i];

            std::string value = "";

            // i.e. if the flag is not boolean, like --Verbose, store value
            if (i+1 < argc && argv[i+1][0] != '-') { 

                value = std::string(argv[i+1]);
                i++;
            }

            args[key] = value;
        }

    }

    return args;

}

void ArgParsing::printUsage() {

    std::cout << "SingleCell: A Simulatable Model of Stochastic Single Cell Dyanmics\n"
            "\n"
            "Example usage:\n"
            "    ./SingleCell --<option> <opt_parameter>\n";
}

double ArgParsing::parseDoubleArgs(
    const std::string& key, 
    const std::any value, 
    double def, 
    const char* arg_name
) {
    char* end = nullptr; // make end point

    std::string v_init = std::any_cast<std::string>(value);

    double v = std::strtod(v_init.c_str(), &end); 

    return v;
}

void ArgParsing::parseKeyValuePairs(
    std::string arg
) {

    size_t pos = arg.find('=');
    
    if (pos == std::string::npos) {

        std::cerr << "Bad modifier format, must specify '<SBMLEntity>=<Double>" << "\n";
        std::exit(EXIT_FAILURE);

    }

    std::any new_value;

    std::string key = arg.substr(0, pos);
    std::string temp_value = arg.substr(1, pos);

    //quick attempt to convert numerics to proper type before passing
    try {
        char* end = nullptr;

        new_value = std::strtod(temp_value.c_str(), &end);
    }
    catch(...) {
        new_value = temp_value;
    }


    this->cli_map[key] = new_value;
}