#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <map>
#include <sys/time.h>
#include <cassert>
#include <float.h>
#include <string.h>
#include <queue>
#include <limits.h>
#include <sstream>
#include <vector>

using namespace std;

typedef long long ll;

const int SPACE_SIZE = 1024;
const int MAX_STAR = 2000;
const int MAX_SHIP = 10;
const int MAX_UFO = 20;
const ll CYCLE_PER_SEC = 2400000000;
double TIME_LIMIT = 15.0;
double FIRST_TIME_LIMIT = 3.0;

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

struct Star {
  int y;
  int x;
  bool visited;

  Star (int y = -1, int x = -1) {
    this->y = y;
    this->x = x;
    this->visited = false;
  }
};

struct Ship {
  int sid;
  int nid;
  int uid;
  bool flag;
  vector<int> path;

  Ship () {
    this->sid = -1;
    this->nid = -1;
    this->uid = -1;
    this->flag = false;
  }
};

struct UFO {
  int capacity;
  int crew;
  int sid;
  int nid;
  int nnid;
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

vector<Star> g_starList;
Ship g_shipList[MAX_SHIP];
UFO g_ufoList[MAX_UFO];
vector<int> g_path;
int g_psize;

int g_turn;
int g_index;
int g_starCount;
int g_shipCount;
int g_ufoCount;
int g_timeLimit;
int g_remainCount;
int g_changeLine;
int g_flagShipId;
int g_noupdate;
double g_currentCost;
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
      g_currentCost = 0.0;
      g_remainCount = g_starCount;
      g_checkFlag = false;
      g_flag = false;
      g_noupdate = 0;
      vector<int> path;

      for (int i = 0; i < g_starCount; i++) {
        int x = stars[i*2];
        int y = stars[i*2+1];

        g_starList.push_back(Star(y,x));
        path.push_back(i);
      }

      setupDistTable();

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
        g_changeLine = 64;
      } else {
        g_changeLine = 256;
      }
    } 

    vector<int> makeMoves(vector<int> ufos, vector<int> ships) {
      g_turn++;
      g_timeLimit--;

      g_ufoCount = ufos.size() / 3;
      g_shipCount = ships.size();

      if (g_turn > 1) {
        if (checkVisited(ships)) {
          g_noupdate = 0;
        } else {
          g_noupdate++;
        }
      }

      setParameter();
      updateUFOInfo(ufos);
      updateShipInfo(ships);

      if (!g_flag && g_remainCount > g_timeLimit) {
        g_flag = true;
        fprintf(stderr,"remain count = %d\n", g_remainCount);
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

        g_psize = path.size();
        ll startCycle = getCycle();
        double minDist = DBL_MAX;
        vector<int> firstPath;

        for (int i = 0; i < g_psize; i++) {
          g_path = createFirstPath(path, i);
          double dist = calcPathDist();

          if (minDist > dist) {
            minDist = dist;
            firstPath = g_path;
          }

          double currentTime = getTime(startCycle);

          if (currentTime > FIRST_TIME_LIMIT) {
            break;
          }
        }

        g_path = TSPSolver(firstPath);
        g_shipList[0].path = g_path;
      }

      if (g_turn == 1) {
          moveShipFirst(ships);
      } else if (g_flag) {
          if (g_shipCount <= 1) {
            moveShipSingle();
          } else {
            moveShip();
          }
      } else {
        moveShipWithUFO();
      }

      vector<int> ret = getOutput();
      //fprintf(stderr,"current cost = %f\n", g_currentCost);

      return ret;
    }

    vector<int> createFirstPath(vector<int> &path, int index) {
      map<int, bool> checkList;
      vector<int> ret;
      int psize = path.size();
      int cid = path[index];
      int nid;
      ret.push_back(cid);

      for (int i = 0; i < psize-1; i++) {
        double minDist = DBL_MAX;
        checkList[cid] = true;

        for (int j = 0; j < psize; j++) {
          int id = path[j];

          if (checkList[id]) continue;

          double dist = DIST_TABLE[cid][id];

          if (minDist > dist) {
            minDist = dist;
            nid = id;
          }
        }

        ret.push_back(nid);
        cid = nid;
      }

      return ret;
    }

    bool checkVisited(vector<int> &ships) {
      bool update = false;

      for (int i = 0; i < g_shipCount; i++) {
        Star *star = getStar(ships[i]);
        if (star->visited) continue;
        star->visited = true;

        update = true;
        g_remainCount--;
      }

      return update;
    }

    void updateUFOInfo(vector<int> &ufos) {
      for (int i = 0; i < g_ufoCount; i++) {
        UFO *ufo = getUFO(i);

        ufo->sid = ufos[i*3];
        ufo->nid = ufos[i*3+1];
        ufo->nnid = ufos[i*3+2];
        Star *star = getStar(ufo->sid);

        double dist = DIST_TABLE[ufo->sid][ufo->nid];

        if (!star->visited) {
          ufo->hitCount++;
        }

        ufo->totalMoveDist += dist;
        ufo->totalCount++;
      }
    }

    void updateShipInfo(vector<int> &ships) {
      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);
        ship->sid = ships[i];
      }
    }

    void moveShipFirst(vector<int> &ships) {
      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);
        ship->nid = ships[i];
      }
    }

    void moveShipSingle() {
      Ship *flagShip = getShip(g_flagShipId);
      int nid = flagShip->path[0];
      flagShip->path.erase(flagShip->path.begin());
      Star *star = getStar(nid);

      if (!star->visited) {
        flagShip->nid = nid;
      }
    }

    void moveShip() {
      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);
        if (ship->path.size() == 0) continue;

        ship->nid = ship->path[0];
        ship->path.erase(ship->path.begin());
      }
    }

    void moveShipWithUFO() {
      map<int, double> distList;

      for (int j = 0; j < g_ufoCount; j++) {
        UFO *ufo = getUFO(j);
        Star *nstar = getStar(ufo->nid);

        if (ufo->capacity <= ufo->crew) continue; 

        double minDist = DBL_MAX;
        int shipId = -1;

        for (int i = 0; i < g_shipCount; i++) {
          Ship *ship = getShip(i);
          double dist = DIST_TABLE[ufo->nid][ship->sid];

          if (ship->uid >= 0) continue;

          if (minDist > dist) {
            minDist = dist;
            shipId = i;
          }
        }

        Ship *ship = getShip(shipId);

        if (minDist <= g_changeLine) {
          double ndist = DIST_TABLE[ship->sid][ufo->nnid];

          if (!nstar->visited || minDist < ndist) {
            ship->nid = ufo->nid;
            ship->uid = j;
            g_ufoList[j].crew++;
          }
        }
      }

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);

        if (ship->uid >= 0) {
          UFO *ufo = getUFO(ship->uid);
          Star *nstar = getStar(ufo->nid);

          if (ship->sid == ufo->sid && ufo->sid == ufo->nnid && nstar->visited) {
            ship->nid = ufo->sid;
          } else {
            ship->nid = ufo->nid;
          }
        }
      }
    }

    vector<int> MTSPSolver(vector<int> &stars) {
      g_path = stars;
      g_psize = g_path.size();
      vector<int> bestPath = g_path;
      vector<int> goodPath = g_path;
      int c1, c2;
      int s1, s2;

      if (g_psize <= 2 || g_shipCount == 1) {
        return bestPath;
      }

      double bestScore = calcPathDistMulti();
      double goodScore = bestScore;

      ll startCycle = getCycle();
      double currentTime;
      ll tryCount = 0;

      double T = 1000.0;
      double k = 10.0;
      double alpha = 0.999;
      int type;

      while(1) {
        do {
          c1 = xor128() % g_psize;
          c2 = xor128() % g_psize;
        } while (c1 == c2);

        do {
          s1 = xor128() % g_shipCount;
          s2 = xor128() % g_shipCount;
        } while (s1 == s2);

        type = xor128()%3;
        Ship *ship1 = getShip(s1);
        Ship *ship2 = getShip(s2);

        tryCount++;

        switch(type) {
          case 0:
            reconnectPath(c1, c2, ship1->path);
            break;
          case 1:
            swapStar(c1, c2);
            break;
          case 2:
            insertStar(c1, c2);
            break;
        }

        double newScore = calcPathDistMulti();
        double scoreDiff = goodScore - newScore;

        if (bestScore > newScore) {
          bestScore = newScore;
          bestPath = g_path;
        }

        if (goodScore > newScore) {
          goodScore = newScore;
        } else if (xor128()%100 < 100*exp(scoreDiff/(k*T))) {
          goodScore = newScore;
        } else {
          switch (type) {
            case 0:
              reconnectPath(c1, c2, ship1->path);
              break;
            case 1:
              swapStar(c1, c2);
              break;
            case 2:
              g_path = bestPath;
              break;
          }
        }

        if (tryCount % 100 == 0) {
          currentTime = getTime(startCycle);

          if (currentTime > TIME_LIMIT) {
            break;
          }
        }

        T *= alpha;
      }

      fprintf(stderr,"tryCount = %lld\n", tryCount);
      fprintf(stderr,"path size = %d, pathDist = %f\n", g_psize, bestScore);

      return bestPath;
    }


    vector<int> TSPSolver(vector<int> &stars) {
      g_path = stars;
      g_psize = g_path.size();
      vector<int> bestPath = g_path;
      vector<int> goodPath = g_path;
      int c1, c2;

      if (g_psize <= 2) {
        return bestPath;
      }

      double bestScore = calcPathDist();
      double goodScore = bestScore;

      ll startCycle = getCycle();
      double currentTime;
      ll tryCount = 0;

      double T = 1000.0;
      double k = 10.0;
      double alpha = 0.999;
      int type;

      while(1) {
        do {
          c1 = xor128() % g_psize;
          c2 = xor128() % g_psize;
        } while (c1 == c2);

        type = xor128()%4;

        if (type == 3 && (c1 > g_psize-3 || c2 > g_psize-3)) {
          continue;
        }

        tryCount++;

        switch(type) {
          case 0:
            reconnectPath(c1, c2, g_path);
            break;
          case 1:
            swapStar(c1, c2);
            break;
          case 2:
            insertStar(c1, c2);
            break;
          case 3:
            insertStar2(c1, c2);
            break;
        }

        double newScore = calcPathDist();
        double scoreDiff = goodScore - newScore;

        if (bestScore > newScore) {
          bestScore = newScore;
          bestPath = g_path;
        }

        if (goodScore > newScore) {
          goodScore = newScore;
        } else if (xor128()%100 < 100*exp(scoreDiff/(k*T))) {
          goodScore = newScore;
        } else {
          switch (type) {
            case 0:
              reconnectPath(c1, c2, g_path);
              break;
            case 1:
              swapStar(c1, c2);
              break;
            case 2:
              g_path = bestPath;
              break;
            case 3:
              g_path = bestPath;
              break;
          }
        }

        if (tryCount % 100 == 0) {
          currentTime = getTime(startCycle);

          if (currentTime > TIME_LIMIT) {
            break;
          }
        }

        T *= alpha;
      }

      fprintf(stderr,"tryCount = %lld\n", tryCount);
      fprintf(stderr,"path size = %d, pathDist = %f\n", g_psize, bestScore);

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

    void insertStar(int c1, int c2) {
      int temp = g_path[c1];

      g_path.erase(g_path.begin()+c1);
      g_path.insert(g_path.begin()+c2, temp);
    }

    void insertStarMulti(int s1, int s2) {
      Ship *ship1 = getShip(s1);
      Ship *ship2 = getShip(s2);

      int c1 = xor128() % ship1->path.size();
      int c2 = xor128() % ship2->path.size();

      int temp = ship1->path[c1];

      ship1->path.erase(g_path.begin()+c1);
      ship2->path.insert(g_path.begin()+c2, temp);
    }

    void insertStar2(int c1, int c2) {
      int temp = g_path[c1];
      int temp2 = g_path[c1+1];

      g_path.erase(g_path.begin()+c1);
      g_path.erase(g_path.begin()+c1);
      g_path.insert(g_path.begin()+c2, temp);
      g_path.insert(g_path.begin()+c2, temp2);
    }

    void reconnectPath(int c1, int c3, vector<int> &path) {
      int i = c1;
      int j = c3;

      if (c1 > c3) {
        i = c3;
        j = c1;
      }

      while (i < j) {
        int temp = path[i];
        path[i] = path[j];
        path[j] = temp;

        i++;
        j--;
      }
    }

    double calcPathDist() {
      double totalDist = 0;

      for (int i = 0; i < g_psize; i++) {
        int s1 = g_path[i];
        int s2 = g_path[(i+1)%g_psize];
        double dist = DIST_TABLE[s1][s2];

        totalDist += dist;
      }

      return totalDist;
    }

    double calcPathDistMulti() {
      double totalDist = 0.0;

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);

        int size = ship->path.size();
        int sid = ship->sid;

        for (int j = 0; j < size; j++) {
          int nid = ship->path[j];
          double dist = DIST_TABLE[sid][nid];

          totalDist += dist;
          sid = nid;
        }
      }

      return totalDist;
    }

    vector<int> getOutput() {
      vector<int> ret;

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);
        ret.push_back(ship->nid);

        double dist = DIST_TABLE[ship->sid][ship->nid];

        for (int j = 0; j < g_ufoCount; j++) {
          UFO *ufo = getUFO(j);

          if (ufo->sid == ship->sid && ufo->nid == ship->nid) {
            dist *= 0.001;
          }
        }

        ship->sid = ship->nid;
        g_currentCost += dist;
      }

      return ret;
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
  FIRST_TIME_LIMIT = 1.0;
  int NStars; cin >> NStars; vector<int> stars(NStars);
  getVector(stars); StarTraveller algo;
  int ignore = algo.init(stars);
  cout << ignore << endl; cout.flush();
  while (true) { int NUfo;
    cin >> NUfo; if (NUfo<0) break; vector<int> ufos(NUfo);
    getVector(ufos); int NShips; cin >> NShips;
    vector<int> ships(NShips); getVector(ships);
    vector<int> ret = algo.makeMoves(ufos, ships);
    cout << ret.size() << endl;
    for (int i = 0; i < ret.size(); ++i) { cout << ret[i] << endl; }
    cout.flush();}}

