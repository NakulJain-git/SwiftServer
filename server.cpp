#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <thread>
#include <filesystem>
#include <fstream>
using namespace std;

const char *Not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
const char *JUST_found = "HTTP/1.1 200 OK\r\n\r\n";
string GET_Check = "GET /user-agent HTTP/1.1";


void handleClient(int client_fd, const string &file_path)
    {
        cout << "Handling client in thread: " << this_thread::get_id() << endl;
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        const char *message = Not_found;

        int checking = read(client_fd, buffer, 1024);
        if (checking < 0)
        {
            cerr << "Error reading from client" << endl;
            close(client_fd);
            return;
        }
        else
        {
            cout << "Client connected: " << buffer << endl;
        }
        // Structure of the raw request sent by curl command
        //  GET /echo/hello HTTP/1.1\r\n
        //  Host: localhost:4221\r\n
        //  User-Agent: curl/7.68.0\r\n
        //  Accept: */*\r\n
        //  \r\n

        char *token = strtok(buffer, "\r\n");
        vector<string> request_lines;
        while (token != NULL)
        {
            request_lines.push_back(string(token));
            token = strtok(NULL, "\r\n");
        }
        // checking the request lines for my referenece
        // for (const auto &line : request_lines)
        // {
        //     cout << "Request line: " << line << endl;
        // }
        if (request_lines.empty()) {
            cerr << "Empty request. Skipping.\n";
            close(client_fd);
            return;
        }

        string checker = request_lines[0];
        string User_Agent;
        cout << "Checker: " << checker << endl;
        size_t isPost = checker.find("POST");
        size_t found = checker.find("echo");
        size_t Filefound = checker.find("files");

        // User_Agent
        for(auto &it : request_lines){
            if (it[0] == 'U'){
                User_Agent = it;
                break;
            }
        }

        cout<< "isPost: " << isPost << endl;
        cout<< "found: " << found << endl;
        cout<< "Filefound: " << Filefound << endl;
        cout<< "User_Agent: " << User_Agent << endl;
        
        int flag2=0;
        // response generation based on the type of request
        if (found != string::npos)
        {
            int flag = 0;
            string s;
            for (int i = 1; i < checker.length() - 1; i++){
                if (flag == 1){
                    if (checker[i] == ' ') break;
                    s += checker[i];
                }
                else if (checker[i - 1] == 'o' && checker[i] == '/') flag = 1;
            }
            string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Content-Length: ";
            response += to_string(s.length());
            response += "\r\n\r\n";
            response += s;
            response += "\r\n";
            if (flag == 1){
                message = response.c_str();
                int bytes_sent = send(client_fd, message, strlen(message), 0);
                return;
            }
        }
        else if(GET_Check==checker){
            cout<<User_Agent<<endl;
            int flag=0;
            string s;
            for (int i = 1; i < User_Agent.length(); i++){
                if (flag == 1){
                    s += User_Agent[i];
                }
                else if (User_Agent[i - 1] == ':' && User_Agent[i] == ' ') flag = 1;
            }
            string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Content-Length: ";
            response += to_string(s.length());
            response += "\r\n\r\n";
            response += s;
            response += "\r\n";

            if (flag == 1)
            {
                message = response.c_str();
                int bytes_sent = send(client_fd, message, strlen(message), 0);
                return;
            }
        }
        else if(Filefound!=string::npos){
            int flag = 0;
            string s;
            for (int i = 1; i < checker.length() - 1; i++)
            {
                if (flag == 1)
                {
                    if (checker[i] == ' ')
                    break;
                    s += checker[i];
                }
                else if (checker[i - 1] == 's' && checker[i] == '/')
                {
                    flag = 1;
                }
            }
            //writing file
            if (isPost != string::npos)
            {
                cout<<s<<endl;
                string full_file_path = file_path + "/" + s;
                ofstream new_file(full_file_path);

                if (!new_file) {
                    cerr << "Error: Failed to create file at " << full_file_path << endl;
                    const char *response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
                    send(client_fd, response, strlen(response), 0);
                    return;
                }

                string buff = request_lines.back();
                cout << buff << endl;

                new_file << buff;
                new_file.close();

                // Check if file exists after writing
                if (!filesystem::exists(full_file_path)) {
                    cerr << "Error: File was not written to disk." << endl;
                    const char *response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
                    send(client_fd, response, strlen(response), 0);
                    return;
                }

                string response = "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n";
                send(client_fd, response.c_str(), response.length(), 0);

            }
            //reading file
            else{
                string full_path = file_path + '/';
                full_path += (s);
                ifstream file(full_path);
                if (file.is_open())
                {
                    string file_content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                    cout << file_content;

                    string response = "HTTP/1.1 200 OK\r\n";
                    response += "Content-Type: application/octet-stream\r\n";
                    response += "Content-Length: ";
                    response += to_string(file_content.length());
                    response += "\r\n\r\n";
                    response += file_content;
                    response += "\r\n";

                    if (flag == 1) {
                        message = response.c_str();
                        int bytes_sent = send(client_fd, message, strlen(message), 0);
                        file.close();
                        return;
                    }
                }
                else
                {
                    const char *response = "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
                    int bytes_sent = send(client_fd, response, strlen(response), 0);

                    return;
                }
                file.close();
            }
        }
        else {
        for (int i = 1; i < checker.length(); i++)
            {
                if (checker[i] == '/' && checker[i + 1] == ' ')
                {
                    flag2 = 1;
                    break;
                }
            }
            message = (flag2 == 1) ? JUST_found : Not_found;
        }

        int bytes_sent = send(client_fd, message, strlen(message), 0);
        return;
    }

int main(int argc, char *argv[])
    {
        // setting file path if given as argument
        string file_path = "Your_Directory_Here";// default directory
        if (argc == 3 && argv[1] == "--directory")
        {
            file_path = argv[2];
        }
        // server_fd is socket file descriptor
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
        {
            cerr << "Error creating socket" << endl;
            return 1;
        }
        // reuse of port
        int reuse = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
        {
            cerr << "setsockopt failed" << endl;
            return 1;
        }
        // binding socket to all IP and declaring port 4221

        // decalaring server address
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(4221);

        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
        {
            cerr << "Failed to bind to port 4221\n";
            return 1;
        }
        int connection_backlog = 5;
        if (listen(server_fd, connection_backlog) != 0)
        {
            cerr << "listen failed\n";
            return 1;
        }
        // initializing client address
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        // multiple threads for multiplee users
        vector<thread> CLIENTS;
        while (true)
        {
            cout << "Waiting for a client to connect...\n";
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
            if (client_fd < 0)
            {
                cerr << "Failed to accept client connection\n";
                break;
            }
            CLIENTS.emplace_back(handleClient, client_fd, file_path);
        }

        for (auto &it : CLIENTS)
        {
            if (it.joinable())
            {
                it.join();
            }
        }
        close(server_fd);
        cout << "Server shutting down...\n";
        return 0;
    }