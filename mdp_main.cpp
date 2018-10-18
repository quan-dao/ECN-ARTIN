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
    simple_mdp.valueIteration(0.94, 0.01);
}
