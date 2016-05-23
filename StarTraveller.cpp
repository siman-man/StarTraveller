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
double TIME_SPAN = 1.0;
double FIRST_TIME_LIMIT = 1.0;

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
  vector<int> path;

  Ship () {
    this->sid = -1;
    this->nid = -1;
    this->uid = -1;
  }
};

struct UFO {
  int capacity;
  int crew;
  int sid;
  int nid;
  int nnid;
  int totalCount;
  double totalMoveDist;
  bool rideoff;

  UFO () {
    this->crew = 0;
    this->capacity = 10;
    this->totalCount = 0;
    this->totalMoveDist = 0.0;
    this->rideoff = false;
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
double g_currentCost;
bool g_TSPMode;
bool g_checkFlag;
vector< vector<int> > g_bestPaths(MAX_SHIP);
vector< vector<int> > g_tempBestPaths(MAX_SHIP);

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
      g_TSPMode = false;
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

        DIST_TABLE[i][i] = 0.0;
        for (int j = i+1; j < g_starCount; j++) {
          Star *to = getStar(j);
          double dist = calcDist(from->y, from->x, to->y, to->x);

          DIST_TABLE[i][j] = dist;
          DIST_TABLE[j][i] = dist;
        }
      }
    }

    void setParameter() {
      if (g_shipCount <= 3 && g_ufoCount == 1) {
        g_changeLine = 756;
      } else if (g_turn <= g_starCount) {
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
        checkVisited(ships);
      }

      setParameter();
      updateUFOInfo(ufos);
      updateShipInfo(ships);

      if (!g_TSPMode && g_remainCount > g_timeLimit) {
        g_TSPMode = true;
        fprintf(stderr,"remain count = %d\n", g_remainCount);
      }

      if (g_TSPMode && !g_checkFlag) {
        vector<int> path = getUnvisitedStarList();
        g_checkFlag = true;

        g_path = path;
        g_psize = path.size();
        vector<int> firstPath = nearestNeighbor(path);
        vector<int> secondPath = selectBestFI(path);

        if (g_shipCount == 1) {
          double minScore = DBL_MAX;
          vector<int> bestPath, pathA;

          for (int i = 0; i < 6; i++) {
            if (i % 2 == 0) {
              pathA = TSPSolver(firstPath, TIME_LIMIT/6);
            } else {
              pathA = TSPSolver(secondPath, TIME_LIMIT/6);
            }
            double score = calcPathDist();

            if (minScore > score) {
              minScore = score;
              bestPath = pathA;
            }
          }

          g_shipList[0].path = bestPath;
          cleanPathSingle(0);
        } else {
          double minScore = DBL_MAX;
          vector<int> bp;
          vector<int> pathA = TSPSolver(firstPath, 2.5);
          vector<int> pathB = TSPSolver(secondPath, 2.5);

          for (int i = 0; i < 10; i++) {
            if (i % 2 == 0) {
              MTSPSolver(pathA, TIME_SPAN);
            } else {
              MTSPSolver(pathB, TIME_SPAN);
            }
            double score = calcPathDistMulti();

            if (minScore > score) {
              minScore = score;
              g_tempBestPaths = g_bestPaths;
            }
          }

          for (int i = 0; i < g_shipCount; i++) {
            g_shipList[i].path = g_tempBestPaths[i];
          }
        }
      }

      if (g_turn == 1) {
        moveShipFirst(ships);
      } else if (g_TSPMode) {
        moveShip();
      } else {
        moveShipWithUFO();
      }

      vector<int> ret = getOutput();

      return ret;
    }

    vector<int> getUnvisitedStarList() {
      vector<int> path;

      for (int i = 0; i < g_starCount; i++) {
        Star *star = getStar(i);

        if (!star->visited) {
          path.push_back(i);
        }
      }

      return path;
    }

    vector<int> nearestNeighbor(vector<int> &path) {
      double minDist = DBL_MAX;
      ll startCycle = getCycle();
      vector<int> result;

      for (int i = 0; i < g_psize; i++) {
        g_path = createFirstPath(path, i);
        double dist = calcPathDist();

        if (minDist > dist) {
          minDist = dist;
          result = g_path;
        }

        double currentTime = getTime(startCycle);
        if (currentTime > FIRST_TIME_LIMIT) {
          break;
        }
      }

      return result;
    }

    vector<int> selectBestFI(vector<int> &path) {
      g_psize = path.size();
      double minScore = DBL_MAX;
      ll startCycle = getCycle();
      vector<int> bestPath;

      for (int i = 0; i < g_psize; i++) {
        g_path = farthestInsertion(path, i);
        double score = calcPathDist();

        if (minScore > score) {
          bestPath = g_path;
          minScore = score;
        }

        double currentTime = getTime(startCycle);
        if (currentTime > FIRST_TIME_LIMIT) {
          break;
        }
      }

      return bestPath;
    }

    vector<int> farthestInsertion(vector<int> path, int index) {
      vector<int> result;
      result.push_back(path[index]);
      path.erase(path.begin()+index);

      int psize = path.size();
      map<int, bool> checkList;

      for (int i = 0; i < psize; i++) {
        int size = psize-i;
        int rsize = result.size();

        double md = -1.0;
        int index = -1;

        for (int k = 0; k < size; k++) {
          int m = path[k];

          double mmd = DBL_MAX;

          for (int j = 0; j < rsize; j++) {
            double dist = DIST_TABLE[m][result[j]];
            mmd = min(mmd, dist);
          }

          if (md < mmd) {
            md = mmd;
            index = k;
          }
        }

        int sid = path[index];
        path.erase(path.begin() + index);

        double minDist = DBL_MAX;
        int minIndex = -1;

        for (int j = 0; j < rsize; j++) {
          int aid = (j+1)%rsize;

          double d1 = DIST_TABLE[result[j]][sid];
          double d2 = DIST_TABLE[sid][result[aid]];
          double d3 = DIST_TABLE[result[j]][result[aid]];
          double dist = d1 + d2 - d3;

          if (minDist > dist) {
            minDist = dist;
            minIndex = aid;
          }
        }

        checkList[sid] = true;
        result.insert(result.begin()+minIndex, sid);
      }

      return result;
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

    void checkVisited(vector<int> &ships) {
      for (int i = 0; i < g_shipCount; i++) {
        Star *star = getStar(ships[i]);
        if (star->visited) continue;

        star->visited = true;
        g_remainCount--;
      }
    }

    void updateUFOInfo(vector<int> &ufos) {
      for (int i = 0; i < g_ufoCount; i++) {
        UFO *ufo = getUFO(i);

        ufo->sid = ufos[i*3];
        ufo->nid = ufos[i*3+1];
        ufo->nnid = ufos[i*3+2];
        ufo->rideoff = false;
        Star *star = getStar(ufo->sid);

        double dist = DIST_TABLE[ufo->sid][ufo->nid];

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

        if (ufo->capacity <= ufo->crew && g_turn <= 3*g_starCount) continue;

        double minDist = (ufo->crew == 0)? DBL_MAX : 0.1;
        int shipId = -1;

        for (int i = 0; i < g_shipCount; i++) {
          Ship *ship = getShip(i);
          double dist = DIST_TABLE[ufo->nid][ship->sid];

          if (ship->uid >= 0) continue;
          if (ufo->crew == 1 && existAroundStar(ship->sid)) continue;

          if (minDist > dist) {
            minDist = dist;
            shipId = i;
          }
        }

        if (shipId >= 0 && minDist <= g_changeLine) {
          Ship *ship = getShip(shipId);
          double ndist = DIST_TABLE[ship->sid][ufo->nnid];

          if (!nstar->visited || minDist < ndist) {
            fprintf(stderr,"turn %d: ship %d ride on ufo %d, dist = %f\n", g_turn, shipId, j, minDist);
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
          } else if (ufo->crew >= 2 && !ufo->rideoff && existAroundStar(ship->sid)) {
            fprintf(stderr,"turn %d: ship %d ride off ufo %d\n", g_turn, i, ship->uid);
            ship->uid = -1;
            ship->nid = ufo->sid;
            ufo->crew--;
            ufo->rideoff = true;
          } else {
            ship->nid = ufo->nid;
          }
        }
      }
    }

    void MTSPSolver(vector<int> stars, double timeLimit = TIME_LIMIT) {
      fprintf(stderr,"MTSPSolver =>\n");
      vector< vector<int> > bestPaths(g_shipCount);

      for (int i = 0; i < g_shipCount; i++) {
        g_bestPaths[i].clear();
      }

      double minDist = DBL_MAX;
      int minId = -1;
      int psize = stars.size();

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);

        for (int j = 0; j < psize; j++) {
          double dist = DIST_TABLE[ship->sid][stars[j]];

          if (minDist > dist) {
            minDist = dist;
            minId = i;
          }
        }
      }

      g_bestPaths[minId] = stars;
      g_shipList[minId].path = stars;
      vector<int> goodPath = stars;
      int c1, c2;
      int s1, s2;

      double bestScore = calcPathDistMulti();
      double goodScore = bestScore;

      ll startCycle = getCycle();
      double currentTime;
      ll tryCount = 0;

      int type;
      double newScore, subScore;

      while(1) {
        if (g_psize > 1) {
          do {
            c1 = xor128() % g_psize;
            c2 = xor128() % g_psize;
          } while (c1 == c2);
        } else {
          c1 = 0;
          c2 = 0;
        }

        do {
          s1 = xor128() % g_shipCount;
          s2 = xor128() % g_shipCount;
        } while (s1 == s2);

        type = xor128()%9;
        Ship *ship1 = getShip(s1);
        Ship *ship2 = getShip(s2);
        int size1 = ship1->path.size();
        int size2 = ship2->path.size();

        if (type <= 1 && size1 <= max(c1, c2)) {
          continue;
        } else if (type == 0 && size1 <= 2) {
          continue;
        } else if (type == 1 && size1 <= 1) {
          continue;
        } else if (type == 2 && size1 == 0) {
          continue;
        } else if (type == 3 && size1 == 0 && size2 == 0) {
          continue;
        } else if (type == 4 && size1 <= 1) {
          continue;
        } else if (type == 5 && size1 == 0) {
          continue;
        } else if (type == 6 && size1 == 0) {
          continue;
        } else if (type == 7 && size1 <= 1) {
          continue;
        } else if (type == 8 && size1 <= 2) {
          continue;
        }

        tryCount++;

        switch(type) {
          case 0:
            reconnectPath(c1, c2, ship1->path);
            break;
          case 1:
            subScore = calcSubPathDist(c1) + calcSubPathDist(c2);
            swapStar(c1, c2, ship1->path);
            break;
          case 2:
            insertStarMulti(s1, s2);
            break;
          case 3:
            swapPath(s1, s2);
            break;
          case 4:
            reversePath(s1);
            break;
          case 5:
            cutPath(s1, s2);
            break;
          case 6:
            cutPathReverse(s1, s2);
            break;
          case 7:
            insertStarMulti2(s1, s2);
            break;
          case 8:
            insertStarMS(ship1->path);
            break;
        }

        if (type == 1) {
          newScore = goodScore - (subScore - (calcSubPathDist(c1) + calcSubPathDist(c2)));
        } else {
          newScore = calcPathDistMulti();
        }

        if (bestScore > newScore) {
          bestScore = newScore;

          for (int i = 0; i < g_shipCount; i++) {
            g_bestPaths[i] = g_shipList[i].path;
          }
        }

        if (goodScore > newScore) {
          goodScore = newScore;
        } else {
          switch (type) {
            case 1:
              swapStar(c1, c2, ship1->path);
              break;
            default:
              for (int i = 0; i < g_shipCount; i++) {
                g_shipList[i].path = g_bestPaths[i];
              }
            break;
          }
        }

        if (tryCount % 10 == 0) {
          currentTime = getTime(startCycle);

          if (currentTime > timeLimit) {
            break;
          }
        }
      }

      for (int i = 0; i < g_shipCount; i++) {
        Ship *ship = getShip(i);
        ship->path = g_bestPaths[i];
        fprintf(stderr,"ship %d: path size = %lu\n", i, ship->path.size());
      }

      fprintf(stderr,"tryCount = %lld\n", tryCount);
      fprintf(stderr,"path size = %d, pathDist = %f\n", g_psize, bestScore + g_currentCost);
    }

    vector<int> TSPSolver(vector<int> stars, double timeLimit = TIME_LIMIT) {
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
      double subScore, newScore;

      ll startCycle = getCycle();
      double currentTime;
      ll tryCount = 0;

      double T = 10000.0;
      double k = 1.0;
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
            subScore = calcSubPathDist(c1) + calcSubPathDist(c2);
            reconnectPath(c1, c2, g_path);
            break;
          case 1:
            subScore = calcSubPathDist(c1) + calcSubPathDist(c2);
            swapStar(c1, c2, g_path);
            break;
          case 2:
            insertStar(c1, c2, g_path);
            break;
          case 3:
            insertStar2(c1, c2);
            break;
        }

        if (type <= 1) {
          newScore = goodScore - (subScore - (calcSubPathDist(c1) + calcSubPathDist(c2)));
        } else {
          newScore = calcPathDist();
        }

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
              swapStar(c1, c2, g_path);
              break;
            default:
              g_path = bestPath;
              break;
          } 
        }

        if (tryCount % 10 == 0) {
          currentTime = getTime(startCycle);

          if (currentTime > timeLimit) {
            break;
          }
        }

        T *= alpha;
      }

      fprintf(stderr,"tryCount = %lld\n", tryCount);
      fprintf(stderr,"path size = %d, pathDist = %f\n", g_psize, bestScore + g_currentCost);

      return bestPath;
    }

    void swapStar(int c1, int c2, vector<int> &path) {
      int temp = path[c1];
      path[c1] = path[c2];
      path[c2] = temp;
    }

    void insertStar(int c1, int c2, vector<int> &path) {
      int temp = g_path[c1];

      path.erase(path.begin()+c1);
      path.insert(path.begin()+c2, temp);
    }

    void insertStarMS(vector<int> &path) {
      int c1, c2;
      int size = path.size();
      do {
        c1 = xor128() % size;
        c2 = xor128() % size;
      } while (c1 == c2);

      int temp = path[c1];
      path.erase(path.begin()+c1);
      path.insert(path.begin()+c2, temp);
    }

    void insertStarMulti(int s1, int s2) {
      Ship *ship1 = getShip(s1);
      Ship *ship2 = getShip(s2);
      int size1 = ship1->path.size();
      int size2 = ship2->path.size();

      int c1 = xor128() % size1;
      int c2 = (size2 == 0)? 0 : xor128() % size2;

      int temp = ship1->path[c1];

      ship1->path.erase(ship1->path.begin()+c1);
      ship2->path.insert(ship2->path.begin()+c2, temp);
    }

    void insertStarMulti2(int s1, int s2) {
      Ship *ship1 = getShip(s1);
      Ship *ship2 = getShip(s2);
      int size1 = ship1->path.size();
      int size2 = ship2->path.size();

      int c1 = (size1 == 0)? 0 : xor128() % size1;
      int c2 = (size2 == 0)? 0 : xor128() % size2;

      if (c1 > size1-3 || c2 > size2-3) {
        return;
      }

      int temp = ship1->path[c1];
      int temp2 = ship1->path[c1+1];

      ship1->path.erase(ship1->path.begin()+c1);
      ship1->path.erase(ship1->path.begin()+c1);
      ship2->path.insert(ship2->path.begin()+c2, temp);
      ship2->path.insert(ship2->path.begin()+c2, temp2);
    }

    void swapPath(int s1, int s2) {
      Ship *ship1 = getShip(s1);
      Ship *ship2 = getShip(s2);

      vector<int> path = ship1->path;
      ship1->path = ship2->path;
      ship2->path = path;
    }

    void reversePath(int s1) {
      Ship *ship1 = getShip(s1);
      reverse(ship1->path.begin(), ship1->path.end());
    }

    void cutPath(int s1, int s2) {
      Ship *ship1 = getShip(s1);
      Ship *ship2 = getShip(s2);
      int size1 = ship1->path.size();

      vector<int> path = ship1->path;
      int c1 = (size1 == 0)? 0 : xor128() % size1;
      ship1->path.clear();

      for (int i = 0; i < size1; i++) {
        if (i < c1) {
          ship1->path.push_back(path[i]);
        } else {
          ship2->path.push_back(path[i]);
        }
      }
    }

    void cutPathReverse(int s1, int s2) {
      Ship *ship1 = getShip(s1);
      Ship *ship2 = getShip(s2);
      int size1 = ship1->path.size();

      vector<int> path = ship1->path;
      int c1 = (size1 == 0)? 0 : xor128() % size1;
      ship1->path.clear();

      for (int i = 0; i < size1; i++) {
        if (i < c1) {
          ship1->path.push_back(path[i]);
        } else {
          ship2->path.insert(ship2->path.begin(), path[i]);
        }
      }
    }

    void cleanPathSingle(int shipId) {
      double minDist = DBL_MAX;
      int index = -1;
      Ship *ship = getShip(shipId);
      int size = ship->path.size();

      if (size <= 1) return;

      for (int i = 0; i < size; i++) {
        int sid = ship->path[i];
        Star *star = getStar(sid);

        double dist = DIST_TABLE[ship->sid][sid];
        
        if (minDist > dist) {
          minDist = dist;
          index = i;
        }
      }

      int bid = (index == 0)? size-1 : index-1;
      int aid = (index+1)%size;

      double d1 = DIST_TABLE[ship->path[bid]][ship->path[index]];
      double d2 = DIST_TABLE[ship->path[index]][ship->path[aid]];
      vector<int> npath;

      if (d1 > d2) {
        for (int i = 0; i < size; i++) {
          npath.push_back(ship->path[(index+i)%size]);
        }
      } else {
        for (int i = 0; i < size; i++) {
          int j = index-i;

          if (j < 0) {
            npath.push_back(ship->path[size+j]);
          } else {
            npath.push_back(ship->path[j]);
          }
        }
      }

      ship->path = npath;
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

    double calcSubPathDist(int index) {
      int bid = (index == 0)? g_psize-1 : index-1;
      int aid = (index+1)%g_psize;

      double d1 = DIST_TABLE[g_path[bid]][g_path[index]];
      double d2 = DIST_TABLE[g_path[index]][g_path[aid]];

      return (d1+d2);
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

    bool existAroundStar(int sid) {
      for (int i = 0; i < g_starCount; i++) {
        Star *star = getStar(i);
        if (star->visited) continue;

        if (DIST_TABLE[sid][i] <= 40.0) {
          return true;
        }
      }

      return false;
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

    void showPath(vector<int> &path) {
      int size = path.size();
      if (size == 0) return;
      for (int i = 0; i < size-1; i++) {
        fprintf(stderr," %d ->", path[i]);
      }
      fprintf(stderr," %d\n", path[size-1]);
    }
};

// -------8<------- end of solution submitted to the website -------8<-------
template<class T> void getVector(vector<T>& v) { for (int i = 0; i < v.size(); ++i) cin >> v[i];}
int main() {
  //TIME_LIMIT = 16.0;
  //FIRST_TIME_LIMIT = 1.0;
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

