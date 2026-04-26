// glitch.cpp - image corruption tool for making glitch art
//
// Usage: glitch <input> <output> [level 1-10]
//
// Picks a corruption strategy based on the file's magic bytes:
//   BMP  - bit-flip pixel data after the pixel-array offset
//   PNG  - mutate bytes inside IDAT chunks (CRC will be broken; many viewers still render)
//   JPEG - mutate scan data between SOS and EOI, skipping stuffed FF 00 sequences
//   GIF  - mutate LZW data inside image-block sub-blocks
//   else - generic mid-file byte mangling
//
// Corruption level (1=subtle .. 10=destroyed) controls how many bytes are touched
// and which mix of bit flips / byte swaps / random replacements is used.

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using Bytes = std::vector<uint8_t>;

enum class Format { BMP, PNG, JPEG, GIF, UNKNOWN };

static Format detect(const Bytes& b) {
    if (b.size() >= 2 && b[0] == 'B' && b[1] == 'M') return Format::BMP;
    if (b.size() >= 8 && b[0] == 0x89 && b[1] == 'P' && b[2] == 'N' && b[3] == 'G') return Format::PNG;
    if (b.size() >= 3 && b[0] == 0xFF && b[1] == 0xD8 && b[2] == 0xFF) return Format::JPEG;
    if (b.size() >= 6 && b[0] == 'G' && b[1] == 'I' && b[2] == 'F') return Format::GIF;
    return Format::UNKNOWN;
}

static const char* name(Format f) {
    switch (f) {
        case Format::BMP:  return "BMP";
        case Format::PNG:  return "PNG";
        case Format::JPEG: return "JPEG";
        case Format::GIF:  return "GIF";
        default:           return "unknown";
    }
}

// Mutate a single byte using a strategy chosen from the level.
// Higher levels favour more destructive replacements over single-bit flips.
static uint8_t mutate(uint8_t v, int level, std::mt19937& rng) {
    std::uniform_int_distribution<int> pick(0, 9);
    int roll = pick(rng);
    if (roll < std::max(1, 10 - level)) {
        // bit flip: subtle
        return v ^ (1u << (rng() & 7));
    } else if (roll < 7) {
        // byte rotate: shifts colours weirdly
        int s = 1 + (rng() % 7);
        return static_cast<uint8_t>((v << s) | (v >> (8 - s)));
    } else {
        // full random replacement: harshest
        return static_cast<uint8_t>(rng() & 0xFF);
    }
}

// Pick `count` distinct positions in [lo, hi) and mutate each.
static void scatter(Bytes& b, size_t lo, size_t hi, size_t count, int level, std::mt19937& rng) {
    if (hi <= lo) return;
    size_t span = hi - lo;
    count = std::min(count, span);
    std::uniform_int_distribution<size_t> at(lo, hi - 1);
    for (size_t i = 0; i < count; ++i) {
        size_t p = at(rng);
        b[p] = mutate(b[p], level, rng);
    }
}

// Density: roughly what fraction of the corruptible region we touch.
static double density(int level) {
    // level 1 -> 0.05%, level 10 -> 25%
    static const double table[11] = {
        0.0,   0.0005, 0.001, 0.003, 0.008, 0.02,
        0.04,  0.07,   0.12,  0.18,  0.25
    };
    return table[level];
}

static void glitch_bmp(Bytes& b, int level, std::mt19937& rng) {
    // Pixel-array offset lives at bytes 10..13 of the BMP file header.
    if (b.size() < 14) return;
    uint32_t off = uint32_t(b[10]) | (uint32_t(b[11]) << 8)
                 | (uint32_t(b[12]) << 16) | (uint32_t(b[13]) << 24);
    if (off >= b.size()) off = 54; // fall back to a typical 54-byte header
    size_t span = b.size() - off;
    size_t count = static_cast<size_t>(span * density(level));
    scatter(b, off, b.size(), count, level, rng);
}

static void glitch_png(Bytes& b, int level, std::mt19937& rng) {
    // Walk chunks: 4-byte length, 4-byte type, length-byte data, 4-byte CRC.
    size_t i = 8; // skip PNG signature
    size_t total_idat = 0;
    std::vector<std::pair<size_t,size_t>> idat_ranges;
    while (i + 12 <= b.size()) {
        uint32_t len = (uint32_t(b[i]) << 24) | (uint32_t(b[i+1]) << 16)
                     | (uint32_t(b[i+2]) << 8) | uint32_t(b[i+3]);
        if (i + 8 + len + 4 > b.size()) break;
        bool is_idat = b[i+4] == 'I' && b[i+5] == 'D' && b[i+6] == 'A' && b[i+7] == 'T';
        if (is_idat && len > 2) {
            // Skip the first two bytes of the first IDAT (zlib header) so the stream still parses.
            size_t data_start = i + 8 + (idat_ranges.empty() ? 2 : 0);
            size_t data_end   = i + 8 + len;
            idat_ranges.emplace_back(data_start, data_end);
            total_idat += data_end - data_start;
        }
        i += 8 + len + 4;
    }
    if (idat_ranges.empty()) {
        // No IDAT? fall back to generic mid-file mangling.
        scatter(b, b.size() / 4, b.size(), static_cast<size_t>(b.size() * density(level) * 0.5), level, rng);
        return;
    }
    size_t count = static_cast<size_t>(total_idat * density(level));
    // Distribute the budget across IDATs proportional to their size.
    for (auto [lo, hi] : idat_ranges) {
        double share = double(hi - lo) / double(total_idat);
        scatter(b, lo, hi, static_cast<size_t>(count * share), level, rng);
    }
}

static void glitch_jpeg(Bytes& b, int level, std::mt19937& rng) {
    // Find SOS (FF DA): scan data starts after SOS's segment payload.
    size_t i = 2; // skip SOI
    size_t scan_start = 0;
    while (i + 1 < b.size()) {
        if (b[i] != 0xFF) { ++i; continue; }
        uint8_t marker = b[i+1];
        if (marker == 0x00 || marker == 0xFF) { ++i; continue; }
        if (marker == 0xD9) break; // EOI
        // Markers without payload: RSTn (D0..D7) and SOI (D8)
        if (marker >= 0xD0 && marker <= 0xD8) { i += 2; continue; }
        if (i + 4 > b.size()) break;
        uint16_t seg_len = (uint16_t(b[i+2]) << 8) | b[i+3];
        size_t next = i + 2 + seg_len;
        if (marker == 0xDA) { scan_start = next; break; }
        i = next;
    }
    if (scan_start == 0 || scan_start >= b.size()) {
        scatter(b, b.size() / 3, b.size(), static_cast<size_t>(b.size() * density(level) * 0.3), level, rng);
        return;
    }
    // Trim trailing EOI so we don't touch it.
    size_t scan_end = b.size();
    if (scan_end >= 2 && b[scan_end-2] == 0xFF && b[scan_end-1] == 0xD9) scan_end -= 2;
    if (scan_end <= scan_start) return;

    // Build a list of safe positions (skip the second byte of stuffed FF 00 sequences and any markers).
    std::vector<size_t> safe;
    safe.reserve((scan_end - scan_start) / 2);
    for (size_t k = scan_start; k < scan_end; ++k) {
        if (k > 0 && b[k-1] == 0xFF) continue; // would form/disturb a marker
        if (b[k] == 0xFF) continue;            // don't turn a normal byte into a marker prefix
        safe.push_back(k);
    }
    if (safe.empty()) return;
    size_t count = static_cast<size_t>(safe.size() * density(level));
    std::uniform_int_distribution<size_t> at(0, safe.size() - 1);
    for (size_t n = 0; n < count; ++n) {
        size_t p = safe[at(rng)];
        uint8_t v = mutate(b[p], level, rng);
        if (v == 0xFF) v = 0xFE; // never introduce a stray marker prefix
        b[p] = v;
    }
}

static void glitch_gif(Bytes& b, int level, std::mt19937& rng) {
    // Skip header (6) + logical screen descriptor (7) + optional global colour table.
    if (b.size() < 13) return;
    size_t i = 13;
    uint8_t packed = b[10];
    if (packed & 0x80) {
        size_t gct = 3u * (1u << ((packed & 0x07) + 1));
        i += gct;
    }
    std::vector<std::pair<size_t,size_t>> data_ranges;
    size_t total = 0;
    while (i < b.size()) {
        uint8_t tag = b[i];
        if (tag == 0x3B) break;        // trailer
        if (tag == 0x21) {             // extension block
            if (i + 2 >= b.size()) break;
            i += 2;                    // introducer + label
            while (i < b.size() && b[i] != 0) { i += 1 + b[i]; }
            i += 1;                    // block terminator
        } else if (tag == 0x2C) {      // image descriptor
            if (i + 10 > b.size()) break;
            uint8_t img_packed = b[i+9];
            i += 10;
            if (img_packed & 0x80) {
                size_t lct = 3u * (1u << ((img_packed & 0x07) + 1));
                i += lct;
            }
            if (i >= b.size()) break;
            i += 1;                    // LZW minimum code size
            while (i < b.size() && b[i] != 0) {
                size_t sub_len = b[i];
                size_t lo = i + 1;
                size_t hi = std::min(lo + sub_len, b.size());
                if (hi > lo) {
                    data_ranges.emplace_back(lo, hi);
                    total += hi - lo;
                }
                i = hi;
            }
            i += 1;                    // block terminator
        } else {
            ++i;                       // unknown, walk forward
        }
    }
    if (data_ranges.empty()) {
        scatter(b, i / 2 + 1, b.size(), static_cast<size_t>(b.size() * density(level) * 0.3), level, rng);
        return;
    }
    size_t count = static_cast<size_t>(total * density(level));
    for (auto [lo, hi] : data_ranges) {
        double share = double(hi - lo) / double(total);
        scatter(b, lo, hi, static_cast<size_t>(count * share), level, rng);
    }
}

static void glitch_unknown(Bytes& b, int level, std::mt19937& rng) {
    // Leave the first 5% alone (most formats keep critical headers up front).
    size_t lo = b.size() / 20;
    size_t count = static_cast<size_t>((b.size() - lo) * density(level) * 0.5);
    scatter(b, lo, b.size(), count, level, rng);
}

int main(int argc, char** argv) {
    if (argc < 3 || argc > 4) {
        std::cerr << "usage: " << argv[0] << " <input> <output> [level 1-10]\n";
        return 1;
    }
    int level = 5;
    if (argc == 4) {
        try { level = std::stoi(argv[3]); }
        catch (...) { std::cerr << "level must be an integer 1-10\n"; return 1; }
        if (level < 1 || level > 10) { std::cerr << "level must be 1-10\n"; return 1; }
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in) { std::cerr << "cannot open input: " << argv[1] << "\n"; return 1; }
    Bytes buf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (buf.empty()) { std::cerr << "input is empty\n"; return 1; }

    Format fmt = detect(buf);
    std::random_device rd;
    std::mt19937 rng(rd());

    switch (fmt) {
        case Format::BMP:  glitch_bmp(buf, level, rng);  break;
        case Format::PNG:  glitch_png(buf, level, rng);  break;
        case Format::JPEG: glitch_jpeg(buf, level, rng); break;
        case Format::GIF:  glitch_gif(buf, level, rng);  break;
        default:           glitch_unknown(buf, level, rng); break;
    }

    std::ofstream out(argv[2], std::ios::binary);
    if (!out) { std::cerr << "cannot open output: " << argv[2] << "\n"; return 1; }
    out.write(reinterpret_cast<const char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
    if (!out) { std::cerr << "write failed\n"; return 1; }

    std::cout << "glitched " << name(fmt) << " (" << buf.size() << " bytes) at level " << level
              << " -> " << argv[2] << "\n";
    return 0;
}
