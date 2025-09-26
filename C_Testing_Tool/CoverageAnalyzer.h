#ifndef COVERAGEANALYZER_H
#define COVERAGEANALYZER_H

#include "Parser.h"
#include "CFG.h"
#include "Solver.h"
#include <unordered_set>
#include <iomanip>

class CoverageAnalyzer
{
    CFG cfg;
    vector<Identifier> ids;
    vector<pair<Path, TestCase>> pathsAndCasesTable;
    unordered_set<int> visitedNodes;
    vector<pair<int, int>> visitedBranches;

    // Detect unreachable code lines
    void findDeadCodeNodes();

    // Perform statement coverage
    void C0();

    // Perform branch coverage
    void C1();

    // Print table of coverage analysis results
    void printTable(vector<TestCase> testSuite);

public:
    CoverageAnalyzer(string fileName, int maxIterForLoops = 10000);

    void analyze(int testOption);
};

#endif