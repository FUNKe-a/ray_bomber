import socket


HOST = "127.0.0.1"
PORT = 6769


MSG_GREETING = 0
MSG_PLAYER_JOINED = 1
MSG_PLAYER_LEFT = 2
MSG_MOVE_REQUESTED = 3
MSG_PLAYER_MOVED = 4


def build_message(message_type: int, body: bytes) -> bytes:
    if len(body) > 65535:
        raise ValueError("Message body is too large")

    return (
        bytes([message_type])
        + len(body).to_bytes(2, byteorder="big")
        + body
    )


def send_message(client: socket.socket, message_type: int, body: bytes):
    packet = build_message(message_type, body)

    print()
    print("Sending:")
    print(f"  Type:   {message_type}")
    print(f"  Length: {len(body)}")
    print(f"  Body:   {body.hex(' ')}")
    print(f"  Packet: {packet.hex(' ')}")

    client.sendall(packet)


def send_greeting(client: socket.socket, player_id: int):
    send_message(
        client,
        MSG_GREETING,
        bytes([player_id]),
    )


def send_player_joined(
    client: socket.socket,
    player_id: int,
    x: int,
    y: int,
):
    send_message(
        client,
        MSG_PLAYER_JOINED,
        bytes([player_id, x, y]),
    )


def send_player_left(client: socket.socket, player_id: int):
    send_message(
        client,
        MSG_PLAYER_LEFT,
        bytes([player_id]),
    )


def send_player_moved(
    client: socket.socket,
    player_id: int,
    x: int,
    y: int,
):
    send_message(
        client,
        MSG_PLAYER_MOVED,
        bytes([player_id, x, y]),
    )


def print_help():
    print()
    print("Commands:")
    print()
    print("  greeting <id>")
    print("  joined   <id> <x> <y>")
    print("  left     <id>")
    print("  moved    <id> <x> <y>")
    print()
    print("  help")
    print("  quit")
    print()


def main():
    print("======================================")
    print("      Ray Bomber Protocol Tester      ")
    print("======================================")
    print()
    print(f"Listening on {HOST}:{PORT}")
    print()
    print("Waiting for client...")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.setsockopt(
            socket.SOL_SOCKET,
            socket.SO_REUSEADDR,
            1,
        )

        server.bind((HOST, PORT))
        server.listen(1)

        client, address = server.accept()

        with client:
            print()
            print(f"Client connected from {address[0]}:{address[1]}")

            print_help()

            while True:
                try:
                    command = input("protocol> ").strip()

                    if not command:
                        continue

                    parts = command.split()
                    command_name = parts[0].lower()

                    if command_name == "quit":
                        break

                    if command_name == "help":
                        print_help()
                        continue

                    if command_name == "greeting":
                        if len(parts) != 2:
                            print("Usage: greeting <id>")
                            continue

                        player_id = int(parts[1])

                        send_greeting(
                            client,
                            player_id,
                        )

                    elif command_name == "joined":
                        if len(parts) != 4:
                            print("Usage: joined <id> <x> <y>")
                            continue

                        player_id = int(parts[1])
                        x = int(parts[2])
                        y = int(parts[3])

                        send_player_joined(
                            client,
                            player_id,
                            x,
                            y,
                        )

                    elif command_name == "left":
                        if len(parts) != 2:
                            print("Usage: left <id>")
                            continue

                        player_id = int(parts[1])

                        send_player_left(
                            client,
                            player_id,
                        )

                    elif command_name == "moved":
                        if len(parts) != 4:
                            print("Usage: moved <id> <x> <y>")
                            continue

                        player_id = int(parts[1])
                        x = int(parts[2])
                        y = int(parts[3])

                        send_player_moved(
                            client,
                            player_id,
                            x,
                            y,
                        )

                    else:
                        print(f"Unknown command: {command_name}")
                        print_help()

                except ValueError as error:
                    print(f"Invalid argument: {error}")

                except (ConnectionResetError, BrokenPipeError):
                    print()
                    print("Client disconnected.")
                    break

                except KeyboardInterrupt:
                    print()
                    print("Stopping...")
                    break


if __name__ == "__main__":
    main()