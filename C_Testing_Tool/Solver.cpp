#include "Solver.h"

using namespace z3;

//.........................SYMBOLIC CONTEXT

// Constructor
SymbolicContext::SymbolicContext()
{
    vars = {};
}

// Import variables from source code as Z3 expressions
void SymbolicContext::importVars(vector<Identifier> ids)
{
    for (int idIndex = 0; idIndex < ids.size(); idIndex++)
    {
        idType id_type = ids.at(idIndex).getIdType();
        if (id_type == STD_VAR || id_type == INPUT_VAR)
        {
            std::string varName = "var" + std::to_string(idIndex);
            lexeme dataType = ids.at(idIndex).getDataType();
            expr varExpr = ctx.int_const(varName.c_str());
            switch (dataType)
            {
            case LEX_FLOAT: case LEX_DOUBLE:
                varExpr = ctx.real_const(varName.c_str());
                break;

            case LEX_BOOL:
                varExpr = ctx.bool_const(varName.c_str());
                break;

            default:
                break;
            }
            vars.push_back(std::make_pair(idIndex, varExpr));
        }
    }
}

// Reset values of all variables
void SymbolicContext::resetVars(vector<Identifier> ids)
{
    vars.clear();
    importVars(ids);
}

int SymbolicContext::getIndex(int val) const
{
    auto isPresent = [val](const pair<int, expr>& entry) { return entry.first == val; };
    auto it = std::find_if(vars.begin(), vars.end(), isPresent);
    if (it != vars.end())
    {
        int index = std::distance(vars.begin(), it);
        return index;
    }
    return -1;
}

expr SymbolicContext::getExpr(int val) const
{
    return vars.at(getIndex(val)).second;
}

void SymbolicContext::setExpr(int val, expr newExpr)
{
    vars.at(getIndex(val)).second = newExpr;
}


//.........................TEST CASE CLASS

// Constructor for infeasible test case
TestCase::TestCase()
{
    sat = false;
}

// Constructor for feasible test case
TestCase::TestCase(std::vector<Identifier> funcParams, expr_vector evals, expr returnVal) : ids(funcParams)
{
    sat = true;
    if (evals.size() == 0)
    {
        for (int i = 0; i < ids.size(); i++)
        {
            values.push_back("Any value");
        }
    }
    else
    {
        for (int i = 0; i < ids.size(); i++)
        {
            values.push_back(evals[i].to_string());
        }
    }
    output = returnVal.simplify().to_string();
}

// Get test case feasibility
bool TestCase::isFeasible()
{
    return sat;
}

// Get number of input variables
int TestCase::getSize()
{
    return values.size();
}

// Get identifier
Identifier TestCase::getId(int index)
{
    return ids.at(index);
}

// Get value of the identifier
string TestCase::getIdValue(int index)
{
    return values.at(index);
}

// Get the function output
string TestCase::getOutput()
{
    return output;
}


//.........................SMT SOLVER CLASS
void Solver::collectPaths(int id, Path currPath, std::set<int> visitedNodes, int loopIterCount = 0)
{
    Node& node = cfg.nodes[id];
    currPath.push_back(node);
    visitedNodes.insert(id);

    if (node.edges.empty())
    {
        paths.push_back(currPath);
    }
    else
    {
        Token branch = node.code.front();
        if (branch.getLexeme() == LEX_WHILE || branch.getLexeme() == LEX_FOR)
        {
            loopIterCount++;
        }

        if (node.edges.size() == 1 && !node.edges.front().condition.empty())
        {
            paths.push_back(currPath);
            collectPaths(node.edges.front().idTarget, currPath, visitedNodes, loopIterCount);
        }
        else
        {
            for (const auto& edge : node.edges)
            {
                //if (visitedNodes.count(edge.idTarget) == 0 || edge.condition.empty())
                if (loopIterCount <= 2 || edge.condition.empty())
                {
                    collectPaths(edge.idTarget, currPath, visitedNodes, loopIterCount);
                }
            }
        }
    }
}

Token Solver::getLexeme(const std::vector<Token>& line, int& index, lexeme& type)
{
    if (index >= line.size())
    {
        return Token(LEX_FIN, LEX_FIN);
    }
    int i = index;
    type = line.at(i).getLexeme();
    index++;
    return line.at(i);
}

void Solver::ungetLexeme(const std::vector<Token>& line, int& index, lexeme& type)
{
    if (index > 0)
    {
        index--;
        type = line.at(index).getLexeme();
    }
}

void Solver::solveAssign(const std::vector<Token>& code, int& index, lexeme& type)
{
    Token lex = getLexeme(code, index, type);
    lexeme idType = LEX_NULL;
    if (type >= LEX_INT && type <= LEX_STRING)
    {
        idType = type;
        lex = getLexeme(code, index, type);
    }
    if (type == LEX_ID)
    {
        int idVal = lex.getValue();
        getLexeme(code, index, type);
        expr assignExpr = sym.getExpr(idVal);
        switch (type)
        {
        case LEX_ASSIGN:
            sym.setExpr(idVal, STMNT(code, index, type));
            break;

        case LEX_PLUS_ASSIGN:
            sym.setExpr(idVal, sym.getExpr(idVal) + STMNT(code, index, type));
            break;

        case LEX_MINUS_ASSIGN:
            sym.setExpr(idVal, sym.getExpr(idVal) - STMNT(code, index, type));
            break;

        case LEX_TIMES_ASSIGN:
            sym.setExpr(idVal, sym.getExpr(idVal) * STMNT(code, index, type));
            break;

        case LEX_SLASH_ASSIGN:
            sym.setExpr(idVal, sym.getExpr(idVal) / STMNT(code, index, type));
            break;

        default:
            switch (idType)
            {
            case LEX_INT:
                sym.setExpr(idVal, sym.ctx.int_val(DEFAULT_NUM_VALUE));
                break;

            case LEX_FLOAT: case LEX_DOUBLE:
                sym.setExpr(idVal, sym.ctx.real_val(DEFAULT_NUM_VALUE));
                break;

            case LEX_CHAR:
                //sym.setExpr(idVal, sym.ctx.string_val(DEFAULT_CHAR_VALUE));
                break;
            }
            break;
        }
    }
}

expr Solver::solveCondition(const std::vector<Token>& cnd)
{
    int index = 0;
    lexeme type;
    expr cndExpr = sym.ctx.bool_val(true); // default
    getLexeme(cnd, index, type);
    if (type == LEX_LEFT_PAREN)
    {
        cndExpr = STMNT(cnd, index, type);
    }
    return cndExpr;
}

void Solver::solveLoop(const Path& path, solver& solver, int& currNodeIndex)
{
    auto currNode = path.at(currNodeIndex);
    int loopStartNodeId = currNode.id;
    int loopEndIndex = 0;
    auto it = std::find_if(
        path.begin() + currNodeIndex + 1,
        path.end(),
        [loopStartNodeId](Node node)
        {
            return node.id == loopStartNodeId;
        }
    );
    if (it != path.end())
    {
        loopEndIndex = distance(path.begin(), it);
    }
    //cout << "Loop Start Node: " << loopStartNodeId << "\nLoop End Index: " << loopEndIndex << "\n";
    if (loopEndIndex > 0)
    {
        int loopIter = 0;
        auto cnd = currNode.edges.front().condition;
        expr loopCndExpr = solveCondition(cnd);

        solver.push();                                  // save current state of the solver
        solver.add(loopCndExpr);                        // add temporary expression
        bool loopCondSat = solver.check() == sat;
        solver.pop();                                   // erase temporary expression

        Path loopBodyPath{};
        /*
        cout << "Loop body: ";
        for (int k = currNodeIndex + 1; k < loopEndIndex; k++)
        {
            loopBodyPath.push_back(path.at(k));
            cout << loopBodyPath.at(k - currNodeIndex - 1).id << "  ";
        }
        cout << "\n";
        */
        currNodeIndex = loopEndIndex;
        while (loopIter < maxIterForLoops && loopCondSat)
        {
            bool debug = false;
            evaluatePathConstraints(loopBodyPath, solver, debug);
            loopCndExpr = solveCondition(cnd);
            solver.push();
            solver.add(loopCndExpr);
            loopCondSat = solver.check() == unsat;
            solver.pop();
            loopIter++;
        }
        expr finalLoopCndExpr = loopIter < maxIterForLoops ? !loopCndExpr : sym.ctx.bool_val(false);
        solver.add(finalLoopCndExpr);
    }
    else
    {
        auto cndFalse = currNode.edges.back().condition;
        expr cndExpr = solveCondition(cndFalse);
        solver.add(cndExpr);
    }
}

expr Solver::STMNT(const std::vector<Token>& line, int& currIndex, lexeme& currType)
{
    expr stmntExpr = DISJ(line, currIndex, currType);
    executeUnaryOperations();
    return stmntExpr;
}

expr Solver::DISJ(const std::vector<Token>& line, int& currIndex, lexeme& currType)
{
    expr logicAddExpr = CONJ(line, currIndex, currType);
    while (currType == LEX_OR)
    {
        logicAddExpr = logicAddExpr || CONJ(line, currIndex, currType);
    }
    return logicAddExpr;
}

expr Solver::CONJ(const std::vector<Token>& line, int& currIndex, lexeme& currType)
{
    expr logicMultiExpr = CMP(line, currIndex, currType);
    while (currType == LEX_AND)
    {
        logicMultiExpr = logicMultiExpr && CMP(line, currIndex, currType);
    }
    return logicMultiExpr;
}

expr Solver::CMP(const std::vector<Token>& line, int& currIndex, lexeme& currType)
{
    expr stmntExpr = ADD(line, currIndex, currType);
    switch (currType)
    {
    case LEX_EQ:
        stmntExpr = stmntExpr == ADD(line, currIndex, currType);
        break;

    case LEX_GREATER:
        stmntExpr = stmntExpr > ADD(line, currIndex, currType);
        break;

    case LEX_LESS:
        stmntExpr = stmntExpr < ADD(line, currIndex, currType);
        break;

    case LEX_GREATER_EQ:
        stmntExpr = stmntExpr >= ADD(line, currIndex, currType);
        break;

    case LEX_LESS_EQ:
        stmntExpr = stmntExpr <= ADD(line, currIndex, currType);
        break;

    case LEX_NOT_EQ:
        stmntExpr = stmntExpr != ADD(line, currIndex, currType);
        break;

    default:
        break;
    }
    return stmntExpr;
}

expr Solver::ADD(const std::vector<Token>& line, int& currIndex, lexeme& currType)
{
    expr addExpr = MULTI(line, currIndex, currType);
    while (currType == LEX_PLUS || currType == LEX_MINUS)
    {
        switch (currType)
        {
        case LEX_PLUS:
            addExpr = addExpr + MULTI(line, currIndex, currType);
            break;

        case LEX_MINUS:
            addExpr = addExpr - MULTI(line, currIndex, currType);
            break;

        default:
            break;
        }
    }
    return addExpr;
}

expr Solver::MULTI(const std::vector<Token>& line, int& currIndex, lexeme& currType)
{
    expr multiExpr = FIN(line, currIndex, currType);
    while (currType >= LEX_TIMES && currType <= LEX_PERCENT)
    {
        switch (currType)
        {
        case LEX_TIMES:
            multiExpr = multiExpr * FIN(line, currIndex, currType);
            break;

        case LEX_SLASH:
            multiExpr = multiExpr / FIN(line, currIndex, currType);
            break;

        case LEX_PERCENT:
            multiExpr = multiExpr % FIN(line, currIndex, currType);
            break;

        default:
            break;
        }
    }
    return multiExpr;
}

expr Solver::FIN(const std::vector<Token>& line, int& currIndex, lexeme& currType)
{
    int lexValue;
    expr finExpr = sym.ctx.int_val(0); // default
    Token currLex = getLexeme(line, currIndex, currType);
    switch (currType)
    {
    case LEX_NOT:
        currLex = getLexeme(line, currIndex, currType);
        if (currType == LEX_LEFT_PAREN)
        {
            finExpr = !STMNT(line, currIndex, currType);
        }
        else
        {
            lexValue = currLex.getValue();
            finExpr = !sym.getExpr(lexValue);
        }
        getLexeme(line, currIndex, currType);
        break;

    case LEX_LEFT_PAREN:
        finExpr = STMNT(line, currIndex, currType);
        break;

    case LEX_PLUS:
        finExpr = FIN(line, currIndex, currType);
        break;

    case LEX_MINUS:
        finExpr = -FIN(line, currIndex, currType);
        break;

    case LEX_PLUS_PLUS:
        currLex = getLexeme(line, currIndex, currType);
        lexValue = currLex.getValue();
        finExpr = sym.getExpr(lexValue) + sym.ctx.int_val(1);
        sym.setExpr(lexValue, finExpr);
        checkUnaryOperation(line, currIndex, currType, lexValue);
        break;

    case LEX_MINUS_MINUS:
        currLex = getLexeme(line, currIndex, currType);
        lexValue = currLex.getValue();
        finExpr = sym.getExpr(lexValue) - sym.ctx.int_val(1);
        sym.setExpr(lexValue, finExpr);
        checkUnaryOperation(line, currIndex, currType, lexValue);
        break;

    case LEX_ID:
        lexValue = currLex.getValue();
        finExpr = sym.getExpr(lexValue);
        checkUnaryOperation(line, currIndex, currType, lexValue);
        break;

    case LEX_NUM:
        if (currLex.getValue() - (int)currLex.getValue() == 0)
        {
            lexValue = currLex.getValue();
            finExpr = sym.ctx.int_val(lexValue);
        }
        else
        {
            double doubleVal = currLex.getValue();
            int numDigits = 0;
            while (doubleVal - (int)doubleVal != 0)
            {
                doubleVal *= 10;
                numDigits++;
            }
            lexValue = doubleVal;
            finExpr = sym.ctx.real_val(lexValue, std::pow(10, numDigits));
        }
        break;

    case LEX_TRUE:
        finExpr = sym.ctx.bool_val(true);
        break;

    case LEX_FALSE:
        finExpr = sym.ctx.bool_val(false);
        break;

    default:
        break;
    }
    if (currIndex < line.size())
    {
        getLexeme(line, currIndex, currType);
    }
    return finExpr;
}

void Solver::checkUnaryOperation(const std::vector<Token>& line, int& currIndex, lexeme& currType, int idValue)
{
    if (currIndex < line.size())
    {
        getLexeme(line, currIndex, currType);
        if (currType == LEX_PLUS_PLUS || currType == LEX_MINUS_MINUS)
        {
            unaryOpTable.push_back(std::make_pair(idValue, currType));
        }
        else
        {
            ungetLexeme(line, currIndex, currType);
        }
    }
}

void Solver::executeUnaryOperations()
{
    for (auto& entry : unaryOpTable)
    {
        int idVal = entry.first;
        lexeme unaryOpType = entry.second;
        expr unaryOpResult = unaryOpType == LEX_PLUS_PLUS ?
            sym.getExpr(idVal) + sym.ctx.int_val(1) :
            sym.getExpr(idVal) - sym.ctx.int_val(1);
        sym.setExpr(idVal, unaryOpResult);
    }
    unaryOpTable.clear();
}

Solver::Solver(CFG currCfg, vector<Identifier> ids, int maxIter) : cfg(currCfg), maxIterForLoops(maxIter), ids(ids)
{
    sym.importVars(ids);
}

void Solver::setMaxIterForLoops(int newMaxIter)
{
    maxIterForLoops = newMaxIter;
}

TestCase Solver::evaluatePathConstraints(const Path& path, solver& solver, bool debugPrint = false)
{
    expr returnVal = sym.ctx.int_val(DEFAULT_NUM_VALUE);
    for (int i = 0; i < path.size(); i++)
    {
        auto node = path[i];
        Node nextNode;

        CodeLine nestedVar;
        CodeLine iteration;

        int index = 0;
        lexeme type;
        int loopStartNodeId;
        int loopEndIndex;
        Path::const_iterator pathIt;
        CodeLine::const_iterator codeIt;

        getLexeme(node.code, index, type);
        switch (type)
        {
        case LEX_IF:
            if (i == path.size() - 1)
            {
                auto cnd = node.edges.back().condition;
                expr cndExpr = solveCondition(cnd);
                if (node.edges.size() == 1)
                {
                    cndExpr = !cndExpr;
                }
                solver.add(cndExpr);
            }
            else
            {
                nextNode = path.at(i + 1);
                for (auto& edge : node.edges)
                {
                    if (edge.idTarget == nextNode.id)
                    {
                        // Condition parser
                        auto cnd = edge.condition;
                        expr cndExpr = solveCondition(cnd);
                        solver.add(cndExpr);
                    }
                }
            }
            break;

        case LEX_DO:
            break;

        case LEX_WHILE:
            solveLoop(path, solver, i);
            break;

        case LEX_FOR:
            nestedVar = {};
            iteration = {};

            index++;
            while (node.code.at(index).getLexeme() != LEX_SEMICOLON)
            {
                nestedVar.push_back(node.code.at(index));
                index++;
            }
            index++;
            while (node.code.at(index).getLexeme() != LEX_RIGHT_PAREN)
            {
                iteration.push_back(node.code.at(index));
                index++;
            }

            if (!nestedVar.empty())
            {
                int nestedVarIndex = 0;
                auto nestedVarType = LEX_NULL;
                solveAssign(nestedVar, nestedVarIndex, nestedVarType);
            }

            loopStartNodeId = node.id;
            loopEndIndex = 0;
            pathIt = std::find_if(
                path.begin() + i + 1,
                path.end(),
                [loopStartNodeId](Node node)
                {
                    return node.id == loopStartNodeId;
                }
            );
            if (pathIt != path.end())
            {
                loopEndIndex = distance(path.begin(), pathIt);
            }
            if (loopEndIndex > 0)
            {
                int loopIter = 0;
                auto cnd = node.edges[0].condition;
                expr cndExpr = solveCondition(cnd);

                solver.push();          // save current state of the solver
                solver.add(cndExpr);    // add temporary expression
                bool loopCondSat = solver.check() == sat;
                solver.pop();           // erase temporary expression

                Path loopBodyPath{};
                for (int k = i + 1; k < loopEndIndex; k++)
                {
                    loopBodyPath.push_back(path[k]);
                    cout << loopBodyPath.at(k - i - 1).id << "  ";
                }
                i = loopEndIndex;
                cout << "\n";
                while (loopIter < maxIterForLoops && loopCondSat)
                {
                    evaluatePathConstraints(loopBodyPath, solver);

                    if (!iteration.empty())
                    {
                        int iterIndex = 0;
                        auto iterType = LEX_NULL;

                        codeIt = std::find_if(
                            iteration.begin(),
                            iteration.end(),
                            [](Token lex)
                            {
                                return (lex.getLexeme() == LEX_ASSIGN || lex.getLexeme() >= LEX_PLUS_ASSIGN && lex.getLexeme() <= LEX_SLASH_ASSIGN);
                            }
                        );
                        if (codeIt != iteration.end())
                        {
                            solveAssign(iteration, iterIndex, iterType);
                        }
                        else
                        {
                            STMNT(iteration, iterIndex, iterType);
                        }
                    }

                    cndExpr = solveCondition(cnd);
                    solver.push();
                    solver.add(cndExpr);
                    loopCondSat = solver.check() == sat;
                    solver.pop();
                    loopIter++;
                }
                if (loopIter == maxIterForLoops)
                {
                    cout << "MAX ITER REACHED\n";
                    solver.add(sym.ctx.bool_val(false));
                }
            }
            else
            {
                auto cnd = node.edges[1].condition;
                expr cndExpr = solveCondition(cnd);
                solver.add(cndExpr);
            }
            break;

        case LEX_RETURN:
            returnVal = STMNT(node.code, index, type);
            break;

        default:
            // reset index to start parsing codeline from the first lexeme
            index = 0;
            type = LEX_NULL;

            codeIt = std::find_if(
                node.code.begin(),
                node.code.end(),
                [](Token lex)
                {
                    return (
                        lex.getLexeme() == LEX_ASSIGN ||
                        lex.getLexeme() >= LEX_INT && lex.getLexeme() <= LEX_STRING ||
                        lex.getLexeme() >= LEX_PLUS_ASSIGN && lex.getLexeme() <= LEX_SLASH_ASSIGN
                        );
                }
            );
            if (codeIt != node.code.end())
            {
                solveAssign(node.code, index, type);
            }
            else
            {
                STMNT(node.code, index, type);
            }
            break;
        }
    }
    bool isSat = solver.check() == sat;

    expr_vector varsVec(sym.ctx);
    expr_vector evalVec(sym.ctx);
    
    if (isSat)
    {
        for (int i = 0; i < solver.get_model().size(); i++)
        {
            expr eval = solver.get_model().eval(sym.vars.at(i).second, true);
            varsVec.push_back(sym.vars.at(i).second);
            evalVec.push_back(eval);

        }
        returnVal = returnVal.substitute(varsVec, evalVec);
    }

    // Print feasibility results
    if (debugPrint)
    {
        std::cout << "Path: ";
        for (auto line : path)
        {
            std::cout << line.id << " ";
        }
        std::cout << "\n  Result: " << (isSat ? "feasible" : "infeasible") << "\n";

        if (isSat)
        {
            std::cout << "  Test case:\n";
            for (int i = 0; i < evalVec.size(); i++)
            {
                string evalStr = evalVec[i].to_string();
                std::cout << "    " << ids.at(i + 1).getName() << " = " << evalStr << "\n";
            }
            std::cout << "  Return value: " << returnVal.simplify() << "\n";
        }
        cout << "\n";
    }

    if (isSat)
    {
        vector<Identifier> inputVars;
        for (auto& id : ids)
        {
            if (id.getIdType() == INPUT_VAR)
            {
                inputVars.push_back(id);
            }
        }
        return TestCase(
            inputVars,
            evalVec,
            returnVal
        );
    }
    return TestCase();
}

void Solver::debugPrintPaths()
{
    cout << "PATHS COLLECTED\n";
    int i = 1;
    for (auto& path : paths)
    {
        cout << "Path " << i << ":\n   ";
        for (auto& node : path)
        {
            cout << node.id << " ";
        }
        cout << "\n";
        i++;
    }
    cout << "\n";
}

void Solver::checkAllPaths(bool debug = false)
{
    collectPaths(cfg.idEntry, {}, {});
    if (debug)
    {
        debugPrintPaths();
    }

    for (auto& path : paths)
    {
        solver z3Solver(sym.ctx);
        TestCase newTestCase = evaluatePathConstraints(path, z3Solver, debug);
        testSuite.push_back(newTestCase);
        sym.resetVars(ids);
    }
}

std::vector<std::pair<Path, TestCase>> Solver::getPathsAndCases()
{
    std::vector<std::pair<Path, TestCase>> pathsAndModels;
    checkAllPaths();
    for (int i = 0; i < paths.size(); i++)
    {
        pathsAndModels.push_back(std::make_pair(paths.at(i), testSuite.at(i)));
    }
    return pathsAndModels;
}