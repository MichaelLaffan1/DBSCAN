import random

def generate_test_case(filename, epsilon, min_points, num_points):
    with open(filename, 'w') as file:
        file.write(f"{epsilon} {min_points}\n")
        for i in range(num_points):
            x = random.uniform(0, num_points/100)
            y = random.uniform(0, num_points/100)
            file.write(f"{x} {y}\n")

def main():
    generate_test_case("testcase_1k.txt", 30, 5, 1000)
    generate_test_case("testcase_2k.txt", 30, 5, 2000)
    generate_test_case("testcase_5k.txt", 30, 5, 5000)
    generate_test_case("testcase_10k.txt", 30, 5, 10000)
    generate_test_case("testcase_20k.txt", 30, 5, 20000)
    generate_test_case("testcase_50k.txt", 30, 5, 50000)
    generate_test_case("testcase_100k.txt", 30, 5, 100000)

if __name__ == "__main__":
    main()
