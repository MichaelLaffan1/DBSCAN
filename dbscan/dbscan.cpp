#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>

using namespace std;

const int UNCLASSIFIED = -1;
const int NOISE = 0;

class Point {
public:
    double x, y;
    int c_id;
    Point(double x = 0, double y = 0) : x(x), y(y), c_id(UNCLASSIFIED) {}
};

// Global variables
int numPoints;
double eps;
int minPts;
mutex mtx;

// Compute Euclidean distance between two points
double distance(const Point& p1, const Point& p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    return sqrt(pow(dx, 2) + pow(dy, 2));
}

// Expand cluster by finding density-connected points
void expandCluster(Point* points, int num_points, int index, int cluster_id, int* visited) {
    points[index].c_id = cluster_id;
    for (int i = 0; i < num_points; ++i) {
        if (visited[i] == 0 && distance(points[index], points[i]) <= eps) {
            visited[i] = 1;
            points[i].c_id = cluster_id;
            expandCluster(points, num_points, i, cluster_id, visited); 
        }
    }
}

void writeToFile(const string& filename, Point* points) {
    mtx.lock();
    ofstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < numPoints; i++) {
            file << points[i].x << " " << points[i].y << " " << points[i].c_id << endl;
        }
        file.close();
    }
    else {
        cerr << "Error opening file for writing: " << filename << endl;
    }
    mtx.unlock();
}

void readFromFile(const string& filename, Point*& points, int*& visited) {
    ifstream file(filename);
    if (file.is_open()) {
        file >> eps >> minPts;

        if (minPts <= 0) {
            cerr << "Minimum points should be greater than 0" << endl;
            exit(1);
        }
        if (eps < 0) {
            cerr << "Epsilon must be a non-negative value" << endl;
            exit(1);
        }

        double x, y;
        numPoints = 0;
        while (file >> x >> y) {
            numPoints++;
        }
        file.clear();
        file.seekg(0, ios::beg);

        file >> eps >> minPts;

        points = new Point[numPoints];
        visited = (int*)malloc(numPoints * sizeof(int));

        int i = 0;
        while (file >> x >> y) {
            points[i] = Point(x, y);
            visited[i] = 0;
            i++;
        }

        file.close();
    }
    else {
        cerr << "Error opening file for reading: " << filename << endl;
        exit(1);
    }
}

void dbscan_thread(Point* points, int num_points, int thread_id, int total_threads, int* visited, int& cluster_id) {
    int start = thread_id * (num_points / total_threads);
    int end = (thread_id == total_threads - 1) ? num_points : (thread_id + 1) * (num_points / total_threads);

    for (int i = start; i < end; ++i) {
        if (points[i].c_id == UNCLASSIFIED) {
            int neighbor_count = 0;
            for (int j = 0; j < num_points; ++j) {
                if (distance(points[i], points[j]) <= eps) {
                    neighbor_count++;
                }
            }
            if (neighbor_count >= minPts) {
                mtx.lock();
                cluster_id++;
                mtx.unlock();
                expandCluster(points, num_points, i, cluster_id, visited);
            }
            else {
                points[i].c_id = NOISE; // Noise point
            }
        }
    }
}

int main() {
    string filein = "points.txt";
    string outfile = "results.txt";
    int cluster_id = 0;

    Point* points;
    int* visited;
    const int numThreads = 10;

    readFromFile(filein, points, visited);

    thread threads [numThreads];

    for (int i = 0; i < numThreads; ++i) {
        threads[i] = thread(dbscan_thread, points, numPoints, i, numThreads, visited, ref(cluster_id));
    }

    for (int i = 0; i < numThreads; ++i) {
        threads[i].join();
    }

    writeToFile(outfile, points);

    free(points);
    free(visited);

    return 0;
}
