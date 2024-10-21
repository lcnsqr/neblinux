Neblinux

Firmware para Arduino Nano (ATmega328p) e utilitário de configuração.

## Dependências do utilitário

Bibliotecas extras necessárias para rodar o utilitário compilado.

### Debian/Ubuntu

libqt6core6 libqt6gui6 libqt6widgets6 libqt6charts6 libqt6serialport6

### Fedora/CentOS/RHEL

qt6-qtbase qt6-qtcharts qt6-qtserialport

### Arch

qt6-base qt6-charts qt6-serialport

## Instruções de compilação do utilitário de configuração

    git clone https://github.com/lcnsqr/neblinux.git
    cd neblinux
    mkdir build
    cd build
    qmake6 ../src/util/Util.pro
    make
    ./Util
