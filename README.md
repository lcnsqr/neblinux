Neblinux

Firmware para Arduino Nano (ATmega328p) e utilitário de configuração.

## Instruções de compilação do utilitário de configuração

    git clone https://github.com/lcnsqr/neblinux.git
    cd neblinux
    mkdir build
    cd build
    qmake6 ../src/util/Util.pro
    make
    ./Util
