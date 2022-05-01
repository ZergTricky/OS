#ifndef NODE
#define NODE

struct Node {
    Node * left;    
    Node * right;

    int id;
    int pid;
    
    long long timer;
    bool isTimerActive;
    
    long long lastActive;
    Node(int _pid,int _id){
        left = nullptr;
        right = nullptr;
        pid = _pid;
        id = _id;
        lastActive = now();
        timer = 0;
        isTimerActive = false;
    }

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
};

#endif // NODE