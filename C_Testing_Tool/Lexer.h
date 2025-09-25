#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <fstream>
#include <algorithm>
#include <functional>
#include <conio.h>

using namespace std;

//.........................LEXICAL ANALYZER CLASS
class Lexer
{
	string fileName;													// source copde file name
	ifstream f;															// file descriptor of a model language program

	enum state
	{
		INIT,															// initial state
		ID,																// identifier
		NUMBER,
		DECIMAL,
		CHAR,
		STRING,
		COMMENT,														// comment
		COMMENT_STRING,													// one-line comment
		DELIM,															// delimeter
		NOT_EQ,															// not equal state
		FIN																// final state
	};

	state currState;

	static vector<string> keywordTableStr;
	static vector<string> delimTableStr;

	vector<string> funcTableStr;
	vector<std::function<double(double)>> funcTable;

	Position currPos;													// current position in code file
	Position lexStart;													// start of a current lexeme
	Position lexEnd;													// end of a current lexeme
	vector<unsigned int> linesLen;

	char currChar;														// the current character 
	string buffer;														// buffer for the string being entered
	int bufferTop;														// position of the last non-empty character in buffer

	bool charWasRead;													// Identificator that a char was read

private:
	// Open model language program file for reading
	void openFile(const string fileName);

	// Clear buffer
	void clearBuffer();

	// Add the current character to buffer
	void addToBuffer();

	// Add a different character to buffer
	void addToBuffer(char c);

	// Check if a string in buffer is present in the lexemes list
	int checkLexeme(const string& buf, vector<string> strTable);

	// Reading the next character of a model language program
	void getChar();

	// Pushing the current character back into the input stream
	void ungetChar();

	// Lexical error processing
	void lexicalError(string err);

	// Highlight error in a code line
	string highlightError(Position errStart, Position errEnd);

public:
	Lexer(const string name);

	Tables tables;

	bool addFunctions(vector<std::function<double(double)>> funcs, vector<string> funcsStr);

	Token makeToken();

	vector<Token> convertToTokens();
};

#endif