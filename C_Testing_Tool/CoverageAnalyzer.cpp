#include "CoverageAnalyzer.h"

using namespace std;

// Check whether path2 contains all nodes of path1
bool checkIfContainsAllNodes(Path path1, Path path2)
{
    bool contains = true;
    for (auto& path1Node : path1)
    {
        int currId = path1Node.id;
        auto it = std::find_if(
            path2.begin(),
            path2.end(),
            [currId](Node path2Node)
            {
                return path2Node.id == currId;
            }
        );
        if (it == path2.end())
        {
            contains = false;
        }
    }
    return contains;
}

// Check whether path1 and path2 are identical
bool checkEqualPaths(Path path1, Path path2)
{
    return checkIfContainsAllNodes(path1, path2) && checkIfContainsAllNodes(path2, path1);
}

// Check if two code lines are identical
bool compareCodelines(CodeLine line1, CodeLine line2)
{
    if (line1.size() != line2.size())
    {
        return false;
    }
    for (int i = 0; i < line1.size(); i++)
    {
        auto token1 = line1.at(i);
        auto token2 = line2.at(i);
        if (
            token1.getLexeme() != token2.getLexeme() ||
            token1.getValue() != token2.getValue() ||
            token1.getStartPosition().lineNum != token2.getStartPosition().lineNum ||
            token1.getStartPosition().colNum != token2.getStartPosition().colNum ||
            token1.getEndPosition().lineNum != token2.getEndPosition().lineNum ||
            token1.getEndPosition().colNum != token2.getEndPosition().colNum
            ) {
            return false;
        }
    }
    return true;
}


//.........................COVERAGE ANALYZER CLASS

// Detect unreachable code lines
void CoverageAnalyzer::findDeadCodeNodes()
{
    int idMax = cfg.nodes.size();
    vector<int> deadCodeNodes;
    for (int id = 1; id <= idMax; id++)
    {
        auto it = find(visitedNodes.begin(), visitedNodes.end(), id);
        if (it == visitedNodes.end())
        {
            deadCodeNodes.push_back(id);
        }
    }
    vector<int> lineNums;
    for (auto& id : deadCodeNodes)
    {
        Node currNode = cfg.nodes[id];
        for (auto& token : currNode.code)
        {
            int lexEndLineNum = token.getEndPosition().lineNum;
            if (find(lineNums.begin(), lineNums.end(), lexEndLineNum) == lineNums.end())
            {
                lineNums.push_back(lexEndLineNum);
            }
        }
    }

    cout << "Warning: Unreachable code detected in" << (lineNums.size() > 1 ? " lines " : " line ") << lineNums.front();
    for (int i = 1; i < lineNums.size(); i++)
    {
        cout << ", " << lineNums.at(i);
    }
    cout << "\n";
}

// Perform statement coverage
void CoverageAnalyzer::C0()
{
    if (cfg.nodes.empty())
    {
        cout << "Your function is empty!";
        return;
    }

    vector<pair<int, Path>> testCases;
    for (int tableIndex = 0; tableIndex < pathsAndCasesTable.size(); tableIndex++)
    {
        auto currPath = pathsAndCasesTable.at(tableIndex).first;
        bool isFeasible = pathsAndCasesTable.at(tableIndex).second.isFeasible();
        if (isFeasible)
        {
            // Check if the current path contains all nodes of an existing test case
            // Since each path is longer or equal than the previous one, there cannot be the case
            // that the current path is contained in any previous path
            bool containsAllNodes = false;
            int pathIndex = 0;
            while (pathIndex < testCases.size() && !containsAllNodes)
            {
                auto testCase = testCases.at(pathIndex).second;
                containsAllNodes = checkIfContainsAllNodes(testCase, currPath);
                ++pathIndex;
            }
            // If the current path contains all nodes of an existing test case, this test case is redundant
            if (containsAllNodes)
            {
                testCases.at(pathIndex - 1) = make_pair(tableIndex, currPath);  // replace this test case with the current path
            }
            else
            {
                testCases.push_back(make_pair(tableIndex, currPath));           // else: add current path to test cases without replacement
            }
        }
    }
    for (auto& entry : testCases)                                               // Collect visited nodes to compute coverage rate
    {
        auto currPath = entry.second;
        for (auto& node : currPath)
        {
            if (!visitedNodes.count(node.id))
            {
                visitedNodes.insert(node.id);
            }
        }
    }
    double coverageRate = 100.0 * visitedNodes.size() / cfg.nodes.size();
    cout << fixed << setprecision(2) << "Statement coverage: " << coverageRate << "%\n";

    vector<TestCase> testSuite;
    for (int i = 0; i < testCases.size(); i++)
    {
        int k = 0;
        while (!checkEqualPaths(pathsAndCasesTable.at(k).first, testCases.at(i).second))
        {
            k++;
        }
        auto model = pathsAndCasesTable.at(k).second;
        testSuite.push_back(model);
    }
    printTable(testSuite);
	cout << "Result: ";
    if (coverageRate == 100.0)
    {
        cout << "Full coverage!\n";
    }
    else
    {
        findDeadCodeNodes();
    }
}

// Perform branch coverage
void CoverageAnalyzer::C1()
{
    vector<pair<int, int>> edges;
    for (auto& idAndNode : cfg.nodes)
    {
        auto node = idAndNode.second;
        for (auto& edge : node.edges)
        {
            edges.push_back(make_pair(node.id, edge.idTarget));
        }
    }

    visitedBranches = {};
    vector<TestCase> testSuite;
    for (auto& entry : pathsAndCasesTable)
    {
        auto currPath = entry.first;
        auto testCase = entry.second;
        if (testCase.isFeasible())
        {
            for (int i = 0; i < currPath.size() - 1; i++)
            {
                auto node = currPath.at(i);
                auto nextNode = currPath.at(i + 1);
                auto it = find_if(
                    visitedBranches.begin(),
                    visitedBranches.end(),
                    [node, nextNode](pair<int, int> currEdge)
                    {
                        return currEdge.first == node.id && currEdge.second == nextNode.id;
                    }
                );
                if (it == visitedBranches.end())
                {
                    visitedBranches.push_back(make_pair(node.id, nextNode.id));
                }
            }
            testSuite.push_back(testCase);
        }
    }
    double coverageRate = 100.0 * visitedBranches.size() / edges.size();
    cout << fixed << setprecision(2) << "Branch coverage " << coverageRate << "%\n";
    printTable(testSuite);
    if (coverageRate == 100.0)
    {
        cout << "Full coverage!\n";
    }
}

// Print table of coverage analysis results
void CoverageAnalyzer::printTable(vector<TestCase> testSuite)
{
    vector<vector<string>> table;

    string caseStr = "Test case:";
    string paramsStr = "Function parameters:";
    string outputStr = "Function output:";

    vector<string> header{ caseStr, paramsStr, outputStr };
    table.push_back(header);
    int caseNum = 1;

    for (auto testCase : testSuite)                               // Add data from test suite to table
    {
        vector<string> row{ to_string(caseNum) };
        string inputParams;
        for (int i = 0; i < testCase.getSize(); i++)
        {
            inputParams = inputParams + testCase.getId(i).getName() + " = " + testCase.getIdValue(i) + "; ";
        }
        row.push_back(inputParams);
        row.push_back(testCase.getOutput());
        table.push_back(row);
        caseNum++;
    }

    vector<int> longestEntriesPerColumn{ 0, 0, 0 };             // Find the longest string entries in each column of the table
    for (auto& row : table)
    {
        for (int i = 0; i < row.size(); i++)
        {
            if (row.at(i).length() > longestEntriesPerColumn.at(i))
            {
                longestEntriesPerColumn.at(i) = row.at(i).length();
            }
        }
    }

    for (auto& row : table)                                     // Center all entries in the table 
    {                                                           // according to the longest one in their column
        for (int i = 0; i < row.size(); i++)
        {
            int diff = longestEntriesPerColumn.at(i) - row.at(i).length();
            int leftMarginSize = diff / 2;
            int rightMarginSize = diff - leftMarginSize;

            row.at(i).insert(0, leftMarginSize, ' ');
            row.at(i).insert(row.at(i).length(), rightMarginSize, ' ');
        }
    }

    string bar = "++++";                              // Horizontal bar dividing table headers from the rows
    int shift = 3;
    for (auto& margin : longestEntriesPerColumn)
    {
        bar.insert(bar.length() - shift, margin, '-');
        shift--;
    }

    header = table.front();                                     // Print the header
    string pipe = "|";
    cout << bar << "\n" << pipe;
    for (auto& entry : header)
    {
        cout << entry << pipe;
    }
    cout << "\n" << bar << "\n";

    for (int i = 1; i < table.size(); i++)                      // Print the table
    {
        auto row = table.at(i);
        cout << pipe;
        for (auto& entry : row)
        {
            cout << entry << pipe;
        }
        cout << "\n";
    }
    cout << bar << "\n";
}

// Constructor
CoverageAnalyzer::CoverageAnalyzer(string fileName, int maxIterForLoops)
{
    Parser parser(fileName);

    auto res = parser.analyze();

	CFGBuilder builder(res.sourceCode);
    cfg = builder.buildCFG();

    Solver solver(
        cfg,
        res.tables.ids,
        res.tables.charConsts,
        res.tables.strConsts
    );
    pathsAndCasesTable = solver.getPathsAndCases();                     // Import all paths and models from the SMT solver
    std::sort(                                                          // Sort them by path length    
        pathsAndCasesTable.begin(),
        pathsAndCasesTable.end(),
        [](pair<Path, TestCase> entry1, pair<Path, TestCase> entry2)
        {
            auto entry1Path = entry1.first;
            auto entry2Path = entry2.first;
            return entry1Path.size() < entry2Path.size();
        }
    );
}

// Print coverage results
void CoverageAnalyzer::analyze(int testOption)
{
    switch (testOption)
    {
    case 0:
        C0();
        break;

    case 1:
        C1();
        break;

    default:
        break;
    }
}