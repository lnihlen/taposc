#include "benchmark/benchmark.h"
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"

extern "C" {
#include "lo/lo.h"
}

#include <array>
#include <vector>

static const char* dolorem = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
        "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
        "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit "
        "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa "
        "qui officia deserunt mollit anim id est laborum.";

static void BM_liblo_serialize_empty(benchmark::State& state) {
    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_int32_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_add_int32(message, 0);
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_int32_series(benchmark::State& state) {
    std::array<char, 1024> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        for (int i = 0; i < 100; ++i) {
            lo_message_add_int32(message, i);
        }
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_float_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_add_float(message, 0.0f);
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_float_series(benchmark::State& state) {
    std::array<char, 1024> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        for (int i = 0; i < 100; ++i) {
            lo_message_add_float(message, static_cast<float>(i));
        }
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_string_short(benchmark::State& state) {
    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_add_string(message, "test");
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_string_long(benchmark::State& state) {
    std::array<char, 1024> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_add_string(message, dolorem);
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_blob_small(benchmark::State& state) {
    lo_message blobMessage = lo_message_new();
    lo_message_add_int32(blobMessage, 23);
    lo_message_add_float(blobMessage, 14.0f);
    std::array<char, 64> blobBuffer;
    size_t blobMessageSize = 0;
    lo_message_serialise(blobMessage, "blobMessage", blobBuffer.data(), &blobMessageSize);
    lo_message_free(blobMessage);

    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_blob blob = lo_blob_new(static_cast<int32_t>(blobMessageSize), blobBuffer.data());
        lo_message message = lo_message_new();
        lo_message_add_blob(message, blob);
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
        lo_blob_free(blob);
    }
}

static void BM_liblo_serialize_blob_medium(benchmark::State& state) {
    lo_message blobMessage = lo_message_new();
    for (int i = 0; i < 100; ++i) {
        lo_message_add_int32(blobMessage, i);
    }
    std::array<char, 1024> blobBuffer;
    size_t blobMessageSize = 0;
    lo_message_serialise(blobMessage, "blobMessage", blobBuffer.data(), &blobMessageSize);
    lo_message_free(blobMessage);

    std::array<char, 1024> buffer;
    for (auto _ : state) {
        lo_blob blob = lo_blob_new(static_cast<int32_t>(blobMessageSize), blobBuffer.data());
        lo_message message = lo_message_new();
        lo_message_add_blob(message, blob);
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
        lo_blob_free(blob);
    }
}

static void BM_liblo_serialize_blob_large(benchmark::State& state) {
    std::array<char, 2048> blobBuffer;
    for (int i = 0; i < 2048; ++i) {
        blobBuffer[i] = static_cast<char>(i);
    }

    std::array<char, 4096> buffer;
    for (auto _ : state) {
        lo_blob blob = lo_blob_new(blobBuffer.size(), blobBuffer.data());
        lo_message message = lo_message_new();
        lo_message_add_blob(message, blob);
        lo_message_serialise(message, "serialize", buffer.data(), nullptr);
        lo_message_free(message);
        lo_blob_free(blob);
    }
}

static void BM_liblo_deserialize_empty(benchmark::State& state) {
    std::array<char, 64> buffer;
    lo_message serialMessage = lo_message_new();
    size_t messageSize = 0;
    lo_message_serialise(serialMessage, "serialize", buffer.data(), &messageSize);
    lo_message_free(serialMessage);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_int32_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    lo_message serialMessage = lo_message_new();
    lo_message_add_int32(serialMessage, 0);
    size_t messageSize = 0;
    lo_message_serialise(serialMessage, "serialize", buffer.data(), &messageSize);
    lo_message_free(serialMessage);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_int32_series(benchmark::State& state) {
    std::array<char, 1024> buffer;
    lo_message serialMessage = lo_message_new();
    for (int i = 0; i < 100; ++i) {
        lo_message_add_int32(serialMessage, i);
    }
    size_t messageSize = 0;
    lo_message_serialise(serialMessage, "serialize", buffer.data(), &messageSize);
    lo_message_free(serialMessage);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_float_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    lo_message serialMessage = lo_message_new();
    lo_message_add_float(serialMessage, 0.0f);
    size_t messageSize = 0;
    lo_message_serialise(serialMessage, "serialize", buffer.data(), &messageSize);
    lo_message_free(serialMessage);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_float_series(benchmark::State& state) {
    std::array<char, 1024> buffer;
    lo_message serialMessage = lo_message_new();
    for (int i = 0; i < 100; ++i) {
        lo_message_add_float(serialMessage, static_cast<float>(i));
    }
    size_t messageSize = 0;
    lo_message_serialise(serialMessage, "serialize", buffer.data(), &messageSize);
    lo_message_free(serialMessage);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_string_short(benchmark::State& state) {
    std::array<char, 64> buffer;
    lo_message serialMessage = lo_message_new();
    lo_message_add_string(serialMessage, "test");
    size_t messageSize = 0;
    lo_message_serialise(serialMessage, "serialize", buffer.data(), &messageSize);
    lo_message_free(serialMessage);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_string_long(benchmark::State& state) {
    std::array<char, 1024> buffer;
    lo_message serialMessage = lo_message_new();
    lo_message_add_string(serialMessage, dolorem);
    size_t messageSize = 0;
    lo_message_serialise(serialMessage, "serialize", buffer.data(), &messageSize);
    lo_message_free(serialMessage);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_blob_small(benchmark::State& state) {
    lo_message blobMessage = lo_message_new();
    lo_message_add_int32(blobMessage, 23);
    lo_message_add_float(blobMessage, 14.0f);
    std::array<char, 64> blobBuffer;
    size_t blobMessageSize = 0;
    lo_message_serialise(blobMessage, "blobMessage", blobBuffer.data(), &blobMessageSize);
    lo_message_free(blobMessage);

    std::array<char, 64> buffer;
    lo_blob blob = lo_blob_new(static_cast<int32_t>(blobMessageSize), blobBuffer.data());
    lo_message message = lo_message_new();
    lo_message_add_blob(message, blob);
    size_t messageSize = 0;
    lo_message_serialise(message, "serialize", buffer.data(), &messageSize);
    lo_message_free(message);
    lo_blob_free(blob);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_blob_medium(benchmark::State& state) {
    lo_message blobMessage = lo_message_new();
    for (int i = 0; i < 100; ++i) {
        lo_message_add_int32(blobMessage, i);
    }
    std::array<char, 1024> blobBuffer;
    size_t blobMessageSize = 0;
    lo_message_serialise(blobMessage, "blobMessage", blobBuffer.data(), &blobMessageSize);
    lo_message_free(blobMessage);

    std::array<char, 1024> buffer;
    lo_blob blob = lo_blob_new(static_cast<int32_t>(blobMessageSize), blobBuffer.data());
    lo_message message = lo_message_new();
    lo_message_add_blob(message, blob);
    size_t messageSize = 0;
    lo_message_serialise(message, "serialize", buffer.data(), &messageSize);
    lo_message_free(message);
    lo_blob_free(blob);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_deserialize_blob_large(benchmark::State& state) {
    std::array<char, 2048> blobBuffer;
    for (int i = 0; i < 2048; ++i) {
        blobBuffer[i] = static_cast<char>(i);
    }

    std::array<char, 4096> buffer;
    lo_blob blob = lo_blob_new(blobBuffer.size(), blobBuffer.data());
    lo_message message = lo_message_new();
    lo_message_add_blob(message, blob);
    size_t messageSize = 0;
    lo_message_serialise(message, "serialize", buffer.data(), &messageSize);
    lo_message_free(message);
    lo_blob_free(blob);

    for (auto _ : state) {
        lo_message message = lo_message_deserialise(buffer.data(), messageSize, nullptr);
        lo_message_free(message);
    }
}

static void BM_oscpack_serialize_empty(benchmark::State& state) {
    std::array<char, 64> buffer;

    for (auto _ : state) {
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_int32_zero(benchmark::State& state) {
    std::array<char, 1024> buffer;

    for (auto _ : state) {
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        p << 0;
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_int32_series(benchmark::State& state) {
    std::array<char, 1024> buffer;

    for (auto _ : state) {
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        for (int i = 0; i < 100; ++i) {
            p << i;
        }
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_float_zero(benchmark::State& state) {
    std::array<char, 64> buffer;

    for (auto _ : state) {
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        p << 0.0f;
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_float_series(benchmark::State& state) {
    std::array<char, 1024> buffer;

    for (auto _ : state) {
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        for (int i = 0; i < 100; ++i) {
            p << static_cast<float>(i);
        }
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_string_short(benchmark::State& state) {
    std::array<char, 1024> buffer;

    for (auto _ : state) {
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        p << "test";
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_string_long(benchmark::State& state) {
    std::array<char, 1024> buffer;

    for (auto _ : state) {
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        p << dolorem;
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_blob_small(benchmark::State& state) {
    std::array<char, 64> blobBuffer;
    osc::OutboundPacketStream blobMessage(blobBuffer.data(), blobBuffer.size());
    blobMessage << osc::BeginMessage("blobMessage") << 23 << 14.0f << osc::EndMessage;

    std::array<char, 64> buffer;
    for (auto _ : state) {
        osc::Blob blob(blobMessage.Data(), blobMessage.Size());
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        p << blob;
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_blob_medium(benchmark::State& state) {
    std::array<char, 1024> blobBuffer;
    osc::OutboundPacketStream blobMessage(blobBuffer.data(), blobBuffer.size());
    blobMessage << osc::BeginMessage("blobMessage");
    for (int i = 0; i < 100; ++i) {
        blobMessage << i;
    }
    blobMessage << osc::EndMessage;

    std::array<char, 1024> buffer;
    for (auto _ : state) {
        osc::Blob blob(blobMessage.Data(), blobMessage.Size());
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        p << blob;
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_serialize_blob_large(benchmark::State& state) {
    std::array<char, 2048> blobBuffer;
    for (int i = 0; i < 2048; ++i) {
        blobBuffer[i] = static_cast<char>(i);
    }

    std::array<char, 4096> buffer;
    for (auto _ : state) {
        osc::Blob blob(blobBuffer.data(), blobBuffer.size());
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("serialize");
        p << blob;
        p << osc::EndMessage;
        if (p.Data() != buffer.data()) {
            state.SkipWithError("data mismatch!");
        }
    }
}

static void BM_oscpack_deserialize_empty(benchmark::State& state) {
    std::array<char, 64> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize") << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_int32_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    p << 0;
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_int32_series(benchmark::State& state) {
    std::array<char, 1024> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    for (int i = 0; i < 100; ++i) {
        p << i;
    }
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_float_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    p << 0.0;
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_float_series(benchmark::State& state) {
    std::array<char, 1024> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    for (int i = 0; i < 100; ++i) {
        p << static_cast<float>(i);
    }
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_string_short(benchmark::State& state) {
    std::array<char, 64> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    p << "test";
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_string_long(benchmark::State& state) {
    std::array<char, 1024> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    p << dolorem;
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_blob_small(benchmark::State& state) {
    std::array<char, 64> blobBuffer;
    osc::OutboundPacketStream blobMessage(blobBuffer.data(), blobBuffer.size());
    blobMessage << osc::BeginMessage("blobMessage") << 23 << 14.0f << osc::EndMessage;
    osc::Blob blob(blobMessage.Data(), blobMessage.Size());
    std::array<char, 64> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    p << blob;
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_blob_medium(benchmark::State& state) {
    std::array<char, 1024> blobBuffer;
    osc::OutboundPacketStream blobMessage(blobBuffer.data(), blobBuffer.size());
    blobMessage << osc::BeginMessage("blobMessage");
    for (int i = 0; i < 100; ++i) {
        blobMessage << i;
    }
    blobMessage << osc::EndMessage;
    std::array<char, 1024> buffer;
    osc::Blob blob(blobMessage.Data(), blobMessage.Size());
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    p << blob;
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpack_deserialize_blob_large(benchmark::State& state) {
    std::array<char, 2048> blobBuffer;
    for (int i = 0; i < 2048; ++i) {
        blobBuffer[i] = static_cast<char>(i);
    }
    osc::Blob blob(blobBuffer.data(), blobBuffer.size());
    std::array<char, 4096> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("serialize");
    p << blob;
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

BENCHMARK(BM_liblo_serialize_empty);
BENCHMARK(BM_liblo_serialize_int32_zero);
BENCHMARK(BM_liblo_serialize_int32_series);
BENCHMARK(BM_liblo_serialize_float_zero);
BENCHMARK(BM_liblo_serialize_float_series);
BENCHMARK(BM_liblo_serialize_string_short);
BENCHMARK(BM_liblo_serialize_string_long);
BENCHMARK(BM_liblo_serialize_blob_small);
BENCHMARK(BM_liblo_serialize_blob_medium);
BENCHMARK(BM_liblo_serialize_blob_large);

BENCHMARK(BM_liblo_deserialize_empty);
BENCHMARK(BM_liblo_deserialize_int32_zero);
BENCHMARK(BM_liblo_deserialize_int32_series);
BENCHMARK(BM_liblo_deserialize_float_zero);
BENCHMARK(BM_liblo_deserialize_float_series);
BENCHMARK(BM_liblo_deserialize_string_short);
BENCHMARK(BM_liblo_deserialize_string_long);
BENCHMARK(BM_liblo_deserialize_blob_small);
BENCHMARK(BM_liblo_deserialize_blob_medium);
BENCHMARK(BM_liblo_deserialize_blob_large);

BENCHMARK(BM_oscpack_serialize_empty);
BENCHMARK(BM_oscpack_serialize_int32_zero);
BENCHMARK(BM_oscpack_serialize_int32_series);
BENCHMARK(BM_oscpack_serialize_float_zero);
BENCHMARK(BM_oscpack_serialize_float_series);
BENCHMARK(BM_oscpack_serialize_string_short);
BENCHMARK(BM_oscpack_serialize_string_long);
BENCHMARK(BM_oscpack_serialize_blob_small);
BENCHMARK(BM_oscpack_serialize_blob_medium);
BENCHMARK(BM_oscpack_serialize_blob_large);

BENCHMARK(BM_oscpack_deserialize_empty);
BENCHMARK(BM_oscpack_deserialize_int32_zero);
BENCHMARK(BM_oscpack_deserialize_int32_series);
BENCHMARK(BM_oscpack_deserialize_float_zero);
BENCHMARK(BM_oscpack_deserialize_float_series);
BENCHMARK(BM_oscpack_deserialize_string_short);
BENCHMARK(BM_oscpack_deserialize_string_long);
BENCHMARK(BM_oscpack_deserialize_blob_small);
BENCHMARK(BM_oscpack_deserialize_blob_medium);
BENCHMARK(BM_oscpack_deserialize_blob_large);

BENCHMARK_MAIN();
