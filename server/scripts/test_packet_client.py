#!/usr/bin/env python3

import argparse
import socket
import struct
import sys
import time


HEADER_SIZE = 12
BODY_HEADER_SIZE = 8
MSG_ID_LOGIN_REQUEST = 1001
MSG_ID_LOGIN_RESPONSE = 1002
ERROR_CODE_OK = 0


def encode_packet(msg_id: int, seq_id: int, payload: bytes) -> bytes:
    length = BODY_HEADER_SIZE + len(payload)
    return struct.pack("!III", length, msg_id, seq_id) + payload


def encode_varint(value: int) -> bytes:
    out = bytearray()
    while value >= 0x80:
        out.append((value & 0x7F) | 0x80)
        value >>= 7
    out.append(value)
    return bytes(out)


def decode_varint(data: bytes, offset: int) -> tuple[int, int]:
    shift = 0
    value = 0

    while offset < len(data):
        byte = data[offset]
        offset += 1
        value |= (byte & 0x7F) << shift

        if byte < 0x80:
            return value, offset

        shift += 7
        if shift >= 64:
            raise ValueError("varint is too long")

    raise ValueError("truncated varint")


def encode_string_field(field_number: int, value: str) -> bytes:
    raw = value.encode("utf-8")
    tag = (field_number << 3) | 2
    return encode_varint(tag) + encode_varint(len(raw)) + raw


def encode_login_request(username: str, password: str) -> bytes:
    return (
        encode_string_field(1, username)
        + encode_string_field(2, password)
    )


def skip_field(data: bytes, offset: int, wire_type: int) -> int:
    if wire_type == 0:
        _, offset = decode_varint(data, offset)
        return offset

    if wire_type == 2:
        size, offset = decode_varint(data, offset)
        return offset + size

    raise ValueError(f"unsupported protobuf wire type: {wire_type}")


def decode_login_response(payload: bytes) -> dict:
    offset = 0
    response = {
        "error_code": ERROR_CODE_OK,
        "message": "",
        "player_id": 0,
    }

    while offset < len(payload):
        tag, offset = decode_varint(payload, offset)
        field_number = tag >> 3
        wire_type = tag & 0x07

        if field_number == 1 and wire_type == 0:
            response["error_code"], offset = decode_varint(payload, offset)
        elif field_number == 2 and wire_type == 2:
            size, offset = decode_varint(payload, offset)
            response["message"] = payload[offset:offset + size].decode("utf-8")
            offset += size
        elif field_number == 3 and wire_type == 0:
            response["player_id"], offset = decode_varint(payload, offset)
        else:
            offset = skip_field(payload, offset, wire_type)

    return response


def recv_exact(sock: socket.socket, size: int) -> bytes:
    data = bytearray()
    while len(data) < size:
        chunk = sock.recv(size - len(data))
        if not chunk:
            raise ConnectionError("connection closed while receiving data")
        data.extend(chunk)
    return bytes(data)


def recv_packet(sock: socket.socket) -> tuple[int, int, bytes]:
    header = recv_exact(sock, HEADER_SIZE)
    length, msg_id, seq_id = struct.unpack("!III", header)

    if length < BODY_HEADER_SIZE:
        raise ValueError(f"invalid response length: {length}")

    payload = recv_exact(sock, length - BODY_HEADER_SIZE)
    return msg_id, seq_id, payload


def connect(host: str, port: int) -> socket.socket:
    sock = socket.create_connection((host, port), timeout=3.0)
    sock.settimeout(3.0)
    return sock


def assert_packet(actual: tuple[int, int, bytes], expected: tuple[int, int, bytes]) -> None:
    if actual != expected:
        raise AssertionError(f"expected={expected!r}, actual={actual!r}")


def assert_login_response(
    actual_packet: tuple[int, int, bytes],
    expected_seq_id: int,
    expected_message: str = "login ok",
) -> None:
    msg_id, seq_id, payload = actual_packet

    if msg_id != MSG_ID_LOGIN_RESPONSE:
        raise AssertionError(f"expected msg_id={MSG_ID_LOGIN_RESPONSE}, actual={msg_id}")

    if seq_id != expected_seq_id:
        raise AssertionError(f"expected seq_id={expected_seq_id}, actual={seq_id}")

    response = decode_login_response(payload)

    if response["error_code"] != ERROR_CODE_OK:
        raise AssertionError(f"expected error_code=0, actual={response['error_code']}")

    if response["message"] != expected_message:
        raise AssertionError(
            f"expected message={expected_message!r}, actual={response['message']!r}"
        )

    print(f"       response={response}")


def test_login_packet(host: str, port: int) -> None:
    with connect(host, port) as sock:
        seq_id = 1
        payload = encode_login_request("alice", "123456")
        sock.sendall(encode_packet(MSG_ID_LOGIN_REQUEST, seq_id, payload))
        actual = recv_packet(sock)
        assert_login_response(actual, seq_id)


def test_sticky_login_packets(host: str, port: int) -> None:
    with connect(host, port) as sock:
        first = (
            MSG_ID_LOGIN_REQUEST,
            11,
            encode_login_request("alice", "123456"),
        )
        second = (
            MSG_ID_LOGIN_REQUEST,
            12,
            encode_login_request("bob", "abcdef"),
        )
        sock.sendall(encode_packet(*first) + encode_packet(*second))

        actual_first = recv_packet(sock)
        actual_second = recv_packet(sock)

        assert_login_response(actual_first, first[1])
        assert_login_response(actual_second, second[1])


def test_half_login_packet(host: str, port: int) -> None:
    with connect(host, port) as sock:
        seq_id = 21
        payload = encode_login_request("split_user", "split_password")
        packet = encode_packet(MSG_ID_LOGIN_REQUEST, seq_id, payload)
        mid = 5

        sock.sendall(packet[:mid])
        time.sleep(0.2)
        sock.sendall(packet[mid:])

        actual = recv_packet(sock)
        assert_login_response(actual, seq_id)


def test_invalid_packet(host: str, port: int) -> None:
    with connect(host, port) as sock:
        invalid_length = 4
        sock.sendall(struct.pack("!I", invalid_length))

        try:
            data = sock.recv(1)
        except (ConnectionResetError, TimeoutError, socket.timeout):
            return

        if data:
            raise AssertionError(f"expected connection close, got data={data!r}")


def run_test(name: str, func, host: str, port: int) -> bool:
    try:
        func(host, port)
    except Exception as exc:
        print(f"[FAIL] {name}: {exc}")
        return False

    print(f"[PASS] {name}")
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description="Nebula Duel packet protocol test client")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    parser.add_argument(
        "--case",
        choices=["all", "login", "sticky", "half", "invalid"],
        default="all",
    )
    args = parser.parse_args()

    tests = {
        "login": ("login protobuf request", test_login_packet),
        "sticky": ("sticky login packets", test_sticky_login_packets),
        "half": ("half login packet", test_half_login_packet),
        "invalid": ("invalid packet closes connection", test_invalid_packet),
    }

    selected = tests.items() if args.case == "all" else [(args.case, tests[args.case])]

    ok = True
    for _, (name, func) in selected:
        ok = run_test(name, func, args.host, args.port) and ok

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
