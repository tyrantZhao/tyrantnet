#ifndef __TYRANTNET_COMMON_ARRAY_H__
#define __TYRANTNET_COMMON_ARRAY_H__

/* 通用可拓展的数组 */
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

/* 自定义可拓展数组 */
struct array_s;

/* 新建数组 */
struct array_s* ox_array_new(int num, int element_size);
/* 销毁数组 */
void ox_array_delete(struct array_s* self);
/* 访问制定位置的数组元素 */
char* ox_array_at(struct array_s* self, int index);
/* 修改已有数组的元素 */
bool ox_array_set(struct array_s* self, int index, const void* data);
/* 拓展数组成一个新的数组 */
bool ox_array_increase(struct array_s* self, int increase_num);
/* 获取数组元素数量 */
int ox_array_num(const struct array_s* self);

#ifdef  __cplusplus
}
#endif

#endif //__TYRANTNET_COMMON_ARRAY_H__
