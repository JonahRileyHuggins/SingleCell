/**
 * @file: ArgParsing.h
 * 
 * @authors  Jonah R. Huggins
 * @date 30-05-2025
 * 
 * @brief Creates class for main function to interpret Command line arguments
 */
//========================header file definition============================//
#pragma once

#ifndef ARGPARSING_h
#define ARGPARSING_h

//===========================Library Import=================================//
//Std Libaries
#include <any>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>

// Internal libraries

//==========================Class Declaration===============================//
class ArgParsing {
    public:
    // -------------------------Methods-----------------------------------//
        ArgParsing( //Constructor. ctor
            int argc, 
            char* argv[]
        );

        ~ArgParsing();

        //---------------------------Members----------------------------------//
        std::unordered_map<std::string, std::any> cli_map;


        private:
        // -------------------------Methods-----------------------------------//
        /**
         * @brief converts key-value pair args from the command line into a map
         * for reference in main
         * 
         * @param argc OS-defined parameter for CLI-argument count
         * @param argv OS-defined parameter for CLI-arguements as a vector of char*,
         *  representing individual CLI args.
         * 
         * @returns None stores map in class-member cli_map
         */
        std::unordered_map<std::string, std::any> cliToMap(
            int argc,
            char* argv[]
        );

        //---------------------------Members----------------------------------//


        protected:
        // -------------------------Methods-----------------------------------//
        /**
         * @brief Examine a string thats a path and return as kvp
         * 
         * @param argc
         * @param argv
         * @param index
         * @param def
         * @param arg_name 
         * 
         * @returns None updates cli_map class-member
         */
        std::string parsePathArgs(
            int argc, 
            char* argv[],
            int index, 
            std::string def,
            const char* arg_name
        );

        /**
         * @brief Examine a float-64 double thats a timepoint parameter and return as kvp
         * 
         * @param argc
         * @param argv
         * @param index
         * @param def
         * @param arg_name 
         * 
         * @returns None updates cli_map class-member
         */
        double parseDoubleArgs(
            const std::string& key, 
            const std::any value, 
            double def, 
            const char* arg_name
        );

        /**
         * @brief basic print help statement, used for informing users
         * 
         * @param None
         * 
         * @returns None
         * 
         */
        void printUsage();
        
        /**
         * @brief Converts key-value pair options, separated by '=', into dictionary entries
         * 
         * @param args CLI argument separated by parenthesis
         * 
         * @returns None Updates class-member cli_map
         */
        void parseKeyValuePairs(
            std::string arg
        );

        //---------------------------Members----------------------------------//
        

};

#endif // ARGPARSING_h
