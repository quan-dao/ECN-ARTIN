#include "markovdecision.h"
#include <iostream>


int main()
{
    MarkovDecision simple_mdp;
    simple_mdp.display('r');
    std::cout << "Value map\n";
    simple_mdp.display('v');
    std::cout << "\nPolicy map\n";
    simple_mdp.display('p');
    std::cout << "\nValue Iteration\n";
    simple_mdp.valueIteration(0.99, 0.01);
//    std::cout << "Policy Iteration\n";
//    simple_mdp.policyIteration(0.99);
//    std::cout << "----------------\n";
//    state _state = {0, 0};
//    auto generated = simple_mdp.transition(_state, '^');
//    for(auto state : generated) {
//        std::cout << state._state.x << "\t" << state._state.y << "\t" << state.prob << "\n";
//    }

}
