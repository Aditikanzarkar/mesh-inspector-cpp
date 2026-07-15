#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "core/ThreadPool.h"
#include "core/Triangle.h"
#include "geometry/AreaCalculator.h"
#include "geometry/VolumeCalculator.h"
#include "parser/STLParser.h"

struct JobState {
    enum class Status { kQueued, kRunning, kCompleted, kFailed } status = Status::kQueued;
    double area = 0.0;
    double volume = 0.0;
    std::string error;
};

class MeshJobManager {
public:
    MeshJobManager()
        : nextJobId_(1) {
    }

    std::string submitJob(const std::string& filename, const std::string& fileData) {
        const std::string jobId = "job-" + std::to_string(nextJobId_.fetch_add(1, std::memory_order_relaxed));
        {
            std::lock_guard<std::mutex> lock(mutex_);
            jobs_[jobId] = JobState{};
        }

        pool_.enqueue([this, jobId, filename, fileData]() {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                jobs_[jobId].status = JobState::Status::kRunning;
            }

            try {
                const std::vector<Triangle> triangles = STLParser::parseFromMemory(fileData, filename);
                const double area = AreaCalculator::totalArea(triangles);
                const double volume = VolumeCalculator::estimateVolume(triangles);

                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    jobs_[jobId].status = JobState::Status::kCompleted;
                    jobs_[jobId].area = area;
                    jobs_[jobId].volume = volume;
                }
            } catch (const std::exception& e) {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    jobs_[jobId].status = JobState::Status::kFailed;
                    jobs_[jobId].error = e.what();
                }
            }
        });

        return jobId;
    }

    JobState getJobStatus(const std::string& jobId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = jobs_.find(jobId);
        if (it == jobs_.end()) {
            throw std::invalid_argument("unknown job id");
        }
        return it->second;
    }

private:
    mutable std::mutex mutex_;
    std::atomic<std::size_t> nextJobId_;
    std::unordered_map<std::string, JobState> jobs_;
    ThreadPool pool_{4};
};
