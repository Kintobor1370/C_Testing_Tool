#include "Parser.h"

using namespace std;

template <class T1, class T2>

// Extract item from stack
void extract(T1& stack, T2& item)
{
	item = stack.top();
	stack.pop();
}

// Get lexeme with the position following the provided lexeme
Token getNextPosition(Token token)
{
	Position highlightStart = token.getEndPosition();
	Position highlightEnd = highlightStart;
	highlightEnd.colNum++;

	return Token(LEX_NULL, (int)LEX_NULL, highlightStart, highlightEnd);
}

//.........................PARSER CLASS

// Extract data types from string constant (to detect expected data in 'scanf', 'print' and 'printf')
vector<lexeme> Parser::extractFromStringConst(string str_cnst)
{
	vector<lexeme> expectedData;
	for (int i = 0; i < str_cnst.length(); i++)
	{
		if (str_cnst[i] == '%')
		{
			switch (str_cnst[i + 1])
			{
			case 'd':
				expectedData.push_back(LEX_INT);
				i++;
				break;

			case 'f':
				expectedData.push_back(LEX_DOUBLE);
				i++;
				break;

			case 'c':
				expectedData.push_back(LEX_CHAR);
				i++;
				break;

			case 's':
				expectedData.push_back(LEX_STRING);
				i++;
				break;

			default:
				syntaxError(											// Syntax error 1
					"Unknown data type '%" + string(1, str_cnst[i + 1]) + "'",
					prevToken
				);
				break;
			}
		}
	}
	return expectedData;
}

// Check if a lexeme describes a function type
bool Parser::isFuncType(lexeme lex)
{
	return lex >= LEX_INT && lex <= LEX_VOID;
}

// Check if a lexeme describes a data type
bool Parser::isDataType(lexeme lex)
{
	return lex >= LEX_INT && lex <= LEX_STRING;
}

// Check if a lexeme is a literal
bool Parser::isLiteral(lexeme lex)
{
	return lex >= LEX_NUM && lex <= LEX_STR_CONST || lex == LEX_TRUE || lex == LEX_FALSE;
}

// Analyze source code starting from the function header
void Parser::FUNC_HEADER()
{
	if (!isFuncType(currToken.getLexeme()))								// if first lexeme is not the function's data type:
	{
		syntaxError("Expected function type", currToken);				// Syntax error 2
	}
	funcType = currToken.getLexeme();
	getToken();															//   get next lexeme
	if (currToken.getLexeme() != LEX_ID)								// if the next lexeme is not an identifier
	{
		syntaxError("Expected function name", currToken);				// Syntax error 3
	}
	int idTableIndex = currToken.getValue();
	lexer.tables.ids[idTableIndex].setIdType(FUNCTION);

	getToken();
	if (currToken.getLexeme() != LEX_LEFT_PAREN)
	{
		syntaxError(													// Syntax error 4
			"Expected '('",
			getNextPosition(prevToken)
		);
	}
	getToken();
	PARAMS();															// analyze them as variables

	getToken();
	CODE_BLOCK();														// analyze the function's main body
	idsUsedCheck();
}

// Analysis of descriptions of the function parameters
void Parser::PARAMS()
{
	if (currToken.getLexeme() != LEX_RIGHT_PAREN)
	{
		PARAM();
		while (currToken.getLexeme() == LEX_COMMA)						// analyze function parameters description while a comma is met after the description
		{
			getToken();
			PARAM();
		}
	}
}

// Input parameter analysis
void Parser::PARAM()
{
	auto idType = currToken.getLexeme();								// save the type of variable to be described further (to assing a value of the same type when required)
	idTypeStack.push(idType);
	getToken();
	if (currToken.getLexeme() != LEX_ID)								// if the lexeme is not an identifier:	
	{
		syntaxError(													//   Syntax error 5
			"Expected variable name",
			getNextPosition(prevToken)
		);
	}

	int idTableIndex = currToken.getValue();
	if (lexer.tables.ids[idTableIndex].isDeclared())
	{
		semanticError(
			"Variable '" + lexer.tables.ids[idTableIndex].getName() + "' has already been declared",
			currToken
		);
	}
	lexer.tables.ids[idTableIndex].setIdType(INPUT_VAR);
	setDataTypeForId();													// assign this identifier its data type (recently saved in tokens stack)
	getToken();
	if (currToken.getLexeme() == LEX_ASSIGN)							// if the identifier above is being assigned a constant value
	{
		getToken();
		if (!isLiteral(currToken.getLexeme()))
		{
			syntaxError("Expected a constant value", currToken);		// Syntax error 6
		}
		assignMatchingTypeCheck();
	}

	if (
		currToken.getLexeme() != LEX_COMMA &&							// if the next lexeme is not a comma or a closing parenthese:
		currToken.getLexeme() != LEX_RIGHT_PAREN
		) {
		syntaxError(													//   Syntax error 7
			"Expected ',' or ')'",
			getNextPosition(prevToken)
		);
	}
	tokenStack.pop();
}

// Multiple operators analysis
void Parser::CODE_BLOCK()
{
	if (currToken.getLexeme() != LEX_LEFT_BRACE)						// if next lexeme is left brace:
	{
		syntaxError(													//   Syntax error 8
			"Expected '{'",
			getNextPosition(prevToken)
		);
	}

	getToken();
	while (currToken.getLexeme() != LEX_RIGHT_BRACE)					// while the closing bracket is not met
	{
		STMNT();														// analyse current statement
		if (currToken.getLexeme() == LEX_FIN)							// if the code has ended without closing the code body
		{
			syntaxError("Expected '}'", currToken);						//   Syntax error 9
		}
	}
	getToken();
}

// Statement analysis
void Parser::STMNT()
{
	if (isDataType(currToken.getLexeme()))
	{
		lexeme dataType = currToken.getLexeme();							// save the type of variable to be described further (to assing a value of the same type when required)
		idTypeStack.push(dataType);
		getToken();
		if (currToken.getLexeme() != LEX_ID)								// if the lexeme is not an identifier:	
		{
			syntaxError(													//   Syntax error 10
				"Expected variable name",
				getNextPosition(prevToken)
			);
		}
		int idTableIndex = currToken.getValue();
		if (lexer.tables.ids[idTableIndex].isDeclared())
		{
			semanticError(
				"Variable '" + lexer.tables.ids[idTableIndex].getName() + "' has already been declared",
				currToken
			);
		}
		lexer.tables.ids[idTableIndex].setIdType(STD_VAR);
		setDataTypeForId();													// assign this identifier its data type (recently saved in tokens stack)
		getToken();
		if (currToken.getLexeme() == LEX_ASSIGN)							// if the identifier above is being assigned a constant value
		{
			getToken();
			EXPR();
			assignMatchingTypeCheck();
		}
		missingSemicolonCheck();
		tokenStack.pop();													// analyze the variable
	}
	else
	{
		switch (currToken.getLexeme())
		{
		case LEX_IF:														// if() operator
			getToken();
			if (currToken.getLexeme() != LEX_LEFT_PAREN)
			{
				syntaxError(												// Syntax error 11
					"In 'if' statement: Expected '(' after 'if'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			EXPR();
			conditionTypeCheck();
			if (currToken.getLexeme() != LEX_RIGHT_PAREN)
			{
				syntaxError(												// Syntax error 12
					"In 'if' statement: Expected ')'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			STMNT();
			if (currToken.getLexeme() == LEX_ELSE)
			{
				getToken();
				STMNT();
			}
			break;

		case LEX_WHILE:														// while() loop
			getToken();
			if (currToken.getLexeme() != LEX_LEFT_PAREN)
			{
				syntaxError(												// Syntax error 13
					"In 'while' statement: Expected '(' after 'while'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			EXPR();
			conditionTypeCheck();

			if (currToken.getLexeme() != LEX_RIGHT_PAREN)
			{
				syntaxError(												// Syntax error 14
					"In 'while' statement: Expected ')'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			STMNT();
			break;

		case LEX_DO:														// do { ... } while() loop
			getToken();
			STMNT();

			if (currToken.getLexeme() != LEX_WHILE)
			{
				syntaxError(												// Syntax error 15
					"In 'do' statement: Expected 'while'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			if (currToken.getLexeme() != LEX_LEFT_PAREN)
			{
				syntaxError(												// Syntax error 16
					"In 'do' statement: Expected '(' after 'while'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			EXPR();
			conditionTypeCheck();

			if (currToken.getLexeme() != LEX_RIGHT_PAREN)
			{
				syntaxError(												// Syntax error 17
					"In 'do' statement: Expected ')'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			missingSemicolonCheck();
			getToken();
			break;

		case LEX_FOR:														// for(;;) loop
			getToken();
			if (currToken.getLexeme() != LEX_LEFT_PAREN)
			{
				syntaxError(												// Syntax error 19
					"In 'for' statement: Expected '(' after 'for'",
					getNextPosition(prevToken)
				);
			}
			getToken();

			// for(<analyzing this part>; ...; ...)
			switch (currToken.getLexeme())
			{
			case LEX_SEMICOLON:												// either empty statement
				getToken();
				break;

			default:
				ASSIGN();													// or an assignment statement
				break;
			}

			// for(...; <analyzing this part>; ...)
			if (currToken.getLexeme() == LEX_SEMICOLON)
			{
				getToken();
			}
			else
			{
				EXPR();
				conditionTypeCheck();
				if (currToken.getLexeme() != LEX_SEMICOLON)
				{
					syntaxError(											// Syntax error 20
						"In 'for' statement: Expected ';' between last two statements",
						currToken
					);
				}
				getToken();
			}

			// for(...; ...; <analysing this part>)
			if (currToken.getLexeme() == LEX_RIGHT_PAREN)
			{
				getToken();
			}
			else
			{
				EXPR();
				if (currToken.getLexeme() != LEX_RIGHT_PAREN)
				{
					syntaxError(											// Syntax error 21
						"In 'for' statement: Expected ')'",
						getNextPosition(prevToken)
					);
				}
				getToken();
			}
			STMNT();
			break;

		case LEX_SCANF:														// scanf() operator
			getToken();
			if (currToken.getLexeme() != LEX_LEFT_PAREN)
			{
				syntaxError(												// Syntax error 22
					"In 'scanf' statement: Expected '(' after 'scanf'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			if (currToken.getLexeme() != LEX_QUOTE_DOUBLE)
			{
				syntaxError(												// Syntax error 23
					"In 'scanf' statement: Expected string constant as format",
					currToken
				);
			}
			getToken();
			int tableIndex;
			tableIndex = currToken.getValue();
			int count;
			count = 1;

			getToken();
			getToken();
			for (auto& inputVarType : extractFromStringConst(lexer.tables.strConsts[tableIndex]))
			{
				if (currToken.getLexeme() != LEX_COMMA)
				{
					syntaxError(											// Syntax error 24
						"In 'scanf' statement: Expected ','",
						getNextPosition(prevToken)
					);
				}

				getToken();
				lexeme currLex;
				currLex = currToken.getLexeme();
				if (currLex != LEX_ID)
				{
					syntaxError(											// Syntax error 24
						"In 'scanf' statement: Expected variable of matching type",
						currToken
					);
				}
				currLex = lexer.tables.ids[currToken.getValue()].getDataType();
				if (currLex != inputVarType)
				{
					syntaxError(											// Syntax error 25
						"In 'scanf' statement: Input argument " + to_string(count) + " has a wrong format",
						currToken
					);
				}
				getToken();
				count++;
			}
			if (currToken.getLexeme() != LEX_RIGHT_PAREN)
			{
				syntaxError(												// Syntax error 26
					"In 'scanf' statement: Expected ')'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			missingSemicolonCheck();
			getToken();
			break;

		case LEX_PRINT:	case LEX_PRINTF:									// print() and printf() operators
		{
			string expr = currToken.getLexeme() == LEX_PRINT ? "'print'" : "'printf'";
			getToken();
			if (currToken.getLexeme() != LEX_LEFT_PAREN)
			{
				syntaxError(												// Syntax error 27
					"In '" + expr + "' statement: Expected '(' after '" + expr + "'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			if (currToken.getLexeme() == LEX_RIGHT_PAREN)
			{
				syntaxError(												// Syntax error 28
					"In '" + expr + "' statement: No arguments found",
					currToken
				);
			}
			else if (currToken.getLexeme() != LEX_QUOTE_DOUBLE)
			{
				syntaxError(												// Syntax error 29
					"In '" + expr + "' statement: Expected string constant as format",
					currToken
				);
			}
			getToken();
			int tableIndex;
			tableIndex = currToken.getValue();
			int count;
			count = 1;

			getToken();
			getToken();
			for (auto& inputVarType : extractFromStringConst(lexer.tables.strConsts[tableIndex]))
			{
				if (currToken.getLexeme() != LEX_COMMA)
				{
					syntaxError(											// Syntax error 30
						"In '" + expr + "' statement: Expected ','",
						getNextPosition(prevToken)
					);
				}

				getToken();
				lexeme currLex;
				currLex = currToken.getLexeme();
				if (currLex != LEX_ID)
				{
					syntaxError(											// Syntax error 31
						"In '" + expr + "' statement: Expected variable of matching type",
						currToken
					);
				}
				currLex = lexer.tables.ids[currToken.getValue()].getDataType();
				if (currLex != inputVarType)
				{
					syntaxError(											// Syntax error 32
						"In '" + expr + "' statement: Input argument " + to_string(count) + " has a wrong format",
						currToken
					);
				}
				getToken();
				count++;
			}
			if (currToken.getLexeme() != LEX_RIGHT_PAREN)
			{
				syntaxError(												// Syntax error 33
					"In '" + expr + "' statement: Expected ')'",
					getNextPosition(prevToken)
				);
			}
			getToken();
			missingSemicolonCheck();
			getToken();
			break;
		}

		case LEX_LEFT_BRACE:												// Composite operator
			CODE_BLOCK();
			break;

		case LEX_RETURN:
			getToken();
			FIN();
			returnCheck();
			missingSemicolonCheck();
			getToken();
			break;

		case LEX_SEMICOLON:
			getToken();
			break;

		default:															// Assignment statement
			ASSIGN();
			break;
		}
	}
}

// Assignment statement analysis
void Parser::ASSIGN()
{
	if (currToken.getLexeme() != LEX_ID)
	{
		EXPR();
	}
	else
	{
		idDeclaredCheck(currToken);
		int idTableIndex = currToken.getValue();
		Position idPos = currToken.getStartPosition();
		getToken();
		if (															// if the current operation is assignment:
			currToken.getLexeme() == LEX_ASSIGN ||
			currToken.getLexeme() >= LEX_PLUS_ASSIGN && currToken.getLexeme() <= LEX_SLASH_ASSIGN
			) {
			lexer.tables.ids[idTableIndex].setUse(idPos.lineNum);
			getToken();
			EXPR();
			assignMatchingTypeCheck();
		}
		else
		{
			ungetToken();
			EXPR();
		}
	}
	missingSemicolonCheck();
	getToken();
}

// Expression analysis
void Parser::EXPR()
{
	DISJ();
	if (																// if the current operation is assignment:
		currToken.getLexeme() == LEX_ASSIGN ||
		currToken.getLexeme() >= LEX_PLUS_ASSIGN && currToken.getLexeme() <= LEX_SLASH_ASSIGN
		) {
		syntaxError(													// Syntax error 35
			"Lvalue required as a left operand of assignment",
			prevToken
		);
	}
}

void Parser::DISJ()
{
	CONJ();
	while (currToken.getLexeme() == LEX_OR)
	{
		tokenStack.push(currToken.getLexeme());
		getToken();
		CONJ();
		singleOperationCheck();
	}
}

void Parser::CONJ()
{
	CMP();
	while (currToken.getLexeme() == LEX_AND)
	{
		tokenStack.push(currToken.getLexeme());
		getToken();
		CMP();
		singleOperationCheck();
	}
}

void Parser::CMP()
{
	ADD();
	if (currToken.getLexeme() >= LEX_EQ && currToken.getLexeme() <= LEX_NOT_EQ)
	{
		tokenStack.push(currToken.getLexeme());
		getToken();
		ADD();
		singleOperationCheck();
	}
}

void Parser::ADD()
{
	MULTI();
	while (currToken.getLexeme() == LEX_PLUS || currToken.getLexeme() == LEX_MINUS)
	{
		tokenStack.push(currToken.getLexeme());
		getToken();
		MULTI();
		singleOperationCheck();
	}
}

void Parser::MULTI()
{
	FIN();
	while (currToken.getLexeme() >= LEX_TIMES && currToken.getLexeme() <= LEX_PERCENT)
	{
		tokenStack.push(currToken.getLexeme());
		getToken();
		FIN();
		singleOperationCheck();
	}
}

void Parser::FIN()
{
	switch (currToken.getLexeme())
	{
	case LEX_ID:
	{
		idDeclaredCheck(currToken);
		int idTableIndex = currToken.getValue();
		Position idPos = currToken.getStartPosition();
		lexer.tables.ids[idTableIndex].setUse(idPos.lineNum);
		getToken();
		if (currToken.getLexeme() == LEX_PLUS_PLUS || currToken.getLexeme() == LEX_MINUS_MINUS)
		{
			incrementCheck();
			getToken();
		}
		break;
	}
	case LEX_NUM:
		bool noDecimal;
		noDecimal = fmod(currToken.getValue(), 1.0) == 0;
		if (noDecimal)														// if the number has no decimal part
		{
			tokenStack.push(LEX_INT);							    		//     put integer type in the tokens stack
		}
		else if (typeid(currToken.getValue()) == typeid(float))				// else if the number is float
		{
			tokenStack.push(LEX_FLOAT);										//     put float type in the tokens stack
		}
		else
		{
			tokenStack.push(LEX_DOUBLE);									// else the number is double. Put double type in the tokens stack
		}
		getToken();
		break;

	case LEX_PLUS: case LEX_MINUS:
		getToken();
		FIN();
		unaryOperationCheck();
		break;

	case LEX_PLUS_PLUS: case LEX_MINUS_MINUS:
		getToken();
		if (currToken.getLexeme() != LEX_ID)
		{
			syntaxError(													// Syntax error 36
				"Lvalue required as an increment operand",
				currToken
			);
		}
		idDeclaredCheck(currToken);
		incrementCheck();
		getToken();
		break;

	case LEX_QUOTE_SINGLE: case LEX_QUOTE_DOUBLE:
		tokenStack.push(currToken.getLexeme() == LEX_QUOTE_SINGLE ? LEX_CHAR : LEX_STRING);
		getToken();															// get the constant
		getToken();															// get the terminating quotation mark (if missing, lexical error will be triggered)
		getToken();															// get the next token
		break;

	case LEX_TRUE: case LEX_FALSE:
		tokenStack.push(LEX_BOOL);											// true and false are bool => put bool in the tokens stack
		getToken();
		break;

	case LEX_NOT:
		getToken();
		FIN();
		notOperatorCheck();
		break;

	case LEX_LEFT_PAREN:
		getToken();
		EXPR();
		if (currToken.getLexeme() != LEX_RIGHT_PAREN)
		{
			syntaxError("Expected ')'", getNextPosition(prevToken));		// Syntax error 37
		}
		getToken();
		break;

	default:
		syntaxError("No matching operand found", currToken);				// Syntax error 38
		break;
	}
}


//.........................SEMANTIC ANALYSIS

// Set the variable's type and confirm that it has been declared
void Parser::setDataTypeForId()
{
	int idTableIndex = currToken.getValue();
	lexeme definedType;
	extract(idTypeStack, definedType);
	int idLineNum = currToken.getStartPosition().lineNum;

	lexer.tables.ids[idTableIndex].setDataType(definedType);				// assign the variable its type (which is kept in the end of the tokens stack)
	lexer.tables.ids[idTableIndex].declare(idLineNum);						// confirm the variable has been declared
	tokenStack.push(definedType);
}

// Check whether a used identifier was declared or not
void Parser::idDeclaredCheck(Token idLex)
{
	int tableIndex = idLex.getValue();
	if (lexer.tables.ids[tableIndex].isDeclared())							// if declared:
	{
		tokenStack.push(lexer.tables.ids[tableIndex].getDataType());		//   add it to the tokens stack
	}
	else																	// else:
	{
		auto idName = lexer.tables.ids[tableIndex].getName();
		semanticError("Variable '" + idName + "' has not been declared", idLex);//   Semantic error
		tokenStack.push(LEX_VOID);
	}
}

// Check whether all declared identifiers were used or not 
void Parser::idsUsedCheck()
{
	for (auto id : lexer.tables.ids)
	{
		if (id.isDeclared() && !id.isUsed())
		{
			semanticWarning(
				"Variable '" + id.getName() + "' declared but not used"
			);
		}
	}
}

// Single operation check
void Parser::singleOperationCheck()
{
	lexeme opLeft;															// left operand 
	lexeme opRight;															// right operand
	lexeme oper;															// operator

	lexeme opType;															// operation type
	lexeme resType;															// operation result type

	extract(tokenStack, opRight);
	extract(tokenStack, oper);
	extract(tokenStack, opLeft);

	if (opLeft == LEX_STRING && opLeft == opRight)
	{
		opType = LEX_STRING;
		if (oper == LEX_PLUS)
		{
			resType = LEX_STRING;
		}
		else if (oper >= LEX_EQ && oper <= LEX_NOT_EQ)
		{
			resType = LEX_BOOL;
		}
		else
		{
			semanticError("Invalid operator for variables of type 'char*'", oper);
		}
	}
	else
	{
		opType = opLeft;
		if (oper >= LEX_EQ && oper <= LEX_OR)
		{
			resType = LEX_BOOL;
		}
		else if (oper >= LEX_PLUS && oper <= LEX_PERCENT)
		{
			resType = LEX_DOUBLE;
		}
	}

	bool exception = (														// make exceptions for type mismatch
		opLeft >= LEX_INT && opLeft <= LEX_BOOL &&
		opRight >= LEX_INT && opRight <= LEX_BOOL ||
		opLeft == LEX_DOUBLE && opRight == LEX_FLOAT ||						//     assigning float to double
		opLeft == LEX_FLOAT && opRight == LEX_DOUBLE						//     assigning double to float
		);
	if (
		opLeft == opRight && opLeft == opType ||
		opType == LEX_BOOL && opLeft != LEX_STRING && opRight != LEX_STRING ||
		exception
		) {
		tokenStack.push(resType);
	}
	else
	{
		semanticError("Data types in the operation do not match", opRight);
	}
}

void Parser::unaryOperationCheck()
{
	lexeme operand = tokenStack.top();										// operand is kept at the top of the tokens stack
	if (
		operand != LEX_INT &&												// if the type of the operand is not integer or float or double:
		operand != LEX_FLOAT &&
		operand != LEX_DOUBLE
		) {
		semanticError("Invalid data type for unary operation", operand);	//   semantic error
	}
}

void Parser::incrementCheck()
{
	lexeme operand = tokenStack.top();
	if (operand != LEX_INT)													// if the type of the operand is not integer:
	{
		semanticError("Increment only allows int data type", operand);		//   semantic error
	}
}

// '!' operator check
void Parser::notOperatorCheck()
{
	lexeme operand = tokenStack.top();
	if (operand != LEX_BOOL)
	{
		semanticError("'!' statement only allows bool data type", operand);
	}
}

// Check of equality of variable type and statement type before assignment
void Parser::assignMatchingTypeCheck()
{
	lexeme valueType;														// statement (or right variable) type
	extract(tokenStack, valueType);											// extract it from the tokens stack

	bool exception = (														// make exceptions for type mismatch
		tokenStack.top() >= LEX_INT && tokenStack.top() <= LEX_BOOL && valueType >= LEX_INT && valueType <= LEX_BOOL ||
		tokenStack.top() == LEX_DOUBLE && valueType == LEX_FLOAT ||			//     assigning float to double
		tokenStack.top() == LEX_FLOAT && valueType == LEX_DOUBLE			//     assigning double to float
		);
	if (tokenStack.top() != valueType && !exception)
	{
		semanticError("Assigned data type does not match", valueType);
	}
}

// Check of statement type in conditions of if() / while() / for(;;) / do-while()
void Parser::conditionTypeCheck()
{
	if (tokenStack.top() == LEX_BOOL)										// must be bool
	{
		tokenStack.pop();
	}
	else
	{
		semanticError("The conditional expression must be bool", tokenStack.top());
	}
}

void Parser::returnCheck()
{
	lexeme valueType = tokenStack.top();
	if (valueType != funcType)
	{
		semanticError(
			"Data type of the returned value does not match the function type",
			currToken
		);
	}
}

// Get the next token from the tokenized source code
void Parser::getToken()
{
	prevToken = currToken;
	currToken = sourceCode.at(currTokenIndex);
	//currToken = lexer.makeToken();
	//sourceCode.push_back(currToken);

	currTokenIndex++;
	currPos = currToken.getEndPosition();
}

// Unget the current token
void Parser::ungetToken()
{
	currTokenIndex--;
	currToken = prevToken;
	prevToken = sourceCode.at(currTokenIndex - 1);
}

void Parser::missingSemicolonCheck()
{
	if (currToken.getLexeme() != LEX_SEMICOLON)
	{
		syntaxError(													//   Syntax error 39
			"Expected ';'",
			getNextPosition(prevToken)
		);
	}
}

// Syntax error processing
void Parser::syntaxError(string err, Token errToken)
{
	try
	{
		throw err;
	}
	catch (string s)
	{
		auto errLineNum = errToken.getStartPosition().lineNum;
		auto errColNum = errToken.getStartPosition().colNum;
		cout << fileName << ":" << errLineNum << ":" << errColNum
			<< ": Syntax error: " << s << "\n" << highlightError(errToken) << "\n";
		_getch();
		exit(1);
	}
}

// Semantic error processing
void Parser::semanticError(string err, Token errToken)
{
	try
	{
		throw err;
	}
	catch (string s)
	{
		auto errLineNum = errToken.getStartPosition().lineNum;
		auto errColNum = errToken.getStartPosition().colNum;
		cout << fileName << ":" << errLineNum << ":" << errColNum
			<< ": Semantic error: " << s << "\n" << highlightError(errToken) << "\n";
		semErrorCount++;
	}
}

// Semantic warning processing
void Parser::semanticWarning(string wrn)
{
	try
	{
		throw wrn;
	}
	catch (string s)
	{
		cout << "Semantic warning: " << s << "\n";
	}
}

// Highlight specific lexeme in a code line
string Parser::highlightError(Token token)
{
	ifstream code(fileName);
	string codeLine;

	auto lineNum = token.getStartPosition().lineNum;
	auto lexStart = token.getStartPosition().colNum;
	auto lexEnd = token.getEndPosition().colNum;

	for (int i = 0; i < lineNum; i++)
	{
		getline(code, codeLine);
	}

	string highlight = "^";
	highlight.insert(0, lexStart - 1, ' ');
	int lexLength = lexEnd - lexStart - 1;
	if (lexLength > 0)
	{
		highlight.insert(highlight.length(), lexLength, '~');
	}

	string lineNumStr = to_string(lineNum);
	string margin = "";
	margin.insert(0, lineNumStr.length(), ' ');


	return lineNumStr + " | " + codeLine + "\n" + margin + " | " + highlight + "\n";
}

// Constructor
Parser::Parser(string name) : fileName(name), lexer(name)
{
	currTokenIndex = 0;
	semErrorCount = 0;
}

parserResults Parser::analyze()
{
	lexer.tables.clearTables();
	sourceCode = lexer.convertToTokens();

	getToken();
	FUNC_HEADER();

	if (semErrorCount > 0)
	{
		_getch();
		exit(1);
	}

	parserResults res;
	res.sourceCode = sourceCode;
	res.tables = lexer.tables;
	return res;
}