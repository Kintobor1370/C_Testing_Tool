#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_ENT 13
#define KEY_ESC 27

#include "Lexer.h"
#include "Parser.h"
#include "CoverageAnalyzer.h"
#include <windows.h>
#include <conio.h>

using namespace std;

class UserInterface
{
    vector<string> menu;
    int optionsCount;
    int currOptionIndex;

    const string CHOSEN_STR = "  > ";
    const string EMPTY_STR = "    ";
    const string BAR = "==========================================\n";
    const string GREETINGS_MSG = "   Welcome to the C Code Testing Tool   \n";
    const string SELECT_MSG = "Please select an option:\n";
    const string HINT_MSG = "\nHint: Navigate with up arrow & down arrow\n      Press ENTER to confirm\n      Press ESC to quit\n";
    const string ENTER_FILENAME_MSG = "Please navigate to the \'Tests\' folder,\nplace your C code file there and enter the file name.\n";
	const string TEST_START_MSG = "Testing started...\n";
	const string TEST_END_MSG = "Testing completed.\nPress any key to return to the main menu.\nPress ESC to quit.\n";
	const string LEX_NO_ERR_MSG = "No lexical errors. Well done!\n";
	const string LEX_SYN_SEM_NO_ERR_MSG = "No lexical, syntax or semantic errors. Way to go!\n";
    const string NO_TEST_MSG = "ERROR: This test is unavailable.\n";

    const string filePath = "..\\Tests\\";
    string fileName;
    string fullFileName;

	vector<string> supportedExtensions = { "c" };

    void displayMenu();
    void goUp();
    void goDown();
    
	string getExtension();
	bool isExtensionSupported();
	bool fileOpens();

public:
    UserInterface(vector<string> options);

    void goLive();
};

#endif