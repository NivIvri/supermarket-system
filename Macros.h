#ifndef MACROS_H_
#define MACROS_H_

#define CLOSE_RETURN_0(fp) {fclose(fp); return 0;};
#define CHECK_RETURN_0(val) {if (!(val)) return 0;}
#define CHECK_MSG_RETURN_0(val, msg) {if (!(val)) {printf("%s", msg); return 0;}}
#define FREE_CLOSE_FILE_RETURN_0(val, freeKey, closeP) {if (!(val)) {free(freeKey); fclose(closeP); return 0;}}

#endif /* MACROS_H_ */