#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "signals.h"
#include "Commands.h"


using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    cout << "smash: got ctrl-Z" << endl;
    JobsList* jobs_ptr =SmallShell::getInstance().getJobsList();
    const char* ctrlZ_cmd_line = SmallShell::getInstance().getCurCmdLine();
    Command* cur_cmd = SmallShell::getInstance().getCurCmd();
    if( SmallShell::getInstance().getFgPid() != -1) {// there is a process in the fg
        if (kill(getpid(), SIGSTOP) == -1) {
            perror("smash error: kill failed");
            return;
        }
        jobs_ptr->addJob(cur_cmd, true); // add a stopped job to the jobslist
        cout << "smash: process " << SmallShell::getInstance().getFgPid()
                << " was stopped" << endl;

    }


}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    cout << "smash: got ctrl-Z" << endl;
    if( SmallShell::getInstance().getFgPid() != -1) {// there is a process in the fg
        if (kill(getpid(), SIGINT) == -1) {
            perror("smash error: kill failed");
            return;
        }
    }
}
