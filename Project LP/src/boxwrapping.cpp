#include<fstream>
#include<map>
#include<vector>
#include<algorithm>
#include <ilcplex/ilocplex.h>

ILOSTLBEGIN

// Auxiliar function to compare two boxes by size. Used when sorting
// the input boxes.
bool compareBoxes(const pair<int,int>& b1, const pair<int,int>& b2) {
  return (b1.first*b1.second) > (b2.first*b2.second);
}

int main(int argc, char* argv[]) {
  // Read input and save it. 
  ifstream infile;
  vector<pair<int,int> > boxes;
  IloInt w, i = 0;
  // maxLength defines the maximum length the program can return, which 
  // is the sum of the largest dimension of each box (i.e. as if we placed
  // them in a single column).
  IloInt maxLength = 0;
  bool firstLine = true;
  infile.open(argv[1]);
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
  // Sort the boxes by decreasing size.
  sort(boxes.begin(),boxes.end(),compareBoxes);
  IloEnv env;
  try {
    IloIntVarArray x_tl(env,boxes.size(),0,w-1);
    IloIntVarArray x_br(env,boxes.size(),0,w-1);
    IloIntVarArray y_tl(env,boxes.size(),0,maxLength);
    IloIntVarArray y_br(env,boxes.size(),0,maxLength);
    IloExpr length(env);
    IloModel model(env);
    for(IloInt i = 0; i < boxes.size(); i++) {
      int boxWidth = boxes[i].first;
      int boxHeight = boxes[i].second;
      IloIntVar finalWidth(env);
      IloIntVar finalHeight(env);
      model.add(finalWidth == boxWidth || finalWidth == boxHeight);
      model.add(finalHeight == boxHeight || finalHeight == boxWidth);
      if(boxWidth != boxHeight) 
	model.add(finalWidth != finalHeight);
      if(i == 0) 
	model.add(x_tl[0] <= 1/2*(w - finalWidth));
      model.add(x_tl[i] <= w-finalWidth);
      model.add(x_tl[i] <= x_br[i]);
      model.add(y_tl[i] <= y_br[i]);
      model.add((x_br[i] == x_tl[i]+finalWidth-1));
      model.add((y_br[i] == y_tl[i]+finalHeight-1));
      for(IloInt j = i+1; j < boxes.size(); j++) {
	if(boxes[j].first*boxes[j].second == boxes[i].first*boxes[i].second)
	  model.add((x_tl[j] >= x_br[i]+1) || (y_tl[j] >= y_br[i]+1)); 
	// Two boxes cannot be overlapped in the paper roll. 
	else 
	  model.add((x_tl[j] >= x_br[i]+1) || (x_br[j] <= x_tl[i]-1) || (y_tl[j] >= y_br[i]+1) || (y_br[j] <= y_tl[i]-1));
      }
    }
    length = IloMax(y_br)+1;
    model.add(IloMinimize(env,length));
    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());
    if (cplex.solve()) {
      cout << cplex.getObjValue() << endl;
      for (IloInt i = 0; i < boxes.size(); i++) {
	IloInt xtl = cplex.getValue(x_tl[i]);
	IloInt ytl = cplex.getValue(y_tl[i]);
	IloInt xbr = cplex.getValue(x_br[i]);
	IloInt ybr = cplex.getValue(y_br[i]);
	if(xtl < 1) cout << "0 ";
	else cout << xtl << " ";
	if(ytl < 1) cout << "0    ";
	else cout << ytl << "    ";
	if(xbr < 1) cout << "0 ";
	else cout << xbr << " ";
	if(ybr < 1) cout << "0 " << endl;
	else cout << ybr << " " << endl;	
      }
    }
    else {
      cout << " No solution found" << endl;
    }
   }
   catch (IloException& ex) {
     cerr << "Error: " << ex << endl;
   }
   catch (...) {
     cerr << "Error" << endl;
   }
   env.end();
   return 0;
}
