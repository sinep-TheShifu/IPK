// TODO pri errore close(fd)
/**
 * VUT FIT IPK PROJECT 
 * Simple file transfer protocol server
 *
 * @file SFTP server
 * @author Daniel Andraško <xandra05@stud.fit.vutbr.cz>
 * @date 16.04.2021
 */

#include "ipk-simpleftp-server.hpp"

int main(int argc, char *argv[])
{

    // Parse arguments
    parse_arguments(argc, argv);

    // Starts SFTP server
    start_listening();

    return EXIT_SUCCESS;
}

void parse_arguments(int argc, char *argv[])
{
    bool path_defined = false;
     
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) help();
        else if(strcmp(argv[i], "-i") == 0) interface = argv[i + 1];
        else if(strcmp(argv[i], "-p") == 0)
        {
            try { port = stoi(argv[i + 1]); }               
            catch (const invalid_argument & e) { quit_program(false, e.what()); }
            catch (const out_of_range & e) { quit_program(false, e.what()); }          
        }
        else if(strcmp(argv[i], "-u") == 0)
        {
            path = argv[i + 1];
            path_defined = true;
        }
    }   

    if (path_defined == false) quit_program(false, "Path was not defined.");      
}

void help()
{
    cout << "Help" << endl;
    quit_program(true, "Help called");
}

void quit_program(bool program_ok, const char* message)
{
    if (!program_ok) 
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

string get_working_dir()
{
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != NULL)
        working_dir = cwd;
    else
        quit_program(false, "Error getting current directory");
    return cwd;
}

void start_listening()
{
    int fd, new_socket, valread = 0;
    int opt = 1;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    int addrlen = sizeof(address);
    unsigned int length = 1024;
    string response, message, msg_send;
    char buffer[1024] = {0};

    // Getting current working directory
    working_dir = get_working_dir();

    // Creating socket file descriptor
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        quit_program(false, "Couldn't create socket file descriptor");

    // Attach socket to the port
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    {
        close(fd);
        quit_program(false, "Couldn't attach socket to the port");
    }
        

    // Getting all interfaces and choosing the one to connect to
    // https://stackoverflow.com/questions/40980874/operation-not-permitted-server-setsockopt-error-for-so-bindtodevice/50067325
    // edited
    if (interface != "")
    {
        struct ifaddrs *ifa, *ifa_tmp;
        if (getifaddrs(&ifa) == -1) 
        {
            close(fd);
            quit_program(false, "Ifaddrs failed");
        }

        ifa_tmp = ifa;
        while (ifa_tmp) 
        {
            if (!strcmp(ifa_tmp->ifa_name, interface) && ifa_tmp->ifa_addr && (ifa_tmp->ifa_addr->sa_family == AF_INET)) 
            {
                cout << "addr = %s\n" << inet_ntoa(((struct sockaddr_in *)ifa_tmp->ifa_addr)->sin_addr) << endl;
                address.sin_addr.s_addr = ((struct sockaddr_in *)ifa_tmp->ifa_addr)->sin_addr.s_addr;
                break;
            }
            ifa_tmp = ifa_tmp->ifa_next;
        }
        freeifaddrs(ifa);

        if (address.sin_addr.s_addr == 0) 
        {
            close(fd);
            quit_program(false, "Interface not found");
        }   
    }
    else
    {   // Listening on any interface
        address.sin_addr.s_addr = INADDR_ANY;
    }
    
    address.sin_family = AF_INET;
    address.sin_port = htons( port );

    // Attach socket to the port
    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        quit_program(false, "Bind error");
    
    // Listening
    cout << "-------------------------------\n"
            "SFTP server starts listening\n"
            "-------------------------------\n\n";
    if (listen(fd, 3) < 0)
        quit_program(false, "Listen error");

    while(1)
    {
        char buffer_r[1024] = {0};
        // New client is connecting
        if (valread == 0)
        {
            // Deleting last user information
            user_name = "";
            logged_in = false;
            send_tobe = false;
            rename_file = "";

            // Accept
            if ((new_socket = accept(fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
            {
                close(fd);
                close(new_socket);
                quit_program(false, "Accept error");
            }
            cout << "--- NEW CLIENT IS CONNECTED ---" << endl;
                           
            // Welcome message from server
            const char* send_mess = "+MIT-XX SFTP Service";            
            if((send(new_socket , send_mess, strlen(send_mess) , 0)) == -1)
            {
                close(fd);
                close(new_socket);
                quit_program(false, "Sending data to client was unsuccessfull");     
            }
        }        

        // Reading data from client
        valread = read(new_socket, buffer_r, 1024);
        if(valread < 0)
        {
            close(fd);
            close(new_socket);
            quit_program(false, "Error reading from client");         
        }            
        cout << buffer_r << endl;

        // Getting resposne
        Command command = get_command(buffer_r);
        response = get_response(command);
        cout << response << "\n" << endl;

        // Sending response to client
        msg_send = string(buffer_r) + "\n" + response;
        if((send(new_socket , msg_send.c_str() , strlen(msg_send.c_str()) , 0)) == -1)
        {
            close(fd);
            close(new_socket);
            quit_program(false, "Sending data to client was unsuccessfull");     
        }            

        // DONE command, ends connection with client
        if (strcmp(response.c_str(), "+We are done\r\n") == 0)
        {
            string close_conn_mess = "+MIT-XX closing connection\r\n";
            if((send(new_socket , close_conn_mess.c_str() , strlen(close_conn_mess.c_str()) , 0)) == -1)
            {
                close(fd);
                close(new_socket);
                quit_program(false, "Sending data to client was unsuccessfull");     
            }             
            close(new_socket);
            valread = 0;
        }  
    }    

    // Closing sockets
    close(fd);
    close(new_socket);
}

Command get_command(char buffer[1024])
{
    string s = string(buffer);  // Message from client
    string delimiter = " ";
    size_t pos = 0;
    string token;

    int counter = 0;
    string command_tokens[3] = {"", "", ""};

    // Parsing command by white space
    while ((pos = s.find(delimiter)) != string::npos) 
    {
        token = s.substr(0, pos);
        s.erase(0, pos + delimiter.length());  

        if (counter > 2)        
            quit_program(false, "Command error");
              
        command_tokens[counter] = token;
        counter++;
    }

    if(counter == 0)
        command_tokens[0] = s;
    else if(counter == 1)
        command_tokens[1] = s;
    else if(counter == 2)
        command_tokens[2] = s;    

    // Command name to uppercase
    transform(command_tokens[0].begin(), command_tokens[0].end(), command_tokens[0].begin(), ::toupper);
   
    // Creating new command object
    Command* command = new Command(command_tokens[0], command_tokens[1], command_tokens[2]);
    return *command;
}

string get_response(Command command)
{
    string response = "";
    // User is not logged in
    if (!logged_in)
    {
        // USER 
        if (command.name == "USER" && user_name == "")
        {               
            user_name = command.argument1;
            bool username_exists = check_username();   // Checks if user name exists
            if (username_exists)
                response = "+User-id valid, send password\r\n";
            else
                response = "-Invalid user-id, try again\r\n";            
        }
        // PASS
        else if (command.name == "PASS" && user_name != "")
        {
            bool pass_ok = check_password(command.argument1);
            if (pass_ok)
            {
                response = "! Logged in\r\n";
                logged_in = true;
            }
            else
                response = "- Wrong password, try again\r\n";            
        }
        // OTHER
        else
            response = "-Invalid command, you should login first. Try again.\r\n";        
    }
    // User is logged in
    else
    {
        if(send_tobe && command.name != "TOBE")
            response = "-Error, TOBE command should be next\r\n";
        
        // TYPE command
        else if(command.name == "TYPE")
        {
            response = "TYPE";
        }

        // LIST command
        else if(command.name == "LIST")
        {
            // Getting directory
            string directory;
            if(command.argument2 == "")
                directory = working_dir;
            else
                directory = command.argument2;

            // Wrong command arguments
            if(command.argument1 != "F" && command.argument1 != "V")
            {
                response = "-Wrong command arguments\r\n";
                return response;
            }

            DIR *dir;
            struct dirent *ent;
            if ((dir = opendir (directory.c_str())) != NULL) 
            {
                response = "+" + directory + "\r\n";
                
                /* get all the files and directories within directory */
                while ((ent = readdir (dir)) != NULL) 
                {
                    // File or directory found
                    if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                    {
                        // Classic write out
                        if(command.argument1 == "F")
                        {
                            response += ent->d_name;
                            response += "\r\n";
                        }
                        // Verbose
                        else
                        {
                            streampos begin,end;                            
                            ifstream myfile (ent->d_name, ios::binary);
                            begin = myfile.tellg();
                            myfile.seekg (0, ios::end);
                            end = myfile.tellg();
                            myfile.close();

                            int status;
                            struct stat buffer;
                            status = stat(ent->d_name, &buffer);

                            response += ent->d_name;
                            response += " ";
                            response += to_string((int)(end-begin));
                            response += " bytes, permission: ";
                            response += to_string(status);
                            response += "\r\n";
                        }
                    }
                }
                closedir (dir);
            }
            else 
            {
                response = "-";
                response += strerror(errno);
            }
        }

        // CDIR
        else if(command.name == "CDIR")
        {
            string directory = command.argument1;
            DIR *dir;
            struct dirent *ent;
            if ((dir = opendir (directory.c_str())) != NULL) 
            {
                response = "!Changed working dir to " + directory;
                working_dir = directory;
                closedir(dir);
            }                
            else
            {
                response = "-Cant't connect to directory becase: ";
                response += strerror(errno);
            }
        }

        // KILL
        else if(command.name == "KILL")
        {
            if(remove(command.argument1.c_str()) != 0)
            {
                response = "-Not deleted because ";
                response += strerror(errno);
            }                
            else            
                response = "+" + command.argument1 + " deleted";
        }
        // NAME
        else if(command.name == "NAME")
        {
            ifstream f(command.argument1.c_str());
            if(f.good() == true)
            {
                response = "File exists";
                rename_file = command.argument1;
                send_tobe = true;
            }                
            else
            {
                response = "-Can't find ";
                response += command.argument1;
            }
        }
        else if(command.name == "TOBE")
        {
            if (rename(rename_file.c_str() , command.argument1.c_str()) == 0)
            {
                response = rename_file + " renamed to " + command.argument1;
                rename_file = "";
                send_tobe = false;
            }
            else
            {
                rename_file = "";
                send_tobe = false;
                response = "-File wasn't renamed because ";
                response += strerror(errno);
            }
        }
        // DONE
        else if(command.name == "DONE")
        {
            response = "+We are done\r\n";
        }
        // RETR
        else if(command.name == "RETR")
        {
            response = "RETR\r\n";
        }
        // STOR
        else if(command.name == "STOR")
        {
            response = "STOR\r\n";
        }
        // SIZE
        else if(command.name == "SIZE")
        {
            response = "SIZE\r\n";
        }
        // Unknown command
        else
        {
            response = "-Unknown command\r\n";
        }
    }
    
    return response;
}

bool check_username()
{
    bool username_found = false;
    int pos;
    string line;
    ifstream file(path);

    // File error
    if(file.fail())
        quit_program(false, "File error");

    // Loop through all lines in file
    while (getline(file, line))
    {
        pos = line.find(":");

        if(line.substr(0, pos) == user_name)
            username_found = true;
    }

    file.close();
    return username_found;
}

bool check_password(string password)
{
    bool password_match = false;
    int pos;
    string line;
    ifstream file(path);

    // File error
    if(file.fail())
        quit_program(false, "File error");

    while (getline(file, line))
    {
        pos = line.find(":");

        // Finds the first occurance of username and checks password
        if(line.substr(0, pos) == user_name)
            if(line.substr(pos + 1, line.length()) == password)
            {
                password_match = true;
                break;
            }                
    }    

    file.close();
    return password_match;
}






// https://www.reddit.com/r/cpp_questions/comments/4frasp/binding_sockets_to_specific_interfacevirtual/
// https://stackoverflow.com/questions/14478167/bind-socket-to-network-interface
// https://www.geeksforgeeks.org/socket-programming-cc/