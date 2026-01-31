#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

int main() {
    long long r = 100000; // rozsah hodnot pro x a y: -r až r-1
    const long long target = 2022;  // cílová hodnota v podmínce

    // počet vláken 
    unsigned int thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0) thread_count = 4;

    long long total_x = r;
    std::atomic<long long> processed_x{0};
    std::atomic<int> last_percent{0};
    std::mutex cout_mutex;

    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    auto worker = [r, total_x, target, &processed_x, &last_percent, &cout_mutex](long long x_start, long long x_end) {
        for (long long x = x_start; x < x_end; ++x) {
            long long y = -r;
            while (y <= r) {
                long long val = x * x - x * y + y * y;
                if (val == target) {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << x << " " << y << std::endl;
                }
                ++y;
            }
            long long done = ++processed_x;
            int percent = static_cast<int>(done * 100 / total_x);
            if (percent > 100) percent = 100;

            int prev = last_percent.load(std::memory_order_relaxed);
            if (percent > prev) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                if (percent > last_percent.load(std::memory_order_relaxed)) {
                    last_percent.store(percent, std::memory_order_relaxed);
                    std::cout << "Progress: " << percent << "%" << std::endl;
                }
            }
        }
    };

    long long base_chunk = total_x / static_cast<long long>(thread_count);
    long long rest = total_x % static_cast<long long>(thread_count);

    long long current = 0;
    for (unsigned int i = 0; i < thread_count; ++i) {
        int extra = (i < static_cast<unsigned int>(rest)) ? 1 : 0;
        long long next = current + base_chunk + extra;
        threads.emplace_back(worker, current, next);
        current = next;
    }

    for (auto &t : threads) {
        t.join();
    }

    std::cout << "Done!" << std::endl;
    return 0;
}

