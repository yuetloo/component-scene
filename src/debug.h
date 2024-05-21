
#ifndef __FIREFLY_INTERNAL_DEBUG_H__
#define __FIREFLY_INTERNAL_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


// If indent is -1, do not log anything, just return if the type matches
typedef int32_t (*DebugDumpFunc)(FfxNode node, int32_t indent);

int32_t debugDumpGroup(FfxNode node, int32_t indent);
int32_t debugDumpBox(FfxNode node, int32_t indent);
int32_t debugDumpFill(FfxNode node, int32_t indent);
int32_t debugDumpImage(FfxNode node, int32_t indent);

int32_t debugDumpNode(FfxNode node, int32_t indent);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FIREFLY_INTERNAL_DEBUG_H__ */
