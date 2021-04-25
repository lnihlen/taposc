#include "benchmark/benchmark.h"
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"
#include "oscpkt.hh"
#include "oscpp/client.hpp"
#include "oscpp/server.hpp"

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
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_int32_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_add_int32(message, 0);
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
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
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_float_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_add_float(message, 0.0f);
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
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
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_string_short(benchmark::State& state) {
    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_add_string(message, "test");
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_string_long(benchmark::State& state) {
    std::array<char, 1024> buffer;
    for (auto _ : state) {
        lo_message message = lo_message_new();
        lo_message_add_string(message, dolorem);
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
        lo_message_free(message);
    }
}

static void BM_liblo_serialize_blob_small(benchmark::State& state) {
    lo_message blobMessage = lo_message_new();
    lo_message_add_int32(blobMessage, 23);
    lo_message_add_float(blobMessage, 14.0f);
    std::array<char, 64> blobBuffer;
    size_t blobMessageSize = 0;
    lo_message_serialise(blobMessage, "/blobMessage", blobBuffer.data(), &blobMessageSize);
    lo_message_free(blobMessage);

    std::array<char, 64> buffer;
    for (auto _ : state) {
        lo_blob blob = lo_blob_new(static_cast<int32_t>(blobMessageSize), blobBuffer.data());
        lo_message message = lo_message_new();
        lo_message_add_blob(message, blob);
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
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
    lo_message_serialise(blobMessage, "/blobMessage", blobBuffer.data(), &blobMessageSize);
    lo_message_free(blobMessage);

    std::array<char, 1024> buffer;
    for (auto _ : state) {
        lo_blob blob = lo_blob_new(static_cast<int32_t>(blobMessageSize), blobBuffer.data());
        lo_message message = lo_message_new();
        lo_message_add_blob(message, blob);
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
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
        lo_message_serialise(message, "/seriaize", buffer.data(), nullptr);
        lo_message_free(message);
        lo_blob_free(blob);
    }
}

static void BM_liblo_deserialize_empty(benchmark::State& state) {
    std::array<char, 64> buffer;
    lo_message serialMessage = lo_message_new();
    size_t messageSize = 0;
    lo_message_serialise(serialMessage, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(serialMessage, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(serialMessage, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(serialMessage, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(serialMessage, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(serialMessage, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(serialMessage, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(blobMessage, "/blobMessage", blobBuffer.data(), &blobMessageSize);
    lo_message_free(blobMessage);

    std::array<char, 64> buffer;
    lo_blob blob = lo_blob_new(static_cast<int32_t>(blobMessageSize), blobBuffer.data());
    lo_message message = lo_message_new();
    lo_message_add_blob(message, blob);
    size_t messageSize = 0;
    lo_message_serialise(message, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(blobMessage, "/blobMessage", blobBuffer.data(), &blobMessageSize);
    lo_message_free(blobMessage);

    std::array<char, 1024> buffer;
    lo_blob blob = lo_blob_new(static_cast<int32_t>(blobMessageSize), blobBuffer.data());
    lo_message message = lo_message_new();
    lo_message_add_blob(message, blob);
    size_t messageSize = 0;
    lo_message_serialise(message, "/seriaize", buffer.data(), &messageSize);
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
    lo_message_serialise(message, "/seriaize", buffer.data(), &messageSize);
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
        p << osc::BeginMessage("/seriaize");
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
        p << osc::BeginMessage("/seriaize");
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
        p << osc::BeginMessage("/seriaize");
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
        p << osc::BeginMessage("/seriaize");
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
        p << osc::BeginMessage("/seriaize");
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
        p << osc::BeginMessage("/seriaize");
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
        p << osc::BeginMessage("/seriaize");
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
    blobMessage << osc::BeginMessage("/blobMessage") << 23 << 14.0f << osc::EndMessage;

    std::array<char, 64> buffer;
    for (auto _ : state) {
        osc::Blob blob(blobMessage.Data(), blobMessage.Size());
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("/seriaize");
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
    blobMessage << osc::BeginMessage("/blobMessage");
    for (int i = 0; i < 100; ++i) {
        blobMessage << i;
    }
    blobMessage << osc::EndMessage;

    std::array<char, 1024> buffer;
    for (auto _ : state) {
        osc::Blob blob(blobMessage.Data(), blobMessage.Size());
        osc::OutboundPacketStream p(buffer.data(), buffer.size());
        p << osc::BeginMessage("/seriaize");
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
        p << osc::BeginMessage("/seriaize");
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
    p << osc::BeginMessage("/seriaize") << osc::EndMessage;

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
    p << osc::BeginMessage("/seriaize");
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
    p << osc::BeginMessage("/seriaize");
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
    p << osc::BeginMessage("/seriaize");
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
    p << osc::BeginMessage("/seriaize");
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
    p << osc::BeginMessage("/seriaize");
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
    p << osc::BeginMessage("/seriaize");
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
    blobMessage << osc::BeginMessage("/blobMessage") << 23 << 14.0f << osc::EndMessage;
    osc::Blob blob(blobMessage.Data(), blobMessage.Size());
    std::array<char, 64> buffer;
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("/seriaize");
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
    blobMessage << osc::BeginMessage("/blobMessage");
    for (int i = 0; i < 100; ++i) {
        blobMessage << i;
    }
    blobMessage << osc::EndMessage;
    std::array<char, 1024> buffer;
    osc::Blob blob(blobMessage.Data(), blobMessage.Size());
    osc::OutboundPacketStream p(buffer.data(), buffer.size());
    p << osc::BeginMessage("/seriaize");
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
    p << osc::BeginMessage("/seriaize");
    p << blob;
    p << osc::EndMessage;

    for (auto _ : state) {
        osc::ReceivedPacket message(p.Data(), p.Size());
        if (!message.IsMessage()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_serialize_empty(benchmark::State& state) {
    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_int32_zero(benchmark::State& state) {
    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        message.pushInt32(0);
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_int32_series(benchmark::State& state) {
    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        for (int i = 0; i < 100; ++i) {
            message.pushInt32(i);
        }
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_float_zero(benchmark::State& state) {
    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        message.pushFloat(0.0f);
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_float_series(benchmark::State& state) {
    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        for (int i = 0; i < 100; ++i) {
            message.pushFloat(static_cast<float>(i));
        }
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_string_short(benchmark::State& state) {
    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        message.pushStr("test");
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_string_long(benchmark::State& state) {
    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        message.pushStr(dolorem);
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_blob_small(benchmark::State& state) {
    oscpkt::Message blobMessage("/blobMessage");
    blobMessage.pushInt32(23);
    blobMessage.pushFloat(14.0f);
    oscpkt::PacketWriter blobPacket;
    blobPacket.addMessage(blobMessage);

    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        message.pushBlob(blobPacket.packetData(), blobPacket.packetSize());
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_blob_medium(benchmark::State& state) {
    oscpkt::Message blobMessage("/blobMessage");
    for (int i = 0; i < 100; ++i) {
        blobMessage.pushInt32(i);
    }
    oscpkt::PacketWriter blobPacket;
    blobPacket.addMessage(blobMessage);

    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        message.pushBlob(blobPacket.packetData(), blobPacket.packetSize());
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_serialize_blob_large(benchmark::State& state) {
    std::array<char, 2048> blobBuffer;
    for (int i = 0; i < 2048; ++i) {
        blobBuffer[i] = static_cast<char>(i);
    }

    for (auto _ : state) {
        oscpkt::Message message("/seriaize");
        message.pushBlob(blobBuffer.data(), blobBuffer.size());
        oscpkt::PacketWriter pw;
        pw.addMessage(message);
    }
}

static void BM_oscpkt_deserialize_empty(benchmark::State& state) {
    oscpkt::Message serialMessage("/seriaize");
    oscpkt::PacketWriter pw;
    pw.addMessage(serialMessage);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_int32_zero(benchmark::State& state) {
    oscpkt::Message serialMessage("/seriaize");
    serialMessage.pushInt32(0);
    oscpkt::PacketWriter pw;
    pw.addMessage(serialMessage);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_int32_series(benchmark::State& state) {
    oscpkt::Message serialMessage("/seriaize");
    for (int i = 0; i < 100; ++i) {
        serialMessage.pushInt32(i);
    }
    oscpkt::PacketWriter pw;
    pw.addMessage(serialMessage);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_float_zero(benchmark::State& state) {
    oscpkt::Message serialMessage("/seriaize");
    serialMessage.pushFloat(0.0f);
    oscpkt::PacketWriter pw;
    pw.addMessage(serialMessage);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_float_series(benchmark::State& state) {
    oscpkt::Message serialMessage("/seriaize");
    for (int i = 0; i < 100; ++i) {
        serialMessage.pushFloat(static_cast<float>(i));
    }
    oscpkt::PacketWriter pw;
    pw.addMessage(serialMessage);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_string_short(benchmark::State& state) {
    oscpkt::Message serialMessage("/seriaize");
    serialMessage.pushStr("test");
    oscpkt::PacketWriter pw;
    pw.addMessage(serialMessage);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_string_long(benchmark::State& state) {
    oscpkt::Message serialMessage("/seriaize");
    serialMessage.pushStr(dolorem);
    oscpkt::PacketWriter pw;
    pw.addMessage(serialMessage);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_blob_small(benchmark::State& state) {
    oscpkt::Message blobMessage("/blobMessage");
    blobMessage.pushInt32(23);
    blobMessage.pushFloat(14.0f);
    oscpkt::PacketWriter blobPacket;
    blobPacket.addMessage(blobMessage);
    oscpkt::Message message("/seriaize");
    message.pushBlob(blobPacket.packetData(), blobPacket.packetSize());
    oscpkt::PacketWriter pw;
    pw.addMessage(message);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_blob_medium(benchmark::State& state) {
    oscpkt::Message blobMessage("/blobMessage");
    for (int i = 0; i < 100; ++i) {
        blobMessage.pushInt32(i);
    }
    oscpkt::PacketWriter blobPacket;
    blobPacket.addMessage(blobMessage);
    oscpkt::Message message("/seriaize");
    message.pushBlob(blobPacket.packetData(), blobPacket.packetSize());
    oscpkt::PacketWriter pw;
    pw.addMessage(message);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpkt_deserialize_blob_large(benchmark::State& state) {
    std::array<char, 2048> blobBuffer;
    for (int i = 0; i < 2048; ++i) {
        blobBuffer[i] = static_cast<char>(i);
    }
    oscpkt::Message message("/seriaize");
    message.pushBlob(blobBuffer.data(), blobBuffer.size());
    oscpkt::PacketWriter pw;
    pw.addMessage(message);

    for (auto _ : state) {
        oscpkt::PacketReader reader(pw.packetData(), pw.packetSize());
        if (!reader.isOk()) {
            state.SkipWithError("not message!");
        }
    }
}

static void BM_oscpp_serialize_empty(benchmark::State& state) {
    std::array<char, 64> buffer;

    for (auto _ : state) {
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 0);
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_int32_zero(benchmark::State& state) {
    std::array<char, 64> buffer;

    for (auto _ : state) {
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 1);
        packet.int32(0);
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_int32_series(benchmark::State& state) {
    std::array<char, 1024> buffer;

    for (auto _ : state) {
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 100);
        for (int i = 0; i < 100; ++i) {
            packet.int32(i);
        }
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_float_zero(benchmark::State& state) {
    std::array<char, 64> buffer;

    for (auto _ : state) {
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 1);
        packet.float32(0.0f);
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_float_series(benchmark::State& state) {
    std::array<char, 1024> buffer;

    for (auto _ : state) {
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 100);
        for (int i = 0; i < 100; ++i) {
            packet.float32(static_cast<float>(i));
        }
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_string_short(benchmark::State& state) {
    std::array<char, 64> buffer;

    for (auto _ : state) {
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 1);
        packet.string("test");
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_string_long(benchmark::State& state) {
    std::array<char, 1024> buffer;

    for (auto _ : state) {
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 1);
        packet.string(dolorem);
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_blob_small(benchmark::State& state) {
    std::array<char, 64> blobBuffer;
    OSCPP::Client::Packet blobPacket(blobBuffer.data(), blobBuffer.size());
    blobPacket.openMessage("/blobMessage", 2).int32(23).float32(14.0f).closeMessage();

    std::array<char, 64> buffer;
    for (auto _ : state) {
        OSCPP::Blob blob(blobPacket.data(), blobPacket.size());
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 1);
        packet.blob(blob);
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_blob_medium(benchmark::State& state) {
    std::array<char, 1024> blobBuffer;
    OSCPP::Client::Packet blobPacket(blobBuffer.data(), blobBuffer.size());
    blobPacket.openMessage("/blobMessage", 100);
    for (int i = 0; i < 100; ++i) {
        blobPacket.int32(i);
    }
    blobPacket.closeMessage();

    std::array<char, 1024> buffer;
    for (auto _ : state) {
        OSCPP::Blob blob(blobPacket.data(), blobPacket.size());
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 1);
        packet.blob(blob);
        packet.closeMessage();
    }
}

static void BM_oscpp_serialize_blob_large(benchmark::State& state) {
    std::array<char, 2048> blobBuffer;
    for (int i = 0; i < 2048; ++i) {
        blobBuffer[i] = static_cast<char>(i);
    }

    std::array<char, 4096> buffer;
    for (auto _ : state) {
        OSCPP::Blob blob(blobBuffer.data(), blobBuffer.size());
        OSCPP::Client::Packet packet(buffer.data(), buffer.size());
        packet.openMessage("/serialize", 1);
        packet.blob(blob);
        packet.closeMessage();
    }
}

static void BM_oscpp_deserialize_empty(benchmark::State& state) {
    std::array<char, 64> buffer;
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 0);
    clientPacket.closeMessage();

    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_int32_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 1);
    clientPacket.int32(0);
    clientPacket.closeMessage();

    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_int32_series(benchmark::State& state) {
    std::array<char, 1024> buffer;
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 100);
    for (int i = 0; i < 100; ++i) {
        clientPacket.int32(i);
    }
    clientPacket.closeMessage();

    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_float_zero(benchmark::State& state) {
    std::array<char, 64> buffer;
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 1);
    clientPacket.float32(0.0f);
    clientPacket.closeMessage();

    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_float_series(benchmark::State& state) {
    std::array<char, 1024> buffer;
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 100);
    for (int i = 0; i < 100; ++i) {
        clientPacket.float32(static_cast<float>(i));
    }
    clientPacket.closeMessage();

    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_string_short(benchmark::State& state) {
    std::array<char, 64> buffer;
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 1);
    clientPacket.string("test");
    clientPacket.closeMessage();

    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_string_long(benchmark::State& state) {
    std::array<char, 1024> buffer;
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 1);
    clientPacket.string(dolorem);
    clientPacket.closeMessage();

    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_blob_small(benchmark::State& state) {
    std::array<char, 64> blobBuffer;
    OSCPP::Client::Packet blobPacket(blobBuffer.data(), blobBuffer.size());
    blobPacket.openMessage("/blobMessage", 2).int32(23).float32(14.0f).closeMessage();

    std::array<char, 64> buffer;
    OSCPP::Blob blob(blobPacket.data(), blobPacket.size());
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 1);
    clientPacket.blob(blob);
    clientPacket.closeMessage();
    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_blob_medium(benchmark::State& state) {
    std::array<char, 1024> blobBuffer;
    OSCPP::Client::Packet blobPacket(blobBuffer.data(), blobBuffer.size());
    blobPacket.openMessage("/blobMessage", 100);
    for (int i = 0; i < 100; ++i) {
        blobPacket.int32(i);
    }
    blobPacket.closeMessage();

    std::array<char, 1024> buffer;
    OSCPP::Blob blob(blobPacket.data(), blobPacket.size());
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 1);
    clientPacket.blob(blob);
    clientPacket.closeMessage();
    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
    }
}

static void BM_oscpp_deserialize_blob_large(benchmark::State& state) {
    std::array<char, 2048> blobBuffer;
    for (int i = 0; i < 2048; ++i) {
        blobBuffer[i] = static_cast<char>(i);
    }

    std::array<char, 4096> buffer;
    OSCPP::Blob blob(blobBuffer.data(), blobBuffer.size());
    OSCPP::Client::Packet clientPacket(buffer.data(), buffer.size());
    clientPacket.openMessage("/serialize", 1);
    clientPacket.blob(blob);
    clientPacket.closeMessage();
    for (auto _ : state) {
        OSCPP::Server::Packet packet(clientPacket.data(), clientPacket.size());
        OSCPP::Server::Message message(packet);
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

BENCHMARK(BM_oscpkt_serialize_empty);
BENCHMARK(BM_oscpkt_serialize_int32_zero);
BENCHMARK(BM_oscpkt_serialize_int32_series);
BENCHMARK(BM_oscpkt_serialize_float_zero);
BENCHMARK(BM_oscpkt_serialize_float_series);
BENCHMARK(BM_oscpkt_serialize_string_short);
BENCHMARK(BM_oscpkt_serialize_string_long);
BENCHMARK(BM_oscpkt_serialize_blob_small);
BENCHMARK(BM_oscpkt_serialize_blob_medium);
BENCHMARK(BM_oscpkt_serialize_blob_large);

BENCHMARK(BM_oscpkt_deserialize_empty);
BENCHMARK(BM_oscpkt_deserialize_int32_zero);
BENCHMARK(BM_oscpkt_deserialize_int32_series);
BENCHMARK(BM_oscpkt_deserialize_float_zero);
BENCHMARK(BM_oscpkt_deserialize_float_series);
BENCHMARK(BM_oscpkt_deserialize_string_short);
BENCHMARK(BM_oscpkt_deserialize_string_long);
BENCHMARK(BM_oscpkt_deserialize_blob_small);
BENCHMARK(BM_oscpkt_deserialize_blob_medium);
BENCHMARK(BM_oscpkt_deserialize_blob_large);

BENCHMARK(BM_oscpp_serialize_empty);
BENCHMARK(BM_oscpp_serialize_int32_zero);
BENCHMARK(BM_oscpp_serialize_int32_series);
BENCHMARK(BM_oscpp_serialize_float_zero);
BENCHMARK(BM_oscpp_serialize_float_series);
BENCHMARK(BM_oscpp_serialize_string_short);
BENCHMARK(BM_oscpp_serialize_string_long);
BENCHMARK(BM_oscpp_serialize_blob_small);
BENCHMARK(BM_oscpp_serialize_blob_medium);
BENCHMARK(BM_oscpp_serialize_blob_large);

BENCHMARK(BM_oscpp_deserialize_empty);
BENCHMARK(BM_oscpp_deserialize_int32_zero);
BENCHMARK(BM_oscpp_deserialize_int32_series);
BENCHMARK(BM_oscpp_deserialize_float_zero);
BENCHMARK(BM_oscpp_deserialize_float_series);
BENCHMARK(BM_oscpp_deserialize_string_short);
BENCHMARK(BM_oscpp_deserialize_string_long);
BENCHMARK(BM_oscpp_deserialize_blob_small);
BENCHMARK(BM_oscpp_deserialize_blob_medium);
BENCHMARK(BM_oscpp_deserialize_blob_large);

BENCHMARK_MAIN();
