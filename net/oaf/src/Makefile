oaf-objs := app_filter.o af_utils.o  af_config.o regexp.o cJSON.o af_log.o af_client.o af_client_fs.o af_conntrack.o af_rule_config.o af_user_config.o af_whitelist_config.o
obj-m += oaf.o

# Legacy vendor module (destan19/OpenAppFilter) predates CONFIG_WERROR-by-
# default; kernel 6.18 promotes its unused-variable / missing-prototypes
# warnings to errors. Un-promote at the module level: package EXTRA_CFLAGS do
# NOT reach the in-kernel module compile, but ccflags-y here lands after the
# kernel's -Werror=. Harmless on older kernels.
ccflags-y += -Wno-error
