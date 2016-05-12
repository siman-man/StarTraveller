#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace std;

struct Star {
  int y;
  int x;

  Star (int y, int x) {
    this->y = y;
    this->x = x;
  }
};

vector<Star> g_startList;

int g_turn;

class StarTraveller {
  public:
    int NStars;
    vector<int> used;

    int init(vector<int> stars) {
      NStars = stars.size()/2;
      used.resize(NStars, 0);
      g_turn = 0;

      for (int i = 0; i < NStars; i++) {
        int y = stars[i*2];
        int x = stars[i*2+1];

        g_startList.push_back(Star(y,x));
      }

      return 0;
    }

    vector<int> makeMoves(vector<int> ufos, vector<int> ships) {
      g_turn++;

      vector<int> ret;
      for (int i = 0;i < NStars; i++) {
        if (!used[i]) {
          used[i] = 1;
          ret.push_back(i);
          if (ret.size() == ships.size()) break;
        }
      }

      // Make sure the return is filled with valid moves for the final move.
      while (ret.size() < ships.size()) {
        ret.push_back((ships[ret.size()]+1)%NStars);
      }

      return ret;
    }
};

// -------8<------- end of solution submitted to the website -------8<-------

template<class T> void getVector(vector<T>& v) {
  for (int i = 0; i < v.size(); ++i)
    cin >> v[i];
}

int main() {
  int NStars;
  cin >> NStars;
  vector<int> stars(NStars);
  getVector(stars);

  StarTraveller algo;
  int ignore = algo.init(stars);
  cout << ignore << endl;
  cout.flush();

  while (true) {
    int NUfo;
    cin >> NUfo;
    if (NUfo<0) break;
    vector<int> ufos(NUfo);
    getVector(ufos);
    int NShips;
    cin >> NShips;
    vector<int> ships(NShips);
    getVector(ships);
    vector<int> ret = algo.makeMoves(ufos, ships);
    cout << ret.size() << endl;
    for (int i = 0; i < ret.size(); ++i) {
      cout << ret[i] << endl;
    }
    cout.flush();
  }
}

