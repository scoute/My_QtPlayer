#ifndef QSYSTEMDETECTION_H
#define QSYSTEMDETECTION_H
#include <QString>

// этот заголовочный файл определяет тип ОС, чтобы потом можно было писать
// платформо-зависимый код, без явного указания типа ОС в файле проекта (.pro).


// СТРАННО. Я убрал этот заголовочный файл из mainwindow.cpp, но макросы (Q_OS_LINUX) всё равно сработали ...



// тут происходит вся магия автоопределения ОС
#if defined(__linux__) || defined(__linux)
#  define Q_OS_LINUX
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64) || defined(Q_OS_WINRT)
#  define Q_OS_WIN
#endif

#if defined(Q_OS_WIN)
#  undef Q_OS_UNIX
#elif !defined(Q_OS_UNIX)
#  define Q_OS_UNIX
#endif




#endif // QSYSTEMDETECTION_H
