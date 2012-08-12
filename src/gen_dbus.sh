#!/bin/sh

qdbusxml2cpp -N -p keyboardproxy -c KeyboardProxy org.fcitx.Fcitx.Keyboard.xml -i layout.h
qdbusxml2cpp -N -p inputmethodproxy -c InputMethodProxy org.fcitx.Fcitx.InputMethod.xml -i im.h
