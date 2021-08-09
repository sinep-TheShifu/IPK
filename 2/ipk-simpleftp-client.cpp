/**
 * VUT FIT IPK PROJECT 
 * Simple file transfer protocol client
 *
 * @file SFTP client
 * @author Daniel Andraško <xandra05@stud.fit.vutbr.cz>
 * @date 16.04.2021
 */

#include "ipk-simpleftp-client.hpp"

int main(int argc, char *argv[])
{

    // Parse arguments
    parse_arguments(argc, argv);

    start_connection();

    return EXIT_SUCCESS;
}

void parse_arguments(int argc, char *argv[])
{
    bool ip_defined = false;
     
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0) help();
        else if(strcmp(argv[i], "-p") == 0)
        {
            try { port = stoi(argv[i + 1]); }               
            catch (const invalid_argument & e) { quit_program(false, e.what()); }
            catch (const out_of_range & e) { quit_program(false, e.what()); }          
        }
        else if(strcmp(argv[i], "-h") == 0)
        {
            ip = argv[i + 1];            
            ip_defined = true;
        }
    }   

    if (ip_defined == false) quit_program(false, "IPv4 or IPv6 was not defined.");      
}

void help()
{
    cout << "Help" << endl;
    quit_program(true, "Help called");
}

void quit_program(bool program_ok, const char* message)
{
    if (program_ok == false) 
    {
        perror(message);
        exit(EXIT_FAILURE);        
    }
    else
    {
        printf("%s\n", message);
        exit(EXIT_SUCCESS);
    } 
}

void start_connection()
{
    int sock = 0, valread, optval = 1;
    struct sockaddr_in serv_addr;
    string message;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        quit_program(false, "Creaing socket error.");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        close(sock);
        quit_program(false, "Invalid or not supported address.");
    }
        
    if (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        close(sock);
        quit_program(false, "Connection failed.");
    }        

    cout << "-------------------------------\n"
            "SFTP client is connected\n"
            "-------------------------------\n\n";

    // Reading welcome message from server
    valread = read(sock, buffer, 1024);
    if (valread <= 0)        
    {
        close(sock);
        quit_program(false, "Reading data from server error.");
    }
        
    cout << buffer << "\r\n\r\n";


    cout << "*** Enter commands ***\n\n";
    while(getline(cin, message))
    {
        cout << endl;
        // Seding message to server
        if(send(sock, message.c_str(), strlen(message.c_str()), 0) == -1)
        {
            close(sock);
            quit_program(false, "Send error");
        }
        message.clear();

        // Reading response from server
        char buffer_r[1024] = {0};

        valread = read(sock, buffer_r, 1024);
        if (valread <= 0)        
        {
            close(sock);
            quit_program(false, "Connection closed.");
        }
            
        cout << "--- Server response ---\n" << buffer_r << "\n\n";
    }
    
    // Closing socket
    close(sock);    
}



















