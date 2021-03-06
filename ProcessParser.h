#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"

using namespace std;

class ProcessParser {
    public:
        static std::string get_cmd(std::string pid);
        static int get_number_of_cores();
        static std::vector<std::string> get_pid_list();
        static std::string get_vm_size(std::string pid);
        static std::string get_cpu_percent(std::string pid);
        static long int get_sys_up_time();
        static std::string get_proc_up_time(std::string pid);
        static std::string get_proc_user(std::string pid);
        static std::vector<std::string> get_sys_cpu_percent(std::string coreNumber = "");
        static float get_sys_ram_percent();
        static std::string get_sys_kernel_version();
        static int get_total_threads();
        static int get_total_number_of_processes();
        static int get_number_of_running_processes();
        static string get_os_name();
        static std::string print_cpu_stats(std::vector<std::string> values1, std::vector<std::string>values2);
        static bool is_pid_existing(string pid);
};

// Reading /proc/[PID]/status for memory status of specific process 
string ProcessParser::get_vm_size(string pid)
{
    string line;
    //Declaring search attribute for file
    string name = "VmData";
    string value;
    float result;
    // Opening stream for specific file
    ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    while(std::getline(stream, line)){
        // Searching line by line
        if (line.compare(0, name.size(),name) == 0) {
            // slicing string line on ws for values using sstream
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            //conversion kB -> GB
            result = (stof(values[1])/float(1024));
            break;
        }
    }
    return to_string(result);
}

// Reading CPU usage percentage 
string ProcessParser::get_cpu_percent(string pid)
{
    string line;
    string value;
    float result;
    ifstream stream = Util::getStream((Path::basePath()+ pid + "/" + Path::statPath()));
    getline(stream, line);
    string str = line;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end); // done!
    // acquiring relevant times for calculation of active occupation of CPU for selected process
    float utime = stof(ProcessParser::get_proc_up_time(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = ProcessParser::get_sys_up_time();
    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    result = 100.0*((total_time/freq)/seconds);
    return to_string(result);
}

// reading procs uptime
string ProcessParser::get_proc_up_time(string pid)
{
    string line;
    string value;
    float result;
    ifstream stream = Util::getStream((Path::basePath() + pid + "/" +  Path::statPath()));
    getline(stream, line);
    string str = line;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end); // done!
    // Using sysconf to get clock ticks of the host machine
    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}

// system uptime
long int ProcessParser::get_sys_up_time()
{
    string line;
    ifstream stream = Util::getStream((Path::basePath() + Path::upTimePath()));
    getline(stream,line);
    istringstream buf(line);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    return stoi(values[0]);
}

// user's processes
string ProcessParser::get_proc_user(string pid)
{
    string line;
    string name = "Uid:";
    string result ="";
    ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    // Getting UID for user
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result =  values[1];
            break;
        }
    }
    stream = Util::getStream("/etc/passwd");
    name =("x:" + result);
    // Searching for name of the user with selected UID
    while (std::getline(stream, line)) {
        if (line.find(name) != std::string::npos) {
            result = line.substr(0, line.find(":"));
            return result;
        }
    }
    return "";
}

// reading processes id into list
vector<string> ProcessParser::get_pid_list()
{
    DIR* dir;
    // Basically, we are scanning /proc dir for all directories with numbers as their names
    // If we get valid check we store dir names in vector as list of machine pids
    vector<string> container;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));

    while (dirent* dirp = readdir(dir)) {
        // is this a directory?
        if(dirp->d_type != DT_DIR)
            continue;
        // Is every character of the name a digit?
        if (all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
            container.push_back(dirp->d_name);
        }
    }
    //Validating process of directory closing
    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));
    return container;
}

// reading command line string for each process
string ProcessParser::get_cmd(string pid)
{
    string line;
    ifstream stream = Util::getStream((Path::basePath() + pid + Path::cmdPath()));
    std::getline(stream, line);
    return line;
}

// getting the CPU number of cores
int ProcessParser::get_number_of_cores()
{
    // Get the number of host cpu cores
    string line;
    string name = "cpu cores";
    ifstream stream = Util::getStream((Path::basePath() + "cpuinfo"));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return stoi(values[3]);
        }
    }
    return 0;
}

// to get the core usage percentage
vector<string> ProcessParser::get_sys_cpu_percent(string coreNumber)
{
    // It is possible to use this method for selection of data for overall cpu or every core.
    // when nothing is passed "cpu" line is read
    // when, for example "0" is passed  -> "cpu0" -> data for first core is read
    string line;
    string name = "cpu" + coreNumber;
    string value;
    int result;
    ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            // set of cpu data active and idle times;
            return values;
        }
    }
    return (vector<string>());
}

// active CPU time
float get_sys_active_cpu_time(vector<string> values)
{
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float get_sys_idle_cpu_time(vector<string>values)
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

// print CPU statistics
string ProcessParser::print_cpu_stats(vector<string> values1, vector<string> values2)
{

/*
Because CPU stats can be calculated only if you take measures in two different time,
this function has two paramaters: two vectors of relevant values.
We use a formula to calculate overall activity of processor.
*/
    float active_time = get_sys_active_cpu_time(values2)-get_sys_active_cpu_time(values1);
    float idle_time = get_sys_idle_cpu_time(values2) - get_sys_idle_cpu_time(values1);
    float total_time = active_time + idle_time;
    float result = 100.0*(active_time / total_time);
    return to_string(result);
}

// reading system RAM usage percentage
float ProcessParser::get_sys_ram_percent()
{
    string line;
    string name1 = "MemAvailable:";
    string name2 = "MemFree:";
    string name3 = "Buffers:";

    string value;
    int result;
    ifstream stream = Util::getStream((Path::basePath() + Path::memInfoPath()));
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;
    while (std::getline(stream, line)) {
        if (total_mem != 0 && free_mem != 0)
            break;
        if (line.compare(0, name1.size(), name1) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
        }
        if (line.compare(0, name2.size(), name2) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
        }
        if (line.compare(0, name3.size(), name3) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
        }
    }
    //calculating usage:
    return float(100.0*(1-(free_mem/(total_mem-buffers))));
}

// reading system kernel version
string ProcessParser::get_sys_kernel_version()
{
    string line;
    string name = "Linux version ";
    ifstream stream = Util::getStream((Path::basePath() + Path::versionPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values[2];
        }
    }
    return "";
}

// getting OS name
string ProcessParser::get_os_name()
{
    string line;
    string name = "PRETTY_NAME=";

    ifstream stream = Util::getStream(("/etc/os-release"));

    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
              std::size_t found = line.find("=");
              found++;
              string result = line.substr(found);
              result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
              return result;
        }
    }
    return "";

}

// calculate the total number of threads
int ProcessParser::get_total_threads()
{
    string line;
    int result = 0;
    string name = "Threads:";
    vector<string>pid_list = ProcessParser::get_pid_list();
  
    for (int i=0 ; i<pid_list.size();i++) {
        string pid = pid_list[i];

        //getting every process and reading their number of their threads
        ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
        while (std::getline(stream, line)) {
            if (line.compare(0, name.size(), name) == 0) {
                istringstream buf(line);
                istream_iterator<string> beg(buf), end;
                vector<string> values(beg, end);
                result += stoi(values[1]);
                break;
            }
        }
    }
    return result;
}

// calculate the total number of processes
int ProcessParser::get_total_number_of_processes()
{
    string line;
    int result = 0;
    string name = "processes";
    ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

// calculate the total number of active processes
int ProcessParser::get_number_of_running_processes()
{
    string line;
    int result = 0;
    string name = "procs_running";
    ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

// checking if the passed PID exist in the system
bool ProcessParser::is_pid_existing(string pid)
{
    // get the processes list
    vector<string>pid_list = ProcessParser::get_pid_list();
    // check if pid is in the list
    if (std::find(pid_list.begin(), pid_list.end(), pid) != pid_list.end())
    {
        // return true if it's in the list
        return true;
    }
    return false;
}
