#ifndef SYS_ATTRIBUTE_H
#define SYS_ATTRIBUTE_H


#define __used			__attribute__((used))
#define __unused		__attribute__((unused))
#define __packed		__attribute__((packed))
#define __section(sec)	__attribute__((section(sec)))
#define __align(base)	__attribute__((aligned(base)))
#define __alias(f)		__attribute__((alias (#f)))


#endif // SYS_ATTRIBUTE_H
