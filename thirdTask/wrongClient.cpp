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
#include <cassert>

using namespace std;


string generateMessage(string id, string code){
    /*
    precondition: gets unique id and code
    postcondition: returns text message based on the given id and code
    */
    assert(id != "" && code != "");
    string message = "Hello, " + id + "! Your code is " + code + ".";
    return message;
}


string generateId()
{
    /*
    precondition: none
    postcondition: returns unique id based on time
    */
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string id = to_string(ltm->tm_mday) + to_string(1 + ltm->tm_mon) + to_string(1900 + ltm->tm_year) + to_string(1 + ltm->tm_hour) + to_string(1 + ltm->tm_min) + to_string(1 + ltm->tm_sec);
    return id;
}


int main(int argc, char **argv)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = PF_INET;
    serverAddress.sin_port = htons(8000);
    struct hostent *host;
    host = gethostbyname("127.0.0.1"); // netdb.h
    memcpy(&serverAddress.sin_addr, host->h_addr, host->h_length);

    if (connect(s, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) != 0)
    {
        perror("Can't connect.");
        close(s);
        return -1;
    }

#define BUFFER_SIZE 1024

    char buffer[BUFFER_SIZE];
    string id = generateId();
    // add id to buffer
    strcpy(buffer, id.c_str());
    send(s, buffer, strlen(buffer), 0);
    // receive code from server
    recv(s, buffer, BUFFER_SIZE, 0);
    string code = "111111";
    cout << "Your code is: " << code << endl;

    
    int s1 = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_port = htons(8001);
    memcpy(&serverAddress.sin_addr, host->h_addr, host->h_length);

    if (connect(s1, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) != 0)
    {
        perror("Can't connect.");
        close(s1);
        return -1;
    }

    char buffer1[BUFFER_SIZE];
    
    string message = id + "\n" + code + "\n" + "Hello, " + id + "! Your code is " + code + ".";
    strcpy(buffer1, message.c_str());
    cout << "Sending message:\n" << message << endl;
    send(s1, buffer1, strlen(buffer1), 0);
    
    
    memset(buffer1, 0, BUFFER_SIZE);
    recv(s1, buffer1, BUFFER_SIZE, 0);
    cout << "Received message:\n" << buffer1 << endl;
    
    close(s);
    close(s1);
    return 0;
}
