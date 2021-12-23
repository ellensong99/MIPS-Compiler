#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <deque>
#include <set>
using namespace std;

class Node {
    public:
    string rule;
    vector<string> tokens;
    vector<Node> children;
    Node(string rule, vector<string> tokens): rule(rule), tokens(tokens){}
    void insertChild(Node node) { children.push_back(node); }
};

class Error {
    string msg;
    public:
    Error(string msg) : msg(msg){}
    string what() { return msg; }
};

vector<string> split_ws(string s) {
    vector<string> ret;
    stringstream ss(s);
    string str;
    while (getline(ss, str, ' ')) {
        ret.push_back(str);
    }
    return ret;
}


Node buildTree(istream &in) {
    string rule;
    vector<string> tokens;
    vector<Node> children;
    if (getline(in, rule)) {
        tokens = split_ws(rule);
        Node n = Node(rule, tokens);
        if (!((tokens[0][0] >= 'A') & (tokens[0][0] <= 'Z'))) {
            for (int i = 0; i < tokens.size()-1; i++) {
                n.insertChild(buildTree(in));
            }
        }
        return n;
    }
    throw Error("ERROR: Invalid WLP4I Input.");
}

// The global variable table is to store the symbol table 
map<string, pair<vector<string>, map<string, pair<string, int>>>> table;

// The global variable count is to record the number of loops
int count = 1;

void push(int r) {
    cout << "sw $" << r << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

void pop(int r) {
    cout << "add $30, $30, $4" << endl;
    cout << "lw $" << r << ", -4($30)" << endl;
}

void getDcls(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& dclTable, int* offset) {
    if (n.children.size() != 0) {
        getDcls(n.children[0], dclTable, offset);
        string name = n.children[1].children[1].tokens[1];
        string type = (n.children[1].children[0].children.size() == 1) ? "int" : "int*";
        dclTable.second.emplace(name, pair<string, int>(type, *offset));
        if (type == "int") {
            cout << "lis $3" << endl;
            cout << ".word " << n.children[3].tokens[1] << endl;
            cout << "sw $3, " << *offset << "($29)" << endl;
            cout << "sub $30, $30, $4" << endl;
        } else {
            cout << "add $3, $11, $0" << endl;
            cout << "sw $3, " << *offset << "($29)" << endl;
            cout << "sub $30, $30, $4" << endl;
        }
        *offset -= 4;
    } else {
        return;
    }
}

string getFactor(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& factorTable);

string getLvalue(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& lvalueTable) {
    for (auto c: n.children) {
        if (c.tokens[0] == "ID") {
            return lvalueTable.second[c.tokens[1]].first;
        } else if (c.tokens[0] == "factor") {
            string factor = getFactor(c, lvalueTable);
            return "int";
        } else if (c.tokens[0] == "lvalue") {
            return getLvalue(c, lvalueTable);
        }
    }
}

string getExpr(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& exprTable);

int getArglist(const Node& n, int maxlen, int len, pair<vector<string>, map<string, pair<string, int>>>& argsTable, string pro) {
    for (auto c: n.children) {
        if (c.tokens[0] == "expr") {
            string expr = getExpr(c, argsTable);
            len++;
            push(3);
        } else if (c.tokens[0] == "arglist") {
            len = getArglist(c, maxlen, len, argsTable, pro);
        }
    }
    return len;
}


string getFactor(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& factorTable) {
    if (n.children.size() < 3) {
        if (n.children[0].tokens[0] == "ID") {
            string var = n.children[0].tokens[1];
            int offset = factorTable.second[var].second;
            cout << "lw $3, " << offset << "($29)" << endl;
            return factorTable.second[var].first;
        } else if (n.children[0].tokens[0] == "NUM") {
            cout << "lis $3" << endl;
            cout <<".word " << n.children[0].tokens[1] << endl;
            return "int";
        } else if (n.children[0].tokens[0] == "NULL") {
            cout << "add $3, $0, $11" << endl;
            return "int*";
        } else if (n.children[0].tokens[0] == "AMP") {
            string lvalue = getLvalue(n.children[1], factorTable);
            if (n.children[1].children.size() == 1) {
                string var = n.children[1].children[0].tokens[1];
                int offset = factorTable.second[var].second;
                cout << "lis $3" << endl;
                cout << ".word " << offset << endl;
                cout << "add $3, $3, $29" << endl;
            }   
            return "int*";
        } else if (n.children[0].tokens[0] == "STAR") {
            string factor = getFactor(n.children[1], factorTable);
            cout << "lw $3, 0($3)" << endl;
            return "int";
        } 
    }
    string name = "";
     if (n.children[1].tokens[0] == "expr") {
        return getExpr(n.children[1], factorTable);
    }
    if (n.children[0].tokens[0] == "ID") {
        name = n.children[0].tokens[1];
        push(29);
        push(31);
        int len = 0;
        if (n.children[2].tokens[0] == "arglist") {
            int maxlen = table[name].first.size() - 1;
            len = getArglist(n.children[2], maxlen, 0, factorTable, name);
        }
        cout << "lis $5" << endl;
        cout << ".word S" << name << endl;
        cout << "jalr $5" << endl;
        for (int i = 0; i < len; i++) {
            pop(31);
        }
        pop(31);
        pop(29);
    } else if (n.children[3].tokens[0] == "expr") {
        string expr = getExpr(n.children[3], factorTable);
        cout << "add $1, $3, $0" << endl;
        push(31);
        cout << "lis $5" << endl;
        cout << ".word new" << endl;
        cout << "jalr $5" << endl;
        pop(31);
        cout << "bne $3, $0, 1" << endl;
        cout << "add $3, $11, $0" << endl;
        return "int*";
    }
    return "int";
}


string getTerm(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& termTable) {
    if (n.children.size() <= 1) {
        return getFactor(n.children[0], termTable);
    }
    string factor, term;
    for (auto c: n.children) {
        if (c.tokens[0] == "factor") {
            factor = getFactor(c, termTable);
        } else if (c.tokens[0] == "term") {
            term = getTerm(c, termTable);
            push(3);
        }
    }
    if (n.children[1].tokens[0] == "STAR") {
        pop(5);
        cout << "mult $5, $3" << endl;
        cout << "mflo $3" << endl;
    } 
    if (n.children[1].tokens[0] == "SLASH") {
        pop(5);
        cout << "div $5, $3" << endl;
        cout << "mflo $3" << endl;
    } 
    if (n.children[1].tokens[0] == "PCT") {
        pop(5);
        cout << "div $5, $3" << endl;
        cout << "mfhi $3" << endl;
    }
    return "int";

}

string getExpr(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& exprTable) {
    if (n.children.size() <= 1) {
        return getTerm(n.children[0], exprTable);;
    }
    string expr, term;
    for (auto c: n.children) {
        if (c.tokens[0] == "expr") {
            expr = getExpr(c, exprTable);
            push(3);
        } else if (c.tokens[0] == "term") {
            term = getTerm(c, exprTable);
        }
    }
    if (n.children[1].tokens[0] == "PLUS") {
        if (expr == "int" & term == "int") {
            pop(5);
            cout << "add $3, $5, $3" << endl;
            return "int";
        } else if (expr != term) {
            if (expr == "int*" && term == "int") {
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
                pop(5);
                cout << "add $3, $5, $3" << endl;
            } else if (expr == "int" && term == "int*") {
                pop(5);
                cout << "mult $5, $4" << endl;
                cout << "mflo $5" << endl;
                cout << "add $3, $3, $5" << endl;
            }
            return "int*";
        } 
    } else if (n.children[1].tokens[0] == "MINUS") {
        if (expr == "int*" & term == "int") {
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            pop(5);
            cout << "sub $3, $5, $3" << endl;
            return "int*";
        } else if (expr == term) {
            if (expr == "int") {
                pop(5);
                cout << "sub $3, $5, $3" << endl;
            } else {
                pop(5);
                cout << "sub $3, $5, $3" << endl;
                cout << "div $3, $4" << endl;
                cout << "mflo $3" << endl;
            }
            return "int";
        }
    } 
}

void getTest(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& testTable) {
    string f = getExpr(n.children[0], testTable);
    push(3);
    string s = getExpr(n.children[n.children.size()-1], testTable);
    string comp = (f == "int") ? "slt" : "sltu";
    if (n.children[1].tokens[0] == "LT") {
        pop(5);
        cout << comp << " $3, $5, $3" << endl;
    } else if (n.children[1].tokens[0] == "GT") {
        pop(5);
        cout << comp << " $3, $3, $5" << endl;
    } else if (n.children[1].tokens[0] == "NE") {
        pop(5);
        cout << comp << " $6, $3, $5" << endl;
        cout << comp << " $7, $5, $3" << endl;
        cout << "add $3, $6, $7" << endl;
    } else if (n.children[1].tokens[0] == "EQ") {
        pop(5);
        cout << comp << " $6, $3, $5" << endl;
        cout << comp << " $7, $5, $3" << endl;
        cout << "add $3, $6, $7" << endl;
        cout << "sub $3, $11, $3" << endl;
    } else if (n.children[1].tokens[0] == "LE") {
        pop(5);
        cout << comp << " $3, $3, $5" << endl;
        cout << "sub $3, $11, $3" << endl;
    } else if (n.children[1].tokens[0] == "GE") {
        pop(5);
        cout << comp << " $3, $5, $3" << endl;
        cout << "sub $3, $11, $3" << endl;
    }
}

void getStatement(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& stmtTable);

void getStatements(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& stmtTable) {
    if (n.children.size() != 0) {
        getStatements(n.children[0], stmtTable);
        getStatement(n.children[1], stmtTable);
    } else {
        return;
    }
}

void getStatement(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& stmtTable) {
    if (n.children[0].tokens[0] == "IF") {
        int local = count;
        count++;
        getTest(n.children[2], stmtTable);
        cout << "beq $3, $0, else" << local << endl;
        getStatements(n.children[5], stmtTable);
        cout << "beq $0, $0, endif" << local << endl;
        cout << "else" << local << ":"  << endl;
        getStatements(n.children[9], stmtTable);
        cout << "endif" << local << ":"  << endl;
    } else if (n.children[0].tokens[0] == "lvalue") {
        string expr = getExpr(n.children[2], stmtTable);
        push(3);
        string lvalue = getLvalue(n.children[0], stmtTable);
        if (n.children[0].children.size() == 2) {
            pop(5);
            cout << "sw $5, 0($3)" << endl;
        } else if (n.children[0].children.size() == 1) {
            string name = n.children[0].children[0].tokens[1];
            int offset = stmtTable.second[name].second;
            cout << "sw $3, " << offset << "($29)" << endl; 
        }
    } else if (n.children[0].tokens[0] == "WHILE") {
        int local = count;
        count++;
        cout << "loop" << local << ":"  << endl;
        getTest(n.children[2], stmtTable);
        cout << "beq $3, $0, endWhile" << local << endl;
        getStatements(n.children[5], stmtTable);
        cout << "beq $0, $0, loop" << local << endl;
        cout << "endWhile" << local << ":"  << endl;
    } else if (n.children[0].tokens[0] == "PRINTLN") {
        push(1);
        string expr = getExpr(n.children[2], stmtTable);
        cout << "add $1, $3, $0" << endl;
        push(31);
        cout << "jalr $10" << endl;
        pop(31);
        pop(1);
    } else if (n.children[0].tokens[0] == "DELETE") {
        string expr = getExpr(n.children[3], stmtTable);
        int local = count;
        count++;
        cout << "beq $3,  $11, skipDelete" << local << endl;
        cout << "add $1, $3, $0" << endl;
        push(31);
        cout << "lis $5" << endl;
        cout << ".word delete" << endl;
        cout << "jalr $5" << endl;
        pop(31);
        cout << "skipDelete" << local << ":" << endl;
    }
}

int getParamlist(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& paramTable, int* param) {
    int pos = *param + 1;
    if (n.children.size() == 3) {
        *param += 1;
        int para = getParamlist(n.children[n.children.size()-1], paramTable, param);
    } else {
        *param += 1;
    }
    if (n.children[0].children[0].children.size() == 1) {
        paramTable.first.push_back("int");
    } else {
        paramTable.first.push_back("int*");
    }
    string name = n.children[0].children[1].tokens[1];
    int offset = (*param - pos + 1) * 4;
    paramTable.second.emplace(name, pair<string, int>(paramTable.first[paramTable.first.size()-1], offset));
    return *param;
}

void getProcedure(const Node& n, pair<vector<string>, map<string, pair<string, int>>>& proTable) {
    proTable.first.push_back(n.children[1].tokens[1] + ":");
    cout << "S" << n.children[1].tokens[1] + ":" << endl;
    cout << "sub $29, $30, $4" << endl;
    if (n.children[3].children.size() != 0) {
        int num = 0;
        int para = getParamlist(n.children[3].children[0], proTable, &num);
    }
    int offset = 0;
    getDcls(n.children[6], proTable, &offset);
    push(5);
    push(6);
    push(7);
    getStatements(n.children[7], proTable);
    string expr = getExpr(n.children[9], proTable);
    pop(7);
    pop(6);
    pop(5);
    cout << "add $30, $29, $4" << endl;
    cout << "jr $31" << endl;
}

void getMain(Node& n, pair<vector<string>, map<string, pair<string, int>>>& mainTable) {
    mainTable.first.push_back(n.children[1].tokens[1] + ":");
    Node dcl = n.children[3];
    if (dcl.children[0].children.size() == 1) {
        mainTable.first.push_back("int");
    } else {
        mainTable.first.push_back("int*");
    }
    Node dcl1 =n.children[5];
    if (dcl1.children[0].children.size() == 1) {
        mainTable.first.push_back("int");
    } 
    mainTable.second.emplace(dcl.children[1].tokens[1], pair<string, int>(mainTable.first[1], 0));
    string dclName = dcl1.children[1].tokens[1];
    mainTable.second.emplace(dclName, pair<string, int>(mainTable.first[2], -4));
    cout << "main:" << endl;
    cout << "sub $29, $30, $4" << endl;
    cout << "sw $1, 0($29)" << endl;
    cout << "sub $30, $30, $4" << endl;
    cout << "sw $2, -4($29)" << endl;
    cout << "sub $30, $30, $4" << endl;
    if (mainTable.first[1] == "int") {
        cout << "add $2, $0, $0" << endl;
    }
    push(31);
    cout << "lis $5" << endl;
    cout << ".word init" << endl;
    cout << "jalr $5" << endl;
    pop(31);
    int offset = -8;
    getDcls(n.children[8], mainTable, &offset);
    getStatements(n.children[9], mainTable);
    string expr = getExpr(n.children[11], mainTable);
    cout << "jr $31" << endl;
}

void buildSymbolTable(Node& n) {
    for (auto c: n.children) {
        if (c.tokens[0] == "procedures") {
            buildSymbolTable(c);
        } else if (c.tokens[0] == "main") {
            table.emplace("main", pair<vector<string>, map<string, pair<string, int>>>(vector<string>(), map<string, pair<string, int>>()));
            getMain(c, table.at("main"));
        } else if (c.tokens[0] == "procedure") {
            string pro = c.children[1].tokens[1];
            table.emplace(pro, pair<vector<string>, map<string, pair<string, int>>>(vector<string>(), map<string, pair<string, int>>()));
            getProcedure(c, table.at(pro));
        }
    }
}

int main() {
    try {
        auto root = buildTree(cin);
        cout << ".import print" << endl;
        cout << ".import init" << endl;
        cout << ".import new" << endl;
        cout << ".import delete" << endl;
        cout << "lis $4" << endl;
        cout << ".word 4" << endl;
        cout << "lis $10" << endl;
        cout << ".word print" << endl;
        cout << "lis $11" << endl;
        cout << ".word 1" << endl;
        cout << "beq $0, $0, main" << endl;
        buildSymbolTable(root);
    } catch (Error &e) {
        cerr << e.what();
        return 1;
    }
}



