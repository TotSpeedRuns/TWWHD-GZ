#ifndef GZ_PRODUCTION_FEATURES_H
#define GZ_PRODUCTION_FEATURES_H
/* Private source-resource inventory experiments remain available in an
 * explicitly enabled laboratory build. This production candidate excludes
 * their fixed scratch tables and eager SDK allocation. Public save-state,
 * birth/ownership observers, forest restore and gameplay participants remain.
 */
#define GZ_RESOURCE_CAPTURE_LAB_ENABLED 0
extern const unsigned int _gz_resource_capture_lab_enabled;
#endif
