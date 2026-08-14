#include <cube/gfx/configurable_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>
#include <cube/core/math.hpp>

using namespace cube::gfx;
using namespace cube::core;
using namespace std::chrono;

namespace
{

constexpr int LETTER_HEIGHT = 13;
constexpr int LETTER_WIDTH = 8;
constexpr int NUM_CHARS = 95;
constexpr unsigned char FONT_ROWS[NUM_CHARS][13] = {
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // 32
    {0b00000000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00000000}, // 33 !
    {0b00000000, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // 34 "
    {0b00000000, 0b00000000, 0b00100100, 0b00100100, 0b01111110, 0b00100100, 0b00100100, 0b01111110, 0b00100100, 0b00100100, 0b00000000, 0b00000000, 0b00000000}, // 35 #
    {0b00000000, 0b00001000, 0b00111110, 0b01001001, 0b01001000, 0b00111110, 0b00001001, 0b00001001, 0b01001001, 0b00111110, 0b00001000, 0b00000000, 0b00000000}, // 36 $
    {0b00000000, 0b00110001, 0b01001010, 0b01001010, 0b00110100, 0b00001000, 0b00001000, 0b00010110, 0b00101001, 0b00101001, 0b01000110, 0b00000000, 0b00000000}, // 37 %
    {0b00000000, 0b00011100, 0b00100010, 0b00100010, 0b00010100, 0b00011000, 0b00101001, 0b01000101, 0b01000010, 0b01000110, 0b00111001, 0b00000000, 0b00000000}, // 38 &
    {0b00000000, 0b00001100, 0b00001100, 0b00001100, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // 39 '
    {0b00000000, 0b00000100, 0b00001000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00001000, 0b00000100, 0b00000000, 0b00000000}, // 40 (
    {0b00000000, 0b00100000, 0b00010000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00010000, 0b00100000, 0b00000000, 0b00000000}, // 41 )
    {0b00000000, 0b00000000, 0b00000000, 0b00001000, 0b01001001, 0b00101010, 0b00011100, 0b00101010, 0b01001001, 0b00001000, 0b00000000, 0b00000000, 0b00000000}, // 42 *
    {0b00000000, 0b00000000, 0b00000000, 0b00001000, 0b00001000, 0b00001000, 0b01111111, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000, 0b00000000}, // 43 +
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00010000, 0b00000000}, // 44 ,
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // 45 -
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00000000}, // 46 .
    {0b00000000, 0b00000001, 0b00000010, 0b00000010, 0b00000100, 0b00001000, 0b00001000, 0b00010000, 0b00010000, 0b00100000, 0b01000000, 0b00000000, 0b00000000}, // 47 /
    {0b00000000, 0b00111100, 0b01000010, 0b01000110, 0b01001010, 0b01010010, 0b01100010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 48 0
    {0b00000000, 0b00001000, 0b00011000, 0b00101000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00111110, 0b00000000, 0b00000000}, // 49 1
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b01111110, 0b00000000, 0b00000000}, // 50 2
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b00000010, 0b00011100, 0b00000010, 0b00000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 51 3
    {0b00000000, 0b00000100, 0b00001100, 0b00010100, 0b00100100, 0b01000100, 0b01000100, 0b01111110, 0b00000100, 0b00000100, 0b00000100, 0b00000000, 0b00000000}, // 52 4
    {0b00000000, 0b01111110, 0b01000000, 0b01000000, 0b01000000, 0b01111100, 0b00000010, 0b00000010, 0b00000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 53 5
    {0b00000000, 0b00111100, 0b01000010, 0b01000000, 0b01000000, 0b01111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 54 6
    {0b00000000, 0b01111110, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00000000, 0b00000000}, // 55 7
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 56 8
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111110, 0b00000010, 0b00000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 57 9
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00000000}, // 58 :
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00010000, 0b00000000}, // 59 ;
    {0b00000000, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000000, 0b00000000}, // 60 <
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111110, 0b00000000, 0b00000000, 0b00000000, 0b01111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // 61 =
    {0b00000000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b00000000, 0b00000000}, // 62 >
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b00000010, 0b00000100, 0b00001000, 0b00001000, 0b00000000, 0b00001000, 0b00001000, 0b00000000, 0b00000000}, // 63 ?
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01001110, 0b01010010, 0b01010110, 0b01001010, 0b01000000, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 64 @
    {0b00000000, 0b00011000, 0b00100100, 0b01000010, 0b01000010, 0b01000010, 0b01111110, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 65 A
    {0b00000000, 0b01111100, 0b01000010, 0b01000010, 0b01000010, 0b01111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01111100, 0b00000000, 0b00000000}, // 66 B
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 67 C
    {0b00000000, 0b01111000, 0b01000100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000100, 0b01111000, 0b00000000, 0b00000000}, // 68 D
    {0b00000000, 0b01111110, 0b01000000, 0b01000000, 0b01000000, 0b01111100, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01111110, 0b00000000, 0b00000000}, // 69 E
    {0b00000000, 0b01111110, 0b01000000, 0b01000000, 0b01000000, 0b01111100, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b00000000, 0b00000000}, // 70 F
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01000000, 0b01001110, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 71 G
    {0b00000000, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01111110, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 72 H
    {0b00000000, 0b00111110, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00111110, 0b00000000, 0b00000000}, // 73 I
    {0b00000000, 0b00011111, 0b00000100, 0b00000100, 0b00000100, 0b00000100, 0b00000100, 0b00000100, 0b01000100, 0b01000100, 0b00111000, 0b00000000, 0b00000000}, // 74 J
    {0b00000000, 0b01000010, 0b01000100, 0b01001000, 0b01010000, 0b01100000, 0b01100000, 0b01010000, 0b01001000, 0b01000100, 0b01000010, 0b00000000, 0b00000000}, // 75 K
    {0b00000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01111110, 0b00000000, 0b00000000}, // 76 L
    {0b00000000, 0b01000010, 0b01100110, 0b01011010, 0b01011010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 77 M
    {0b00000000, 0b01000010, 0b01100010, 0b01010010, 0b01001010, 0b01000110, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 78 N
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 79 O
    {0b00000000, 0b01111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01111100, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b00000000, 0b00000000}, // 80 P
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01011010, 0b01100110, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 81 Q
    {0b00000000, 0b01111100, 0b01000010, 0b01000010, 0b01000010, 0b01111100, 0b01001000, 0b01000100, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 82 R
    {0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01000000, 0b00111100, 0b00000010, 0b00000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 83 S
    {0b00000000, 0b01111111, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000}, // 84 T
    {0b00000000, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 85 U
    {0b00000000, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00100100, 0b00100100, 0b00100100, 0b00011000, 0b00011000, 0b00011000, 0b00000000, 0b00000000}, // 86 V
    {0b00000000, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01011010, 0b01011010, 0b01100110, 0b01100110, 0b01000010, 0b00000000, 0b00000000}, // 87 W
    {0b00000000, 0b01000010, 0b01000010, 0b00100100, 0b00100100, 0b00011000, 0b00011000, 0b00100100, 0b00100100, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 88 X
    {0b00000000, 0b01000001, 0b01000001, 0b00100010, 0b00100010, 0b00010100, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000}, // 89 Y
    {0b00000000, 0b01111110, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b01000000, 0b01000000, 0b01111110, 0b00000000, 0b00000000}, // 90 Z
    {0b00000000, 0b00011110, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00011110, 0b00000000, 0b00000000}, // 91 [
    {0b00000000, 0b01000000, 0b00100000, 0b00100000, 0b00010000, 0b00001000, 0b00001000, 0b00000100, 0b00000100, 0b00000010, 0b00000001, 0b00000000, 0b00000000}, // 92 backslash
    {0b00000000, 0b01111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b01111000, 0b00000000, 0b00000000}, // 93 ]
    {0b00000000, 0b00001000, 0b00010100, 0b00100010, 0b01000001, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // 94 ^
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111111, 0b00000000, 0b00000000}, // 95 _
    {0b00000000, 0b00010000, 0b00001000, 0b00000100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // 96 `
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111100, 0b00000010, 0b00111110, 0b01000010, 0b01000010, 0b01000110, 0b00111010, 0b00000000, 0b00000000}, // 97 a
    {0b00000000, 0b01000000, 0b01000000, 0b01000000, 0b01011100, 0b01100010, 0b01000010, 0b01000010, 0b01000010, 0b01100010, 0b01011100, 0b00000000, 0b00000000}, // 98 b
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111100, 0b01000010, 0b01000000, 0b01000000, 0b01000000, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 99 c
    {0b00000000, 0b00000010, 0b00000010, 0b00000010, 0b00111010, 0b01000110, 0b01000010, 0b01000010, 0b01000010, 0b01000110, 0b00111010, 0b00000000, 0b00000000}, // 100 d
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01111110, 0b01000000, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 101 e
    {0b00000000, 0b00001100, 0b00010010, 0b00010000, 0b01111100, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00000000, 0b00000000}, // 102 f
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111010, 0b01000110, 0b01000010, 0b01000110, 0b00111010, 0b00000010, 0b00111100, 0b00000000, 0b00000000}, // 103 g
    {0b00000000, 0b01000000, 0b01000000, 0b01000000, 0b01011100, 0b01100010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 104 h
    {0b00000000, 0b00001000, 0b00000000, 0b00000000, 0b00111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00111110, 0b00000000, 0b00000000}, // 105 i
    {0b00000000, 0b00000100, 0b00000000, 0b00000000, 0b00011100, 0b00000100, 0b00000100, 0b00000100, 0b00000100, 0b01000100, 0b00111000, 0b00000000, 0b00000000}, // 106 j
    {0b00000000, 0b01000000, 0b01000000, 0b01000000, 0b01000100, 0b01001000, 0b01010000, 0b01110000, 0b01001000, 0b01000100, 0b01000010, 0b00000000, 0b00000000}, // 107 k
    {0b00000000, 0b00111000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00111110, 0b00000000, 0b00000000}, // 108 l
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01110110, 0b01001001, 0b01001001, 0b01001001, 0b01001001, 0b01001001, 0b01001001, 0b00000000, 0b00000000}, // 109 m
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01011100, 0b01100010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 110 n
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 111 o
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01011100, 0b01100010, 0b01000010, 0b01000010, 0b01100010, 0b01011100, 0b01000000, 0b01000000, 0b00000000}, // 112 p
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111010, 0b01000110, 0b01000010, 0b01000010, 0b01000110, 0b00111010, 0b00000010, 0b00000010, 0b00000000}, // 113 q
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01001110, 0b01110000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b00000000, 0b00000000}, // 114 r
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111100, 0b01000010, 0b00110000, 0b00001100, 0b00000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 115 s
    {0b00000000, 0b00010000, 0b00010000, 0b00010000, 0b01111100, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010010, 0b00001100, 0b00000000, 0b00000000}, // 116 t
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000110, 0b00111010, 0b00000000, 0b00000000}, // 117 u
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01000010, 0b01000010, 0b01000010, 0b00100100, 0b00100100, 0b00011000, 0b00011000, 0b00000000, 0b00000000}, // 118 v
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01000001, 0b01000001, 0b01001001, 0b01001001, 0b01001001, 0b01001001, 0b00110110, 0b00000000, 0b00000000}, // 119 w
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01000010, 0b01000010, 0b00100100, 0b00011000, 0b00100100, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // 120 x
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01000010, 0b01000010, 0b01000010, 0b01000110, 0b00111010, 0b00000010, 0b00111100, 0b00000000, 0b00000000}, // 121 y
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01111110, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01111110, 0b00000000, 0b00000000}, // 122 z
    {0b00000000, 0b00000110, 0b00001000, 0b00001000, 0b00001000, 0b00110000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000110, 0b00000000, 0b00000000}, // 123 {
    {0b00000000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000}, // 124 |
    {0b00000000, 0b01100000, 0b00010000, 0b00010000, 0b00010000, 0b00001100, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b01100000, 0b00000000, 0b00000000}, // 125 }
    {0b00000000, 0b00000000, 0b00110010, 0b01001100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // 126 ~
};

struct glyph { unsigned char const * rows; int width; };

glyph get_glyph(char c)
{
    if (c >= 32 && c <= 126)
        return {FONT_ROWS[c - 32], LETTER_WIDTH};
    return {FONT_ROWS['?' - 32], LETTER_WIDTH};
}

struct sparkle
{
    voxel_t pos;
    double life;
    double fade_rate;
    double hue;
    int radius;
    double wobble_offset;
};

struct burst_particle
{
    glm::dvec3 pos;
    glm::dvec3 vel;
    double life;
    double fade_rate;
    double hue;
};

struct firework_burst
{
    glm::dvec3 center;
    std::vector<burst_particle> particles;
    bool done;

    void explode();
    void update(std::chrono::milliseconds dt);
    void paint(painter & p) const;
};

struct flying_text :
    configurable_animation
{
    flying_text(engine_context & context);

    animation_trait traits() const override { return animation_trait::transition; }
    void state_changed(animation_state state) override;
    void scene_tick(milliseconds dt) override;
    void paint(graphics_device & device) override;
    std::unordered_map<std::string, property_value_t> extra_properties() const override;

    void next_char();
    void draw_letter(painter & p, int cx, int cy, int cz, double angle, double zoom, double hue) const;
    void draw_sparkles(painter & p) const;
    void update_sparkles();

    void build_text();

    int char_idx_;
    int time_ms_;
    int phase_ms_;

    enum class phase { fly_in, rotate, fly_out };
    phase phase_;

    milliseconds fly_in_dur_;
    milliseconds rotate_dur_;
    milliseconds fly_out_dur_;

    std::vector<sparkle> sparkles_;
    double sparkle_mood_;
    std::vector<firework_burst> bursts_;
    int burst_cooldown_;

    std::vector<glyph> glyphs_;
};

animation_publisher<flying_text> const publisher;

constexpr milliseconds default_fly_in_duration{500ms};
constexpr milliseconds default_rotation_duration{400ms};
constexpr milliseconds default_fly_out_duration{500ms};
constexpr unsigned int default_number_of_sparkles{static_cast<unsigned int>(std::ceil(150.0 * cube::cube_size_1d / 64.0))};
constexpr double default_motion_blur{0.93};

flying_text::flying_text(engine_context & context) :
    configurable_animation(context),
    char_idx_(-1), time_ms_(0), phase_ms_(0), sparkle_mood_(1.0)
{ }

void flying_text::state_changed(animation_state state)
{
    switch (state) {
        case animation_state::running: {
            fly_in_dur_ = read_property<milliseconds>("fly_in_time_ms");
            rotate_dur_ = read_property<milliseconds>("rotate_time_ms");
            fly_out_dur_ = read_property<milliseconds>("fly_out_time_ms");
            char_idx_ = -1;
            time_ms_ = 0;
            phase_ms_ = 0;
            bursts_.clear();
            burst_cooldown_ = 0;

            build_text();

            sparkles_.resize(read_property<unsigned int>("number_of_sparkles"));
            for (auto & s : sparkles_) {
                s.pos = random_voxel();
                s.life = randd();
                s.fade_rate = randd({0.003, 0.025});
                s.hue = randd() * 360.0;
                s.radius = (randd() < 0.3) ? rand(range{1, (cube::cube_size_1d >= 48 ? 2 : 1)}) : 0;
                s.wobble_offset = randd() * 2.0 * M_PI;
            }
            next_char();
            break;
        }
        default:;
    }
}

void flying_text::build_text()
{
    std::string text = read_property<std::string>("text");
    glyphs_.clear();
    for (char c : text)
        glyphs_.push_back(get_glyph(c));
}

void flying_text::next_char()
{
    char_idx_++;
    if (glyphs_.empty() || char_idx_ >= static_cast<int>(glyphs_.size()))
        char_idx_ = 0;
    phase_ = phase::fly_in;
    phase_ms_ = 0;
}

void flying_text::scene_tick(milliseconds dt)
{
    int d = static_cast<int>(dt.count());
    time_ms_ += d;
    phase_ms_ += d;

    switch (phase_) {
        case phase::fly_in:
            if (phase_ms_ >= static_cast<int>(fly_in_dur_.count()))
                phase_ = phase::rotate, phase_ms_ = 0;
            break;
        case phase::rotate:
            if (phase_ms_ >= static_cast<int>(rotate_dur_.count()))
                phase_ = phase::fly_out, phase_ms_ = 0;
            break;
        case phase::fly_out:
            if (phase_ms_ >= static_cast<int>(fly_out_dur_.count()))
                next_char();
            break;
    }

    update_sparkles();

    // Spawn firework bursts randomly
    burst_cooldown_ -= d;
    if (burst_cooldown_ <= 0) {
        burst_cooldown_ = static_cast<int>(rand(range{1500, 3500}));

        firework_burst burst;
        burst.center = {
            randd({0.0, static_cast<double>(cube::cube_axis_max_value)}),
            randd({0.0, static_cast<double>(cube::cube_axis_max_value)}),
            randd({0.0, static_cast<double>(cube::cube_axis_max_value)})
        };
        burst.done = true;
        burst.explode();
        bursts_.push_back(std::move(burst));

        bursts_.erase(std::remove_if(bursts_.begin(), bursts_.end(),
            [](auto const & b) { return b.done; }), bursts_.end());
    }

    for (auto & b : bursts_)
        b.update(dt);
}

void firework_burst::explode()
{
    done = false;
    unsigned int n = static_cast<unsigned int>(rand(range{40, 100}) * static_cast<double>(cube::cube_size_1d) / 32.0);
    particles.resize(n);

    for (auto & p : particles) {
        double theta = randd({0.0, 2.0 * M_PI});
        double phi = std::acos(2.0 * randd() - 1.0);
        double speed = randd({0.01, 0.03}) * map(static_cast<double>(cube::cube_size_1d), range{1.0, 64.0}, range{1.0, 2.0});

        p.pos = center;
        p.vel = {
            speed * std::sin(phi) * std::cos(theta),
            speed * std::sin(phi) * std::sin(theta),
            speed * std::cos(phi)
        };
        p.life = 1.0;
        p.fade_rate = randd({0.001, 0.01});
        p.hue = randd() * 360.0;
    }
}

void firework_burst::update(std::chrono::milliseconds dt)
{
    if (done) return;

    double d = static_cast<double>(dt.count());
    constexpr double gravity = -0.000001 * cube::cube_size_1d;

    bool all_dead = true;
    for (auto & p : particles) {
        p.vel.z += gravity * d;
        p.pos += p.vel * d;
        p.life -= p.fade_rate;
        if (p.life > 0.0)
            all_dead = false;
    }
    if (all_dead)
        done = true;
}

void firework_burst::paint(painter & p) const
{
    if (done) return;

    for (auto const & bp : particles) {
        if (bp.life <= 0.0) continue;
        voxel_t v{
            static_cast<int>(std::round(bp.pos.x)),
            static_cast<int>(std::round(bp.pos.y)),
            static_cast<int>(std::round(bp.pos.z))
        };

        double bright = bp.life * bp.life;
        p.set_color(hsv(bp.hue, 1.0, bright));
        int r = (bp.life > 0.5) ? 1 : 0;
        if (r > 0)
            p.sphere(v, r);
        else
            p.draw(v);
    }
}

void flying_text::update_sparkles()
{
    for (auto & s : sparkles_) {
        s.life -= s.fade_rate;
        if (s.life <= 0.0) {
            s.pos = random_voxel();
            s.life = 1.0;
            s.fade_rate = randd({0.003, 0.025});
            s.hue = randd() * 360.0;
            s.radius = (randd() < 0.3) ? rand(range{1, 2}) : 0;
            s.wobble_offset = randd() * 2.0 * M_PI;
        }
    }
    sparkle_mood_ = 0.2 + 0.8 * (0.5 + 0.5 * std::sin(static_cast<double>(time_ms_) * 0.003 + std::sin(static_cast<double>(time_ms_) * 0.007) * 0.5));
}

void flying_text::draw_letter(painter & p, int cx, int depth, int cz, double angle, double zoom, double hue) const
{
    auto const & g = glyphs_[char_idx_];
    double spacing = zoom * cube::cube_size_1d / 16.0;

    for (int row = 0; row < LETTER_HEIGHT; ++row) {
        uint8_t bits = g.rows[row];
        for (int col = 0; col < g.width; ++col) {
            if (!(bits & (1 << (7 - col))))
                continue;

            double lx = (col - g.width / 2.0) * spacing;
            double lz = (LETTER_HEIGHT / 2.0 - row) * spacing;

            // Rotate around Z axis (vertical): X and Y rotate, Z stays
            double ca = cos(angle), sa = sin(angle);
            int x = cx + static_cast<int>(std::round(lx * ca));
            int y = depth + static_cast<int>(std::round(lx * sa));
            int z = cz + static_cast<int>(std::round(lz));

            if (x < 0 || x > cube::cube_axis_max_value ||
                y < 0 || y > cube::cube_axis_max_value ||
                z < 0 || z > cube::cube_axis_max_value)
                continue;

            p.set_color(hsv(hue + static_cast<double>(col * 20 + row * 10), 1.0, 1.0));

            int r = std::max(0, static_cast<int>(zoom * cube::cube_size_1d / 25.0));
            p.sphere({x, y, z}, r);
        }
    }
}

void flying_text::draw_sparkles(painter & p) const
{
    for (auto const & s : sparkles_) {
        double bright = s.life * sparkle_mood_;
        if (bright > 0.02) {
            double hue = fmod(s.hue + static_cast<double>(time_ms_) * 0.08 + std::sin(s.wobble_offset + static_cast<double>(time_ms_) * 0.005) * 30.0, 360.0);
            p.set_color(hsv(hue, 0.9 + 0.1 * bright, bright));
            if (s.radius > 0)
                p.sphere(s.pos, s.radius);
            else
                p.draw(s.pos);
        }
    }
}

void flying_text::paint(graphics_device & device)
{
    painter p(device);

    draw_sparkles(p);

    for (auto const & b : bursts_)
        b.paint(p);

    int const cx = cube::cube_axis_max_value / 2;
    int const cz = cube::cube_axis_max_value / 2;

    switch (phase_) {
        case phase::fly_in: {
            double t = std::min(1.0, static_cast<double>(phase_ms_) / static_cast<double>(fly_in_dur_.count()));
            double eased = std::sin(0.5 * M_PI * t);
            int depth = static_cast<int>(map(eased, range{0.0, 1.0},
                range{static_cast<double>(cube::cube_axis_max_value), static_cast<double>(cube::cube_axis_max_value / 2)}));
            double zoom = map(t, range{0.0, 1.0}, range{0.3, 1.2});
            double hue = fmod(static_cast<double>(time_ms_) * 0.15, 360.0);
            draw_letter(p, cx, depth, cz, 0.0, zoom, hue);
            break;
        }
        case phase::rotate: {
            double t = std::min(1.0, static_cast<double>(phase_ms_) / static_cast<double>(rotate_dur_.count()));
            double angle = 2.0 * M_PI * t;
            int depth = cube::cube_axis_max_value / 2;
            double hue = fmod(static_cast<double>(time_ms_) * 0.3, 360.0);
            draw_letter(p, cx, depth, cz, angle, 1.2, hue);
            break;
        }
        case phase::fly_out: {
            double t = std::min(1.0, static_cast<double>(phase_ms_) / static_cast<double>(fly_out_dur_.count()));
            double eased = std::sin(0.5 * M_PI * t);
            int depth = static_cast<int>(map(eased, range{0.0, 1.0},
                range{static_cast<double>(cube::cube_axis_max_value / 2), -5.0}));
            double zoom = 1.2;
            double hue = fmod(static_cast<double>(time_ms_) * 0.2 + 180.0, 360.0);
            draw_letter(p, cx, depth, cz, 0.0, zoom, hue);
            break;
        }
    }
}

std::unordered_map<std::string, property_value_t> flying_text::extra_properties() const
{
    return {
        {"text", std::string("HELLO WORLD")},
        {"fly_in_time_ms", default_fly_in_duration},
        {"rotate_time_ms", default_rotation_duration},
        {"fly_out_time_ms", default_fly_out_duration},
        {"number_of_sparkles", default_number_of_sparkles},
        {"motion_blur", default_motion_blur},
    };
}

} // End of namespace
