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

#ifndef BASE64_H
#define BASE64_H

/// Encode input string to base64.
/// Returns a pointer to the encoded string that should be freed after use.
char* base64m_encode(const char* input);
/// Decode base64 encoded string.
/// Returns a pointer to the decoded string that should be freed after use.
char* base64m_decode(const char* input);

#endif
