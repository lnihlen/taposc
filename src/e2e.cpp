#include "gflags/gflags.h"

#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"
#include "oscpkt.hh"
#include "oscpp/client.hpp"
#include "oscpp/server.hpp"

extern "C" {
#include "lo/lo.h"
}

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

DEFINE_string(sender, "liblo", "The OSC library to use to send packets, one of liblo, oscpack, oscpkt, or oscpp.");
DEFINE_string(receiver, "liblo", "The OSC library to use to receive packets, one of liblo, oscpack, oscpkt, or oscpp.");
DEFINE_int32(iterations, 100000, "Number of iterations to measure e2e latency of.");
DEFINE_string(port, "54321", "The localhost port to send measurement traffic over.");

struct State {
    int serial;
    bool quit;
    std::chrono::high_resolution_clock::time_point startTime;
    std::mutex sendMutex;
    std::condition_variable sendCondition;
    std::vector<int64_t> durations;
};

void libloError(int number, const char* message, const char* path) {
    std::cerr << "liblo error received: " << message << std::endl;
}

int libloHandleReceive(const char* path, const char* types, lo_arg** argv, int argc, lo_message message,
        void* userData) {
    std::chrono::high_resolution_clock::time_point receiveTime = std::chrono::high_resolution_clock::now();
    State* state = reinterpret_cast<State*>(userData);
    if (argc != 1 || *types != 'i') {
        std::cerr << "Bad liblo received message format." << std::endl;
        state->quit = true;
    } else {
        std::lock_guard<std::mutex> lock(state->sendMutex);
        state->serial = *reinterpret_cast<int32_t*>(argv[0]);
        int64_t dur = std::chrono::duration_cast<std::chrono::nanoseconds>(receiveTime - state->startTime).count();
        state->durations.push_back(dur);
    }
    state->sendCondition.notify_all();
    return 0;
}

int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    std::unique_ptr<State> state = std::make_unique<State>();
    state->quit = false;
    state->serial = -1;

    lo_server_thread libloReceiverThread;
    lo_server libloReceiverServer;
    if (FLAGS_receiver == "liblo") {
        libloReceiverThread = lo_server_thread_new_with_proto(FLAGS_port.data(), LO_UDP, libloError);
        if (!libloReceiverThread) {
            std::cerr << "Error creating liblo receiver thread." << std::endl;
            return -1;
        }
        libloReceiverServer = lo_server_thread_get_server(libloReceiverThread);
        lo_server_thread_add_method(libloReceiverThread, nullptr, nullptr, libloHandleReceive, state.get());
        if (lo_server_thread_start(libloReceiverThread) < 0) {
            std::cerr << "Failed to start liblo receiver server." << std::endl;
            return -1;
        }
    }

    lo_address libloSenderAddress;
    if (FLAGS_sender == "liblo") {
        libloSenderAddress = lo_address_new(nullptr, FLAGS_port.data());
        if (!libloSenderAddress) {
            std::cerr << "Error creating liblo sender address." << std::endl;
            return -1;
        }
        for (int i = 0; i < FLAGS_iterations; ++i) {
            if (state->quit) {
                break;
            }
            state->startTime = std::chrono::high_resolution_clock::now();
            lo_send(libloSenderAddress, "/timer", "i", i);
            {
                std::unique_lock<std::mutex> lock(state->sendMutex);
                const State* statePtr = state.get();
                state->sendCondition.wait(lock, [statePtr, i] { return statePtr->quit || statePtr->serial >= i; });
            }
        }
    }

    // Compute results
    int64_t sum = 0;
    for (auto dur : state->durations) {
        sum += dur;
    }
    int64_t mean = sum / state->durations.size();
    sum = 0;
    for (auto dur : state->durations) {
        int64_t diff = dur - mean;
        sum += (diff * diff);
    }
    int64_t dev = sqrt(sum / state->durations.size());

    std::cout << "mean duration: " << mean << " ns, std dev: " << dev << " ns" << std::endl;
    return 0;
}
