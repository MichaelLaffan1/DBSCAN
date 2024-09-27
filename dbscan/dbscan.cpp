#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>

using namespace std;
using namespace chrono;

const int UNCLASSIFIED = -1;
const int NOISE = 0;

class Point {
public:
    double x, y;
    int c_id;
    int t_id;
    Point(double x = 0, double y = 0) : x(x), y(y), c_id(UNCLASSIFIED), t_id(t_id) {}
};

// Global variables
int numPoints;
double eps;
int minPts;
mutex mtx;
unsigned int maxCores = thread::hardware_concurrency(); // returns the number of concurrent threads supported by the hardware implementation

// Compute Euclidean distance between two points
double distance(const Point& p1, const Point& p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    return sqrt(pow(dx, 2) + pow(dy, 2));
}

//expands a cluster
void expandCluster(Point* points, int num_points, int index, int cluster_id, int* visited, int thread_id) {
    // Simulate a stack using a dynamic array
    int* neighbors = (int*)malloc(num_points * sizeof(int));
    int top = -1;

    // Start with the initial point
    neighbors[++top] = index;

    points[index].c_id = cluster_id;
    points[index].t_id = thread_id;

    while (top >= 0) {
        int current = neighbors[top--]; // Pop from the stack

        for (int i = 0; i < num_points; ++i) {
            if (visited[i] == 0 && distance(points[current], points[i]) <= eps) {
                visited[i] = 1;
                points[i].c_id = cluster_id;
                points[i].t_id = thread_id;
                neighbors[++top] = i;  // Push to the stack
            }
        }
    }

    // Free the allocated memory for the "stack"
    free(neighbors);
}


//writes results to results file
void writeToFile(const string& filename, Point* points) {
    ofstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < numPoints; i++) {
            file << points[i].x << " " << points[i].y << " " << points[i].c_id << " " << points[i].t_id << endl;
        }
        file.close();
    }
    else {
        cerr << "Error opening file for writing: " << filename << endl;
    }
}

//reads input from file
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
}

//performs dbscan and distributes work for each thread
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
                expandCluster(points, num_points, i, cluster_id, visited, thread_id);
            }
            else {
                points[i].c_id = NOISE; // Noise point
            }
        }
    }
}

//prompt for selecting yes or no
bool getYesNo(string& prompt) {
    string input;
    while (true) {
        cout << prompt << " (yes/no): ";
        cin >> input;

        for (auto& c : input)
            c = tolower(c); // Convert to lowercase for easier comparison

        if (input == "yes" or input == "y")
            return true;
        else if (input == "no" or input == "n")
            return false;
        else
            std::cout << "Invalid input. Please enter 'yes' or 'no'." << std::endl;
    }
}

//gets number of threads from user
int getThreads() {
    int numThreads = -1;
    cout << "Enter the number of threads you want to run DBSCAN with. The maximum for your system is: " << maxCores << endl;
    while (numThreads == -1) {
        cin >> numThreads;
        if (numThreads <= 0) {
            cout << "Invalid number of threads. Please use a positive integer" << endl;
        }
        else if (numThreads > maxCores) {
            string prompt = "Are you sure? There will be more threads than cores...";
            bool answer = getYesNo(prompt);
            if (answer)
                return numThreads;
            else
                cout << "Using max number of cores instead, " << maxCores << endl;
                numThreads = maxCores;
        }
    }
    return numThreads;
}

//main method
int main() {
    string filein = "points.txt";
    string outfile = "results.txt";
    int cluster_id = 0;

    Point* points;
    int* visited;

    const int numThreads = getThreads();

    auto start = high_resolution_clock::now();

    readFromFile(filein, points, visited);

    thread* threads = new thread[numThreads];

    for (int i = 0; i < numThreads; ++i) {
        threads[i] = thread(dbscan_thread, points, numPoints, i, numThreads, visited, ref(cluster_id));
    }

    
    for (int i = 0; i < numThreads; ++i) {
        threads[i].join();
    }
    
    writeToFile(outfile, points);

    delete[] threads;
    free(points);
    free(visited);

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Runtime: " << duration.count() << " milliseconds." << endl;

    return 0;
}
