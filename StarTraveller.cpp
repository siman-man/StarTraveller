#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <map>
#include <cassert>
#include <limits.h>
#include <sstream>
#include <vector>

using namespace std;

const int SPACE_SIZE = 1024;
const int MAX_STAR = 2000;
const int MAX_GALAXY = 16;

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

inline double calcDist(int y1, int x1, int y2, int x2) {
  return sqrt((y2-y1) * (y2-y1) + (x2-x1) * (x2-x1));
}

inline int calcDistFast(int y1, int x1, int y2, int x2) {
  return (y2-y1) * (y2-y1) + (x2-x1) * (x2-x1);
}

struct Star {
  int y;
  int x;
  int gid;
  bool visited;

  Star (int y = -1, int x = -1) {
    this->y = y;
    this->x = x;
    this->gid = -1;
    this->visited = false;
  }
};

struct Galaxy {
  int y;
  int x;
  int gsize;
  vector<int> stars;

  Galaxy (int y = -1, int x= -1) {
    this->y = y;
    this->x = x;
    this->gsize = 0;
  }
};

Star g_starList[MAX_STAR];
Galaxy g_galaxyList[MAX_GALAXY];

int g_turn;
int g_galaxyCount;
int g_starCount;

class StarTraveller {
  public:
    vector<int> used;

    int init(vector<int> stars) {
      g_starCount = stars.size()/2;
      used.resize(g_starCount, 0);
      g_turn = 0;
      int maxH = 0;
      int minH = 1024;
      int maxW = 0;
      int minW = 1024;

      for (int i = 0; i < g_starCount; i++) {
        int x = stars[i*2];
        int y = stars[i*2+1];

        maxH = max(maxH, y);
        minH = min(minH, y);
        maxW = max(maxW, x);
        minW = min(minW, x);

        g_starList[i] = Star(y,x);
      }

      fprintf(stderr,"maxW = %d, minW = %d, maxH = %d, minH = %d\n", maxW, minW, maxH, minH);

      kmeans();

      return 0;
    }

    vector<int> makeMoves(vector<int> ufos, vector<int> ships) {
      g_turn++;

      vector<int> ret;
      for (int i = 0;i < g_starCount; i++) {
        if (!used[i]) {
          used[i] = 1;
          ret.push_back(i);
          if (ret.size() == ships.size()) break;
        }
      }

      // Make sure the return is filled with valid moves for the final move.
      while (ret.size() < ships.size()) {
        ret.push_back((ships[ret.size()]+1)%g_starCount);
      }

      return ret;
    }

    void kmeans() {
      for (int i = 1; i <= MAX_GALAXY; i++) {
        g_galaxyCount = i;

        for (int j = 0; j < 5; j++) {
          setRandomGalaxy(i);

          while(updateCluster()) {
            changeGalaxyPoint();
          }

          commitGalaxy();

          if (checkCluster()) {
            return;
          }
        }
      }
    }

    void commitGalaxy() {
      for (int i = 0; i < g_galaxyCount; i++) {
        g_galaxyList[i].stars.clear();
      }

      for (int i = 0; i < g_starCount; i++) {
        Star *star = getStar(i);

        g_galaxyList[star->gid].stars.push_back(i);
      }
    }

    bool checkCluster() {
      //fprintf(stderr,"checkCluster =>\n");

      for (int i = 0; i < g_galaxyCount; i++) {
        Galaxy *galaxy = getGalaxy(i);

        int gsize = galaxy->stars.size();
        for (int j = 0; j < gsize; j++) {
          int sid = galaxy->stars[j];
          Star *star = getStar(sid);

          int dist = calcDistFast(star->y, star->x, galaxy->y, galaxy->x);
          if (dist > 90000) {
            return false;
          }
        }
      }

      return true;
    }

    void setRandomGalaxy(int n) {
      //fprintf(stderr,"%d: setRandomGalaxy =>\n", n);
      for (int i = 0; i < g_galaxyCount; i++) {
        int gy = xor128()%SPACE_SIZE;
        int gx = xor128()%SPACE_SIZE;

        g_galaxyList[i].y = gy;
        g_galaxyList[i].x = gx;
        g_galaxyList[i].gsize = 0;
      }
    }

    bool updateCluster() {
      bool update = false;
      //fprintf(stderr,"%d: updateCluster =>\n", g_galaxyCount);

      for (int i = 0; i < g_galaxyCount; i++) {
        g_galaxyList[i].gsize = 0;
      }

      for (int i = 0; i < g_starCount; ++i) {
        Star *star = getStar(i);

        int minDist = INT_MAX;
        int gid = -1;

        for (int j = 0; j < g_galaxyCount; j++) {
          Galaxy *galaxy = getGalaxy(j);
          int dist = calcDistFast(star->y, star->x, galaxy->y, galaxy->x);

          if (minDist > dist) {
            minDist = dist;
            gid = j;
          }
        }

        if (star->gid != gid) {
          //fprintf(stderr,"star %d: %d -> %d\n", i, star->gid, gid);
          update = true;
          star->gid = gid;
        }

        g_galaxyList[gid].gsize++;
      }

      return update;
    }

    void changeGalaxyPoint() {
      //fprintf(stderr,"changeGalaxyPoint =>\n");
      int CY[MAX_GALAXY];
      int CX[MAX_GALAXY];

      memset(CY, 0, sizeof(CY));
      memset(CX, 0, sizeof(CX));

      for (int i = 0; i < g_starCount; i++) {
        Star *star = getStar(i);
        if (star->gid < 0 || star->gid >= g_galaxyCount) {
          fprintf(stderr,"star->gid = %d\n", star->gid);
          assert(star->gid >= 0);
        }

        CY[star->gid] += star->y;
        CX[star->gid] += star->x;
      }


      for (int i = 0; i < g_galaxyCount; i++) {
        Galaxy *galaxy = getGalaxy(i);
        int gsize = galaxy->gsize;

        //fprintf(stderr,"%d: gsize = %d\n", i, gsize);
        if (gsize == 0) continue;

        galaxy->y = CY[i] / gsize;
        galaxy->x = CX[i] / gsize;
      }
    }

    Galaxy *getGalaxy(int id) {
      return &g_galaxyList[id];
    }

    Star *getStar(int id) {
      return &g_starList[id];
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

