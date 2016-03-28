#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include<fstream>
#include<map>
#include<set>

using namespace Gecode;
using namespace std;

class BoxWrapping : public Space {

protected:

  IntVar length;
  IntVarArray x_tl;
  IntVarArray x_br;
  IntVarArray y_tl;
  IntVarArray y_br;

public:

  BoxWrapping(int w, map<int,pair<int,int> >& boxes) : 
    length(*this,0,10000),
    x_tl(*this,boxes.size(),0,w-1), 
    x_br(*this,boxes.size(),0,w-1),
    y_tl(*this,boxes.size(),0,10000),
    y_br(*this,boxes.size(),0,10000)
  {
    for(int i = 0; i < boxes.size(); i++){
      int width = boxes[i].first;
      int length = boxes[i].second;      
      rel(*this,x_tl[i] <= x_br[i]);
      rel(*this,y_tl[i] <= y_br[i]);
      rel(*this,(x_br[i]-x_tl[i]) == (width-1));
      rel(*this,(y_br[i]-y_tl[i]) == (length-1));
      for(int j = 0; j < boxes.size(); j++) {
	if(i != j) {
	  rel(*this,!(x_tl[j] >= x_tl[i] && x_tl[j] <= x_br[i]) || (y_br[j] < y_tl[i]) || (y_tl[j] > y_br[i]));
	  rel(*this,!(x_br[j] >= x_tl[i] && x_br[j] <= x_br[i]) || (y_br[j] < y_tl[i]) || (y_tl[j] > y_br[i]));
	  rel(*this,!(y_tl[j] >= y_tl[i] && y_tl[j] <= y_br[i]) || (x_br[j] < x_tl[i]) || (x_tl[j] > x_br[i]));
	  rel(*this,!(y_br[j] >= y_tl[i] && y_br[j] <= y_br[i]) || (x_br[j] < x_tl[i]) || (x_tl[j] > x_br[i]));
	}
      }
    }
    rel(*this, max(y_br)+1 == length);
    branch(*this, x_tl, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    branch(*this, x_br, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    branch(*this, y_tl, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    branch(*this, y_br, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
  }

  BoxWrapping(bool share, BoxWrapping& s) : Space(share, s) {
    length.update(*this,share,s.length);
    x_tl.update(*this,share,s.x_tl);
    x_br.update(*this,share,s.x_br);
    y_tl.update(*this,share,s.y_tl);
    y_br.update(*this,share,s.y_br);
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

  // constrain function
  virtual void constrain(const Space& _b) {
    const BoxWrapping& b = static_cast<const BoxWrapping&>(_b);
    // The best solution is the one which has the least number of colors.
    // This is already constrained by INT_VAL_MIN.
    rel(*this,length < b.length);
  }
};

int main(int argc, char* argv[]) {
  // Read input and save it. 
  ifstream infile;
  map<int,pair<int,int> > boxes;
  int w, i = 0;
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
	int length = atoi(tokens[2].c_str());
	int nBoxes = atoi(tokens[0].c_str());
	for(int j = i; j < nBoxes+i; j++) {
	  boxes[j] = make_pair(width,length);
	}
	i += nBoxes;
      }
    }
  }
  infile.close();
  BoxWrapping* m = new BoxWrapping(w,boxes);
  BAB<BoxWrapping> e(m);
  delete m;
  BoxWrapping* best;
  BoxWrapping* s;
  while(s = e.next()) {
    if(s)
      best = s;
  }    
  if(best) {
    best->print();
  }
  delete s;
  delete best;
  return 0;
}
