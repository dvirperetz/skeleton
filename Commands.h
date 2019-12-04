#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)
#define MAXPATHLEN (4096)
#define MAX_HISTORY_SIZE (50)
#define COMMAND_LINE_MAX_SIZE (80) // etai
#define MIN_JOB_ID (1)


class Command {
// TODO: Add your data members
protected:
    const char* cmd;
    char* args[COMMAND_MAX_ARGS];
    bool isExternal;
public:
    explicit Command(const char* cmd_line);
    virtual ~Command() = default;
    virtual void execute() = 0;
    const char* getCmd(){return cmd;};
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line) : Command(cmd_line){};
    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line) : Command(cmd_line){ isExternal= true;};
    virtual ~ExternalCommand() = default;
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
    char** plastPwd;
public:
    ChangeDirCommand(const char* cmd_line, char** plastPwd) :
            BuiltInCommand(cmd_line), plastPwd(plastPwd) {};
    virtual ~ChangeDirCommand() = default;
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line) :
        BuiltInCommand(cmd_line){};
    virtual ~GetCurrDirCommand() = default;
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
    virtual ~ShowPidCommand() = default;
    void execute() override;
};
class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    JobsList* qJobs;
public:
    QuitCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), qJobs(jobs){};
    virtual ~QuitCommand() = default;
    void execute() override;
};

class CommandsHistory {
public:

    class CommandHistoryEntry {
        // TODO: Add your data members
        const char* cmd_text;
        unsigned int timestamp;
    public:
        CommandHistoryEntry(const char* cmd_txt, unsigned int count) : cmd_text(cmd_txt), timestamp(count + 1) {};
        ~CommandHistoryEntry() = default;
        const char* getCmdText(){return cmd_text;};
        unsigned int getTimeStamp(){return timestamp;};
        void incTimeStamp(){timestamp++;};

    };
    // TODO: Add your data member
    CommandHistoryEntry* history_list[MAX_HISTORY_SIZE];
    unsigned int cmd_count;
    CommandsHistory() : history_list(), cmd_count(0){};
    ~CommandsHistory() {
        for(int i=0; i< MAX_HISTORY_SIZE; ++i){
            delete history_list[i];
        }
    }
    void addRecord(const char* cmd_line);
    void printHistory();
};

class HistoryCommand : public BuiltInCommand {
    // TODO: Add your data members
    CommandsHistory* history;
public:
    HistoryCommand(const char* cmd_line, CommandsHistory* history)
            : BuiltInCommand(cmd_line), history(history){};
    virtual ~HistoryCommand() = default;
    void execute() override;
};

class JobsList {

public:
    class JobEntry {
        // TODO: Add your data members
        const char* cmd_text;
        bool isStopped;
        unsigned long created_time;
        pid_t pid;
        unsigned int job_id;
    public:
        JobEntry(const char* cmd_text, pid_t pid);
        void setJobID(unsigned int job_id){this->job_id=job_id;};
        unsigned int getJobID(){return this->job_id;};
        pid_t getPID(){return this->pid;};
        const char* getCMD(){return cmd_text;};
        unsigned long getCreatedTime(){return created_time;};
        bool getIsStopped(){return isStopped;};
        void setIsStopped(bool b){this->isStopped=b;};
        bool operator< (const JobEntry& job) const{
            return (job_id < job.job_id);
        }
    };
    // TODO: Add your data members
    unsigned int job_counter;
    std::vector<JobEntry*> job_list;
    JobsList() : job_counter(0), job_list() {};
    ~JobsList(){
      job_list.clear();
    };
    void addJob(Command* cmd,pid_t pid, int fg_job_id = 0, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    int getMaxId() const;
  //  bool sortByJobID( JobEntry* first, JobEntry* second);
};
class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
public:
    JobsCommand(const char* cmd_line, JobsList* jobs) :
            BuiltInCommand(cmd_line),
            jobs(jobs){};
    virtual ~JobsCommand() = default;
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
public:
    KillCommand(const char* cmd_line, JobsList* jobs) :
            BuiltInCommand(cmd_line), jobs(jobs) {};
    virtual ~KillCommand() = default;
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* fgJobList;
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs) :
            BuiltInCommand(cmd_line), fgJobList(jobs){} ;
    virtual ~ForegroundCommand() = default;
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* bgJobList;
public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs) :
            BuiltInCommand(cmd_line), bgJobList(jobs) {};
    virtual ~BackgroundCommand() = default;
    void execute() override;
};

class CopyCommand : public BuiltInCommand {
public:
    CopyCommand(const char* cmd_line) :
            BuiltInCommand(cmd_line){};
    virtual ~CopyCommand() = default;
    void execute() override;
};



class SmallShell {
private:
    // TODO: Add your data members
    char* last_path;
    CommandsHistory* history;
    JobsList* jobs;
    pid_t fg_pid;
    Command* cur_cmd;
    int curr_fg_job_id;
    SmallShell();
public:
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
    JobsList* getJobsList();
    const char* getCurCmdLine();
    Command* getCurCmd();
    pid_t getFgPid(){return this->fg_pid;};
    void SetFgPid(pid_t pid){this->fg_pid = pid;};
    void setLastPath(char* path);
    char* getLastPath(){return last_path;};
    void setFgJobId(int id){this->curr_fg_job_id = id;};
    void SetCurCmd ( Command * new_cur_cmd){this->cur_cmd = new_cur_cmd;};
    int getCurFgJobId(){ return this->curr_fg_job_id;};
};

#endif //SMASH_COMMAND_H_


