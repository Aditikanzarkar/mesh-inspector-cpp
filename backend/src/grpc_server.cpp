#include <iostream>
#include <string>

#include "grpc_service.h"

int main() {
    std::cout << "mesh inspector grpc service placeholder\n";
    MeshJobManager manager;
    const std::string jobId = manager.submitJob("demo.stl", "not-a-real-stl");
    std::cout << "submitted job: " << jobId << '\n';
    return 0;
}
