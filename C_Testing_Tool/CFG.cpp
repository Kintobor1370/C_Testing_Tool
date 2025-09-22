#include "CFG.h"

using namespace std;

//.........................CONTROL FLOW GRAPH NODE
Node::Node(int id, CodeLine code) : id(id), code(code) {}


//.........................EDGE
Edge::Edge(int id, CodeLine cnd) : idTarget(id), condition(cnd) {}


//.........................CONTROL FLOW GRAPH

// Constructor
CFG::CFG()
{
    idEntry = -1;
}

// Find node by its ID
bool CFG::findNode(int id) const
{
    auto it = nodes.find(id);
    return it == nodes.end() ? false : true;
}

// Add node to CFG
void CFG::addNode(int id, CodeLine code)
{
    idEntry = nodes.empty() ? id : idEntry;
    nodes[id] = Node(id, code);
}

// Add edge to CFG: for existing nodes only!
void CFG::addEdge(int idFrom, int idTo, CodeLine cnd)
{
    if (findNode(idFrom) && findNode(idTo))
    {
        nodes[idFrom].edges.push_back(Edge(idTo, cnd));
    }
}

// Print resulting graph
void CFG::printGraph() const
{
    cout << nodes.size() << " NODES\n";
    for (auto& node : nodes)
    {
        cout << "Node " << node.first << ": ";
        for (auto& token : node.second.code)
        {
            cout << token.getLexeme() << ", ";
        }
        cout << "\n";
        for (auto& edge : node.second.edges)
        {
            cout << "  --> Node " << edge.idTarget;
            if (!edge.condition.empty())
            {
                cout << " [Condition: ";
                for (auto& token : edge.condition)
                {
                    cout << token.getLexeme() << ", ";
                }
                cout << "]";
            }
            cout << "\n";
        }
    }
}


//.........................CONTROL FLOW GRAPH BUILDER CLASS

// Erase function header from the source code
CodeLine CFGBuilder::eraseFunctionHeader(vector<Token> prog)
{
    int leftBraceIndex = 0;
    while (prog.at(leftBraceIndex).getLexeme() != LEX_LEFT_BRACE)
    {
        leftBraceIndex++;
    }
    auto itFuncBodyStart = prog.begin() + leftBraceIndex;
    prog.erase(prog.begin(), itFuncBodyStart);
    return prog;
}

// Get next token
void CFGBuilder::getToken()
{
    currToken = codeBody.front();
    currLex = currToken.getLexeme();
    codeBody.erase(codeBody.begin());
}

// Return lexeme to vectorized function body
void CFGBuilder::ungetToken(Token prevToken)
{
    codeBody.insert(codeBody.begin(), currToken);
    currToken = prevToken;
    currLex = currToken.getLexeme();
}

// Extract next condition from the conditions stack
CodeLine CFGBuilder::extractFromCndStack()
{
    auto cnd = cndStack.top();
    cndStack.pop();
    return cnd;
}

// Make condition from lexemes
CodeLine CFGBuilder::makeCondition()
{
    CodeLine cnd;
    cnd.push_back(currToken);                                       // Add opening parenthese to condition code line
    getToken();
    while (currLex != LEX_RIGHT_PAREN)                              // While closing parenthese is not met:
    {
        if (currLex == LEX_LEFT_PAREN)                              //   If there is another opening parenthese
        {
            CodeLine nestedCnd = makeCondition();                   //     Collect the following tokens recursively
            cnd.insert(cnd.end(), nestedCnd.begin(), nestedCnd.end());//   Add the resulting line to the condition
        }
        else
        {
            cnd.push_back(currToken);                               //   Else: add current token to the condition
        }
        getToken();
    }
    cnd.push_back(currToken);                                       // Add the closing parenthese
    return cnd;
}

// Make a negative condtion from an existing condition
CodeLine CFGBuilder::negateCondition(CodeLine cnd)
{
    cnd.insert(cnd.begin(), Token(LEX_NOT));                        // Add '!' operator before the condition
    cnd.insert(cnd.begin(), Token(LEX_LEFT_PAREN));                 // Add parentheses
    cnd.insert(cnd.end(), Token(LEX_RIGHT_PAREN));
    return cnd;
}

// Add a new node and edge to CFG
void CFGBuilder::addNodeAndEdge(CodeLine line, int prevId = 0)
{
    prevId = prevId == 0 ? currId - 1 : prevId;
    auto cnd = isBranch ? extractFromCndStack() : CodeLine{};      // If the current CFG state is a branch: extract condition from stack. Else: no condition
    isBranch = false;                                              // Remove the branch state

    cfg.addNode(currId, line);
    if (prevId > 0)
    {
        cfg.addEdge(prevId, currId, cnd);
    }
    currId++;
}

// Add all feasible edges from the potential edges tables
void CFGBuilder::addPotentialEdges()
{
    for (auto& edge : potentialEdges)
    {
        int idFrom = get<0>(edge);
        int idTo = get<1>(edge);
        auto cnd = get<2>(edge);

        cfg.addEdge(idFrom, idTo, cnd);
    }
    potentialEdges.clear();
}

// Breakdown conditional branch
void CFGBuilder::breakdownBranch(CodeLine cnd, int branchId)
{
    // make standard and negative conditions and place them in the conditions stack
    cndStack.push(negateCondition(cnd));
    cndStack.push(cnd);

    isBranch = true;                                            // Set the CFG state to a branch
    getToken();
    int prevBranch = 0;
    bool endsWithBranchOrLoop = false;
    prevId = 0;
    if (currLex == LEX_LEFT_BRACE)
    {
        endsWithBranchOrLoop = breakdownCodeBlock();
    }
    else
    {
        prevBranch = currId;
        endsWithBranchOrLoop = breakdownSingleLine();
    }

    auto prevToken = currToken;
    getToken();
    if (currLex == LEX_ELSE)                                   // If there is a negative branch:
    {
        prevBranch = prevBranch == 0 ? currId - 1 : prevBranch;
        isBranch = true;
        prevId = branchId;                                     // Reset the previous node to a branch
        getToken();
        if (currLex == LEX_LEFT_BRACE)
        {
            breakdownCodeBlock();
        }
        else
        {
            breakdownSingleLine();
        }
        prevId = 0;

        // If the previous branch did not finish with branching or looping
        if (!endsWithBranchOrLoop)
        {
            // Add a potential edge leading from the final node of the previous branch to the following code line
            potentialEdges.push_back(make_tuple(prevBranch, currId, CodeLine{}));
        }
    }
    else
    {
        ungetToken(prevToken);
        // Add a potential edge leading from the start of branching to the following code line
        potentialEdges.push_back(make_tuple(branchId, currId, extractFromCndStack()));
    }
}

// Breakdown entry loop (while(), for(;;))
void CFGBuilder::breakdownEntryLoop(CodeLine cnd, int branchId)
{
    // make standard and negative conditions and place them in the stack
    cndStack.push(negateCondition(cnd));
    cndStack.push(cnd);

    isBranch = true;
    prevId = 0;
    getToken();
    if (currLex == LEX_LEFT_BRACE)
    {
        breakdownCodeBlock();
    }
    else
    {
        breakdownSingleLine();
    }

    /*
    if (!potentialEdges.empty())
    {
        auto latestEdge = potentialEdges.top();
        int idFrom = get<0>(latestEdge);
        int idTo = get<1>(latestEdge);
        auto latestEdgeCnd = get<2>(latestEdge);

        // if the latest edge in the stack is an 'if' condition skip
        if (!cnd.empty() && idTo == currId)
        {
            int idTo = branchId;
            potentialEdges.pop();
            potentialEdges.push(make_tuple(idFrom, idTo, latestEdgeCnd));
        }
    }
    */
    if (!potentialEdges.empty())
    {
        auto latestEdge = potentialEdges.back();
        int idFrom = get<0>(latestEdge);
        int idTo = get<1>(latestEdge);
        auto latestEdgeCnd = get<2>(latestEdge);

        // if the latest edge in the stack is an 'if' condition skip
        if (!cnd.empty() && idTo == currId)
        {
            int idTo = branchId;
            potentialEdges.back() = make_tuple(idFrom, idTo, latestEdgeCnd);
        }
    }
    potentialEdges.push_back(make_tuple(branchId, currId, extractFromCndStack()));
    potentialEdges.push_back(make_tuple(currId - 1, branchId, CodeLine{}));
}

// Breakdown entry loop (do { ... } while())
void CFGBuilder::breakdownExitLoop(int branchId)
{
    if (currLex == LEX_LEFT_BRACE)
    {
        breakdownCodeBlock();
    }
    else
    {
        breakdownSingleLine();
    }

    // For nested conditional branches
    if (currLex == LEX_RIGHT_BRACE || currLex == LEX_SEMICOLON)
    {
        getToken();
    }

    if (currLex == LEX_WHILE)
    {
        addNodeAndEdge(CodeLine{ currToken }, prevId);
        getToken();
        auto cnd = makeCondition();
        auto cndNeg = negateCondition(cnd);

        potentialEdges.push_back(make_tuple(currId - 1, currId, cndNeg));
        potentialEdges.push_back(make_tuple(currId - 1, branchId, cnd));
        prevId = -1;
        getToken();
    }
}

// Breakdown 'for' loop parameters
pair<CodeLine, CodeLine> CFGBuilder::breakdownForLoopParameters()
{
    CodeLine iterOperators{ Token(LEX_LEFT_PAREN) };
    CodeLine cnd{ Token(LEX_LEFT_PAREN) };

    getToken();
    while (currLex != LEX_SEMICOLON)
    {
        iterOperators.push_back(currToken);
        getToken();
    }
    iterOperators.push_back(currToken);

    getToken();
    while (currLex != LEX_SEMICOLON)
    {
        cnd.push_back(currToken);
        getToken();
    }
    if (cnd.size() == 1)                                            // if the condition contains only opening parenthese
    {
        cnd.push_back(Token(LEX_TRUE));                             //   the condition is empty => always true
    }
    cnd.push_back(Token(LEX_RIGHT_PAREN));                          // add the closing parenthese to the condition

    getToken();
    while (currLex != LEX_RIGHT_PAREN)
    {
        iterOperators.push_back(currToken);
        getToken();
    }
    iterOperators.push_back(currToken);

    return make_pair(cnd, iterOperators);
}

// Code block breakdown
bool CFGBuilder::breakdownCodeBlock()
{
    bool endsWithBranchOrLoop = false;
    getToken();
    {
        while (currLex != LEX_RIGHT_BRACE)                          // while the closing bracket is not met
        {
            endsWithBranchOrLoop = breakdownSingleLine();           // breakdown a single code line
            getToken();
        }
    }
    return endsWithBranchOrLoop;
}

// Single code line breakdown
bool CFGBuilder::breakdownSingleLine()
{
    CodeLine currLine;
    int branchId;
    bool endsWithBranchOrLoop = true;

    switch (currLex)
    {
    // 'if': start of a conditional statement
    case LEX_IF:
        branchId = currId;                                          // Save current node ID as a branch
        currLine.push_back(currToken);
        addNodeAndEdge(currLine, prevId);                           // Add the current node and connect it with the previous one
        prevId = 0;
        currLine.clear();
        getToken();
        breakdownBranch(makeCondition(), branchId);
        break;

    // 'while': start of an entry loop
    case LEX_WHILE:
        branchId = currId;
        currLine.push_back(currToken);
        addNodeAndEdge(currLine, prevId);
        currLine.clear();
        getToken();
        breakdownEntryLoop(makeCondition(), branchId);
        prevId = -1;
        break;

    // 'do': start of an exit loop
    case LEX_DO:
        branchId = currId;
        currLine.push_back(currToken);
        addNodeAndEdge(currLine, prevId);
        currLine.clear();
        getToken();
        breakdownExitLoop(branchId);
        break;

    // 'for': start of a fixed-iteration entry loop
    case LEX_FOR:
    {
        branchId = currId;
        currLine.push_back(currToken);
        getToken();
        auto loopParams = breakdownForLoopParameters();            // Breakdown for loop parameters into:
        CodeLine cnd = loopParams.first;                           //   Condition
        CodeLine iterOperators = loopParams.second;                //   and iterative operations (counter initialization, counter increment)
        currLine.insert(currLine.end(), iterOperators.begin(), iterOperators.end());
        addNodeAndEdge(currLine, prevId);
        currLine.clear();
        breakdownEntryLoop(loopParams.first, branchId);            // Breakdown 'for' loop as an entry loop
        prevId = -1;
        break;
    }
    // Repeated semicolons without any codelines
    case LEX_SEMICOLON:
        while (currLex == LEX_SEMICOLON)
        {
            getToken();
        }
        ungetToken(Token(LEX_SEMICOLON));
        break;

    default:
        while (currLex != LEX_SEMICOLON)                           // While ';' is not met:
        {
            currLine.push_back(currToken);                         //   Add tokens to the current code line 
            getToken();
        }
        addNodeAndEdge(currLine, prevId);
        prevId = 0;
        currLine.clear();
        endsWithBranchOrLoop = false;
        break;
    }
    return endsWithBranchOrLoop;
}

CFGBuilder::CFGBuilder(string fileName) : parser(fileName)
{
    currId = 1;
    isBranch = false;

    vector<Token> code = parser.analyze().sourceCode;
    codeBody = eraseFunctionHeader(code);
}

CFG CFGBuilder::buildCFG()
{
    getToken();
    breakdownCodeBlock();
    addPotentialEdges();
    return cfg;
}