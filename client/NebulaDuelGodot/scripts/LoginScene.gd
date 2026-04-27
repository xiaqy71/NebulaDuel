extends Control

@onready var username_input: LineEdit = %UsernameInput
@onready var password_input: LineEdit = %PasswordInput
@onready var login_button: Button = %LoginButton
@onready var status_label: Label = %StatusLabel

func _ready() -> void:
	_setup_ui()
	login_button.pressed.connect(_on_login_button_pressed)

func _setup_ui() -> void:
	username_input.placeholder_text = "Username"
	password_input.placeholder_text = "Password"
	password_input.secret = true

	status_label.text = ""
	login_button.text = "LOGIN"

func _on_login_button_pressed() -> void:
	var username := username_input.text.strip_edges()

	if username.is_empty():
		_set_status("请输入用户名")
		return

	_set_status("准备连接服务器...")

func _set_status(msg: String) -> void:
	status_label.text = msg
