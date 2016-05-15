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
  bool flag;
  queue<int> path;

  Ship () {
    this->sid = -1;
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
int g_flagShipId;
int g_noupdate;
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
        g_changeLine = 100;
      } else {
        g_changeLine = 512;
      }
    } 

    void directFlagShip() {
      double minDist = DBL_MAX;
      int targetId = -1;
      int index = -1;

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);

        for (int j = 0; j < g_psize; j++) {
          int target = g_path[j];
          Star *star = getStar(target);

          if (star->visited) continue;

          double dist = DIST_TABLE[ship->sid][target];

          if (minDist > dist) {
            minDist = dist;
            g_flagShipId = i;
            targetId = target;
            index = j;
          }
        }
      }

      Ship *flagShip = getShip(g_flagShipId);
      flagShip->flag = true;

      int bid = (index == 0)? g_psize-1 : index-1;
      int aid = (index+1)%g_psize;

      int d1 = DIST_TABLE[g_path[bid]][g_path[index]];
      int d2 = DIST_TABLE[g_path[index]][g_path[aid]];

      if (d1 > d2) {
        for (int i = 0; i < g_psize; i++) {
          int sid = g_path[(index+i)%g_psize];
          flagShip->path.push(sid);
        }
      } else {
        for (int i = 0; i < g_psize; i++) {
          int j = index-i;
          int sid;

          if (j < 0) {
            sid = g_path[g_psize+j];
          } else {
            sid = g_path[j];
          }
          flagShip->path.push(sid);
        }
      }
    }

    void changeFlagShip(int id) {
      fprintf(stderr,"changeFlagShip %d -> %d\n", g_flagShipId, id);
      Ship *flagShip = getShip(g_flagShipId);
      flagShip->flag = false;

      g_flagShipId = id;
      Ship *newFlagShip = getShip(id);
      newFlagShip->flag = true;
      newFlagShip->path = flagShip->path;
    }

    vector<int> makeMoves(vector<int> ufos, vector<int> ships) {
      g_turn++;
      g_timeLimit--;

      g_ufoCount = ufos.size() / 3;
      g_shipCount = ships.size();
      int ssize = ships.size();

      if (g_turn > 1) {
        if (checkVisited(ships)) {
          g_noupdate = 0;
        } else {
          g_noupdate++;
        }
      }

      setParameter();

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

        //fprintf(stderr,"UFO %d: hitRate = %f\n", i, ufo->hitRate());
      }

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

        vector<int> firstPath = createFirstPath(path);
        g_path = TSPSolver(firstPath);
        directFlagShip();
      }

      for (int i = 0; i < ssize; i++) {
        Ship *ship = getShip(i);

        if (g_turn == 1) {
          ship->sid = ships[i];
        } else if (g_flag) {
          if (g_shipCount <= 1) {
            moveShipSingle();
          } else {
            moveShip();
          }
          break;
        } else {
          moveShipWithUFO(ufos);
          break;
        }
      }

      vector<int> ret = getOutput();
      return ret;
    }

    vector<int> createFirstPath(vector<int> &path) {
      map<int, bool> checkList;
      vector<int> ret;
      int psize = path.size();
      int cid = path[0];
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

        if (!star->visited) {
          update = true;
          g_remainCount--;
        }

        star->visited = true;
      }

      return update;
    }

    void moveShipSingle() {
      Ship *flagShip = getShip(g_flagShipId);
      int nid = flagShip->path.front(); flagShip->path.pop();
      Star *star = getStar(nid);

      if (!star->visited) {
        flagShip->sid = nid;
      }
    }

    void moveShip() {
      Ship *flagShip = getShip(g_flagShipId);
      int nid = flagShip->path.front(); flagShip->path.pop();
      double minDist = DBL_MAX;
      int fid = -1;
      Star *star = getStar(nid);

      if (star->visited) return;

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);

        double dist = DIST_TABLE[nid][ship->sid];

        if (minDist > dist) {
          minDist = dist;
          fid = i;
        }
      }

      if (fid == g_flagShipId) {
        flagShip->sid = nid;
      } else {
        changeFlagShip(fid);
        Ship *newFlagShip = getShip(g_flagShipId);
        newFlagShip->sid = nid;
      }
    }

    void moveShipWithUFO(vector<int> &ufos) {
      map<int, double> distList;

      for (int j = 0; j < g_ufoCount; j++) {
        UFO *ufo = getUFO(j);

        if (ufo->capacity <= ufo->crew) continue; 

        double minDist = DBL_MAX;
        int uid = -1;
        int nid = -1;
        int shipId = -1;

        for (int i = 0; i < g_shipCount; i++) {
          Ship *ship = getShip(i);

          if (ship->uid >= 0) continue;

          double dist = DIST_TABLE[ufo->nid][ship->sid];

          if (minDist > dist) {
            minDist = dist;
            nid = ufo->nid;
            uid = j;
            shipId = i;
          }
        }

        Ship *ship = getShip(shipId);

        if (minDist <= g_changeLine) {
          ship->sid = nid;
          ship->uid = uid;
          g_ufoList[uid].crew++;
        }
      }

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);

        if (ship->uid >= 0) {
          UFO *ufo = getUFO(ship->uid);
          Star *nstar = getStar(ufo->nid);

          if (ship->sid == ufo->sid && ufo->sid == ufo->nnid && nstar->visited) {
            ship->sid = ufo->sid;
          } else {
            ship->sid = ufo->nid;
          }
        }
      }
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

        type = xor128()%3;

        switch(type) {
          case 0:
            reconnectPath(c1, c2);
            break;
          case 1:
            swapStar(c1, c2);
            break;
          case 2:
            insertStar(c1, c2);
            break;
          default:
            reconnectPath(c1, c2);
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
        } else if (xor128()%100 < 100 * exp(scoreDiff/(k*T))) {
          goodScore = newScore;
        } else {
          switch (type) {
            case 0:
              reconnectPath(c1, c2);
              break;
            case 1:
              swapStar(c1, c2);
              break;
            case 2:
              g_path = bestPath;
              break;
            default:
              reconnectPath(c1, c2);
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
      //double pathDist = sqrt(calcPathDist());
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

    void reconnectPath(int c1, int c3) {
      int i = c1;
      int j = c3;

      if (c1 > c3) {
        i = c3;
        j = c1;
      }

      while(i < j) {
        int temp = g_path[i];
        g_path[i] = g_path[j];
        g_path[j] = temp;

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

