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
#include "request.h"

#define GROUP_NUMBER "29"

#define ASSERT(_e, ...) if (_e) { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }
static char mq_name1[80];

int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the message queue (whose name is provided in the
    //    arguments)
    //  * repeatingly:
    //      - get the next job request 
    //      - send the request to the Req message queue
    //    until there are no more requests to send
    //  * close the message queue
    fprintf(stderr,"Client: Executing Client Code...\n");
    const char *req_queue = argv[0];

    sprintf (mq_name1, "/mq_request_%s", GROUP_NUMBER);

    mqd_t mq_fd_req = mq_open(req_queue, O_WRONLY);
    ASSERT(mq_fd_req == (mqd_t)-1, "Opening request queue failed: %s\n", strerror(errno));

    MQ_REQUEST_MESSAGE req;

    while(getNextRequest(&req.request_id, &req.data, &req.service_id) != -1) {
        fprintf(stderr,"Client: Sending request=%d\n", req.request_id);
        ASSERT(mq_send(mq_fd_req, (char*)&req, sizeof(req), 0) == -1, "mq_send failed: %s\n", strerror(errno));
    }
    fprintf(stderr, "Client: Closing queues...\n");
    mq_close(mq_fd_req);
    fprintf(stderr, "Client: Finished closing queues!\n");
    return (0);
}