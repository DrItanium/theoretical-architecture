#ifndef _IRIS_BASE_H
#define _IRIS_BASE_H
#define INDIRECTOR(a, ...) PRIMITIVE_INDIRECTOR(a, __VA_ARGS__)
#define PRIMITIVE_INDIRECTOR(a, ...) a ## __VA_ARGS__
typedef unsigned char byte;
namespace iris {


template<typename T, typename F, T bitmask, T shiftcount = 0>
F decodeBits(T input) {
   return (F)((input & bitmask) >> shiftcount);
}

template<typename T, typename F, T bitmask, T shiftcount = 0>
F decodeBits(T* input) {
   return (F)((*input & bitmask) >> shiftcount);
}

template<typename T, typename F, T bitmask, T shiftcount = 0>
T encodeBits(T input, F value) {
	return (T)((input & ~bitmask) | (value << shiftcount));
}

template<typename T, typename F, T bitmask, T shiftcount = 0> 
void encodeBits(T* input, F value) {
	*input = ((*input & ~bitmask) | (value << shiftcount));
}

}
#endif
