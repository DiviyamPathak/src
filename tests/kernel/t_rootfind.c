#include <atf-c.h>
#include <sys/types.h>
// #include <sys/systm.h>
#include <sys/device.h>
// #include <sys/cdefs.h>
#include <machine/bootinfo.h>

#include <stdbool.h>
#include <string.h>

typedef struct {
    char devname[16];
    int partition;
} btinfo_rootdevice;

typedef struct {
    int biosdev;
    int startblk;
    int nblks;
    char matchhash[16];
} btinfo_bootwedge;

typedef struct {
    int biosdev;
    int partition;
    struct {
        int type;
        int checksum;
        char packname[16];
    } label;
} btinfo_bootdisk;

typedef struct {
    int biosdev;
} btinfo_biosgeom;

typedef struct {
    int type;
    void* info;
} bootinfo;

extern device_t booted_device;
int booted_partition;
int booted_nblks;
const char* booted_method;

void findroot(void);
void 
findroot(void){
    
}
ATF_TC(test_btinfo_rootdevice);
ATF_TC_HEAD(test_btinfo_rootdevice,tc)
{
    atf_tc_set_md_var(tc, "descr", "test finroot for btinfo_rootdevice ");
}
ATF_TC_BODY(test_btinfo_rootdevice, tc)
{
    bootinfo bi;
    btinfo_rootdevice bir = {
        .devname = "sd0a",
        .partition = 0
    };
    bi.type = BTINFO_ROOTDEVICE;
    bi.info = &bir;
    findroot();
    ATF_REQUIRE_STREQ(device_xname(booted_device), "sd0");
    ATF_REQUIRE_EQ(booted_partition, 0);
    ATF_REQUIRE_EQ(booted_nblks, 0);
    ATF_REQUIRE_STREQ(booted_method, "bootinfo/rootdevice");
}
ATF_TC(test_btinfo_bootwedge,tc);
ATF_TC_HEAD(test_btinfo_bootwedge,tc)
{
    atf_tc_set_md_var(tc, "descr", "test finroot for btinfo_bootwedge ");
}
ATF_TC_BODY(test_btinfo_bootwedge, tc)
{
    bootinfo bi;
    btinfo_bootwedge biw = {
        .biosdev = 0x80,
        .startblk = 100,
        .nblks = 50,
        .matchhash = "testhash"
    };
    bi.type = BTINFO_BOOTWEDGE;
    bi.info = &biw;
    findroot();
    ATF_REQUIRE_STREQ(device_xname(booted_device), "wd0");
    ATF_REQUIRE_EQ(booted_partition, 0);
    ATF_REQUIRE_EQ(booted_nblks, 50);
    ATF_REQUIRE_STREQ(booted_method, "bootinfo/bootwedge");
}
ATF_TC(test_btinfo_bootdisk);
ATF_TC_HEAD(test_btinfo_bootdisk,tc)
{
    atf_tc_set_md_var(tc, "descr", "test finroot for btinfo_bootdisk ");
}
ATF_TC_BODY(test_btinfo_bootdisk, tc)
{
    bootinfo bi;
    btinfo_bootdisk bid = {
        .biosdev = 0x80,
        .partition = 1,
        .label = {
            .type = 0,
            .checksum = 0,
            .packname = "disklabel"
        }
    };
    bi.type = BTINFO_BOOTDISK;
    bi.info = &bid;
    findroot();
    ATF_REQUIRE_STREQ(device_xname(booted_device), "wd0");
    ATF_REQUIRE_EQ(booted_partition, 1);
    ATF_REQUIRE_EQ(booted_nblks, 0);
    ATF_REQUIRE_STREQ(booted_method, "bootinfo/bootdisk");
}
ATF_TC(test_btinfo_biosgeom);
ATF_TC_HEAD(test_btinfo_biosgeom,tc)
{
    atf_tc_set_md_var(tc, "descr", "test finroot for btinfo_biosgeom");
}
ATF_TC_BODY(test_btinfo_biosgeom, tc)
{
    bootinfo bi;
    btinfo_biosgeom bibg = {
        .biosdev = 0x90 // CDROM device
    };
    bi.type = BTINFO_BIOSGEOM;
    bi.info = &bibg;
    findroot();
    ATF_REQUIRE_STREQ(device_xname(booted_device), "cd0");
    ATF_REQUIRE_EQ(booted_partition, 0);
    ATF_REQUIRE_EQ(booted_nblks, 0);
    ATF_REQUIRE_STREQ(booted_method, "bootinfo/biosgeom");
}
ATF_TC(test_no_bootinfo)
ATF_TC_HEAD(test_no_bootinfo,tc)
{
    atf_tc_set_md_var(tc, "descr", "test finroot for no boot info");
}
ATF_TC_BODY(test_no_bootinfo, tc)
{
    booted_device = NULL;
    booted_partition = -1;
    booted_nblks = -1;
    booted_method = NULL;
    findroot();
    ATF_REQUIRE(booted_device == NULL);
    ATF_REQUIRE_EQ(booted_partition, -1);
    ATF_REQUIRE_EQ(booted_nblks, -1);
    ATF_REQUIRE(booted_method == NULL);
}

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, test_btinfo_rootdevice);
    ATF_TP_ADD_TC(tp, test_btinfo_bootwedge);
    ATF_TP_ADD_TC(tp, test_btinfo_bootdisk);
    ATF_TP_ADD_TC(tp, test_btinfo_biosgeom);
    ATF_TP_ADD_TC(tp, test_no_bootinfo);
    return atf_no_error();
}
