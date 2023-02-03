#ifndef LOGO_H_
#define LOGO_H_


#include <stdint.h>

#define IMG_R1_X (112)
#define IMG_R1_Y (57)

#define IMG_R2_X (46)
#define IMG_R2_Y (76)

#define IMG_EL_X (23)
#define IMG_EL_Y (24)

#define IMG_ER_X (23)
#define IMG_ER_Y (23)

#define IMG_EYE_X (34)
#define IMG_EYE_Y (21)




extern const uint16_t base_r1[57][112];
extern const uint16_t base_r2[76][46];
extern const uint16_t eleft_1[24][23];
extern const uint16_t eleft_2[24][23];
extern const uint16_t eright_1[23][23];
extern const uint16_t eright_2[23][23];
extern const uint16_t eye_1[21][34];
extern const uint16_t eye_2[21][34];
extern const uint16_t eye_3[21][34];

extern const uint16_t level[64];
extern const uint16_t level_back[64];

extern const uint16_t level2[64];
extern const uint16_t level2_back[64];
extern const uint16_t level_gs[64];
extern const uint16_t level_gs_back[64];
extern const uint16_t level_xg[64];
extern const uint16_t level_xg_back[64];

extern uint16_t level_free[64];
extern uint16_t level_free_back[64];

#define IMG_KEY_WIDTH (12)
#define IMG_KEY_HIGHT (3)
extern const uint16_t keyboard[36];

#endif /* LOGO_H_ */
