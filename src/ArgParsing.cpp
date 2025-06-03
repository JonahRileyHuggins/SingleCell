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
#include <sstream>
#include <iostream>
#include <optional>
#include <typeinfo>
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
        return;
    }

    // Parse key-value pair updates
    if (cli_map.count("-m") || cli_map.count("--modify")) {
        try {
            std::string kv_string = std::any_cast<std::string>(cli_map["--modify"]);
            // implement a method to restructure the input from '{}', and comma-separated
            parseDict(kv_string);
        } catch (const std::bad_any_cast&) {
            std::cerr << "Error: --modify must be followed by a string like 'x=1.0'\n";
            std::exit(EXIT_FAILURE);
        }
    }
}

std::unordered_map<std::string, std::any> ArgParsing::cliToMap(
    int argc, 
    char* argv[]
) {

    // Populate with defaults for simulator args
    std::unordered_map<std::string, std::any>args = setDefaults();

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


            try {

                if (key == "--start" || key == "--stop" || key == "--step") {

                    char* end = nullptr; // make end point, req' of strtod

                    double value_d = std::strtod(value.c_str(), &end); 
                    args[key] = value_d;

                } else {
                    args[key] = value;
                }

            } catch (...) {

                std::cout << "Key {" << key << "}:" << "value is of type: " << typeid(value).name();

                args[key] = value;
            }
        }
    }

    return args;

}

std::unordered_map<std::string, std::any> ArgParsing::setDefaults() {

    // empty map for storing Default values
    std::unordered_map<std::string, std::any> args_map;

    //defaults for current simulator:
    args_map["--start"] = 0.0;
    args_map["--stop"] = 60.0;
    args_map["--step"] = 1.0;
    args_map["--stochastic_model"] = std::string("../tests/Stochastic.sbml");
    args_map["--deterministic_model"] = std::string("../tests/Deterministic.sbml");

    return args_map;
}

void ArgParsing::printUsage() {

    std::cout << "SingleCell: A Simulatable Model of Stochastic Single Cell Dynamics\n"
            "\n"
            "Example usage:\n"
            "    ./SingleCell --<option> <opt_parameter>\n"
            "===================flags======================\n"
            "     --start <Double> {[Optional] Default:0.0}\n"
            "     --stop <Double> {[Optional] Default: 60.0}\n"
            "     --step <Double> {[Optional] Default:1.0}\n"
            "     --stochastic_model <std::string> {[Optional] Default:  ../tests/Stochastic.sbml}\n"
            "     --deterministic_model <std::string> {[Optional] Default:  ../tests/Deterministic.sbml}\n"
            "     --modify <SpeciesId || ParameterId || CompartmentId>=<Double> {[Optional]}\n";

            std::exit(EXIT_SUCCESS);
}

void ArgParsing::parseDict(
    std::string arg
) {

    size_t start = arg.find('{');
    size_t end = arg.find('}');
    
    if (start == std::string::npos || end == std::string::npos) {

        std::cerr << "Bad modifier format, must specify '{ }' to start perturbations list." << "\n";
        std::exit(EXIT_FAILURE);

    }


    // std::vector<int> comma_pos_list;


    // size_t pos = arg.find(',', start);

    // while (pos != std::string::npos && pos < end) {

    //     comma_pos_list.push_back(pos);

    //     pos = arg.find(',', pos+1);

    // }

    std::string content = arg.substr(start + 1, end - start - 1);

    std::stringstream ss(content);

    std::string kv_pair;

    while (std::getline(ss, kv_pair, ',')) {

        parseKeyValuePairs(kv_pair);

    }

}

void ArgParsing::parseKeyValuePairs(
    std::string arg
) {

    size_t pos = arg.find('=');
    
    if (pos == std::string::npos) {

        std::cerr << "Bad modifier format, must specify '<SBMLEntity>=<Double>" << "\n";
        std::exit(EXIT_FAILURE);

    }

    std::string key = arg.substr(0, pos);
    std::string temp_value = arg.substr(pos+1);

    char* end = nullptr;

    double new_value = std::strtod(temp_value.c_str(), &end);

    this->entity_map[key] = new_value;
}