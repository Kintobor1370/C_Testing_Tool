#include "UserInterface.h"

int main()
{
    std::vector<string> currentTestOptions = {
        "Lexical Analysis",
        "Syntax and Semantic Analysis",
        "Statement Coverage Test",
        "Branch Coverage Test"
    };

    UserInterface UI(currentTestOptions);
    UI.goLive();
}
