#include "Token.h"

using namespace std;

//.........................POSITION IN A CODE FILE

// Constructor
Position::Position(unsigned int ln, unsigned int col) : lineNum(ln), colNum(col) {}


//.........................TOKEN CLASS

// Constructor
Token::Token(lexeme lex, double val, Position st, Position fn) : lex(lex), value(val), start(st), fin(fn) {}

// Get token's lexeme
lexeme Token::getLexeme() const
{
	return lex;
}

// Get token's value
double Token::getValue() const
{
	return value;
}

// Get position where token starts
Position Token::getStartPosition() const
{
	return start;
}

// Get position where token ends
Position Token::getEndPosition() const
{
	return fin;
}

// Print token attributes
ostream& operator << (ostream& out, Token t)
{
	out << '(' << t.lex << ", " << t.value
		<< ", st: " << t.start.lineNum << ":" << t.start.colNum
		<< ", fin: " << t.fin.lineNum << ":" << t.fin.colNum << ");";
	return out;
}


//.........................IDENTIFIER CLASS

// Constructor
Identifier::Identifier(const string idName) : name(idName)
{
	idtype = NO_TYPE;
	dataType = LEX_NULL;
	value = 0;
}

string Identifier::getName() const
{
	return name;
}

bool Identifier::isDeclared() const
{
	return !defLineNums.empty();
}

void Identifier::declare(int lineNum)
{
	defLineNums.push_back(lineNum);
}

idType Identifier::getIdType()
{
	return idtype;
}

bool Identifier::setIdType(idType newType)
{
	if (idtype == NO_TYPE)
	{
		idtype = newType;
		return true;
	}
	return false;
}

lexeme Identifier::getDataType() const
{
	return dataType;
}

bool Identifier::setDataType(lexeme newType)
{
	if (dataType == LEX_NULL)
	{
		dataType = newType;
		return true;
	}
	return false;
}

bool Identifier::isUsed() const
{
	return !useLineNums.empty();
}

void Identifier::setUse(int lineNum)
{
	useLineNums.push_back(lineNum);
}

int Identifier::getValue() const
{
	return value;
}

string Identifier::getStrValue() const
{
	return strValue;
}

void Identifier::setValue(int newValue)
{
	value = newValue;
}

void Identifier::setValue(string newStrValue)
{
	strValue = newStrValue;
}


//.........................IDENTIFIER AND STRING CONSTANTS TABLES CLASS

// Filling the identifier table with unique entries
int Tables::addUniqueId(const string idName)
{
	auto it = std::find_if(
		ids.begin(),
		ids.end(),
		[idName](Identifier id)
		{
			return id.getName() == idName;
		}
	);
	if (it != ids.end())												// if an identifier with this name is already present in the table:
	{
		return distance(ids.begin(), it);								// return its position in the table
	}
	ids.push_back(Identifier(idName));				  					// else: add the ID in the end of the table
	return ids.size() - 1;								    			// and return its position
}

// Filling the char constants table with unique entries
int Tables::addUniqueCharConst(const char c)
{
	auto it = find(charConsts.begin(), charConsts.end(), c);

	if (it != charConsts.end())            			 		    		// if the current string is already present in the table:
	{
		return distance(charConsts.begin(), it);   			   	      	// return its position in the table
	}
	charConsts.push_back(c);             						    	// else: add the string in the end of the table
	return charConsts.size() - 1;      						        	// and return its position
}

// Filling the sting constants table with unique entries
int Tables::addUniqueStrConst(const string str)
{
	auto it = find(strConsts.begin(), strConsts.end(), str);

	if (it != strConsts.end())            			 		    		// if the current string is already present in the table:
	{
		return distance(strConsts.begin(), it);   			   	      	// return its position in the table
	}
	strConsts.push_back(str);             						    	// else: add the string in the end of the table
	return strConsts.size() - 1;      						        	// and return its position
}

// Clear both tables
void Tables::clearTables()
{
	ids.clear();
	strConsts.clear();
}