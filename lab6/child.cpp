#include "iostream"
#include "string"
#include "unistd.h"
#include "./message_data.h"
#include <sys/wait.h>
#include "memory.h"
#include <zmq.h>
#include "chrono"
#include <random>
#include "thread"
#include "mutex"

using namespace std;

int parent;
int pd;
int id;

int leftPid = -1;
int rightPid = -1;

long long timer;
bool isTimerActive;
long long lastActive;

mutex mtx;

 void stop(){
        if(isTimerActive){
            timer += now() - lastActive;
        }
        lastActive = now();
        isTimerActive = false;
    }

void start(){
    if(isTimerActive)return;
    isTimerActive = true;
    lastActive = now();
}

void update(){
    if(isTimerActive){
        timer += now() - lastActive;
    }
    lastActive = now();
}

void sendMessage(MessageData md, int where = -1){
    mtx.lock();
    if(md.type == FEEDBACK){
        cout << id << endl;
        if(parent == -1){
            void* context = zmq_ctx_new();
            void* senderSocker = zmq_socket(context, ZMQ_PUSH);
            
            string senderTCP = "tcp://127.0.0.1:" + to_string(md.initialPort);
            
            zmq_bind(senderSocker, senderTCP.c_str());

            usleep(timeToWait);

            zmq_msg_t message;

            usleep(timeToWait);
            zmq_msg_init_size(&message, sizeof(MessageData));

            memcpy(zmq_msg_data(&message), &md, sizeof(MessageData));

            zmq_msg_send(&message, senderSocker, ZMQ_DONTWAIT);
            zmq_close(senderSocker);
            zmq_ctx_destroy(context);
        }
        else{
            void * context = zmq_ctx_new();
            void* senderSocker = zmq_socket(context, ZMQ_PUSH);
            
            string senderTCP = "tcp://127.0.0.1:" + to_string(parent); 

            zmq_bind(senderSocker, senderTCP.c_str());

            usleep(timeToWait);

            zmq_msg_t message;

            zmq_msg_init_size(&message, sizeof(MessageData));

            memcpy(zmq_msg_data(&message), &md, sizeof(MessageData));

            zmq_msg_send(&message, senderSocker, ZMQ_DONTWAIT);

            zmq_close(senderSocker);
            zmq_ctx_destroy(context);
        }
    }
    else{
        void * context = zmq_ctx_new();
        void* senderSocker = zmq_socket(context, ZMQ_PUSH);
            
        string senderTCP = "tcp://127.0.0.1:" + to_string(where); 

        zmq_bind(senderSocker, senderTCP.c_str());

        usleep(timeToWait);

        zmq_msg_t message;

        zmq_msg_init_size(&message, sizeof(MessageData));

        memcpy(zmq_msg_data(&message), &md, sizeof(MessageData));

        zmq_msg_send(&message, senderSocker, ZMQ_DONTWAIT);

        zmq_close(senderSocker);
        zmq_ctx_destroy(context);
    }
    mtx.unlock();
}

int main(int argc, char *argv[]){
    void * context = zmq_ctx_new();
    void * senderSocket = zmq_socket(context, ZMQ_PULL);

    lastActive = now();

    pd = getpid() * 10;
    parent = stoi(argv[1]);
    
    id = stoi(argv[2]);

    string senderTCP = "tcp://127.0.0.1:" + to_string(pd);

    zmq_connect(senderSocket, senderTCP.c_str());

    senderTCP = "tcp://127.0.0.1:" + to_string(pd + 1);

    zmq_connect(senderSocket, senderTCP.c_str());

    senderTCP = "tcp://127.0.0.1:" + to_string(pd + 2);

    zmq_connect(senderSocket, senderTCP.c_str());
    
    usleep(timeToWait);

    while(1){
        zmq_msg_t msg;

        zmq_msg_init(&msg);
        int status = zmq_msg_recv(&msg, senderSocket, ZMQ_DONTWAIT);
        if(status < 0){
            continue;
        }
        MessageData *m = (MessageData *) zmq_msg_data(&msg);
        
        MessageData md = *m;

        zmq_msg_close(&msg);
        if((md.needId == id || md.msgType == GET_DEPTH || md.msgType == PING_ALL) && md.type == QUESTION){
            if(md.msgType == CREATE){
                int pid;
                if(leftPid == -1){
                    pid = fork();
                    if(!pid){
                        execl("./child", "./child", to_string(pd + 1).c_str(), to_string(m->newID).c_str(), NULL);
                        cout << "Bad fork!\n";
                    }
                    leftPid = pid;
                }
                else{
                    pid = fork();
                    if(!pid){
                        execl("./child", "./child", to_string(pd + 2).c_str(), to_string(m->newID).c_str(), NULL);
                        cout << "Bad fork!\n";
                    }
                    rightPid = pid;
                }
                md.result = pid;
            }
            else if(md.msgType == TIMER_START){
                start();
            }
            else if(md.msgType == TIMER_STOP){
                stop();
            }
            else if(md.msgType == TIMER_GET){
                update();
                md.result = timer / (long long)(1e6);
            }

            bool ok = true;

            if(md.msgType == GET_DEPTH){
                md.result++; 
                if(leftPid != -1 && rightPid != -1){
                    ok = false;
                }
            }
            if(ok){
            MessageData mdCopy = md;

            mdCopy.type = FEEDBACK;
            mdCopy.where = id;

            thread th(sendMessage, mdCopy, -1);

            th.detach();}
        }
        if(md.needId != id){
            if(md.type == FEEDBACK){
                thread th(sendMessage, md, -1);

                th.detach();
            }
            else{
                if(leftPid != -1){
                    thread th(sendMessage, md, leftPid * 10);

                    th.detach();
                }
                if(rightPid != -1){
                    thread th(sendMessage, md, rightPid * 10);

                    th.detach();
                }
            }
        }
    }

    zmq_close(senderSocket);
	zmq_ctx_destroy(context);
    return 0;
}