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

#ifndef MESSAGES_H
#define MESSAGES_H

// Message structure for requests sent to the router-dealer
typedef struct {
    int request_id;
    int service_id;
    int data;
} MQ_REQUEST_MESSAGE;

// Message structure for responses from workers
typedef struct {
    int request_id;
    int result;
} MQ_RESPONSE_MESSAGE;

#endif
