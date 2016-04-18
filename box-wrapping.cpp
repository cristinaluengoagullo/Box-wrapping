#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include<fstream>
#include<map>
#include<set>

using namespace Gecode;
using namespace std;

bool compareBoxes(const pair<int,int>& b1, const pair<int,int>& b2) {
  return (b1.first*b1.second) > (b2.first*b2.second);
}

class BoxWrapping : public Space {

private:

  vector<pair<int,int> > boxes;
  int w;

protected:

  IntVar length;
  IntVarArray x_tl;
  IntVarArray x_br;
  IntVarArray y_tl;
  IntVarArray y_br;

public:

  int value_aux(IntVar x, int i) const {
    if(i == 0) {
      return x.min();
    }
    int val = x.min();
    for (IntVarValues k(x); k( ) ; ++k) {
      int maxDim = max(boxes[i-1].first,boxes[i-1].second);
      if(maxDim > w)
	maxDim = min(boxes[i-1].first,boxes[i-1].second);
      if(k.val() >= x_tl[i-1].val()+maxDim) {
	val = k.val();
	break;
      }
    }
    return val;
  }

  static int value(const Space& home, IntVar x, int i) {
    static_cast<const BoxWrapping&>(home).value_aux(x,i);
  }

  static void commit(Space& home, unsigned int a,
		IntVar x, int i, int n) {
    if (a == 0U) rel(home, x, IRT_EQ, n);
    else         rel(home, x, IRT_NQ, n);
  }

  BoxWrapping(int _w, int maxLength, const vector<pair<int,int> >& _boxes) : 
    length(*this,0,maxLength),
    x_tl(*this,_boxes.size(),0,_w-1), 
    x_br(*this,_boxes.size(),0,_w-1),
    y_tl(*this,_boxes.size(),0,maxLength),
    y_br(*this,_boxes.size(),0,maxLength),
    w(_w),
    boxes(_boxes)
  {
    for(int i = 0; i < boxes.size(); i++){
      int width = boxes[i].first;
      int height = boxes[i].second;    
      int dims[2] = {width, height};
      IntSet dimsSet(dims,2);
      // We define the domain for the width and height of the box. Since boxes can be rotated,
      // the domain contains both the width and height originally defined for the box.
      IntVar bwidth(*this,dimsSet);
      IntVar bheight(*this,dimsSet);
      if(width != height) 
	rel(*this,bwidth != bheight);
      if(i == 0) {
	rel(*this, x_tl[0] <= 1/2*(w - bwidth));
      }
      rel(*this, x_tl[i] <= w-bwidth);
      rel(*this,x_tl[i] <= x_br[i]);
      rel(*this,y_tl[i] <= y_br[i]);
      rel(*this,(x_br[i] == x_tl[i]+bwidth-1));
      rel(*this,(y_br[i] == y_tl[i]+bheight-1));
      for(int j = i+1; j < boxes.size(); j++) {
	// If a box has the same size than another box, we constrain its domain to the bounds
	// that the other box defines. Since we know that the algorithm will try to assign
	// the smallest values possible to variables, we know that if a box with a certain 
	// size has been assigned some values, it is because they are the smallest ones
	// it could get. And hence a box with the same size can only get bigger values in
	// its left-side limits.
	//if(j > i+1 and boxes[j].first*boxes[j].second == boxes[j-1].first*boxes[j-1].second)
	if(boxes[j].first*boxes[j].second == boxes[i].first*boxes[i].second)
	  rel(*this,(x_tl[j] > x_br[i]) || (y_tl[j] > y_br[i])); 
	// Two boxes cannot be overlapped in the paper roll. 
	else 
	  rel(*this,(x_tl[j] > x_br[i]) || (x_br[j] < x_tl[i]) || (y_tl[j] > y_br[i]) || (y_br[j] < y_tl[i]));
      }
    }
    rel(*this, max(y_br)+1 == length);
    branch(*this, x_tl, INT_VAR_NONE(), INT_VAL(&value, &commit));
    branch(*this, y_tl, INT_VAR_NONE(), INT_VAL_MIN());
    branch(*this, y_br, INT_VAR_NONE(), INT_VAL_MIN());
  }

  BoxWrapping(bool share, BoxWrapping& s) : Space(share, s) {
    length.update(*this,share,s.length);
    x_tl.update(*this,share,s.x_tl);
    x_br.update(*this,share,s.x_br);
    y_tl.update(*this,share,s.y_tl);
    y_br.update(*this,share,s.y_br);
    boxes = s.boxes;
    w = s.w;
  }

  virtual Space* copy(bool share) {
    return new BoxWrapping(share,*this);
  }

  void print(void) const {
    cout << length.val() << endl;
    for(int i = 0; i < x_tl.size(); i++) {
    cout << x_tl[i].val() << " " << y_tl[i].val() << "    " << x_br[i].val() << " " << y_br[i].val() << endl;
    }
  }

  // Constrain function
  virtual void constrain(const Space& _b) {
    const BoxWrapping& b = static_cast<const BoxWrapping&>(_b);
    // The best solution is the one which has the least length of paper.
    rel(*this,length < b.length);
  }
};

int main(int argc, char* argv[]) {
  // Read input and save it. 
  ifstream infile;
  vector<pair<int,int> > boxes;
  int w, i = 0;
  // maxLength defines the maximum length the program can return, which 
  // is the sum of the largest dimension of each box (i.e. as if we placed
  // them in a single column).
  int maxLength = 0;
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
  sort(boxes.begin(),boxes.end(),compareBoxes);
  BoxWrapping* m = new BoxWrapping(w,maxLength,boxes);
  BAB<BoxWrapping> e(m);
  delete m;
  BoxWrapping* best;
  BoxWrapping* s;
  while(s = e.next()) {
    if(s) {
      //s->print();
      best = s;
    }
  }    
  if(best) {
    best->print();
  }
  delete s;
  delete best;
  return 0;
}
