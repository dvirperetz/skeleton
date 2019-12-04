#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "signals.h"
#include "Commands.h"


using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    JobsList* jobs_ptr = SmallShell::getInstance().getJobsList();
    Command* cur_cmd = SmallShell::getInstance().getCurCmd();
    int fg_job_pid = SmallShell::getInstance().getFgPid();
    if( fg_job_pid != -1) {// there is a process in the fg
        jobs_ptr->addJob(cur_cmd,fg_job_pid,SmallShell::getInstance().getCurFgJobId() ,true ); // add a stopped job to the jobslist
        if (kill(fg_job_pid, 19) == -1) {
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " << fg_job_pid
                << " was stopped" << endl;
    }

}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    cout << "smash: got ctrl-C" << endl;
    if( SmallShell::getInstance().getFgPid() != -1) {// there is a process in the fg
        if (kill(SmallShell::getInstance().getFgPid(), SIGINT) == -1) {
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " << SmallShell::getInstance().getFgPid() << " was killed" << endl;
    }
}
