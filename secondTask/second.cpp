/*
By Nikita Matsnev
how to run:
    g++ second.cpp -o second
    ./second source_path replica_path synchronization_interval log_path
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <iterator>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/stat.h>
#include <dirent.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

struct item
{
    string name;
    string path;
    int type; // 0 - file, 1 - directory

    bool operator==(const item &other) const
    {
        return (name == other.name && path == other.path && type == other.type);
    }
};

inline bool
exists(const string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

string exec(const char *cmd)
{
    /*
    precondition: gets command to execute
    postcondition: returns output of command
    */
    assert(cmd != nullptr);
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

bool compareFiles(string path1, string path2)
{
    /*
    precondition: gets two paths to files
    postcondition: returns true if files are identical, false otherwise
    */
    assert(path1 != "" && path2 != "");
    ifstream file1(path1);
    ifstream file2(path2);
    if (file1.is_open() && file2.is_open())
    {
        string line1, line2;
        while (getline(file1, line1) && getline(file2, line2))
        {
            if (line1 != line2)
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

vector<item> getItems(string path)
{
    /*
    precondition: gets path to directory
    postcondition: returns vector of items
    */
    assert(path != "");
    vector<item> items;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_name[0] != '.')
            {
                item newItem;
                newItem.name = ent->d_name;
                newItem.path = path;
                if (ent->d_type == DT_DIR)
                {
                    newItem.type = 1;
                    vector<item> temp = getItems(path + "/" + newItem.name);
                    for (int i = 0; i < temp.size(); i++)
                    {
                        items.push_back(temp[i]);
                    }
                }
                else
                {
                    newItem.type = 0;
                }
                items.push_back(newItem);
            }
        }
        closedir(dir);
    }
    else
    {
        perror("");
    }
    return items;
}

void updateDirectory(string folderPath, vector<item> items, ofstream& logFile)
{
    /*
    precondition: gets path of folder and vector of items
    postcondition: updates the directory, if the file or folder does not exist in the vector
    */
    assert(folderPath != "");
    vector<item> contents = getItems(folderPath);

    for (int i = 0; i < contents.size(); i++)
    {
        cout << "content: " << contents[i].name << " " << contents[i].path << " " << contents[i].type << endl;
        logFile << "content: " << contents[i].name << " " << contents[i].path << " " << contents[i].type << endl;
    }
    for (int i = 0; i < items.size(); i++)
    {
        cout << "item: " << items[i].name << " " << items[i].path << " " << items[i].type << endl;
        logFile << "item: " << items[i].name << " " << items[i].path << " " << items[i].type << endl;
    }

    if (contents.size() == 0)
    {
        string command = "cp -r " + items[0].path + "/. " + folderPath;
        cout << "Command " << command << " was performed" << endl;
        logFile << "Command " << command << " was performed" << endl;
        exec(command.c_str());
        cout << "Folder " << folderPath << " was updated" << endl;
        logFile << "Folder " << folderPath << " was updated" << endl;
    }
    else
    {
        for (int i = 0; i < contents.size(); i++)
        {
            if (find(items.begin(), items.end(), contents[i]) == items.end())
            {
                if (contents[i].type == 0)
                {
                    string command = "rm " + folderPath + "/" + contents[i].name;
                    cout << "Command " << command << " was performed" << endl;
                    logFile << "Command " << command << " was performed" << endl;
                    cout << "File " << folderPath + "/" + contents[i].name << " was deleted" << endl;
                    logFile << "File " << folderPath + "/" + contents[i].name << " was deleted" << endl;
                    exec(command.c_str());
                }
                else
                {
                    string command = "rm -r " + folderPath + "/" + contents[i].name;
                    cout << "Command " << command << " was performed" << endl;
                    logFile << "Command " << command << " was performed" << endl;
                    cout << "Folder " << folderPath + "/" + contents[i].name << " was deleted" << endl;
                    logFile << "Folder " << folderPath + "/" + contents[i].name << " was deleted" << endl;
                    exec(command.c_str());
                }
            }
            else
            {
                if (contents[i].type == 0)
                {
                    if (!compareFiles(folderPath + "/" + contents[i].name, contents[i].path))
                    {
                        string command = "cp " + contents[i].path + " " + folderPath;
                        cout << "Command " << command << " was performed" << endl;
                        logFile << "Command " << command << " was performed" << endl;
                        exec(command.c_str());
                    }
                }
                else
                {
                    string command = "mkdir " + folderPath + "/" + contents[i].name;
                    cout << "Command " << command << " was performed" << endl;
                    logFile << "Command " << command << " was performed" << endl;
                    exec(command.c_str());
                    updateDirectory(folderPath + "/" + contents[i].name, items, logFile);
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    string sourcePath = argv[1];
    cout << "Source path: " << sourcePath << endl;
    string replicaPath = argv[2];
    cout << "Replica path: " << replicaPath << endl;
    int syncInterval = atoi(argv[3]);
    cout << "Sync interval: " << syncInterval << endl;
    string logFilePath = argv[4];
    cout << "Log file path: " << logFilePath << endl;

    // counter for amount of times we synchronize
    int counter = 0;

    while (counter < 10)
    {
        ofstream logFile;
        logFile.open(logFilePath, ios::app);
        vector<item> source = getItems(sourcePath);
        cout << "Source folder contents: " << endl;
        logFile << "Source folder contents: " << endl;
        for (int i = 0; i < source.size(); i++)
        {
            cout << source[i].path << "/" << source[i].name << " type:" << source[i].type << endl;
            logFile << source[i].path << "/" << source[i].name << " type:" << source[i].type << endl;
        }
        // for all items in source remove everything before the first slash
        for (int i = 0; i < source.size(); i++)
        {
            source[i].path = source[i].path.substr(source[i].path.find("/") + 1);
        }
        updateDirectory(replicaPath, source, logFile);
        time_t t = time(0); // get time now
        tm *now = localtime(&t);
        cout << "Replica folder was successfully updated at : " << now->tm_mday << '-' << (now->tm_mon + 1) << '-' << (now->tm_year + 1900) << ' ' << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << endl;
        cout << "-----------------------------------------------------" << endl;
        logFile << "Replica folder was successfully updated at : " << now->tm_mday << '-' << (now->tm_mon + 1) << '-' << (now->tm_year + 1900) << ' ' << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << endl;
        logFile << "-----------------------------------------------------" << endl;
        counter++;
        sleep(syncInterval);
        logFile.close();
    }
}