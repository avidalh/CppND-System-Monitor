#include "Process.h"
#include <string>
#include <vector>
using std::string;
using std::vector;

class ProcessContainer {
    public:
        ProcessContainer()
        {
            this->refreshList();
        }
        void refreshList();
        string printList();
        vector<string> get_list();

    private:
        vector<Process>_list;
};

void ProcessContainer::refreshList()
{
    vector<string> pids = ProcessParser::get_pid_list();
    this->_list.clear();
    for (auto pid : pids) {
        Process proc(pid);
        this->_list.push_back(proc);
    }
}

string ProcessContainer::printList()
{
    std::string result="";
    for (auto i : _list) {
        result += i.get_process();
    }
    return result;
}

vector<string> ProcessContainer::get_list() 
{
    vector<string> values;
    for (int i = (this->_list.size()-10); i < this->_list.size(); i++){
        values.push_back(this->_list[i].get_process());
    }
    return values;
}

