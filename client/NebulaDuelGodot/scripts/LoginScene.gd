extends Control

@onready var username_input: LineEdit = %UsernameInput
@onready var password_input: LineEdit = %PasswordInput
@onready var login_button: Button = %LoginButton
@onready var status_label: Label = %StatusLabel

func _ready() -> void:
	_setup_ui()
	if not login_button.pressed.is_connected(_on_login_button_pressed):
		login_button.pressed.connect(_on_login_button_pressed)

	NetworkClient.connected.connect(_on_server_connected)
	NetworkClient.connection_failed.connect(_on_connection_failed)
	NetworkClient.disconnected.connect(_on_server_disconnected)

func _setup_ui() -> void:
	username_input.placeholder_text = "Username"
	password_input.placeholder_text = "Password"
	password_input.secret = true

	status_label.text = ""
	login_button.text = "LOGIN"

func _on_login_button_pressed() -> void:
	var username := username_input.text.strip_edges()
	var password := password_input.text

	if username.is_empty():
		_set_status("请输入用户名")
		return

	if password.is_empty():
		_set_status("请输入密码")
		return

	login_button.disabled = true
	_set_status("准备连接服务器...")
	NetworkClient.connect_to_server()

func _on_server_connected() -> void:
	login_button.disabled = false
	_set_status("已连接服务器，Day 3 客户端验收通过")

func _on_connection_failed(message: String) -> void:
	login_button.disabled = false
	_set_status(message)

func _on_server_disconnected() -> void:
	login_button.disabled = false
	_set_status("已断开服务器连接")

func _set_status(msg: String) -> void:
	status_label.text = msg
