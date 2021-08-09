#ifndef FTPSERVER_H
#define FTPSERVER_H

/**
 * VUT FIT IPK PROJECT 
 * Simple file transfer protocol client
 *
 * @file Header file
 * @author Daniel Andraško <xandra05@stud.fit.vutbr.cz>
 * @date 16.04.2021
 */

using namespace std;

#include <arpa/inet.h>
#include <algorithm>

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <cstdio>

#include <dirent.h>

#include <errno.h>
#include <experimental/filesystem>

#include <fstream> 

#include <ifaddrs.h>
#include <iostream>

#include <limits.h>

#include <netinet/in.h>

#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

namespace efs = experimental::filesystem;

// Program arguments
char* interface = (char*) "";
int port = 115;
string path = "";


// Userfull variables
bool logged_in = false; 
string user_name = "";
string working_dir;
string rename_file;
bool send_tobe = false;


class Command
{
    public:
        string name;
        string argument1;
        string argument2;
    
    Command(string n, string arg1, string arg2)
    {
        name = n;
        argument1 = arg1;
        argument2 = arg2;
    }    
};

/** 
 * @brief Parsing arguments.
 * 
 * @param argc Number of arguments.
 * @param argv Arguments string.
 * 
 * @return Void.
 */
void parse_arguments(int argc, char *argv[]);

/**
 * @brief Help.
 * 
 * @return Void. 
 */
void help();

/**
 * @brief Ends program with defined return code and message.
 * 
 * @param string Type of program termination, either success or failure.
 * @param string Printed message.
 *
 * @return Void. 
 */
void quit_program(bool program_ok, const char* message);

/**
 * @brief Returns current working directory.
 *
 * @return String directory. 
 */
string get_working_dir();


/**
 * @brief SFTP server starts listening
 *
 * @return Void.
 */
void start_listening();

/**
 * @brief Parsing message from client to get command
 *
 * @param char buffer[1024] contains message from server
 * 
 * @return Command.
 */
Command get_command(char buffer[1024]);

/**
 * @brief Defines which response should be send from server
 * 
 * @param Command command which is command from client
 *
 * @return String.
 */
string get_response(Command command);

/**
 * @brief Checks if username exists in database
 *
 * @return Bool.
 */
bool check_username();

/**
 * @brief Checks if password matches the username in database
 *
 * @return Bool.
 */
bool check_password(string password);



#endif
