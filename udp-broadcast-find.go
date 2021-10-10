package main

import (
	"fmt"
	"log"
	"net"
)

func main() {
	pc, err := net.ListenPacket("udp4", ":8804")
	if err != nil {
		log.Fatalf("could not start UDP listener, %v", err)
	}
	defer pc.Close()

	log.Printf("listening on port 8804\n")
	for {
		buf := make([]byte, 1024)
		n, addr, err := pc.ReadFrom(buf)
		if err != nil {
			log.Printf("could not read packet, %v\n", err)
			continue
		}

		fmt.Printf("%s: %s\n", addr, buf[:n])
	}
}
