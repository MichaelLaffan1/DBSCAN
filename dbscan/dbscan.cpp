// dbscan.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <thread>
//#include <atomic>
//#include <semaphore>
#include <mutex>

using namespace std;

const int UNCLASSIFIED = -1;
const int NOISE = 0;
const int MAX_POINTS = 1000; // could play around w/ higher numbers of points

class Point {
public:
    double x, y;
    int c_id;
    Point(double x = 0, double y = 0) : x(x), y(y), c_id(UNCLASSIFIED) {}
};

// Global variables
int numPoints = 0;
Point* points;
mutex mtx;


void formatPoint(Point& point, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%.1f %.1f %d", point.x, point.y, point.c_id);
}

// Compute square root distance between two points
double distance(Point& p1, Point& p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dist = sqrt(pow(dx, 2) + pow(dy, 2));
    return dist;
}

// Find points within the epsilon neighborhood of a point
int regionQuery(Point& point, double Eps, Point* neighbors[], int maxNeighbors) {
    int count = 0;
    for (int i = 0; i < numPoints; ++i) {
        if (distance(point, points[i]) <= Eps) {
            if (count < maxNeighbors) {
                neighbors[count++] = &points[i];
            }
        }
    }
    return count;
}

// Expand a cluster
bool expandCluster(Point* point, int c_id, double Eps, int MinPts) {
    Point* neighbors[MAX_POINTS];
    int numNeighbors = regionQuery(*point, Eps, neighbors, MAX_POINTS);
    if (numNeighbors < MinPts) {
        point->c_id = NOISE; // can't be point.c_id b/c 'point' is a pointer, not a direct instance
        return false;
    }
    else {
        point->c_id = c_id;
        for (int i = 0; i < numNeighbors; ++i) {
            if (neighbors[i]->c_id == UNCLASSIFIED || neighbors[i]->c_id == NOISE) {
                if (neighbors[i]->c_id == UNCLASSIFIED) {
                    Point* newNeighbors[MAX_POINTS];
                    int numNewNeighbors = regionQuery(*neighbors[i], Eps, newNeighbors, MAX_POINTS);
                    for (int j = 0; j < numNewNeighbors; ++j) {
                        if (newNeighbors[j]->c_id == UNCLASSIFIED) {
                            neighbors[numNeighbors++] = newNeighbors[j];
                        }
                    }
                }
                neighbors[i]->c_id = c_id;
            }
        }
        return true;
    }
}

// Clustering algorithm
void DBSCAN(double Eps, int MinPts, Point*& points, int numPoints) {
    int ClusterId = 1;

    for (int i = 0; i < numPoints; ++i) {
        if (points[i].c_id == UNCLASSIFIED) {
            if (expandCluster(&points[i], ClusterId, Eps, MinPts)) {
                ClusterId++;
            }
        }
    }
}

void writeToFile(const string& filename) {
    char buffer[256];
    ofstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < numPoints; i++) {
            formatPoint(points[i], buffer, sizeof(buffer));
            file << buffer << endl;
        }
        file.close();
    }
    else {
        cerr << "Error opening file for writing: " << filename << endl;
    }
}

void readFromFile(const string& filename, Point*& points) {
    ifstream file(filename);
    if (file.is_open()) {
        double x, y;
        while (file >> x >> y) {
            numPoints++;
        }
        file.clear();
        file.seekg(0, ios::beg);
        points = new Point[numPoints];

        int i = 0;
        while (file.good()) {
            file >> x >> y;
            points[i++] = Point(x, y);
        }
        file.close();
    }
    else {
        cerr << "Error opening file for reading: " << filename << endl;
        exit(1);
    }
}

int main()
{

    string filename = "points.txt";
    
    double eps = 1.5;
    int minPts = 2;

    readFromFile(filename, points);

    DBSCAN(eps, minPts, points, numPoints);

    writeToFile(filename);

    return 0;
}
