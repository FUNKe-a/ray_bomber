# PROTOCOL LAYOUT GUIDE

data endianness = **big endian**

## BASIC LAYOUT

### Header

The message header specifies the type of the message and the length of the message's body in bytes.

| field  | size         |
|--------|--------------|
| Type   | 1 byte       |
| Length | 2 bytes      |

### Body

The message body holds the data. The data layout will change based on the message's type.

| field | size         |
|-------|--------------|
| Body  | Length bytes |


## Message Types

|    msg name   | ID |    msg route    |
|:-------------:|:--:|:---------------:|
| Greeting      | 0  | server->player  |
| PlayerJoined  | 1  | server->players |
| PlayerLeft    | 2  | server->players |
| MoveRequested | 3  | player->server  |
| PlayerMoved   | 4  | server->players |
