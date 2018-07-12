#ifndef FALCONLIC_LICENSE_H
#define FALCONLIC_LICENSE_H


namespace falconlic {

namespace flexlm {
  typedef enum {
    FALCON_XILINX=0, 
    FALCON_ALTERA, 
    FALCON_CUSTOM, 
    FALCON_RT, 
    FALCON_DNA
  } Feature;

  void add_feature(Feature f);
}

void enable_aws();       // default: disabled
void enable_hwc();       // default: disabled
void enable_flexlm();    // default: disabled 

void disable_aws();
void disable_hwc();
void disable_flexlm();

void set_verbose(int v); // default: 0

int license_verify();
int license_clean();

const int SUCCESS        = 0;
const int API_ERROR      = -1;
const int SIGN_ERROR     = -2;
const int PARSE_ERROR    = -3;
const int PRD_VERF_ERROR = -4;
const int FLEXLM_ERROR   = -5;

} // namespace falconlic

#endif
