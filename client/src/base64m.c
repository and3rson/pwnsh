/*
 * Автор: Андрій Дунай
 * Цей файл є частиною проєкту "pwnsh"
 * 6 червня 2024
 * Ліцензія: WTFPL
 *
 *           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                   Version 2, December 2004
 *
 *  Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
 *
 *  Everyone is permitted to copy and distribute verbatim or modified
 *  copies of this license document, and changing it is allowed as long
 *  as the name is changed.
 *
 *           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#include <cstdlib>
#include <cstring>

#include "base64m.h"

// Use _ and - instead of + and / to make the string a valid hostname
const char base64m_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

char* base64m_encode(const char* input) {
    size_t input_length = strlen(input);
    size_t output_length = 4 * ((input_length + 2) / 3); // Determine output buffer size
    char* output = (char*)malloc(output_length + 1); // +1 for null terminator
    size_t i = 0, j = 0;

    while (i < input_length) {
        unsigned char byte1 = input[i++];
        unsigned char byte2 = (i < input_length) ? input[i++] : 0;
        unsigned char byte3 = (i < input_length) ? input[i++] : 0;

        output[j++] = base64m_chars[byte1 >> 2];
        output[j++] = base64m_chars[((byte1 & 0x03) << 4) | ((byte2 & 0xF0) >> 4)];
        output[j++] = (i < input_length + 2) ? base64m_chars[((byte2 & 0x0F) << 2) | ((byte3 & 0xC0) >> 6)] : '=';
        output[j++] = (i < input_length + 1) ? base64m_chars[byte3 & 0x3F] : '=';
    }
    output[j] = '\0'; // Null-terminate the string
    return output;
}

unsigned char base64m_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '_') return 62;
    if (c == '-') return 63;
    return 0; // Invalid character
}
char* base64m_decode(const char* input) {
    size_t input_length = strlen(input);
    size_t output_length = (input_length / 4) * 3;
    if (input[input_length - 1] == '=') output_length--;
    if (input[input_length - 2] == '=') output_length--;
    char* output = (char*)malloc(output_length + 1);
    size_t i = 0, j = 0;

    while (i < input_length) {
        unsigned char sextet1 = base64m_value(input[i++]);
        unsigned char sextet2 = base64m_value(input[i++]);
        unsigned char sextet3 = base64m_value(input[i++]);
        unsigned char sextet4 = base64m_value(input[i++]);

        unsigned char byte1 = (sextet1 << 2) | (sextet2 >> 4);
        unsigned char byte2 = (sextet2 << 4) | (sextet3 >> 2);
        unsigned char byte3 = (sextet3 << 6) | sextet4;

        output[j++] = byte1;
        if (j < output_length) output[j++] = byte2;
        if (j < output_length) output[j++] = byte3;
    }
    output[j] = '\0'; // Null-terminate the string
    return output;
}
