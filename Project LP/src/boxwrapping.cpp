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
    // Top-left horizontal coordinate.
    IloNumVarArray x_tl(env,boxes.size(),0,w-1);
    // Top-left vertical coordinate.
    IloNumVarArray y_tl(env,boxes.size(),0,maxLength-1);
    // Widths of the boxes.
    IloNumVarArray width(env,boxes.size(),0,w);
    // Heights of the boxes.
    IloNumVarArray height(env,boxes.size(),0,maxLength-1);
    // Total length of the paper roll used. 
    IloNumVar length(env,0,maxLength-1);
    IloModel model(env);
    x_tl[0].setBounds(0,0);
    y_tl[0].setBounds(0,0);
    for(IloInt i = 0; i < boxes.size(); i++) {
      int boxWidth = boxes[i].first;
      int boxHeight = boxes[i].second;
      // If a box has different width and height dimensions, the final width of it can either 
      // be the original width or the original height. The same applies to the height of the box. 
      if(boxWidth != boxHeight) {
	model.add(width[i] == boxWidth || width[i] == boxHeight);
	model.add(height[i] == boxHeight || height[i] == boxWidth);
	model.add(width[i] != height[i]);
      }
      // If a box has the same width and height, the width and height of the box keep
      // having their original values.
      else {
	width[i].setBounds(boxWidth,boxWidth);
	height[i].setBounds(boxHeight,boxHeight);
      }
      // The top-left horizontal of a box is at most the total width of the paper minus its width. 
      model.add(x_tl[i]+width[i] <= w);
      for(IloInt j = i+1; j < boxes.size(); j++) {
	// If two boxes have the same dimensions, one of them bounds the possible values of the coordinates
	// of the other. Since they have the same dimensions, it makes sense to determine the coordinates of
	// one based on the other. For instance, if one of the boxes does not fit somewhere, the other one
	// would not fit there either.
	if(boxes[j].first == boxes[i].first && boxes[j].second == boxes[i].second) {
	  model.add((x_tl[j] >= x_tl[i]+width[i]) || (y_tl[j] >= y_tl[i]+height[i]));
	} 
	// Two boxes cannot be overlapped in the paper roll. 
	else
	  model.add((x_tl[j]+width[j] <= x_tl[i]) || (x_tl[j] >= x_tl[i]+width[i]) || (y_tl[j]+height[j] <= y_tl[i]) || (y_tl[j] >= y_tl[i]+height[i]));
      }
      // The length of the paper is equal to the maximum bottom-right coordinate of all boxes. 
      model.add(length >= y_tl[i]+height[i]-1);
    }
    model.add(IloMinimize(env,length));
    IloCplex cplex(model);
    cplex.setOut(env.getNullStream());
    if (cplex.solve()) {
      cout << cplex.getObjValue()+1 << endl;
      for (IloInt i = 0; i < boxes.size(); i++) {
	// The final values get rounded up. 
	IloInt xtl = int(cplex.getValue(x_tl[i])+0.5);
	IloInt ytl = int(cplex.getValue(y_tl[i])+0.5);
	IloInt w = int(cplex.getValue(width[i])+0.5);
	IloInt h = int(cplex.getValue(height[i])+0.5);
	if(xtl < 1) cout << "0 ";
	else cout << xtl << " ";
	if(ytl < 1) cout << "0    ";
	else cout << ytl << "    ";
	if(xtl < 1) cout << w-1 << " ";
	else cout << xtl+w-1 << " ";
	if(ytl < 1) cout << h-1 << endl;	
	else cout << ytl+h-1 << endl;	
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
