/*
 By Nikita Matsnev
how to run server:
    g++ server.cpp -o server
    ./server
how to run client:
    g++ client.cpp -o client
    ./client
how to run wrong client:    
    g++ wrongClient.cpp -o wrongClient
    ./wrongClient
*/
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <wait.h>
#include <vector>
#include <cassert>
#include <fstream>
#include <sstream>

#define BUFFER_SIZE 1024

using namespace std;

struct client
{
    string id;
    string code;
    string message;
};

bool checkClient(vector<client> clients, string code)
{
    /*
    precondition: gets vector of clients and code
    postcondition: returns true if client is in vector, false otherwise
    */
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].code == code)
        {
            return true;
        }
    }
    return false;
}

string generateCode(string id)
{
    /*
    precondition: gets unique id
    postcondition: returns unique code based on the given id
    */
    assert(id != "");
    string code = "";
    for (int i = 0; i < id.length(); i++)
    {
        code += to_string((int)id[i]);
    }
    return code;
}

client getClient(string text)
{
    /*
    precondition: gets text message
    postcondition: returns client struct
    */
    client c;
    stringstream ss(text);
    string line;
    int i = 0;
    while (getline(ss, line, '\n'))
    {
        if (i == 0)
        {
            c.id = line;
        }
        else if (i == 1)
        {
            c.code = line;
        }
        else
        {
            c.message += line;
        }
        i++;
    }
    return c;
}

// 8001 port
string waitForCode(client our_client)
{
    /*
    precondition: none
    postcondition: returns 1 if bad and 0 if good
    */
    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = PF_INET;
    serverAddress.sin_port = htons(8001);
    struct hostent *host;
    host = gethostbyname("127.0.0.1"); // netdb.h
    memcpy(&serverAddress.sin_addr, host->h_addr, host->h_length);

    if (bind(s, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) != 0)
    {
        perror("Can't bind.");
        close(s);
        return "1";
    }

    if (listen(s, 1) != 0)
    {
        perror("Can't listen.");
        close(s);
        return "1";
    }

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int s2 = accept(s, (struct sockaddr *)&clientAddress, &clientAddressLength);
    if (s2 == -1)
    {
        perror("Can't accept.");
        close(s);
        return "1";
    }

    char buffer[BUFFER_SIZE];
    recv(s2, buffer, BUFFER_SIZE, 0);    
    client cl = getClient(buffer);
    string code = cl.code;
    if (our_client.code == code)
    {
        cout << "Client with code " << code << " exists." << endl;
        close(s);
        close(s2);
        // create logfile named after id and write message to it
        ofstream logfile;
        logfile.open("log" + cl.id + ".txt");
        cout << "id:" << cl.id << endl;
        cout << "code:" << cl.code << endl;
        cout << "message:" << cl.message << endl;
        logfile << cl.message;
        logfile.close();
        return "0";
    }
    memset(buffer, 0, BUFFER_SIZE);
    // send error message to client
    strcpy(buffer, "Error happened!!!");
    send(s2, buffer, strlen(buffer), 0);
    close(s2);
    close(s);
    return "1";
}

int main(int argc, char **argv)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in address;
    address.sin_family = PF_INET;
    address.sin_port = htons(8000);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *)&address, sizeof(address)) != 0)
    {
        perror("Problem with bind()"); // System message "Problem with bind(): error from system"
        close(s);
        return -1;
    }

    if (listen(s, 10) != 0)
    { // 10 - how many clients could be served
        perror("Can't set socket to listen.");
        close(s);
        return -1;
    }

    struct sockaddr_in remoteAddress;
    socklen_t remoteAddressLength;

    client our_client;

    while (1)
    {
        int c = accept(s, (struct sockaddr *)&remoteAddress, &remoteAddressLength);

        if (c < 0)
        {
            perror("Can't accept new connection.");
            close(s);
            return -1;
        }

        pid_t pid = fork(); // copy of actual process
        if (pid == 0)
        {             // copy of process, does not have child
            close(s); // do not need listening socket in the copy

            char buffer[BUFFER_SIZE];

            struct timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            fd_set sockets;
            while (1)
            {
                FD_ZERO(&sockets);
                FD_SET(c, &sockets);
                if (select(c + 1, &sockets, NULL, NULL, &timeout) <
                    0)
                { // is called before recv of each message, first argument: last FD +1
                    perror("Error in select.");
                    close(c);
                    return -1;
                }

                if (!FD_ISSET(c, &sockets))
                {
                    cout << "Connection timeout!" << endl;
                    close(c);
                    return -1;
                }

                int bytesRecieved = recv(c, &buffer, BUFFER_SIZE - 1, 0);
                if (bytesRecieved <= 0)
                {
                    perror("Can't read.");
                    close(c);
                    return -1;
                }
                buffer[bytesRecieved] = '\0';
                if (buffer == string("end"))
                {
                    break;
                }
                cout << "id:" << buffer << endl;
                string code = generateCode(buffer);

                our_client = {buffer, code, ""};
                // client
                cout << "Client " << buffer << " connected with code " << code << endl;
                // send code to client
                strcpy(buffer, code.c_str());
                send(c, buffer, strlen(buffer), 0);
                // wait for code from client from the port 8001
                string result = waitForCode(our_client);
            }
            close(c);
            return 0;
        }
        int status = 0;
        waitpid(0, &status, WNOHANG);
        close(c);
    }
    close(s);
    return 0;
}
