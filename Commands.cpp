#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <algorithm>
#include "Commands.h"
#include "fcntl.h"


#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::right;
using std::setw;


const std::string WHITESPACE = " \n\r\t\f\v";
string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string whitespace = " \t\n";
    const string str(cmd_line);
    return str[str.find_last_not_of(whitespace)] == '&';
}

bool sortByJobID(class JobsList::JobEntry * first, class JobsList::JobEntry * second) {
    return (first->getJobID() < second->getJobID()) ;
}


void _removeBackgroundSign(char* cmd_line) {
    const string whitespace = " \t\n";
    const string str(cmd_line);
    // find last character other than spaces
    size_t idx = str.find_last_not_of(whitespace);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(whitespace, idx-1) + 1] = 0;
}


// TODO: Add your implementation for classes in Commands.h
Command::Command(const char* cmd_line) : cmd(cmd_line), args(){
    _parseCommandLine(cmd_line, args);
    isExternal=false;
}

void GetCurrDirCommand::execute() {
    char buf[MAXPATHLEN];
    char* path = getcwd(buf, MAXPATHLEN);
    string current = path;
    cout << current << endl;
}

void ChangeDirCommand::execute() {
    char buf[MAXPATHLEN];
    getcwd(buf, MAXPATHLEN);
    char* current_path = buf;

    if(this->args[2] != nullptr){
        cout << "smash error: cd: too many arguments" << endl;
    }
    // cd -
    else if(strcmp(this->args[1], "-") == 0){
        if((*plastPwd) == nullptr){
            cout << "smash error: cd: OLDPWD not set" << endl;
        } else{
            if(chdir(*plastPwd) == -1){
                perror("smash error: chdir failed");
                return;
            }
            SmallShell::getInstance().setLastPath(current_path);
        }
    }
    else if(strcmp(this->args[1],"..") == 0){
        // cd ..
        char cwd[MAXPATHLEN];
        getcwd(cwd, sizeof(cwd));
        string pPath = cwd;
        pPath = pPath.substr(0,pPath.find_last_of('/'));
        if(chdir(pPath.c_str()) == -1){
            perror("smash error: chdir failed");
            return;
        }
        SmallShell::getInstance().setLastPath(current_path);
    } else{
        // cd <PATH>
        if(chdir(args[1]) == -1){
            perror("smash error: chdir failed");
            return;
        }
        SmallShell::getInstance().setLastPath(current_path);
    }
}

void CommandsHistory::addRecord(const char* cmd_line){
    CommandHistoryEntry* last_entry = history_list[(cmd_count-1)%50];
    if(last_entry && strcmp(cmd_line, last_entry->getCmdText()) == 0){
        last_entry->incTimeStamp();
        return;
    }
    char* temp = (char*) malloc(strlen(cmd_line) + 1);
    strcpy(temp, cmd_line);
    int new_timestamp = last_entry ? last_entry->getTimeStamp() : 0;
    CommandHistoryEntry* entry = new CommandHistoryEntry(temp, new_timestamp);
    cmd_count++;
    free(this->history_list[(cmd_count-1)%50]);
    this->history_list[(cmd_count-1)%50] = entry;
}

void CommandsHistory::printHistory() {
    if(cmd_count <= MAX_HISTORY_SIZE){
        for(unsigned int i = 0; i < cmd_count; i++){
            //  don't print duplicates
            /*if(i < (MAX_HISTORY_SIZE-1) && history_list[(i+1)] && !strcmp(history_list[i]->getCmdText(),history_list[(i+1)]->getCmdText())){
                continue;
            }*/

            cout << right << setw(5) << history_list[i]->getTimeStamp()
                 << "  " << history_list[i]->getCmdText()  << endl;
        }
    } else{
        int pivot = cmd_count%MAX_HISTORY_SIZE;
        for(int i = pivot; i < MAX_HISTORY_SIZE; i++){
            /*if(i < (MAX_HISTORY_SIZE) && history_list[(i+1)%MAX_HISTORY_SIZE] && !strcmp(history_list[i]->getCmdText(),history_list[(i+1)%MAX_HISTORY_SIZE]->getCmdText())){
                continue;
            }*/
            cout << right << setw(5) << history_list[i]->getTimeStamp()
                 << "  " << history_list[i]->getCmdText() << endl;
        }
        for(int i = 0; i < pivot; i++){
            /*if(i < (MAX_HISTORY_SIZE-1) && history_list[i+1] && !strcmp(history_list[i]->getCmdText(),history_list[i+1]->getCmdText())){
                continue;
            }*/
            cout << right << setw(5) << history_list[i]->getTimeStamp()
                 << "  " << history_list[i]->getCmdText() << endl;
        }
    }

}

void HistoryCommand::execute() {
    this->history->printHistory();
}

void JobsCommand::execute() {
    this->jobs->printJobsList();
}

int JobsList::getMaxId() const {
    int max = 0;
    for (auto it = job_list.begin(); it != job_list.end(); it++){
        if((int)(*it)->getJobID() > max){
            max = (*it)->getJobID();
        }
    }
    return max;
}

void JobsList::addJob(Command* cmd, pid_t pid,int fg_job_id, bool isStopped){
    char* temp = (char*) malloc(strlen(cmd->getCmd()) + 1);
    strcpy(temp, cmd->getCmd());
    JobEntry* new_job = new JobEntry(temp, pid);
    new_job->setIsStopped(isStopped);
    removeFinishedJobs();
    job_counter = getMaxId();
    if( fg_job_id == 0){ // case : its a new job, so assign max id+1
        new_job->setJobID(++job_counter);
        } else{ // case : its an old job so we recover its id
        new_job->setJobID(fg_job_id);
        if( fg_job_id > (int) job_counter){
            job_counter = fg_job_id;
        }
    }
    this->job_list.push_back(new_job);
}

void JobsList::removeFinishedJobs() {
    for (auto it = job_list.begin(); it != job_list.end();) {

        int cur_pid = (*it)->getPID();

        if (waitpid(cur_pid,NULL, WNOHANG)) {
            job_list.erase(it);
        } else {
            it++;
        }
    }
}


void JobsList::printJobsList(){
    removeFinishedJobs();
    std::sort(job_list.begin(), job_list.end(),sortByJobID);
    for(auto & it : this->job_list) {
        // the printing:
        // the printing:
        if(!it->getIsStopped()){
            cout << "[" << it->getJobID() << "] " << it->getCMD() << " : " <<
                 it->getPID() << " " <<
                 (difftime(time(nullptr), it->getCreatedTime() )) <<
                 " secs" << endl;
        }else{
            cout << "[" << it->getJobID() << "] " << it->getCMD() << " : " <<
                 it->getPID() << " " <<
                 (difftime(time(nullptr), it->getCreatedTime() )) <<
                 " secs (stopped)" << endl;
        }
    }
}

static bool is_number(const char* c){
    for(unsigned int i = 0; i < strlen(c); i++){
        if(!isdigit(c[i])){
            return false;
        }
    }
    return true;
}

void KillCommand::execute() {
    //  make sure the kill command comes with exactly 2 arguments, no less
    //  and no more.
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    if(this->args[1] == nullptr || this->args[2] == nullptr ||
       this->args[3] != nullptr || !(is_number(this->args[2]))){
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    int job_id = strtol(this->args[2], nullptr,0);
    if(jobs->getJobById(job_id) == nullptr){
        cout << "smash error: kill: job-id " << this->args[2]
             << " does not exist" << endl;
    } else{
        int signal = abs(strtol(this->args[1], nullptr,0));
        pid_t pid = jobs->getJobById(job_id)->getPID();

        if(kill(pid,signal) == -1 ){
            //  at kill failure
            perror("smash error: kill failed");
        } else{
            //  at kill success
            cout << "signal number " << signal << " was sent to pid " <<
                 pid << endl;
        }
    }
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

void ForegroundCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    int job_id = 0;
    if(args[1] == nullptr && fgJobList->job_list.empty()){
        cout << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    else if (args[2] != nullptr || (args[1] && strtol(args[1], nullptr,0) < MIN_JOB_ID)){
        cout << "smash error: fg: invalid arguments" << endl;
        return;
    }
    else if(args[1] && fgJobList->getJobById(strtol(args[1], nullptr,0)) == nullptr){
        cout << "smash error: fg: job-id " << strtol(args[1], nullptr,0) <<
             " does not exist" << endl;
        return;
    }
    else if (args[1] != nullptr){ // case: we got and jobid with the fg command
        job_id = strtol(args[1], nullptr,0);
        cout << fgJobList->getJobById(job_id)->getCMD() << " : " <<
             fgJobList->getJobById(job_id)->getPID() << endl;
    }
    else{ // case: no args (and the joblist isnt empty)
        cout << fgJobList->getLastJob(nullptr)->getCMD() << " : " <<
             fgJobList->getLastJob(nullptr)->getPID() << endl;
        job_id = fgJobList->getLastJob(nullptr)->getJobID();
    }
    Command* new_cmd = SmallShell::getInstance().CreateCommand(fgJobList->getJobById(job_id)->getCMD());
    SmallShell::getInstance().SetCurCmd(new_cmd);
    SmallShell::getInstance().setFgJobId(job_id);
    if(kill(fgJobList->getJobById(job_id)->getPID(), SIGCONT) == -1){ // send signal to move the job to run in the fg
        perror("smash error: kill failed");
    }
    int child_pid = fgJobList->getJobById(job_id)->getPID();
    SmallShell::getInstance().SetFgPid(child_pid);
    int w = waitpid(child_pid, nullptr, WUNTRACED);
    if (w == -1){
        perror("smash error: waitpid failed");
    }
    fgJobList->removeJobById(job_id);
}

void JobsList::removeJobById(int jobId){
    int index;
    for (vector<JobEntry*>::iterator it = this->job_list.begin(); it != this->job_list.end() ; it++){
        if ( (int) (*it)->getJobID() == jobId ){
            index = std::distance(this->job_list.begin(), it);
            this->job_list.erase(this->job_list.begin()+index);
            return;
        }
    }
}

JobsList::JobEntry::JobEntry(const char* cmd_text, pid_t pid) :
        cmd_text(cmd_text), isStopped(false) , pid(pid){
    created_time= time(nullptr);
};

JobsList::JobEntry* JobsList::getJobById(int jobId){
    for(vector<JobEntry*>::iterator it = job_list.begin(); it != job_list.end();
    ++it){
        if((int) (*it)->getJobID() == jobId){
            return (*it);
        }
    }
    return nullptr;
}

JobsList::JobEntry* JobsList::getLastJob(int *lastJobId) {
    if(lastJobId != nullptr){
        *lastJobId = this->job_list.back()->getJobID();
    }
    return  this->job_list.back();
}

void BackgroundCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    int job_id;
    if(args[1] == nullptr && (bgJobList->getLastStoppedJob(nullptr) == nullptr)){
        cout << "smash error: bg: there is no stopped jobs to resume" << endl;
        return;
    }
    else if (args[2] != nullptr || (args[1] && strtol(args[1], nullptr,0)== 0)){
        cout << "smash error: bg: invalid arguments" << endl;
        return;
    }
    else if(args[1] && bgJobList->getJobById(strtol(args[1], nullptr,0)) == nullptr){
        cout << "smash error: bg: job-id " << strtol(args[1], nullptr,0) <<
             " does not exist" << endl;
        return;
    }
    else if (args[1] && bgJobList->getJobById(strtol(args[1], nullptr,0))->getIsStopped() == 0 ) {// case: job exist but not stopped
        cout << "smash error: bg: job-id " << strtol(args[1], nullptr,0) <<
             " is already running in the background" << endl;
        return;
    }
    else if ( args[1] == nullptr ){ // case : execute on the last stopped job
        job_id = bgJobList->getLastStoppedJob(nullptr)->getJobID();
    }
    else{
        job_id = strtol(args[1], nullptr,0); // case : execute on the <job id> stopped job
    }
    cout <<  bgJobList->getJobById(job_id)->getCMD() << " : " <<
         bgJobList->getJobById(job_id)->getPID() << endl;
    kill(bgJobList->getJobById(job_id)->getPID(), 18); // send signal SIGCONT
    bgJobList->getJobById(job_id)->setIsStopped(false);
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId) {
    for (vector<JobEntry*>::reverse_iterator i = job_list.rbegin(); i!= job_list.rend(); ++i){
        if ( (*i) && (*i)->getIsStopped() ){
            if( jobId!= nullptr){
                *jobId = (*i)->getJobID();
            }
            return  *i;
        }
    }
    return nullptr; // no stopped job in the list
}

void QuitCommand::execute() {
    SmallShell::getInstance().getJobsList()->removeFinishedJobs();
    if(args[1] && strcmp(args[1], "kill") == 0){
        cout << "smash: sending SIGKILL signal to " << qJobs->job_list.size() << " jobs:" << endl;
        for(vector<JobsList::JobEntry*>::iterator it = qJobs->job_list.begin(); it != qJobs->job_list.end(); it++){
            cout << (*it)->getPID() << ": " << (*it)->getCMD() << endl;
        }
        qJobs->killAllJobs();
    }
    exit(0);
}


void JobsList::killAllJobs() {
    for(vector<JobsList::JobEntry*>::iterator it = job_list.begin(); it != job_list.end(); it++){
        kill((*it)->getPID(), SIGKILL);
    }
}

void CopyCommand::execute() {
    char buffer[MAXPATHLEN];
    int source, dest;
    ssize_t count;

    source = open(args[1], O_RDONLY);
    if(source == -1){
        perror("smash error: open failed");
        return;
    }
    dest = open(args[2], O_WRONLY | O_CREAT , 0644); // mode -rw-r---r--
    if(dest == -1){
        close(source);
        perror("smash error: open failed");
        return;
    }
    cout << "smash: " << args[1] << " was copied to " << args[2] << endl;
    while ((count = read(source, buffer, sizeof(buffer))) != 0){
        write(dest, buffer, count);
    }
}
/**
 * A function that checks whether a command is a simple one(return false) or complex one(true)
 */
bool CheckIfComplex ( const char* cmd){
    string cmd_s = string(cmd);
    if(cmd_s.find("?") == 0 || cmd_s.find("*") == 0) {
        return true;
    }
    return false;
}

void ExternalCommand::execute() {
    pid_t pid = fork();
    if(pid == -1) { // fork failed
        perror("smash error: fork failed");
    }
    if( pid == 0) { // child process ( we will execv from it )
        setpgrp(); //   changes the son's group pid to its own
        SmallShell::getInstance().SetFgPid(getpid()); // updating the smash member : Foreground pid

        std::string str = string(cmd);
        char * writable = new char[str.size() + 1];
        std::copy(str.begin(), str.end(), writable);
        writable[str.size()] = '\0';
        _removeBackgroundSign(writable);
        char *extern_args[4] = {(char*) "/bin/bash", (char*) "-c", writable, nullptr};
        if (execv("/bin/bash", extern_args) == -1 ){
            perror("smash error: execv failed");
        }

        /*
        else { //  the command is simple ; execute in the smash
            string cmd_s = string(this->cmd);
            string first_word = cmd_s.substr(0, cmd_s.find(" "));
            const char *execFileName = first_word.c_str();  // getting the command to execute
            char *simple_args[sizeof(this->args) + 1];  // creating an array for the args (for execv syscall)
            simple_args[0] = const_cast<char*>(execFileName); // first arg is the fileName
            for (int i = 1; i < sizeof(simple_args); i++) {
                simple_args[i] = this->args[i - 1];
            }
            if (execvp(execFileName, simple_args) ==
                -1) { // execvp : search for executable file according to the external command
                perror("smash error: execvp failed");
            }
        }*/
    }
    else{ // parent process : the smash
        int status;
        if( !_isBackgroundComamnd(this->getCmd())){
            SmallShell::getInstance().SetFgPid(pid); // updating the smash member : Foreground pid
            waitpid(pid,&status,WUNTRACED);
        } else{ //it's a background command
            SmallShell::getInstance().getJobsList()->addJob(this,pid, 0,false);
        }
        return;
    }
}


JobsList* SmallShell::getJobsList() {
    return this->jobs;
}
const char* SmallShell::getCurCmdLine() {
    return this->cur_cmd->getCmd();
}
Command* SmallShell::getCurCmd() {
    return this->cur_cmd;
}


SmallShell::SmallShell() :
    last_path(nullptr), history(new CommandsHistory()),
    jobs(new JobsList()), fg_pid(-1), cur_cmd(nullptr) , curr_fg_job_id(0){}


SmallShell::~SmallShell() {
    delete history;
    delete jobs;
}

void SmallShell::setLastPath(char* path){
    if(last_path != nullptr){
        free(last_path);
    }
    char* temp = (char*) malloc(strlen(path)+1);
    strcpy(temp, path);
    last_path = temp;
}

Command * SmallShell::CreateCommand(const char* cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    _parseCommandLine(cmd_line,args);
    if (strcmp(args[0], "pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (strcmp(args[0], "cd") == 0){
        return new ChangeDirCommand(cmd_line, &last_path);
    }
    else if (strcmp(args[0], "history") == 0){
        return new HistoryCommand(cmd_line, history);
    }
    else if(strcmp(args[0], "jobs") == 0){
        return new JobsCommand(cmd_line, jobs);
    }
    else if (strcmp(args[0], "kill") == 0){
        return new KillCommand(cmd_line, jobs);
    }
    else if (strcmp(args[0], "showpid") == 0){
        return new ShowPidCommand(cmd_line);
    }
    else if (strcmp(args[0], "fg") == 0){
        return new ForegroundCommand(cmd_line, jobs);
    }
    else if (strcmp(args[0], "bg") == 0){
        return new BackgroundCommand(cmd_line, jobs);
    }
    else if (strcmp(args[0], "quit") == 0){
        return new QuitCommand(cmd_line, jobs);
    }
    else if (strcmp(args[0], "cp") == 0){
        return new CopyCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_line);
    }
    return nullptr;
}



void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    history->addRecord(cmd_line);
    string cmd_s = (string)cmd_line; // etai :





    //
    if( (int) cmd_s.find("|") > 0 ){
      //  string cmd_s = (string)cmd_line; // etai :
        string first_cmd_line_s= cmd_s.substr(0,cmd_s.find("|",0));
        const char* first_cmd_line = first_cmd_line_s.c_str();
        string second_cmd_s = cmd_s.substr(cmd_s.find("|") + 1 );
        cout << " first cmd is :\"" << first_cmd_line_s  << "\"" << endl;
        cout << " second cmd is :\"" << second_cmd_s << "\"" <<endl;
        const char* second_cmd_line = second_cmd_s.c_str();
        Command* first_cmd = CreateCommand(first_cmd_line);
        Command* second_cmd = CreateCommand(second_cmd_line);
        int my_pipe[2];
        pipe ( my_pipe );
        int pid;
        pid = fork();
        if ( pid == 0){
            dup2(my_pipe[1],1);
            this->cur_cmd = first_cmd;
            first_cmd->execute();
            abort();
        } else{
            dup2(my_pipe[0],0);
            close(my_pipe[1]);
            this->cur_cmd = second_cmd;
            second_cmd->execute();
            this->fg_pid = -1;
            this->curr_fg_job_id = 0;
            this->jobs->removeFinishedJobs();
        }
    }else {
        Command *cmd = CreateCommand(cmd_line); // example
        this->cur_cmd = cmd;
        cmd->execute();
        this->fg_pid = -1;
        this->curr_fg_job_id = 0;
        this->jobs->removeFinishedJobs();
    }
}
// Please note that you must fork smash process for some commands (e.g., external commands....)

