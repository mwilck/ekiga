#define PPLUGIN_STATIC_LOAD(serviceName, serviceType) \
  class PPlugin_##serviceType##_##serviceName##_Registration; \
  extern PPlugin_##serviceType##_##serviceName##_Registration PPlugin_##serviceType##_##serviceName##_Registration_Instance; \
  PPlugin_##serviceType##_##serviceName##_Registration const *PPlugin_##serviceType##_##serviceName##_Registration_Static_Library_Loader = &PPlugin_##serviceType##_##serviceName##_Registration_Instance;

PPLUGIN_STATIC_LOAD(WindowsMultimedia, PSoundChannel);
PPLUGIN_STATIC_LOAD(DirectShow, PVideoInputDevice);
