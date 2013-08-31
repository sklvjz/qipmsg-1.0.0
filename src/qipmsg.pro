TEMPLATE = app

CONFIG += qt warn_on release

QT += network
LIBS += -lasound

RESOURCES = icons.qrc


HEADERS += \
	about_dialog.h \
	msg_server.h \
	constants.h \
	version.h \
	dir_dialog.h \
	recv_file_finish_dialog.h \
	global.h \
	helper.h \
	ipmsg.h \
	lockfile.h \
	main_window.h \
        search_user_dialog.h \
        msg.h \
	msg_base.h \
	send_msg.h \
	recv_msg.h \
	msg_thread.h \
	msg_readed_window.h \
	msg_window.h \
	preferences.h \
	qipmsg.h \
        recv_file.h \
        recv_file_handle.h \
	recv_file_model.h \
	recv_file_map.h \
	recv_file_transfer.h \
	recv_file_thread.h \
	recvfilesortfilterproxymodel.h \
	retry_recv_file_dialog.h \
	recv_file_window.h \
	send_file_model.h \
	send_file_map.h \
	send_file.h \
	send_file_handle.h \
	send_file_thread.h \
	send_file_window.h \
	send_file_manager.h \
	serve_socket.h \
	setup_window.h \
	sizecolumndelegate.h \
	systray.h \
	file_server.h \
	transfer_codec.h \
	translator.h \
	owner.h \
	sound.h \
	sound_thread.h \
	user_manager.h \
	transfer_file_window.h \
	transfer_file_model.h \
	window_manager.h

SOURCES += \
	about_dialog.cpp \
	msg_server.cpp \
	dir_dialog.cpp \
	recv_file_finish_dialog.cpp \
	global.cpp \
	helper.cpp \
	lockfile.cpp \
	main.cpp \
	main_window.cpp \
        search_user_dialog.cpp \
        msg.cpp \
	msg_base.cpp \
	send_msg.cpp \
	recv_msg.cpp \
	msg_thread.cpp \
	msg_readed_window.cpp \
	msg_window.cpp \
	preferences.cpp \
	qipmsg.cpp \
        recv_file.cpp \
        recv_file_handle.cpp \
	recv_file_model.cpp \
	recv_file_map.cpp \
	recv_file_transfer.cpp \
	recv_file_thread.cpp \
	recvfilesortfilterproxymodel.cpp \
	retry_recv_file_dialog.cpp \
	recv_file_window.cpp \
	send_file_model.cpp \
	send_file_map.cpp \
	send_file.cpp \
	send_file_handle.cpp \
	send_file_thread.cpp \
	send_file_window.cpp \
	send_file_manager.cpp \
	serve_socket.cpp \
	setup_window.cpp \
	sizecolumndelegate.cpp \
	systray.cpp \
	file_server.cpp \
	transfer_codec.cpp \
	translator.cpp \
	owner.cpp \
	sound.cpp \
	sound_thread.cpp \
	user_manager.cpp \
	transfer_file_window.cpp \
	transfer_file_model.cpp \
	window_manager.cpp

TRANSLATIONS = translations/qipmsg_zh_CN.ts

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj

  DEFINES += DATA_PATH=$(DATA_PATH)
  DEFINES += SOUND_PATH=$(SOUND_PATH)
  DEFINES += TRANSLATION_PATH=$(TRANSLATION_PATH)
  DEFINES += ICON_PATH=$(ICON_PATH)
  #DEFINES += NO_DEBUG_ON_CONSOLE
}

win32 {
}

