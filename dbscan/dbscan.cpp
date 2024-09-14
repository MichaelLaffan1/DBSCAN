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
const int NOISE = -2;
const int MAX_POINTS = 1000; // could play around w/ higher numbers of points

class Point {
public:
    double x, y;
    int c_id;
    Point(double x = 0, double y = 0) : x(x), y(y), c_id(UNCLASSIFIED) {}
};

// Global variables
Point points[MAX_POINTS];
int numPoints = 0;
mutex mtx;

// Compute distance^2 between two points
double squaredDistance(const Point& p1, const Point& p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return dx * dx + dy * dy;
}

// Find points within the epsilon neighborhood of a point
int regionQuery(const Point& point, double EpsSquared, Point* neighbors[], int maxNeighbors) {
    int count = 0;
    for (int i = 0; i < numPoints; ++i) {
        if (squaredDistance(point, points[i]) <= EpsSquared) {
            if (count < maxNeighbors) {
                neighbors[count++] = &points[i];
            }
        }
    }
    return count;
}

// Expand a cluster
bool expandCluster(Point* point, int c_id, double EpsSquared, int MinPts) {
    Point* neighbors[MAX_POINTS];
    int numNeighbors = regionQuery(*point, EpsSquared, neighbors, MAX_POINTS);
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
                    int numNewNeighbors = regionQuery(*neighbors[i], EpsSquared, newNeighbors, MAX_POINTS);
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
void DBSCAN(double Eps, int MinPts) {
    int ClusterId = 0;
    double EpsSquared = Eps * Eps;

    for (int i = 0; i < numPoints; ++i) {
        if (points[i].c_id == UNCLASSIFIED) {
            if (expandCluster(&points[i], ClusterId, EpsSquared, MinPts)) {
                ClusterId++;
            }
        }
    }
}

void writeToFile(const char* filename, const char* content) {
    ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    }
    else {
        cerr << "Error opening file for writing: " << filename << endl;
    }
}

void readFromFile(const char* filename) {
    char buffer[256]; // max length per line
    ifstream file(filename);
    if (file.is_open()) {
        while (file.getline(buffer, 256)) {
            cout << buffer << endl;
        }
        file.close();
    }
    else {
        cerr << "Error opening file for reading: " << filename << endl;
    }
}

void formatPoint(const Point& point, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%.1f,%.1f,%d", point.x, point.y, point.c_id);
}

int main()
{

    /* char* filename = "../testcases/filename.txt";
    const char* content = "Can I write to a file AND read from it?";
    writeToFile(filename, content);

    readFromFile(filename);*/

    numPoints = 8;
    points[0] = Point(1.0, 2.0); // should be in cluster 0
    points[1] = Point(2.0, 2.0); // 0
    points[2] = Point(2.0, 3.0); // 0
    points[3] = Point(8.0, 7.0); // 1
    points[4] = Point(8.0, 8.0); // 1
    points[5] = Point(15.0, 50.0); // all alone
    points[6] = Point(1.0, 35.0); // 2
    points[7] = Point(2.0, 40.0); // 2

    double eps = 6;
    int minPts = 2;

    DBSCAN(eps, minPts);

    ofstream outFile("../outputs/points.txt");
    if (!outFile) {
        cerr << "Error opening file for writing." << endl;
        return 1;
    }
    
    char buffer[256];
    // Write points to the file
    for (int i = 0; i < numPoints; ++i) {
        formatPoint(points[i], buffer, sizeof(buffer));
        cout << buffer << endl;
        outFile << buffer << endl;
    }

    outFile.close();

    return 0;
}
