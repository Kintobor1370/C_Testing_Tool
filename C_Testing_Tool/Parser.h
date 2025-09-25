#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <conio.h>

using namespace std;

struct parserResults
{
	vector<Token> sourceCode;
	Tables tables;
};


/*___________________________________________SYNTAX STRUCTURE OF A C PROGRAM___________________________________________
 * Legend:
 *	1) (A | B) = (A OR B)
 *  2) (A; [ B | C | D ]) = (A; B) OR (A; C) OR (A; D)
 * 	3) (A <; B>) = (A) OR (A; B) OR (A; B; B) OR (A; B; B; B) OR ...
 *  4) ({ A; B }) = code block of operations A and B
 *
 *
 * Function's header:		FUNC_HEADER --> [int | float | double | char | char* | bool | void] id_name ([ | PARAMS]) CODE_BLOCK
 *
 * Function parameters:		PARAMS		--> PARAM <, PARAM>
 * Function parameter:		PARAM		--> [int | float | double | char | char* | bool] ID [ | = CONST]
 * Constant parameter		CONST		--> number | 'char_constant' | "string_constant" | true | false
 *
 * Operations				CODE_BLOCK	--> { <STMNT> }
 * Operation				STMNT		--> ASSIGN | CODE_BLOCK | if (EXPR) OP <else OP> | while (EXPR) OP |
 * 											for ([ASSIGN]; [EXPR]; [ASSIGN]) OP | scanf("string_constant", id_name); |
 *											[print | printf]("string_constant", id_name <, id_name>); | return [ | id_name];
 * Statement operator		ASSIGN		--> EXPR | ID = EXPR;
 * Statement				EXPR		--> DISJ
 * Disjunction				DISJ		--> CONJ <|| CONJ>
 * Conjunction				CONJ		--> CMP <&& CMP>
 * Comparison				CMP			--> ADD | ADD [ == | < | > | <= | >= | != ] ADD
 * Additive state			ADD			--> MULTI <[ + | - ] MULTI>
 * Multiplicative state		MULTI		--> FIN <[ * | / | % ] FIN>
 * Final state 				FIN			--> id_name [ | [ ++ | -- ]] | [ ++ | -- ] id_name | CONST | [ + | - ] FIN | !FIN | (EXPR)
 */


 //.........................PARSER CLASS
class Parser
{
	string fileName;

	Lexer lexer;														// Lexical analyzer
	vector<Token> sourceCode;											// Tokenized source code

	vector<Identifier> funcName;										// function names table (vectorized)
	Position currPos;													// current position in the code file
	int currTokenIndex;

	stack<lexeme> idTypeStack;
	stack<lexeme> tokenStack;
	Token currToken;                                                    // Current token
	Token prevToken;													// Previous token

	lexeme funcType;													// Data type of the function
	int semErrorCount;													// Semantic error count

	bool isFuncType(lexeme lex);
	bool isDataType(lexeme lex);
	bool isLiteral(lexeme lex);

	// Syntax actions
	vector<lexeme> extractFromStringConst(string str_cnst);				// Extract data types from string constant		
	void FUNC_HEADER();													// Function's header

	void PARAMS();														// Descriptions of function parameters
	void PARAM();														// Description of function parameters											

	void CODE_BLOCK();													// Code block
	void STMNT();														// Statement
	void ASSIGN();														// Assignment statement
	void EXPR();														// Expression
	void DISJ();														// Disjunction
	void CONJ();														// Conjunction
	void CMP();															// Comparison
	void ADD();															// Addition
	void MULTI();														// Multiplication
	void FIN();															// Final state

	// Semantic actions
	void setDataTypeForId();
	void idDeclaredCheck(Token id);
	void idsUsedCheck();
	void singleOperationCheck();
	void unaryOperationCheck();
	void incrementCheck();
	void notOperatorCheck();
	void assignMatchingTypeCheck();
	void conditionTypeCheck();
	void returnCheck();

	// Get the next token
	void getToken();

	// Unget the current token
	void ungetToken();

	void missingSemicolonCheck();

	// Syntax error processing
	void syntaxError(string err, Token errToken);

	// Semantic error processing
	void semanticError(string err, Token errToken);

	// Semantic warning processing
	void semanticWarning(string wrn);

	// Highlight error in a code line
	string highlightError(Token token);

public:
	Parser(string name);

	parserResults analyze();
};

#endif