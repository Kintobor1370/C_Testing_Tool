#include "Lexer.h"

using namespace std;

//.........................SCANNER CLASS
vector<string> Lexer::keywordTableStr =
{
	"int",
	"float",
	"double",
	"bool",
	"char",
	"char*",
	"void",

	"if",
	"else",
	"switch",
	"case",
	"break",
	"continue",

	"true",
	"false",

	"for",
	"while",
	"do",

	"scanf",
	"print",
	"printf",

	"return"
};

vector<string> Lexer::delimTableStr =
{
	"{",
	"}",

	"\'",
	"\"",
	";",
	",",
	":",
	"=",

	"+",
	"-",
	"*",
	"/",
	"%",

	"++",
	"--",

	"+=",
	"-=",
	"*=",
	"/=",

	"(",
	")",

	"!",
	"==",
	">",
	"<",
	">=",
	"<=",
	"!=",
	"&&",
	"||",
};

// Open model language program file for reading
void Lexer::openFile(const string fileName)
{
	f = ifstream(fileName);
	if (!f.is_open())
	{
		cout << "ERROR: could not open file " + fileName + "\n";
		_getch();
		exit(1);
	}
}

// Clear buffer
void Lexer::clearBuffer()
{
	bufferTop = 0;
	buffer.clear();
}

// Add the current character to buffer
void Lexer::addToBuffer()
{
	buffer.push_back(currChar);
	bufferTop++;
}

// Add a different character to buffer
void Lexer::addToBuffer(char c)
{
	buffer.push_back(c);
	bufferTop++;
}

// Check if a buffered string is present in the lexeme table
int Lexer::checkLexeme(const string& buf, vector<string> strTable)
{
	int index = 0;
	auto it = find(strTable.begin(), strTable.end(), buf);			// find buffered string in the lexeme table
	if (it != strTable.end())										// if buffered string is present in the table
	{
		index = it - strTable.begin() + 1;							// return its position in the table
	}
	return index;													// return 0 if buffered string is not present
}

// Reading the next character of a model language program
void Lexer::getChar()
{
	lexEnd = currPos;
	currChar = f.get();
	if (currChar == '\n')
	{
		currPos.lineNum++;
		linesLen.push_back(currPos.colNum);
		currPos.colNum = 1;
	}
	else
	{
		currPos.colNum++;
	}
	//cout << currChar << " " << currPos.lineNum << ":" << currPos.colNum << "\n";
}

// Pushing the current character back into the input stream
void Lexer::ungetChar()
{
	f.unget();
	if (currPos.colNum == 1)
	{
		currPos.lineNum--;
		currPos.colNum = linesLen.at(currPos.lineNum - 1);
	}
	else
	{
		f.unget();
		currPos.colNum--;
	}
}

// Lexical error processing
void Lexer::lexicalError(string err)
{
	try
	{
		throw err;
	}
	catch (string token)
	{
		auto errStart = lexStart;
		auto errEnd = lexEnd;
		string errMsg;
		switch (token[0])
		{
		case '\\':
			errMsg = "Unknown escape sequence: \'" + token + "\'";
			errStart = errEnd;
			errStart.colNum--;
			errEnd.colNum += token.length() - 1;
			break;

		case '\'': case '\"':
			errMsg = "Missing terminating " + string(1, token[0]) + " character";
			break;

		case '_':
			errMsg = "Empty char constant";
			break;

		default:
			break;
		}
		cout << fileName << ":" << currPos.lineNum << ":" << currPos.colNum
			<< ": Lexical error: " << errMsg << "\n" << highlightError(errStart, errEnd) << "\n";
		_getch();
		exit(1);
	}
}

string Lexer::highlightError(Position errStart, Position errEnd)
{
	ifstream code(fileName);
	string codeLine;

	auto lineNum = errStart.lineNum;
	auto startCol = errStart.colNum;
	auto endCol = errEnd.colNum;

	for (int i = 0; i < lineNum; i++)
	{
		getline(code, codeLine);
	}

	string highlight = "^";
	highlight.insert(0, startCol - 1, ' ');
	int lexLength = endCol - startCol - 1;
	if (lexLength > 0)
	{
		highlight.insert(highlight.length(), lexLength, '~');
	}

	string lineNumStr = to_string(lineNum);
	string margin = "";
	margin.insert(0, lineNumStr.length(), ' ');


	return lineNumStr + " | " + codeLine + "\n" + margin + " | " + highlight + "\n";
}

Lexer::Lexer(const string name) : fileName(name)
{
	openFile(fileName);
	currState = INIT;
	currPos.lineNum = 1;
	currPos.colNum = 1;
	charWasRead = false;
	getChar();
}

bool Lexer::addFunctions(vector<std::function<double(double)>> funcs, vector<string> funcsStr)
{
	if (funcs.size() == funcsStr.size())
	{
		funcTable.insert(funcTable.end(), funcs.begin(), funcs.end());
		funcTableStr.insert(funcTableStr.end(), funcsStr.begin(), funcsStr.end());
		return true;
	}
	return false;
}

Token Lexer::makeToken()
{
	clearBuffer();
	lexStart = lexEnd;
	int number = 0;													// an integer value, encountered in source code
	int decimal = 0;												// an after decimal value, encountered in source code
	int numberDigitsCount = 0;										// number of digits in the number
	int decimalDigitsCount = 0;										// number of digits after decimal
	int tokenVal;													// value of the current token

	do
	{
		switch (currState)
		{
		case INIT:													// Initial state:
			if (
				currChar == ' ' ||									//   if the character is space/end of the line/new line:
				currChar == '\n' ||
				currChar == '\r' ||
				currChar == '\t'
				) {
				getChar();
				lexStart = lexEnd;
			}
			else if (isalpha(currChar))								//   if the character is an identifier:									
			{
				clearBuffer();
				addToBuffer();
				currState = ID;
				getChar();
			}
			else if (isdigit(currChar))								//   if the character is a number:
			{
				number = currChar - '0';
				currState = NUMBER;
				getChar();
			}
			else if (currChar == '.')
			{
				number = 0;
				currState = DECIMAL;
				getChar();
			}
			else if (currChar == '\'')
			{
				addToBuffer();
				currState = CHAR;
				getChar();
				tokenVal = checkLexeme(buffer, delimTableStr);
				return Token(LEX_QUOTE_SINGLE, tokenVal, lexStart, lexEnd);
			}
			else if (currChar == '\"')
			{
				addToBuffer();
				currState = STRING;
				getChar();
				tokenVal = checkLexeme(buffer, delimTableStr);
				return Token(LEX_QUOTE_DOUBLE, tokenVal, lexStart, lexEnd);
			}
			else if (currChar == '/')                               //   if the character is start of a comment:
			{
				clearBuffer();
				addToBuffer();
				getChar();
				switch (currChar)
				{
				case '*':									        //     in case of multiple-line comment:
					clearBuffer();
					currState = COMMENT;
					getChar();
					break;

				case '/':											//     in case of one-line comment:
					clearBuffer();
					currState = COMMENT_STRING;
					getChar();
					break;

				default:											//     in case of not a comment:
					currState = DELIM;								//       return to analysing '/' as delimeter
					break;
				}
			}
			else if (currChar == '!')                               //   if the character is 'not equal' sign:
			{
				clearBuffer();
				addToBuffer();
				currState = NOT_EQ;
				getChar();
			}
			else if (currChar == EOF)                               //   if the character is end of file:
			{
				currState = FIN;
			}
			else                                                    //    else: delimeter
			{
				clearBuffer();
				addToBuffer();
				currState = DELIM;
			}
			break;

		case ID:													// Identifier state:
			tokenVal = checkLexeme(buffer, keywordTableStr);
			if (isalpha(currChar) || isdigit(currChar))             //   if the character is alphabetic or a number:
			{
				addToBuffer();							            //     add it to the buffer as a part of an identifier
				getChar();
			}
			else                                                    //   else: the identifier is finalised 
			{
				currState = INIT;									//     switch back to the initial state
				if (tokenVal)										//     if identifier in buffer is a functional word:
				{
					if ((lexeme)tokenVal == LEX_CHAR)
					{
						addToBuffer();
						tokenVal = checkLexeme(buffer, keywordTableStr);
						if ((lexeme)tokenVal == LEX_STRING)
						{
							getChar();
						}
						else
						{
							return Token(LEX_CHAR, (int)LEX_CHAR, lexStart, lexEnd);
						}
					}
					return Token((lexeme)tokenVal, tokenVal, lexStart, lexEnd);//       return its lexeme
				}
				else                                                //     else:
				{
					tokenVal = tables.addUniqueId(buffer);			//       add the identifier to the IDs table
					return Token(LEX_ID, tokenVal, lexStart, lexEnd);
				}
			}
			break;

		case NUMBER:                                                // Number state
			if (isdigit(currChar))                                  //   if the character is a digit:
			{
				int newDigit = currChar - '0';
				number = 10 * number + newDigit;                    //     make it continue the number
				numberDigitsCount++;								//	   count the number of integer digits
				getChar();
			}
			else if (currChar == '.')
			{
				currState = DECIMAL;
				getChar();
			}
			else                                                    //   else: the number is finalised
			{
				currState = INIT;
				return Token(LEX_NUM, number, lexStart, lexEnd);	//     return the number as a lexeme
			}
			break;

		case DECIMAL:
			if (isdigit(currChar))									//   if the character is a digit:
			{
				int newDigit = currChar - '0';
				decimal = 10 * decimal + newDigit;                  //     make it continue the number after decimal
				decimalDigitsCount++;								//	   count the number of digits after decimal
				getChar();
			}
			else
			{
				double finalNumber = number + decimal / pow(10, decimalDigitsCount);
				currState = INIT;
				auto newLex = Token(LEX_NUM, finalNumber, lexEnd);
				return Token(LEX_NUM, finalNumber, lexStart, lexEnd);//     return the number as a lexeme
			}
			break;

		case CHAR:													// Char state
			clearBuffer();
			switch (currChar)
			{
			case '\\':												//   if the character is a control character:
				getChar();
				switch (currChar)									//     checkLexeme the next character
				{
				case 'n':											//      new line character
					addToBuffer('\n');
					break;

				case '0':											//		end of line character
					addToBuffer('\0');
					break;

				case 'r':											//      carriage return
					addToBuffer('\r');
					break;

				case 't':											//      horizontal tab character
					addToBuffer('\t');
					break;

				case '\\': case '\'': case '\"': case '\?': case '\%':// standard characters
					addToBuffer();
					break;

				default:											//      wrong control character
					string wrongToken = "\\" + string(1, currChar);
					lexicalError(wrongToken);
					break;
				}
				break;

			case '\n':												//   if the character is an end of line:
				if (charWasRead)
					lexicalError("\'");									//     lexical error: no terminating quotation mark
				break;

			case '\'':
				if (charWasRead)									//   if char has already been read, the terminating quotation mark is met
				{
					charWasRead = false;
					addToBuffer();									//     add it to the buffer 
					getChar();
					currState = INIT;								//     go out of the char state
					tokenVal = checkLexeme(buffer, delimTableStr);
					return Token(LEX_QUOTE_SINGLE, tokenVal, lexStart, lexEnd);//   return the terminating quotation mark
				}
				lexicalError("_");									//   else: the char constant is empty. Throw a lexical error
				break;

			default:												//    default: the character is a char
				charWasRead = true;
				tokenVal = tables.addUniqueCharConst(currChar);		//   add the char to the identifiers table
				getChar();
				return Token(LEX_CHAR_CONST, tokenVal, lexStart, lexEnd);
				break;
			}
			break;

		case STRING:												// String state
			clearBuffer();
			if (currChar != '\"')									// if the character is NOT a terminating quotation mark
			{
				while (currChar != '\"')							//   consider each next character a part of string until a terminating quotation mark is met
				{
					if (currChar == '\\')							//   if the character is a control character:
					{
						getChar();
						switch (currChar)							//     checkLexeme the next character
						{
						case 'n':									//      new line character
							addToBuffer('\n');
							break;

						case '0':									//		end of line character
							addToBuffer('\0');
							break;

						case 'r':									//      carriage return
							addToBuffer('\r');
							break;

						case 't':									//      horizontal tab character
							addToBuffer('\t');
							break;

						case '\\': case '\'': case '\"': case '\?': case '\%':// standard characters
							addToBuffer();
							break;

						case '\n':									//	    string's continuation in the next line of the code
							getChar();
							while (currChar == '\t')				//		while the character is not horizontal tab used for code allignment:	
							{
								getChar();							//        get next character
							}
							ungetChar();							//		unget character that is not a tab
							break;

						default:									//      wrong control character
							string wrongToken = "\\" + string(1, currChar);
							lexicalError(wrongToken);
							break;
						}
					}
					else if (currChar == '\n')						//   else: if the character is an end of line:
					{
						lexicalError("\"");							//     lexical error: no terminating quotation mark
					}
					else											//    else: the character is a part of string
					{
						addToBuffer();								//      so add it to the buffer
					}
					getChar();
				}													//   when the terminating quotation mark is met, the string is complete
				tokenVal = tables.addUniqueStrConst(buffer);			//   add the completed string to the identifiers table
				return Token(LEX_STR_CONST, tokenVal, lexStart, lexEnd);
			}
			else													// if the character is a terminating quotation mark
			{
				addToBuffer();
				getChar();
				currState = INIT;									//   go out of the string state
				tokenVal = checkLexeme(buffer, delimTableStr);
				return Token(LEX_QUOTE_DOUBLE, tokenVal, lexStart, lexEnd); //   return the treminating quotation mark
			}
			break;

		case COMMENT:                                               // Multiple-line comment state
			addToBuffer();
			if (currChar == '*')
			{
				getChar();
				if (currChar == '/')
				{
					addToBuffer();
					getChar();
					currState = INIT;
					lexStart = lexEnd;
				}
			}
			else if (currChar == EOF)
			{
				currState = FIN;
			}
			else
			{
				getChar();
			}
			break;

		case COMMENT_STRING:                                        // One-line comment state
			while (currChar != '\n' && currChar != EOF)
			{
				getChar();
			}
			if (currChar == EOF)
			{
				currState = FIN;
			}
			else
			{
				getChar();
				currState = INIT;
				lexStart = lexEnd;
			}
			break;

		case NOT_EQ:                                                // 'not equal' sign state
			if (currChar == '=')                                    //   if the character is '='
			{
				addToBuffer();
				currState = INIT;
				getChar();
				tokenVal = checkLexeme(buffer, delimTableStr);
				return Token(LEX_NOT_EQ, tokenVal, lexStart, lexEnd);
			}
			else                                                    //   else: lexical error
			{
				currState = INIT;
				tokenVal = checkLexeme(buffer, delimTableStr);
				return Token(LEX_NOT, tokenVal, lexStart, lexEnd);
			}
			break;

		case DELIM:                                                 // Delimeter state:
			char first;
			first = currChar;
			getChar();
			char second;
			second = currChar;

			// Composite delimeter analysis
			if ((
				first == '+' ||										// "+="
				first == '-' ||										// "-="
				first == '*' ||										// "*="
				first == '/' ||										// "/="
				first == '>' ||										// ">="
				first == '<' ||										// "<="
				first == '=')										// "=="
				&& second == '=' ||
				(first == '+' || first == '-')
				&& second == first ||								// "++" or "--"
				first == '&' && second == '&' ||					// "&&"
				first == '|' && second == '|'						// "||"
				) {
				addToBuffer(second);
				getChar();
			}
			currState = INIT;
			tokenVal = checkLexeme(buffer, delimTableStr);
			if (tokenVal)
			{
				return Token((lexeme)(tokenVal + (int)LEX_FIN), tokenVal, lexStart, lexEnd);
			}
			else
			{
				lexicalError("'" + buffer + "'");
			}
			break;

		case FIN:
			return Token(LEX_FIN, 0, lexStart, lexEnd);
			break;
		}
	} while (true);
};

vector<Token> Lexer::convertToTokens()
{
	vector<Token> convertedSourceCode;
	Token newToken = makeToken();
	convertedSourceCode.push_back(newToken);
	while (newToken.getLexeme() != LEX_FIN)
	{
		newToken = makeToken();
		convertedSourceCode.push_back(newToken);
	}
	return convertedSourceCode;
}