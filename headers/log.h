#ifndef LOG_H
#define LOG_H
	#ifdef DEBUG
		#include <stdio.h>
		#define log(message, ...) printf("%s(%d): " message "\n", __FILE__, __LINE__, __VA_ARGS__);
	#else
		#define log(message, ...) do {} while(0);
	#endif
#endif
