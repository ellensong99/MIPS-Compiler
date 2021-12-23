#include <iostream>
#include <string>
#include <vector>
#include "scanner.h"
#include <map>
#include <deque>
using namespace std;
using std::pair;
using std::string;
using std::vector;

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains the main function of your program. By default, it just
 * prints the scanned list of tokens back to standard output.
 */

bool valid_label(string s)
{
  int len = s.length();
  if (len > 0)
  {
    for (int i = 0; i < len; i++)
    {
      if (i == 0)
      {
        if (!(((s[i] >= 'a') & (s[i] <= 'z')) | ((s[i] >= 'A') & (s[i] <= 'Z'))))
        {
          return false;
        }
      }
      else if (i == len - 1)
      {
        if (s[len - 1] == ':')
        {
          break;
        }
        else
        {
          return false;
        }
      }
      else if (!(((s[i] >= 'a') & (s[i] <= 'z')) | ((s[i] >= 'A') & (s[i] <= 'Z')) | ((s[i] >= '0') & (s[i] <= '9'))))
      {
        return false;
      }
    }
    return true;
  }
}

bool valid_register(int n)
{
  //int n = stoi(reg.substr(1));
  return (n >= 0) & (n <= 31);
}

int main()
{
  string line;
  vector<vector<Token>> loT;
  int address = 0;
  vector<long> output;
  std::map<string, int> labelMapping;
  try
  {
    while (getline(std::cin, line))
    {
      vector<Token> tokenLine = scan(line);
      std::deque<Token> dtL;
      /*   vector<Token::Kind> tokenKind;
      vector<string> tokenLexeme;*/
      int len = tokenLine.size();
      for (int i = 0; i < len; i++)
      {
        dtL.push_back(tokenLine[i]);
      }
      while (!dtL.empty())
      {
        Token::Kind curKind = dtL[0].getKind();
        string curLex = dtL[0].getLexeme();
        if (curKind == Token::LABEL)
        {
          if (!valid_label(curLex))
          {
            throw ScanningFailure("ERROR: Invalid Label Defined");
          }
          if (labelMapping.count(curLex.substr(0, curLex.size() - 1)) != 0)
          {
            throw ScanningFailure("ERROR: Duplicated Labels.");
          }
          labelMapping[curLex.substr(0, curLex.size() - 1)] = address;
          dtL.pop_front();
        }
        else if (curKind == Token::WORD)
        {
          Token::Kind nextKind = dtL[1].getKind();
          if (nextKind == Token::INT | nextKind == Token::HEXINT | nextKind == Token::ID)
          {
            vector<Token> saved;
            for (int i = 0; i < 2; i++)
            {
              saved.push_back(dtL[0]);
              dtL.pop_front();
            }
            loT.push_back(saved);
            address += 4;
          }
          else
          {
            throw ScanningFailure("ERROR: Invalid Input followed by .word");
          }
        }
        else if (curKind == Token::ID)
        {
          if (curLex == "jr" | curLex == "jalr" | (curLex == "mfhi") | (curLex == "mflo") | (curLex == "lis"))
          {
            if (dtL.size() != 2)
            {
              cerr << "ERROR: Invalid Input, Number of arguments exceeds." << endl;
              return 1;
            }
            else if (dtL[1].getKind() == Token::REG)
            {
              if (valid_register(dtL[1].toNumber()))
              {
                vector<Token> saved;
                for (int i = 0; i < 2; i++)
                {
                  saved.push_back(dtL[0]);
                  dtL.pop_front();
                }
                loT.push_back(saved);
                address += 4;
              }
              else
              {
                cerr << "ERROR: Invalid Register." << endl;
                return 1;
              }
            }
            else
            {
              cerr << "ERROR: Invalid Input. Enter a register." << endl;
              return 1;
            }
          }
          else if (curLex == "add" | curLex == "sub" | curLex == "slt" | curLex == "sltu")
          {
            if (dtL.size() != 6)
            {
              throw ScanningFailure("ERROR: Invalid Input. Need 3 valid registers.");
            }
            else
            {
              for (int i = 1; i < 6; i++)
              {
                if ((dtL[i].getKind() == Token::COMMA & (i % 2 == 0)) | ((dtL[i].getKind() == Token::REG & (i % 2 == 1) & valid_register(dtL[i].toNumber()))))
                {
                  continue;
                }
                else
                {
                  cerr << "ERROR: Invalid Input. Need 3 valid registers." << endl;
                  return 1;
                }
              }
              vector<Token> saved;
              for (int i = 0; i < 6; i++)
              {
                saved.push_back(dtL[0]);
                dtL.pop_front();
              }
              loT.push_back(saved);
              address += 4;
            }
          }
          else if ((curLex == "mult") | (curLex == "multu") | (curLex == "div") | (curLex == "divu"))
          {
            if (dtL.size() != 4)
            {
              cerr << "ERROR: Invalid Input. Incorrect number of arguments for mult/multu/div/divu." << endl;
              return 1;
            }
            else
            {
              for (int i = 1; i < 4; i++)
              {
                if (((i == 2) & (dtL[2].getKind() == Token::COMMA)) | ((i % 2 == 1) & (dtL[i].getKind() == Token::REG) & (valid_register(dtL[i].toNumber()))))
                {
                  continue;
                }
                else
                {
                  cerr << "ERROR: Invalid Input. Need 2 valid registers for mult/multu/div/divu" << endl;
                  return 1;
                }
              }
              vector<Token> saved;
              for (int i = 0; i < 4; i++)
              {
                saved.push_back(dtL[0]);
                dtL.pop_front();
              }
              loT.push_back(saved);
              address += 4;
            }
          }
          else if ((curLex == "lw") | (curLex == "sw"))
          {
            if (dtL.size() != 7)
            {
              cerr << "ERROR: Invalid Input. Incorrect number of arguments for lw/sw." << endl;
              return 1;
            }
            else if ((dtL[1].getKind() == Token::REG) & (dtL[2].getKind() == Token::COMMA) & ((dtL[3].getKind() == Token::INT) | (dtL[3].getKind() == Token::HEXINT)) & (dtL[4].getKind() == Token::LPAREN) & (dtL[5].getKind() == Token::REG) & (dtL[6].getKind() == Token::RPAREN))
            {
              if (valid_register(dtL[1].toNumber()) & valid_register(dtL[5].toNumber()))
              {
                vector<Token> saved;
                for (int i = 0; i < 7; i++)
                {
                  saved.push_back(dtL[0]);
                  dtL.pop_front();
                }
                loT.push_back(saved);
                address += 4;
              }
            }
            else
            {
              cerr << "ERROR: Invalid Input for lw/sw." << endl;
              return 1;
            }
          }
          else
          {
            throw ScanningFailure("ERROR: Invalid Input.");
          }
        }
        else
        {
          throw ScanningFailure("ERROR: Invalid Command.");
        }
      }
    }
  }
  catch (ScanningFailure &f)
  {
    cerr << f.what() << endl;
    return 1;
  }
  // now we start to proceed instructions
  for (int i = 0; i < loT.size(); i++)
  {
    vector<Token::Kind> tokenKind;
    vector<string> tokenLexeme;
    for (int j = 0; j < loT[i].size(); j++)
    {
      tokenKind.push_back(loT[i][j].getKind());
      tokenLexeme.push_back(loT[i][j].getLexeme());
    }
    if (tokenKind[0] == Token::WORD)
    {
      if (tokenKind[1] == Token::INT)
      {
        int64_t n = loT[i][1].toNumber();
        if (n <= 4294967295 & n >= -2147483648)
        {
          output.push_back(n);
        }
        else
        {
          throw ScanningFailure("ERROR: Integer Overflow.");
        }
      }
      else if (tokenKind[1] == Token::ID)
      {
        string label = tokenLexeme[1];
        if (labelMapping.count(label) != 0)
        {
          int n = labelMapping[label];
          output.push_back(n);
        }
        else
        {
          cerr << "ERROR: Label Does Not Exist." << endl;
          return 1;
        }
      }
      else if (tokenKind[1] == Token::HEXINT)
      {
        int64_t n = loT[i][1].toNumber();
        if (n <= 0xffffffff)
        {
          output.push_back(n);
        }
        else
        {
          throw ScanningFailure("ERROR: HEXINT overflow");
        }
      }
      else
      {
        throw ScanningFailure("Invalid Input: .word should be followed by INT/HEXINT/ID");
      }
    }
    else if (tokenKind[0] == Token::ID)
    {
      string op = tokenLexeme[0];
      if ((op == "jr") | (op == "jalr") | (op == "lis") | (op == "mflo") | (op == "mfhi"))
      {
        int n = loT[i][1].toNumber();
        int inst = 0;
        if (op == "jr")
        {
          inst = (n << 21) | 8;
        }
        else if (op == "jalr")
        {
          inst = (n << 21) | 9;
        }
        else if (op == "lis")
        {
          inst = (n << 11) | 20;
        }
        else if (op == "mflo")
        {
          inst = (n << 11) | 18;
        }
        else
        {
          inst = (n << 11) | 16;
        }
        output.push_back(inst);
      }
      else if (op == "add")
      {
        int n1 = loT[i][1].toNumber();
        int n2 = loT[i][3].toNumber();
        int n3 = loT[i][5].toNumber();
        int inst = (n1 << 11) | (n2 << 21) | (n3 << 16) | 32;
        output.push_back(inst);
      }
      else if (op == "sub")
      {
        int n1 = loT[i][1].toNumber();
        int n2 = loT[i][3].toNumber();
        int n3 = loT[i][5].toNumber();
        int inst = (n1 << 11) | (n2 << 21) | (n3 << 16) | 34;
        output.push_back(inst);
      }
      else if (op == "slt")
      {
        int n1 = loT[i][1].toNumber();
        int n2 = loT[i][3].toNumber();
        int n3 = loT[i][5].toNumber();
        int inst = (n1 << 11) | (n2 << 21) | (n3 << 16) | 42;
        output.push_back(inst);
      }
      else if (op == "sltu")
      {
        int n1 = loT[i][1].toNumber();
        int n2 = loT[i][3].toNumber();
        int n3 = loT[i][5].toNumber();
        int inst = (n1 << 11) | (n2 << 21) | (n3 << 16) | 43;
        output.push_back(inst);
      }
      else if ((op == "mult") | (op == "multu") | (op == "div") | (op == "divu"))
      {
        int n1 = loT[i][1].toNumber();
        int n2 = loT[i][3].toNumber();
        int inst = 0;
        if (op == "mult")
        {
          inst = (n1 << 21) | (n2 << 16) | 24;
        }
        else if (op == "multu")
        {
          inst = (n1 << 21) | (n2 << 16) | 25;
        }
        else if (op == "div")
        {
          inst = (n1 << 21) | (n2 << 16) | 26;
        }
        else
        {
          inst = (n1 << 21) | (n2 << 16) | 27;
        }
        output.push_back(inst);
      }
      else if ((op == "lw") | (op == "sw"))
      {
        int64_t n1 = loT[i][1].toNumber();
        int64_t n2 = loT[i][5].toNumber();
        int inst = 0;
        if (tokenKind[3] == Token::INT)
        {
          int64_t n = loT[i][3].toNumber();
          if ((n >= -32768) & (n <= 32767))
          {
            if (op == "lw")
            {
              inst = ((n2 << 21) | (n1 << 16) | (140 << 24) | (n & 65535));
            }
            else
            {
              inst = ((n2 << 21) | (n1 << 16) | (172 << 24) | (n & 65535));
            }
            output.push_back(inst);
          }
          else
          {
            cerr << "ERROR: Invalid offset." << endl;
            return 1;
          }
        }
        else if (tokenKind[3] == Token::HEXINT)
        {
          int64_t n = loT[i][3].toNumber();
          if ((n >= 0) & (n <= 65535))
          {
            if (op == "lw")
            {
              inst = ((n2 << 21) | (n1 << 16) | (140 << 24) | (n & 65535));
            }
            else
            {
              inst = ((n2 << 21) | (n1 << 16) | (172 << 24) | (n & 65535));
            }
            output.push_back(inst);
          }
          else
          {
            cerr << "ERROR: Invalid offset in hex." << endl;
            return 1;
          }
        }
      }
    }
  }
  // print & output
  for (auto i : labelMapping)
  {
    cerr << i.first << " " << i.second << endl;
  }

  for (auto i : output)
  {
    putchar(i >> 24);
    putchar(i >> 16);
    putchar(i >> 8);
    putchar(i);
  }
  return 0;
}

