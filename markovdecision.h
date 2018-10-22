#ifndef MARKOVDECISION_H
#define MARKOVDECISION_H


#include<vector>

const int map_size = 12;
const int wall = -10;
const int x_min = 0;
const int x_max = 3;
const int y_min = 0;
const int y_max = 2;

struct state {
    int x;
    int y;
};


struct transitState {
    state _state;
    double prob;  // prob in [0, 1]
};


class MarkovDecision
{
    std::vector<double> mapReward;
    std::vector<double> mapValue;
    std::vector<char> mapPolicy;
public:
    MarkovDecision();
    std::vector<transitState> transition(const state _state, char _act);
    void display(char map_type);
    double cellReward(state _state);
    double& cellValue(state _state);
    char& cellPolicy(state _state);
    bool checkWall(state _state);
    bool checkTerminate(state _state);
    void valueIteration(double gamma, double epsilon);
    void policyIteration(double gamma);
    char greedyPolicy(state _state);
};

#endif // MARKOVDECISION_H
