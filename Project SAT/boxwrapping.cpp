#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <algorithm>

#define V +

using namespace std;

int w;
int maxLength;
vector<pair<int,int> > boxes;
int n_vars;
int n_clauses;

ofstream cnf;
ifstream sol;

typedef string literal;
typedef string  clause;

bool compareBoxes(const pair<int,int>& b1, const pair<int,int>& b2) {
  return (b1.first*b1.second) > (b2.first*b2.second);
}

literal operator-(const literal& lit) {
  if (lit[0] == '-') return lit.substr(1); 
  else               return "-" + lit;
}


literal tl(int i, int j, int k) {
  assert(0 <= i and i < maxLength);
  assert(0 <= j and j < w);
  return to_string(i*w + j + k*w*maxLength + 1) + "  ";
}

void add_clause(const clause& c) {
  cnf << c << "0" << endl;
  ++n_clauses;
}


void add_amo(const vector<literal>& z) {
  int N = z.size();
  for (int i1 = 0; i1 < N; ++i1)
    for (int i2 = i1+1; i2 < N; ++i2)
      add_clause(-z[i1] V -z[i2]);
}

void write_CNF() {

  n_vars = w*maxLength*boxes.size();

  // Put the first box in the highest top-left coordinate in the paper.
  add_clause(tl(0,0,0));
  for(int i = 0; i < w; i++) {
    for(int j = 0; j < maxLength; j++) {
      if(i or j)
	add_clause(-tl(i,j,0));
    }
  }

  // At least 1 top-left coordinate for each box.
  for(int b = 1; b < boxes.size(); b++) {
    clause c;
    for(int i = 0; i < w; i++) {
      for(int j = 0; j < maxLength; j++) {
	// Boxes can not fall out of the paper.
	if(i > w-boxes[b].first or j > maxLength-boxes[b].second)
	  add_clause(-tl(i,j,b));
	else
	  c += tl(i,j,b);
      }
    }
    add_clause(c);
  }

  // At most one top-left coordinate for each box
  for(int b = 1; b < boxes.size(); b++) {
    vector<literal> z;
    for(int i = 0; i < w; i++) {
      for(int j = 0; j < maxLength; j++) {
	// Boxes can not fall out of the paper.
	if(i <= w-boxes[b].first or j <= maxLength-boxes[b].second)
	  z.push_back(tl(i,j,b));
      }
    }
    add_amo(z);
  }

  // Boxes can not overlap
  for(int b1 = 0; b1 < boxes.size()-1; b1++) {
    for(int i = 0; i <= w-boxes[b1].first; i++) {
      for(int j = 0; j <= maxLength-boxes[b1].second; j++) {    
	for(int k = i; k < i+boxes[b1].first; k++) {
	  for(int l = j; l < j+boxes[b1].second; l++) {
	    vector<literal> z;
	    z.push_back(tl(i,j,b1));
	    for(int b2 = 0; b2 < boxes.size(); b2++) {
	      if(b1 != b2) {
		z.push_back(tl(k,l,b2));
	      }
	    }
	    add_amo(z);
	  }
	}
      }
    }
  }
}


void get_solution(vector<pair<int,int> >& q) {
  int lit;
  while(sol >> lit) {
    if(lit > 0) {
      //cout << lit << " ";
      double x_tl = ((lit-1)%(maxLength*w)) % w;
      double y_tl = ((lit-1)%(maxLength*w)) / maxLength;
      q.push_back(make_pair(x_tl,y_tl));
    }
  }
  cout << endl;
}


void write_solution(vector<pair<int,int> >& q) {
  int max = 0;
  for(int i = 0; i < boxes.size(); i++) {
    if(q[i].second > max) 
      max = q[i].second;
  }
  cout << max+1 << endl;
  for(int i = 0; i < boxes.size(); i++) {
    int x_tl = q[i].first;
    int y_tl = q[i].second;    
    cout << x_tl << " " << y_tl << "     " << x_tl+boxes[i].first-1 << " " << y_tl+boxes[i].second-1 << endl;
  }
}

void readInput(char* argv) {
  // Read input and save it. 
  ifstream infile;
  int i = 0;
  bool firstLine = true;
  infile.open(argv);
  while(!infile.eof()) {
    string line;
    getline(infile,line);
    string buf; 
    stringstream ss(line); 
    vector<string> tokens; 
    while (ss >> buf)
      tokens.push_back(buf);
    if(tokens.size()) {
      if(firstLine) {
	w = atoi(tokens[0].c_str());
	firstLine = false;
      }
      else {
	int width = atoi(tokens[1].c_str());
	int height = atoi(tokens[2].c_str());
	int nBoxes = atoi(tokens[0].c_str());
	for(int j = i; j < nBoxes+i; j++) {
	  boxes.push_back(make_pair(width,height));
	  maxLength += max(width,height);
	}
	i += nBoxes;
      }
    }
  }
  infile.close();
}

int main(int argc, char** argv) {
  assert(argc == 2);
  readInput(argv[1]);
  for(auto& b : boxes)
    cout << b.first << "x" << b.second << endl;
  sort(boxes.begin(),boxes.end(),compareBoxes);
  cnf.open("tmp.rev");
  write_CNF();
  cnf << "p cnf " << n_vars << " " << n_clauses << endl;
  cnf.close();
  system("tac tmp.rev | ./lingeling | grep -E -v \"^c\" | tail --lines=+2 | cut --delimiter=' ' --field=1 --complement > tmp.out");
  vector<pair<int,int> > q;
  sol.open("tmp.out");
  get_solution(q);
  sol.close();
  write_solution(q);
  while(q.size()) {
    write_solution(q);
    system("head -n -1 tmp.rev > temp.txt ; mv temp.txt tmp.rev");
    cnf.open("tmp.rev",ios_base::app);
    int max = 0;
    for(int i = 0; i < boxes.size(); i++) {
      if(q[i].second > max) 
	max = q[i].second;
    }
    for(int b = 0; b < boxes.size(); b++) {
      clause c;
      for(int i = 0; i < w; i++) {
	for(int j = max-boxes[b].second+1; j < maxLength; j++) {
	  add_clause(-tl(i,j,b));
	}
      }
    }
    cnf << "p cnf " << n_vars << " " << n_clauses << endl;
    cnf.close();
    system("tac tmp.rev | ./lingeling | grep -E -v \"^c\" | tail --lines=+2 | cut --delimiter=' ' --field=1 --complement > tmp.out");
    q = vector<pair<int,int> >();
    sol.open("tmp.out");
    get_solution(q);
    sol.close();
  }
}
