#include "image.hpp"
#include "spindafier.hpp"
#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

std::vector<std::filesystem::path> find_input_files(const std::filesystem::path &path){
	typedef std::filesystem::directory_iterator DI;
	std::vector<std::filesystem::path> ret;
	for (DI i(path), e; i != e; ++i)
		ret.push_back(i->path());
	return ret;
}

int main(int argc, char **argv){
	if (argc < 4){
		std::cout << "Usage: spindafy <resources path> <input directory> <output directory>\n";
		return -1;
	}
	try{
		Spindafier spindafier(argv[1]);
		auto inputs = find_input_files(argv[2]);
		std::filesystem::path output_path = argv[3];
		std::filesystem::create_directories(output_path);

		std::vector<std::thread> threads;
		auto n = std::thread::hardware_concurrency();
		threads.reserve(n);

		auto t0 = std::chrono::high_resolution_clock::now();
		std::atomic<size_t> index = 0;
		std::mutex mutex;
		for (decltype(n) i = 0; i < n; i++){
			threads.emplace_back([&inputs, &index, &spindafier, &mutex, &output_path](){
				while (true){
					auto i = index++;
					if (i >= inputs.size())
						break;
					auto path = inputs[i];
					{
						std::lock_guard lg(mutex);
						std::cout << path << std::endl;
					}
					spindafier.spindafy(Image(path)).save_png(output_path / path.filename().replace_extension(".png"));
				}
			});
		}
		for (auto &t : threads)
			t.join();
		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout << "Elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() * 0.001 << " s\n";

	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
		return -1;
	}catch (...){
		std::cerr << "Unknown exception caught.\n";
		return -1;
	}
	return 0;
}
