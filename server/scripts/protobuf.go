package main

import (
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
)

func main() {
	files, err := filepath.Glob("../proto/*.proto")
	if err != nil {
		log.Fatalf("Failed to glob proto files: %v", err)
	}

	if len(files) == 0 {
		fmt.Println("No proto files found.")
		return
	}

	if err := os.MkdirAll("internal/gen/protocol", 0755); err != nil {
		log.Fatalf("Failed to create output directory: %v", err)
	}

	args := []string{
		"--proto_path=../proto",
		"--go_out=internal/gen/protocol",
		"--go_opt=paths=source_relative",
	}
	args = append(args, files...)

	cmd := exec.Command("protoc", args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Run(); err != nil {
		log.Fatalf("protoc failed: %v", err)
	}

	fmt.Println("Successfully generated proto files.")
}
