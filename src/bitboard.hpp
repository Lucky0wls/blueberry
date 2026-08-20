#pragma once
#include <cstdint>

using Bitboard = std::uint64_t;

int popSq(Bitboard& bb);
int count(Bitboard bb);
bool isSet(const Bitboard& bb, int square);
void printBitboard(Bitboard bb);
