import random

def generate_test_case(filename, epsilon_factor, min_points, num_points):
    grid_size = num_points / 10
    
    epsilon = epsilon_factor * grid_size
    
    with open(filename, 'w') as file:
        file.write(f"{epsilon} {min_points}\n")
        for i in range(num_points):
            x = random.uniform(0, grid_size)
            y = random.uniform(0, grid_size)
            file.write(f"{x} {y}\n")

def main():
    epsilon_factor = 0.1 
    
    generate_test_case("testcase_1k.txt", epsilon_factor, 5, 1000)
    generate_test_case("testcase_2k.txt", epsilon_factor, 5, 2000)
    generate_test_case("testcase_5k.txt", epsilon_factor, 5, 5000)
    generate_test_case("testcase_10k.txt", epsilon_factor, 5, 10000)
    generate_test_case("testcase_20k.txt", epsilon_factor, 5, 20000)
    generate_test_case("testcase_50k.txt", epsilon_factor, 5, 50000)
    generate_test_case("testcase_100k.txt", epsilon_factor, 5, 100000)

if __name__ == "__main__":
    main()
