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

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int port = 115;
const char* ip = "";

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
 * @param string Printed message
 *
 * @return Void. 
 */
void quit_program(bool program_ok, const char* message);

void start_connection();


#endif