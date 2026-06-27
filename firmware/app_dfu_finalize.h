#ifndef APP_DFU_FINALIZE_H__
#define APP_DFU_FINALIZE_H__

#include <stdbool.h>

bool app_dfu_finalize_is_ready(void);
bool app_dfu_finalize_check_and_log(void);
void app_dfu_finalize_and_reset(void);

#endif /* APP_DFU_FINALIZE_H__ */
