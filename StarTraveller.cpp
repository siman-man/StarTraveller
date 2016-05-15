#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <map>
#include <sys/time.h>
#include <cassert>
#include <float.h>
#include <string.h>
#include <limits.h>
#include <sstream>
#include <vector>

using namespace std;

typedef long long ll;

const int SPACE_SIZE = 1024;
const int MAX_STAR = 2000;
const int MAX_GALAXY = 16;
const int MAX_SHIP = 10;
const int MAX_UFO = 20;
const ll CYCLE_PER_SEC = 2400000000;
double TIME_LIMIT = 15.0;

double DIST_TABLE[MAX_STAR][MAX_STAR];

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

unsigned long long int getCycle() {
  unsigned int low, high;
  __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
  return ((unsigned long long int)low) | ((unsigned long long int)high << 32);
}

double getTime(unsigned long long int begin_cycle) {
  return (double)(getCycle() - begin_cycle) / CYCLE_PER_SEC;
}

inline double calcDist(int y1, int x1, int y2, int x2) {
  return sqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1));
}

inline int calcDistFast(int y1, int x1, int y2, int x2) {
  return ((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1));
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

struct Ship {
  int sid;
  int uid;

  Ship () {
    this->sid = -1;
    this->uid = -1;
  }
};

struct UFO {
  int capacity;
  int crew;
  int cid;
  int sid;
  int nid;
  int hitCount;
  int totalCount;
  double totalMoveDist;

  UFO () {
    this->crew = 0;
    this->capacity = 1;
    this->hitCount = 0;
    this->totalCount = 0;
    this->totalMoveDist = 0.0;
  }

  double hitRate() {
    return 100.0 * (hitCount / (double)totalCount);
  }

  double averageMoveDist() {
    return totalMoveDist / totalCount;
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

vector<Star> g_starList;
Ship g_shipList[MAX_SHIP];
UFO g_ufoList[MAX_UFO];
Galaxy g_galaxyList[MAX_GALAXY];
vector<int> g_path;
int g_psize;

int g_turn;
int g_index;
int g_galaxyCount;
int g_starCount;
int g_shipCount;
int g_ufoCount;
int g_timeLimit;
int g_remainCount;
int g_changeLine;
bool g_flag;
bool g_checkFlag;

class StarTraveller {
  public:
    vector<int> used;

    int init(vector<int> stars) {
      g_starCount = stars.size()/2;
      used.resize(g_starCount, 0);
      g_turn = 0;
      g_index = 0;
      g_timeLimit = g_starCount * 4;
      g_remainCount = g_starCount;
      g_checkFlag = false;
      g_flag = false;
      vector<int> path;

      for (int i = 0; i < g_starCount; i++) {
        int x = stars[i*2];
        int y = stars[i*2+1];

        g_starList.push_back(Star(y,x));
        path.push_back(i);
      }

      setupDistTable();

      //g_path = TSPSolver(path);
      kmeans();

      return 0;
    }

    void setupDistTable() {
      for (int i = 0; i < g_starCount-1; i++) {
        Star *from = getStar(i);

        for (int j = i+1; j < g_starCount; j++) {
          Star *to = getStar(j);
          double dist = calcDist(from->y, from->x, to->y, to->x);

          DIST_TABLE[i][j] = dist;
          DIST_TABLE[j][i] = dist;
        }
      }
    }

    void setParameter() {
      if (g_turn <= g_starCount) {
        g_changeLine = 100;
      } else {
        g_changeLine = 800;
      }
    } 

    vector<int> makeMoves(vector<int> ufos, vector<int> ships) {
      g_turn++;
      g_timeLimit--;

      g_ufoCount = ufos.size() / 3;
      g_shipCount = ships.size();
      int ssize = ships.size();

      if (g_turn > 1) {
        checkVisited(ships);
      }

      setParameter();

      for (int i = 0; i < g_ufoCount; i++) {
        UFO *ufo = getUFO(i);

        ufo->cid = ufos[i*3];
        ufo->nid = ufos[i*3+1];
        Star *star = getStar(ufo->cid);

        double dist = DIST_TABLE[ufo->cid][ufo->nid];

        if (!star->visited) {
          ufo->hitCount++;
        }

        ufo->totalMoveDist += dist;
        ufo->totalCount++;

        //fprintf(stderr,"UFO %d: hitRate = %f\n", i, ufo->hitRate());
      }

      if (!g_flag && g_remainCount >= g_timeLimit) {
        g_flag = true;
        fprintf(stderr,"remain count = %d\n", g_remainCount);
      }

      for (int i = 0; i < g_ufoCount; i++) {
        UFO *ufo = getUFO(i);
        ufo->sid = ufos[i*3];
        ufo->nid = ufos[i*3+1];
      }

      if (g_flag && !g_checkFlag) {
        vector<int> path;
        g_checkFlag = true;

        for (int i = 0; i < g_starCount; i++) {
          Star *star = getStar(i);

          if (!star->visited) {
            path.push_back(i);
          }
        }

        g_path = TSPSolver(path);
      }

      for (int i = 0; i < ssize; i++) {
        Ship *ship = getShip(i);

        if (g_turn == 1) {
          ship->sid = ships[i];
        } else if (g_flag) {
          moveShip(ships);
          break;
        } else {
          moveShipWithUFO(ufos, ships);
          break;
        }
      }

      vector<int> ret = getOutput();
      return ret;
    }

    void checkVisited(vector<int> &ships) {
      for (int i = 0; i < g_shipCount; i++) {
        Star *star = getStar(ships[i]);

        if (!star->visited) {
          g_remainCount--;
        }

        star->visited = true;
      }
    }

    void moveShip(vector<int> &ships) {
      int target = g_path[g_index];
      g_index++;
      double minDist = DBL_MAX;
      int moveId = -1;

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);
        
        double dist = DIST_TABLE[ship->sid][target];

        if (minDist > dist) {
          minDist = dist;
          moveId = i;
        }
      }

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);

        if (i == moveId) {
          ship->sid = target;
        } else {
          ship->sid = ships[i];
        }
      }
    }

    void moveShipWithUFO(vector<int> &ufos, vector<int> &ships) {
      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);

        if (ship->uid >= 0) {
          UFO *ufo = getUFO(ship->uid);

          ship->sid = ufos[ship->uid*3+1];
        } else {

          double minDist = DBL_MAX;
          int uid = -1;
          int nid = -1;

          for (int j = 0; j < g_ufoCount; j++) {
            UFO *ufo = getUFO(j);

            if (ufo->capacity <= ufo->crew) continue; 

            Star *from = getStar(ufo->nid);
            Star *to = getStar(ship->sid);

            double dist = calcDist(from->y, from->x, to->y, to->x);

            if (minDist > dist) {
              minDist = dist;
              nid = ufo->nid;
              uid = j;
            }
          }

          if (minDist <= g_changeLine) {
            ship->sid = nid;
            ship->uid = uid;
            g_ufoList[uid].crew++;
          } else {
            ship->sid = ships[i];
          }
        }
      }
    }

    vector<int> TSPSolver(vector<int> &stars) {
      g_path = stars;
      g_psize = g_path.size();
      vector<int> bestPath = g_path;
      int c1, c2;

      if (g_psize <= 2) {
        return bestPath;
      }

      double minScore = calcPathDist();
      
      ll startCycle = getCycle();
      double currentTime;
      ll tryCount = 0;

      double T = 10000.0;
      double k = 10.0;
      double alpha = 0.999;
      int type;

      while(1) {
        tryCount++;
        do {
          c1 = xor128() % g_psize;
          c2 = xor128() % g_psize;
        } while (c1 == c2);

        double baseScore = calcPathDist();
        //double baseScore = calcSubDist(c1, c2);

        type = xor128()%2;

        if (type == 0) {
          reconnectPath(c1, c2);
        } else {
          swapStar(c1, c2);
        }
        double newScore = calcPathDist();
        //double newScore = calcSubDist(c1, c2);
        double scoreDiff = baseScore - newScore;

        if (scoreDiff > 0.0) {
          bestPath = g_path;

          double dist = calcPathDist();
          /*
          fprintf(stderr,"swap %d <-> %d\n", g_path[c1], g_path[c2]);
          fprintf(stderr,"score = %f, %f -> %f\n", score, minScore, dist);
          */
          //assert(minScore > dist);
          minScore = dist;
        } else if (T > 0.0 && xor128()%100 < 100 * exp(scoreDiff/(k*T))) {
        } else {
          //g_path = bestPath;

          if (type == 0) {
            //g_path = bestPath;
            reconnectPath(c1, c2);
          } else {
            swapStar(c1, c2);
          }
        }

        currentTime = getTime(startCycle);

        if (currentTime > TIME_LIMIT) {
          break;
        }

        T *= alpha;
      }

      fprintf(stderr,"tryCount = %lld\n", tryCount);
      double pathDist = calcPathDist();
      //double pathDist = sqrt(calcPathDist());
      fprintf(stderr,"path size = %d, pathDist = %f\n", g_psize, pathDist);

      return bestPath;
    }

    void showPath() {
      for (int i = 0; i < g_psize-1; i++) {
        fprintf(stderr," %d ->", g_path[i]);
      }
      fprintf(stderr," %d\n", g_path[g_psize-1]);
    }

    void swapStar(int c1, int c2) {
      int temp = g_path[c1];
      g_path[c1] = g_path[c2];
      g_path[c2] = temp;
    }

    void reconnectPath(int c1, int c3) {
      int c2 = (c1+1)%g_psize;
      int i = c2;
      int j = c3;

      while(i < j) {
        int temp = g_path[i];
        g_path[i] = g_path[j];
        g_path[j] = temp;

        i++;
        j--;
      }
    }

    double calcSubDist(int c1, int c2) {
      int s1b = (c1 == 0)? g_path[g_psize-1] : g_path[c1-1];
      int s1m = g_path[c1];
      int s1a = g_path[(c1+1)%g_psize];

      int s2b = (c2 == 0)? g_path[g_psize-1] : g_path[c2-1];
      int s2m = g_path[c2];
      int s2a = g_path[(c2+1)%g_psize];

      Star *star1 = getStar(s1b);
      Star *star2 = getStar(s1m);
      Star *star3 = getStar(s1a);

      Star *star4 = getStar(s2b);
      Star *star5 = getStar(s2m);
      Star *star6 = getStar(s2a);

      double d1 = calcDist(star1->y, star1->x, star2->y, star2->x);
      double d2 = calcDist(star2->y, star2->x, star3->y, star3->x);
      double d3 = calcDist(star4->y, star4->x, star5->y, star5->x);
      double d4 = calcDist(star5->y, star5->x, star6->y, star6->x);

      return d1 + d2 + d3 + d4;
    }

    ll calcPathDist() {
      int psize = g_path.size();
      ll totalDist = 0;

      for (int i = 0; i < psize; i++) {
        int s1 = g_path[i];
        int s2 = g_path[(i+1)%psize];

        double dist = DIST_TABLE[s1][s2];

        totalDist += dist;
      }

      return totalDist;
    }

    void kmeans() {
      for (int i = 1; i <= MAX_GALAXY; i++) {
        g_galaxyCount = i;

        for (int j = 0; j < 5; j++) {
          setRandomGalaxy();

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

    void setRandomGalaxy() {
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

    vector<int> getOutput() {
      vector<int> ret;

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);
        ret.push_back(ship->sid);
      }

      return ret;
    }

    Galaxy *getGalaxy(int id) {
      return &g_galaxyList[id];
    }

    Star *getStar(int id) {
      return &g_starList[id];
    }

    Ship *getShip(int id) {
      return &g_shipList[id];
    }

    UFO *getUFO(int id) {
      return &g_ufoList[id];
    }
};

// -------8<------- end of solution submitted to the website -------8<-------
template<class T> void getVector(vector<T>& v) { for (int i = 0; i < v.size(); ++i) cin >> v[i];}
int main() {
  TIME_LIMIT = 5.0;
  int NStars; cin >> NStars; vector<int> stars(NStars);
  getVector(stars);
  StarTraveller algo;
  int ignore = algo.init(stars);
  cout << ignore << endl; cout.flush();
  while (true) { int NUfo;
    cin >> NUfo; if (NUfo<0) break; vector<int> ufos(NUfo);
    getVector(ufos); int NShips; cin >> NShips;
    vector<int> ships(NShips); getVector(ships);
    vector<int> ret = algo.makeMoves(ufos, ships);
    cout << ret.size() << endl;
    for (int i = 0; i < ret.size(); ++i) { cout << ret[i] << endl; }
    cout.flush();
  }
}

