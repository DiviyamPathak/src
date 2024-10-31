#include <atf-c.h>
#include <sys/device_if.h>
#include <sys/device.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/conf.h>
#include <stdio.h>
#include <string.h>

#define	NODEV	(dev_t)(-1)
#define DV_DISK 1
#define DV_IFNET 2

//prototypes for subr and stub function
device_t create_device(const char *name, int dv_class);
void set_user_input;
int cngetsn(char *buf, int len);
void setroot_root(device_t test_bootdv,int test_bootpartition);
void setroot(device_t bootdv, int bootpartition);
void setroot_ask(device_t bootdv, int bootpartition);
int tftproot_dhcpboot(device_t bootdv);

//global variables
char rootspec;
extern dev_t rootdev;
extern device_t root_device;

device_t
create_device(const char *name, int dv_class)
{
    static struct device_t device_inter;
    memset(&device_inter, 0, sizeof(device_inter));

    device_inter.dv_class = dv_class; // Assign the device class (e.g., DV_DISK, DV_IFNET)
    strncpy(device_inter.dv_xname, name, sizeof(device_inter.dv_xname));

    device_inter.dv_cfdata = NULL;               // No config data (pseudo-device)
    device_inter.dv_cfdriver = NULL;             // No cfdriver needed for dummy
    device_inter.dv_cfattach = NULL;             // No cfattach for dummy
    device_inter.dv_unit = 0;                    // Default unit number (0)
    device_inter.dv_depth = 0;                   // No parent, root-level device
    device_inter.dv_flags = 0;                   // No flags set
    device_inter.dv_private = NULL;              // No private data
    device_inter.dv_locators = NULL;             // No locators
    device_inter.dv_properties = prop_dictionary_create(); // Create an empty dictionary
    device_inter.dv_handle = -1;                 // Invalid handle for test


    return &device_inter;
}

ATF_TC(testsetroot);
ATF_TC_HEAD(testsetroot, tc)
{

    atf_tc_set_md_var(tc, "descr", "test setroot for rootspec and md_is_root");
}

ATF_TC_BODY(testsetroot, tc)
{
    int test_bootpartition = 0; 
    device_t test_bootdv;
    char* diskname = "wd0"; 
    test_bootdv = create_device(diskname,DV_DISK);
    int md_is_root = 1;
    rootspec = NULL;

    setroot(test_bootdv,test_bootpartition);
    ATF_CHECK_STREQ("wd0", (char)rootspec);
    ATF_REQUIRE(rootdev == NODEV );
    ATF_CHECK_STREQ("md0",device_xname(test_bootdv)
);

}

ATF_TC(testsetroot2);
ATF_TC_HEAD(testsetroot2, tc)
{
    atf_tc_set_md_var(tc, "descr", "test for tftproot root device");
}

ATF_TC_BODY(testsetroot2, tc)
{
    device_t test_bootdv = NULL;
    rootspec = "eth0";

    setroot_root(test_bootdv,test_bootpartition);

    ATF_CHECK_STREQ("eth0", (char)rootspec);
    // how to check for return status of tftp func ?


}

// for setroot_root 
ATF_TC(setroot_root_rootdevice_specified);
ATF_TC_HEAD(setroot_root_rootdevice_specified, tc)
{
    atf_tc_set_md_var(tc, "descr", "test for setroot_root with specified root device");
}

ATF_TC_BODY(setroot_root_rootdevice_specified, tc)
{
    device_t test_bootdv = NULL;
    rootspec = "eth0";
    int bootpartition= 0;
    setroot_root(test_bootdv,test_bootpartition);

    ATF_CHECK_STREQ("eth0", device_xname(rootdv));
    ATF_CHECK_STREQ("eth0a", device_xname(rootdv));


}

ATF_TC(setroot_null_rootspec);
TF_TC_HEAD(setroot_null_rootspec, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test setroot_root with rootspec NULL, using boot device as root device");
}

ATF_TC_BODY(setroot_null_rootspec, tc)
{
    device_t bootdv = create_device("eth0",DV_IFNET)
    int bootpartition = 0;
    rootspec = NULL;  // No specific root specified

    setroot_root(bootdv, bootpartition);

    ATF_CHECK(root_device == bootdv);
}

ATF_TC_HEAD(setroot_defined_rootspec, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test setroot_root with defined rootspec that should be resolved to a device");
}

ATF_TC_BODY(setroot_defined_rootspec, tc)
{
    device_t expected_device = create_device("sd1a",DV_DISK);
    rootspec = "sd1a";  // Define the expected root device

    setroot_root(NULL, 0);

    ATF_CHECK(root_device == expected_device);
}

ATF_TC_HEAD(setroot_device_not_configured, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test setroot_root with an invalid rootspec to verify error handling");
}

ATF_TC_BODY(setroot_device_not_configured, tc)
{
    device_t bootdv = create_device("wd0a",DV_DISK);
    rootspec = "invdas";  // An invalid device name

    setroot_root(bootdv, 0);

    ATF_CHECK_MSG(root_device == NULL, "Expected root_device to be NULL for unconfigured device");
}

ATF_TC_HEAD(setroot_no_partition_support, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test setroot_root with a device that does not support partitions");
}

ATF_TC_BODY(setroot_no_partition_support, tc)
{
    device_t bootdv = create_device("eth0",DV_IFNET);
    int bootpartition = 0;
    rootspec = NULL;  // No specific root specified

    // Simulate the device as one that does not support partitions
    setroot_root(bootdv, bootpartition);

    ATF_CHECK_MSG(root_device == bootdv, "Expected root_device to match boot device without partitions");
}

//for setroot_ask have to mock cngetsn for user input may change in future
static char mock_input[128] = {0};
static int mock_input_set = 0;


void set_user_input(const char *input) {
    strncpy(mock_input, input, sizeof(mock_input) - 1);
    mock_input[sizeof(mock_input) - 1] = '\0';  // Ensure null-termination
    mock_input_set = 1;  
}

// Mock version of cngetsn to use mock input during tests
int cngetsn(char *buf, int len)
{
    if (mock_input_set) {
        strncpy(buf, mock_input, len - 1);
        buf[len - 1] = '\0';  // Ensure null-termination
        mock_input_set = 0;   ~~
        return strlen(buf);
    }


ATF_TC(root_device_default);
ATF_TC_HEAD(root_device_default, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test default root device selection when bootdv is set");
}
ATF_TC_BODY(root_device_default, tc)
{
    device_t bootdv = create_device("bootdev",DV_DISK);
    int bootpartition = 0;  // Default partition

    setroot_ask(bootdv, bootpartition);

    ATF_REQUIRE(strcmp(device_xname(root_device), "bootdev") == 0);
}

ATF_TC(root_device_user_input);
ATF_TC_HEAD(root_device_user_input, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test user-selected root device different from default");
}
ATF_TC_BODY(root_device_user_input, tc)
{
    device_t bootdv = create_device("bootdev",DV_DISK);
    int bootpartition = 1;
    set_user_input("sd3");

    setroot_ask(bootdv, bootpartition);

    ATF_REQUIRE(strcmp(device_xname(root_device), "sd3") == 0);
}

ATF_TC(root_device_no_default);
ATF_TC_HEAD(root_device_no_default, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test root device selection when no bootdv is provided");
}
ATF_TC_BODY(root_device_no_default, tc)
{
    set_user_input("sd1");

    setroot_ask(NULL, 0);

    ATF_REQUIRE(strcmp(device_xname(root_device), "sd1") == 0);
}

ATF_TC(dump_device_default);
ATF_TC_HEAD(dump_device_default, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test default dump device setup based on root device");
}
ATF_TC_BODY(dump_device_default, tc)
{
    device_t bootdv = create_device("bootdev",DV_DISK);
    int bootpartition = 0;

    setroot_ask(bootdv, bootpartition);

    ATF_REQUIRE(strcmp(device_xname(dump_device), "bootdev") == 0);
}

ATF_TC(dump_device_user_none);
ATF_TC_HEAD(dump_device_user_none, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test dump device selection set to none by user");
}
ATF_TC_BODY(dump_device_user_none, tc)
{
    device_t bootdv = create_device("bootdev",DV_DISK);
    set_user_input("none");

    setroot_ask(bootdv, 0);

    ATF_REQUIRE(dump_device == NULL);
}

ATF_TC(filesystem_default);
ATF_TC_HEAD(filesystem_default, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test default filesystem selection when root filesystem type is set");
}
ATF_TC_BODY(filesystem_default, tc)
{
    device_t bootdv = create_device("bootdev",DV_DISK);

    setroot_ask(bootdv, 0);

    ATF_REQUIRE(strcmp(rootfstype, "ffs") == 0);
}

ATF_TC(filesystem_user_generic);
ATF_TC_HEAD(filesystem_user_generic, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test user setting filesystem type to 'generic'");
}
ATF_TC_BODY(filesystem_user_generic, tc)
{
    device_t bootdv = create_device("bootdev",DV_DISK);
    set_user_input("generic");

    setroot_ask(bootdv, 0);

    ATF_REQUIRE(strcmp(rootfstype, "any") == 0);
}

//testcases for tftproot_dhcp

ATF_TC_HEAD(tftproot_with_valid_rootspec, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test tftproot_dhcpboot with a valid rootspec that matches a network interface");
}

ATF_TC_BODY(tftproot_with_valid_rootspec, tc)
{
    rootspec = "eth0"; 

    device_t bootdv = NULL;
    int error = tftproot_dhcpboot(bootdv);

    ATF_CHECK_MSG(error == 0, "Expected error to be 0 when using a valid rootspec");
    ATF_CHECK_MSG(root_device != NULL, "Expected root_device to be set when using a valid rootspec");
}

ATF_TC_HEAD(tftproot_with_null_rootspec_and_valid_bootdv, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test tftproot_dhcpboot with NULL rootspec and valid boot device as network interface");
}

ATF_TC_BODY(tftproot_with_null_rootspec_and_valid_bootdv, tc)
{
    rootspec = NULL;
    device_t bootdv = create_device("eth0",DV_IFNET);

    int error = tftproot_dhcpboot(bootdv);

    ATF_CHECK_MSG(error == 0, "Expected error to be 0 with valid boot device as network interface");
    ATF_CHECK_MSG(root_device == bootdv, "Expected root_device to match boot device when rootspec is NULL");
}

ATF_TC_HEAD(tftproot_with_invalid_rootspec, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test tftproot_dhcpboot with an invalid rootspec, expecting failure");
}

ATF_TC_BODY(tftproot_with_invalid_rootspec, tc)
{
    rootspec = "invalid_iface";  // Assume this is an invalid network interface name
    device_t bootdv = NULL;

    int error = tftproot_dhcpboot(bootdv);

    ATF_CHECK_MSG(error != 0, "Expected error code to indicate failure with an invalid rootspec");
    ATF_CHECK_MSG(root_device == NULL, "Expected root_device to be NULL when rootspec is invalid");
}

ATF_TC_HEAD(tftproot_with_valid_rootspec_no_device_match, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test tftproot_dhcpboot with valid rootspec but no matching device found");
}

ATF_TC_BODY(tftproot_with_valid_rootspec_no_device_match, tc)
{
    rootspec = "eth0";  
    device_t bootdv = NULL;

    device_t
    device_find_by_xname(const char* name){
        return NULL;
    }
    int error = tftproot_dhcpboot(bootdv);

    ATF_CHECK_MSG(error != 0, "Expected error code to indicate failure when no device is found for valid rootspec");
    ATF_CHECK_MSG(root_device == NULL, "Expected root_device to be NULL when no matching device is found");
}

ATF_TC_HEAD(tftproot_with_nfs_boot_failure, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test tftproot_dhcpboot where nfs_boot_init fails");
}

ATF_TC_BODY(tftproot_with_nfs_boot_failure, tc)
{
    rootspec = NULL;
    device_t bootdv =create_device("eth0",DV_IFNET);
    int
    nfs_boot_init(struct nfs_diskless *nd, struct lwp *lwp)
    {
        return -1;
    }
    // Simulate a failure in nfs_boot_init
    int error = tftproot_dhcpboot(bootdv);

    ATF_CHECK_MSG(error != 0, "Expected non-zero error code if nfs_boot_init fails");
    ATF_CHECK_MSG(root_device == NULL, "Expected root_device to be NULL on nfs_boot_init failure");
}

ATF_TC_HEAD(tftproot_with_tftproot_getfile_failure, tc)
{
    atf_tc_set_md_var(tc, "descr", "Test tftproot_dhcpboot where tftproot_getfile fails");
}

ATF_TC_BODY(tftproot_with_tftproot_getfile_failure, tc)
{
    rootspec = NULL;
    device_t bootdv =create_device("eth0",DV_IFNET);

    static int
    tftproot_getfile(struct tftproot_handle *trh, struct lwp *l)
    {
        return -1;
    }
    int error = tftproot_dhcpboot(bootdv);

    ATF_CHECK_MSG(error != 0, "Expected non-zero error code if tftproot_getfile fails");
    ATF_CHECK_MSG(root_device != NULL, "Expected root_device to be set before tftproot_getfile failure occurs");
}

ATF_TP_ADD_TCS(tp) {
    ATF_TP_ADD_TC(tp, testsetroot);
    // ATF_TP_ADD_TC(tp, setroot_root_rootdevice_specified);


    ATF_TP_ADD_TC(tp, root_device_default);
    ATF_TP_ADD_TC(tp, root_device_user_input);
    ATF_TP_ADD_TC(tp, root_device_no_default);
    ATF_TP_ADD_TC(tp, dump_device_default);
    ATF_TP_ADD_TC(tp, dump_device_user_none);
    ATF_TP_ADD_TC(tp, filesystem_default);
    ATF_TP_ADD_TC(tp, filesystem_user_generic);
    ATF_TP_ADD_TC(tp, tftproot_with_valid_rootspec);
    ATF_TP_ADD_TC(tp, tftproot_with_null_rootspec_and_valid_bootdv);
    ATF_TP_ADD_TC(tp, tftproot_with_invalid_rootspec);
    ATF_TP_ADD_TC(tp, tftproot_with_valid_rootspec_no_device_match);
    ATF_TP_ADD_TC(tp, tftproot_with_nfs_boot_failure);
    ATF_TP_ADD_TC(tp, tftproot_with_tftproot_getfile_failure);

    return atf_no_error();

}
