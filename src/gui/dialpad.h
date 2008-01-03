#ifndef __EKIGA_DIALPAD_H__
#define __EKIGA_DIALPAD_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EKIGA_TYPE_DIALPAD             (ekiga_dialpad_get_type())
#define EKIGA_DIALPAD(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), EKIGA_TYPE_DIALPAD, EkigaDialpad))
#define EKIGA_DIALPAD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), EKIGA_TYOE_DIALPAD, EkigaDialpadClass))
#define EKIGA_IS_DIALPAD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), EKIGA_TYPE_DIALPAD))
#define EKIGA_IS_DIALPAD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EKIGA_TYPE_DIALPAD))
#define EKIGA_DIALPAD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), EKIGA_TYPE_DIALPAD, EkigaDialpadClass))

typedef struct _EkigaDialpad        EkigaDialpad;
typedef struct _EkigaDialpadPrivate EkigaDialpadPrivate;
typedef struct _EkigaDialpadClass   EkigaDialpadClass;

struct _EkigaDialpad
{
  GtkTable             parent;
  EkigaDialpadPrivate *priv;
};

struct _EkigaDialpadClass
{
  GtkTableClass parent_class;

  void (* button_clicked) (EkigaDialpad *dialpad, const gchar *button);
};

GType      ekiga_dialpad_get_type         (void) G_GNUC_CONST;
GtkWidget *ekiga_dialpad_new              (GtkAccelGroup *accel_group);

guint      ekiga_dialpad_get_button_code  (EkigaDialpad *dialpad,
                                           char          number);

G_END_DECLS

#endif  /* __EKIGA_DIALPAD_H__ */

/* ex:set ts=2 sw=2 et: */
