#ifndef LOG_H
#define LOG_H


#define eprintf(fmt, args...)						\
do {									\
	fprintf(stderr, "%s: " fmt, ##args);		\
} while (0)


#define dprintf(fmt, args...)						\
do {									\
	fprintf(stderr, "[%s %d]: " fmt, __FUNCTION__, __LINE__, ##args);		\
} while (0)


#endif	/* LOG_H */
