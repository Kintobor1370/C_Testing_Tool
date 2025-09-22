#include "UserInterface.h"

using namespace std;

void UserInterface::displayMenu()
{
    system("cls");
    cout << BAR + GREETINGS_MSG + BAR + "\n" + SELECT_MSG + menu[currOptionIndex] + HINT_MSG;
}

void UserInterface::goUp()
{
    currOptionIndex = (currOptionIndex + optionsCount - 1) % optionsCount;
    displayMenu();
}

void UserInterface::goDown()
{
    currOptionIndex = (currOptionIndex + 1) % optionsCount;
    displayMenu();
}

UserInterface::UserInterface(vector<string> options)
{
    currOptionIndex = 0;
    for (int i = 0; i < options.size(); i++)
    {
        string optionsStr = "";
        for (int j = 0; j < options.size(); j++)
        {
            optionsStr += i == j ? CHOSEN_STR : EMPTY_STR;
            optionsStr += options.at(j) + "\n";
        }
        menu.push_back(optionsStr);
    }
    optionsCount = menu.size();
}

void UserInterface::goLive()
{
    displayMenu();
    char input = getch();

    while (input != KEY_ESC)
    {
        switch (input)
        {
        case KEY_UP:
            goUp();
            input = getch();
            break;

        case KEY_DOWN:
            goDown();
            input = getch();
            break;

        case KEY_ENT:
            system("cls");
            cout << ENTER_FILENAME_MSG;
            cout << "Your file name: ";
            cin >> fileName;
            fullFileName = filePath + fileName;
            cout << fullFileName << "\nIs this correct?\nPress ENTER to confirm\nPress ESC to cancel.\n";
            input = getch();
            while (input != KEY_ENT && input != KEY_ESC)
            {
                input = getch();
            }
            if (input == KEY_ESC)
            {
                goLive();
            }
            else
                switch (currOptionIndex)
                {
                case 0:
                {
                    Lexer lexer(fullFileName);
                    lexer.convertToTokens();
                    cout << "No lexical errors. Well done!\n";
                    input = getch();
                    if (input != KEY_ESC)
                    {
                        goLive();
                    }
                    break;
                }

                case 1:
                {
                    Parser parser(fullFileName);
                    parser.analyze();
                    cout << "No lexical, syntax or semantic errors. Way to go!\n";
                    input = getch();
                    if (input != KEY_ESC)
                    {
                        goLive();
                    }
                    break;
                }

                case 2: case 3:
                {
                    int coverageOption = currOptionIndex - 2;
                    CoverageAnalyzer covAnalyzer(fullFileName);
                    covAnalyzer.analyze(coverageOption);
                    input = getch();
                    if (input != KEY_ESC)
                    {
                        goLive();
                    }
                    break;
                }

                default:
                    cout << "ERROR: This test is unavailable.\n";
                    break;
                }

        default:
            input = getch();
            break;
        }
    }
    system("cls");
}