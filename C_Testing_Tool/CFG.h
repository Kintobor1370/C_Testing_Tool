#ifndef CFG_H
#define CFG_H

#include "Parser.h"
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

using namespace std;

using CodeLine = vector<Token>;

struct Edge;

//.........................CONTROL FLOW GRAPH NODE
struct Node
{
    int id;
    CodeLine code;
    vector<Edge> edges;

    Node(int id = -1, CodeLine code = {});
};

//.........................EDGE
struct Edge
{
    int idTarget;                                               // id of the following node
    CodeLine condition;                                         // condition code line. Empty if unconditional

    Edge(int id, CodeLine cnd = {});
};


//.........................CONTROL FLOW GRAPH
class CFG
{
public:
    unordered_map<int, Node> nodes;
    int idEntry;

    CFG();

    bool findNode(int id) const;
    void addNode(int id, CodeLine code);
    void addEdge(int fromId, int toId, CodeLine condition = {});
    void printGraph() const;
};


//.........................CONTROL FLOW GRAPH BUILDER CLASS
class CFGBuilder
{
    vector<Token> codeBody;                                     // Source code body
    CFG cfg;

    Token currToken;                                            // Current token
    int currLex;                                                // Current token's lexeme
    int currId;                                                 // Current CFG node ID
    int prevId;                                                 // Previous CFG node ID
    bool isBranch;                                              // indicator of branch

    stack<CodeLine> cndStack;                                   // Conditions stack for adding conditions to necessary edges
    vector<tuple<int, int, CodeLine>> potentialEdges;           // Table of potential edges (connecting final nodes of condition branches to the code lines after the branching finishes)

    CodeLine eraseFunctionHeader(vector<Token> prog);
    void getToken();
    void ungetToken(Token prevToken);

    CodeLine extractFromCndStack();
    CodeLine makeCondition();
    CodeLine negateCondition(CodeLine cnd);

    void addNodeAndEdge(CodeLine line, int prevId);
    void addPotentialEdges();

    bool breakdownCodeBlock();
    bool breakdownSingleLine();
    void breakdownBranch(CodeLine cnd, int branchId);
    void breakdownEntryLoop(CodeLine cnd, int branchId);
    void breakdownExitLoop(int branchId);
    pair<CodeLine, CodeLine> breakdownForLoopParameters();

public:
    CFGBuilder(vector<Token> sourceCode);

    CFG buildCFG();
};

#endif