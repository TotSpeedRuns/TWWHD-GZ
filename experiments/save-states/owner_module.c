#define _gz_roh_begin _gz_som_begin_previous
#define _gz_roh_abort_clean _gz_som_abort_previous
#define _gz_roh_user_guard _gz_som_guard_previous
#define _gz_roh_work_replaced _gz_som_work_previous
#include "owner_query.c"
extern RohOwner _gz_ni_owner __attribute__((alias("_gz_roh_owner")));
/* Only the sealed inventory-bomb lane may replace topology comparisons.
 * Idle/ordinary callers retain the literal default scope0 / denied60. */
extern u32 _gz_boh_scope(u32),_gz_boh_validate(u32);
u32 _gz_roh_lifetime_scope(u32 token){return _gz_boh_scope(token);}
u32 _gz_roh_lifetime_validate(u32 token){return _gz_boh_validate(token);}
