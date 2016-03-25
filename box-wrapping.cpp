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


public:

  BoxWrapping(int w, const map<pair<int,int>,int>& boxes) {
    //branch(*this, l, INT_VAR_NONE(), INT_VAL_MIN());
  }

  BoxWrapping(bool share, BoxWrapping& s) : Space(share, s) {
    //k.update(*this, share, s.k);
  }

  virtual Space* copy(bool share) {
    return new BoxWrapping(share,*this);
  }

  void print(void) const {
  }

  // constrain function
  /*virtual void constrain(const Space& _b) {
    const BoxWrapping& b = static_cast<const BoxWrapping&>(_b);
    // The best solution is the one which has the least number of colors.
    // This is already constrained by INT_VAL_MIN.
    rel(*this,k < b.k);
    }*/
};

int main(int argc, char* argv[]) {
  // Read input and save it. 
  ifstream infile;
  map<pair<int,int>,int> boxes;
  int w;
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
	boxes[make_pair(width,length)] = nBoxes;
      }
    }
  }
  infile.close();
  BoxWrapping* m = new BoxWrapping(w,boxes);
  BAB<BoxWrapping> e(m);
  delete m;
  if(BoxWrapping* s = e.next()) {
    s->print(); delete s;
    cout << "------" << endl;
  }
  return 0;
}
