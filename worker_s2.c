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
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()

#include "messages.h"
#include "settings.h"
#include "service1.h"

#define GROUP_NUMBER "29"

#define ASSERT(_e, ...) if (_e) { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }
static void rsleep (int t);

static char mq_name1[80];
static char mq_name2[80];    

int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the
    //    arguments)
    //  * repeatedly:
    //      - read from the S1 message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do the job 
    //      - write the results to the Rsp message queue
    //    until there are no more tasks to do
    //  * close the message queues
    fprintf(stderr, "%s: Executing Code...\n", YELLOW_TEXT("Worker s2"));

    const char *s2_queue = argv[0];
    const char *rsp_queue = argv[1];

	sprintf (mq_name1, "/mq_request_%s", GROUP_NUMBER);
  	sprintf (mq_name2, "/mq_response_%s", GROUP_NUMBER);

    mqd_t mq_fd_s2 = mq_open(s2_queue, O_RDONLY);
    ASSERT(mq_fd_s2 == (mqd_t)-1, "Opening request queue failed: %s\n", strerror(errno));

    mqd_t mq_fd_rsp = mq_open(rsp_queue, O_WRONLY);
    ASSERT(mq_fd_rsp == (mqd_t)-1, "Opening response queue failed: %s\n", strerror(errno));

    fprintf(stderr, "%s: Finished Opening Queues!\n", YELLOW_TEXT("Worker s2"));

    MQ_REQUEST_MESSAGE s2;
    MQ_RESPONSE_MESSAGE rsp;

    fprintf(stderr, "%s: Waiting for requests...\n", YELLOW_TEXT("Worker s2"));
    while (true) {          // MIGHT STILL NEED TO EXIT THE WHILE LOOP WHEN ALL REQUESTS ARE DONE
        ASSERT(mq_receive(mq_fd_s2, (char*)&s2, sizeof(s2), NULL) == -1, "Router: mq_receive failed: %s\n", strerror(errno));
        fprintf(stderr, "%s: Received Request with ID=%d and data=%d\n", YELLOW_TEXT("Worker s2"),s2.request_id, s2.data);
        rsleep(10000);
        rsp.result = service(s2.data);
        rsp.request_id = s2.request_id;

        fprintf(stderr, "%s: Finished applying Service 1!\n", YELLOW_TEXT("Worker s2"));

        fprintf(stderr,"%s: Sending Response with ID=%d and result=%d\n", YELLOW_TEXT("Worker s2"), rsp.request_id, rsp.result);
        ASSERT(mq_send(mq_fd_rsp, (char*)&rsp, sizeof(rsp), 0) == -1, "mq_send failed: %s\n", strerror(errno));
    }

    perror("mq_receive failed");
    fprintf(stderr, "%s: Closing queues...\n", YELLOW_TEXT("Worker s2"));
    mq_close(mq_fd_s2);
    mq_close(mq_fd_rsp);
    fprintf(stderr, "%s: Finished closing queues!\n", YELLOW_TEXT("Worker s2"));
    return 0;
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}