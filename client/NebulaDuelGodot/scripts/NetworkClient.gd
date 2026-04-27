extends Node

signal connected
signal connection_failed(message: String)
signal disconnected

const DEFAULT_HOST := "127.0.0.1"
const DEFAULT_PORT := 9000
const CONNECT_TIMEOUT_SEC := 5.0

var _peer := StreamPeerTCP.new()
var _host := DEFAULT_HOST
var _port := DEFAULT_PORT
var _was_connected := false
var _connect_started_msec := 0

func _process(_delta: float) -> void:
	var status := _peer.get_status()
	if status == StreamPeerTCP.STATUS_NONE and _was_connected:
		_reset_peer()
		disconnected.emit()
		return
	elif status == StreamPeerTCP.STATUS_NONE:
		return

	_peer.poll()

	status = _peer.get_status()
	if status == StreamPeerTCP.STATUS_CONNECTED and not _was_connected:
		_was_connected = true
		connected.emit()
	elif status == StreamPeerTCP.STATUS_CONNECTING and _is_connect_timed_out():
		var timeout_message := "连接服务器超时: %s:%d" % [_host, _port]
		_reset_peer()
		connection_failed.emit(timeout_message)
	elif status == StreamPeerTCP.STATUS_ERROR:
		var message := "连接服务器失败: %s:%d" % [_host, _port]
		_reset_peer()
		connection_failed.emit(message)
	elif status == StreamPeerTCP.STATUS_NONE and _was_connected:
		_reset_peer()
		disconnected.emit()

func connect_to_server(host: String = DEFAULT_HOST, port: int = DEFAULT_PORT) -> void:
	if is_connected_to_server():
		return

	_host = host
	_port = port
	_reset_peer()

	var error := _peer.connect_to_host(_host, _port)
	if error != OK:
		_reset_peer()
		connection_failed.emit("连接请求创建失败: %s" % error_string(error))
	else:
		_connect_started_msec = Time.get_ticks_msec()

func disconnect_from_server() -> void:
	if _peer.get_status() != StreamPeerTCP.STATUS_NONE:
		_peer.disconnect_from_host()
	_reset_peer()
	disconnected.emit()

func is_connected_to_server() -> bool:
	return _peer.get_status() == StreamPeerTCP.STATUS_CONNECTED

func send_text(message: String) -> void:
	if not is_connected_to_server():
		connection_failed.emit("尚未连接服务器")
		return

	_peer.put_data(message.to_utf8_buffer())

func _reset_peer() -> void:
	_peer = StreamPeerTCP.new()
	_was_connected = false
	_connect_started_msec = 0

func _is_connect_timed_out() -> bool:
	if _connect_started_msec <= 0:
		return false

	var elapsed_sec := float(Time.get_ticks_msec() - _connect_started_msec) / 1000.0
	return elapsed_sec >= CONNECT_TIMEOUT_SEC
