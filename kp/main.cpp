#include "pthread.h"
#include "iostream"
#include "string"
#include "queue"
#include "mutex"
#include "thread"
#include "vector"
#include "./libs/json.hpp"
#include <fstream>
#include <unistd.h>

using json = nlohmann::json;
using graph_t = std::vector<std::vector<int>>;

json jobs_data;
graph_t graph;

std::map<std::string, std::mutex> allMutexes;
std::vector<std::thread> threads; 
std::vector<int> jobDegree;
std::vector<std::vector<std::pair<int, std::string>>> jobArgs;

int jobsToDo;

int onGoing = 0;

std::mutex errorMutex;

std::vector<int> errorJobs;

std::queue<int> queueNewThreads;
std::queue<int> doneJobs;

std::mutex q_mutex;

void dfs(int vertex, std::vector<char>& used, const graph_t &graph, bool& ok) {
    used[vertex] = 1;
    for(int to : graph[vertex]){
        if(used[to] == 1){
            ok = false;
            continue;
        }
        if(used[to] == 2){
            continue;
        }
        dfs(to, used, graph, ok);
    }
    used[vertex] = 2;
}

bool isGraphDAG(const int &n, const graph_t &graph) {
    graph_t undirectedGraph(n + 1);
    for (int i = 1; i <= n; ++i) {
        for (int to : graph[i]) {
            undirectedGraph[to].push_back(i);
            undirectedGraph[i].push_back(to);
        }
    }
    // check that DAG has only one component
    std::queue<int> q;
    std::vector<char> used(n + 1, 0);
    q.push(1);
    used[1] = 1;
    while (!q.empty()) {
        int vertex = q.front();
        q.pop();
        for (int to : undirectedGraph[vertex]) {
            if (!used[to]) {
                used[to] = 1;
                q.push(to);
            }
        }
    }
    for (int i = 1; i <= n; ++i) {
        if (!used[i]) {
            return false;
        }
    }

    //check that graph have no cycles
    
    used.assign(n + 1, 0);
    bool ok = true;
    
    for(int i = 1; i <= n; ++i) {
        if(!used[i]) {
            dfs(i, used, graph, ok);
        }
    }

    return ok;
}

void thread_routine(const int& job_id){
    errorMutex.lock();
    if(!errorJobs.empty()){
        errorMutex.unlock();
        return;
    }
    errorMutex.unlock();
    bool hasMutex = false;
    std::string mtx;
    if(jobs_data["jobs_description"][job_id - 1].find("mutex") != jobs_data["jobs_description"][job_id - 1].end()){
        hasMutex = true;
        mtx = jobs_data["jobs_description"][job_id - 1]["mutex"];
    }

    if(hasMutex){
        allMutexes[mtx].lock();
    }
    onGoing++;
    
    sort(jobArgs[job_id].begin(), jobArgs[job_id].end());

    std::string scriptPath = jobs_data["jobs_description"][job_id - 1]["script"];
    scriptPath = "./jobs/" + scriptPath;
    std::string command = "python3 ";
    command += scriptPath + " ";
    command += std::to_string(job_id) + " ";

    for(int i = 0;i < jobArgs[job_id].size();++i){
        command += jobArgs[job_id][i].second + " ";
    }
    errorMutex.lock();
    if(!errorJobs.empty()){
        onGoing--;
        errorMutex.unlock();
        return;
    }
    errorMutex.unlock();
    int status = system(command.c_str());
    if(status != 0){
        errorMutex.lock();
        errorJobs.push_back(job_id);
        onGoing--;
        errorMutex.unlock();
        return;
    }

    for(int to : graph[job_id]){
        jobArgs[to].push_back({job_id, "./jobs_results/" + std::to_string(job_id) + ".txt"}); 
        jobDegree[to]--;
        if(jobDegree[to] == 0){
            q_mutex.lock();
            queueNewThreads.push(to);
            q_mutex.unlock();
        }
    }

    doneJobs.push(job_id);
    jobsToDo--;
    onGoing--;
    if(hasMutex){
        allMutexes[mtx].unlock();
    }
}

int main(int argc, char *argv[]) {

    std::cout << "Enter json file: ";
    std::string s;
    std::cin >> s;

    //reading json
    std::ifstream file_input(s);
    if(!file_input.good()){
        std::cout << "Bad file!\n";
        return 0;
    }
    file_input >> jobs_data;
    
    int jobs_number = jobs_data["jobs_number"];

    jobsToDo = jobs_number;

    jobDegree.assign(jobs_number + 1, 0);
    graph.resize(jobs_number + 1);
    jobArgs.resize(jobs_number + 1);
    threads.resize(jobs_number + 1);


    for (int i = 0; i < jobs_number; ++i) {
        for (auto to : jobs_data["jobs_graph"][i]) {
            graph[i + 1].push_back(to);
            jobDegree[to]++;
        }
    }

    std::cout << "Checking that graph is DAG...\n";
    if(isGraphDAG(jobs_number, graph)) {
        std::cout << "Graph is valid!\n";
    }
    else {
        std::cout << "Graph is invalid!\n";
        return 0;
    }

    for(int i = 1; i <= jobs_number; ++i){
        if(jobDegree[i] == 0)queueNewThreads.push(i);
    }

    std::vector<std::string> mutexNames = jobs_data["mutex_names"];
    for(const std::string &name : mutexNames){
        allMutexes[name];
    }


    while (jobsToDo > 0) {
        errorMutex.lock();
        if(!errorJobs.empty()){
            std::cout << "An error occured!\n";
            errorMutex.unlock();
            int tt = 0;
            while(1){
                ++tt;
                errorMutex.lock();
                if(onGoing == 0){;
                    errorMutex.unlock();
                    break;
                }
                errorMutex.unlock();
            }
            break;
        }
        errorMutex.unlock();
        int have = doneJobs.size();
        while(have > 0){
            int jb = doneJobs.front();
            doneJobs.pop();
            std::cout << "Job " << jb << " is done!\n";
            --have;
        }
        if(queueNewThreads.empty())continue;
        q_mutex.lock();

        int jobNumber = queueNewThreads.front();

        queueNewThreads.pop();

        std::thread newJob(thread_routine, jobNumber);

        threads[jobNumber] = std::move(newJob);

        threads[jobNumber].detach();
        q_mutex.unlock();
    }
    int have = doneJobs.size();
    while(have > 0 && errorJobs.empty()){
        int jb = doneJobs.front();
        doneJobs.pop();
        std::cout << "Job " << jb << " is done!\n";
        --have;
    }
    if(!errorJobs.empty()){
        std::cout << "Problem in ";
        for(auto jb : errorJobs){
            std::cout << jb << " ";
        }
        std::cout << "job.\n";
    }
    else{
        std::cout << "OK!\n";
    }
    return 0;
}