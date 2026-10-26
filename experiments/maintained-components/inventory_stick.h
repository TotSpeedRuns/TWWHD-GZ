#ifndef GZ_INVENTORY_STICK_H
#define GZ_INVENTORY_STICK_H

typedef unsigned int InvStickU;

typedef struct InvStickState {
    InvStickU frame;
    InvStickU direction;
    InvStickU ticks;
    InvStickU have_frame;
} InvStickState;

/* Pure cursor/control metadata. No controller, save, actor or native stores. */
InvStickU _gz_ins_step(InvStickState *state, InvStickU x_bits,
                     InvStickU y_bits, InvStickU digital_navigation,
                     InvStickU frame, InvStickU selection, InvStickU active);
InvStickU _gz_ins_inventory(InvStickU selection, InvStickU held);

#endif
