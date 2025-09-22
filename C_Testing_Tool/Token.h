#ifndef TOKEN_H
#define TOKEN_H

#define LINE_INIT 1
#define COL_INIT 1

#include <iostream>
#include <string>
#include <vector>

using namespace std;

//.........................LEXEMES
enum lexeme
{
	LEX_NULL,															// 0

	//.....................KEYWORDS
	//    Data types
	LEX_INT,															// 1
	LEX_FLOAT,															// 2
	LEX_DOUBLE,															// 3
	LEX_BOOL,															// 4
	LEX_CHAR,															// 5
	LEX_STRING,															// 6

	LEX_VOID,															// 7

	//    Branches
	LEX_IF,																// 8
	LEX_ELSE,															// 9
	LEX_SWITCH,															// 10
	LEX_CASE,															// 11
	LEX_BREAK,															// 12
	LEX_CONTINUE,														// 13

	//    Logical operations
	LEX_TRUE,															// 14
	LEX_FALSE,															// 15

	//    Loops
	LEX_FOR,															// 16
	LEX_WHILE,															// 17
	LEX_DO,																// 18

	//    Input/output
	LEX_SCANF,															// 19
	LEX_PRINT,															// 20
	LEX_PRINTF,															// 21

	//    Return value
	LEX_RETURN,															// 22

	//    Final state
	LEX_FIN,															// 23

	//.....................DELIMETERS
	LEX_LEFT_BRACE,														// 24   1
	LEX_RIGHT_BRACE,													// 25   2

	LEX_QUOTE_SINGLE,													// 26	3
	LEX_QUOTE_DOUBLE,													// 27   4
	LEX_SEMICOLON,														// 28   5
	LEX_COMMA,															// 29   6
	LEX_COLON,															// 30   7
	LEX_ASSIGN,															// 31   8

	LEX_PLUS,															// 32   9
	LEX_MINUS,															// 33   10
	LEX_TIMES,															// 34   11
	LEX_SLASH,															// 35   12
	LEX_PERCENT,														// 36   13

	LEX_PLUS_PLUS,														// 37   14
	LEX_MINUS_MINUS,													// 38	15

	LEX_PLUS_ASSIGN,													// 39   16
	LEX_MINUS_ASSIGN,													// 40   17
	LEX_TIMES_ASSIGN,													// 41   18
	LEX_SLASH_ASSIGN,													// 42   19

	LEX_LEFT_PAREN,														// 43   20
	LEX_RIGHT_PAREN,													// 44   21

	// Condition
	LEX_NOT,															// 45	22
	LEX_EQ,																// 46   23
	LEX_GREATER,														// 47   24
	LEX_LESS,															// 48   25
	LEX_GREATER_EQ,														// 49   26
	LEX_LESS_EQ,														// 50   27
	LEX_NOT_EQ,															// 51   28
	LEX_AND,															// 52   29
	LEX_OR,																// 53   30

	LEX_ID,																// 54
	LEX_NUM,															// 55
	LEX_CHAR_CONST,														// 56
	LEX_STR_CONST,														// 57
	LEX_FUNCTION														// 58
};


//.........................POSITION IN A CODE FILE
struct Position
{
	unsigned int lineNum;
	unsigned int colNum;

	Position(unsigned int ln = LINE_INIT, unsigned int col = COL_INIT);
};


//.........................LEXICAL TOKEN CLASS
class Token
{
	lexeme lex;
	double value;
	Position start;
	Position fin;

public:
	Token(lexeme lex = LEX_NULL, double val = 0, Position st = Position(), Position fn = Position());

	lexeme getLexeme() const;
	double getValue() const;
	Position getStartPosition() const;
	Position getEndPosition() const;
	friend std::ostream& operator << (ostream& out, Token t);
};


enum idType
{
	NO_TYPE,
	STD_VAR,
	INPUT_VAR,
	FUNCTION
};



//.........................IDENTIFIER CLASS
class Identifier
{
	string name;
	idType idtype;
	lexeme dataType;

	vector<int> defLineNums;
	vector<int> useLineNums;

	int value;
	string strValue;

public:
	Identifier(const string idName = "");

	string getName() const;

	bool isDeclared() const;
	void declare(int lineNum);

	idType getIdType();
	bool setIdType(idType newType);

	lexeme getDataType() const;
	bool setDataType(lexeme newType);

	bool isUsed() const;
	void setUse(int lineNum);

	int getValue() const;
	string getStrValue() const;
	void setValue(int newValue);
	void setValue(string newStrValue);
};


//.........................IDENTIFIER AND STRING CONSTANTS TABLES CLASS
struct Tables
{
	vector<Identifier> ids;												// Identifiers table (vectorized)
	vector<char> charConsts;											// Char constants table (vectorized)
	vector<string> strConsts;	    									// String constants table (vectorized)

	// Filling the identifier table with unique entries
	int addUniqueId(const string name);

	// Filling the char constants table with unique entries
	int addUniqueCharConst(const char c);

	// Filling the string constants table with unique entries
	int addUniqueStrConst(const string str);

	// Clear all tables
	void clearTables();
};

#endif