#ifndef MESSAGE_DATA
#define MESSAGE_DATA

#include "string"   
#include <chrono>
#include <time.h>

using namespace std;

int timeToWait = 1000 * 100 * 3;

int timeout = 2;

long long now(){
    return chrono::steady_clock::now().time_since_epoch().count();
}


enum MessageType {
    CREATE,
    PING_ALL,
    TIMER_GET,
    TIMER_START,
    TIMER_STOP,
    GET_DEPTH,
    TRASH
};

enum ForwardType{
    FEEDBACK,
    QUESTION
};

struct MessageData{
    int needId;

    int messageNumber;

    int initialPort;

    long long result;

    int bestDepth;

    int newID;

    int where;

    MessageType msgType;

    ForwardType type;
};

#endif // MESSAGE_DATA