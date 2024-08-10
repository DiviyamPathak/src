#include <atf-c.h>
#include <sys/device_if.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/conf.h>
#include <stdio.h>
#include <string.h>




ATF_TC(testsetroot);
ATF_TC_HEAD(testsetroot, tc) {
    atf_tc_set_md_var(tc, "descr", "test setroot");
}

ATF_TC_BODY(testsetroot, tc) {
    setroot_root();
    ATF_CHECK_STREQ("wd0", rootdvm);
}

ATF_TC(testsetroot2);
ATF_TC_HEAD(testsetroot2, tc) {
    atf_tc_set_md_var(tc, "descr", "impl");
}

ATF_TC_BODY(testsetroot2, tc) {
}

ATF_TP_ADD_TCS(tp) {
    ATF_TP_ADD_TC(tp, testsetroot);
    ATF_TP_ADD_TC(tp, testsetroot2);
    return atf_no_error();
}
