#include "gflags/gflags.h"

#include "ip/UdpSocket.h" // oscpack
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscPacketListener.h"
#include "osc/OscReceivedElements.h"
#include "oscpkt.hh"
#include "oscpp/client.hpp"
#include "oscpp/server.hpp"

extern "C" {
#include "lo/lo.h"
}

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

DEFINE_string(sender, "liblo", "The OSC library to use to send packets, one of liblo, oscpack, oscpkt, or oscpp.");
DEFINE_string(receiver, "liblo", "The OSC library to use to receive packets, one of liblo, oscpack, oscpkt, or oscpp.");
DEFINE_int32(iterations, 100000, "Number of iterations to measure e2e latency of.");
DEFINE_int32(port, 54321, "The localhost port to send measurement traffic over.");

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

class OscPackReceiver : public osc::OscPacketListener {
public:
    OscPackReceiver(State* state): m_state(state) {}
protected:
    virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint) {
        std::chrono::high_resolution_clock::time_point receiveTime = std::chrono::high_resolution_clock::now();
        try {
            int serial = 0;
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            args >> serial;
            {
                std::lock_guard<std::mutex> lock(m_state->sendMutex);
                m_state->serial = serial;
                int64_t dur = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        receiveTime - m_state->startTime).count();
                m_state->durations.push_back(dur);
            }
        } catch (osc::Exception& e) {
            std::cerr << "Exception on oscpack receive." << std::endl;
            m_state->quit = true;
        }

        m_state->sendCondition.notify_all();
    }

    State* m_state;
};

void oscReceiveThread(UdpListeningReceiveSocket* socket) {
    socket->Run();
}

int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    std::cout << "sender: " << FLAGS_sender << " receiver: " << FLAGS_receiver << " iterations: " << FLAGS_iterations
        << std::endl;

    std::unique_ptr<State> state = std::make_unique<State>();
    state->quit = false;
    state->serial = -1;

    // liblo receive
    lo_server_thread libloReceiverThread;
    lo_server libloReceiverServer;
    // oscpack receive
    std::unique_ptr<OscPackReceiver> oscPackReceiver;
    std::unique_ptr<UdpListeningReceiveSocket> oscPackSocket;

    // Receier-specific initialization. We build receivers first so they are sure to be ready for sending loops.
    if (FLAGS_receiver == "liblo") {
        std::array<char, 32> portBuffer;
        std::snprintf(portBuffer.data(), portBuffer.size(), "%d", FLAGS_port);
        libloReceiverThread = lo_server_thread_new_with_proto(portBuffer.data(), LO_UDP, libloError);
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
    } else if (FLAGS_receiver == "oscpack") {
        oscPackReceiver = std::make_unique<OscPackReceiver>(state.get());
        oscPackSocket = std::make_unique<UdpListeningReceiveSocket>(IpEndpointName(IpEndpointName::ANY_ADDRESS,
                    FLAGS_port), oscPackReceiver.get());
        std::thread receiverThread = std::thread(&oscReceiveThread, oscPackSocket.get());
        receiverThread.detach();
    } else if (FLAGS_receiver == "oscpkt") {
    } else if (FLAGS_receiver == "oscpp") {
    } else {
        std::cerr << "Unrecognized receiver library '" << FLAGS_receiver << "'" << std::endl;
        return -1;
    }

    if (FLAGS_sender == "liblo") {
        lo_address senderAddress;
        std::array<char, 32> portBuffer;
        std::snprintf(portBuffer.data(), portBuffer.size(), "%d", FLAGS_port);
        senderAddress = lo_address_new(nullptr, portBuffer.data());
        if (!senderAddress) {
            std::cerr << "Error creating liblo sender address." << std::endl;
            return -1;
        }
        for (int i = 0; i < FLAGS_iterations; ++i) {
            if (state->quit) {
                break;
            }
            state->startTime = std::chrono::high_resolution_clock::now();
            lo_send(senderAddress, "/timer", "i", i);
            {
                std::unique_lock<std::mutex> lock(state->sendMutex);
                const State* statePtr = state.get();
                state->sendCondition.wait(lock, [statePtr, i] { return statePtr->quit || statePtr->serial == i; });
            }
        }

        lo_address_free(senderAddress);
    } else if (FLAGS_sender == "oscpack") {
        UdpTransmitSocket transmitSocket(IpEndpointName("127.0.0.1", FLAGS_port));
        std::array<char, 1024> buffer;

        for (int i = 0; i < FLAGS_iterations; ++i) {
            if (state->quit) {
                break;
            }
            state->startTime = std::chrono::high_resolution_clock::now();
            osc::OutboundPacketStream p(buffer.data(), buffer.size());
            p << osc::BeginMessage("/timer") << i << osc::EndMessage;
            transmitSocket.Send(p.Data(), p.Size());
            {
                std::unique_lock<std::mutex> lock(state->sendMutex);
                const State* statePtr = state.get();
                state->sendCondition.wait(lock, [statePtr, i] { return statePtr->quit || statePtr->serial == i; });
            }
        }
    } else if (FLAGS_sender == "oscpkt") {
    } else if (FLAGS_sender == "oscpp") {
    } else {
        std::cerr << "Unrecognized sender library '" << FLAGS_sender << "'" << std::endl;
        return -1;
    }

    // Compute results
    std::sort(state->durations.begin(), state->durations.end());

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

    std::cout << "mean latency: " << mean << " ns, std dev: " << dev << " ns" << std::endl;
    std::cout << "50%: " << state->durations[state->durations.size() / 2]
            << " 75%: " << state->durations[state->durations.size() * 3 / 4]
            << " 90%: " << state->durations[state->durations.size() * 9 / 10]
            << " 99%: " << state->durations[state->durations.size() * 99 / 100]
            << " worst: " << state->durations[state->durations.size() - 1] << std::endl;

    if (FLAGS_receiver == "liblo") {
        lo_server_thread_stop(libloReceiverThread);
        lo_server_thread_free(libloReceiverThread);
    } else if (FLAGS_receiver == "oscpack") {
        oscPackSocket->AsynchronousBreak();
    } else if (FLAGS_receiver == "oscpkt") {
    } else if (FLAGS_receiver == "oscpp") {
    }

    return 0;
}
