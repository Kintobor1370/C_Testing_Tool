#ifndef SOLVER_H
#define SOLVER_H

#define DEFAULT_NUM_VALUE 0
#define DEFAULT_CHAR_VALUE '\0'

#include "Parser.h"
#include "CFG.h"
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <z3++.h>

using namespace z3;

using Path = std::vector<Node>;

// Symbolic environment for variables
struct SymbolicContext
{
    context ctx;
    std::vector<std::pair<int, expr>> vars;

    SymbolicContext();

    void importVars(vector<Identifier> ids);
    void resetVars(vector<Identifier> ids);

    int getIndex(int val) const;

    expr getExpr(int val) const;
    void setExpr(int val, expr newExpr);
};

// Test case class
class TestCase
{
    bool sat;
    std::vector<Identifier> ids;
    std::vector<string> values;
    std::string output;

public:
    TestCase();

    TestCase(
        std::vector<Identifier> funcParams,
        expr_vector evals,
        expr returnVar
    );

    bool isFeasible();

    int getSize();

    Identifier getId(int index);

    string getIdValue(int index);

    string getOutput();
};

// SMT Solver class
class Solver
{
    CFG cfg;
    SymbolicContext sym;
    std::vector<Identifier> ids;
    std::vector<Path> paths;
    std::vector<TestCase> testSuite;
    std::vector <std::pair<int, lexeme>> unaryOpTable;
    int maxIterForLoops;

    void collectPaths(int id, Path currPath, std::set<int> visitedNodes, int loopIterCount);
    Token getLexeme(const std::vector<Token>& line, int& index, lexeme& type);
    void ungetLexeme(const std::vector<Token>& line, int& index, lexeme& type);

    void solveAssign(const std::vector<Token>& code, int& index, lexeme& type);
    expr solveCondition(const std::vector<Token>& cnd);
    void solveLoop(const Path& path, solver& solver, int& currNodeIndex);

    expr STMNT(const std::vector<Token>& line, int& currIndex, lexeme& currType);
    expr DISJ(const std::vector<Token>& line, int& currIndex, lexeme& currType);
    expr CONJ(const std::vector<Token>& line, int& currIndex, lexeme& currType);
    expr CMP(const std::vector<Token>& line, int& currIndex, lexeme& currType);
    expr ADD(const std::vector<Token>& line, int& currIndex, lexeme& currType);
    expr MULTI(const std::vector<Token>& line, int& currIndex, lexeme& currType);
    expr FIN(const std::vector<Token>& line, int& currIndex, lexeme& currType);

    void checkUnaryOperation(const std::vector<Token>& line, int& currIndex, lexeme& currType, int idValue);
    void executeUnaryOperations();

    TestCase evaluatePathConstraints(const Path& path, solver& solver, bool debugPrint);
    void debugPrintPaths();

public:
    Solver(CFG currCfg, vector<Identifier> ids, int maxIter = 10);

    void setMaxIterForLoops(int newMaxIter);

    void checkAllPaths(bool debug);

    std::vector<std::pair<Path, TestCase>> getPathsAndCases();
};

#endif