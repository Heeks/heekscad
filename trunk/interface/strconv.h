#ifdef UNICODE
extern const char* Ttc(const wchar_t* str);
extern const wchar_t* Ctt(const char* str);
#else
#define Ttc(x) x
#define Ctt(x) x
#endif
