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

typedef string literal;
typedef string  clause;

int w;
int maxLength;
vector<pair<int,int> > boxes;
vector<literal> rotVars;
int n_vars;
int n_clauses;

ofstream cnf;
ifstream sol;

bool compareBoxes(const pair<int,int>& b1, const pair<int,int>& b2) {
  return (b1.first*b1.second) > (b2.first*b2.second);
}

literal operator-(const literal& lit) {
  if (lit[0] == '-') return lit.substr(1); 
  else               return "-" + lit;
}


literal tl(int i, int j, int k) {
  assert(0 <= i and i < w);
  assert(0 <= j and j < maxLength);
  return to_string(j*w + i + k*w*maxLength + 1) + "  ";
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

void add_no_overlap(int b1, int width, int height, literal rot1) {
  for(int i = 0; i <= w-width; i++) {
    for(int j = 0; j <= maxLength-height; j++) {    
      for(int b2 = 0; b2 < boxes.size(); b2++) {
	literal rot2 = rotVars[b2];
	if(boxes[b2].first == boxes[b2].second)
	  rot2 = "";
	int start1 = i-boxes[b2].first+1;
	if(start1 < 0) start1 = 0;
	int finish1 = i+width;
	if(finish1 > w) finish1 = w;
	for(int k = start1; k < finish1; k++) {
	  int start2 = j-boxes[b2].second+1;
	  if(start2 < 0) start2 = 0;
	  int finish2 = j+height;
	  if(finish2 > maxLength) finish2 = maxLength;
	  for(int l = start2; l < finish2; l++) { 
	    if(b1 != b2) {
	      add_clause(-tl(i,j,b1) + " " + -tl(k,l,b2) + " " + rot1 + " " + rot2 + " ");
	    }
	  }
	}
      }
      for(int b2 = 0; b2 < boxes.size(); b2++) {
	literal rot2 = -rotVars[b2];
	if(boxes[b2].first == boxes[b2].second)
	  rot2 = "";
	int start1 = i-boxes[b2].second+1;
	if(start1 < 0) start1 = 0;
	int finish1 = i+width;
	if(finish1 > w) finish1 = w;
	for(int k = start1; k < finish1; k++) {
	  int start2 = j-boxes[b2].first+1;
	  if(start2 < 0) start2 = 0;
	  int finish2 = j+height;
	  if(finish2 > maxLength) finish2 = maxLength;
	  for(int l = start2; l < finish2; l++) {
	    if(b1 != b2) {
	      add_clause(-tl(i,j,b1) + " " + -tl(k,l,b2) + " " + rot1 + " " + rot2 + " ");
	    }
	  }
	}
      }
    }
  }
}

void write_CNF() {
  n_vars = w*maxLength*boxes.size();
  rotVars = vector<literal>(boxes.size());
  for(int i = 1; i <= boxes.size(); i++) {
    rotVars[i-1] = to_string(w*maxLength*boxes.size() + i);
  }
  n_vars += boxes.size();
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
	  c += tl(i,j,b);
      }
    }
    add_clause(c);
  }

  // At most one top-left coordinate for each box.
  for(int b = 1; b < boxes.size(); b++) {
    vector<literal> z;
    for(int i = 0; i < w; i++) {
      for(int j = 0; j < maxLength; j++) {
	  z.push_back(tl(i,j,b));
      }
    }
    add_amo(z);
  }

  // Boxes can not fall out of the paper
  for(int b = 0; b < boxes.size(); b++) {
    // If boxes have the same width and height, we do not rotate them.
    if(boxes[b].first == boxes[b].second) 
      add_clause(-rotVars[b] + " ");
    for(int i = 0; i < w; i++) {
      for(int j = 0; j < maxLength; j++) {
	if(boxes[b].first != boxes[b].second) {
	  if(i > w-boxes[b].first or j > maxLength-boxes[b].second){
	    add_clause(rotVars[b] + " " + -tl(i,j,b));
	  }
	  if(i > w-boxes[b].second or j > maxLength-boxes[b].first) {
	    add_clause(-rotVars[b] + " " + -tl(i,j,b));
	  }
	}
	else {
	  if(i > w-boxes[b].first or j > maxLength-boxes[b].second)
	    add_clause(-tl(i,j,b));
	}
      }
    }
  }

  // Boxes that have the same dimensions limit the coordinates of subsequent boxes.
  for(int b = 0; b < boxes.size(); b++) {
    if(b > 0) {
      if(boxes[b-1].first == boxes[b].first and boxes[b-1].second == boxes[b].second) {
	for(int i = 0; i < w; i++) {
	  for(int j = 0; j < maxLength; j++) {
	    for(int k = 0; k < i; k++) {
	      for(int l = 0; l < j; l++) {
		add_clause(-tl(i,j,b-1) + " " + -tl(k,l,b));
	      }
	    }
	  }
	}
      }
    }
  }

  // Boxes can not overlap
  for(int b1 = 0; b1 < boxes.size(); b1++) {
    literal rot1 = rotVars[b1];
    if(boxes[b1].first == boxes[b1].second)
      rot1 = "";
    add_no_overlap(b1,boxes[b1].first,boxes[b1].second,rot1);
    rot1 = -rotVars[b1];
    if(boxes[b1].first == boxes[b1].second)
      rot1 = "";
    add_no_overlap(b1,boxes[b1].second,boxes[b1].first,rot1);    
  }
}


void get_solution(vector<pair<int,int> >& q, vector<bool>& rotated, int& length) {
  int lit, i = 0;
  while(sol >> lit) {
    if(lit > 0 and lit < w*maxLength*boxes.size()+1) {
      //cout << lit << " ";
      double x_tl = ((lit-1)%(maxLength*w)) % w;
      double y_tl = (((lit-1)-(lit/(maxLength*w))*(maxLength*w))) / w;
      //cout << "-> x_tl = " << x_tl << ", y_tl = " << y_tl << endl;
      q.push_back(make_pair(x_tl,y_tl));
    }
    if(abs(lit) >= w*maxLength*boxes.size()+1) {
      //cout << endl << "rot: " << lit;
      if(lit > 0) {
	rotated[i] = true;
      }
      ++i;
    }
  }
  length = 0;
  if(q.size()) {
    for(int i = 0; i < boxes.size(); i++) {
      int max = q[i].second;
      if(rotated[i]) 
	max += boxes[i].first;
      else 
	max += boxes[i].second;
      if(length < max) {
	length = max;
      }
    }
  }
}


void write_solution(const vector<pair<int,int> >& q, const vector<bool>& rotated, int& length) {
  cout << length << endl;
  for(int i = 0; i < boxes.size(); i++) {
    int x_tl = q[i].first;
    int y_tl = q[i].second;    
    cout << x_tl << " " << y_tl << "     ";
    if(rotated[i]) cout << x_tl+boxes[i].second-1 << " " << y_tl+boxes[i].first-1 << endl;
    else cout << x_tl+boxes[i].first-1 << " "<< y_tl+boxes[i].second-1 << endl;
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
  sort(boxes.begin(),boxes.end(),compareBoxes);
  cnf.open("tmp.rev");
  write_CNF();
  cnf << "p cnf " << n_vars << " " << n_clauses << endl;
  cnf.close();
  system("tac tmp.rev | ./lingeling | grep -E -v \"^c\" | tail --lines=+2 | cut --delimiter=' ' --field=1 --complement > tmp.out");
  vector<pair<int,int> > q;
  vector<bool> rotated(boxes.size(),false);
  int length;
  sol.open("tmp.out");
  get_solution(q,rotated,length);
  sol.close();
  while(q.size()) {
    vector<pair<int,int> > currentSol = q;
    vector<bool> currentRot = rotated;
    int currentLength = length;
    //write_solution(q,rotated,length);
    system("head -n -1 tmp.rev > temp.txt ; mv temp.txt tmp.rev");
    cnf.open("tmp.rev",ios_base::app);
    for(int b = 0; b < boxes.size(); b++) {
      for(int i = 0; i < w; i++) {
	int yCoord1 = boxes[b].second;
	int yCoord2 = boxes[b].first;
	if(yCoord1 == yCoord2) {
	  for(int j = length-yCoord1; j < maxLength; j++) {
	    add_clause(-tl(i,j,b));
	  }
	}
	else {
	  if(rotated[b]) {
	    yCoord1 = boxes[b].first;
	    yCoord2 = boxes[b].second;
	  }
	  int start = length-yCoord1;
	  if(start >= 0) {
	    if(not start) {
	      start = 1;
	      add_clause(rotVars[b] + " ");
	    }
	    for(int j = start; j < maxLength; j++) {
	      add_clause(rotVars[b] + " " + -tl(i,j,b));
	    }
	  }
	  start = length-yCoord2;
	  if(start >= 0) {
	    if(not start) {
	      start = 1;
	      add_clause(rotVars[b] + " ");
	    }
	    for(int j = start; j < maxLength; j++) {
	      add_clause(-rotVars[b] + " " + -tl(i,j,b));
	    }
	  }
	}
      }
    }
    cnf << "p cnf " << n_vars << " " << n_clauses << endl;
    cnf.close();
    system("tac tmp.rev | ./lingeling | grep -E -v \"^c\" | tail --lines=+2 | cut --delimiter=' ' --field=1 --complement > tmp.out");
    q = vector<pair<int,int> >();
    rotated = vector<bool>(boxes.size(),false);
    sol.open("tmp.out");
    get_solution(q,rotated,length);
    sol.close();
    if(length == 1) {
      write_solution(q,rotated,length);
      break;
    }
    if(q.empty()) write_solution(currentSol,currentRot,currentLength);
  }
}
