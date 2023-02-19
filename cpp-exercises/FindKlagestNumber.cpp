#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

int partition(std::vector<int> &arr, int low, int high) {
    int pivot_index = low + rand() % (high - low + 1);
    int pivot = arr[pivot_index];
    std::swap(arr[pivot_index], arr[high]);
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

// Recursive version
int find_kth_largest_recursive(std::vector<int> &arr, int k, int low, int high) {
    if (low == high) {
        return arr[low];
    }

    int pivot_index = partition(arr, low, high);

    if (k == pivot_index) {
        return arr[k];
    } else if (k < pivot_index) {
        return find_kth_largest_recursive(arr, k, low, pivot_index - 1);
    } else {
        return find_kth_largest_recursive(arr, k, pivot_index + 1, high);
    }
}

// Iterative version
int find_kth_largest_iterative(std::vector<int> &arr, int k) {
    int low = 0, high = arr.size() - 1;
    while (low <= high) {
        int pivot_index = partition(arr, low, high);
        if (k == pivot_index) {
            return arr[k];
        } else if (k < pivot_index) {
            high = pivot_index - 1;
        } else {
            low = pivot_index + 1;
        }
    }
    return -1; // This should never happen if k is a valid input
}

int main() {
    srand(time(0));
    std::vector<int> vector = {3, 2, 1, 5, 6, 4};
    int k = 2; // 1-based index, 1st largest number

    // Convert k to 0-based index for k-th largest number
    k = vector.size() - k;

    std::cout << "Recursive: " << find_kth_largest_recursive(vector, k, 0, vector.size() - 1) << std::endl;
    std::cout << "Iterative: " << find_kth_largest_iterative(vector, k) << std::endl;

    return 0;
}