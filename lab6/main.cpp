#include "iostream"
#include "string"
#include "unistd.h"
#include "./message_data.h"
#include <sys/wait.h>
#include "memory.h"
#include <zmq.h>
#include "map"
#include "chrono"
#include "node.h"
#include <random>
#include "thread"
#include "algorithm"
#include "mutex"

using namespace std;

map<int, int> doesExist;

int initialNode = -1;

mutex mtx;

char *getStringPid() {
    int id_pid = getpid();
    char *str_pid = const_cast<char *>(to_string(id_pid).c_str());

    return str_pid;
}

void sendMessage(MessageData md){
    if(md.msgType != CREATE && initialNode == -1){
        cout << "Error:" << md.needId << " can't be accessed!\n";
        return;
    }

    if(md.msgType == CREATE){
        mtx.lock();
    }
    if(md.msgType == CREATE && initialNode == -1){
        int pid = fork();
        if(!pid){
            execl("./child", "./child", "-1", to_string(md.newID).c_str(), NULL);
            cout << "Bad fork!\n";
        }
        doesExist[md.newID] = 1;
        initialNode = pid * 10;
        cout << "Ok: " << pid << endl;
        mtx.unlock();
        return;
    }

    map<int, int> have;


    md.initialPort = getpid() + md.messageNumber + 1000;
    md.result = -1;
    void* context = zmq_ctx_new();
    void* senderSocket = zmq_socket(context, ZMQ_PUSH);

    string senderTCP = "tcp://127.0.0.1:" + to_string(initialNode);
    zmq_bind(senderSocket, senderTCP.c_str());

    string objectTCP = "tcp://127.0.0.1:" + to_string(md.initialPort);

    void* objectSocket = zmq_socket(context, ZMQ_PULL);
    zmq_connect(objectSocket, objectTCP.c_str());

    usleep(timeToWait);

    if(md.msgType == CREATE){
        vector<pair<int,int>> bestDepth;
        md.needId = -1;
        MessageData MsgDepth;

        MsgDepth.msgType = GET_DEPTH;
        MsgDepth.type = QUESTION;
        MsgDepth.messageNumber = md.messageNumber;
        MsgDepth.initialPort = md.initialPort;
        
        zmq_msg_t msg;
        zmq_msg_init_size(&msg, sizeof(MessageData));
        memcpy(zmq_msg_data(&msg), &MsgDepth, sizeof(MessageData));
        
        int snd = zmq_msg_send(&msg, senderSocket, ZMQ_DONTWAIT);
        zmq_msg_close(&msg);
        usleep(timeToWait);

        long long wt = now();

        while(1){
            long long tm = now();
            if((tm - wt)/(long long)(1e9) >= timeout)break;

            zmq_msg_t ms;

            zmq_msg_init(&ms);

            int status = zmq_msg_recv(&ms, objectSocket, ZMQ_DONTWAIT);

            if(status < 0){
                zmq_msg_close(&ms);
                continue;
            }

            MessageData *m = (MessageData *)zmq_msg_data(&ms);

            bestDepth.push_back({m->result, m->where});

            zmq_msg_close(&ms);
        }

        if(bestDepth.empty()){
            md.needId = -1;
        }
        else{
            sort(bestDepth.begin(),bestDepth.end());
            md.needId = bestDepth[0].second;
        }

    }

    if(md.needId == -1 && md.msgType == CREATE){
        int pid = fork();
        if(!pid){
            execl("./child", "./child", "-1", to_string(md.newID).c_str(), NULL);
            cout << "Bad fork!\n";
        }
        doesExist[md.newID] = 1;
        initialNode = pid * 10;
        cout << "Ok: " << pid << endl;
        zmq_close(objectSocket);
        zmq_close(senderSocket);
        zmq_ctx_destroy(context);
        mtx.unlock();
        return;
    }
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(MessageData));
    memcpy(zmq_msg_data(&message), &md, sizeof(MessageData));

    int snd = zmq_msg_send(&message, senderSocket, ZMQ_DONTWAIT);
    zmq_msg_close(&message);

    long long startWaiting = now();

    bool ok = false;

    if(md.msgType == PING_ALL){
        ok = true;
    }

    while(1){
        long long tm = now();
        if((tm - startWaiting)/(long long)(1e9) >= timeout){
            break;
        }
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        int status = zmq_msg_recv(&msg, objectSocket, ZMQ_DONTWAIT);
        if(status >= 0){
            ok = true;
            MessageData* it =(MessageData*)zmq_msg_data(&msg);
            md = *it;
            if(md.msgType == PING_ALL){
                have[it->where] = 1; 
            }
            zmq_msg_close(&msg);
            continue;
        }
        zmq_msg_close(&msg);
    }
    if(!ok){
        cout << "Error:" << md.needId << " can't be accessed!" << endl;
        zmq_close(objectSocket);
        zmq_close(senderSocket);
        zmq_ctx_destroy(context);
        return;
    }

    vector<int> bad;
    
    switch (md.msgType)
    {
        case CREATE:
        doesExist[md.newID] = 1;
        cout << "Ok:" << md.result << endl;
        mtx.unlock();
        break;

        case PING_ALL:
        for(auto &p : doesExist){
            if(have.count(p.first) == 0){
                bad.push_back(p.first);
            }
        }
        cout << "Ok:";
        if(bad.empty()){
            cout << "-1" << endl;
        }
        else {
            for(int i = 0; i < bad.size(); ++i){
                cout << bad[i];
                if(i + 1 < bad.size())cout << ";";
            }
            cout << endl;
        }
        break;

        case TIMER_START:
        cout << "Ok:" << md.needId << endl;
        break;

        case TIMER_STOP:
        cout << "Ok:" << md.needId << endl;
        break;

        case TIMER_GET:
        cout << "Ok:" << md.needId << ":" << md.result << endl;
        break;
    
        default:
        cout << "Error:something strange happened" << endl; 
        break;
    }

    zmq_close(objectSocket);
    zmq_close(senderSocket);
    zmq_ctx_destroy(context);
}

int main(int argc, char *argv[]) {
    string cmd;
    cout << "Enter a command!" << endl;

    int cnt = 0;
    while (cin >> cmd) {
        if(cmd == "create"){
            int id, parent;
            cin >> id;
            MessageData msg;
            msg.needId = parent;
            msg.newID = id;
            msg.msgType = CREATE;
            msg.type = QUESTION;
            msg.messageNumber = cnt;
            thread th(sendMessage, msg);

            th.detach();
        }
        else if (cmd == "exec"){
            int id;
            string type;
            cin >> id >> type;
            MessageData msg;
            msg.needId = id;
            msg.type = QUESTION;
            msg.newID = -1;
            msg.messageNumber = cnt;
            if(type == "time"){
                msg.msgType = TIMER_GET;
            }
            else if(type == "start"){
                msg.msgType = TIMER_START;
            }
            else if(type == "stop"){
                msg.msgType = TIMER_STOP;
            }
            else{
                cout << "Wrong request!" << endl;
                continue;
            }
            thread th(sendMessage, msg);

            th.detach();
        }
        else if(cmd == "pingall"){
            MessageData msg;
            msg.needId = 0;
            msg.messageNumber = cnt;
            msg.newID = -1;
            msg.type = QUESTION;
            msg.msgType = PING_ALL;
            thread th(sendMessage, msg);

            th.detach();
        }
        ++cnt;
        cout << "Enter a command!" << endl;
    }
    int status;
    if (wait(&status) == -1) {
        cout << "An error occured!\n";
    }
    return 0;
}