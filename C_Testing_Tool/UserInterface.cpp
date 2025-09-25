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

string UserInterface::getExtension()
{
    return fileName.substr(fileName.find_last_of(".") + 1);
}

bool UserInterface::isExtensionSupported()
{
    string ext = getExtension();
    for (auto& suppExt : supportedExtensions)
    {
        if (ext == suppExt)
        {
            return true;
        }
    }
    return false;
}

bool UserInterface::fileOpens()
{
    auto f = ifstream(fullFileName);
    return f.is_open();
}

void UserInterface::goLive()
{
    displayMenu();
    char input = _getch();

    while (input != KEY_ESC)
    {
        switch (input)
        {
        case KEY_UP:
            goUp();
            input = _getch();
            break;

        case KEY_DOWN:
            goDown();
            input = _getch();
            break;

        case KEY_ENT:
            system("cls");
            cout << ENTER_FILENAME_MSG;
            cout << "Your file name: ";
            cin >> fileName;
            
            if (!isExtensionSupported())
            {
                cout << "\nERROR: \'" << getExtension() <<"\' files are not supported.\nPress any key to return to the main menu.\n";
                input = _getch();
				goLive();
            }

            fullFileName = filePath + fileName;
            cout << "\nYour file path is: " << fullFileName << "\nIs this correct?\nPress ENTER to confirm\nPress ESC to return to the main menu.\n";
            input = _getch();
            while (input != KEY_ENT && input != KEY_ESC)
            {
                input = _getch();
            }
            if (input == KEY_ESC)
            {
                goLive();
            }
            else
            {
                cout << "\n";
                if (!fileOpens())
                {
                    cout << "ERROR: could not open file " + fullFileName + "\nPress any key to return to the main menu.\n";
                    input = _getch();
                    goLive();
                }

                cout << TEST_START_MSG;
                switch (currOptionIndex)
                {
                case 0:
                {
                    Lexer lexer(fullFileName);
                    lexer.convertToTokens();
                    cout << LEX_NO_ERR_MSG;
                    break;
                }

                case 1:
                {
                    Parser parser(fullFileName);
                    parser.analyze();
                    cout << LEX_SYN_SEM_NO_ERR_MSG;
                    break;
                }

                case 2: case 3:
                {
                    int coverageOption = currOptionIndex - 2;
                    CoverageAnalyzer covAnalyzer(fullFileName);
                    covAnalyzer.analyze(coverageOption);
                    break;
                }

                default:
                    cout << NO_TEST_MSG;
                    break;
                }
				cout << TEST_END_MSG;
                input = _getch();
                if (input != KEY_ESC)
                {
                    goLive();
                }
            }
            break;

        default:
            input = _getch();
            break;
        }
    }
    system("cls");
}