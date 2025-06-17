set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_wrap_ui(UI_HEADERS
    )

add_executable(chessqt
    main.cpp
    login.cpp
    mainwindow.cpp
    chessboard.cpp
    boardview.cpp
    utils.cpp
    resources.qrc
)

target_link_libraries(chessqt PRIVATE Qt6::Widgets Qt6::Sql Qt6::Core Qt6::Network)

install(TARGETS chessqt RUNTIME DESTINATION bin)
