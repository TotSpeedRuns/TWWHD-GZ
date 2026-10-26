#ifndef GZ_FACTORY_READ_SCOPE_H
#define GZ_FACTORY_READ_SCOPE_H

/* Private fixed-image control metadata. This describes one five-view READ
 * batch; it never substitutes for actor, process, collider, or writer checks. */
typedef struct {
    unsigned int magic;
    unsigned int bytes;
    unsigned int version;
    unsigned int valid;
    unsigned int serial;
    unsigned int token;
    unsigned int frame;
    unsigned int epoch;
    unsigned int thread;
    unsigned int request;
    unsigned int ack;
    unsigned int service;
    unsigned int service_calls;
    unsigned int bmh_seal;
    unsigned int progress_seal;
    unsigned int progress_mask;
    unsigned int plan_seal;
    unsigned int run_seal;
    unsigned int view_seal;
    unsigned int hbp_seal;
    unsigned int pin;
    unsigned int source;
    unsigned int used;
    unsigned int selected;
    unsigned int hold;
} GzFactoryReadScope;

typedef char GzFactoryReadScopeSize[sizeof(GzFactoryReadScope) == 100 ? 1 : -1];

/* Only the real current READ batch can return its own fixed-image metadata.
 * These queries do not acquire a pin, read the archive, or grant a writer. */
unsigned int _gz_bmh_read_context_owned(unsigned int token);
const GzFactoryReadScope *_gz_bmh_source_context(unsigned int token,
                                               unsigned int frame);

#endif
