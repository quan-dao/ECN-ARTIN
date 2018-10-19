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


double MarkovDecision::cellReward(int x, int y)
{
    return mapReward[x + 4 * y];
}


double& MarkovDecision::cellValue(int x, int y)
{
    return mapValue[x + 4 * y];
}


char& MarkovDecision::cellPolicy(int x, int y)
{
    return mapPolicy[x + 4 * y];
}


void MarkovDecision::display(char map_type)
{
    for(int j = 0; j < 3; ++j) {
        for (int i = 0; i < 4; ++i) {
            switch(map_type) {
            case 'r' :
                std::cout << cellReward(i, j) << "\t";
                break;
            case 'v' :
                std::cout << cellValue(i, j) << "\t";
                break;
            case 'p' :
                std::cout << cellPolicy(i, j) << "\t";
                break;
            }
        }
        std::cout << "\n";
    }
}


std::vector<transitState> MarkovDecision::transition(const state _state, char _act)
{
    // _act: forth/ back, up/down
    std::vector<transitState> generated;
    if(checkTerminate(_state)) {
        generated.push_back({{_state.x, _state.y}, 1});
        return generated;
    }

    // generate transition state
    switch(_act) {
    case '>' :
        generated.push_back({{_state.x + 1, _state.y}, 0.9});
        generated.push_back({{_state.x, _state.y - 1}, 0.1});
        generated.push_back({{_state.x, _state.y + 1}, 0.1});
        break;
    case '<' :
        generated.push_back({{_state.x - 1, _state.y}, 0.9});
        generated.push_back({{_state.x, _state.y - 1}, 0.1});
        generated.push_back({{_state.x, _state.y + 1}, 0.1});
        break;
    case '^' :
        generated.push_back({{_state.x, _state.y - 1}, 0.9});
        generated.push_back({{_state.x - 1, _state.y}, 0.1});
        generated.push_back({{_state.x + 1, _state.y}, 0.1});
        break;
    case 'v' :
        generated.push_back({{_state.x, _state.y + 1}, 0.9});
        generated.push_back({{_state.x - 1, _state.y}, 0.1});
        generated.push_back({{_state.x + 1, _state.y}, 0.1});
        break;
    }

    // check if hitting wall
    for(int i = 0; i < 3; ++i) {
        if(checkWall({generated[i]._state.x, generated[i]._state.y})) {
            double prob = generated[i].prob;
            generated.erase(generated.begin() + i);
            generated.push_back({{_state.x, _state.y}, prob});
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

    return isNull(cellReward(_state.x, _state.y) - wall);
}


bool MarkovDecision::checkTerminate(state _state)
{
    return isNull(cellReward(_state.x, _state.y) - 1) || isNull(cellReward(_state.x, _state.y) + 1);
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
                if(cellValue(neighbor.x, neighbor.y) > max_u) {
                    max_u = cellValue(neighbor.x, neighbor.y);
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
                        double max_act_u = 0;
                        std::vector<char> acts_list = {'<', '>', '^', 'v'};
                        for (auto act : acts_list) {
                            double act_u = 0;
                            for(auto transit_state : transition(_state, act)) {
                                act_u += transit_state.prob * cellValue(transit_state._state.x, transit_state._state.y);
                            }
                            if(act_u > max_act_u)
                                max_act_u = act_u;
                        }
                        // compute new U
                        double new_u = cellReward(_state.x, _state.y) + gamma * max_act_u;
                        new_mapValue.push_back(new_u); // sai ngay day, dinh cai wall nen lech di 1
                        // check for converges
                        if(std::abs(new_u - cellValue(_state.x, _state.y)) > threshold)
                            conver = false;
                    } else
                        new_mapValue.push_back(cellReward(_state.x, _state.y));
                } else {
                    // hit wall cell
                    new_mapValue.push_back(-10);
                }
            }
        }

        // Update current map
        for(int j = 0; j < 3; ++j) {
            for (int i = 0; i < 4; ++i) {
                cellValue(i, j) = new_mapValue[i + 4 * j];
            }
        }

        // Update Policy map
        for(int j = 0; j < 3; ++j) {
            for (int i = 0; i < 4; ++i) {
                state _state = {i, j};
                if(!checkWall(_state)) {
                    if(!checkTerminate(_state))
                        cellPolicy(_state.x, _state.y) = greedyPolicy(_state);
                    else
                        cellPolicy(_state.x, _state.y) = '-';
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
                    char act = cellPolicy(_state.x, _state.y);
                    // compute new U for this cell
                    new_u = cellReward(_state.x, _state.y);
                    for(auto transitState : transition(_state, act)) {
                        new_u += gamma * transitState.prob * cellValue(transitState._state.x, transitState._state.y);
                    }
                } else {
                    new_u = cellReward(_state.x, _state.y);
                }
                // store new_u
                new_mapValue.push_back(new_u);
            }
        }
        // Update current map
        for(int j = 0; j < 3; ++j) {
            for (int i = 0; i < 4; ++i)
                cellValue(i, j) = new_mapValue[i + 4 * j];
        }
        // Update policy
        bool conver = true;
        for(int j = 0; j < 3; ++j) {
            for(int i = 0; i < 4; ++i) {
                state _state = {i, j};
                if(!checkWall(_state) && !checkTerminate(_state)) {
                    // check convergence
                    if(cellPolicy(_state.x, _state.y) != greedyPolicy(_state)) {
                        conver = false;
                        // update mapPolicy
                        cellPolicy(_state.x, _state.y) = greedyPolicy(_state);
                    }
                } else
                    // wall or terminal state -> no need to go anywhere
                    cellPolicy(_state.x, _state.y) = '-';
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

