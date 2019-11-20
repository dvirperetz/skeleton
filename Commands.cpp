#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    stringstream check1(cmd_line);
    string intermediate;
    int i = 0;
    while(getline(check1, intermediate, ' '))
    {
        args[i] = (char*)malloc(intermediate.length()+1);
        memset(args[i], 0, intermediate.length()+1);
        strcpy(args[i], intermediate.c_str());
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

void GetCurrDirCommand::execute() {
    char buf[MAXPATHLEN];
    char* path = getcwd(buf, MAXPATHLEN);
    string current = path;
    cout << current << endl;
}

void ChangeDirCommand::execute() {
    if(this->args[2] != nullptr){
        cout << "smash error: cd: too many arguments" << endl;
    }
    else if(strcmp(this->args[1], "-") == 0){
        if(!plastPwd){
            cout << "smash error: cd: OLDPWD not set" << endl;
        } else{
            if(chdir(*plastPwd) == -1){
                perror("smash error: chdir failed");
            }
        }
    }
    else if(strcmp(this->args[1],"..") == 0){
        char cwd[MAXPATHLEN];
        getcwd(cwd, sizeof(cwd));

        string pPath = cwd;
        pPath = pPath.substr(0,pPath.find_last_of('/'));

        if(chdir(pPath.c_str()) == -1){
            perror("smash error: chdir failed");
        }
    } else{
        if(chdir(args[1]) == -1){
            perror("smash error: chdir failed");
        }
    }
}

void CommandsHistory::addRecord(const char* cmd_line){
    CommandHistoryEntry* entry = new CommandHistoryEntry(cmd_line);
    free(this->history_list[(cmd_count-1)%50]);
    this->history_list[(cmd_count-1)%50] = entry;
}

void CommandsHistory::printHistory() {
    if(cmd_count <= 50){
        for(int i = 0; i < cmd_count; i++){
            cout << right << setw(5) << history_list[i]->getTimeStamp()
            << " " << history_list[i]->getCmdText() << endl;
        }
    } else{
        int pivot = cmd_count%50;
        for(int i = pivot; i < MAX_HISTORY_SIZE; i++){
          cout << right << setw(5) << history_list[i]->getTimeStamp()
                 << " " << history_list[i]->getCmdText() << endl;
        }
        for(int i = 0; i < pivot; i++){
            cout << right << setw(5) << history_list[i]->getTimeStamp()
                 << " " << history_list[i]->getCmdText() << endl;
        }
    }

}

void HistoryCommand::execute() {
    this->history->printHistory();
}

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:
/*
  string cmd_s = string(cmd_line);
  if (cmd_s.find("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

