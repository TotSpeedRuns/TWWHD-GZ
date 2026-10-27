#ifndef GZ_DELIVERY_COPIES_H
#define GZ_DELIVERY_COPIES_H

typedef unsigned int DelU;
typedef unsigned char DelByte;
enum { DEL_OK = 0, DEL_FULL = 1, DEL_ARGUMENT = 2,
       DEL_TOGGLE = 1, DEL_ADD = 10, DEL_REMOVE = 0xffffffffu,
       DEL_NONE = 255, DEL_FIRST = 140, DEL_LAST = 158, DEL_BUTTONS = 5 };

/* Detached copies only: native delivery slots Save+86..8d, availability90,
 * five saved indices+9..d and their five usable-item bytes. No native API,
 * global, pointer dereference or owner lease exists in this component. */
typedef struct {
    DelByte slots[8];
    DelU flags;
    DelByte selected[DEL_BUTTONS], usable[DEL_BUTTONS], reserved[2];
} DelInput;

typedef struct {
    DelInput after;
    DelU before_count, after_count, operation, changed, grant_bag, binding_mask;
} DelPlan;

typedef char DelInputSize[sizeof(DelInput) == 24 ? 1 : -1];
typedef char DelPlanSize[sizeof(DelPlan) == 48 ? 1 : -1];

DelU _gz_del_count(const DelByte slots[8], DelU item);
DelU _gz_del_plan(const DelInput *, DelU item, DelU action, DelPlan *);

#endif
