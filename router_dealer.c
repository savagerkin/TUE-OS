/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Jing Hao Chen (1849115)
 * Tan Aras (1987321)
 * Erkin Uyguroglu (1799932)
 *
 * Grading:
 * Your work will be evaluated based on the following criteria:
 * - Satisfaction of all the specifications
 * - Correctness of the program
 * - Coding style
 * - Report quality
 * - Deadlock analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>    // for execlp
#include <mqueue.h>    // for mq
#include "settings.h"  
#include "messages.h"
#include <signal.h>
#include <time.h>

#define GROUP_NUMBER "29"

//#define ASSERT(_e, ...) if (_e) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define ASSERT(_e, ...) if (_e) {}

char client2dealer_name[30];
char dealer2worker1_name[30];
char dealer2worker2_name[30];
char worker2dealer_name[30];

int main (int argc, char * argv[]) {
    if (argc != 1) {fprintf (stderr, "%s: invalid arguments\n", argv[0]);}
  
    // TODO:
    //  * create the message queues (see message_queue_test() in
    //    interprocess_basic.c)
    //  * create the child processes (see process_test() and
    //    message_queue_test())
    //  * read requests from the Req queue and transfer them to the workers
    //    with the Sx queues
    //  * read answers from workers in the Rep queue and print them
    //  * wait until the client has been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // Important notice: make sure that the names of the message queues
    // contain your goup number (to ensure uniqueness during testing)

    // Create the names of channels
    sprintf (client2dealer_name, "/Req_queue_%s_%d", GROUP_NUMBER, getpid());
    sprintf (dealer2worker1_name, "/S1_queue_%s_%d", GROUP_NUMBER, getpid());
    sprintf (dealer2worker2_name, "/S2_queue_%s_%d", GROUP_NUMBER, getpid());
    sprintf (worker2dealer_name, "/Rsp_queue_%s_%d", GROUP_NUMBER, getpid());
    mqd_t mq_fd_request, mq_fd_s1, mq_fd_s2, mq_fd_response;
    MQ_REQUEST_MESSAGE req, s1, s2;
    MQ_RESPONSE_MESSAGE rsp;

    struct mq_attr attr;
    attr.mq_maxmsg  = MQ_MAX_MESSAGES;

    fprintf(stderr,"%s: Creating message queues...\n",BLUE("Router"));
    // Create the client to dealer queue
    attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
    mq_fd_request = mq_open (client2dealer_name, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, 0600, &attr);
    ASSERT(mq_fd_request == (mqd_t)-1, "Request queue creation failed: %s\n", strerror(errno));

    // Create the dealer to worker queues
    mq_fd_s1 = mq_open (dealer2worker1_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
    ASSERT(mq_fd_s1 == (mqd_t)-1, "Worker 1 queue creation failed: %s\n", strerror(errno));

    mq_fd_s2 = mq_open (dealer2worker2_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
    ASSERT(mq_fd_s2 == (mqd_t)-1, "Worker 2 queue creation failed: %s\n", strerror(errno));

    // Create the worker to dealer queue
    attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open (worker2dealer_name, O_RDWR | O_CREAT | O_NONBLOCK, 0600, &attr);
    ASSERT(mq_fd_response == (mqd_t)-1, "Response queue creation failed: %s\n", strerror(errno));

    fprintf(stderr,"%s: Creation of message queues finished...\n",BLUE("Router"));



    //STILL NEEDS TO INCLUDE N_SERV1 AND N_SERV2 TO CREATE MULTIPLE WORKERS
    pid_t pid_client = fork();
    ASSERT(pid_client == -1, "fork failed: %s\n", strerror(errno));  
    if (pid_client == 0){
        fprintf(stderr,"%s: Forked Client\n",BLUE("Router"));
        execlp("./client", client2dealer_name, NULL);
        ASSERT(true, "execlp failed: %s\n", strerror(errno));
    }

    // pid_t pid_worker_s1 = fork();
    // ASSERT(pid_worker_s1 == -1, "fork failed: %s\n", strerror(errno));  
    // if (pid_worker_s1 == 0){
    //     fprintf(stderr,"%s: Forked worker s1\n",BLUE("Router"));
    //     execlp("./worker_s1", dealer2worker1_name, worker2dealer_name, NULL);
    //     ASSERT(true, "execlp failed: %s\n", strerror(errno));
    // }

    // pid_t pid_worker_s2 = fork();
    // ASSERT(pid_worker_s2 == -1, "fork failed: %s\n", strerror(errno));  
    // if (pid_worker_s2 == 0){
    //     fprintf(stderr,"%s: Forked worker s2\n",BLUE("Router"));
    //     execlp("./worker_s2", dealer2worker2_name, worker2dealer_name, NULL);
    //     ASSERT(true, "execlp failed: %s\n", strerror(errno));
    // }

    pid_t s1_pids[N_SERV1] = {0};
    for (int i = 1; i <= N_SERV1; i ++) {
        pid_t pid_worker_s1 = fork();
        s1_pids[i-1] = pid_worker_s1;
        ASSERT(pid_worker_s1 == -1, "fork failed: %s\n", strerror(errno));  
        if (pid_worker_s1 == 0){
            fprintf(stderr,"%s: Forked worker s1 number %d\n", BLUE("Router"), i);
            execlp("./worker_s1", dealer2worker1_name, worker2dealer_name, NULL);
            ASSERT(true, "execlp failed: %s\n", strerror(errno));
            }
    }

    pid_t s2_pids[N_SERV2] = {0};
    for (int i = 1; i <= N_SERV2; i ++) {
        pid_t pid_worker_s2 = fork();
        s2_pids[i-1] = pid_worker_s2;
        ASSERT(pid_worker_s2 == -1, "fork failed: %s\n", strerror(errno));  
        if (pid_worker_s2 == 0){
            fprintf(stderr,"%s: Forked worker s2 number %d\n", BLUE("Router"), i);
            execlp("./worker_s2", dealer2worker2_name, worker2dealer_name, NULL);
            ASSERT(true, "execlp failed: %s\n", strerror(errno));
        }
    }

    //waitpid(pid_client, NULL, 0);
    
    int request_total = 0;
    int request_done = 0;
    int client_status = 0;
    while(true) {
        //fprintf(stderr,"%s: Waiting for request from client...\n",BLUE("Router"));
        if(mq_receive(mq_fd_request, (char*)&req, sizeof(req), NULL) != -1){
            fprintf(stderr,"%s: Received request from client with ID=%d and Service %d\n",BLUE("Router"), req.request_id, req.service_id);
            request_total++;
            fprintf(stderr, "TESTING: number of total requests:%d\n",request_total);
            if(req.service_id == 1) {
               s1 = req;
                fprintf(stderr,"%s: Sending ID:%d to worker 1\n",BLUE("Router"),s1.request_id);
                ASSERT(mq_send(mq_fd_s1, (char*)&s1, sizeof(s1), 0) == -1, "%s: mq_send failed: %s\n",BLUE("Router"), strerror(errno));
                fprintf(stderr, "%s: Sending Complete!\n",BLUE("Router"));
            } else if(req.service_id == 2) {
                s2 = req;
                fprintf(stderr,"%s: Sending ID:%d to worker 2\n",BLUE("Router"),s2.request_id);
                ASSERT(mq_send(mq_fd_s2, (char*)&s2, sizeof(s2), 0) == -1, "%s: mq_send failed: %s\n",BLUE("Router"), strerror(errno));
                fprintf(stderr, "%s: Sending Complete!\n",BLUE("Router"));
            } else {
                fprintf(stderr,"Incorrect service id\n");
            }
        } else if (errno != EAGAIN){
            fprintf(stderr, "%s: mq_receive on Req_queue failed: %s\n", BLUE("Router"), strerror(errno));
        } else if (errno == EAGAIN && client_status == 0 && request_total != 0){  //If Req_queue is empty for the first time and atleast one requeste has been sent
            fprintf(stderr,"%s: Req_queue empty! All client request sent to workers!\n",BLUE("Router"));
            client_status = 1; //All requests from client are sent. Req_queue is empty
        }

        if(mq_receive(mq_fd_response, (char*)&rsp, sizeof(rsp), NULL) != -1){
            fprintf(stderr,"%s: Reponse received from worker. Request Finished! Request: ID=%d and Result=%d\n",BLUE("Router"), rsp.request_id, rsp.result);
            printf("%d -> %d\n",rsp.request_id, rsp.result);
            fflush(stdout);     // Used to prevent 
            request_done++;
            fprintf(stderr, "TESTING: request_done:%d, request_total:%d, client_status:%d\n",request_done, request_total, client_status);
            //fprintf(stderr, "TESTING: number of finished requests:%d\n",request_done);
        } else if (errno != EAGAIN){
            fprintf(stderr, "%s: mq_receive failed: %s\n", BLUE("Router"), strerror(errno));
        } else if (errno == EAGAIN && request_done == request_total && client_status == 1){  //If Rsp_queue is empty AND #requests finished equals #total requests
            fprintf(stderr,"%s: All request responses received from workers! Closing up...\n",BLUE("Router"));
            break;
            }
    }

    // Cleaning up
    fprintf(stderr,"%s: Terminating all children...\n",BLUE("Router"));

    // Terminate all children processes in the group
    fprintf(stderr, "%s: Sending SIGTERM to all child processes...\n", BLUE("Router"));
    for (int i = 0; i < N_SERV1; i++)
    {
        kill(s1_pids[i], SIGTERM);
    }
    for (int i = 0; i < N_SERV2; i++)
    {
        kill(s2_pids[i], SIGTERM);
    }

    // Wait for all child processes to terminate
    while (wait(NULL) > 0);  // Reap all child processes

    fprintf(stderr, "%s: All child processes terminated.\n", BLUE("Router"));

    fprintf(stderr,"%s: Closing all queues...\n",BLUE("Router"));
    mq_close(mq_fd_request);
    mq_close(mq_fd_s1);
    mq_close(mq_fd_s2);
    mq_close(mq_fd_response);
    
    mq_unlink(client2dealer_name);
    mq_unlink(dealer2worker1_name);
    mq_unlink(dealer2worker2_name);
    mq_unlink(worker2dealer_name);
    fprintf(stderr,"%s: Shutting down...\n",BLUE("Router"));
    return 0;
}
