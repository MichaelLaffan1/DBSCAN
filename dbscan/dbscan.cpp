#include <iostream>
#include <fstream>
#include <mutex>

using namespace std;

const int UNCLASSIFIED = -1;
const int NOISE = 0;
const int MAX_POINTS = 1000; // Adjust as needed

class Point {
public:
    double x, y;
    int c_id;
    Point(double x = 0, double y = 0) : x(x), y(y), c_id(UNCLASSIFIED) {}
};

// Global variables
int numPoints;
double eps;
double minPts;
Point* points;
mutex mtx;

// Compute square root distance between two points
double distance(const Point& p1, const Point& p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    return sqrt(dx * dx + dy * dy);
}

// Find points within the epsilon neighborhood of a point
int regionQuery(const Point& point, Point* neighbors[], int maxNeighbors) {
    int count = 0;
    for (int i = 0; i < numPoints; ++i) {
        if (distance(point, points[i]) <= eps) {
            if (count < maxNeighbors) {
                neighbors[count++] = &points[i];
            }
        }
    }
    return count;
}

// Expand a cluster
bool expandCluster(Point& point, int c_id) {
    Point* seeds[MAX_POINTS];
    int numSeeds = regionQuery(point, seeds, MAX_POINTS);

    if (numSeeds < minPts) {
        point.c_id = NOISE;
        return false;
    }
    else {
        point.c_id = c_id;
        int seedsCount = numSeeds;

        for (int i = 0; i < numSeeds; ++i) {
            seeds[i]->c_id = c_id;
        }

        int index = 0;
        while (index < seedsCount) {
            Point* currentP = seeds[index];
            index++;

            Point* result[MAX_POINTS];
            int numResultNeighbors = regionQuery(*currentP, result, MAX_POINTS);
            if (numResultNeighbors >= minPts) {
                for (int j = 0; j < numResultNeighbors; ++j) {
                    Point* resultP = result[j];
                    if (resultP->c_id == UNCLASSIFIED || resultP->c_id == NOISE) {
                        if (resultP->c_id == UNCLASSIFIED) {
                            if (seedsCount < MAX_POINTS) {
                                seeds[seedsCount++] = resultP;
                            }
                            else {
                                cerr << "Warning: Seeds array full!" << endl;
                            }
                        }
                        resultP->c_id = c_id;
                    }
                }
            }
        }
        return true;
    }
}

void writeToFile(const string& filename) {
    mtx.lock();
    char buffer[256];
    ofstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < numPoints; i++) {
            snprintf(buffer, sizeof(buffer), "%.1f %.1f %d", points[i].x, points[i].y, points[i].c_id);
            file << buffer << endl;
        }
        file.close();
    }
    else {
        cerr << "Error opening file for writing: " << filename << endl;
    }
    mtx.unlock();
}

void readFromFile(const string& filename) {
    mtx.lock();
    ifstream file(filename);
    if (file.is_open()) {
        numPoints = 0;
        file >> eps >> minPts;

        if (minPts <= 0) {
            cerr << "Minimum points should be greater than 0" << endl;
            exit(1);
        }
        if (int(minPts) != minPts) {
            cerr << "Minimum points should be an integer" << endl;
            exit(1);
        }
        if (eps < 0) {
            cerr << "Epsilon must be a non-negative value" << endl;
            exit(1);
        }


        double x, y;
        while (file >> x >> y) {
            numPoints++;
        }
        file.clear();
        file.seekg(0, ios::beg);

        file >> eps >> minPts;
        points = new Point[numPoints];

        int i = 0;
        while (file >> x >> y) {
            points[i++] = Point(x, y);
        }

        for (int i = 0; i < numPoints; i++) {
            for (int j = 0; j < numPoints; j++) {
                if (i != j) {
                    if (points[i].x == points[j].x && points[i].y == points[j].y) {
                        cerr << "Cannot have duplicate coordinates" << endl;
                        exit(1);
                    }
                }
            }
        }
        file.close();
    }
    else {
        cerr << "Error opening file for reading: " << filename << endl;
        exit(1);
    }
    mtx.unlock();
}

void DBSCAN() {
    mtx.lock();
    int ClusterId = 1;

    for (int i = 0; i < numPoints; ++i) {
        if (points[i].c_id == UNCLASSIFIED) {
            if (expandCluster(points[i], ClusterId)) {
                ClusterId++;
            }
        }
    }
    mtx.unlock();
}

int main() {
    string filein = "points.txt";
    string outfile = "results.txt";

    thread r(readFromFile,filein);
    thread d(DBSCAN);
    thread w(writeToFile, outfile);

    r.join();
    d.join();
    w.join();

    free(points); // Clean up dynamically allocated memory
    return 0;
}
