#include "markovdecision.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

MarkovDecision::MarkovDecision() : mapValue(map_size, 0)
{
    for(int i = 0; i < map_size; ++i) {
        if(i == 3) {
            mapReward.push_back(1);
            mapPolicy.push_back('-');
        } else if(i == 7) {
            mapReward.push_back(-1);
            mapPolicy.push_back('-');
        } else if(i == 5) {
            mapReward.push_back(wall);
            mapPolicy.push_back('-');
        } else {
            mapReward.push_back(-0.04);
            mapPolicy.push_back('<'); // initial policy (could be anything
        }
    }
}


double MarkovDecision::cellReward(state _state)
{
    return mapReward[_state.x + 4 * _state.y];
}


double& MarkovDecision::cellValue(state _state)
{
    return mapValue[_state.x + 4 * _state.y];
}


char& MarkovDecision::cellPolicy(state _state)
{
    return mapPolicy[_state.x + 4 * _state.y];
}


void MarkovDecision::display(char map_type)
{
    for(int j = 0; j < 3; ++j) {
        for (int i = 0; i < 4; ++i) {
            state _state = {i, j};
            switch(map_type) {
            case 'r' :
                std::cout << cellReward(_state) << "\t";
                break;
            case 'v' :
                std::cout << cellValue(_state) << "\t";
                break;
            case 'p' :
                std::cout << cellPolicy(_state) << "\t";
                break;
            }
        }
        std::cout << "\n";
    }
}


void putInRange(state& new_state, state current_state)
{
    new_state.x = current_state.x;
    new_state.y = current_state.y;
}


std::vector<transitState> MarkovDecision::transition(const state _state, char _act)
{
    // _act: forth/ back, up/down
    std::vector<transitState> generated;
    if(checkTerminate(_state)) {
        generated.push_back({_state, 1});
        return generated;
    }

    // generate transition state
    switch(_act) {
    case '>' :
        generated.push_back({{_state.x + 1, _state.y}, 0.8});
        generated.push_back({{_state.x, _state.y - 1}, 0.1});
        generated.push_back({{_state.x, _state.y + 1}, 0.1});
        break;
    case '<' :
        generated.push_back({{_state.x - 1, _state.y}, 0.8});
        generated.push_back({{_state.x, _state.y - 1}, 0.1});
        generated.push_back({{_state.x, _state.y + 1}, 0.1});
        break;
    case '^' :
        generated.push_back({{_state.x, _state.y - 1}, 0.8});
        generated.push_back({{_state.x - 1, _state.y}, 0.1});
        generated.push_back({{_state.x + 1, _state.y}, 0.1});
        break;
    case 'v' :
        generated.push_back({{_state.x, _state.y + 1}, 0.8});
        generated.push_back({{_state.x - 1, _state.y}, 0.1});
        generated.push_back({{_state.x + 1, _state.y}, 0.1});
        break;
    }

    // check if hitting wall
    for(int i = 0; i < 3; i++) {
        if(checkWall(generated.at(i)._state)) {
            putInRange(generated[i]._state, _state);
        }
    }

    return generated;
}


bool isNull(double a)
{
    return std::abs(a) < 1e-5;
}


bool MarkovDecision::checkWall(state _state)
{
    if(_state.x < x_min || _state.x > x_max || _state.y < y_min || _state.y > y_max)
        return true;

    return isNull(cellReward(_state) - wall);
}


bool MarkovDecision::checkTerminate(state _state)
{
    return isNull(cellReward(_state) - 1) || isNull(cellReward(_state) + 1);
}


char MarkovDecision::greedyPolicy(state _state)
{
    std::vector<state> neighborhoods;
    neighborhoods.push_back({_state.x + 1, _state.y});
    neighborhoods.push_back({_state.x - 1, _state.y});
    neighborhoods.push_back({_state.x, _state.y - 1});
    neighborhoods.push_back({_state.x, _state.y + 1});
    // find max neighbor
    double max_u = -1000;  // if put 0 in here, there will be trouble cuz value of most cell is negative
    int new_x = 0, new_y = 0;
    for(auto neighbor : neighborhoods) {
        if(neighbor.x > -1 && neighbor.x < 4 && neighbor.y > -1 && neighbor.y < 3){
            if(!checkWall(neighbor)) {
                if(cellValue(neighbor) > max_u) {
                    max_u = cellValue(neighbor);
                    new_x = neighbor.x;
                    new_y = neighbor.y;
                }
            }
        }
    }
    // infer action
    char action;
    if (new_x > _state.x)
        action = '>';
    else if (new_x < _state.x)
        action = '<';
    else if (new_y > _state.y)
        action = 'v';
    else
        action = '^';

    return action;
}


void MarkovDecision::valueIteration(double gamma, double epsilon)
{
    double threshold = epsilon * (1 - gamma) / gamma;
    int iter = 0;
    while(true) {
        // initialize new_mapValue
        std::vector<double> new_mapValue;
        bool conver = true;
        for(int j = 0; j < 3; ++j) {
            for(int i = 0; i < 4; ++i) {
                state _state = {i, j};
                if(!checkWall(_state)) {
                    if(!checkTerminate(_state)) {
                        // compute max utility for all possible action
                        double max_act_u = -1000; //mistake here
                        std::vector<char> acts_list = {'<', '>', '^', 'v'};
                        for (auto act : acts_list) {
                            double act_u = 0;
                            for(auto transit_state : transition(_state, act)) {
                                act_u += transit_state.prob * cellValue(transit_state._state);
//                                if(i==0 && j==0)
//                                    std::cout << transit_state._state.x << "\t" << transit_state._state.y << "\t" << transit_state.prob << "\t" << act_u << "\n";
                            }
                            if(act_u > max_act_u)
                                max_act_u = act_u;
                        }
                        // compute new U
                        double new_u = cellReward(_state) + gamma * max_act_u;
                        new_mapValue.push_back(new_u); // sai ngay day, dinh cai wall nen lech di 1
                        // check for converges
                        if(std::abs(new_u - cellValue(_state)) > threshold)
                            conver = false;
                    } else
                        new_mapValue.push_back(cellReward(_state));
                } else {
                    // hit wall cell
                    new_mapValue.push_back(-10);
                }
            }
        }

        // Update current map
        for(int j = 0; j < 3; ++j) {
            for (int i = 0; i < 4; ++i) {
                state _state = {i, j};
                cellValue(_state) = new_mapValue[i + 4 * j];
            }
        }

        // Update Policy map
        for(int j = 0; j < 3; ++j) {
            for (int i = 0; i < 4; ++i) {
                state _state = {i, j};
                if(!checkWall(_state)) {
                    if(!checkTerminate(_state))
                        cellPolicy(_state) = greedyPolicy(_state);
                    else
                        cellPolicy(_state) = '-';
                }
            }
        }

        if(conver)
            break;
        else {
            std::cout << "\niter: " << iter <<"\n";
            std::cout << "Value map\n";
            display('v');
            std::cout << "Policy map\n";
            display('p');
            ++iter;
        }

    }
}


void MarkovDecision::policyIteration(double gamma)
{
    int iter = 0;
    while(true) {
        std::vector<double> new_mapValue;
        // Compute new_u for every state
        for(int j = 0; j < 3; ++j) {
            for(int i = 0; i < 4; ++i){
                state _state = {i, j};
                double new_u = 0;
                if(!checkWall(_state) && !checkTerminate(_state)) {
                    // get action
                    char act = cellPolicy(_state);
                    // compute new U for this cell
                    new_u = cellReward(_state);
                    for(auto transitState : transition(_state, act)) {
                        new_u += gamma * transitState.prob * cellValue(transitState._state);
                    }
                } else {
                    new_u = cellReward(_state);
                }
                // store new_u
                new_mapValue.push_back(new_u);
            }
        }
        // Update current map
        for(int j = 0; j < 3; ++j) {
            for (int i = 0; i < 4; ++i) {
                state _state = {i, j};
                cellValue(_state) = new_mapValue[i + 4 * j];
            }
        }
        // Update policy
        bool conver = true;
        for(int j = 0; j < 3; ++j) {
            for(int i = 0; i < 4; ++i) {
                state _state = {i, j};
                if(!checkWall(_state) && !checkTerminate(_state)) {
                    // check convergence
                    if(cellPolicy(_state) != greedyPolicy(_state)) {
                        conver = false;
                        // update mapPolicy
                        cellPolicy(_state) = greedyPolicy(_state);
                    }
                } else
                    // wall or terminal state -> no need to go anywhere
                    cellPolicy(_state) = '-';
            }
        }
        // Display
        std::cout << "\niter: " << iter <<"\n";
        std::cout << "Value map\n";
        display('v');
        std::cout << "Policy map\n";
        display('p');

        ++iter;
        if(conver)
            break;
    }
}

