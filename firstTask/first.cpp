/*
By Nikita Matsnev

g++ first.cpp -o first
./first nameOfExecutionFile TimeIntervalInSeconds
*/
#include <iostream>
#include <cstdlib>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/stat.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <memory>
#include <stdexcept>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <ctime>
#include <dirent.h>

struct sysinfo memInfo;

using namespace std;

inline bool exists (const string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
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

int getCpuPercent(int pid)
{
	/*
	precondition: gets pid of process
	postcondition: returns percent of cpu used by process
	*/
	string command = "ps -p " + to_string(pid) + " -o pcpu | tail -n 1";
	string output = exec(command.c_str());
	cout << "cpu:" << output << endl;
	return stoi(output);
}

int getResidentSetSize(int pid)
{
	/*
	precondition: gets pid of process
	postcondition: returns resident set size of the process
	*/
	string command = "ps -p " + to_string(pid) + " -o %mem | tail -n 1";
	string output = exec(command.c_str());
	cout << "rss:" << output << endl;
	return stoi(output);
}

int getOpenFileDescriptors(int pid)
{
	/*
	precondition: gets pid of process
	postcondition: returns open file descriptors of the process
	*/
	string command = "lsof | grep " + to_string(pid) + " | wc -l";
	string output = exec(command.c_str());
	cout << "open file descriptors:" << output << endl;
	return stoi(output);
}

int getVirtualMemorySize(int pid)
{
	/*
	precondition: gets pid of process
	postcondition: returns virtual memory size of the process
	*/
	string command = "ps -p " + to_string(pid) + " -o vsz | tail -n 1";
	string output = exec(command.c_str());
	cout << "vms:" << output << endl;
	return stoi(output);
}

int getOpenHandles(int pid)
{
	/*
	precondition: gets pid of process
	postcondition: returns open handles of the process
	*/
	string command = "lsof -p " + to_string(pid) + " | wc -l";
	string output = exec(command.c_str());
	cout << "open handles:" << output << endl;
	return stoi(output);
}

int main(int argc, char **argv)
{
	// first command line argument is the path to the file which should be executed
	string pathToProcess = argv[1];
	cout << "pathToProcess: " << pathToProcess << endl;
	// second command line is time interval in seconds to check the process
	int timeInterval = stoi(argv[2]);

	string pid = exec(("pidof " + pathToProcess).c_str());
	
	ofstream myfile;
	if(exists("log.txt")){
		myfile.open("log.txt", ios_base::app);
	}else{
		ofstream myfile("log.txt", ios_base::app);
		myfile.open("log.txt", ios_base::app);
	}
	
	while (pid != "")
	{
		if(!myfile.is_open()){
			myfile.open("log.txt", ios_base::app);
		}
		cout << "PID: " << pid << endl;
		int pidInt = stoi(pid);
		cout << "pidInt: " << pidInt << endl;
#ifdef _WIN32
		cout << "Windows" << endl;
		int openHandles = getOpenFileDescriptors(pidInt);
		int cpuUsage = getCpuPercent(pidInt);
		int residentSetSize = getResidentSetSize(pidInt);
		int virtualMemorySize = getVirtualMemorySize(pidInt);
		int memoryUsage = residentSetSize + virtualMemorySize;
#elif __linux__
		cout << "Linux" << endl;
		int openHandles = getOpenFileDescriptors(pidInt);
		int cpuUsage = getCpuPercent(pidInt);
		int residentSetSize = getResidentSetSize(pidInt);
		int virtualMemorySize = getVirtualMemorySize(pidInt);
		int memoryUsage = residentSetSize + virtualMemorySize;
#endif
		time_t t = time(0); // get time now
		tm* now = localtime(&t);
		myfile << "New Probe:" << endl;
		myfile << "PID(it is for the sake of knowing that the file has been updated): " << pidInt << endl;
		myfile << "CPU Usage: " << cpuUsage << "%" << endl;
		myfile << "Resident Set Size: " << residentSetSize << "KB" << endl;
		myfile << "Virtual Memory Size: " << virtualMemorySize << "KB" << endl;
		myfile << "Total Memory Usage: " << memoryUsage << "KB" << endl;
		myfile << "Open Handles: " << openHandles << endl;
		myfile << "Date and Time: " << now->tm_mday << '-' << (now->tm_mon + 1) << '-' << (now->tm_year + 1900) << ' ' << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << endl;
		myfile << "-----------------------------------------------------" << endl;
		sleep(timeInterval);
		myfile.close();
	}
	return 0;
}