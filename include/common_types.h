#ifndef COMTYPES_H
#define COMTYPES_H


#if defined(_WIN32) || defined(_WIN64)
    #define EXPORT __declspec(dllexport)
    #define _WINDOWS
#endif

#endif //COMTYPES_H