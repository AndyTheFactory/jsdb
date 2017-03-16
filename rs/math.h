
// bit manipulation, from macros.h

inline uint32 SetBit(uint32 Number,int Shift) { return SETBIT(Number,Shift); }

inline uint32 ClearBit(uint32 Number,int Shift) {return CLEARBIT(Number,Shift);}

inline bool IsBitSet(uint32 Number,int Shift) { return GETBIT(Number,Shift) != 0; }

inline double LLpercent(long X, long Y) { return PERCENT(X,Y); }

inline double DDiv(double X, double Y) { return DDIV(X,Y); }

inline double DDPercent(double X, double Y) { return PERCENT(X,Y); }
