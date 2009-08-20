
#ifndef __gm_marshal_MARSHAL_H__
#define __gm_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:POINTER,STRING (gmmarshallers.list:1) */
extern void gm_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);

/* VOID:STRING,UINT (gmmarshallers.list:2) */
extern void gm_marshal_VOID__STRING_UINT (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

/* VOID:STRING,STRING (gmmarshallers.list:3) */
extern void gm_marshal_VOID__STRING_STRING (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* VOID:STRING,BOOLEAN (gmmarshallers.list:4) */
extern void gm_marshal_VOID__STRING_BOOLEAN (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);

/* BOOLEAN:STRING (gmmarshallers.list:5) */
extern void gm_marshal_BOOLEAN__STRING (GClosure     *closure,
                                        GValue       *return_value,
                                        guint         n_param_values,
                                        const GValue *param_values,
                                        gpointer      invocation_hint,
                                        gpointer      marshal_data);

/* BOOLEAN:STRING,STRING (gmmarshallers.list:6) */
extern void gm_marshal_BOOLEAN__STRING_STRING (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

G_END_DECLS

#endif /* __gm_marshal_MARSHAL_H__ */

