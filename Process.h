#include <string>
using std::string;

/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
    private:
        string pid;
        string user;
        string cmd;
        string cpu;
        string mem;
        string up_time;

    public:
        Process(string pid)
        {
            this->pid = pid;
            this->user = ProcessParser::get_proc_user(pid);
            this->mem = ProcessParser::get_vm_size(pid);
            this->cmd = ProcessParser::get_cmd(pid);
            this->up_time = ProcessParser::get_proc_up_time(pid);
            this->cpu  = ProcessParser::get_cpu_percent(pid);
        }
        void set_pid(int pid);
        string get_pid() const;
        string get_user() const;
        string get_cmd() const;
        int get_cpu() const;
        int get_mem() const;
        string get_up_time() const;
        string get_process();
};

void Process::set_pid(int pid)
{
    this->pid = pid;
}
string Process::get_pid() const 
{
    return this->pid;
}
string Process::get_process()
{
    this->mem = ProcessParser::get_vm_size(this->pid);
    this->up_time = ProcessParser::get_proc_up_time(this->pid);
    this->cpu = ProcessParser::get_cpu_percent(this->pid);

    return (this->pid + "   "
                    + this->user
                    + "   "
                    + this->mem.substr(0,5)
                    + "     "
                    + this->cpu.substr(0,5)
                    + "     "
                    + this->up_time.substr(0,5)
                    + "    "
                    + this->cmd.substr(0,30)
                    + "...");
}

